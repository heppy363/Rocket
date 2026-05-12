#include "rocket/WeatherProvider.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <format>
#include <optional>
#include <string_view>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

namespace rocket {

namespace {

std::string trim(std::string_view text) {
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos) {
        return {};
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    return std::string(text.substr(begin, end - begin + 1));
}

std::optional<std::string_view> extractObject(std::string_view json, std::string_view key) {
    const std::string pattern = std::format("\"{}\"", key);
    const std::size_t key_pos = json.find(pattern);
    if (key_pos == std::string_view::npos) {
        return std::nullopt;
    }
    const std::size_t open_pos = json.find('{', key_pos + pattern.size());
    if (open_pos == std::string_view::npos) {
        return std::nullopt;
    }

    int depth = 0;
    for (std::size_t index = open_pos; index < json.size(); ++index) {
        if (json[index] == '{') {
            ++depth;
        } else if (json[index] == '}') {
            --depth;
            if (depth == 0) {
                return json.substr(open_pos, index - open_pos + 1);
            }
        }
    }
    return std::nullopt;
}

std::optional<double> extractNumber(std::string_view json, std::string_view key) {
    const std::string pattern = std::format("\"{}\"", key);
    const std::size_t key_pos = json.find(pattern);
    if (key_pos == std::string_view::npos) {
        return std::nullopt;
    }
    const std::size_t colon_pos = json.find(':', key_pos + pattern.size());
    if (colon_pos == std::string_view::npos) {
        return std::nullopt;
    }

    std::size_t value_begin = colon_pos + 1;
    while (value_begin < json.size() &&
           std::isspace(static_cast<unsigned char>(json[value_begin])) != 0) {
        ++value_begin;
    }

    std::size_t value_end = value_begin;
    while (value_end < json.size()) {
        const char ch = json[value_end];
        if (!(std::isdigit(static_cast<unsigned char>(ch)) != 0 ||
              ch == '-' || ch == '+' || ch == '.' || ch == 'e' || ch == 'E')) {
            break;
        }
        ++value_end;
    }

    if (value_begin == value_end) {
        return std::nullopt;
    }

    const std::string numeric = trim(json.substr(value_begin, value_end - value_begin));
    char* parsed_end = nullptr;
    const double value = std::strtod(numeric.c_str(), &parsed_end);
    if (parsed_end == numeric.c_str() || *parsed_end != '\0') {
        return std::nullopt;
    }
    return value;
}

std::expected<SurfaceWeather, std::string> parseOpenMeteoWeather(std::string_view json) {
    const auto current = extractObject(json, "current");
    if (!current.has_value()) {
        return std::unexpected("Open-Meteo response does not contain a 'current' object.");
    }

    SurfaceWeather weather {};
    const auto temperature = extractNumber(*current, "temperature_2m");
    const auto humidity = extractNumber(*current, "relative_humidity_2m");
    const auto pressure = extractNumber(*current, "surface_pressure");
    const auto wind_speed = extractNumber(*current, "wind_speed_10m");
    const auto wind_direction = extractNumber(*current, "wind_direction_10m");
    const auto wind_gust = extractNumber(*current, "wind_gusts_10m");
    if (!temperature || !humidity || !pressure || !wind_speed || !wind_direction) {
        return std::unexpected("Open-Meteo response is missing one or more required weather fields.");
    }

    weather.temperature_c = *temperature;
    weather.humidity_percent = std::clamp(*humidity, 0.0, 100.0);
    weather.pressure_hpa = *pressure;
    weather.wind_speed_mps = std::max(*wind_speed / 3.6, 0.0);
    weather.wind_direction_deg = std::fmod(*wind_direction + 360.0, 360.0);
    weather.wind_gust_mps = wind_gust.has_value() ? std::max(*wind_gust / 3.6, 0.0) : weather.wind_speed_mps;
    return weather;
}

std::expected<SurfaceWeather, std::string> parseOpenWeatherMapWeather(std::string_view json) {
    const auto current = extractObject(json, "current");
    if (!current.has_value()) {
        return std::unexpected("OpenWeatherMap response does not contain a 'current' object.");
    }

    SurfaceWeather weather {};
    const auto temperature = extractNumber(*current, "temp");
    const auto humidity = extractNumber(*current, "humidity");
    const auto pressure = extractNumber(*current, "pressure");
    const auto wind_speed = extractNumber(*current, "wind_speed");
    const auto wind_direction = extractNumber(*current, "wind_deg");
    const auto wind_gust = extractNumber(*current, "gust");
    if (!temperature || !humidity || !pressure || !wind_speed || !wind_direction) {
        return std::unexpected("OpenWeatherMap response is missing one or more required weather fields.");
    }

    weather.temperature_c = *temperature;
    weather.humidity_percent = std::clamp(*humidity, 0.0, 100.0);
    weather.pressure_hpa = *pressure;
    weather.wind_speed_mps = std::max(*wind_speed, 0.0);
    weather.wind_direction_deg = std::fmod(*wind_direction + 360.0, 360.0);
    weather.wind_gust_mps = wind_gust.has_value() ? std::max(*wind_gust, 0.0) : weather.wind_speed_mps;
    return weather;
}

std::expected<std::string, std::string> weatherQueryUrl(
    const LaunchSite& site,
    WeatherDataSource source) {
    switch (source) {
    case WeatherDataSource::Manual:
        return std::unexpected("Manual weather source does not support remote fetch.");
    case WeatherDataSource::OpenMeteoReady:
        return std::format(
            "https://api.open-meteo.com/v1/forecast?latitude={:.6f}&longitude={:.6f}&current=temperature_2m,relative_humidity_2m,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m",
            site.latitude_deg,
            site.longitude_deg);
    case WeatherDataSource::OpenWeatherMapReady: {
        const char* api_key = std::getenv("OPENWEATHERMAP_API_KEY");
        if (api_key == nullptr || *api_key == '\0') {
            return std::unexpected("OpenWeatherMap requires env var OPENWEATHERMAP_API_KEY.");
        }
        return std::format(
            "https://api.openweathermap.org/data/3.0/onecall?lat={:.6f}&lon={:.6f}&units=metric&exclude=minutely,hourly,daily,alerts&appid={}",
            site.latitude_deg,
            site.longitude_deg,
            api_key);
    }
    }
    return std::unexpected("Unsupported weather source.");
}

#ifdef _WIN32
std::expected<std::string, std::string> httpGet(std::string_view url) {
    std::wstring wide_url(url.begin(), url.end());

    URL_COMPONENTS url_components {};
    url_components.dwStructSize = sizeof(url_components);
    url_components.dwSchemeLength = static_cast<DWORD>(-1);
    url_components.dwHostNameLength = static_cast<DWORD>(-1);
    url_components.dwUrlPathLength = static_cast<DWORD>(-1);
    url_components.dwExtraInfoLength = static_cast<DWORD>(-1);
    if (!WinHttpCrackUrl(wide_url.c_str(), 0, 0, &url_components)) {
        return std::unexpected(std::format("WinHttpCrackUrl failed with code {}.", GetLastError()));
    }

    const std::wstring host(url_components.lpszHostName, url_components.dwHostNameLength);
    const std::wstring path =
        std::wstring(url_components.lpszUrlPath, url_components.dwUrlPathLength) +
        std::wstring(url_components.lpszExtraInfo, url_components.dwExtraInfoLength);
    const bool use_https = url_components.nScheme == INTERNET_SCHEME_HTTPS;

    HINTERNET session = WinHttpOpen(
        L"RocketLab/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (session == nullptr) {
        return std::unexpected(std::format("WinHttpOpen failed with code {}.", GetLastError()));
    }

    HINTERNET connection = WinHttpConnect(session, host.c_str(), url_components.nPort, 0);
    if (connection == nullptr) {
        const DWORD error = GetLastError();
        WinHttpCloseHandle(session);
        return std::unexpected(std::format("WinHttpConnect failed with code {}.", error));
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        use_https ? WINHTTP_FLAG_SECURE : 0);
    if (request == nullptr) {
        const DWORD error = GetLastError();
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return std::unexpected(std::format("WinHttpOpenRequest failed with code {}.", error));
    }

    std::expected<std::string, std::string> result = std::unexpected("HTTP request failed.");
    if (!WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        result = std::unexpected(std::format("WinHttpSendRequest failed with code {}.", GetLastError()));
    } else if (!WinHttpReceiveResponse(request, nullptr)) {
        result = std::unexpected(std::format("WinHttpReceiveResponse failed with code {}.", GetLastError()));
    } else {
        DWORD status_code = 0;
        DWORD status_size = sizeof(status_code);
        if (!WinHttpQueryHeaders(
                request,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX,
                &status_code,
                &status_size,
                WINHTTP_NO_HEADER_INDEX)) {
            result = std::unexpected(std::format("WinHttpQueryHeaders failed with code {}.", GetLastError()));
        } else if (status_code < 200 || status_code >= 300) {
            result = std::unexpected(std::format("HTTP request returned status {}.", status_code));
        } else {
            std::string body;
            for (;;) {
                DWORD bytes_available = 0;
                if (!WinHttpQueryDataAvailable(request, &bytes_available)) {
                    result = std::unexpected(std::format("WinHttpQueryDataAvailable failed with code {}.", GetLastError()));
                    break;
                }
                if (bytes_available == 0) {
                    result = body;
                    break;
                }

                std::string chunk(bytes_available, '\0');
                DWORD bytes_read = 0;
                if (!WinHttpReadData(request, chunk.data(), bytes_available, &bytes_read)) {
                    result = std::unexpected(std::format("WinHttpReadData failed with code {}.", GetLastError()));
                    break;
                }
                chunk.resize(bytes_read);
                body += chunk;
            }
        }
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);
    return result;
}
#else
std::expected<std::string, std::string> httpGet(std::string_view) {
    return std::unexpected("Live weather fetch is currently implemented only on Windows.");
}
#endif

}  // namespace

std::string weatherSourceLabel(WeatherDataSource source) {
    switch (source) {
    case WeatherDataSource::Manual:
        return "Manual";
    case WeatherDataSource::OpenMeteoReady:
        return "Open-Meteo Live";
    case WeatherDataSource::OpenWeatherMapReady:
        return "OpenWeatherMap Live";
    }
    return "Manual";
}

std::expected<WeatherFetchInfo, std::string> fetchSurfaceWeather(
    const LaunchSite& site,
    WeatherDataSource source) {
    const auto query_url = weatherQueryUrl(site, source);
    if (!query_url) {
        return std::unexpected(query_url.error());
    }

    const auto response_body = httpGet(*query_url);
    if (!response_body) {
        return std::unexpected(response_body.error());
    }

    std::expected<SurfaceWeather, std::string> parsed_weather = std::unexpected("Unsupported weather source.");
    switch (source) {
    case WeatherDataSource::Manual:
        return std::unexpected("Manual weather source does not support remote fetch.");
    case WeatherDataSource::OpenMeteoReady:
        parsed_weather = parseOpenMeteoWeather(*response_body);
        break;
    case WeatherDataSource::OpenWeatherMapReady:
        parsed_weather = parseOpenWeatherMapWeather(*response_body);
        break;
    }
    if (!parsed_weather) {
        return std::unexpected(parsed_weather.error());
    }

    return WeatherFetchInfo {
        .weather = *parsed_weather,
        .provider_name = weatherSourceLabel(source),
        .query_url = *query_url
    };
}

std::expected<WeatherFetchInfo, std::string> refreshEnvironmentWeather(Environment& environment) {
    const auto fetched = fetchSurfaceWeather(environment.launchSite(), environment.weatherDataSource());
    if (!fetched) {
        return std::unexpected(fetched.error());
    }
    environment.setSurfaceWeather(fetched->weather);
    return fetched;
}

}  // namespace rocket
