#include "RocketAppTrajectory.hpp"

#include <cmath>

#include "raylib.h"
#include "rocket/SimulationEngine.hpp"

double horizontalRangeM(const rocket::Vector3& point) {
    return std::sqrt(point.x * point.x + point.y * point.y);
}

void drawTrajectory(const std::deque<rocket::TrajectorySample>& history) {
    if (history.size() < 2) {
        return;
    }

    for (std::size_t index = 1; index < history.size(); ++index) {
        const unsigned char alpha = static_cast<unsigned char>(
            50 + (205 * static_cast<int>(index)) / static_cast<int>(history.size()));
        DrawLine3D(
            ::Vector3 {
                static_cast<float>(history[index - 1].state.position_m.x),
                static_cast<float>(history[index - 1].state.position_m.z),
                static_cast<float>(history[index - 1].state.position_m.y)},
            ::Vector3 {
                static_cast<float>(history[index].state.position_m.x),
                static_cast<float>(history[index].state.position_m.z),
                static_cast<float>(history[index].state.position_m.y)},
            Color {64, 196, 255, alpha});
    }
}

void drawReplayGhost(const rocket::SimulationRuntime& runtime) {
    if (!runtime.replay_active || runtime.trajectory_history.size() < 2) {
        return;
    }

    const rocket::Vector3 replay_position =
        rocket::sampleTrajectoryState(runtime.trajectory_history, runtime.replay_time_s).position_m;
    DrawSphere(
        ::Vector3 {
            static_cast<float>(replay_position.x),
            static_cast<float>(replay_position.z),
            static_cast<float>(replay_position.y)},
        0.18f,
        Color {251, 191, 36, 220});
    DrawSphereWires(
        ::Vector3 {
            static_cast<float>(replay_position.x),
            static_cast<float>(replay_position.z),
            static_cast<float>(replay_position.y)},
        0.28f,
        8,
        8,
        Color {125, 211, 252, 180});
}

void drawFlightMarkers3D(const rocket::SimulationRuntime& runtime) {
    DrawSphere(::Vector3 {0.0f, 0.0f, 0.0f}, 0.16f, Color {34, 197, 94, 255});

    if (runtime.burnout_recorded) {
        DrawSphere(
            ::Vector3 {
                static_cast<float>(runtime.burnout_point_m.x),
                static_cast<float>(runtime.burnout_point_m.z),
                static_cast<float>(runtime.burnout_point_m.y)},
            0.14f,
            Color {249, 115, 22, 255});
    }
    if (runtime.apogee_recorded) {
        DrawSphere(
            ::Vector3 {
                static_cast<float>(runtime.apogee_point_m.x),
                static_cast<float>(runtime.apogee_point_m.z),
                static_cast<float>(runtime.apogee_point_m.y)},
            0.16f,
            Color {168, 85, 247, 255});
    }
    if (runtime.impact_recorded) {
        DrawSphere(
            ::Vector3 {
                static_cast<float>(runtime.impact_point_m.x),
                static_cast<float>(runtime.impact_point_m.z),
                static_cast<float>(runtime.impact_point_m.y)},
            0.16f,
            Color {239, 68, 68, 255});
    }
}

void drawFlightMarkerLabels(const rocket::SimulationRuntime& runtime, const raylib::Camera3D& camera) {
    const auto draw_marker = [&](const rocket::Vector3& point, const char* label, Color color) {
        const ::Vector2 screen = GetWorldToScreen(
            ::Vector3 {
                static_cast<float>(point.x),
                static_cast<float>(point.z),
                static_cast<float>(point.y)},
            camera);
        DrawText(label, static_cast<int>(screen.x) + 10, static_cast<int>(screen.y) - 8, 15, color);
    };

    draw_marker({0.0, 0.0, 0.0}, "Launch", Color {34, 197, 94, 255});
    if (runtime.burnout_recorded) {
        draw_marker(runtime.burnout_point_m, "Burnout", Color {249, 115, 22, 255});
    }
    if (runtime.apogee_recorded) {
        draw_marker(runtime.apogee_point_m, "Apogee", Color {168, 85, 247, 255});
    }
    if (runtime.impact_recorded) {
        draw_marker(runtime.impact_point_m, "Impact", Color {239, 68, 68, 255});
    }

    if (runtime.replay_active && runtime.trajectory_history.size() > 1) {
        const rocket::Vector3 replay_position =
            rocket::sampleTrajectoryState(runtime.trajectory_history, runtime.replay_time_s).position_m;
        draw_marker(replay_position, "Replay", Color {251, 191, 36, 255});
    }
}
