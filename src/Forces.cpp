#include "rocket/Aerodynamics.hpp"
#include "rocket/CfdModule.hpp"
#include "rocket/Forces.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

#include "rocket/PhysicalConstants.hpp"

namespace rocket {

namespace {

double dragCoefficientAtMach(
    const VehicleModel& vehicle,
    double mach_number) noexcept {
    const double base_cd = vehicle.aerodynamic_coefficients.drag_coefficient;
    if (mach_number < 0.8) {
        return base_cd;
    }
    if (mach_number < 1.05) {
        const double t = (mach_number - 0.8) / 0.25;
        return base_cd + 0.18 * t;
    }
    if (mach_number < 1.6) {
        const double t = (mach_number - 1.05) / 0.55;
        return base_cd + 0.18 * (1.0 - 0.6 * t);
    }
    return base_cd + 0.05;
}

Vector3 computeDragForce(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    const Vector3& relative_air_velocity_world_mps,
    double* dynamic_pressure_pa,
    double* mach_number) noexcept {
    const double speed_mps = relative_air_velocity_world_mps.magnitude();
    if (speed_mps <= 0.0) {
        if (dynamic_pressure_pa != nullptr) {
            *dynamic_pressure_pa = 0.0;
        }
        if (mach_number != nullptr) {
            *mach_number = 0.0;
        }
        return {};
    }

    const double air_density = environment.airDensityKgPerM3(state.position_m.z);
    const double dynamic_pressure = 0.5 * air_density * speed_mps * speed_mps;
    const double speed_of_sound = std::max(environment.speedOfSoundMps(state.position_m.z), 1.0);
    const double mach = speed_mps / speed_of_sound;
    const double drag_magnitude_n =
        dynamic_pressure * dragCoefficientAtMach(vehicle, mach) * vehicle.reference_area_m2;

    if (dynamic_pressure_pa != nullptr) {
        *dynamic_pressure_pa = dynamic_pressure;
    }
    if (mach_number != nullptr) {
        *mach_number = mach;
    }

    return -drag_magnitude_n * relative_air_velocity_world_mps.normalized();
}

Vector3 computeGravityForce(const FlightState& state, const Environment& environment) noexcept {
    return {0.0, 0.0, -state.mass_kg * environment.gravityMps2(state.position_m.z)};
}

Vector3 computeAngularAccelerationBody(
    const Vector3& moment_body_nm,
    const Vector3& angular_velocity_body_radps,
    const Vector3& principal_inertia_kgm2) noexcept {
    const Vector3 angular_momentum_body {
        principal_inertia_kgm2.x * angular_velocity_body_radps.x,
        principal_inertia_kgm2.y * angular_velocity_body_radps.y,
        principal_inertia_kgm2.z * angular_velocity_body_radps.z
    };
    const Vector3 gyroscopic_term = cross(angular_velocity_body_radps, angular_momentum_body);
    const Vector3 net_rotational_term = moment_body_nm - gyroscopic_term;

    return {
        principal_inertia_kgm2.x > 0.0 ? net_rotational_term.x / principal_inertia_kgm2.x : 0.0,
        principal_inertia_kgm2.y > 0.0 ? net_rotational_term.y / principal_inertia_kgm2.y : 0.0,
        principal_inertia_kgm2.z > 0.0 ? net_rotational_term.z / principal_inertia_kgm2.z : 0.0
    };
}

Quaternion computeAttitudeRate(
    const Quaternion& attitude_body_to_world,
    const Vector3& angular_velocity_body_radps) noexcept {
    const Quaternion omega_body {0.0,
                                 angular_velocity_body_radps.x,
                                 angular_velocity_body_radps.y,
                                 angular_velocity_body_radps.z};
    return 0.5 * (attitude_body_to_world * omega_body);
}

Vector3 inverseRotateVector(const Quaternion& rotation, const Vector3& vector) noexcept {
    return rotateVector(rotation.conjugate(), vector);
}

Vector3 projectOntoAxis(const Vector3& vector, const Vector3& axis_normalized) noexcept {
    return dot(vector, axis_normalized) * axis_normalized;
}

bool isOnLaunchRail(
    const FlightState& state,
    const Environment& environment) noexcept {
    const LaunchRail& rail = environment.launchRail();
    if (!rail.enabled || rail.rail_length_m <= 1e-6) {
        return false;
    }

    const Vector3 rail_axis_world =
        rotateVector(state.attitude_body_to_world, {0.0, 0.0, 1.0}).normalized();
    const double axial_position_m = dot(state.position_m, rail_axis_world);
    const Vector3 radial_offset_m = state.position_m - projectOntoAxis(state.position_m, rail_axis_world);
    return axial_position_m >= -0.05 &&
           axial_position_m < rail.rail_length_m &&
           radial_offset_m.magnitude() <= 0.15;
}

Vector3 computeAerodynamicNormalForce(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    const Vector3& relative_air_velocity_world_mps,
    double* dynamic_pressure_out_pa,
    double* angle_of_attack_rad) noexcept {
    const AerodynamicFrame frame = computeAerodynamicFrame(state, relative_air_velocity_world_mps);
    if (angle_of_attack_rad != nullptr) {
        *angle_of_attack_rad = frame.angle_of_attack_rad;
    }
    if (frame.speed_mps <= 1e-6 || frame.lateral_air_direction_world.magnitude() <= 1e-6) {
        return {};
    }

    const double air_density = environment.airDensityKgPerM3(state.position_m.z);
    const double dynamic_pressure_pa = 0.5 * air_density * frame.speed_mps * frame.speed_mps;
    if (dynamic_pressure_out_pa != nullptr) {
        *dynamic_pressure_out_pa = dynamic_pressure_pa;
    }
    const double effective_aoa_rad = std::clamp(frame.angle_of_attack_rad, -0.35, 0.35);
    const double normal_force_magnitude_n =
        dynamic_pressure_pa * vehicle.reference_area_m2 *
        vehicle.aerodynamic_coefficients.normal_force_slope_per_rad * effective_aoa_rad;

    return -normal_force_magnitude_n * frame.lateral_air_direction_world;
}

Vector3 computeAerodynamicMomentBody(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Vector3& aerodynamic_normal_force_world_n,
    double center_of_gravity_from_nose_m,
    double center_of_pressure_from_nose_m) noexcept {
    const Vector3 cp_offset_body_m {0.0, 0.0, center_of_pressure_from_nose_m - center_of_gravity_from_nose_m};
    const Vector3 aerodynamic_force_body_n =
        inverseRotateVector(state.attitude_body_to_world, aerodynamic_normal_force_world_n);
    const Vector3 restoring_moment_body_nm = cross(cp_offset_body_m, aerodynamic_force_body_n);
    const Vector3 damping_moment_body_nm =
        -vehicle.aerodynamic_coefficients.rotational_damping_coefficient * state.angular_velocity_body_radps;
    return restoring_moment_body_nm + damping_moment_body_nm;
}

bool isParachuteDeployed(
    const FlightState& state,
    const VehicleModel& vehicle,
    double time_s) noexcept {
    const bool descending = state.velocity_mps.z < -0.5;
    const bool after_delay = time_s >= vehicle.recovery_system.deployment_delay_s;
    const bool below_deploy_altitude =
        state.position_m.z <= vehicle.recovery_system.deployment_altitude_m;
    return descending && after_delay && below_deploy_altitude;
}

}  // namespace

MotorCluster::MotorCluster(std::vector<MountedMotor> motors)
    : motors_(std::move(motors)) {}

Vector3 MotorCluster::thrustForceN(double time_s) const noexcept {
    Vector3 total_thrust {};
    for (const MountedMotor& mounted_motor : motors_) {
        if (mounted_motor.failed || time_s < 0.0 || time_s > mounted_motor.motor.burn_time_s) {
            continue;
        }
        total_thrust += mounted_motor.motor.max_thrust_n *
                        mounted_motor.thrust_direction_body.normalized();
    }
    return total_thrust;
}

Vector3 MotorCluster::thrustMomentNm(double time_s) const noexcept {
    Vector3 total_moment {};
    for (const MountedMotor& mounted_motor : motors_) {
        if (mounted_motor.failed || time_s < 0.0 || time_s > mounted_motor.motor.burn_time_s) {
            continue;
        }
        const Vector3 thrust_vector =
            mounted_motor.motor.max_thrust_n * mounted_motor.thrust_direction_body.normalized();
        total_moment += cross(mounted_motor.mount_position_m, thrust_vector);
    }
    return total_moment;
}

double MotorCluster::massFlowKgPerS(double time_s) const noexcept {
    double total_mass_flow_kgps = 0.0;
    for (const MountedMotor& mounted_motor : motors_) {
        if (mounted_motor.failed || time_s < 0.0 || time_s > mounted_motor.motor.burn_time_s) {
            continue;
        }
        if (mounted_motor.motor.burn_time_s > 0.0) {
            total_mass_flow_kgps +=
                mounted_motor.motor.propellant_mass_kg / mounted_motor.motor.burn_time_s;
        }
    }
    return total_mass_flow_kgps;
}

std::size_t MotorCluster::motorCount() const noexcept {
    return motors_.size();
}

bool MotorCluster::motorFailed(std::size_t index) const noexcept {
    if (index >= motors_.size()) {
        return false;
    }
    return motors_[index].failed;
}

const std::vector<MountedMotor>& MotorCluster::mountedMotors() const noexcept {
    return motors_;
}

void MotorCluster::setMotorFailed(std::size_t index, bool failed) noexcept {
    if (index < motors_.size()) {
        motors_[index].failed = failed;
    }
}

bool MotorCluster::isBurning(double time_s) const noexcept {
    return std::ranges::any_of(motors_, [time_s](const MountedMotor& mounted_motor) {
        return !mounted_motor.failed &&
               time_s >= 0.0 &&
               time_s <= mounted_motor.motor.burn_time_s;
    });
}

double MotorCluster::maxBurnTimeS() const noexcept {
    double max_burn_time_s = 0.0;
    for (const MountedMotor& mounted_motor : motors_) {
        if (mounted_motor.failed) {
            continue;
        }
        max_burn_time_s = std::max(max_burn_time_s, mounted_motor.motor.burn_time_s);
    }
    return max_burn_time_s;
}

double MotorCluster::totalPropellantMassKg() const noexcept {
    double total_propellant_mass_kg = 0.0;
    for (const MountedMotor& mounted_motor : motors_) {
        total_propellant_mass_kg += mounted_motor.motor.propellant_mass_kg;
    }
    return total_propellant_mass_kg;
}

ForceResult computeForces(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s) noexcept {
    ForceResult result;
    result.wind_velocity_world_mps = environment.windVelocityWorldMps(state.position_m.z, time_s);
    result.relative_air_velocity_world_mps = state.velocity_mps - result.wind_velocity_world_mps;
    const Vector3 thrust_force_body_n = vehicle.cluster.thrustForceN(time_s);
    result.thrust_force_world_n = rotateVector(state.attitude_body_to_world, thrust_force_body_n);
    result.drag_force_n = computeDragForce(
        state,
        vehicle,
        environment,
        result.relative_air_velocity_world_mps,
        &result.dynamic_pressure_pa,
        &result.mach_number);
    result.parachute_deployed = isParachuteDeployed(state, vehicle, time_s);
    if (result.parachute_deployed) {
        const double speed_mps = result.relative_air_velocity_world_mps.magnitude();
        if (speed_mps > 1e-6) {
            const double air_density = environment.airDensityKgPerM3(state.position_m.z);
            const double recovery_drag_magnitude_n =
                0.5 * air_density * speed_mps * speed_mps *
                vehicle.recovery_system.parachute_drag_coefficient *
                vehicle.recovery_system.parachute_area_m2;
            result.recovery_drag_force_n =
                -recovery_drag_magnitude_n * result.relative_air_velocity_world_mps.normalized();
        }
    }
    result.aerodynamic_normal_force_world_n =
        computeAerodynamicNormalForce(
            state,
            vehicle,
            environment,
            result.relative_air_velocity_world_mps,
            &result.dynamic_pressure_pa,
            &result.angle_of_attack_rad);
    result.gravity_force_n = computeGravityForce(state, environment);
    result.thrust_moment_body_nm = vehicle.cluster.thrustMomentNm(time_s);
    result.center_of_gravity_from_nose_m = computeCenterOfGravityFromNoseM(state, vehicle);
    result.center_of_pressure_from_nose_m = computeCenterOfPressureFromNoseM(vehicle);
    result.static_margin_calibers = computeStaticMarginCalibers(state, vehicle);
    result.aerodynamic_moment_body_nm = computeAerodynamicMomentBody(
        state,
        vehicle,
        result.aerodynamic_normal_force_world_n,
        result.center_of_gravity_from_nose_m,
        result.center_of_pressure_from_nose_m);
    const CfdAugmentation cfd = computeCfdAugmentation(
        state,
        vehicle,
        environment,
        result.relative_air_velocity_world_mps,
        result.dynamic_pressure_pa,
        result.mach_number,
        result.angle_of_attack_rad);
    result.cfd_force_world_n = cfd.force_world_n;
    result.cfd_moment_body_nm = cfd.moment_body_nm;
    result.shockwave_intensity = cfd.shockwave_intensity;
    result.aeroelastic_response = cfd.aeroelastic_response;
    result.total_force_n = result.thrust_force_world_n + result.drag_force_n +
                           result.recovery_drag_force_n +
                           result.aerodynamic_normal_force_world_n +
                           result.cfd_force_world_n +
                           result.gravity_force_n;
    result.on_launch_rail = isOnLaunchRail(state, environment);
    if (result.on_launch_rail) {
        const Vector3 rail_axis_world =
            rotateVector(state.attitude_body_to_world, {0.0, 0.0, 1.0}).normalized();
        result.total_force_n = projectOntoAxis(result.total_force_n, rail_axis_world);
        result.thrust_moment_body_nm = {};
        result.aerodynamic_moment_body_nm = {};
        result.cfd_moment_body_nm = {};
    }
    return result;
}

StateDerivative evaluateStateDerivative(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s) noexcept {
    const ForceResult forces = computeForces(state, vehicle, environment, time_s);

    const double effective_mass_kg = std::max(state.mass_kg, vehicle.dry_mass_kg);
    Vector3 velocity_mps = state.velocity_mps;
    Vector3 acceleration_mps2 = forces.total_force_n / effective_mass_kg;
    Quaternion attitude_rate = computeAttitudeRate(
        state.attitude_body_to_world,
        state.angular_velocity_body_radps);
    Vector3 angular_acceleration_body_radps2 = computeAngularAccelerationBody(
        forces.thrust_moment_body_nm + forces.aerodynamic_moment_body_nm + forces.cfd_moment_body_nm,
        state.angular_velocity_body_radps,
        vehicle.principal_inertia_kgm2);

    if (forces.on_launch_rail) {
        const Vector3 rail_axis_world =
            rotateVector(state.attitude_body_to_world, {0.0, 0.0, 1.0}).normalized();
        const double axial_position_m = dot(state.position_m, rail_axis_world);
        const double axial_velocity_mps = dot(state.velocity_mps, rail_axis_world);
        velocity_mps = projectOntoAxis(state.velocity_mps, rail_axis_world);
        acceleration_mps2 = projectOntoAxis(acceleration_mps2, rail_axis_world);
        if (axial_position_m <= 1e-4 &&
            axial_velocity_mps <= 1e-4 &&
            dot(acceleration_mps2, rail_axis_world) < 0.0) {
            velocity_mps = {};
            acceleration_mps2 = {};
        }
        attitude_rate = {};
        angular_acceleration_body_radps2 = {};
    }

    return {
        .velocity_mps = velocity_mps,
        .acceleration_mps2 = acceleration_mps2,
        .attitude_rate = attitude_rate,
        .angular_acceleration_body_radps2 = angular_acceleration_body_radps2,
        .mass_flow_kgps = -vehicle.cluster.massFlowKgPerS(time_s)
    };
}

}  // namespace rocket
