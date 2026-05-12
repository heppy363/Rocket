#include "rocket/SimulationCore.hpp"

#include <algorithm>

#include "rocket/DesignLibrary.hpp"
#include "rocket/PhysicalConstants.hpp"

namespace rocket {

FlightState buildRestState(const VehicleModel& vehicle) noexcept {
    return {
        .position_m = {0.0, 0.0, 0.0},
        .velocity_mps = {0.0, 0.0, 0.0},
        .attitude_body_to_world = {},
        .angular_velocity_body_radps = {0.0, 0.0, 0.0},
        .mass_kg = vehicle.dry_mass_kg + vehicle.cluster.totalPropellantMassKg()
    };
}

SimulationSnapshot buildSimulationSnapshot(
    const FlightState& state,
    const VehicleModel& vehicle,
    const ForceResult& force_result,
    const Environment& environment,
    double time_s,
    double max_altitude_m,
    int cfd_solver_particle_count,
    int cfd_render_particle_count) {
    const double altitude_m = std::max(state.position_m.z, 0.0);
    const double static_pressure_pa = environment.airPressurePa(altitude_m);
    const double air_density_kgpm3 = environment.airDensityKgPerM3(altitude_m);
    const double air_temperature_k = environment.airTemperatureK(altitude_m);
    const double speed_of_sound_mps = environment.speedOfSoundMps(altitude_m);
    const StructuralMaterialAssessment assessment =
        estimateStructuralMaterialAssessment(vehicle.geometry);
    constexpr double rad_to_deg = 57.29577951308232;

    return SimulationSnapshot {
        .time_s = time_s,
        .state = state,
        .max_altitude_m = max_altitude_m,
        .cg_from_nose_m = force_result.center_of_gravity_from_nose_m,
        .cp_from_nose_m = force_result.center_of_pressure_from_nose_m,
        .static_margin_calibers = force_result.static_margin_calibers,
        .angle_of_attack_deg = force_result.angle_of_attack_rad * rad_to_deg,
        .dynamic_pressure_pa = force_result.dynamic_pressure_pa,
        .static_pressure_pa = static_pressure_pa,
        .total_pressure_pa = static_pressure_pa + std::max(force_result.dynamic_pressure_pa, 0.0),
        .air_density_kgpm3 = air_density_kgpm3,
        .air_temperature_k = air_temperature_k,
        .speed_of_sound_mps = speed_of_sound_mps,
        .mach_number = force_result.mach_number,
        .relative_air_speed_mps = force_result.relative_air_velocity_world_mps.magnitude(),
        .wind_speed_mps = force_result.wind_velocity_world_mps.magnitude(),
        .recommended_max_dynamic_pressure_pa = estimateRecommendedMaxDynamicPressurePa(vehicle.geometry),
        .dynamic_pressure_safety_factor =
            estimateDynamicPressureSafetyFactor(vehicle.geometry, force_result.dynamic_pressure_pa),
        .equivalent_structural_modulus_gpa = assessment.equivalent_modulus_gpa,
        .equivalent_structural_density_kg_per_m3 = assessment.equivalent_density_kg_per_m3,
        .shockwave_intensity = force_result.shockwave_intensity,
        .aeroelastic_response = force_result.aeroelastic_response,
        .cfd_solver_particle_count = cfd_solver_particle_count,
        .cfd_render_particle_count = cfd_render_particle_count,
        .parachute_deployed = force_result.parachute_deployed,
        .nose_shape_label = std::string(noseConeShapeLabel(vehicle.geometry.nose_cone_shape)),
        .fin_shape_label = std::string(finShapeLabel(vehicle.geometry.fin_shape))
    };
}

SimulationMonitorState buildSimulationMonitorState(
    const SimulationSnapshot& snapshot,
    bool paused,
    bool motor_burning,
    const VehicleModel& vehicle,
    const Environment& environment) {
    return {
        .snapshot = snapshot,
        .geometry = vehicle.geometry,
        .recovery_system = vehicle.recovery_system,
        .launch_site = environment.launchSite(),
        .surface_weather = environment.surfaceWeather(),
        .weather_source = environment.weatherDataSource(),
        .paused = paused,
        .motor_burning = motor_burning,
        .motor_count = vehicle.cluster.motorCount()
    };
}

}  // namespace rocket
