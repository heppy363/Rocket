#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <vector>

#include "rocket/Environment.hpp"
#include "rocket/SimulationMonitor.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket {

struct ProjectMotorSettings {
    int motor_count {2};
    double max_thrust_n {180.0};
    double burn_time_s {2.4};
    double propellant_mass_kg {0.24};
    double mount_radius_m {0.04};
    double cant_angle_deg {};
};

struct ProjectDocument {
    RocketPreset active_preset {RocketPreset::ResearchStarter};
    VehicleModel vehicle {};
    LaunchSite launch_site {};
    SurfaceWeather surface_weather {};
    LaunchRail launch_rail {};
    WeatherDataSource weather_source {WeatherDataSource::Manual};
    ProjectMotorSettings motor_settings {};
};

struct TrajectoryRecord {
    double time_s {};
    FlightState state {};
};

struct ProjectExportSummary {
    double mission_time_s {};
    double max_altitude_m {};
    double max_range_m {};
    std::size_t trajectory_sample_count {};
    bool burnout_recorded {false};
    bool apogee_recorded {false};
    bool impact_recorded {false};
    double burnout_time_s {};
    double apogee_time_s {};
    double impact_time_s {};
    Vector3 burnout_point_m {};
    Vector3 apogee_point_m {};
    Vector3 impact_point_m {};
};

[[nodiscard]] bool saveProjectDocument(
    const std::filesystem::path& path,
    const ProjectDocument& document,
    std::string& error_message);

[[nodiscard]] bool loadProjectDocument(
    const std::filesystem::path& path,
    ProjectDocument& document,
    std::string& error_message);

[[nodiscard]] bool exportProjectReport(
    const std::filesystem::path& path,
    const ProjectDocument& document,
    const SimulationSnapshot& snapshot,
    const ProjectExportSummary& summary,
    std::string& error_message);

[[nodiscard]] bool exportTrajectoryCsv(
    const std::filesystem::path& path,
    std::span<const TrajectoryRecord> trajectory,
    std::string& error_message);

}  // namespace rocket
