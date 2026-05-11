#pragma once

#include "rocket/Environment.hpp"
#include "rocket/FlightState.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket {

[[nodiscard]] FlightState integrateRk4(
    const FlightState& current_state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s,
    double dt_s) noexcept;

}  // namespace rocket
