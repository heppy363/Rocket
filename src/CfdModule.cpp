#include "rocket/CfdModule.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "rocket/Aerodynamics.hpp"

namespace rocket {

namespace {

constexpr double pi = 3.14159265358979323846;

std::size_t bandIndex(CfdComponentBand band) noexcept {
    return static_cast<std::size_t>(band);
}

double componentBandStart(const VehicleGeometry& geometry, CfdComponentBand band) noexcept {
    switch (band) {
    case CfdComponentBand::NoseCone:
        return 0.0;
    case CfdComponentBand::BodyTube:
        return geometry.nose_length_m;
    case CfdComponentBand::Payload:
        return geometry.nose_length_m;
    case CfdComponentBand::FinSet:
        return geometry.fin_front_from_nose_m;
    case CfdComponentBand::Transition:
        return geometry.body_length_m - geometry.transition_length_m;
    case CfdComponentBand::MotorMount:
        return geometry.body_length_m - std::max(geometry.transition_length_m, geometry.body_diameter_m * 0.35);
    case CfdComponentBand::Count:
        break;
    }
    return 0.0;
}

double componentBandEnd(const VehicleGeometry& geometry, CfdComponentBand band) noexcept {
    switch (band) {
    case CfdComponentBand::NoseCone:
        return geometry.nose_length_m;
    case CfdComponentBand::BodyTube:
        return geometry.body_length_m - geometry.transition_length_m;
    case CfdComponentBand::Payload:
        return geometry.nose_length_m + geometry.payload_length_m;
    case CfdComponentBand::FinSet:
        return geometry.fin_front_from_nose_m + geometry.fin_root_chord_m;
    case CfdComponentBand::Transition:
        return geometry.body_length_m;
    case CfdComponentBand::MotorMount:
        return geometry.body_length_m;
    case CfdComponentBand::Count:
        break;
    }
    return geometry.body_length_m;
}

CfdComponentBand classifyBand(const VehicleGeometry& geometry, double station_from_nose_m) noexcept {
    if (station_from_nose_m <= geometry.nose_length_m) {
        return CfdComponentBand::NoseCone;
    }
    if (station_from_nose_m <= geometry.nose_length_m + geometry.payload_length_m) {
        return CfdComponentBand::Payload;
    }
    if (station_from_nose_m >= geometry.fin_front_from_nose_m &&
        station_from_nose_m <= geometry.fin_front_from_nose_m + geometry.fin_root_chord_m) {
        return CfdComponentBand::FinSet;
    }
    if (station_from_nose_m >= geometry.body_length_m - geometry.transition_length_m) {
        return CfdComponentBand::Transition;
    }
    if (station_from_nose_m >= geometry.body_length_m - std::max(geometry.transition_length_m, geometry.body_diameter_m * 0.35)) {
        return CfdComponentBand::MotorMount;
    }
    return CfdComponentBand::BodyTube;
}

Vector3 inverseRotateVector(const Quaternion& rotation, const Vector3& vector) noexcept {
    return rotateVector(rotation.conjugate(), vector);
}

Vector3 lateralAirDirectionWorld(const FlightState& state, const Vector3& relative_air_velocity_world_mps) noexcept {
    const Vector3 air_dir_world = relative_air_velocity_world_mps.magnitude() > 1e-6
        ? relative_air_velocity_world_mps.normalized()
        : Vector3 {};
    const Vector3 body_axis_world = rotateVector(state.attitude_body_to_world, Vector3 {0.0, 0.0, 1.0});
    const double axial_alignment = dot(air_dir_world, body_axis_world);
    const Vector3 axial_projection = axial_alignment * body_axis_world;
    const Vector3 lateral = air_dir_world - axial_projection;
    return lateral.magnitude() > 1e-6 ? lateral.normalized() : Vector3 {};
}

double finFlexibilityFactor(const VehicleGeometry& geometry) noexcept {
    const double thickness = std::max(geometry.fin_thickness_m * geometry.fin_controls.thickness_scale, 0.0015);
    const double span = std::max(geometry.fin_span_m * geometry.fin_controls.span_scale, 0.02);
    return std::clamp(span / thickness / 42.0, 0.4, 3.5);
}

double bodySlendernessFactor(const VehicleGeometry& geometry) noexcept {
    return std::clamp(geometry.body_length_m / std::max(geometry.body_diameter_m, 1e-6), 5.0, 28.0);
}

double componentAreaEstimate(const VehicleGeometry& geometry, CfdComponentBand band) noexcept {
    const double radius = geometry.body_diameter_m * 0.5;
    switch (band) {
    case CfdComponentBand::NoseCone:
        return std::max(0.02, geometry.nose_length_m * radius);
    case CfdComponentBand::BodyTube:
        return std::max(0.04, geometry.body_length_m * geometry.body_diameter_m);
    case CfdComponentBand::Transition:
        return std::max(0.02, geometry.transition_length_m * geometry.transition_aft_diameter_m);
    case CfdComponentBand::FinSet:
        return std::max(0.02, 0.5 * (geometry.fin_root_chord_m + geometry.fin_tip_chord_m) *
            geometry.fin_span_m * geometry.fin_controls.span_scale * static_cast<double>(std::max(geometry.fin_count, 1)));
    case CfdComponentBand::Payload:
        return std::max(0.02, geometry.payload_length_m * geometry.body_diameter_m * 0.7);
    case CfdComponentBand::MotorMount:
        return std::max(0.01, geometry.transition_aft_diameter_m * geometry.transition_aft_diameter_m * 0.4);
    case CfdComponentBand::Count:
        break;
    }
    return 0.02;
}

bool isRecoveryFlowState(
    const FlightState& state,
    const VehicleModel& vehicle,
    double time_s) noexcept {
    return state.velocity_mps.z < -0.5 &&
           time_s >= vehicle.recovery_system.deployment_delay_s &&
           state.position_m.z <= vehicle.recovery_system.deployment_altitude_m;
}

}  // namespace

double RealTimeCfdField::nextUnitRandom() noexcept {
    random_state_ ^= random_state_ << 13;
    random_state_ ^= random_state_ >> 17;
    random_state_ ^= random_state_ << 5;
    return static_cast<double>(random_state_ & 0x00FFFFFFu) / static_cast<double>(0x01000000u);
}

const CfdFrameData& RealTimeCfdField::frame() const noexcept {
    return frame_;
}

void RealTimeCfdField::update(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    double time_s,
    double dt_s) {
    const Vector3 wind_velocity_world_mps = environment.windVelocityWorldMps(state.position_m.z, time_s);
    const Vector3 relative_air_velocity_world_mps = state.velocity_mps - wind_velocity_world_mps;
    const double air_speed_mps = std::max(relative_air_velocity_world_mps.magnitude(), 1.0);
    const double air_density = environment.airDensityKgPerM3(state.position_m.z);
    const double dynamic_pressure_pa = 0.5 * air_density * air_speed_mps * air_speed_mps;
    const double speed_of_sound = std::max(environment.speedOfSoundMps(state.position_m.z), 1.0);
    const double mach = air_speed_mps / speed_of_sound;
    const bool recovery_flow = isRecoveryFlowState(state, vehicle, time_s);
    const double aoa_deg = std::abs(std::asin(std::clamp(dot(relative_air_velocity_world_mps.normalized(), rotateVector(state.attitude_body_to_world, Vector3 {0.0, 0.0, 1.0})), -1.0, 1.0))) * 180.0 / pi;
    const double interest_area_m2 =
        componentAreaEstimate(vehicle.geometry, CfdComponentBand::BodyTube) +
        componentAreaEstimate(vehicle.geometry, CfdComponentBand::FinSet) +
        componentAreaEstimate(vehicle.geometry, CfdComponentBand::NoseCone);
    const double density_scale = std::clamp(air_density / 1.225, 0.35, 1.25);
    const double target_particle_scale =
        recovery_flow
            ? std::clamp(interest_area_m2 * 2.4 + dynamic_pressure_pa / 26000.0 + density_scale * 0.8, 0.7, 3.2)
            : std::clamp(interest_area_m2 * 4.0 + dynamic_pressure_pa / 16000.0 + mach * 2.0 + density_scale, 1.0, 10.0);
    const int target_particles = recovery_flow
        ? std::clamp(static_cast<int>(std::round(target_particle_scale * 8000.0)), 6000, 28000)
        : std::clamp(static_cast<int>(std::round(target_particle_scale * 10000.0)), 10000, 100000);
    const int render_cap = recovery_flow
        ? std::clamp(target_particles / 12, 700, 1800)
        : std::clamp(target_particles / 10, 1200, 5000);
    const double body_radius_norm = std::clamp(vehicle.geometry.body_diameter_m / std::max(vehicle.geometry.body_length_m, 0.2) * 2.0, 0.03, 0.16);
    const double center_x = 0.48;
    const double center_y = 0.54;
    const double rocket_length_norm = 0.62;
    const double nose_start_x = center_x - rocket_length_norm * 0.45;
    const double body_end_x = center_x + rocket_length_norm * 0.38;
    const double fin_front_x = nose_start_x + (vehicle.geometry.fin_front_from_nose_m / std::max(vehicle.geometry.body_length_m, 0.2)) * rocket_length_norm;
    const double fin_back_x = fin_front_x + (vehicle.geometry.fin_root_chord_m / std::max(vehicle.geometry.body_length_m, 0.2)) * rocket_length_norm;
    const double fin_span_norm = std::clamp(
        vehicle.geometry.fin_span_m * vehicle.geometry.fin_controls.span_scale / std::max(vehicle.geometry.body_length_m, 0.2) * rocket_length_norm * 1.8,
        0.03,
        0.18);
    const double dt_clamped = std::clamp(dt_s, 1.0 / 240.0, 1.0 / 30.0);

    particles_.resize(static_cast<std::size_t>(target_particles));
    frame_.component_pressure_pa.fill(0.0);
    frame_.solver_particle_count = target_particles;
    frame_.shockwave_intensity = recovery_flow ? 0.0 : std::clamp(1.0 - std::abs(mach - 1.0) / 0.22, 0.0, 1.0);
    frame_.aeroelastic_response = std::clamp(dynamic_pressure_pa / 42000.0 * finFlexibilityFactor(vehicle.geometry) * 0.18, 0.0, 0.35);

    const int grid_w = 40;
    const int grid_h = 24;
    std::array<int, grid_w * grid_h> cell_counts {};
    std::array<double, grid_w * grid_h> cell_velocity_x_sum {};
    std::array<double, grid_w * grid_h> cell_velocity_y_sum {};

    const auto sampleVelocity = [&](double x_norm, double y_norm) {
        const double rel_x = x_norm - center_x;
        const double rel_y = y_norm - center_y;
        const double radial = std::sqrt(rel_x * rel_x + rel_y * rel_y);
        const double lane_speed = recovery_flow
            ? 0.14 + std::clamp(air_speed_mps / 360.0, 0.05, 0.32)
            : 0.22 + std::clamp(air_speed_mps / 280.0, 0.08, 1.1);
        double vx = lane_speed;
        double vy = recovery_flow
            ? std::clamp(aoa_deg / 60.0, -0.18, 0.18) * 0.12
            : std::clamp(aoa_deg / 24.0, -0.6, 0.6) * 0.18;
        if (radial < body_radius_norm * 4.0 && x_norm >= nose_start_x - 0.04 && x_norm <= body_end_x + 0.06) {
            const double body_influence = std::exp(-(radial * radial) / std::max(body_radius_norm * body_radius_norm * 6.0, 1e-6));
            const double swirl = rel_y >= 0.0 ? 1.0 : -1.0;
            vx -= body_influence * (recovery_flow ? 0.035 : 0.08 + frame_.shockwave_intensity * 0.05);
            vy += swirl * body_influence * (recovery_flow ? 0.09 : 0.26 + frame_.shockwave_intensity * 0.12);
        }
        if (!recovery_flow && x_norm >= fin_front_x && x_norm <= fin_back_x && std::abs(rel_y) >= body_radius_norm * 0.7) {
            const double fin_influence = std::exp(-std::pow((std::abs(rel_y) - (body_radius_norm + fin_span_norm * 0.5)) / std::max(fin_span_norm, 0.01), 2.0));
            vy += (rel_y >= 0.0 ? 1.0 : -1.0) * fin_influence * (0.18 + std::abs(aoa_deg) / 80.0);
            vx -= fin_influence * 0.04;
        }
        if (!recovery_flow && frame_.shockwave_intensity > 0.02 && x_norm >= nose_start_x + rocket_length_norm * 0.15 && x_norm <= nose_start_x + rocket_length_norm * 0.32) {
            vy += std::sin((y_norm + x_norm * 0.8) * 14.0 + time_s * 2.4) * frame_.shockwave_intensity * 0.05;
        }
        return Vector3 {vx, vy, 0.0};
    };

    for (Particle& particle : particles_) {
        if (particle.age_s <= 0.0 || particle.x_norm > 1.08 || particle.y_norm < -0.08 || particle.y_norm > 1.08) {
            particle.x_norm = recovery_flow ? (-0.02 - nextUnitRandom() * 0.08) : (-nextUnitRandom() * 0.24);
            particle.y_norm = recovery_flow ? (0.16 + nextUnitRandom() * 0.68) : (0.08 + nextUnitRandom() * 0.84);
            particle.prev_x_norm = particle.x_norm;
            particle.prev_y_norm = particle.y_norm;
            particle.vx_normps = recovery_flow ? (0.08 + nextUnitRandom() * 0.03) : (0.16 + nextUnitRandom() * 0.08);
            particle.vy_normps = recovery_flow ? ((nextUnitRandom() - 0.5) * 0.015) : ((nextUnitRandom() - 0.5) * 0.04);
            particle.age_s = 0.2 + nextUnitRandom() * 0.4;
            particle.kinetic_energy = 0.0;
        }

        particle.prev_x_norm = particle.x_norm;
        particle.prev_y_norm = particle.y_norm;

        const Vector3 flow_velocity = sampleVelocity(particle.x_norm, particle.y_norm);
        particle.vx_normps = particle.vx_normps * 0.82 + flow_velocity.x * 0.18;
        particle.vy_normps = particle.vy_normps * 0.82 + flow_velocity.y * 0.18;
        particle.x_norm += particle.vx_normps * dt_clamped;
        particle.y_norm += particle.vy_normps * dt_clamped;
        particle.age_s += dt_clamped;

        const double local_rel_x = particle.x_norm - center_x;
        const double local_rel_y = particle.y_norm - center_y;
        const double body_y_extent = body_radius_norm;
        const bool inside_body =
            particle.x_norm >= nose_start_x &&
            particle.x_norm <= body_end_x &&
            std::abs(local_rel_y) <= body_y_extent;
        if (inside_body) {
            const double sign = local_rel_y >= 0.0 ? 1.0 : -1.0;
            particle.y_norm = center_y + sign * (body_y_extent + 0.004);
            particle.vy_normps += sign * (recovery_flow ? 0.06 : 0.18 + frame_.shockwave_intensity * 0.08);
            particle.vx_normps *= recovery_flow ? 0.82 : 0.72;
            const double station = std::clamp((particle.x_norm - nose_start_x) / std::max(rocket_length_norm, 1e-6), 0.0, 1.0) * vehicle.geometry.body_length_m;
            const CfdComponentBand band = classifyBand(vehicle.geometry, station);
            frame_.component_pressure_pa[bandIndex(band)] += dynamic_pressure_pa * (0.6 + 0.5 * nextUnitRandom());
        }

        const int grid_x = std::clamp(static_cast<int>(particle.x_norm * grid_w), 0, grid_w - 1);
        const int grid_y = std::clamp(static_cast<int>(particle.y_norm * grid_h), 0, grid_h - 1);
        const std::size_t cell_index = static_cast<std::size_t>(grid_y * grid_w + grid_x);
        ++cell_counts[cell_index];
        cell_velocity_x_sum[cell_index] += particle.vx_normps;
        cell_velocity_y_sum[cell_index] += particle.vy_normps;
        particle.kinetic_energy = 0.5 * air_density * std::pow(std::sqrt(particle.vx_normps * particle.vx_normps + particle.vy_normps * particle.vy_normps) * air_speed_mps, 2.0);
    }

    for (Particle& particle : particles_) {
        const int grid_x = std::clamp(static_cast<int>(particle.x_norm * grid_w), 0, grid_w - 1);
        const int grid_y = std::clamp(static_cast<int>(particle.y_norm * grid_h), 0, grid_h - 1);
        int neighborhood_occupancy = 0;
        double neighborhood_vx = 0.0;
        double neighborhood_vy = 0.0;
        int contributing_cells = 0;
        for (int y_offset = -1; y_offset <= 1; ++y_offset) {
            for (int x_offset = -1; x_offset <= 1; ++x_offset) {
                const int neighbor_x = std::clamp(grid_x + x_offset, 0, grid_w - 1);
                const int neighbor_y = std::clamp(grid_y + y_offset, 0, grid_h - 1);
                const std::size_t neighbor_index = static_cast<std::size_t>(neighbor_y * grid_w + neighbor_x);
                neighborhood_occupancy += cell_counts[neighbor_index];
                neighborhood_vx += cell_velocity_x_sum[neighbor_index];
                neighborhood_vy += cell_velocity_y_sum[neighbor_index];
                ++contributing_cells;
            }
        }

        const double smoothed_occupancy =
            static_cast<double>(neighborhood_occupancy) / std::max(contributing_cells, 1);
        if (smoothed_occupancy > (recovery_flow ? 22.0 : 16.0)) {
            const double spread =
                (smoothed_occupancy - (recovery_flow ? 22.0 : 16.0)) / (recovery_flow ? 22.0 : 16.0);
            particle.y_norm += (nextUnitRandom() - 0.5) * (recovery_flow ? 0.007 : 0.015) * spread;
            particle.x_norm -= (recovery_flow ? 0.0025 : 0.0055) * spread;
        }

        if (neighborhood_occupancy > 0) {
            const double average_vx = neighborhood_vx / static_cast<double>(neighborhood_occupancy);
            const double average_vy = neighborhood_vy / static_cast<double>(neighborhood_occupancy);
            particle.vx_normps = particle.vx_normps * 0.92 + average_vx * 0.08;
            particle.vy_normps = particle.vy_normps * 0.92 + average_vy * 0.08;
        }
    }

    for (std::size_t i = 0; i < frame_.component_pressure_pa.size(); ++i) {
        const auto band = static_cast<CfdComponentBand>(i);
        const double area = componentAreaEstimate(vehicle.geometry, band);
        frame_.component_pressure_pa[i] /= std::max(area * 220.0, 1.0);
    }

    frame_.render_particles.clear();
    frame_.render_particles.reserve(static_cast<std::size_t>(render_cap));
    const double render_step = static_cast<double>(target_particles) / static_cast<double>(render_cap);
    double cursor = 0.0;
    while (static_cast<int>(frame_.render_particles.size()) < render_cap && static_cast<std::size_t>(cursor) < particles_.size()) {
        const Particle& particle = particles_[static_cast<std::size_t>(cursor)];
        frame_.render_particles.push_back(CfdParticleRenderSample {
            .x_norm = particle.x_norm,
            .y_norm = particle.y_norm,
            .prev_x_norm = particle.prev_x_norm,
            .prev_y_norm = particle.prev_y_norm,
            .kinetic_energy = particle.kinetic_energy,
            .age_s = particle.age_s
        });
        cursor += std::max(render_step, 1.0);
    }
    frame_.rendered_particle_count = static_cast<int>(frame_.render_particles.size());
}

CfdAugmentation computeCfdAugmentation(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    const Vector3& relative_air_velocity_world_mps,
    double dynamic_pressure_pa,
    double mach_number,
    double angle_of_attack_rad) noexcept {
    CfdAugmentation augmentation;
    const double speed = relative_air_velocity_world_mps.magnitude();
    if (speed <= 1e-6 || dynamic_pressure_pa <= 1e-6) {
        return augmentation;
    }

    const double aoa_abs = std::abs(angle_of_attack_rad);
    const double shockwave_intensity = std::clamp(1.0 - std::abs(mach_number - 1.0) / 0.22, 0.0, 1.0);
    const double fin_flexibility = finFlexibilityFactor(vehicle.geometry);
    const double aeroelastic_response =
        std::clamp(dynamic_pressure_pa / 42000.0 * fin_flexibility * (0.14 + shockwave_intensity * 0.10), 0.0, 0.42);
    const double drag_gain = 0.035 + shockwave_intensity * 0.11 + aoa_abs * 0.09 + aeroelastic_response * 0.22;
    const double lift_gain = 0.018 + shockwave_intensity * 0.05 + aeroelastic_response * 0.08;
    const Vector3 air_dir = relative_air_velocity_world_mps.normalized();
    const Vector3 lateral_dir = lateralAirDirectionWorld(state, relative_air_velocity_world_mps);
    const double reference_area = std::max(vehicle.reference_area_m2, 1e-4);

    augmentation.force_world_n += (-1.0 * air_dir) * (dynamic_pressure_pa * reference_area * drag_gain);
    augmentation.force_world_n += (-1.0 * lateral_dir) * (dynamic_pressure_pa * reference_area * lift_gain * std::clamp(angle_of_attack_rad, -0.45, 0.45));

    const double cp_to_cg_m =
        computeCenterOfPressureFromNoseM(vehicle) - computeCenterOfGravityFromNoseM(state, vehicle);
    augmentation.moment_body_nm =
        Vector3 {0.0, dynamic_pressure_pa * reference_area * cp_to_cg_m * (0.025 + aeroelastic_response * 0.09) * std::clamp(angle_of_attack_rad, -0.45, 0.45), 0.0};

    for (std::size_t i = 0; i < augmentation.component_pressure_pa.size(); ++i) {
        const auto band = static_cast<CfdComponentBand>(i);
        const double area = componentAreaEstimate(vehicle.geometry, band);
        double local_gain = 0.82;
        if (band == CfdComponentBand::NoseCone) {
            local_gain = 1.18 + shockwave_intensity * 0.28;
        } else if (band == CfdComponentBand::FinSet) {
            local_gain = 0.94 + aeroelastic_response * 0.42 + aoa_abs * 0.35;
        } else if (band == CfdComponentBand::Transition || band == CfdComponentBand::MotorMount) {
            local_gain = 0.88 + shockwave_intensity * 0.12;
        }
        augmentation.component_pressure_pa[i] = dynamic_pressure_pa * local_gain * std::clamp(area / std::max(reference_area, 1e-6), 0.35, 3.8);
    }
    augmentation.shockwave_intensity = shockwave_intensity;
    augmentation.aeroelastic_response = aeroelastic_response;
    (void)environment;
    return augmentation;
}

}  // namespace rocket
