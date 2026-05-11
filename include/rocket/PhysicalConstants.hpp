#pragma once

namespace rocket::physical_constants {

constexpr double gravitational_acceleration = 9.80665;
constexpr double simulation_frequency_hz = 1000.0;
constexpr double standard_air_density = 1.225;
constexpr double atmosphere_scale_height_m = 8500.0;
constexpr double standard_pressure_pa = 101325.0;
constexpr double standard_temperature_k = 288.15;
constexpr double lapse_rate_k_per_m = 0.0065;
constexpr double universal_gas_constant = 8.314462618;
constexpr double molar_mass_dry_air = 0.0289644;
constexpr double specific_gas_constant_dry_air = 287.05;
constexpr double specific_gas_constant_water_vapor = 461.495;
constexpr double ratio_of_specific_heats_air = 1.4;

}  // namespace rocket::physical_constants
