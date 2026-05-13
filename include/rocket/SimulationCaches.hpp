#pragma once

#include "rocket/DesignLibrary.hpp"

namespace rocket {

struct CachedVehicleGeometryAnalysis {
    StructureMassBreakdown mass_breakdown {};
    StructuralMaterialAssessment structural_assessment {};
    double recommended_max_dynamic_pressure_pa {};
};

struct CacheUsageStats {
    std::size_t l2_capacity {};
    std::size_t l2_valid_entries {};
    std::size_t l1_hits {};
    std::size_t l2_hits {};
    std::size_t misses {};
    std::size_t writes {};
};

struct SimulationCacheStats {
    CacheUsageStats geometry {};
};

[[nodiscard]] const CachedVehicleGeometryAnalysis& cachedVehicleGeometryAnalysis(
    const VehicleGeometry& geometry) noexcept;

[[nodiscard]] StructureMassBreakdown cachedStructureMassBreakdown(
    const VehicleGeometry& geometry) noexcept;

[[nodiscard]] StructuralMaterialAssessment cachedStructuralMaterialAssessment(
    const VehicleGeometry& geometry) noexcept;

[[nodiscard]] double cachedRecommendedMaxDynamicPressurePa(
    const VehicleGeometry& geometry) noexcept;

[[nodiscard]] SimulationCacheStats simulationCacheStats() noexcept;

}  // namespace rocket
