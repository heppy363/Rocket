#include "rocket/Aerodynamics.hpp"

#include <algorithm>
#include <cmath>

namespace rocket {

namespace {

constexpr double pi = 3.14159265358979323846;

double clampUnit(double value) noexcept {
    return std::clamp(value, -1.0, 1.0);
}

}  // namespace

double computeCurrentPropellantMassKg(
    const FlightState& state,
    const VehicleModel& vehicle) noexcept {
    return std::max(0.0, state.mass_kg - vehicle.dry_mass_kg);
}

double computeCenterOfGravityFromNoseM(
    const FlightState& state,
    const VehicleModel& vehicle) noexcept {
    const double propellant_mass_kg = computeCurrentPropellantMassKg(state, vehicle);
    const double total_mass_kg = std::max(state.mass_kg, 1e-9);

    return ((vehicle.dry_mass_kg * vehicle.geometry.structure_cg_from_nose_m) +
            (propellant_mass_kg * vehicle.geometry.propellant_cg_from_nose_m)) /
           total_mass_kg;
}

double computeCenterOfPressureFromNoseM(const VehicleModel& vehicle) noexcept {
    const auto& geometry = vehicle.geometry;
    const double radius_m = geometry.body_diameter_m * 0.5;
    const double nose_cn_alpha = 2.0;
    const double nose_cp_from_nose_m = 0.6666666666666666 * geometry.nose_length_m;

    const double fin_mid_chord_m = 0.5 * (geometry.fin_root_chord_m + geometry.fin_tip_chord_m);
    const double fin_planform_area_m2 =
        0.5 * (geometry.fin_root_chord_m + geometry.fin_tip_chord_m) * geometry.fin_span_m;
    const double fin_semispan_effective_m = geometry.fin_span_m + radius_m;
    const double fin_cn_alpha =
        ((4.0 * static_cast<double>(geometry.fin_count)) *
         std::pow(fin_semispan_effective_m / geometry.body_diameter_m, 2.0)) /
        (1.0 + std::sqrt(1.0 + std::pow((2.0 * fin_mid_chord_m) / (geometry.fin_root_chord_m + geometry.fin_tip_chord_m), 2.0)));

    const double sweep_le_m =
        geometry.fin_sweep_length_m + 0.5 * (geometry.fin_tip_chord_m - geometry.fin_root_chord_m);
    const double fin_cp_relative_to_le_m =
        (sweep_le_m / 3.0) *
            ((geometry.fin_root_chord_m + 2.0 * geometry.fin_tip_chord_m) /
             (geometry.fin_root_chord_m + geometry.fin_tip_chord_m)) +
        (1.0 / 6.0) *
            ((std::pow(geometry.fin_root_chord_m, 2.0) + geometry.fin_root_chord_m * geometry.fin_tip_chord_m +
              std::pow(geometry.fin_tip_chord_m, 2.0)) /
             (geometry.fin_root_chord_m + geometry.fin_tip_chord_m));
    const double fin_le_from_nose_m = geometry.fin_front_from_nose_m;
    const double fin_cp_from_nose_m = fin_le_from_nose_m + fin_cp_relative_to_le_m;

    const double total_cn_alpha = nose_cn_alpha + fin_cn_alpha;
    if (total_cn_alpha <= 0.0 || fin_planform_area_m2 <= 0.0) {
        return nose_cp_from_nose_m;
    }

    return ((nose_cn_alpha * nose_cp_from_nose_m) + (fin_cn_alpha * fin_cp_from_nose_m)) / total_cn_alpha;
}

double computeStaticMarginCalibers(
    const FlightState& state,
    const VehicleModel& vehicle) noexcept {
    const double cp_m = computeCenterOfPressureFromNoseM(vehicle);
    const double cg_m = computeCenterOfGravityFromNoseM(state, vehicle);
    return (cp_m - cg_m) / std::max(vehicle.geometry.body_diameter_m, 1e-9);
}

AerodynamicFrame computeAerodynamicFrame(
    const FlightState& state,
    const Vector3& relative_air_velocity_world_mps) noexcept {
    AerodynamicFrame frame;
    frame.body_axis_world = rotateVector(state.attitude_body_to_world, {0.0, 0.0, 1.0}).normalized();
    frame.relative_air_velocity_world_mps = relative_air_velocity_world_mps;
    frame.speed_mps = frame.relative_air_velocity_world_mps.magnitude();

    if (frame.speed_mps <= 1e-6) {
        frame.angle_of_attack_rad = 0.0;
        return frame;
    }

    const Vector3 airflow_direction_world = frame.relative_air_velocity_world_mps / frame.speed_mps;
    const double axial_alignment = clampUnit(dot(frame.body_axis_world, airflow_direction_world));
    frame.angle_of_attack_rad = std::acos(axial_alignment);

    const Vector3 axial_component = dot(airflow_direction_world, frame.body_axis_world) * frame.body_axis_world;
    const Vector3 lateral_component = airflow_direction_world - axial_component;
    frame.lateral_air_direction_world = lateral_component.normalized();
    return frame;
}

AerodynamicFrame computeAerodynamicFrame(const FlightState& state) noexcept {
    return computeAerodynamicFrame(state, state.velocity_mps * -1.0);
}

}  // namespace rocket
