#include "rocket/RungeKutta4.hpp"

#include <algorithm>

#include "rocket/Forces.hpp"

namespace rocket {

FlightState integrateRk4(
    const FlightState& current_state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s,
    double dt_s) noexcept {
    FlightState next_state = integrateRk4Generic(
        current_state,
        [&](const FlightState& state, double sample_time_s) {
            return evaluateStateDerivative(state, vehicle, environment, sample_time_s);
        },
        time_s,
        dt_s);
    next_state.attitude_body_to_world = next_state.attitude_body_to_world.normalized();
    next_state.mass_kg = std::max(next_state.mass_kg, vehicle.dry_mass_kg);
    next_state.position_m.z = std::max(next_state.position_m.z, 0.0);
    return next_state;
}

std::expected<FlightState, ValidationError> tryIntegrateRk4(
    const FlightState& current_state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s,
    Seconds dt) {
    if (ValidationResult validation = validateSimulationInputs(current_state, vehicle, environment, dt); !validation) {
        return std::unexpected(validation.error());
    }
    return integrateRk4(current_state, vehicle, environment, time_s, valueOf(dt));
}

}  // namespace rocket
