#pragma once

#include <expected>

#include "rocket/Concepts.hpp"
#include "rocket/Environment.hpp"
#include "rocket/FlightState.hpp"
#include "rocket/Validation.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket {

template <StateDerivativeEvaluator Evaluator>
[[nodiscard]] inline FlightState integrateRk4Generic(
    const FlightState& current_state,
    Evaluator&& evaluator,
    double time_s,
    double dt_s) noexcept {
    const auto advance_state = [](const FlightState& state, const StateDerivative& derivative, double dt_seconds) {
        return FlightState {
            .position_m = state.position_m + derivative.velocity_mps * dt_seconds,
            .velocity_mps = state.velocity_mps + derivative.acceleration_mps2 * dt_seconds,
            .attitude_body_to_world = state.attitude_body_to_world + derivative.attitude_rate * dt_seconds,
            .angular_velocity_body_radps =
                state.angular_velocity_body_radps + derivative.angular_acceleration_body_radps2 * dt_seconds,
            .mass_kg = state.mass_kg + derivative.mass_flow_kgps * dt_seconds
        };
    };

    const auto weighted_average = [](
                                      const StateDerivative& k1,
                                      const StateDerivative& k2,
                                      const StateDerivative& k3,
                                      const StateDerivative& k4) {
        return StateDerivative {
            .velocity_mps =
                (k1.velocity_mps + 2.0 * k2.velocity_mps + 2.0 * k3.velocity_mps + k4.velocity_mps) / 6.0,
            .acceleration_mps2 =
                (k1.acceleration_mps2 + 2.0 * k2.acceleration_mps2 +
                 2.0 * k3.acceleration_mps2 + k4.acceleration_mps2) / 6.0,
            .attitude_rate =
                (k1.attitude_rate + 2.0 * k2.attitude_rate + 2.0 * k3.attitude_rate + k4.attitude_rate) / 6.0,
            .angular_acceleration_body_radps2 =
                (k1.angular_acceleration_body_radps2 + 2.0 * k2.angular_acceleration_body_radps2 +
                 2.0 * k3.angular_acceleration_body_radps2 + k4.angular_acceleration_body_radps2) / 6.0,
            .mass_flow_kgps =
                (k1.mass_flow_kgps + 2.0 * k2.mass_flow_kgps + 2.0 * k3.mass_flow_kgps + k4.mass_flow_kgps) / 6.0
        };
    };

    const StateDerivative k1 = evaluator(current_state, time_s);
    const StateDerivative k2 = evaluator(
        advance_state(current_state, k1, dt_s * 0.5),
        time_s + dt_s * 0.5);
    const StateDerivative k3 = evaluator(
        advance_state(current_state, k2, dt_s * 0.5),
        time_s + dt_s * 0.5);
    const StateDerivative k4 = evaluator(
        advance_state(current_state, k3, dt_s),
        time_s + dt_s);

    return advance_state(current_state, weighted_average(k1, k2, k3, k4), dt_s);
}

[[nodiscard]] FlightState integrateRk4(
    const FlightState& current_state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s,
    double dt_s) noexcept;

[[nodiscard]] std::expected<FlightState, ValidationError> tryIntegrateRk4(
    const FlightState& current_state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s,
    Seconds dt);

}  // namespace rocket
