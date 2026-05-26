#pragma once

#include <string>

#include "raylib.h"
#include "rocket/SimulationCore.hpp"

namespace rocket::app {

Color blendColor(Color a, Color b, float t);
double airDynamicViscosityPaS(double temperature_k);
double reynoldsNumber(const rocket::SimulationSnapshot& snapshot, double reference_length_m);
float pressureRatio01(double pressure_pa, double reference_pa);
bool pointInsideRect(const ::Vector2& point, const ::Rectangle& rect, float margin = 0.0f);
::Vector2 sampleWindTunnelFlowPoint(
    const ::Rectangle& tunnel,
    float center_y,
    float nose_tip_x,
    float nose_base_x,
    float body_end_x,
    float tail_end_x,
    float total_length,
    float body_radius,
    float aoa_shift,
    float turbulence,
    float compressibility,
    float pressure_bias,
    float lane_t,
    float progress_t,
    float phase_t);
std::string flowRegimeLabel(const rocket::SimulationSnapshot& snapshot);

}  // namespace rocket::app
