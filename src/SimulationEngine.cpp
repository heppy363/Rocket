#include "rocket/SimulationEngine.hpp"

#include <algorithm>

#include "rocket/RungeKutta4.hpp"

namespace rocket {

Quaternion lerpQuaternion(
    const Quaternion& a,
    const Quaternion& b,
    double t) noexcept {
    const Quaternion blended = ((1.0 - t) * a + t * b).normalized();
    return blended.magnitude() > 0.0 ? blended : a;
}

FlightState sampleTrajectoryState(
    const std::deque<TrajectorySample>& history,
    double time_s) noexcept {
    if (history.empty()) {
        return {};
    }
    if (time_s <= history.front().time_s) {
        return history.front().state;
    }
    if (time_s >= history.back().time_s) {
        return history.back().state;
    }

    for (std::size_t index = 1; index < history.size(); ++index) {
        const auto& previous = history[index - 1];
        const auto& current = history[index];
        if (time_s <= current.time_s) {
            const double segment_time = std::max(current.time_s - previous.time_s, 1e-6);
            const double t = std::clamp((time_s - previous.time_s) / segment_time, 0.0, 1.0);
            return {
                .position_m = previous.state.position_m * (1.0 - t) + current.state.position_m * t,
                .velocity_mps = previous.state.velocity_mps * (1.0 - t) + current.state.velocity_mps * t,
                .attitude_body_to_world =
                    lerpQuaternion(previous.state.attitude_body_to_world, current.state.attitude_body_to_world, t),
                .angular_velocity_body_radps =
                    previous.state.angular_velocity_body_radps * (1.0 - t) +
                    current.state.angular_velocity_body_radps * t,
                .mass_kg = previous.state.mass_kg * (1.0 - t) + current.state.mass_kg * t
            };
        }
    }

    return history.back().state;
}

void updateReplayTimeline(
    SimulationRuntime& runtime,
    double frame_time_s) noexcept {
    if (runtime.replay_active && !runtime.trajectory_history.empty()) {
        runtime.replay_time_s += frame_time_s * 0.9;
        const double end_time_s = runtime.trajectory_history.back().time_s;
        if (runtime.replay_time_s > end_time_s) {
            runtime.replay_time_s = 0.0;
        }
    }
}

void clearScrubPreview(SimulationRuntime& runtime) noexcept {
    runtime.scrub_preview_active = false;
    runtime.scrub_preview_time_s = 0.0;
}

void setScrubPreviewTime(
    SimulationRuntime& runtime,
    double time_s) noexcept {
    const double end_time_s =
        runtime.trajectory_history.empty() ? runtime.time_s : runtime.trajectory_history.back().time_s;
    runtime.scrub_preview_active = true;
    runtime.scrub_preview_time_s = std::clamp(time_s, 0.0, std::max(end_time_s, runtime.time_s));
    runtime.replay_active = false;
    runtime.replay_time_s = 0.0;
    runtime.paused = true;
}

double currentRenderTime(const SimulationRuntime& runtime) noexcept {
    if (runtime.scrub_preview_active) {
        return runtime.scrub_preview_time_s;
    }
    if (runtime.keyframe_preview_active) {
        return runtime.keyframe_preview_time_s;
    }
    return runtime.replay_active ? runtime.replay_time_s : runtime.time_s;
}

FlightState currentRenderState(const SimulationRuntime& runtime) noexcept {
    if (runtime.scrub_preview_active && !runtime.trajectory_history.empty()) {
        return sampleTrajectoryState(runtime.trajectory_history, runtime.scrub_preview_time_s);
    }
    if (runtime.keyframe_preview_active && !runtime.trajectory_history.empty()) {
        return sampleTrajectoryState(runtime.trajectory_history, runtime.keyframe_preview_time_s);
    }
    if (runtime.replay_active && !runtime.trajectory_history.empty()) {
        return sampleTrajectoryState(runtime.trajectory_history, runtime.replay_time_s);
    }
    return runtime.state;
}

void resetSimulationRuntime(
    const VehicleModel& vehicle,
    SimulationRuntime& runtime) noexcept {
    runtime.initial_state = buildRestState(vehicle);
    runtime.state = runtime.initial_state;
    runtime.time_s = 0.0;
    runtime.accumulator_s = 0.0;
    runtime.max_altitude_m = 0.0;
    runtime.paused = true;
    runtime.burnout_recorded = false;
    runtime.apogee_recorded = false;
    runtime.impact_recorded = false;
    runtime.replay_active = false;
    runtime.replay_time_s = 0.0;
    runtime.burnout_point_m = runtime.state.position_m;
    runtime.apogee_point_m = runtime.state.position_m;
    runtime.impact_point_m = runtime.state.position_m;
    runtime.burnout_time_s = 0.0;
    runtime.apogee_time_s = 0.0;
    runtime.impact_time_s = 0.0;
    runtime.keyframe_preview_active = false;
    runtime.keyframe_preview_index = -1;
    runtime.keyframe_preview_time_s = 0.0;
    runtime.scrub_preview_active = false;
    runtime.scrub_preview_time_s = 0.0;
    runtime.trajectory_history.clear();
    runtime.trajectory_history.push_back(TrajectorySample {
        .state = runtime.state,
        .time_s = 0.0
    });
}

std::expected<void, ValidationError> stepSimulationRuntime(
    SimulationRuntime& runtime,
    const VehicleModel& vehicle,
    const Environment& environment,
    Seconds dt,
    double frame_time_s) {
    runtime.accumulator_s += frame_time_s;
    while (runtime.accumulator_s >= valueOf(dt)) {
        const bool was_burning = vehicle.cluster.isBurning(runtime.time_s);
        const auto next_state = tryIntegrateRk4(
            runtime.state,
            vehicle,
            environment,
            runtime.time_s,
            dt);
        if (!next_state) {
            return std::unexpected(next_state.error());
        }

        runtime.state = *next_state;
        runtime.time_s += valueOf(dt);
        runtime.accumulator_s -= valueOf(dt);
        runtime.max_altitude_m = std::max(runtime.max_altitude_m, runtime.state.position_m.z);
        if (!runtime.apogee_recorded || runtime.state.position_m.z >= runtime.apogee_point_m.z) {
            runtime.apogee_recorded = true;
            runtime.apogee_point_m = runtime.state.position_m;
            runtime.apogee_time_s = runtime.time_s;
        }

        if (runtime.trajectory_history.empty() ||
            (runtime.state.position_m - runtime.trajectory_history.back().state.position_m).magnitude() > 0.2) {
            runtime.trajectory_history.push_back(TrajectorySample {
                .state = runtime.state,
                .time_s = runtime.time_s
            });
        }
        if (runtime.trajectory_history.size() > 2500) {
            runtime.trajectory_history.pop_front();
        }

        if (!runtime.burnout_recorded && was_burning && !vehicle.cluster.isBurning(runtime.time_s)) {
            runtime.burnout_recorded = true;
            runtime.burnout_point_m = runtime.state.position_m;
            runtime.burnout_time_s = runtime.time_s;
        }
        if (!runtime.impact_recorded &&
            runtime.time_s > 0.5 &&
            runtime.state.position_m.z <= 0.0 &&
            runtime.state.velocity_mps.z < 0.0) {
            runtime.impact_recorded = true;
            runtime.impact_point_m = runtime.state.position_m;
            runtime.impact_time_s = runtime.time_s;
            runtime.replay_active = true;
            runtime.replay_time_s = 0.0;
            clearScrubPreview(runtime);
        }
    }

    return {};
}

}  // namespace rocket
