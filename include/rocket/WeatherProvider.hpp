#pragma once

#include <expected>
#include <string>

#include "rocket/Environment.hpp"

namespace rocket {

struct WeatherFetchInfo {
    SurfaceWeather weather {};
    std::string provider_name;
    std::string query_url;
};

[[nodiscard]] std::string weatherSourceLabel(WeatherDataSource source);

[[nodiscard]] std::expected<WeatherFetchInfo, std::string> fetchSurfaceWeather(
    const LaunchSite& site,
    WeatherDataSource source);

[[nodiscard]] std::expected<WeatherFetchInfo, std::string> refreshEnvironmentWeather(
    Environment& environment);

}  // namespace rocket
