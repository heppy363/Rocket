#pragma once

#include "rocket/FlightState.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket {

struct AerodynamicFrame {
    Vector3 body_axis_world {};
    Vector3 relative_air_velocity_world_mps {};
    Vector3 lateral_air_direction_world {};
    double speed_mps {};
    double angle_of_attack_rad {};
};

[[nodiscard]] double computeCurrentPropellantMassKg(
    const FlightState& state,
    const VehicleModel& vehicle) noexcept;

[[nodiscard]] double computeCenterOfGravityFromNoseM(
    const FlightState& state,
    const VehicleModel& vehicle) noexcept;

[[nodiscard]] double computeCenterOfPressureFromNoseM(
    const VehicleModel& vehicle) noexcept;

[[nodiscard]] double computeStaticMarginCalibers(
    const FlightState& state,
    const VehicleModel& vehicle) noexcept;

[[nodiscard]] AerodynamicFrame computeAerodynamicFrame(
    const FlightState& state,
    const Vector3& relative_air_velocity_world_mps) noexcept;

[[nodiscard]] AerodynamicFrame computeAerodynamicFrame(
    const FlightState& state) noexcept;

}  // namespace rocket
