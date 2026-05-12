#pragma once

#include <concepts>

#include "rocket/FlightState.hpp"

namespace rocket {

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <typename Evaluator>
concept StateDerivativeEvaluator =
    requires(Evaluator evaluator, const FlightState& state, double time_s) {
        { evaluator(state, time_s) } -> std::same_as<StateDerivative>;
    };

}  // namespace rocket
