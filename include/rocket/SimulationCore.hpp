#pragma once

#include "rocket/Environment.hpp"
#include "rocket/Forces.hpp"
#include "rocket/SimulationMonitor.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket {

[[nodiscard]] FlightState buildRestState(const VehicleModel& vehicle) noexcept;

[[nodiscard]] SimulationSnapshot buildSimulationSnapshot(
    const FlightState& state,
    const VehicleModel& vehicle,
    const ForceResult& force_result,
    const Environment& environment,
    double time_s,
    double max_altitude_m,
    int cfd_solver_particle_count = 0,
    int cfd_render_particle_count = 0);

[[nodiscard]] SimulationMonitorState buildSimulationMonitorState(
    const SimulationSnapshot& snapshot,
    bool paused,
    bool motor_burning,
    const VehicleModel& vehicle,
    const Environment& environment);

}  // namespace rocket
