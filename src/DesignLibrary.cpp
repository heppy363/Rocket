#include "rocket/DesignLibrary.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace rocket {

namespace {

constexpr double pi = 3.14159265358979323846;
constexpr std::array<ComponentMaterial, 7> kMaterials {
    ComponentMaterial::PlaCf,
    ComponentMaterial::Aluminum6061,
    ComponentMaterial::Pvc,
    ComponentMaterial::Fiberglass,
    ComponentMaterial::CarbonFiber,
    ComponentMaterial::BirchPlywood,
    ComponentMaterial::Phenolic
};

double shellCylinderVolumeM3(double outer_radius_m, double inner_radius_m, double length_m) noexcept {
    return pi * std::max(length_m, 0.0) *
           std::max(outer_radius_m * outer_radius_m - inner_radius_m * inner_radius_m, 0.0);
}

double shellFrustumVolumeM3(
    double outer_r0_m,
    double outer_r1_m,
    double inner_r0_m,
    double inner_r1_m,
    double length_m) noexcept {
    const auto frustum_volume = [length_m](double r0, double r1) noexcept {
        return (pi * std::max(length_m, 0.0) / 3.0) * (r0 * r0 + r0 * r1 + r1 * r1);
    };
    return std::max(frustum_volume(outer_r0_m, outer_r1_m) - frustum_volume(inner_r0_m, inner_r1_m), 0.0);
}

}  // namespace

MaterialDefinition materialDefinition(ComponentMaterial material) noexcept {
    switch (material) {
    case ComponentMaterial::PlaCf:
        return {"PLA-CF", 1240.0, 11.0, 68.0, 55.0, 1.2, "Stampa rapida, buona rigidita per ogive e pinne prototipali"};
    case ComponentMaterial::Aluminum6061:
        return {"Alluminio 6061", 2700.0, 68.0, 276.0, 150.0, 2.4, "Tubazioni e tail section robuste, penalizza la massa"};
    case ComponentMaterial::Pvc:
        return {"PVC", 1400.0, 3.0, 52.0, 60.0, 0.9, "Economico ma poco rigido, adatto a supporti e payload leggeri"};
    case ComponentMaterial::Fiberglass:
        return {"Fiberglass", 1850.0, 24.0, 210.0, 120.0, 1.8, "Buon compromesso fra costo, rigidita e resistenza termica"};
    case ComponentMaterial::CarbonFiber:
        return {"Carbon Fiber", 1600.0, 70.0, 600.0, 135.0, 3.6, "Molto rigido e leggero, ideale per fusoliere e pinne spinte"};
    case ComponentMaterial::BirchPlywood:
        return {"Betulla Aircraft", 680.0, 10.5, 95.0, 90.0, 1.0, "Classico materiale per pinne, leggero e facile da lavorare"};
    case ComponentMaterial::Phenolic:
        return {"Phenolic Tube", 1420.0, 16.0, 140.0, 180.0, 1.7, "Ottima stabilita termica per body tube e avionics bay"};
    }
    return {"Unknown", 1000.0, 8.0, 60.0, 70.0, 1.0, "Fallback"};
}

const std::array<ComponentMaterial, 7>& availableComponentMaterials() noexcept {
    return kMaterials;
}

std::string_view noseConeShapeLabel(NoseConeShape shape) noexcept {
    switch (shape) {
    case NoseConeShape::Conical:
        return "Conical";
    case NoseConeShape::TangentOgive:
        return "Tangent Ogive";
    case NoseConeShape::Parabolic:
        return "Parabolic";
    case NoseConeShape::LdHaack:
        return "LD-Haack";
    }
    return "Unknown";
}

std::string_view finShapeLabel(FinShape shape) noexcept {
    switch (shape) {
    case FinShape::Trapezoidal:
        return "Trapezoidal";
    case FinShape::Elliptical:
        return "Elliptical";
    case FinShape::Airfoil:
        return "Airfoil";
    }
    return "Unknown";
}

std::string_view transitionShapeLabel(TransitionShape shape) noexcept {
    switch (shape) {
    case TransitionShape::Conical:
        return "Conical";
    case TransitionShape::Curved:
        return "Curved";
    }
    return "Unknown";
}

std::string_view rocketPresetLabel(RocketPreset preset) noexcept {
    switch (preset) {
    case RocketPreset::ResearchStarter:
        return "Research Starter";
    case RocketPreset::SportTrainer:
        return "Sport Trainer";
    case RocketPreset::HighAltitude:
        return "High Altitude";
    case RocketPreset::MinimumDiameter:
        return "Minimum Diameter";
    case RocketPreset::HeavyLift:
        return "Heavy Lift";
    }
    return "Preset";
}

VehicleGeometry makePresetGeometry(RocketPreset preset) noexcept {
    switch (preset) {
    case RocketPreset::ResearchStarter:
        return {
            .body_length_m = 3.0,
            .body_diameter_m = 0.104,
            .wall_thickness_m = 0.003,
            .nose_length_m = 0.48,
            .nose_cone_shape = NoseConeShape::LdHaack,
            .nose_material = ComponentMaterial::PlaCf,
            .transition_length_m = 0.18,
            .transition_aft_diameter_m = 0.084,
            .transition_shape = TransitionShape::Conical,
            .transition_material = ComponentMaterial::Aluminum6061,
            .body_material = ComponentMaterial::Aluminum6061,
            .fin_front_from_nose_m = 2.15,
            .fin_root_chord_m = 0.30,
            .fin_tip_chord_m = 0.14,
            .fin_span_m = 0.17,
            .fin_sweep_length_m = 0.08,
            .fin_thickness_m = 0.006,
            .fin_shape = FinShape::Trapezoidal,
            .fin_material = ComponentMaterial::PlaCf,
            .fin_count = 4,
            .payload_length_m = 0.20,
            .payload_mass_kg = 0.9,
            .payload_material = ComponentMaterial::Pvc,
            .nose_controls = {.mid_radius_scale = 1.0, .shoulder_radius_scale = 1.0},
            .body_controls = {.fore_radius_scale = 1.0, .mid_radius_scale = 1.0, .aft_radius_scale = 1.0},
            .transition_controls = {.mid_radius_scale = 1.0},
            .fin_controls = {.tip_le_offset_m = 0.0, .tip_te_offset_m = 0.0, .span_scale = 1.0, .thickness_scale = 1.0},
            .structure_cg_from_nose_m = 1.45,
            .propellant_cg_from_nose_m = 2.40
        };
    case RocketPreset::SportTrainer:
        return {
            .body_length_m = 2.2,
            .body_diameter_m = 0.086,
            .wall_thickness_m = 0.0025,
            .nose_length_m = 0.34,
            .nose_cone_shape = NoseConeShape::TangentOgive,
            .nose_material = ComponentMaterial::PlaCf,
            .transition_length_m = 0.12,
            .transition_aft_diameter_m = 0.074,
            .transition_shape = TransitionShape::Conical,
            .transition_material = ComponentMaterial::Aluminum6061,
            .body_material = ComponentMaterial::Aluminum6061,
            .fin_front_from_nose_m = 1.56,
            .fin_root_chord_m = 0.22,
            .fin_tip_chord_m = 0.11,
            .fin_span_m = 0.12,
            .fin_sweep_length_m = 0.06,
            .fin_thickness_m = 0.005,
            .fin_shape = FinShape::Trapezoidal,
            .fin_material = ComponentMaterial::PlaCf,
            .fin_count = 3,
            .payload_length_m = 0.12,
            .payload_mass_kg = 0.35,
            .payload_material = ComponentMaterial::Pvc,
            .nose_controls = {.mid_radius_scale = 1.0, .shoulder_radius_scale = 1.0},
            .body_controls = {.fore_radius_scale = 1.0, .mid_radius_scale = 1.0, .aft_radius_scale = 1.0},
            .transition_controls = {.mid_radius_scale = 1.0},
            .fin_controls = {.tip_le_offset_m = 0.0, .tip_te_offset_m = 0.0, .span_scale = 1.0, .thickness_scale = 1.0},
            .structure_cg_from_nose_m = 1.06,
            .propellant_cg_from_nose_m = 1.72
        };
    case RocketPreset::HighAltitude:
        return {
            .body_length_m = 3.7,
            .body_diameter_m = 0.098,
            .wall_thickness_m = 0.0025,
            .nose_length_m = 0.62,
            .nose_cone_shape = NoseConeShape::LdHaack,
            .nose_material = ComponentMaterial::PlaCf,
            .transition_length_m = 0.22,
            .transition_aft_diameter_m = 0.072,
            .transition_shape = TransitionShape::Curved,
            .transition_material = ComponentMaterial::Aluminum6061,
            .body_material = ComponentMaterial::Aluminum6061,
            .fin_front_from_nose_m = 2.85,
            .fin_root_chord_m = 0.26,
            .fin_tip_chord_m = 0.10,
            .fin_span_m = 0.13,
            .fin_sweep_length_m = 0.11,
            .fin_thickness_m = 0.005,
            .fin_shape = FinShape::Airfoil,
            .fin_material = ComponentMaterial::PlaCf,
            .fin_count = 3,
            .payload_length_m = 0.26,
            .payload_mass_kg = 0.7,
            .payload_material = ComponentMaterial::Pvc,
            .nose_controls = {.mid_radius_scale = 0.94, .shoulder_radius_scale = 1.02},
            .body_controls = {.fore_radius_scale = 0.98, .mid_radius_scale = 1.0, .aft_radius_scale = 1.0},
            .transition_controls = {.mid_radius_scale = 0.88},
            .fin_controls = {.tip_le_offset_m = 0.03, .tip_te_offset_m = -0.01, .span_scale = 0.96, .thickness_scale = 0.92},
            .structure_cg_from_nose_m = 1.70,
            .propellant_cg_from_nose_m = 2.82
        };
    case RocketPreset::MinimumDiameter:
        return {
            .body_length_m = 3.5,
            .body_diameter_m = 0.075,
            .wall_thickness_m = 0.0022,
            .nose_length_m = 0.61,
            .nose_cone_shape = NoseConeShape::LdHaack,
            .nose_material = ComponentMaterial::PlaCf,
            .transition_length_m = 0.0,
            .transition_aft_diameter_m = 0.075,
            .transition_shape = TransitionShape::Conical,
            .transition_material = ComponentMaterial::Aluminum6061,
            .body_material = ComponentMaterial::Aluminum6061,
            .fin_front_from_nose_m = 2.78,
            .fin_root_chord_m = 0.20,
            .fin_tip_chord_m = 0.08,
            .fin_span_m = 0.11,
            .fin_sweep_length_m = 0.10,
            .fin_thickness_m = 0.0045,
            .fin_shape = FinShape::Airfoil,
            .fin_material = ComponentMaterial::PlaCf,
            .fin_count = 3,
            .payload_length_m = 0.18,
            .payload_mass_kg = 0.48,
            .payload_material = ComponentMaterial::Pvc,
            .nose_controls = {.mid_radius_scale = 0.92, .shoulder_radius_scale = 1.0},
            .body_controls = {.fore_radius_scale = 0.98, .mid_radius_scale = 1.0, .aft_radius_scale = 0.98},
            .transition_controls = {.mid_radius_scale = 1.0},
            .fin_controls = {.tip_le_offset_m = 0.03, .tip_te_offset_m = -0.01, .span_scale = 0.94, .thickness_scale = 0.86},
            .structure_cg_from_nose_m = 1.54,
            .propellant_cg_from_nose_m = 2.60
        };
    case RocketPreset::HeavyLift:
        return {
            .body_length_m = 3.4,
            .body_diameter_m = 0.125,
            .wall_thickness_m = 0.0035,
            .nose_length_m = 0.50,
            .nose_cone_shape = NoseConeShape::TangentOgive,
            .nose_material = ComponentMaterial::PlaCf,
            .transition_length_m = 0.20,
            .transition_aft_diameter_m = 0.094,
            .transition_shape = TransitionShape::Conical,
            .transition_material = ComponentMaterial::Aluminum6061,
            .body_material = ComponentMaterial::Aluminum6061,
            .fin_front_from_nose_m = 2.35,
            .fin_root_chord_m = 0.38,
            .fin_tip_chord_m = 0.18,
            .fin_span_m = 0.22,
            .fin_sweep_length_m = 0.07,
            .fin_thickness_m = 0.007,
            .fin_shape = FinShape::Elliptical,
            .fin_material = ComponentMaterial::PlaCf,
            .fin_count = 4,
            .payload_length_m = 0.24,
            .payload_mass_kg = 1.2,
            .payload_material = ComponentMaterial::Pvc,
            .nose_controls = {.mid_radius_scale = 1.05, .shoulder_radius_scale = 1.0},
            .body_controls = {.fore_radius_scale = 1.0, .mid_radius_scale = 1.03, .aft_radius_scale = 0.97},
            .transition_controls = {.mid_radius_scale = 1.04},
            .fin_controls = {.tip_le_offset_m = -0.01, .tip_te_offset_m = 0.02, .span_scale = 1.08, .thickness_scale = 1.08},
            .structure_cg_from_nose_m = 1.56,
            .propellant_cg_from_nose_m = 2.58
        };
    }

    return makePresetGeometry(RocketPreset::ResearchStarter);
}

StructureMassBreakdown estimateStructureMassBreakdown(const VehicleGeometry& geometry) noexcept {
    const double outer_radius_m = geometry.body_diameter_m * 0.5;
    const double inner_radius_m = std::max(outer_radius_m - geometry.wall_thickness_m, 0.0);
    const double cylindrical_length_m =
        std::max(geometry.body_length_m - geometry.nose_length_m - geometry.transition_length_m, 0.2);

    const double body_mass_kg =
        shellCylinderVolumeM3(outer_radius_m, inner_radius_m, cylindrical_length_m) *
        materialDefinition(geometry.body_material).density_kg_per_m3;

    const double nose_mass_kg =
        shellFrustumVolumeM3(outer_radius_m, 0.0, inner_radius_m, 0.0, geometry.nose_length_m) *
        materialDefinition(geometry.nose_material).density_kg_per_m3 * 0.82;

    const double transition_outer_aft_radius_m = std::max(geometry.transition_aft_diameter_m * 0.5, 0.0);
    const double transition_inner_aft_radius_m = std::max(transition_outer_aft_radius_m - geometry.wall_thickness_m, 0.0);
    const double transition_mass_kg =
        shellFrustumVolumeM3(
            outer_radius_m,
            transition_outer_aft_radius_m,
            inner_radius_m,
            transition_inner_aft_radius_m,
            geometry.transition_length_m) *
        materialDefinition(geometry.transition_material).density_kg_per_m3;

    const double fin_span_m = geometry.fin_span_m * geometry.fin_controls.span_scale;
    const double fin_root_chord_m = geometry.fin_root_chord_m;
    const double fin_tip_chord_m = std::max(geometry.fin_tip_chord_m + geometry.fin_controls.tip_te_offset_m, 0.04);
    const double fin_thickness_m = std::max(geometry.fin_thickness_m * geometry.fin_controls.thickness_scale, 0.002);
    const double fin_area_single_m2 = 0.5 * (fin_root_chord_m + fin_tip_chord_m) * fin_span_m;
    const double fin_mass_kg =
        fin_area_single_m2 * fin_thickness_m * static_cast<double>(geometry.fin_count) *
        materialDefinition(geometry.fin_material).density_kg_per_m3;

    const double payload_shell_radius_m = std::max(outer_radius_m * 0.88, 0.01);
    const double payload_shell_thickness_m = std::clamp(geometry.wall_thickness_m * 0.6, 0.001, 0.006);
    const double payload_shell_inner_radius_m = std::max(payload_shell_radius_m - payload_shell_thickness_m, 0.0);
    const double payload_bay_mass_kg =
        shellCylinderVolumeM3(
            payload_shell_radius_m,
            payload_shell_inner_radius_m,
            std::max(geometry.payload_length_m, 0.04)) *
        materialDefinition(geometry.payload_material).density_kg_per_m3 * 0.42;

    return {
        .nose_mass_kg = nose_mass_kg,
        .body_mass_kg = body_mass_kg,
        .transition_mass_kg = transition_mass_kg,
        .fin_mass_kg = fin_mass_kg,
        .payload_bay_mass_kg = payload_bay_mass_kg,
        .total_mass_kg = body_mass_kg + nose_mass_kg + transition_mass_kg + fin_mass_kg + payload_bay_mass_kg
    };
}

double estimateStructureMassKg(const VehicleGeometry& geometry) noexcept {
    return estimateStructureMassBreakdown(geometry).total_mass_kg;
}

double estimateDryMassKg(const VehicleGeometry& geometry, double avionics_mass_kg) noexcept {
    return estimateStructureMassKg(geometry) + geometry.payload_mass_kg + std::max(avionics_mass_kg, 0.0);
}

double estimateComponentDynamicPressureLimitPa(ComponentType component, const VehicleGeometry& geometry) noexcept {
    const auto body = materialDefinition(geometry.body_material);
    const auto nose = materialDefinition(geometry.nose_material);
    const auto transition = materialDefinition(geometry.transition_material);
    const auto fins = materialDefinition(geometry.fin_material);
    const auto payload = materialDefinition(geometry.payload_material);

    const double wall_ratio = geometry.wall_thickness_m / std::max(geometry.body_diameter_m, 0.02);
    const double fin_thickness_m = std::max(geometry.fin_thickness_m * geometry.fin_controls.thickness_scale, 0.0015);
    const double fin_span_m = std::max(geometry.fin_span_m * geometry.fin_controls.span_scale, 0.04);
    const double fin_slenderness = fin_thickness_m / fin_span_m;

    const double nose_limit =
        nose.yield_strength_mpa * 1.0e6 *
        (geometry.wall_thickness_m / std::max(geometry.nose_length_m, 0.08)) * 0.22;
    const double body_limit = body.yield_strength_mpa * 1.0e6 * wall_ratio * 0.0105;
    const double transition_limit = transition.yield_strength_mpa * 1.0e6 * wall_ratio * 0.0095;
    const double fin_limit =
        (fins.youngs_modulus_gpa * 1.0e9) * fin_slenderness * fin_slenderness * 0.00032;
    const double payload_limit =
        payload.yield_strength_mpa * 1.0e6 *
        (geometry.wall_thickness_m / std::max(geometry.payload_length_m, 0.06)) * 0.17;
    const double motor_limit = body.yield_strength_mpa * 1.0e6 * wall_ratio * 0.0125;

    const double raw_limit = [&] {
        switch (component) {
        case ComponentType::NoseCone:
            return nose_limit;
        case ComponentType::BodyTube:
            return body_limit;
        case ComponentType::Transition:
            return transition_limit;
        case ComponentType::FinSet:
            return fin_limit;
        case ComponentType::MotorMount:
            return motor_limit;
        case ComponentType::Payload:
            return payload_limit;
        }
        return body_limit;
    }();

    return std::clamp(raw_limit, 8000.0, 250000.0);
}

double estimateRecommendedMaxDynamicPressurePa(const VehicleGeometry& geometry) noexcept {
    return std::min(
        {
            estimateComponentDynamicPressureLimitPa(ComponentType::NoseCone, geometry),
            estimateComponentDynamicPressureLimitPa(ComponentType::BodyTube, geometry),
            estimateComponentDynamicPressureLimitPa(ComponentType::Transition, geometry),
            estimateComponentDynamicPressureLimitPa(ComponentType::FinSet, geometry),
            estimateComponentDynamicPressureLimitPa(ComponentType::Payload, geometry),
            estimateComponentDynamicPressureLimitPa(ComponentType::MotorMount, geometry)
        });
}

double estimateDynamicPressureSafetyFactor(
    const VehicleGeometry& geometry,
    double dynamic_pressure_pa) noexcept {
    return estimateRecommendedMaxDynamicPressurePa(geometry) / std::max(dynamic_pressure_pa, 1.0);
}

StructuralMaterialAssessment estimateStructuralMaterialAssessment(const VehicleGeometry& geometry) noexcept {
    const StructureMassBreakdown masses = estimateStructureMassBreakdown(geometry);
    const double total_mass = std::max(masses.total_mass_kg, 1e-6);
    const double weighted_density =
        materialDefinition(geometry.nose_material).density_kg_per_m3 * masses.nose_mass_kg +
        materialDefinition(geometry.body_material).density_kg_per_m3 * masses.body_mass_kg +
        materialDefinition(geometry.transition_material).density_kg_per_m3 * masses.transition_mass_kg +
        materialDefinition(geometry.fin_material).density_kg_per_m3 * masses.fin_mass_kg +
        materialDefinition(geometry.payload_material).density_kg_per_m3 * masses.payload_bay_mass_kg;
    const double weighted_modulus =
        materialDefinition(geometry.nose_material).youngs_modulus_gpa * masses.nose_mass_kg +
        materialDefinition(geometry.body_material).youngs_modulus_gpa * masses.body_mass_kg +
        materialDefinition(geometry.transition_material).youngs_modulus_gpa * masses.transition_mass_kg +
        materialDefinition(geometry.fin_material).youngs_modulus_gpa * masses.fin_mass_kg +
        materialDefinition(geometry.payload_material).youngs_modulus_gpa * masses.payload_bay_mass_kg;

    return {
        .equivalent_density_kg_per_m3 = weighted_density / total_mass,
        .equivalent_modulus_gpa = weighted_modulus / total_mass,
        .recommended_max_dynamic_pressure_pa = estimateRecommendedMaxDynamicPressurePa(geometry)
    };
}

}  // namespace rocket
