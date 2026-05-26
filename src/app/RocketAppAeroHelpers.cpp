#include "RocketAppAeroHelpers.hpp"

#include <algorithm>
#include <cmath>

namespace rocket::app {

namespace {

constexpr float kPi = 3.14159265358979323846f;

}  // namespace

Color blendColor(Color a, Color b, float t) {
    const float clamped = std::clamp(t, 0.0f, 1.0f);
    const auto lerp_channel = [clamped](unsigned char lhs, unsigned char rhs) {
        return static_cast<unsigned char>(
            static_cast<float>(lhs) + (static_cast<float>(rhs) - static_cast<float>(lhs)) * clamped);
    };
    return {
        lerp_channel(a.r, b.r),
        lerp_channel(a.g, b.g),
        lerp_channel(a.b, b.b),
        lerp_channel(a.a, b.a)};
}

double airDynamicViscosityPaS(double temperature_k) {
    const double safe_temperature_k = std::max(temperature_k, 180.0);
    return 1.458e-6 * std::pow(safe_temperature_k, 1.5) / (safe_temperature_k + 110.4);
}

double reynoldsNumber(const rocket::SimulationSnapshot& snapshot, double reference_length_m) {
    const double mu = airDynamicViscosityPaS(snapshot.air_temperature_k);
    return (snapshot.air_density_kgpm3 * std::max(snapshot.relative_air_speed_mps, 0.0) *
            std::max(reference_length_m, 0.001)) /
           std::max(mu, 1e-7);
}

float pressureRatio01(double pressure_pa, double reference_pa) {
    return std::clamp(static_cast<float>(pressure_pa / std::max(reference_pa, 1.0)), 0.0f, 1.0f);
}

bool pointInsideRect(const ::Vector2& point, const ::Rectangle& rect, float margin) {
    return point.x >= rect.x - margin && point.x <= rect.x + rect.width + margin &&
           point.y >= rect.y - margin && point.y <= rect.y + rect.height + margin;
}

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
    float phase_t) {
    const float y_seed = tunnel.y + 20.0f + lane_t * (tunnel.height - 40.0f);
    const float relative_to_body = (y_seed - center_y) / std::max(body_radius * 1.8f, 1.0f);
    const float body_influence = std::exp(-relative_to_body * relative_to_body);
    const float x = tunnel.x + 12.0f + progress_t * (tunnel.width - 24.0f);
    float y = y_seed + aoa_shift * (0.12f + 0.72f * lane_t) * progress_t;
    if (x >= nose_tip_x - 24.0f && x <= tail_end_x + 24.0f) {
        const float body_center_x = nose_tip_x + total_length * 0.45f;
        const float x_norm = (x - body_center_x) / std::max(total_length * 0.42f, 1.0f);
        y += body_influence * std::exp(-(x_norm * x_norm) * (3.8f + compressibility * 1.2f)) *
             (relative_to_body * 22.0f + aoa_shift * 0.15f);
        y += std::sin((progress_t * 9.0f + lane_t * 1.8f + phase_t) * kPi) * turbulence *
             (2.2f + compressibility * 1.2f);
        if (progress_t > 0.1f && x > nose_base_x && x < body_end_x && compressibility > 0.66f) {
            y += std::sin((progress_t * 15.0f + lane_t * 2.0f + phase_t * 0.6f) * kPi) *
                 (1.4f + pressure_bias * 1.8f);
        }
    }
    return ::Vector2 {x, y};
}

std::string flowRegimeLabel(const rocket::SimulationSnapshot& snapshot) {
    if (snapshot.parachute_deployed) {
        return "Recovery flow";
    }
    if (snapshot.mach_number < 0.8) {
        return "Subsonic";
    }
    if (snapshot.mach_number < 1.2) {
        return "Transonic";
    }
    return "Supersonic";
}

}  // namespace rocket::app
