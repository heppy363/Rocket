#include "rocket/Validation.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <ranges>

namespace rocket {

namespace {

[[nodiscard]] ValidationError makeError(
    ValidationErrorCode code,
    std::string message) {
    return ValidationError {
        .code = code,
        .message = std::move(message)
    };
}

[[nodiscard]] bool finiteVector(const Vector3& value) noexcept {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

[[nodiscard]] bool finiteQuaternion(const Quaternion& value) noexcept {
    return std::isfinite(value.w) && std::isfinite(value.x) &&
           std::isfinite(value.y) && std::isfinite(value.z);
}

}  // namespace

ValidationResult validateVehicleModel(const VehicleModel& vehicle) {
    if (!std::isfinite(vehicle.dry_mass_kg) || vehicle.dry_mass_kg <= 0.0) {
        return std::unexpected(makeError(
            ValidationErrorCode::InvalidMass,
            "Vehicle dry mass must be finite and > 0 kg."));
    }
    if (!std::isfinite(vehicle.reference_area_m2) || vehicle.reference_area_m2 <= 0.0) {
        return std::unexpected(makeError(
            ValidationErrorCode::InvalidReferenceArea,
            "Vehicle reference area must be finite and > 0 m^2."));
    }
    if (!finiteVector(vehicle.principal_inertia_kgm2) ||
        vehicle.principal_inertia_kgm2.x <= 0.0 ||
        vehicle.principal_inertia_kgm2.y <= 0.0 ||
        vehicle.principal_inertia_kgm2.z <= 0.0) {
        return std::unexpected(makeError(
            ValidationErrorCode::InvalidInertia,
            "Principal inertia must be finite and strictly positive on all axes."));
    }

    const auto& motors = vehicle.cluster.mountedMotors();
    const bool invalid_motor = std::ranges::any_of(motors, [](const MountedMotor& mounted_motor) {
        return !std::isfinite(mounted_motor.motor.max_thrust_n) ||
               mounted_motor.motor.max_thrust_n < 0.0 ||
               !std::isfinite(mounted_motor.motor.burn_time_s) ||
               mounted_motor.motor.burn_time_s < 0.0 ||
               !std::isfinite(mounted_motor.motor.propellant_mass_kg) ||
               mounted_motor.motor.propellant_mass_kg < 0.0 ||
               !finiteVector(mounted_motor.mount_position_m) ||
               !finiteVector(mounted_motor.thrust_direction_body);
    });
    if (invalid_motor) {
        return std::unexpected(makeError(
            ValidationErrorCode::InvalidMotor,
            "Motor cluster contains non-finite values or negative thrust/burn/mass parameters."));
    }

    return {};
}

ValidationResult validateEnvironment(const Environment& environment) {
    const LaunchSite& site = environment.launchSite();
    if (!std::isfinite(site.latitude_deg) || !std::isfinite(site.longitude_deg) ||
        !std::isfinite(site.elevation_m)) {
        return std::unexpected(makeError(
            ValidationErrorCode::InvalidLaunchSite,
            "Launch site coordinates must be finite."));
    }

    const SurfaceWeather& weather = environment.surfaceWeather();
    const auto finite_weather_values = std::array {
        weather.pressure_hpa,
        weather.temperature_c,
        weather.humidity_percent,
        weather.wind_speed_mps,
        weather.wind_direction_deg,
        weather.wind_gust_mps
    };
    const bool invalid_weather =
        std::ranges::any_of(finite_weather_values, [](double value) { return !std::isfinite(value); });
    if (invalid_weather || weather.pressure_hpa <= 0.0) {
        return std::unexpected(makeError(
            ValidationErrorCode::InvalidWeather,
            "Surface weather must be finite and use a positive pressure."));
    }

    return {};
}

ValidationResult validateFlightState(const FlightState& state, const VehicleModel& vehicle) {
    if (!finiteVector(state.position_m) ||
        !finiteVector(state.velocity_mps) ||
        !finiteVector(state.angular_velocity_body_radps) ||
        !finiteQuaternion(state.attitude_body_to_world)) {
        return std::unexpected(makeError(
            ValidationErrorCode::InvalidState,
            "Flight state contains non-finite position, velocity, attitude or angular velocity."));
    }
    if (!std::isfinite(state.mass_kg) || state.mass_kg < vehicle.dry_mass_kg) {
        return std::unexpected(makeError(
            ValidationErrorCode::InvalidMass,
            "Flight state mass must be finite and not smaller than vehicle dry mass."));
    }
    return {};
}

ValidationResult validateSimulationInputs(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    Seconds dt) {
    if (!std::isfinite(valueOf(dt)) || valueOf(dt) <= 0.0) {
        return std::unexpected(makeError(
            ValidationErrorCode::InvalidTimeStep,
            "Simulation time step must be finite and > 0 seconds."));
    }
    if (ValidationResult vehicle_result = validateVehicleModel(vehicle); !vehicle_result) {
        return vehicle_result;
    }
    if (ValidationResult environment_result = validateEnvironment(environment); !environment_result) {
        return environment_result;
    }
    return validateFlightState(state, vehicle);
}

}  // namespace rocket
