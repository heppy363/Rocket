#pragma once

#include "rocket/Environment.hpp"
#include "rocket/FlightState.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket {

struct ForceResult {
    Vector3 total_force_n {};
    Vector3 thrust_force_world_n {};
    Vector3 drag_force_n {};
    Vector3 recovery_drag_force_n {};
    Vector3 aerodynamic_normal_force_world_n {};
    Vector3 cfd_force_world_n {};
    Vector3 gravity_force_n {};
    Vector3 relative_air_velocity_world_mps {};
    Vector3 wind_velocity_world_mps {};
    Vector3 thrust_moment_body_nm {};
    Vector3 aerodynamic_moment_body_nm {};
    Vector3 cfd_moment_body_nm {};
    double center_of_gravity_from_nose_m {};
    double center_of_pressure_from_nose_m {};
    double static_margin_calibers {};
    double angle_of_attack_rad {};
    double dynamic_pressure_pa {};
    double mach_number {};
    double shockwave_intensity {};
    double aeroelastic_response {};
    bool parachute_deployed {};
    bool on_launch_rail {};
};

[[nodiscard]] ForceResult computeForces(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s) noexcept;

[[nodiscard]] StateDerivative evaluateStateDerivative(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s) noexcept;

}  // namespace rocket
