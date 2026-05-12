#pragma once

#include <deque>

#include "rocket/FlightState.hpp"

namespace rocket {

struct TrajectorySample {
    FlightState state {};
    double time_s {};
};

struct SimulationRuntime {
    FlightState initial_state {};
    FlightState state {};
    double time_s {};
    double accumulator_s {};
    double max_altitude_m {};
    bool paused {true};
    std::deque<TrajectorySample> trajectory_history {};
    bool burnout_recorded {false};
    bool apogee_recorded {false};
    bool impact_recorded {false};
    bool replay_active {false};
    double replay_time_s {};
    Vector3 burnout_point_m {};
    Vector3 apogee_point_m {};
    Vector3 impact_point_m {};
    double burnout_time_s {};
    double apogee_time_s {};
    double impact_time_s {};
    bool keyframe_preview_active {false};
    int keyframe_preview_index {-1};
    double keyframe_preview_time_s {};
};

}  // namespace rocket
