#pragma once

#include <cmath>

namespace rocket {

struct Vector3 {
    double x {};
    double y {};
    double z {};

    constexpr Vector3 operator+(const Vector3& other) const noexcept {
        return {x + other.x, y + other.y, z + other.z};
    }

    constexpr Vector3 operator-(const Vector3& other) const noexcept {
        return {x - other.x, y - other.y, z - other.z};
    }

    constexpr Vector3 operator*(double scalar) const noexcept {
        return {x * scalar, y * scalar, z * scalar};
    }

    constexpr Vector3 operator/(double scalar) const noexcept {
        return {x / scalar, y / scalar, z / scalar};
    }

    constexpr Vector3& operator+=(const Vector3& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr Vector3& operator-=(const Vector3& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    [[nodiscard]] double magnitude() const noexcept {
        return std::sqrt(x * x + y * y + z * z);
    }

    [[nodiscard]] Vector3 normalized() const noexcept {
        const double norm = magnitude();
        if (norm <= 0.0) {
            return {};
        }
        return *this / norm;
    }
};

constexpr Vector3 operator*(double scalar, const Vector3& vector) noexcept {
    return vector * scalar;
}

constexpr double dot(const Vector3& lhs, const Vector3& rhs) noexcept {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

constexpr Vector3 cross(const Vector3& lhs, const Vector3& rhs) noexcept {
    return {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x
    };
}

struct Quaternion {
    double w {1.0};
    double x {};
    double y {};
    double z {};

    constexpr Quaternion operator+(const Quaternion& other) const noexcept {
        return {w + other.w, x + other.x, y + other.y, z + other.z};
    }

    constexpr Quaternion operator*(double scalar) const noexcept {
        return {w * scalar, x * scalar, y * scalar, z * scalar};
    }

    constexpr Quaternion operator/(double scalar) const noexcept {
        return {w / scalar, x / scalar, y / scalar, z / scalar};
    }

    constexpr Quaternion operator*(const Quaternion& other) const noexcept {
        return {
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w
        };
    }

    [[nodiscard]] double magnitude() const noexcept {
        return std::sqrt(w * w + x * x + y * y + z * z);
    }

    [[nodiscard]] Quaternion normalized() const noexcept {
        const double norm = magnitude();
        if (norm <= 0.0) {
            return {};
        }
        return *this / norm;
    }

    [[nodiscard]] Quaternion conjugate() const noexcept {
        return {w, -x, -y, -z};
    }
};

constexpr Quaternion operator*(double scalar, const Quaternion& quaternion) noexcept {
    return quaternion * scalar;
}

[[nodiscard]] inline Vector3 rotateVector(const Quaternion& rotation, const Vector3& vector) noexcept {
    const Quaternion pure_vector {0.0, vector.x, vector.y, vector.z};
    const Quaternion rotated = rotation * pure_vector * rotation.conjugate();
    return {rotated.x, rotated.y, rotated.z};
}

}  // namespace rocket
