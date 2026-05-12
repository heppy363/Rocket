#include "rocket/SimulationCaches.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>

namespace rocket {

namespace {

constexpr std::size_t l2_cache_capacity = 16;

template <typename T>
void hashCombine(std::uint64_t& seed, const T& value) noexcept {
    const std::uint64_t bits = std::bit_cast<std::uint64_t>(value);
    seed ^= bits + 0x9e3779b97f4a7c15ULL + (seed << 6U) + (seed >> 2U);
}

template <typename Enum>
void hashEnum(std::uint64_t& seed, Enum value) noexcept {
    seed ^= static_cast<std::uint64_t>(value) + 0x9e3779b97f4a7c15ULL + (seed << 6U) + (seed >> 2U);
}

std::uint64_t geometryFingerprint(const VehicleGeometry& geometry) noexcept {
    std::uint64_t seed = 0xcbf29ce484222325ULL;

    hashCombine(seed, geometry.body_length_m);
    hashCombine(seed, geometry.body_diameter_m);
    hashCombine(seed, geometry.wall_thickness_m);
    hashCombine(seed, geometry.nose_length_m);
    hashEnum(seed, geometry.nose_cone_shape);
    hashEnum(seed, geometry.nose_material);
    hashCombine(seed, geometry.transition_length_m);
    hashCombine(seed, geometry.transition_aft_diameter_m);
    hashEnum(seed, geometry.transition_shape);
    hashEnum(seed, geometry.transition_material);
    hashEnum(seed, geometry.body_material);
    hashCombine(seed, geometry.fin_front_from_nose_m);
    hashCombine(seed, geometry.fin_root_chord_m);
    hashCombine(seed, geometry.fin_tip_chord_m);
    hashCombine(seed, geometry.fin_span_m);
    hashCombine(seed, geometry.fin_sweep_length_m);
    hashCombine(seed, geometry.fin_thickness_m);
    hashEnum(seed, geometry.fin_shape);
    hashEnum(seed, geometry.fin_material);
    hashCombine(seed, static_cast<double>(geometry.fin_count));
    hashCombine(seed, geometry.payload_length_m);
    hashCombine(seed, geometry.payload_mass_kg);
    hashEnum(seed, geometry.payload_material);
    hashCombine(seed, geometry.nose_controls.mid_radius_scale);
    hashCombine(seed, geometry.nose_controls.shoulder_radius_scale);
    hashCombine(seed, geometry.body_controls.fore_radius_scale);
    hashCombine(seed, geometry.body_controls.mid_radius_scale);
    hashCombine(seed, geometry.body_controls.aft_radius_scale);
    hashCombine(seed, geometry.transition_controls.mid_radius_scale);
    hashCombine(seed, geometry.fin_controls.tip_le_offset_m);
    hashCombine(seed, geometry.fin_controls.tip_te_offset_m);
    hashCombine(seed, geometry.fin_controls.span_scale);
    hashCombine(seed, geometry.fin_controls.thickness_scale);
    hashCombine(seed, geometry.structure_cg_from_nose_m);
    hashCombine(seed, geometry.propellant_cg_from_nose_m);

    return seed;
}

struct GeometryCacheEntry {
    std::uint64_t fingerprint {};
    CachedVehicleGeometryAnalysis analysis {};
    bool valid {false};
};

struct GeometryCache {
    GeometryCacheEntry l1 {};
    std::array<GeometryCacheEntry, l2_cache_capacity> l2 {};
    std::size_t next_l2_slot {};
};

thread_local GeometryCache geometry_cache;

const CachedVehicleGeometryAnalysis& computeAndStore(
    GeometryCache& cache,
    const VehicleGeometry& geometry,
    std::uint64_t fingerprint) noexcept {
    CachedVehicleGeometryAnalysis analysis {
        .mass_breakdown = estimateStructureMassBreakdown(geometry),
        .structural_assessment = estimateStructuralMaterialAssessment(geometry),
        .recommended_max_dynamic_pressure_pa = estimateRecommendedMaxDynamicPressurePa(geometry)
    };

    cache.l1 = GeometryCacheEntry {
        .fingerprint = fingerprint,
        .analysis = analysis,
        .valid = true
    };
    cache.l2[cache.next_l2_slot] = cache.l1;
    cache.next_l2_slot = (cache.next_l2_slot + 1U) % cache.l2.size();
    return cache.l1.analysis;
}

}  // namespace

const CachedVehicleGeometryAnalysis& cachedVehicleGeometryAnalysis(
    const VehicleGeometry& geometry) noexcept {
    const std::uint64_t fingerprint = geometryFingerprint(geometry);
    if (geometry_cache.l1.valid && geometry_cache.l1.fingerprint == fingerprint) {
        return geometry_cache.l1.analysis;
    }

    for (const auto& entry : geometry_cache.l2) {
        if (entry.valid && entry.fingerprint == fingerprint) {
            geometry_cache.l1 = entry;
            return geometry_cache.l1.analysis;
        }
    }

    return computeAndStore(geometry_cache, geometry, fingerprint);
}

StructureMassBreakdown cachedStructureMassBreakdown(
    const VehicleGeometry& geometry) noexcept {
    return cachedVehicleGeometryAnalysis(geometry).mass_breakdown;
}

StructuralMaterialAssessment cachedStructuralMaterialAssessment(
    const VehicleGeometry& geometry) noexcept {
    return cachedVehicleGeometryAnalysis(geometry).structural_assessment;
}

double cachedRecommendedMaxDynamicPressurePa(
    const VehicleGeometry& geometry) noexcept {
    return cachedVehicleGeometryAnalysis(geometry).recommended_max_dynamic_pressure_pa;
}

}  // namespace rocket
