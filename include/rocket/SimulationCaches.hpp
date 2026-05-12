#pragma once

#include "rocket/DesignLibrary.hpp"

namespace rocket {

struct CachedVehicleGeometryAnalysis {
    StructureMassBreakdown mass_breakdown {};
    StructuralMaterialAssessment structural_assessment {};
    double recommended_max_dynamic_pressure_pa {};
};

[[nodiscard]] const CachedVehicleGeometryAnalysis& cachedVehicleGeometryAnalysis(
    const VehicleGeometry& geometry) noexcept;

[[nodiscard]] StructureMassBreakdown cachedStructureMassBreakdown(
    const VehicleGeometry& geometry) noexcept;

[[nodiscard]] StructuralMaterialAssessment cachedStructuralMaterialAssessment(
    const VehicleGeometry& geometry) noexcept;

[[nodiscard]] double cachedRecommendedMaxDynamicPressurePa(
    const VehicleGeometry& geometry) noexcept;

}  // namespace rocket
