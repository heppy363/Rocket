#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "rocket/Environment.hpp"
#include "rocket/FlightState.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket {

struct SimulationSnapshot {
    double time_s {};
    FlightState state {};
    double max_altitude_m {};
    double cg_from_nose_m {};
    double cp_from_nose_m {};
    double static_margin_calibers {};
    double angle_of_attack_deg {};
    double dynamic_pressure_pa {};
    double static_pressure_pa {};
    double total_pressure_pa {};
    double air_density_kgpm3 {};
    double air_temperature_k {};
    double speed_of_sound_mps {};
    double mach_number {};
    double relative_air_speed_mps {};
    double wind_speed_mps {};
    double recommended_max_dynamic_pressure_pa {};
    double dynamic_pressure_safety_factor {};
    double equivalent_structural_modulus_gpa {};
    double equivalent_structural_density_kg_per_m3 {};
    double shockwave_intensity {};
    double aeroelastic_response {};
    int cfd_solver_particle_count {};
    int cfd_render_particle_count {};
    bool parachute_deployed {};
    std::string nose_shape_label {};
    std::string fin_shape_label {};
};

struct SimulationMonitorState {
    SimulationSnapshot snapshot {};
    VehicleGeometry geometry {};
    RecoverySystem recovery_system {};
    LaunchSite launch_site {};
    SurfaceWeather surface_weather {};
    WeatherDataSource weather_source {WeatherDataSource::Manual};
    bool paused {true};
    bool motor_burning {false};
    std::size_t motor_count {};
};

class SimulationMonitor {
public:
    SimulationMonitor();
    ~SimulationMonitor();

    SimulationMonitor(const SimulationMonitor&) = delete;
    SimulationMonitor& operator=(const SimulationMonitor&) = delete;
    SimulationMonitor(SimulationMonitor&&) noexcept;
    SimulationMonitor& operator=(SimulationMonitor&&) noexcept;

    void start();
    void stop();
    void publish(const SimulationMonitorState& state);

    [[nodiscard]] bool isRunning() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace rocket
