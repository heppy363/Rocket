#pragma once

#include <array>
#include <vector>

#include "rocket/Environment.hpp"
#include "rocket/FlightState.hpp"
#include "rocket/MathTypes.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket {

enum class CfdComponentBand {
    NoseCone,
    BodyTube,
    Transition,
    FinSet,
    Payload,
    MotorMount,
    Count
};

struct CfdParticleRenderSample {
    double x_norm {};
    double y_norm {};
    double prev_x_norm {};
    double prev_y_norm {};
    double kinetic_energy {};
    double age_s {};
};

struct CfdAugmentation {
    Vector3 force_world_n {};
    Vector3 moment_body_nm {};
    std::array<double, static_cast<std::size_t>(CfdComponentBand::Count)> component_pressure_pa {};
    double shockwave_intensity {};
    double aeroelastic_response {};
};

struct CfdFrameData {
    std::vector<CfdParticleRenderSample> render_particles {};
    std::array<double, static_cast<std::size_t>(CfdComponentBand::Count)> component_pressure_pa {};
    int solver_particle_count {};
    int rendered_particle_count {};
    double shockwave_intensity {};
    double aeroelastic_response {};
};

class RealTimeCfdField {
public:
    void update(
        const FlightState& state,
        const VehicleModel& vehicle,
        const Environment& environment,
        double time_s,
        double dt_s);

    [[nodiscard]] const CfdFrameData& frame() const noexcept;

private:
    struct Particle {
        double x_norm {};
        double y_norm {};
        double prev_x_norm {};
        double prev_y_norm {};
        double vx_normps {};
        double vy_normps {};
        double age_s {};
        double kinetic_energy {};
    };

    std::vector<Particle> particles_ {};
    CfdFrameData frame_ {};
    unsigned int random_state_ {0xA341316Cu};

    [[nodiscard]] double nextUnitRandom() noexcept;
};

[[nodiscard]] CfdAugmentation computeCfdAugmentation(
    const FlightState& state,
    const VehicleModel& vehicle,
    const Environment& environment,
    const Vector3& relative_air_velocity_world_mps,
    double dynamic_pressure_pa,
    double mach_number,
    double angle_of_attack_rad) noexcept;

}  // namespace rocket
