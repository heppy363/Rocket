#include <cmath>
#include <cstdlib>
#include <expected>
#include <iostream>

#include "rocket/Aerodynamics.hpp"
#include "rocket/CfdModule.hpp"
#include "rocket/Environment.hpp"
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
            !runtime.trajectory_history.empty(),
        "Simulation runtime should advance time and keep trajectory history");
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

}  // namespace

int main() {
    bool ok = true;
    ok = testValidationRejectsZeroDt() && ok;
    ok = testTryIntegrateRk4AdvancesState() && ok;
    ok = testSimulationRuntimeStep() && ok;
    ok = testTwoLevelCachesPreserveResults() && ok;
    ok = testExtendedCachesExposeHits() && ok;

    if (!ok) {
        return EXIT_FAILURE;
    }

    std::cout << "rocket_core_tests passed\n";
    return EXIT_SUCCESS;
}
