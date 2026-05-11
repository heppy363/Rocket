#pragma once

#include <array>
#include <string_view>

#include "rocket/VehicleModel.hpp"

namespace rocket {

struct MaterialDefinition {
    std::string_view label;
    double density_kg_per_m3;
    double youngs_modulus_gpa;
    double yield_strength_mpa;
    double max_service_temperature_c;
    double relative_cost_index;
    std::string_view note;
};

struct StructureMassBreakdown {
    double nose_mass_kg {};
    double body_mass_kg {};
    double transition_mass_kg {};
    double fin_mass_kg {};
    double payload_bay_mass_kg {};
    double total_mass_kg {};
};

struct StructuralMaterialAssessment {
    double equivalent_density_kg_per_m3 {};
    double equivalent_modulus_gpa {};
    double recommended_max_dynamic_pressure_pa {};
};

[[nodiscard]] MaterialDefinition materialDefinition(ComponentMaterial material) noexcept;
[[nodiscard]] const std::array<ComponentMaterial, 7>& availableComponentMaterials() noexcept;
[[nodiscard]] std::string_view noseConeShapeLabel(NoseConeShape shape) noexcept;
[[nodiscard]] std::string_view finShapeLabel(FinShape shape) noexcept;
[[nodiscard]] std::string_view transitionShapeLabel(TransitionShape shape) noexcept;
[[nodiscard]] std::string_view rocketPresetLabel(RocketPreset preset) noexcept;

[[nodiscard]] VehicleGeometry makePresetGeometry(RocketPreset preset) noexcept;
[[nodiscard]] StructureMassBreakdown estimateStructureMassBreakdown(const VehicleGeometry& geometry) noexcept;
[[nodiscard]] double estimateStructureMassKg(const VehicleGeometry& geometry) noexcept;
[[nodiscard]] double estimateDryMassKg(const VehicleGeometry& geometry, double avionics_mass_kg) noexcept;
[[nodiscard]] double estimateComponentDynamicPressureLimitPa(ComponentType component, const VehicleGeometry& geometry) noexcept;
[[nodiscard]] double estimateRecommendedMaxDynamicPressurePa(const VehicleGeometry& geometry) noexcept;
[[nodiscard]] double estimateDynamicPressureSafetyFactor(
    const VehicleGeometry& geometry,
    double dynamic_pressure_pa) noexcept;
[[nodiscard]] StructuralMaterialAssessment estimateStructuralMaterialAssessment(const VehicleGeometry& geometry) noexcept;

}  // namespace rocket
