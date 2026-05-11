#pragma once

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

enum class WeatherDataSource {
    Manual,
    OpenMeteoReady,
    OpenWeatherMapReady
};

class Environment {
public:
    [[nodiscard]] const LaunchSite& launchSite() const noexcept;
    [[nodiscard]] const SurfaceWeather& surfaceWeather() const noexcept;
    [[nodiscard]] WeatherDataSource weatherDataSource() const noexcept;

    void setLaunchSite(const LaunchSite& site) noexcept;
    void setSurfaceWeather(const SurfaceWeather& weather) noexcept;
    void setWeatherDataSource(WeatherDataSource source) noexcept;

    [[nodiscard]] double airDensityKgPerM3(double altitude_m) const noexcept;
    [[nodiscard]] double airPressurePa(double altitude_m) const noexcept;
    [[nodiscard]] double airTemperatureK(double altitude_m) const noexcept;
    [[nodiscard]] double speedOfSoundMps(double altitude_m) const noexcept;
    [[nodiscard]] double gravityMps2(double altitude_m) const noexcept;
    [[nodiscard]] Vector3 windVelocityWorldMps(double altitude_m, double time_s) const noexcept;
    [[nodiscard]] std::string weatherApiQueryUrl() const;

private:
    LaunchSite launch_site_ {};
    SurfaceWeather surface_weather_ {};
    WeatherDataSource weather_data_source_ {WeatherDataSource::Manual};
};

}  // namespace rocket
