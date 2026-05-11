#pragma once

#include "rocket/MathTypes.hpp"

namespace rocket {

struct FlightState {
    Vector3 position_m {};
    Vector3 velocity_mps {};
    Quaternion attitude_body_to_world {};
    Vector3 angular_velocity_body_radps {};
    double mass_kg {};
};

struct StateDerivative {
    Vector3 velocity_mps {};
    Vector3 acceleration_mps2 {};
    Quaternion attitude_rate {};
    Vector3 angular_acceleration_body_radps2 {};
    double mass_flow_kgps {};
};

}  // namespace rocket
