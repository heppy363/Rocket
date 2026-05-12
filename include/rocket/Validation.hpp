#pragma once

#include <expected>
#include <string>

#include "rocket/Environment.hpp"
#include "rocket/FlightState.hpp"
#include "rocket/Units.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket {

enum class ValidationErrorCode {
    InvalidTimeStep,
    InvalidMass,
    InvalidReferenceArea,
    InvalidInertia,
    InvalidState,
    InvalidLaunchSite,
    InvalidWeather,
    InvalidMotor
};

struct ValidationError {
    ValidationErrorCode code {ValidationErrorCode::InvalidState};
    std::string message;
};

using ValidationResult = std::expected<void, ValidationError>;

[[nodiscard]] ValidationResult validateVehicleModel(const VehicleModel& vehicle);
[[nodiscard]] ValidationResult validateEnvironment(const Environment& environment);
[[nodiscard]] ValidationResult validateFlightState(const FlightState& state, const VehicleModel& vehicle);
[[nodiscard]] ValidationResult validateSimulationInputs(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    Seconds dt);

}  // namespace rocket
