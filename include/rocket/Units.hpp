#pragma once

namespace rocket {

template <typename Tag>
struct StrongValue {
    double value {};

    [[nodiscard]] constexpr explicit operator double() const noexcept {
        return value;
    }
};

struct SecondsTag;
struct MetersTag;
struct KilogramsTag;
struct NewtonTag;

using Seconds = StrongValue<SecondsTag>;
using Meters = StrongValue<MetersTag>;
using Kilograms = StrongValue<KilogramsTag>;
using Newton = StrongValue<NewtonTag>;

[[nodiscard]] constexpr double valueOf(Seconds value) noexcept {
    return value.value;
}

[[nodiscard]] constexpr double valueOf(Meters value) noexcept {
    return value.value;
}

[[nodiscard]] constexpr double valueOf(Kilograms value) noexcept {
    return value.value;
}

[[nodiscard]] constexpr double valueOf(Newton value) noexcept {
    return value.value;
}

}  // namespace rocket
