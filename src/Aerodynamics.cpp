#include "rocket/Aerodynamics.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>

namespace rocket {

namespace {

constexpr double pi = 3.14159265358979323846;
constexpr std::size_t l2_cache_capacity = 16;

double clampUnit(double value) noexcept {
    return std::clamp(value, -1.0, 1.0);
}

template <typename T>
void hashCombine(std::uint64_t& seed, const T& value) noexcept {
    const std::uint64_t bits = std::bit_cast<std::uint64_t>(value);
    seed ^= bits + 0x9e3779b97f4a7c15ULL + (seed << 6U) + (seed >> 2U);
}

template <typename Enum>
void hashEnum(std::uint64_t& seed, Enum value) noexcept {
    seed ^= static_cast<std::uint64_t>(value) + 0x9e3779b97f4a7c15ULL + (seed << 6U) + (seed >> 2U);
}

std::uint64_t aerodynamicGeometryFingerprint(const VehicleGeometry& geometry) noexcept {
    std::uint64_t seed = 0xcbf29ce484222325ULL;
    hashCombine(seed, geometry.body_length_m);
    hashCombine(seed, geometry.body_diameter_m);
    hashCombine(seed, geometry.nose_length_m);
    hashCombine(seed, geometry.fin_front_from_nose_m);
    hashCombine(seed, geometry.fin_root_chord_m);
    hashCombine(seed, geometry.fin_tip_chord_m);
    hashCombine(seed, geometry.fin_span_m);
    hashCombine(seed, geometry.fin_sweep_length_m);
    hashCombine(seed, geometry.fin_thickness_m);
    hashCombine(seed, static_cast<double>(geometry.fin_count));
    hashCombine(seed, geometry.fin_controls.span_scale);
    hashCombine(seed, geometry.fin_controls.thickness_scale);
    hashEnum(seed, geometry.nose_cone_shape);
    hashEnum(seed, geometry.fin_shape);
    return seed;
}

struct CachedAerodynamicGeometry {
    double center_of_pressure_from_nose_m {};
};

struct AerodynamicCacheEntry {
    std::uint64_t fingerprint {};
    CachedAerodynamicGeometry analysis {};
    bool valid {false};
};

struct AerodynamicCache {
    AerodynamicCacheEntry l1 {};
    std::array<AerodynamicCacheEntry, l2_cache_capacity> l2 {};
    std::size_t next_l2_slot {};
    AerodynamicsCacheStats stats {
        .l2_capacity = l2_cache_capacity
    };
};

thread_local AerodynamicCache aerodynamic_cache;

CachedAerodynamicGeometry analyzeAerodynamicGeometry(const VehicleGeometry& geometry) noexcept {
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
    const double fin_cp_from_nose_m = geometry.fin_front_from_nose_m + fin_cp_relative_to_le_m;

    const double total_cn_alpha = nose_cn_alpha + fin_cn_alpha;
    return CachedAerodynamicGeometry {
        .center_of_pressure_from_nose_m =
            (total_cn_alpha <= 0.0 || fin_planform_area_m2 <= 0.0)
                ? nose_cp_from_nose_m
                : ((nose_cn_alpha * nose_cp_from_nose_m) + (fin_cn_alpha * fin_cp_from_nose_m)) / total_cn_alpha
    };
}

const CachedAerodynamicGeometry& cachedAerodynamicGeometry(const VehicleGeometry& geometry) noexcept {
    const std::uint64_t fingerprint = aerodynamicGeometryFingerprint(geometry);
    if (aerodynamic_cache.l1.valid && aerodynamic_cache.l1.fingerprint == fingerprint) {
        ++aerodynamic_cache.stats.l1_hits;
        return aerodynamic_cache.l1.analysis;
    }

    for (const auto& entry : aerodynamic_cache.l2) {
        if (entry.valid && entry.fingerprint == fingerprint) {
            aerodynamic_cache.l1 = entry;
            ++aerodynamic_cache.stats.l2_hits;
            return aerodynamic_cache.l1.analysis;
        }
    }

    aerodynamic_cache.l1 = AerodynamicCacheEntry {
        .fingerprint = fingerprint,
        .analysis = analyzeAerodynamicGeometry(geometry),
        .valid = true
    };
    aerodynamic_cache.l2[aerodynamic_cache.next_l2_slot] = aerodynamic_cache.l1;
    aerodynamic_cache.next_l2_slot = (aerodynamic_cache.next_l2_slot + 1U) % aerodynamic_cache.l2.size();
    ++aerodynamic_cache.stats.misses;
    ++aerodynamic_cache.stats.writes;
    return aerodynamic_cache.l1.analysis;
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
    return cachedAerodynamicGeometry(vehicle.geometry).center_of_pressure_from_nose_m;
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

AerodynamicsCacheStats aerodynamicsCacheStats() noexcept {
    const auto valid_entries = static_cast<std::size_t>(std::count_if(
        aerodynamic_cache.l2.begin(),
        aerodynamic_cache.l2.end(),
        [](const AerodynamicCacheEntry& entry) {
            return entry.valid;
        }));

    AerodynamicsCacheStats stats = aerodynamic_cache.stats;
    stats.l2_valid_entries = valid_entries;
    return stats;
}

}  // namespace rocket
