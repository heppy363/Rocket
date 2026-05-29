#include <cmath>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "rocket/Aerodynamics.hpp"
#include "rocket/CfdModule.hpp"
#include "rocket/Environment.hpp"
#include "rocket/ProjectIO.hpp"
#include "rocket/RungeKutta4.hpp"
#include "rocket/SimulationCaches.hpp"
#include "rocket/SimulationEngine.hpp"
#include "rocket/SimulationRuntime.hpp"
#include "rocket/Validation.hpp"
#include "rocket/VehicleModel.hpp"

namespace {

rocket::VehicleModel makeValidVehicle() {
    return rocket::VehicleModel {
        .dry_mass_kg = 8.0,
        .reference_area_m2 = 0.0085,
        .principal_inertia_kgm2 = {1.8, 1.8, 0.08},
        .geometry = {},
        .aerodynamic_coefficients = {
            .drag_coefficient = 0.62,
            .normal_force_slope_per_rad = 3.4,
            .rotational_damping_coefficient = 1.1
        },
        .recovery_system = {},
        .cluster = rocket::MotorCluster {}
    };
}

bool nearlyEqual(double lhs, double rhs, double epsilon = 1e-9) {
    return std::abs(lhs - rhs) <= epsilon;
}

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        return false;
    }
    return true;
}

bool testValidationRejectsZeroDt() {
    const rocket::VehicleModel vehicle = makeValidVehicle();
    const rocket::Environment environment;
    const rocket::FlightState state = rocket::buildRestState(vehicle);
    const auto result =
        rocket::validateSimulationInputs(state, vehicle, environment, rocket::Seconds {0.0});
    return check(!result.has_value(), "validateSimulationInputs should reject zero dt");
}

bool testTryIntegrateRk4AdvancesState() {
    const rocket::VehicleModel vehicle = makeValidVehicle();
    const rocket::Environment environment;
    rocket::FlightState state = rocket::buildRestState(vehicle);
    state.position_m.z = 100.0;

    const auto next_state =
        rocket::tryIntegrateRk4(state, vehicle, environment, 0.0, rocket::Seconds {0.01});
    if (!check(next_state.has_value(), "tryIntegrateRk4 should succeed on valid inputs")) {
        return false;
    }

    return check(
        next_state->velocity_mps.z < 0.0 && next_state->mass_kg >= vehicle.dry_mass_kg,
        "RK4 integration should produce downward velocity and preserve dry-mass floor");
}

bool testSimulationRuntimeStep() {
    const rocket::VehicleModel vehicle = makeValidVehicle();
    const rocket::Environment environment;
    rocket::SimulationRuntime runtime;
    rocket::resetSimulationRuntime(vehicle, runtime);
    runtime.state.position_m.z = 100.0;
    runtime.initial_state.position_m.z = 100.0;

    const auto result = rocket::stepSimulationRuntime(
        runtime,
        vehicle,
        environment,
        rocket::Seconds {0.01},
        0.02);
    if (!check(result.has_value(), "stepSimulationRuntime should succeed on valid inputs")) {
        return false;
    }

    return check(
        nearlyEqual(runtime.time_s, 0.02) &&
            runtime.max_altitude_m <= 100.0 &&
            !runtime.trajectory_history.empty() &&
            runtime.trajectory_history.back().static_pressure_pa > 0.0 &&
            runtime.trajectory_history.back().air_density_kgpm3 > 0.0 &&
            runtime.trajectory_history.back().reynolds_number >= 0.0,
        "Simulation runtime should advance time and keep trajectory history telemetry");
}

bool testTwoLevelCachesPreserveResults() {
    const rocket::VehicleModel vehicle = makeValidVehicle();
    const rocket::Environment environment;

    const double altitude_m = 125.0;
    const double pressure_direct = environment.airPressurePa(altitude_m);
    const double density_direct = environment.airDensityKgPerM3(altitude_m);
    const auto structural_direct = rocket::estimateStructuralMaterialAssessment(vehicle.geometry);
    const auto structural_cached = rocket::cachedStructuralMaterialAssessment(vehicle.geometry);

    return check(
               nearlyEqual(pressure_direct, environment.airPressurePa(altitude_m)) &&
                   nearlyEqual(density_direct, environment.airDensityKgPerM3(altitude_m)),
               "Environment L1/L2 software cache should preserve atmosphere results") &&
           check(
               nearlyEqual(structural_direct.equivalent_modulus_gpa, structural_cached.equivalent_modulus_gpa) &&
                   nearlyEqual(structural_direct.equivalent_density_kg_per_m3, structural_cached.equivalent_density_kg_per_m3) &&
                   nearlyEqual(structural_direct.recommended_max_dynamic_pressure_pa, structural_cached.recommended_max_dynamic_pressure_pa),
               "Vehicle geometry L1/L2 software cache should preserve derived structural results");
}

bool testExtendedCachesExposeHits() {
    const rocket::VehicleModel vehicle = makeValidVehicle();
    const rocket::Environment environment;
    rocket::FlightState state = rocket::buildRestState(vehicle);
    state.position_m.z = 150.0;
    state.velocity_mps = {12.0, 1.5, 64.0};

    const auto aero_before = rocket::aerodynamicsCacheStats();
    const auto cfd_before = rocket::cfdCacheStats();

    const double cp_first = rocket::computeCenterOfPressureFromNoseM(vehicle);
    const double cp_second = rocket::computeCenterOfPressureFromNoseM(vehicle);
    const auto cfd_first = rocket::computeCfdAugmentation(
        state,
        vehicle,
        environment,
        state.velocity_mps,
        3200.0,
        0.82,
        0.07);
    const auto cfd_second = rocket::computeCfdAugmentation(
        state,
        vehicle,
        environment,
        state.velocity_mps,
        3200.0,
        0.82,
        0.07);

    const auto aero_after = rocket::aerodynamicsCacheStats();
    const auto cfd_after = rocket::cfdCacheStats();

    return check(
               nearlyEqual(cp_first, cp_second) &&
                   aero_after.l1_hits + aero_after.l2_hits > aero_before.l1_hits + aero_before.l2_hits,
               "Aerodynamics L1/L2 software cache should record reuse on repeated CP queries") &&
           check(
               nearlyEqual(cfd_first.aeroelastic_response, cfd_second.aeroelastic_response) &&
                   cfd_after.l1_hits + cfd_after.l2_hits > cfd_before.l1_hits + cfd_before.l2_hits,
               "CFD geometry L1/L2 software cache should record reuse on repeated augmentation queries");
}

bool testMotorClusterBurnWindowTracksArmedMotors() {
    rocket::MotorCluster cluster({
        rocket::MountedMotor {
            .motor = rocket::Motor {.max_thrust_n = 100.0, .burn_time_s = 1.8, .propellant_mass_kg = 0.10},
            .mount_position_m = {0.0, 0.0, 0.0},
            .thrust_direction_body = {0.0, 0.0, 1.0},
            .failed = false},
        rocket::MountedMotor {
            .motor = rocket::Motor {.max_thrust_n = 100.0, .burn_time_s = 3.2, .propellant_mass_kg = 0.10},
            .mount_position_m = {0.0, 0.0, 0.0},
            .thrust_direction_body = {0.0, 0.0, 1.0},
            .failed = true},
        rocket::MountedMotor {
            .motor = rocket::Motor {.max_thrust_n = 100.0, .burn_time_s = 2.6, .propellant_mass_kg = 0.10},
            .mount_position_m = {0.0, 0.0, 0.0},
            .thrust_direction_body = {0.0, 0.0, 1.0},
            .failed = false},
    });

    return check(
               nearlyEqual(cluster.maxBurnTimeS(), 2.6),
               "Motor cluster burn window should follow the longest armed motor") &&
           check(
               [&cluster]() {
                   cluster.setMotorFailed(0, true);
                   cluster.setMotorFailed(2, true);
                   return nearlyEqual(cluster.maxBurnTimeS(), 0.0);
               }(),
               "Motor cluster burn window should collapse to zero when no motors are armed");
}

bool testProjectDocumentRoundTripPreservesMeshEdits() {
    namespace fs = std::filesystem;

    rocket::ProjectDocument saved;
    saved.active_preset = rocket::RocketPreset::HeavyLift;
    saved.vehicle = makeValidVehicle();
    saved.vehicle.geometry = rocket::makePresetGeometry(rocket::RocketPreset::HeavyLift);
    saved.vehicle.geometry.nose_vertex_mods.component_type = rocket::ComponentType::NoseCone;
    saved.vehicle.geometry.nose_vertex_mods.is_active = true;
    saved.vehicle.geometry.nose_vertex_mods.modified_vertices.push_back(rocket::FreeControlVertex {
        .vertex_id = 7,
        .base_position_m = {0.01, -0.02, 0.33},
        .offset_m = {0.004, 0.001, -0.006},
        .influence_radius_m = 0.085,
        .locked = true});
    saved.vehicle.geometry.motor_topology_override.component_type = rocket::ComponentType::MotorMount;
    saved.vehicle.geometry.motor_topology_override.is_active = true;
    saved.vehicle.geometry.motor_topology_override.vertex_positions_body_m = {
        {-0.02, 0.00, -0.11},
        {0.02, 0.00, -0.11},
        {0.00, 0.03, -0.16}};
    saved.vehicle.geometry.motor_topology_override.indices = {0u, 1u, 2u};
    saved.launch_site = {.latitude_deg = 45.1, .longitude_deg = 9.2, .elevation_m = 220.0};
    saved.surface_weather = {
        .pressure_hpa = 1008.0,
        .temperature_c = 21.5,
        .humidity_percent = 48.0,
        .wind_speed_mps = 6.5,
        .wind_direction_deg = 135.0,
        .wind_gust_mps = 9.1};
    saved.weather_source = rocket::WeatherDataSource::OpenMeteoReady;
    saved.motor_settings = {
        .motor_count = 3,
        .max_thrust_n = 310.0,
        .burn_time_s = 2.7,
        .propellant_mass_kg = 0.41,
        .mount_radius_m = 0.052,
        .cant_angle_deg = 1.75};

    const fs::path temp_path = fs::temp_directory_path() / "rocket_project_roundtrip_test.rlab";
    std::string error_message;
    const bool saved_ok = rocket::saveProjectDocument(temp_path, saved, error_message);

    rocket::ProjectDocument loaded;
    const bool loaded_ok = saved_ok && rocket::loadProjectDocument(temp_path, loaded, error_message);
    fs::remove(temp_path);

    return check(saved_ok, "saveProjectDocument should write a valid .rlab file") &&
           check(loaded_ok, "loadProjectDocument should load a saved .rlab file") &&
           check(
               loaded.active_preset == saved.active_preset &&
                   loaded.weather_source == saved.weather_source &&
                   nearlyEqual(loaded.motor_settings.cant_angle_deg, saved.motor_settings.cant_angle_deg),
               "Project roundtrip should preserve preset, weather source, and motor editor state") &&
           check(
               loaded.vehicle.geometry.nose_vertex_mods.is_active &&
                   loaded.vehicle.geometry.nose_vertex_mods.modified_vertices.size() == 1 &&
                   loaded.vehicle.geometry.nose_vertex_mods.modified_vertices.front().locked &&
                   nearlyEqual(
                       loaded.vehicle.geometry.nose_vertex_mods.modified_vertices.front().offset_m.z,
                       saved.vehicle.geometry.nose_vertex_mods.modified_vertices.front().offset_m.z),
               "Project roundtrip should preserve free vertex modifiers") &&
           check(
               loaded.vehicle.geometry.motor_topology_override.is_active &&
                   loaded.vehicle.geometry.motor_topology_override.vertex_positions_body_m.size() == 3 &&
                   loaded.vehicle.geometry.motor_topology_override.indices == saved.vehicle.geometry.motor_topology_override.indices &&
                   nearlyEqual(
                       loaded.vehicle.geometry.motor_topology_override.vertex_positions_body_m.back().y,
                       saved.vehicle.geometry.motor_topology_override.vertex_positions_body_m.back().y),
               "Project roundtrip should preserve topology overrides");
}

bool testProjectDocumentRejectsUnsupportedFormatVersion() {
    namespace fs = std::filesystem;

    const fs::path temp_path = fs::temp_directory_path() / "rocket_project_bad_version_test.rlab";
    {
        std::ofstream output(temp_path, std::ios::binary);
        output << "rocket_lab_project\n";
        output << "format_version=99\n";
        output << "\n[project]\n";
        output << "active_preset=Research Starter\n";
    }

    rocket::ProjectDocument loaded;
    std::string error_message;
    const bool ok = rocket::loadProjectDocument(temp_path, loaded, error_message);
    fs::remove(temp_path);

    return check(!ok, "loadProjectDocument should reject unsupported format versions") &&
           check(
               error_message.find("Unsupported .rlab format_version") != std::string::npos,
               "loadProjectDocument should report an unsupported format version clearly");
}

}  // namespace

int main() {
    bool ok = true;
    ok = testValidationRejectsZeroDt() && ok;
    ok = testTryIntegrateRk4AdvancesState() && ok;
    ok = testSimulationRuntimeStep() && ok;
    ok = testTwoLevelCachesPreserveResults() && ok;
    ok = testExtendedCachesExposeHits() && ok;
    ok = testMotorClusterBurnWindowTracksArmedMotors() && ok;
    ok = testProjectDocumentRoundTripPreservesMeshEdits() && ok;
    ok = testProjectDocumentRejectsUnsupportedFormatVersion() && ok;

    if (!ok) {
        return EXIT_FAILURE;
    }

    std::cout << "rocket_core_tests passed\n";
    return EXIT_SUCCESS;
}
