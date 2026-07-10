#include "rocket/Environment.hpp"

#include <algorithm>
#include <cmath>
#include <format>

#include "rocket/PhysicalConstants.hpp"

namespace rocket {

namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double earth_radius_m = 6371000.0;

struct WindLayerAnchor {
    double altitude_m {};
    double speed_scale {};
    double direction_offset_deg {};
};

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

double wrapDegrees360(double degrees) noexcept {
    return std::fmod(degrees + 360.0, 360.0);
}

double smoothstep01(double value) noexcept {
    const double t = std::clamp(value, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double mix(double lhs, double rhs, double t) noexcept {
    return lhs + (rhs - lhs) * t;
}

double interpolateLayerValue(
    double altitude_m,
    double low_altitude_m,
    double low_value,
    double high_altitude_m,
    double high_value) noexcept {
    if (high_altitude_m <= low_altitude_m) {
        return high_value;
    }
    const double t = smoothstep01((altitude_m - low_altitude_m) / (high_altitude_m - low_altitude_m));
    return mix(low_value, high_value, t);
}

WindLayerAnchor interpolatedWindLayer(double altitude_m) noexcept {
    constexpr std::array<WindLayerAnchor, 5> anchors {{
        {.altitude_m = 0.0, .speed_scale = 0.82, .direction_offset_deg = -3.0},
        {.altitude_m = 120.0, .speed_scale = 1.00, .direction_offset_deg = 0.0},
        {.altitude_m = 600.0, .speed_scale = 1.20, .direction_offset_deg = 8.0},
        {.altitude_m = 1800.0, .speed_scale = 1.42, .direction_offset_deg = 18.0},
        {.altitude_m = 4200.0, .speed_scale = 1.30, .direction_offset_deg = 26.0}
    }};

    if (altitude_m <= anchors.front().altitude_m) {
        return anchors.front();
    }
    if (altitude_m >= anchors.back().altitude_m) {
        return anchors.back();
    }

    for (std::size_t index = 1; index < anchors.size(); ++index) {
        if (altitude_m <= anchors[index].altitude_m) {
            const auto& low = anchors[index - 1];
            const auto& high = anchors[index];
            return WindLayerAnchor {
                .altitude_m = altitude_m,
                .speed_scale = interpolateLayerValue(
                    altitude_m,
                    low.altitude_m,
                    low.speed_scale,
                    high.altitude_m,
                    high.speed_scale),
                .direction_offset_deg = interpolateLayerValue(
                    altitude_m,
                    low.altitude_m,
                    low.direction_offset_deg,
                    high.altitude_m,
                    high.direction_offset_deg)
            };
        }
    }

    return anchors.back();
}

}  // namespace

const LaunchSite& Environment::launchSite() const noexcept {
    return launch_site_;
}

const SurfaceWeather& Environment::surfaceWeather() const noexcept {
    return surface_weather_;
}

const LaunchRail& Environment::launchRail() const noexcept {
    return launch_rail_;
}

WeatherDataSource Environment::weatherDataSource() const noexcept {
    return weather_data_source_;
}

void Environment::setLaunchSite(const LaunchSite& site) noexcept {
    launch_site_ = site;
    invalidateCaches();
}

void Environment::setSurfaceWeather(const SurfaceWeather& weather) noexcept {
    surface_weather_ = weather;
    invalidateCaches();
}

void Environment::setLaunchRail(const LaunchRail& rail) noexcept {
    launch_rail_ = rail;
}

void Environment::setWeatherDataSource(WeatherDataSource source) noexcept {
    weather_data_source_ = source;
}

double Environment::airTemperatureK(double altitude_m) const noexcept {
    return atmosphereSample(altitude_m).temperature_k;
}

double Environment::airPressurePa(double altitude_m) const noexcept {
    return atmosphereSample(altitude_m).pressure_pa;
}

double Environment::airDensityKgPerM3(double altitude_m) const noexcept {
    return atmosphereSample(altitude_m).density_kgpm3;
}

double Environment::speedOfSoundMps(double altitude_m) const noexcept {
    return atmosphereSample(altitude_m).speed_of_sound_mps;
}

double Environment::gravityMps2(double altitude_m) const noexcept {
    return atmosphereSample(altitude_m).gravity_mps2;
}

Vector3 Environment::windVelocityWorldMps(double altitude_m, double time_s) const noexcept {
    return windSample(altitude_m, time_s).velocity_world_mps;
}

EnvironmentCacheStats Environment::cacheStats() const noexcept {
    const auto count_valid = [](const auto& entries) {
        return static_cast<std::size_t>(std::count_if(
            entries.begin(),
            entries.end(),
            [](const auto& entry) {
                return entry.valid;
            }));
    };

    EnvironmentCacheUsageStats atmosphere_stats = atmosphere_cache_stats_;
    atmosphere_stats.l2_valid_entries = count_valid(atmosphere_l2_);

    EnvironmentCacheUsageStats wind_stats = wind_cache_stats_;
    wind_stats.l2_valid_entries = count_valid(wind_l2_);

    return EnvironmentCacheStats {
        .atmosphere = atmosphere_stats,
        .wind = wind_stats
    };
}

const Environment::AtmosphereCacheEntry& Environment::atmosphereSample(double altitude_m) const noexcept {
    if (atmosphere_l1_.valid && atmosphere_l1_.altitude_m == altitude_m) {
        ++atmosphere_cache_stats_.l1_hits;
        return atmosphere_l1_;
    }

    for (const auto& cached : atmosphere_l2_) {
        if (cached.valid && cached.altitude_m == altitude_m) {
            atmosphere_l1_ = cached;
            ++atmosphere_cache_stats_.l2_hits;
            return atmosphere_l1_;
        }
    }

    const double relative_altitude_m = std::max(altitude_m, 0.0);
    const double surface_temperature_k = surface_weather_.temperature_c + 273.15;
    const double temperature_k = std::max(
        180.0,
        surface_temperature_k - physical_constants::lapse_rate_k_per_m * relative_altitude_m);
    const double clamped_surface_pressure_pa =
        std::max(surface_weather_.pressure_hpa * 100.0, 1000.0);
    const double base = 1.0 - (physical_constants::lapse_rate_k_per_m * relative_altitude_m) / surface_temperature_k;
    const double exponent =
        (physical_constants::gravitational_acceleration * physical_constants::molar_mass_dry_air) /
        (physical_constants::universal_gas_constant * physical_constants::lapse_rate_k_per_m);
    const double pressure_pa = clamped_surface_pressure_pa * std::pow(std::max(base, 1e-6), exponent);
    const double relative_humidity = humidityClampedPercent(surface_weather_.humidity_percent) * 0.01;
    const double vapor_pressure_pa =
        relative_humidity * saturationVaporPressurePa(temperature_k - 273.15);
    const double dry_air_pressure_pa = std::max(pressure_pa - vapor_pressure_pa, 0.0);
    const double density_kgpm3 =
        (dry_air_pressure_pa / (physical_constants::specific_gas_constant_dry_air * temperature_k)) +
        (vapor_pressure_pa / (physical_constants::specific_gas_constant_water_vapor * temperature_k));
    const double speed_of_sound_mps = std::sqrt(
        physical_constants::ratio_of_specific_heats_air *
        physical_constants::specific_gas_constant_dry_air *
        temperature_k);
    const double latitude_rad = degToRad(launch_site_.latitude_deg);
    const double sin_lat = std::sin(latitude_rad);
    const double sin2 = sin_lat * sin_lat;
    const double surface_gravity =
        9.780327 * (1.0 + 0.0053024 * sin2 - 0.0000058 * std::sin(2.0 * latitude_rad) * std::sin(2.0 * latitude_rad));
    const double radius = earth_radius_m + launch_site_.elevation_m + relative_altitude_m;
    const double gravity_mps2 = surface_gravity * std::pow(earth_radius_m / radius, 2.0);
    atmosphere_l1_ = AtmosphereCacheEntry {
        .altitude_m = altitude_m,
        .temperature_k = temperature_k,
        .pressure_pa = pressure_pa,
        .density_kgpm3 = density_kgpm3,
        .speed_of_sound_mps = speed_of_sound_mps,
        .gravity_mps2 = gravity_mps2,
        .valid = true
    };
    atmosphere_l2_[atmosphere_l2_next_slot_] = atmosphere_l1_;
    atmosphere_l2_next_slot_ = (atmosphere_l2_next_slot_ + 1U) % atmosphere_l2_.size();
    ++atmosphere_cache_stats_.misses;
    ++atmosphere_cache_stats_.writes;
    return atmosphere_l1_;
}

const Environment::WindCacheEntry& Environment::windSample(double altitude_m, double time_s) const noexcept {
    if (wind_l1_.valid && wind_l1_.altitude_m == altitude_m && wind_l1_.time_s == time_s) {
        ++wind_cache_stats_.l1_hits;
        return wind_l1_;
    }

    for (const auto& cached : wind_l2_) {
        if (cached.valid && cached.altitude_m == altitude_m && cached.time_s == time_s) {
            wind_l1_ = cached;
            ++wind_cache_stats_.l2_hits;
            return wind_l1_;
        }
    }

    const double relative_altitude_m = std::max(altitude_m, 0.0);
    const WindLayerAnchor layer = interpolatedWindLayer(relative_altitude_m);
    const double base_direction_deg = wrapDegrees360(surface_weather_.wind_direction_deg);
    const double surface_wind_speed_mps = std::max(surface_weather_.wind_speed_mps, 0.0);
    const double gust_excess_mps =
        std::max(surface_weather_.wind_gust_mps - surface_wind_speed_mps, 0.0);
    const double boundary_layer_scale = std::clamp(
        std::pow(std::max((relative_altitude_m + 20.0) / 20.0, 0.3), 0.08),
        0.92,
        1.14);
    const double altitude_band_strength =
        smoothstep01(std::min(relative_altitude_m / 1600.0, 1.0));
    const double gust_wave =
        gust_excess_mps *
        (0.18 + 0.12 * altitude_band_strength) *
        std::sin(0.19 * time_s + relative_altitude_m * 0.009);
    const double meander_wave =
        surface_wind_speed_mps *
        (0.03 + 0.04 * altitude_band_strength) *
        std::sin(0.05 * time_s + relative_altitude_m * 0.0017 + 1.3);
    const double directional_wave_deg =
        (2.0 + 4.0 * altitude_band_strength + 0.25 * gust_excess_mps) *
        std::sin(0.07 * time_s + relative_altitude_m * 0.0028);
    const double wind_speed_mps = std::max(
        0.0,
        surface_wind_speed_mps * layer.speed_scale * boundary_layer_scale +
            gust_wave +
            meander_wave);
    const double direction_rad = degToRad(
        wrapDegrees360(base_direction_deg + layer.direction_offset_deg + directional_wave_deg));

    wind_l1_ = WindCacheEntry {
        .altitude_m = altitude_m,
        .time_s = time_s,
        .velocity_world_mps = {
            -wind_speed_mps * std::sin(direction_rad),
            -wind_speed_mps * std::cos(direction_rad),
            0.0
        },
        .valid = true
    };
    wind_l2_[wind_l2_next_slot_] = wind_l1_;
    wind_l2_next_slot_ = (wind_l2_next_slot_ + 1U) % wind_l2_.size();
    ++wind_cache_stats_.misses;
    ++wind_cache_stats_.writes;
    return wind_l1_;
}

void Environment::invalidateCaches() noexcept {
    atmosphere_l1_ = {};
    wind_l1_ = {};
    atmosphere_l2_.fill({});
    wind_l2_.fill({});
    atmosphere_l2_next_slot_ = 0;
    wind_l2_next_slot_ = 0;
    atmosphere_cache_stats_ = EnvironmentCacheUsageStats {.l2_capacity = 16};
    wind_cache_stats_ = EnvironmentCacheUsageStats {.l2_capacity = 16};
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
