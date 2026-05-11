#pragma once

#include <algorithm>
#include <cmath>

namespace rocket::secure {

template <typename T>
[[nodiscard]] constexpr T clampToRange(T value, T min_value, T max_value) noexcept {
    return std::clamp(value, min_value, max_value);
}

[[nodiscard]] inline double clampFinite(double value, double fallback, double min_value, double max_value) noexcept {
    if (!std::isfinite(value)) {
        return fallback;
    }
    return std::clamp(value, min_value, max_value);
}

[[nodiscard]] inline int clampFiniteInt(int value, int fallback, int min_value, int max_value) noexcept {
    return std::clamp(value, min_value, max_value);
}

}  // namespace rocket::secure
