#include "rocket/Environment.hpp"

#include <algorithm>
#include <cmath>
#include <format>

#include "rocket/PhysicalConstants.hpp"

namespace rocket {

namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double earth_radius_m = 6371000.0;

double saturationVaporPressurePa(double temperature_c) noexcept {
    const double exponent = (17.625 * temperature_c) / (temperature_c + 243.04);
    return 610.94 * std::exp(exponent);
}

double humidityClampedPercent(double humidity_percent) noexcept {
    return std::clamp(humidity_percent, 0.0, 100.0);
}

double degToRad(double degrees) noexcept {
    return degrees * pi / 180.0;
}

}  // namespace

const LaunchSite& Environment::launchSite() const noexcept {
    return launch_site_;
}

const SurfaceWeather& Environment::surfaceWeather() const noexcept {
    return surface_weather_;
}

WeatherDataSource Environment::weatherDataSource() const noexcept {
    return weather_data_source_;
}

void Environment::setLaunchSite(const LaunchSite& site) noexcept {
    launch_site_ = site;
}

void Environment::setSurfaceWeather(const SurfaceWeather& weather) noexcept {
    surface_weather_ = weather;
}

void Environment::setWeatherDataSource(WeatherDataSource source) noexcept {
    weather_data_source_ = source;
}

double Environment::airTemperatureK(double altitude_m) const noexcept {
    const double relative_altitude_m = std::max(altitude_m, 0.0);
    const double surface_temperature_k = surface_weather_.temperature_c + 273.15;
    return std::max(
        180.0,
        surface_temperature_k - physical_constants::lapse_rate_k_per_m * relative_altitude_m);
}

double Environment::airPressurePa(double altitude_m) const noexcept {
    const double relative_altitude_m = std::max(altitude_m, 0.0);
    const double surface_temperature_k = surface_weather_.temperature_c + 273.15;
    const double clamped_surface_pressure_pa =
        std::max(surface_weather_.pressure_hpa * 100.0, 1000.0);
    const double base = 1.0 - (physical_constants::lapse_rate_k_per_m * relative_altitude_m) / surface_temperature_k;
    const double exponent =
        (physical_constants::gravitational_acceleration * physical_constants::molar_mass_dry_air) /
        (physical_constants::universal_gas_constant * physical_constants::lapse_rate_k_per_m);
    return clamped_surface_pressure_pa * std::pow(std::max(base, 1e-6), exponent);
}

double Environment::airDensityKgPerM3(double altitude_m) const noexcept {
    const double temperature_k = airTemperatureK(altitude_m);
    const double pressure_pa = airPressurePa(altitude_m);
    const double relative_humidity = humidityClampedPercent(surface_weather_.humidity_percent) * 0.01;
    const double vapor_pressure_pa =
        relative_humidity * saturationVaporPressurePa(temperature_k - 273.15);
    const double dry_air_pressure_pa = std::max(pressure_pa - vapor_pressure_pa, 0.0);

    return (dry_air_pressure_pa / (physical_constants::specific_gas_constant_dry_air * temperature_k)) +
           (vapor_pressure_pa / (physical_constants::specific_gas_constant_water_vapor * temperature_k));
}

double Environment::speedOfSoundMps(double altitude_m) const noexcept {
    return std::sqrt(
        physical_constants::ratio_of_specific_heats_air *
        physical_constants::specific_gas_constant_dry_air *
        airTemperatureK(altitude_m));
}

double Environment::gravityMps2(double altitude_m) const noexcept {
    const double latitude_rad = degToRad(launch_site_.latitude_deg);
    const double sin_lat = std::sin(latitude_rad);
    const double sin2 = sin_lat * sin_lat;
    const double surface_gravity =
        9.780327 * (1.0 + 0.0053024 * sin2 - 0.0000058 * std::sin(2.0 * latitude_rad) * std::sin(2.0 * latitude_rad));
    const double radius = earth_radius_m + launch_site_.elevation_m + std::max(altitude_m, 0.0);
    return surface_gravity * std::pow(earth_radius_m / radius, 2.0);
}

Vector3 Environment::windVelocityWorldMps(double altitude_m, double time_s) const noexcept {
    const double relative_altitude_m = std::max(altitude_m, 0.0);
    const double direction_rad = degToRad(surface_weather_.wind_direction_deg);
    const double power_law_scale = std::pow(std::max((relative_altitude_m + 10.0) / 10.0, 0.25), 0.14);
    const double gust_component =
        surface_weather_.wind_gust_mps * 0.35 * std::sin(0.23 * time_s + relative_altitude_m * 0.012);
    const double wind_speed_mps = std::max(0.0, surface_weather_.wind_speed_mps * power_law_scale + gust_component);

    // Wind direction indicates where wind comes from, so the velocity points opposite.
    return {
        -wind_speed_mps * std::sin(direction_rad),
        -wind_speed_mps * std::cos(direction_rad),
        0.0
    };
}

std::string Environment::weatherApiQueryUrl() const {
    switch (weather_data_source_) {
    case WeatherDataSource::Manual:
        return "manual-weather-profile";
    case WeatherDataSource::OpenMeteoReady:
        return std::format(
            "https://api.open-meteo.com/v1/forecast?latitude={:.6f}&longitude={:.6f}&current=temperature_2m,relative_humidity_2m,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m&hourly=temperature_2m,relative_humidity_2m,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m",
            launch_site_.latitude_deg,
            launch_site_.longitude_deg);
    case WeatherDataSource::OpenWeatherMapReady:
        return std::format(
            "https://api.openweathermap.org/data/3.0/onecall?lat={:.6f}&lon={:.6f}&units=metric",
            launch_site_.latitude_deg,
            launch_site_.longitude_deg);
    }
    return "manual-weather-profile";
}

}  // namespace rocket
