#include "rocket/RungeKutta4.hpp"

#include <algorithm>

#include "rocket/Forces.hpp"

namespace rocket {

namespace {

FlightState advanceState(
    const FlightState& state,
    const StateDerivative& derivative,
    double dt_s) noexcept {
    return {
        .position_m = state.position_m + derivative.velocity_mps * dt_s,
        .velocity_mps = state.velocity_mps + derivative.acceleration_mps2 * dt_s,
        .attitude_body_to_world = state.attitude_body_to_world + derivative.attitude_rate * dt_s,
        .angular_velocity_body_radps =
            state.angular_velocity_body_radps + derivative.angular_acceleration_body_radps2 * dt_s,
        .mass_kg = state.mass_kg + derivative.mass_flow_kgps * dt_s
    };
}

StateDerivative weightedAverage(
    const StateDerivative& k1,
    const StateDerivative& k2,
    const StateDerivative& k3,
    const StateDerivative& k4) noexcept {
    return {
        .velocity_mps =
            (k1.velocity_mps + 2.0 * k2.velocity_mps + 2.0 * k3.velocity_mps + k4.velocity_mps) / 6.0,
        .acceleration_mps2 =
            (k1.acceleration_mps2 + 2.0 * k2.acceleration_mps2 + 2.0 * k3.acceleration_mps2 + k4.acceleration_mps2) /
            6.0,
        .attitude_rate =
            (k1.attitude_rate + 2.0 * k2.attitude_rate + 2.0 * k3.attitude_rate + k4.attitude_rate) / 6.0,
        .angular_acceleration_body_radps2 =
            (k1.angular_acceleration_body_radps2 + 2.0 * k2.angular_acceleration_body_radps2 +
             2.0 * k3.angular_acceleration_body_radps2 + k4.angular_acceleration_body_radps2) /
            6.0,
        .mass_flow_kgps =
            (k1.mass_flow_kgps + 2.0 * k2.mass_flow_kgps + 2.0 * k3.mass_flow_kgps + k4.mass_flow_kgps) / 6.0
    };
}

}  // namespace

FlightState integrateRk4(
    const FlightState& current_state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s,
    double dt_s) noexcept {
    const StateDerivative k1 = evaluateStateDerivative(current_state, vehicle, environment, time_s);
    const StateDerivative k2 = evaluateStateDerivative(
        advanceState(current_state, k1, dt_s * 0.5), vehicle, environment, time_s + dt_s * 0.5);
    const StateDerivative k3 = evaluateStateDerivative(
        advanceState(current_state, k2, dt_s * 0.5), vehicle, environment, time_s + dt_s * 0.5);
    const StateDerivative k4 = evaluateStateDerivative(
        advanceState(current_state, k3, dt_s), vehicle, environment, time_s + dt_s);

    FlightState next_state = advanceState(current_state, weightedAverage(k1, k2, k3, k4), dt_s);
    next_state.attitude_body_to_world = next_state.attitude_body_to_world.normalized();
    next_state.mass_kg = std::max(next_state.mass_kg, vehicle.dry_mass_kg);
    next_state.position_m.z = std::max(next_state.position_m.z, 0.0);
    return next_state;
}

}  // namespace rocket
