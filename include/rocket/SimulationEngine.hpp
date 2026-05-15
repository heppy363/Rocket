#pragma once

#include <deque>
#include <expected>

#include "rocket/Environment.hpp"
#include "rocket/SimulationCore.hpp"
#include "rocket/SimulationRuntime.hpp"
#include "rocket/Units.hpp"
#include "rocket/Validation.hpp"

namespace rocket {

[[nodiscard]] Quaternion lerpQuaternion(
    const Quaternion& a,
    const Quaternion& b,
    double t) noexcept;

[[nodiscard]] FlightState sampleTrajectoryState(
    const std::deque<TrajectorySample>& history,
    double time_s) noexcept;

void updateReplayTimeline(
    SimulationRuntime& runtime,
    double frame_time_s) noexcept;

void clearScrubPreview(
    SimulationRuntime& runtime) noexcept;

void setScrubPreviewTime(
    SimulationRuntime& runtime,
    double time_s) noexcept;

[[nodiscard]] double currentRenderTime(
    const SimulationRuntime& runtime) noexcept;

[[nodiscard]] FlightState currentRenderState(
    const SimulationRuntime& runtime) noexcept;

void resetSimulationRuntime(
    const VehicleModel& vehicle,
    SimulationRuntime& runtime) noexcept;

[[nodiscard]] std::expected<void, ValidationError> stepSimulationRuntime(
    SimulationRuntime& runtime,
    const VehicleModel& vehicle,
    const Environment& environment,
    Seconds dt,
    double frame_time_s);

}  // namespace rocket
