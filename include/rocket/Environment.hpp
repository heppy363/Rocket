#pragma once

#include <array>
#include <string>

#include "rocket/MathTypes.hpp"

namespace rocket {

struct LaunchSite {
    double latitude_deg {45.4642};
    double longitude_deg {9.19};
    double elevation_m {120.0};
};

struct SurfaceWeather {
    double pressure_hpa {1013.25};
    double temperature_c {15.0};
    double humidity_percent {55.0};
    double wind_speed_mps {2.0};
    double wind_direction_deg {0.0};
    double wind_gust_mps {0.0};
};

struct LaunchRail {
    bool enabled {true};
    double rail_length_m {5.0};
};

enum class WeatherDataSource {
    Manual,
    OpenMeteoReady,
    OpenWeatherMapReady
};

struct EnvironmentCacheUsageStats {
    std::size_t l2_capacity {};
    std::size_t l2_valid_entries {};
    std::size_t l1_hits {};
    std::size_t l2_hits {};
    std::size_t misses {};
    std::size_t writes {};
};

struct EnvironmentCacheStats {
    EnvironmentCacheUsageStats atmosphere {};
    EnvironmentCacheUsageStats wind {};
};

class Environment {
public:
    [[nodiscard]] const LaunchSite& launchSite() const noexcept;
    [[nodiscard]] const SurfaceWeather& surfaceWeather() const noexcept;
    [[nodiscard]] const LaunchRail& launchRail() const noexcept;
    [[nodiscard]] WeatherDataSource weatherDataSource() const noexcept;

    void setLaunchSite(const LaunchSite& site) noexcept;
    void setSurfaceWeather(const SurfaceWeather& weather) noexcept;
    void setLaunchRail(const LaunchRail& rail) noexcept;
    void setWeatherDataSource(WeatherDataSource source) noexcept;

    [[nodiscard]] double airDensityKgPerM3(double altitude_m) const noexcept;
    [[nodiscard]] double airPressurePa(double altitude_m) const noexcept;
    [[nodiscard]] double airTemperatureK(double altitude_m) const noexcept;
    [[nodiscard]] double speedOfSoundMps(double altitude_m) const noexcept;
    [[nodiscard]] double gravityMps2(double altitude_m) const noexcept;
    [[nodiscard]] Vector3 windVelocityWorldMps(double altitude_m, double time_s) const noexcept;
    [[nodiscard]] std::string weatherApiQueryUrl() const;
    [[nodiscard]] EnvironmentCacheStats cacheStats() const noexcept;

private:
    struct AtmosphereCacheEntry {
        double altitude_m {};
        double temperature_k {};
        double pressure_pa {};
        double density_kgpm3 {};
        double speed_of_sound_mps {};
        double gravity_mps2 {};
        bool valid {false};
    };

    struct WindCacheEntry {
        double altitude_m {};
        double time_s {};
        Vector3 velocity_world_mps {};
        bool valid {false};
    };

    [[nodiscard]] const AtmosphereCacheEntry& atmosphereSample(double altitude_m) const noexcept;
    [[nodiscard]] const WindCacheEntry& windSample(double altitude_m, double time_s) const noexcept;
    void invalidateCaches() noexcept;

    LaunchSite launch_site_ {};
    SurfaceWeather surface_weather_ {};
    LaunchRail launch_rail_ {};
    WeatherDataSource weather_data_source_ {WeatherDataSource::Manual};
    mutable AtmosphereCacheEntry atmosphere_l1_ {};
    mutable WindCacheEntry wind_l1_ {};
    mutable std::array<AtmosphereCacheEntry, 16> atmosphere_l2_ {};
    mutable std::array<WindCacheEntry, 16> wind_l2_ {};
    mutable std::size_t atmosphere_l2_next_slot_ {};
    mutable std::size_t wind_l2_next_slot_ {};
    mutable EnvironmentCacheUsageStats atmosphere_cache_stats_ {
        .l2_capacity = 16
    };
    mutable EnvironmentCacheUsageStats wind_cache_stats_ {
        .l2_capacity = 16
    };
};

}  // namespace rocket
