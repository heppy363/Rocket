#pragma once

#include <vector>

#include "rocket/MathTypes.hpp"

namespace rocket {

struct Motor {
    double max_thrust_n {};
    double burn_time_s {};
    double propellant_mass_kg {};
};

struct MountedMotor {
    Motor motor {};
    Vector3 mount_position_m {};
    Vector3 thrust_direction_body {0.0, 0.0, 1.0};
    bool failed {false};
};

class MotorCluster {
public:
    explicit MotorCluster(std::vector<MountedMotor> motors);

    [[nodiscard]] Vector3 thrustForceN(double time_s) const noexcept;
    [[nodiscard]] Vector3 thrustMomentNm(double time_s) const noexcept;
    [[nodiscard]] double massFlowKgPerS(double time_s) const noexcept;
    [[nodiscard]] bool isBurning(double time_s) const noexcept;
    [[nodiscard]] double totalPropellantMassKg() const noexcept;

    [[nodiscard]] std::size_t motorCount() const noexcept;
    [[nodiscard]] bool motorFailed(std::size_t index) const noexcept;
    [[nodiscard]] const std::vector<MountedMotor>& mountedMotors() const noexcept;
    void setMotorFailed(std::size_t index, bool failed) noexcept;

private:
    std::vector<MountedMotor> motors_;
};

}  // namespace rocket
