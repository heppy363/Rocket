#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <deque>
#include <format>
#include <memory>
#include <string>
#include <string_view>

#include <slint.h>

#include "rocket/DesignLibrary.hpp"
#include "rocket/Forces.hpp"
#include "rocket/PhysicalConstants.hpp"
#include "rocket/RocketApp.hpp"
#include "rocket/RungeKutta4.hpp"
#include "rocket/SimulationMonitor.hpp"
#include "rocket/VehicleModel.hpp"
#include "rocket_lab.h"

namespace {

constexpr double kPi = 3.14159265358979323846;

struct TrajectorySample {
    rocket::FlightState state {};
    double time_s {};
};

struct RuntimeState {
    rocket::FlightState initial_state {};
    rocket::FlightState state {};
    double time_s {};
    double accumulator_s {};
    double max_altitude_m {};
    bool paused {true};
    bool replay_active {false};
    double replay_time_s {};
    bool burnout_recorded {false};
    bool apogee_recorded {false};
    bool impact_recorded {false};
    rocket::Vector3 burnout_point_m {};
    rocket::Vector3 apogee_point_m {};
    rocket::Vector3 impact_point_m {};
    double burnout_time_s {};
    double apogee_time_s {};
    double impact_time_s {};
    bool keyframe_preview_active {false};
    int keyframe_preview_index {-1};
    double keyframe_preview_time_s {};
    std::deque<TrajectorySample> trajectory_history {};
};

struct MissionKeyframe {
    std::string label;
    double time_s {};
};

std::string toStdString(const slint::SharedString& text) {
    return std::string(std::string_view(text));
}

slint::SharedString toSharedString(std::string_view text) {
    return slint::SharedString(text);
}

rocket::FlightState buildRestState(const rocket::VehicleModel& vehicle) {
    return {
        .position_m = {0.0, 0.0, 0.0},
        .velocity_mps = {0.0, 0.0, 0.0},
        .attitude_body_to_world = {},
        .angular_velocity_body_radps = {0.0, 0.0, 0.0},
        .mass_kg = vehicle.dry_mass_kg + vehicle.cluster.totalPropellantMassKg()
    };
}

double deriveBaseDragCoefficient(const rocket::VehicleGeometry& geometry) {
    const auto nose_material = rocket::materialDefinition(geometry.nose_material);
    const auto body_material = rocket::materialDefinition(geometry.body_material);
    const auto transition_material = rocket::materialDefinition(geometry.transition_material);
    const auto fin_material = rocket::materialDefinition(geometry.fin_material);
    const auto surface_drag_bias = [](const rocket::MaterialDefinition& material) {
        if (material.label == "Carbon Fiber") {
            return -0.022;
        }
        if (material.label == "Fiberglass") {
            return -0.014;
        }
        if (material.label == "Phenolic Tube") {
            return -0.010;
        }
        if (material.label == "Alluminio 6061") {
            return -0.006;
        }
        if (material.label == "PLA-CF") {
            return 0.010;
        }
        if (material.label == "Betulla Aircraft") {
            return 0.014;
        }
        return 0.018;
    };
    double drag = 0.46;

    switch (geometry.nose_cone_shape) {
    case rocket::NoseConeShape::Conical:
        drag += 0.08;
        break;
    case rocket::NoseConeShape::TangentOgive:
        drag += 0.03;
        break;
    case rocket::NoseConeShape::Parabolic:
        drag += 0.05;
        break;
    case rocket::NoseConeShape::LdHaack:
        break;
    }

    switch (geometry.fin_shape) {
    case rocket::FinShape::Trapezoidal:
        drag += 0.03;
        break;
    case rocket::FinShape::Elliptical:
        drag += 0.02;
        break;
    case rocket::FinShape::Airfoil:
        break;
    }

    if (geometry.transition_length_m > 1e-6) {
        drag += geometry.transition_shape == rocket::TransitionShape::Curved ? -0.03 : -0.01;
    }

    const double fineness_ratio = geometry.body_length_m / std::max(geometry.body_diameter_m, 1e-6);
    if (fineness_ratio > 18.0) {
        drag -= 0.02;
    } else if (fineness_ratio < 10.0) {
        drag += 0.03;
    }

    const double thickness_ratio =
        geometry.fin_thickness_m * geometry.fin_controls.thickness_scale / std::max(geometry.body_diameter_m, 1e-6);
    drag += std::clamp((thickness_ratio - 0.045) * 0.9, -0.015, 0.05);
    drag += 0.22 * surface_drag_bias(nose_material);
    drag += 0.36 * surface_drag_bias(body_material);
    drag += 0.14 * surface_drag_bias(transition_material);
    drag += 0.28 * surface_drag_bias(fin_material);

    return std::clamp(drag, 0.34, 0.82);
}

double deriveNormalForceSlopePerRad(const rocket::VehicleGeometry& geometry) {
    const auto fin_material = rocket::materialDefinition(geometry.fin_material);
    const auto body_material = rocket::materialDefinition(geometry.body_material);
    const double radius_m = geometry.body_diameter_m * 0.5;
    const double tip_chord_m = std::max(geometry.fin_tip_chord_m + geometry.fin_controls.tip_te_offset_m, 0.04);
    const double mean_chord_m = 0.5 * (geometry.fin_root_chord_m + tip_chord_m);
    const double effective_span_m = geometry.fin_span_m * geometry.fin_controls.span_scale + radius_m;
    const double aspect_ratio =
        (2.0 * effective_span_m * effective_span_m) /
        std::max((geometry.fin_root_chord_m + tip_chord_m) * effective_span_m, 1e-6);
    const double fin_term =
        (2.1 + 0.24 * static_cast<double>(geometry.fin_count)) *
        std::clamp(aspect_ratio / 2.3, 0.7, 1.8);

    double nose_term = 1.4;
    if (geometry.nose_cone_shape == rocket::NoseConeShape::TangentOgive) {
        nose_term = 1.55;
    } else if (geometry.nose_cone_shape == rocket::NoseConeShape::LdHaack) {
        nose_term = 1.48;
    }

    const double tail_bonus = geometry.transition_length_m > 1e-6 ? 0.12 : 0.0;
    const double chord_bias = std::clamp(mean_chord_m / std::max(geometry.body_diameter_m, 1e-6), 0.7, 3.0);
    const double fin_stiffness_factor = std::clamp(fin_material.youngs_modulus_gpa / 18.0, 0.78, 1.18);
    const double body_support_factor = std::clamp(body_material.youngs_modulus_gpa / 28.0, 0.88, 1.08);
    return std::clamp(
        nose_term + fin_term * 0.46 * fin_stiffness_factor + tail_bonus + 0.12 * chord_bias + 0.08 * (body_support_factor - 1.0),
        2.6,
        8.8);
}

double deriveRotationalDampingCoefficient(const rocket::VehicleGeometry& geometry, double dry_mass_kg) {
    const auto fin_material = rocket::materialDefinition(geometry.fin_material);
    const auto body_material = rocket::materialDefinition(geometry.body_material);
    const double reference_area_m2 = kPi * std::pow(geometry.body_diameter_m * 0.5, 2.0);
    const double fin_area_m2 =
        0.5 * (geometry.fin_root_chord_m + geometry.fin_tip_chord_m) *
        (geometry.fin_span_m * geometry.fin_controls.span_scale) *
        static_cast<double>(geometry.fin_count);
    const double lever_arm_m =
        std::max(geometry.fin_front_from_nose_m - geometry.structure_cg_from_nose_m, 0.12);
    const double fin_damping_factor = std::clamp(std::sqrt(fin_material.youngs_modulus_gpa / 11.0), 0.82, 1.28);
    const double shell_damping_factor = std::clamp(body_material.density_kg_per_m3 / 1600.0, 0.82, 1.26);
    const double damping =
        0.42 + 2.8 * fin_area_m2 / std::max(reference_area_m2, 1e-6) * fin_damping_factor +
        0.18 * lever_arm_m + 0.025 * dry_mass_kg + 0.12 * (shell_damping_factor - 1.0);
    return std::clamp(damping, 0.7, 4.5);
}

rocket::Vector3 estimatePrincipalInertiaKgM2(const rocket::VehicleGeometry& geometry, double dry_mass_kg) {
    const auto breakdown = rocket::estimateStructureMassBreakdown(geometry);
    const double avionics_mass_kg = std::max(dry_mass_kg - breakdown.total_mass_kg - geometry.payload_mass_kg, 0.0);
    const double body_radius_m = geometry.body_diameter_m * 0.5;
    const double aft_radius_m = std::max(geometry.transition_aft_diameter_m * 0.5, 0.01);
    const double cylinder_length_m =
        std::max(geometry.body_length_m - geometry.nose_length_m - geometry.transition_length_m, 0.2);
    struct InertiaPart {
        double mass_kg;
        double station_from_nose_m;
        double axial_radius_m;
        double local_length_m;
    };
    const std::array<InertiaPart, 7> parts {{
        {breakdown.nose_mass_kg, geometry.nose_length_m * 0.45, body_radius_m * 0.72, geometry.nose_length_m},
        {breakdown.payload_bay_mass_kg, geometry.nose_length_m + geometry.payload_length_m * 0.5, body_radius_m * 0.85, geometry.payload_length_m},
        {geometry.payload_mass_kg, geometry.nose_length_m + geometry.payload_length_m * 0.5, body_radius_m * 0.55, geometry.payload_length_m * 0.8},
        {breakdown.body_mass_kg, geometry.nose_length_m + cylinder_length_m * 0.5, body_radius_m, cylinder_length_m},
        {breakdown.transition_mass_kg, geometry.body_length_m - geometry.transition_length_m * 0.5, 0.5 * (body_radius_m + aft_radius_m), geometry.transition_length_m},
        {breakdown.fin_mass_kg, geometry.fin_front_from_nose_m + geometry.fin_root_chord_m * 0.45, body_radius_m + geometry.fin_span_m * geometry.fin_controls.span_scale * 0.7, geometry.fin_root_chord_m},
        {avionics_mass_kg, geometry.nose_length_m + geometry.payload_length_m + cylinder_length_m * 0.18, body_radius_m * 0.45, std::max(geometry.payload_length_m, 0.10)}
    }};
    double cg_from_nose_m = 0.0;
    for (const auto& part : parts) {
        cg_from_nose_m += part.mass_kg * part.station_from_nose_m;
    }
    cg_from_nose_m /= std::max(dry_mass_kg, 1e-6);
    double i_transverse = 0.0;
    double i_axial = 0.0;
    for (const auto& part : parts) {
        const double local_transverse =
            (part.mass_kg / 12.0) * (3.0 * part.axial_radius_m * part.axial_radius_m + part.local_length_m * part.local_length_m);
        const double local_axial = 0.5 * part.mass_kg * part.axial_radius_m * part.axial_radius_m;
        const double lever_m = part.station_from_nose_m - cg_from_nose_m;
        i_transverse += local_transverse + part.mass_kg * lever_m * lever_m;
        i_axial += local_axial;
    }
    return {
        std::max(i_transverse, 0.02),
        std::max(i_transverse, 0.02),
        std::max(i_axial, 0.002)
    };
}

void syncVehicleDerivedValues(rocket::VehicleModel& vehicle) {
    auto& geometry = vehicle.geometry;

    geometry.body_length_m = std::clamp(geometry.body_length_m, 0.8, 8.0);
    geometry.body_diameter_m = std::clamp(geometry.body_diameter_m, 0.04, 0.35);
    geometry.wall_thickness_m = std::clamp(geometry.wall_thickness_m, 0.0015, 0.01);
    geometry.nose_length_m = std::clamp(geometry.nose_length_m, 0.12, geometry.body_length_m * 0.45);
    geometry.transition_length_m = std::clamp(geometry.transition_length_m, 0.0, geometry.body_length_m * 0.25);
    geometry.transition_aft_diameter_m = std::clamp(geometry.transition_aft_diameter_m, 0.03, geometry.body_diameter_m);
    geometry.fin_root_chord_m = std::clamp(geometry.fin_root_chord_m, 0.08, 1.5);
    geometry.fin_tip_chord_m = std::clamp(geometry.fin_tip_chord_m, 0.04, 1.2);
    geometry.fin_span_m = std::clamp(geometry.fin_span_m, 0.04, 0.8);
    geometry.fin_sweep_length_m = std::clamp(geometry.fin_sweep_length_m, 0.0, geometry.fin_root_chord_m * 0.95);
    geometry.fin_count = std::clamp(geometry.fin_count, 3, 6);
    geometry.payload_length_m = std::clamp(geometry.payload_length_m, 0.08, geometry.body_length_m * 0.25);
    geometry.payload_mass_kg = std::clamp(geometry.payload_mass_kg, 0.1, 8.0);
    geometry.nose_controls.mid_radius_scale = std::clamp(geometry.nose_controls.mid_radius_scale, 0.55, 1.45);
    geometry.nose_controls.shoulder_radius_scale = std::clamp(geometry.nose_controls.shoulder_radius_scale, 0.7, 1.25);
    geometry.body_controls.fore_radius_scale = std::clamp(geometry.body_controls.fore_radius_scale, 0.7, 1.3);
    geometry.body_controls.mid_radius_scale = std::clamp(geometry.body_controls.mid_radius_scale, 0.7, 1.3);
    geometry.body_controls.aft_radius_scale = std::clamp(geometry.body_controls.aft_radius_scale, 0.55, 1.3);
    geometry.transition_controls.mid_radius_scale = std::clamp(geometry.transition_controls.mid_radius_scale, 0.55, 1.35);
    geometry.fin_controls.tip_le_offset_m = std::clamp(geometry.fin_controls.tip_le_offset_m, -0.12, 0.12);
    geometry.fin_controls.tip_te_offset_m = std::clamp(geometry.fin_controls.tip_te_offset_m, -0.12, 0.12);
    geometry.fin_controls.span_scale = std::clamp(geometry.fin_controls.span_scale, 0.55, 1.45);
    geometry.fin_controls.thickness_scale = std::clamp(geometry.fin_controls.thickness_scale, 0.55, 1.6);
    geometry.structure_cg_from_nose_m = std::clamp(geometry.structure_cg_from_nose_m, 0.2, geometry.body_length_m - 0.1);
    geometry.propellant_cg_from_nose_m = std::clamp(
        geometry.propellant_cg_from_nose_m,
        geometry.structure_cg_from_nose_m,
        geometry.body_length_m - 0.05);
    geometry.fin_front_from_nose_m = std::clamp(
        geometry.fin_front_from_nose_m,
        geometry.nose_length_m + 0.05,
        geometry.body_length_m - geometry.transition_length_m - geometry.fin_root_chord_m - 0.02);

    vehicle.reference_area_m2 = kPi * std::pow(geometry.body_diameter_m * 0.5, 2.0);
    vehicle.dry_mass_kg = rocket::estimateDryMassKg(geometry, 0.45);
    vehicle.aerodynamic_coefficients.drag_coefficient = deriveBaseDragCoefficient(geometry);
    vehicle.aerodynamic_coefficients.normal_force_slope_per_rad = deriveNormalForceSlopePerRad(geometry);
    vehicle.aerodynamic_coefficients.rotational_damping_coefficient =
        deriveRotationalDampingCoefficient(geometry, vehicle.dry_mass_kg);
    vehicle.principal_inertia_kgm2 = estimatePrincipalInertiaKgM2(geometry, vehicle.dry_mass_kg);
}

std::vector<rocket::MountedMotor> buildClusterLayout(
    int motor_count,
    double thrust_n,
    double burn_time_s,
    double propellant_mass_kg,
    double mount_radius_m) {
    std::vector<rocket::MountedMotor> motors;
    const int count = std::max(motor_count, 1);
    motors.reserve(static_cast<std::size_t>(count));

    for (int index = 0; index < count; ++index) {
        double angle_rad = 0.0;
        rocket::Vector3 mount_position {};
        if (count > 1) {
            angle_rad = (2.0 * kPi * static_cast<double>(index)) / static_cast<double>(count);
            mount_position = {
                mount_radius_m * std::cos(angle_rad),
                mount_radius_m * std::sin(angle_rad),
                0.0
            };
        }

        motors.push_back(rocket::MountedMotor {
            .motor = rocket::Motor {
                .max_thrust_n = thrust_n,
                .burn_time_s = burn_time_s,
                .propellant_mass_kg = propellant_mass_kg
            },
            .mount_position_m = mount_position,
            .thrust_direction_body = {0.0, 0.0, 1.0},
            .failed = false
        });
    }

    return motors;
}

void resetRuntime(const rocket::VehicleModel& vehicle, RuntimeState& runtime) {
    runtime.initial_state = buildRestState(vehicle);
    runtime.state = runtime.initial_state;
    runtime.time_s = 0.0;
    runtime.accumulator_s = 0.0;
    runtime.max_altitude_m = 0.0;
    runtime.paused = true;
    runtime.replay_active = false;
    runtime.replay_time_s = 0.0;
    runtime.burnout_recorded = false;
    runtime.apogee_recorded = false;
    runtime.impact_recorded = false;
    runtime.burnout_point_m = runtime.state.position_m;
    runtime.apogee_point_m = runtime.state.position_m;
    runtime.impact_point_m = runtime.state.position_m;
    runtime.burnout_time_s = 0.0;
    runtime.apogee_time_s = 0.0;
    runtime.impact_time_s = 0.0;
    runtime.keyframe_preview_active = false;
    runtime.keyframe_preview_index = -1;
    runtime.keyframe_preview_time_s = 0.0;
    runtime.trajectory_history.clear();
    runtime.trajectory_history.push_back({runtime.state, 0.0});
}

std::vector<MissionKeyframe> buildMissionKeyframes(const RuntimeState& runtime) {
    std::vector<MissionKeyframe> keyframes;
    keyframes.push_back({"Launch", 0.0});
    if (runtime.burnout_recorded) {
        keyframes.push_back({"Burnout", runtime.burnout_time_s});
    }
    if (runtime.apogee_recorded) {
        keyframes.push_back({"Apogee", runtime.apogee_time_s});
    }
    if (runtime.impact_recorded) {
        keyframes.push_back({"Impact", runtime.impact_time_s});
    }
    return keyframes;
}

void cycleMissionKeyframePreview(RuntimeState& runtime) {
    const auto keyframes = buildMissionKeyframes(runtime);
    if (keyframes.empty()) {
        runtime.keyframe_preview_active = false;
        runtime.keyframe_preview_index = -1;
        runtime.keyframe_preview_time_s = 0.0;
        return;
    }

    if (!runtime.keyframe_preview_active) {
        runtime.keyframe_preview_active = true;
        runtime.keyframe_preview_index = 0;
        runtime.keyframe_preview_time_s = keyframes.front().time_s;
        runtime.paused = true;
        return;
    }

    const int next_index = runtime.keyframe_preview_index + 1;
    if (next_index >= static_cast<int>(keyframes.size())) {
        runtime.keyframe_preview_active = false;
        runtime.keyframe_preview_index = -1;
        runtime.keyframe_preview_time_s = 0.0;
        return;
    }

    runtime.keyframe_preview_index = next_index;
    runtime.keyframe_preview_time_s = keyframes[static_cast<std::size_t>(next_index)].time_s;
    runtime.paused = true;
}

rocket::FlightState sampleTrajectoryState(const std::deque<TrajectorySample>& history, double time_s) {
    if (history.empty()) {
        return {};
    }
    if (time_s <= history.front().time_s) {
        return history.front().state;
    }
    if (time_s >= history.back().time_s) {
        return history.back().state;
    }

    for (std::size_t index = 1; index < history.size(); ++index) {
        const auto& previous = history[index - 1];
        const auto& current = history[index];
        if (time_s <= current.time_s) {
            const double segment_time = std::max(current.time_s - previous.time_s, 1e-6);
            const double t = std::clamp((time_s - previous.time_s) / segment_time, 0.0, 1.0);
            return {
                .position_m = previous.state.position_m * (1.0 - t) + current.state.position_m * t,
                .velocity_mps = previous.state.velocity_mps * (1.0 - t) + current.state.velocity_mps * t,
                .attitude_body_to_world =
                    ((1.0 - t) * previous.state.attitude_body_to_world + t * current.state.attitude_body_to_world).normalized(),
                .angular_velocity_body_radps =
                    previous.state.angular_velocity_body_radps * (1.0 - t) + current.state.angular_velocity_body_radps * t,
                .mass_kg = previous.state.mass_kg * (1.0 - t) + current.state.mass_kg * t
            };
        }
    }

    return history.back().state;
}

rocket::FlightState currentRenderState(const RuntimeState& runtime) {
    if (runtime.keyframe_preview_active && !runtime.trajectory_history.empty()) {
        return sampleTrajectoryState(runtime.trajectory_history, runtime.keyframe_preview_time_s);
    }
    if (runtime.replay_active && !runtime.trajectory_history.empty()) {
        return sampleTrajectoryState(runtime.trajectory_history, runtime.replay_time_s);
    }
    return runtime.state;
}

double currentRenderTime(const RuntimeState& runtime) {
    if (runtime.keyframe_preview_active) {
        return runtime.keyframe_preview_time_s;
    }
    return runtime.replay_active ? runtime.replay_time_s : runtime.time_s;
}

double horizontalRangeM(const rocket::Vector3& point) {
    return std::sqrt(point.x * point.x + point.y * point.y);
}

class RocketLabController {
public:
    RocketLabController()
        : vehicle_(buildDefaultVehicle()) {
        current_preset_ = rocket::RocketPreset::ResearchStarter;
        motor_count_ = 2;
        thrust_n_ = 180.0;
        burn_time_s_ = 2.4;
        propellant_mass_kg_ = 0.24;
        mount_radius_m_ = 0.04;
        rebuildMotorCluster();
    }

    void switchWorkspace(int index) {
        workspace_index_ = std::clamp(index, 0, 1);
    }

    void selectPreset(const std::string& label) {
        if (label == "Research Starter") {
            current_preset_ = rocket::RocketPreset::ResearchStarter;
        } else if (label == "Sport Trainer") {
            current_preset_ = rocket::RocketPreset::SportTrainer;
        } else if (label == "High Altitude") {
            current_preset_ = rocket::RocketPreset::HighAltitude;
        } else if (label == "Minimum Diameter") {
            current_preset_ = rocket::RocketPreset::MinimumDiameter;
        } else if (label == "Heavy Lift") {
            current_preset_ = rocket::RocketPreset::HeavyLift;
        }
        vehicle_.geometry = rocket::makePresetGeometry(current_preset_);
        syncVehicleDerivedValues(vehicle_);
        rebuildMotorCluster();
    }

    void setNoseShape(const std::string& label) {
        if (label == "Conical") {
            vehicle_.geometry.nose_cone_shape = rocket::NoseConeShape::Conical;
        } else if (label == "Tangent Ogive") {
            vehicle_.geometry.nose_cone_shape = rocket::NoseConeShape::TangentOgive;
        } else if (label == "Parabolic") {
            vehicle_.geometry.nose_cone_shape = rocket::NoseConeShape::Parabolic;
        } else if (label == "LD-Haack") {
            vehicle_.geometry.nose_cone_shape = rocket::NoseConeShape::LdHaack;
        }
        rebuildVehicle();
    }

    void setFinShape(const std::string& label) {
        if (label == "Trapezoidal") {
            vehicle_.geometry.fin_shape = rocket::FinShape::Trapezoidal;
        } else if (label == "Elliptical") {
            vehicle_.geometry.fin_shape = rocket::FinShape::Elliptical;
        } else if (label == "Airfoil") {
            vehicle_.geometry.fin_shape = rocket::FinShape::Airfoil;
        }
        rebuildVehicle();
    }

    void setTransitionShape(const std::string& label) {
        vehicle_.geometry.transition_shape =
            label == "Curved" ? rocket::TransitionShape::Curved : rocket::TransitionShape::Conical;
        rebuildVehicle();
    }

    void setMaterial(const std::string& component, const std::string& label) {
        const auto material = parseMaterial(label);
        if (component == "body") {
            vehicle_.geometry.body_material = material;
        } else if (component == "nose") {
            vehicle_.geometry.nose_material = material;
        } else if (component == "fins") {
            vehicle_.geometry.fin_material = material;
        }
        rebuildVehicle();
    }

    void changeInt(const std::string& field, int value) {
        if (field == "body_length_cm") {
            vehicle_.geometry.body_length_m = static_cast<double>(value) / 100.0;
        } else if (field == "body_diameter_mm") {
            vehicle_.geometry.body_diameter_m = static_cast<double>(value) / 1000.0;
        } else if (field == "wall_thickness_tenths_mm") {
            vehicle_.geometry.wall_thickness_m = static_cast<double>(value) / 10000.0;
        } else if (field == "nose_length_cm") {
            vehicle_.geometry.nose_length_m = static_cast<double>(value) / 100.0;
        } else if (field == "payload_length_cm") {
            vehicle_.geometry.payload_length_m = static_cast<double>(value) / 100.0;
        } else if (field == "payload_mass_g") {
            vehicle_.geometry.payload_mass_kg = static_cast<double>(value) / 1000.0;
        } else if (field == "fin_root_cm") {
            vehicle_.geometry.fin_root_chord_m = static_cast<double>(value) / 100.0;
        } else if (field == "fin_tip_cm") {
            vehicle_.geometry.fin_tip_chord_m = static_cast<double>(value) / 100.0;
        } else if (field == "fin_span_cm") {
            vehicle_.geometry.fin_span_m = static_cast<double>(value) / 100.0;
        } else if (field == "fin_sweep_cm") {
            vehicle_.geometry.fin_sweep_length_m = static_cast<double>(value) / 100.0;
        } else if (field == "fin_count") {
            vehicle_.geometry.fin_count = value;
        } else if (field == "motor_count") {
            motor_count_ = value;
            rebuildMotorCluster();
            return;
        } else if (field == "motor_thrust_n") {
            thrust_n_ = static_cast<double>(value);
            rebuildMotorCluster();
            return;
        } else if (field == "burn_time_tenths_s") {
            burn_time_s_ = static_cast<double>(value) / 10.0;
            rebuildMotorCluster();
            return;
        } else if (field == "propellant_mass_g") {
            propellant_mass_kg_ = static_cast<double>(value) / 1000.0;
            rebuildMotorCluster();
            return;
        } else if (field == "mount_radius_mm") {
            mount_radius_m_ = static_cast<double>(value) / 1000.0;
            rebuildMotorCluster();
            return;
        } else if (field == "launch_elevation_m") {
            auto site = environment_.launchSite();
            site.elevation_m = static_cast<double>(value);
            environment_.setLaunchSite(site);
            resetRuntime(vehicle_, runtime_);
            return;
        } else if (field == "wind_speed_tenths_mps") {
            auto weather = environment_.surfaceWeather();
            weather.wind_speed_mps = static_cast<double>(value) / 10.0;
            environment_.setSurfaceWeather(weather);
            resetRuntime(vehicle_, runtime_);
            return;
        } else if (field == "wind_gust_tenths_mps") {
            auto weather = environment_.surfaceWeather();
            weather.wind_gust_mps = static_cast<double>(value) / 10.0;
            environment_.setSurfaceWeather(weather);
            resetRuntime(vehicle_, runtime_);
            return;
        } else if (field == "wind_direction_deg") {
            auto weather = environment_.surfaceWeather();
            weather.wind_direction_deg = static_cast<double>(value);
            environment_.setSurfaceWeather(weather);
            resetRuntime(vehicle_, runtime_);
            return;
        } else if (field == "deploy_altitude_m") {
            vehicle_.recovery_system.deployment_altitude_m = static_cast<double>(value);
            resetRuntime(vehicle_, runtime_);
            return;
        } else if (field == "chute_area_tenths_m2") {
            vehicle_.recovery_system.parachute_area_m2 = static_cast<double>(value) / 10.0;
            resetRuntime(vehicle_, runtime_);
            return;
        }

        rebuildVehicle();
    }

    void triggerAction(const std::string& action) {
        if (action == "toggle_pause") {
            runtime_.paused = !runtime_.paused;
        } else if (action == "reset") {
            resetRuntime(vehicle_, runtime_);
        } else if (action == "toggle_replay") {
            runtime_.replay_active = !runtime_.replay_active;
            runtime_.replay_time_s = 0.0;
            runtime_.paused = runtime_.replay_active ? true : runtime_.paused;
        } else if (action == "next_keyframe") {
            cycleMissionKeyframePreview(runtime_);
        }
    }

    void toggleMotor(int index) {
        if (index < 0 || static_cast<std::size_t>(index) >= vehicle_.cluster.motorCount()) {
            return;
        }
        vehicle_.cluster.setMotorFailed(static_cast<std::size_t>(index), !vehicle_.cluster.motorFailed(static_cast<std::size_t>(index)));
        resetRuntime(vehicle_, runtime_);
    }

    void tick() {
        updateReplayTimeline();
        if (runtime_.paused || runtime_.replay_active || runtime_.keyframe_preview_active) {
            return;
        }

        constexpr double simulation_dt = 1.0 / rocket::physical_constants::simulation_frequency_hz;
        constexpr double frame_time_s = 1.0 / 60.0;
        runtime_.accumulator_s += frame_time_s;

        while (runtime_.accumulator_s >= simulation_dt) {
            const bool was_burning = vehicle_.cluster.isBurning(runtime_.time_s);
            runtime_.state = rocket::integrateRk4(runtime_.state, vehicle_, environment_, runtime_.time_s, simulation_dt);
            runtime_.time_s += simulation_dt;
            runtime_.accumulator_s -= simulation_dt;
            runtime_.max_altitude_m = std::max(runtime_.max_altitude_m, runtime_.state.position_m.z);

            if (!runtime_.apogee_recorded || runtime_.state.position_m.z >= runtime_.apogee_point_m.z) {
                runtime_.apogee_recorded = true;
                runtime_.apogee_point_m = runtime_.state.position_m;
                runtime_.apogee_time_s = runtime_.time_s;
            }

            if (runtime_.trajectory_history.empty() ||
                (runtime_.state.position_m - runtime_.trajectory_history.back().state.position_m).magnitude() > 0.2) {
                runtime_.trajectory_history.push_back({runtime_.state, runtime_.time_s});
            }
            if (runtime_.trajectory_history.size() > 2500) {
                runtime_.trajectory_history.pop_front();
            }

            if (!runtime_.burnout_recorded && was_burning && !vehicle_.cluster.isBurning(runtime_.time_s)) {
                runtime_.burnout_recorded = true;
                runtime_.burnout_point_m = runtime_.state.position_m;
                runtime_.burnout_time_s = runtime_.time_s;
            }

            if (!runtime_.impact_recorded &&
                runtime_.time_s > 0.5 &&
                runtime_.state.position_m.z <= 0.0 &&
                runtime_.state.velocity_mps.z < 0.0) {
                runtime_.impact_recorded = true;
                runtime_.impact_point_m = runtime_.state.position_m;
                runtime_.impact_time_s = runtime_.time_s;
                runtime_.replay_active = true;
                runtime_.replay_time_s = 0.0;
                runtime_.paused = true;
            }
        }
    }

    template <typename UiHandle>
    void refresh(UiHandle& ui) const {
        const auto snapshot = buildSnapshot();

        ui->set_workspace_index(workspace_index_);
        ui->set_active_preset(toSharedString(presetLabel(current_preset_)));
        ui->set_nose_shape(toSharedString(std::string(rocket::noseConeShapeLabel(vehicle_.geometry.nose_cone_shape))));
        ui->set_fin_shape(toSharedString(std::string(rocket::finShapeLabel(vehicle_.geometry.fin_shape))));
        ui->set_transition_shape(toSharedString(std::string(rocket::transitionShapeLabel(vehicle_.geometry.transition_shape))));
        ui->set_body_material(toSharedString(std::string(rocket::materialDefinition(vehicle_.geometry.body_material).label)));
        ui->set_nose_material(toSharedString(std::string(rocket::materialDefinition(vehicle_.geometry.nose_material).label)));
        ui->set_fin_material(toSharedString(std::string(rocket::materialDefinition(vehicle_.geometry.fin_material).label)));

        ui->set_model_mass(toSharedString(std::format("{:.2f} kg", snapshot.state.mass_kg)));
        ui->set_model_stability(toSharedString(std::format("{:.2f} cal", snapshot.static_margin_calibers)));
        ui->set_model_cgcp(toSharedString(std::format("{:.2f} / {:.2f} m", snapshot.cg_from_nose_m, snapshot.cp_from_nose_m)));
        ui->set_model_drag(toSharedString(std::format("Cd {:.2f}", vehicle_.aerodynamic_coefficients.drag_coefficient)));

        ui->set_sim_time(toSharedString(std::format("{:.2f} s", snapshot.time_s)));
        ui->set_sim_altitude(toSharedString(std::format("{:.1f} m", snapshot.state.position_m.z)));
        ui->set_sim_speed(toSharedString(std::format("{:.1f} m/s", snapshot.relative_air_speed_mps)));
        ui->set_sim_mach(toSharedString(std::format("{:.2f}", snapshot.mach_number)));
        ui->set_sim_aoa(toSharedString(std::format("{:.2f} deg", snapshot.angle_of_attack_deg)));
        ui->set_sim_dynamic_pressure(toSharedString(std::format("{:.0f} Pa", snapshot.dynamic_pressure_pa)));
        ui->set_sim_apogee(toSharedString(std::format("{:.1f} m", runtime_.max_altitude_m)));
        ui->set_sim_range(toSharedString(std::format("{:.1f} m", horizontalRangeM(currentRenderState(runtime_).position_m))));

        ui->set_paused(runtime_.paused);
        ui->set_replay_active(runtime_.replay_active);

        ui->set_body_length_cm(static_cast<int>(std::lround(vehicle_.geometry.body_length_m * 100.0)));
        ui->set_body_diameter_mm(static_cast<int>(std::lround(vehicle_.geometry.body_diameter_m * 1000.0)));
        ui->set_wall_thickness_tenths_mm(static_cast<int>(std::lround(vehicle_.geometry.wall_thickness_m * 10000.0)));
        ui->set_nose_length_cm(static_cast<int>(std::lround(vehicle_.geometry.nose_length_m * 100.0)));
        ui->set_payload_length_cm(static_cast<int>(std::lround(vehicle_.geometry.payload_length_m * 100.0)));
        ui->set_payload_mass_g(static_cast<int>(std::lround(vehicle_.geometry.payload_mass_kg * 1000.0)));
        ui->set_fin_root_cm(static_cast<int>(std::lround(vehicle_.geometry.fin_root_chord_m * 100.0)));
        ui->set_fin_tip_cm(static_cast<int>(std::lround(vehicle_.geometry.fin_tip_chord_m * 100.0)));
        ui->set_fin_span_cm(static_cast<int>(std::lround(vehicle_.geometry.fin_span_m * 100.0)));
        ui->set_fin_sweep_cm(static_cast<int>(std::lround(vehicle_.geometry.fin_sweep_length_m * 100.0)));
        ui->set_fin_count(vehicle_.geometry.fin_count);
        ui->set_motor_count(motor_count_);
        ui->set_motor_thrust_n(static_cast<int>(std::lround(thrust_n_)));
        ui->set_burn_time_tenths_s(static_cast<int>(std::lround(burn_time_s_ * 10.0)));
        ui->set_propellant_mass_g(static_cast<int>(std::lround(propellant_mass_kg_ * 1000.0)));
        ui->set_mount_radius_mm(static_cast<int>(std::lround(mount_radius_m_ * 1000.0)));

        ui->set_launch_elevation_m(static_cast<int>(std::lround(environment_.launchSite().elevation_m)));
        ui->set_wind_speed_tenths_mps(static_cast<int>(std::lround(environment_.surfaceWeather().wind_speed_mps * 10.0)));
        ui->set_wind_gust_tenths_mps(static_cast<int>(std::lround(environment_.surfaceWeather().wind_gust_mps * 10.0)));
        ui->set_wind_direction_deg(static_cast<int>(std::lround(environment_.surfaceWeather().wind_direction_deg)));
        ui->set_deploy_altitude_m(static_cast<int>(std::lround(vehicle_.recovery_system.deployment_altitude_m)));
        ui->set_chute_area_tenths_m2(static_cast<int>(std::lround(vehicle_.recovery_system.parachute_area_m2 * 10.0)));

        ui->set_mission_state(toSharedString(missionState(snapshot)));
        ui->set_mission_hint(toSharedString(missionHint(snapshot)));
        ui->set_weather_source(toSharedString(weatherSourceLabel(environment_.weatherDataSource())));
        ui->set_trajectory_summary(toSharedString(trajectorySummary(snapshot)));
        ui->set_keyframe_status(toSharedString(keyframeStatus()));

        setMotorUi(ui, 1);
        setMotorUi(ui, 2);
        setMotorUi(ui, 3);
        setMotorUi(ui, 4);
        setMotorUi(ui, 5);
        setMotorUi(ui, 6);
    }

private:
    static rocket::VehicleModel buildDefaultVehicle() {
        rocket::VehicleModel vehicle {
            .dry_mass_kg = 8.0,
            .reference_area_m2 = kPi * std::pow(0.104 * 0.5, 2.0),
            .principal_inertia_kgm2 = {1.8, 1.8, 0.08},
            .geometry = rocket::makePresetGeometry(rocket::RocketPreset::ResearchStarter),
            .aerodynamic_coefficients = {
                .drag_coefficient = 0.62,
                .normal_force_slope_per_rad = 3.4,
                .rotational_damping_coefficient = 1.1
            },
            .cluster = rocket::MotorCluster(buildClusterLayout(2, 180.0, 2.4, 0.24, 0.04))
        };
        syncVehicleDerivedValues(vehicle);
        return vehicle;
    }

    void rebuildVehicle() {
        syncVehicleDerivedValues(vehicle_);
        resetRuntime(vehicle_, runtime_);
    }

    void rebuildMotorCluster() {
        vehicle_.cluster = rocket::MotorCluster(buildClusterLayout(
            std::clamp(motor_count_, 1, 6),
            std::clamp(thrust_n_, 20.0, 2500.0),
            std::clamp(burn_time_s_, 0.4, 20.0),
            std::clamp(propellant_mass_kg_, 0.02, 8.0),
            std::clamp(mount_radius_m_, 0.0, vehicle_.geometry.body_diameter_m * 0.42)));
        vehicle_.geometry.propellant_cg_from_nose_m =
            std::clamp(vehicle_.geometry.body_length_m - vehicle_.geometry.transition_length_m - 0.22, 0.4, vehicle_.geometry.body_length_m - 0.05);
        rebuildVehicle();
    }

    rocket::SimulationSnapshot buildSnapshot() const {
        const auto render_state = currentRenderState(runtime_);
        const auto render_time = currentRenderTime(runtime_);
        const auto force = rocket::computeForces(render_state, vehicle_, environment_, render_time);
        return {
            .time_s = render_time,
            .state = render_state,
            .max_altitude_m = runtime_.max_altitude_m,
            .cg_from_nose_m = force.center_of_gravity_from_nose_m,
            .cp_from_nose_m = force.center_of_pressure_from_nose_m,
            .static_margin_calibers = force.static_margin_calibers,
            .angle_of_attack_deg = force.angle_of_attack_rad * (180.0 / kPi),
            .dynamic_pressure_pa = force.dynamic_pressure_pa,
        .mach_number = force.mach_number,
        .relative_air_speed_mps = force.relative_air_velocity_world_mps.magnitude(),
        .wind_speed_mps = force.wind_velocity_world_mps.magnitude(),
        .recommended_max_dynamic_pressure_pa = rocket::estimateRecommendedMaxDynamicPressurePa(vehicle_.geometry),
        .dynamic_pressure_safety_factor =
            rocket::estimateDynamicPressureSafetyFactor(vehicle_.geometry, force.dynamic_pressure_pa),
        .equivalent_structural_modulus_gpa =
            rocket::estimateStructuralMaterialAssessment(vehicle_.geometry).equivalent_modulus_gpa,
        .equivalent_structural_density_kg_per_m3 =
            rocket::estimateStructuralMaterialAssessment(vehicle_.geometry).equivalent_density_kg_per_m3,
        .parachute_deployed = force.parachute_deployed,
        .nose_shape_label = std::string(rocket::noseConeShapeLabel(vehicle_.geometry.nose_cone_shape)),
        .fin_shape_label = std::string(rocket::finShapeLabel(vehicle_.geometry.fin_shape))
        };
    }

    void updateReplayTimeline() {
        if (runtime_.replay_active && !runtime_.trajectory_history.empty()) {
            runtime_.replay_time_s += 1.0 / 60.0 * 0.9;
            const double end_time_s = runtime_.trajectory_history.back().time_s;
            if (runtime_.replay_time_s > end_time_s) {
                runtime_.replay_time_s = 0.0;
            }
        }
    }

    static rocket::ComponentMaterial parseMaterial(const std::string& label) {
        if (label == "PLA-CF") {
            return rocket::ComponentMaterial::PlaCf;
        }
        if (label == "PVC") {
            return rocket::ComponentMaterial::Pvc;
        }
        return rocket::ComponentMaterial::Aluminum6061;
    }

    static std::string presetLabel(rocket::RocketPreset preset) {
        const std::string raw(rocket::rocketPresetLabel(preset));
        if (raw == "Research Starter") {
            return raw;
        }
        if (raw == "Sport Trainer") {
            return raw;
        }
        if (raw == "High Altitude") {
            return raw;
        }
        if (raw == "Minimum Diameter") {
            return raw;
        }
        if (raw == "Heavy Lift") {
            return raw;
        }
        return raw;
    }

    static std::string weatherSourceLabel(rocket::WeatherDataSource source) {
        switch (source) {
        case rocket::WeatherDataSource::Manual:
            return "Manual";
        case rocket::WeatherDataSource::OpenMeteoReady:
            return "Open-Meteo Ready";
        case rocket::WeatherDataSource::OpenWeatherMapReady:
            return "OpenWeatherMap Ready";
        }
        return "Manual";
    }

    std::string missionState(const rocket::SimulationSnapshot& snapshot) const {
        if (runtime_.keyframe_preview_active) {
            return "Keyframe analysis";
        }
        if (runtime_.time_s > 0.0 && vehicle_.cluster.isBurning(runtime_.time_s)) {
            return "Boost phase active";
        }
        if (snapshot.parachute_deployed) {
            return "Recovery descent";
        }
        if (runtime_.impact_recorded) {
            return "Ground contact";
        }
        if (runtime_.replay_active) {
            return "Replay active";
        }
        return "Ready on stand";
    }

    std::string missionHint(const rocket::SimulationSnapshot& snapshot) const {
        if (runtime_.keyframe_preview_active) {
            return "Use the keyframe control to inspect the next important mission instant.";
        }
        if (runtime_.time_s > 0.0 && vehicle_.cluster.isBurning(runtime_.time_s)) {
            return "Telemetry is live: watch Mach, AoA and dynamic pressure evolve together.";
        }
        if (snapshot.parachute_deployed) {
            return "Recovery is active: compare altitude, descent rate and expected impact range.";
        }
        if (runtime_.impact_recorded) {
            return "Mission completed: review the replay or change the scenario for the next run.";
        }
        return "Use the primary control to start the simulation.";
    }

    std::string keyframeStatus() const {
        if (!runtime_.keyframe_preview_active) {
            return "Launch keyframe ready";
        }
        const auto keyframes = buildMissionKeyframes(runtime_);
        if (runtime_.keyframe_preview_index >= 0 &&
            runtime_.keyframe_preview_index < static_cast<int>(keyframes.size())) {
            const auto& keyframe = keyframes[static_cast<std::size_t>(runtime_.keyframe_preview_index)];
            return std::format("{} at {:.2f} s", keyframe.label, keyframe.time_s);
        }
        return "Keyframe unavailable";
    }

    std::string trajectorySummary(const rocket::SimulationSnapshot& snapshot) const {
        const auto render_state = currentRenderState(runtime_);
        return std::format(
            "Mission state: {}.\nCurrent altitude: {:.1f} m.\nRelative air speed: {:.1f} m/s.\nMach: {:.2f}, AoA: {:.2f} deg, q: {:.0f} Pa.\nEstimated apogee so far: {:.1f} m.\nCurrent downrange: {:.1f} m.\nNose profile: {}. Fin profile: {}.",
            missionState(snapshot),
            render_state.position_m.z,
            snapshot.relative_air_speed_mps,
            snapshot.mach_number,
            snapshot.angle_of_attack_deg,
            snapshot.dynamic_pressure_pa,
            runtime_.max_altitude_m,
            horizontalRangeM(render_state.position_m),
            snapshot.nose_shape_label,
            snapshot.fin_shape_label);
    }

    template <typename UiHandle>
    void setMotorUi(UiHandle& ui, int slot) const {
        const std::size_t index = static_cast<std::size_t>(slot - 1);
        const bool enabled = index < vehicle_.cluster.motorCount();
        const bool failed = enabled ? vehicle_.cluster.motorFailed(index) : false;
        const auto label = enabled
            ? std::format("Motor {} {}", slot, failed ? "FAILED" : "OK")
            : std::format("Motor {} N/A", slot);

        switch (slot) {
        case 1:
            ui->set_motor_1_label(toSharedString(label));
            ui->set_motor_1_enabled(enabled);
            ui->set_motor_1_failed(failed);
            break;
        case 2:
            ui->set_motor_2_label(toSharedString(label));
            ui->set_motor_2_enabled(enabled);
            ui->set_motor_2_failed(failed);
            break;
        case 3:
            ui->set_motor_3_label(toSharedString(label));
            ui->set_motor_3_enabled(enabled);
            ui->set_motor_3_failed(failed);
            break;
        case 4:
            ui->set_motor_4_label(toSharedString(label));
            ui->set_motor_4_enabled(enabled);
            ui->set_motor_4_failed(failed);
            break;
        case 5:
            ui->set_motor_5_label(toSharedString(label));
            ui->set_motor_5_enabled(enabled);
            ui->set_motor_5_failed(failed);
            break;
        case 6:
            ui->set_motor_6_label(toSharedString(label));
            ui->set_motor_6_enabled(enabled);
            ui->set_motor_6_failed(failed);
            break;
        default:
            break;
        }
    }

    int workspace_index_ {};
    rocket::RocketPreset current_preset_ {rocket::RocketPreset::ResearchStarter};
    rocket::VehicleModel vehicle_;
    rocket::Environment environment_ {};
    RuntimeState runtime_ {};
    int motor_count_ {2};
    double thrust_n_ {180.0};
    double burn_time_s_ {2.4};
    double propellant_mass_kg_ {0.24};
    double mount_radius_m_ {0.04};
};

}  // namespace

int rocket::runRocketLabApp() {
    auto controller = std::make_shared<RocketLabController>();
    auto ui = RocketLabWindow::create();
    slint::ComponentWeakHandle<RocketLabWindow> weak_ui(ui);

    controller->refresh(ui);

    ui->on_switch_workspace([controller, weak_ui](int index) {
        controller->switchWorkspace(index);
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });
    ui->on_select_preset([controller, weak_ui](const slint::SharedString& value) {
        controller->selectPreset(toStdString(value));
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });
    ui->on_set_nose_shape([controller, weak_ui](const slint::SharedString& value) {
        controller->setNoseShape(toStdString(value));
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });
    ui->on_set_fin_shape([controller, weak_ui](const slint::SharedString& value) {
        controller->setFinShape(toStdString(value));
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });
    ui->on_set_transition_shape([controller, weak_ui](const slint::SharedString& value) {
        controller->setTransitionShape(toStdString(value));
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });
    ui->on_set_material([controller, weak_ui](const slint::SharedString& component, const slint::SharedString& material) {
        controller->setMaterial(toStdString(component), toStdString(material));
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });
    ui->on_change_int([controller, weak_ui](const slint::SharedString& field, int value) {
        controller->changeInt(toStdString(field), value);
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });
    ui->on_trigger_action([controller, weak_ui](const slint::SharedString& action) {
        controller->triggerAction(toStdString(action));
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });
    ui->on_toggle_motor([controller, weak_ui](int index) {
        controller->toggleMotor(index);
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });

    slint::Timer timer;
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(16), [controller, weak_ui]() {
        controller->tick();
        if (auto ui = weak_ui.lock()) {
            controller->refresh(*ui);
        }
    });

    ui->run();
    return 0;
}
