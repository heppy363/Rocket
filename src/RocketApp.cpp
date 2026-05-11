#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <deque>
#include <format>
#include <string>
#include <vector>

#include "Camera3D.hpp"
#include "Window.hpp"
#include "raylib.h"
#include "rocket/Aerodynamics.hpp"
#include "rocket/CfdModule.hpp"
#include "rocket/DesignLibrary.hpp"
#include "rocket/Forces.hpp"
#include "rocket/MeshGenerator.hpp"
#include "rocket/PhysicalConstants.hpp"
#include "rocket/RocketApp.hpp"
#include "rocket/RungeKutta4.hpp"
#include "rocket/SecureValidation.hpp"
#include "rocket/SimulationMonitor.hpp"
#include "rocket/VehicleModel.hpp"

namespace {

#include "app/RocketAppState.inl"
#include "app/RocketAppInteraction.inl"
#include "app/RocketAppUiCommon.inl"
#include "app/RocketAppUiModeling.inl"
#include "app/RocketAppUiSimulation.inl"

}  // namespace

int rocket::runRocketLabApp() {
    using namespace rocket;

    VehicleModel vehicle {
        .dry_mass_kg = 8.0,
        .reference_area_m2 = pi * std::pow(0.104 * 0.5, 2.0),
        .principal_inertia_kgm2 = {1.8, 1.8, 0.08},
        .geometry = makePresetGeometry(RocketPreset::ResearchStarter),
        .aerodynamic_coefficients = AerodynamicCoefficients {
            .drag_coefficient = 0.62,
            .normal_force_slope_per_rad = 3.4,
            .rotational_damping_coefficient = 1.1
        },
        .cluster = MotorCluster({
            MountedMotor {
                .motor = Motor {
                    .max_thrust_n = 180.0,
                    .burn_time_s = 2.4,
                    .propellant_mass_kg = 0.24
                },
                .mount_position_m = {-0.04, 0.0, 0.0},
                .thrust_direction_body = {0.0, 0.0, 1.0},
                .failed = false
            },
            MountedMotor {
                .motor = Motor {
                    .max_thrust_n = 180.0,
                    .burn_time_s = 2.4,
                    .propellant_mass_kg = 0.24
                },
                .mount_position_m = {0.04, 0.0, 0.0},
                .thrust_direction_body = {0.0, 0.0, 1.0},
                .failed = true
            }
        })
    };

    syncVehicleDerivedValues(vehicle);

    const double dt_s = 1.0 / physical_constants::simulation_frequency_hz;
    Environment environment;
    AppState app_state;
    SimulationRuntime simulation_runtime;
    MotorEditorState motor_editor;
    SimulationMonitor simulation_monitor;
    motor_editor.motor_count = 2;
    motor_editor.max_thrust_n = 180.0;
    motor_editor.burn_time_s = 2.4;
    motor_editor.propellant_mass_kg = 0.24;
    motor_editor.mount_radius_m = 0.04;
    app_state.active_preset = RocketPreset::ResearchStarter;

    raylib::Window window(1600, 960, "The Rocket Lab", FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    window.SetMinSize(1280, 760);
    window.SetTargetFPS(60);
    SetWindowPosition(80, 60);

    MeshGenerator mesh_generator(vehicle.geometry, vehicle.cluster);
    resetSimulationRuntime(vehicle, simulation_runtime);

    raylib::Camera3D camera(
        ::Vector3 {22.0f, 14.0f, 22.0f},
        ::Vector3 {0.0f, 6.0f, 0.0f},
        ::Vector3 {0.0f, 1.0f, 0.0f},
        45.0f,
        CAMERA_PERSPECTIVE);

    while (!window.ShouldClose()) {
        const bool f3_pressed = IsKeyPressed(KEY_F3);

        if (IsKeyPressed(KEY_F1)) {
            app_state.workspace = Workspace::Modeling;
        }
        if (IsKeyPressed(KEY_F2)) {
            app_state.workspace = Workspace::Simulation;
        }
        if (f3_pressed) {
            app_state.show_simulation_window = !app_state.show_simulation_window;
            if (app_state.show_simulation_window) {
                simulation_monitor.start();
            } else {
                simulation_monitor.stop();
            }
        }
        if (app_state.workspace == Workspace::Modeling && IsKeyPressed(KEY_TAB)) {
            app_state.camera_orbit = !app_state.camera_orbit;
        }
        if (app_state.workspace == Workspace::Modeling) {
            if (IsKeyPressed(KEY_ONE)) {
                app_state.modeling_tool = ModelingTool::Select;
            }
            if (IsKeyPressed(KEY_TWO)) {
                app_state.modeling_tool = ModelingTool::Move;
            }
            if (IsKeyPressed(KEY_THREE)) {
                app_state.modeling_tool = ModelingTool::Rotate;
            }
            if (IsKeyPressed(KEY_FOUR)) {
                app_state.modeling_tool = ModelingTool::Scale;
            }
            if (IsKeyPressed(KEY_FIVE)) {
                app_state.modeling_tool = ModelingTool::AddPart;
            }
            if (IsKeyPressed(KEY_SIX)) {
                app_state.modeling_tool = ModelingTool::Measure;
            }
        }

        if (app_state.show_simulation_window && !simulation_monitor.isRunning() && !f3_pressed) {
            app_state.show_simulation_window = false;
        }

        const ::Rectangle modeling_workspace_button {286.0f, 14.0f, 176.0f, 36.0f};
        const ::Rectangle simulation_workspace_button {472.0f, 14.0f, 176.0f, 36.0f};
        const ::Rectangle replay_route_button {982.0f, 14.0f, 176.0f, 36.0f};
        const ::Rectangle markers_button {1172.0f, 14.0f, 148.0f, 36.0f};
        const ::Rectangle fixed_camera_button {1334.0f, 14.0f, 84.0f, 36.0f};
        const ::Rectangle follow_camera_button {1428.0f, 14.0f, 84.0f, 36.0f};
        const ::Rectangle free_camera_button {1522.0f, 14.0f, 58.0f, 36.0f};
        if (isRectangleClicked(modeling_workspace_button)) {
            app_state.workspace = Workspace::Modeling;
        }
        if (isRectangleClicked(simulation_workspace_button)) {
            app_state.workspace = Workspace::Simulation;
            app_state.camera_orbit = false;
        }
        if (app_state.workspace == Workspace::Simulation && isRectangleClicked(replay_route_button)) {
            simulation_runtime.replay_active = !simulation_runtime.replay_active;
            simulation_runtime.replay_time_s = 0.0;
        }
        if (app_state.workspace == Workspace::Simulation && isRectangleClicked(markers_button)) {
            app_state.show_flight_markers = !app_state.show_flight_markers;
        }
        if (app_state.workspace == Workspace::Simulation && isRectangleClicked(fixed_camera_button)) {
            app_state.simulation_camera_mode = SimulationCameraMode::Fixed;
        }
        if (app_state.workspace == Workspace::Simulation && isRectangleClicked(follow_camera_button)) {
            app_state.simulation_camera_mode = SimulationCameraMode::Follow;
        }
        if (app_state.workspace == Workspace::Simulation && isRectangleClicked(free_camera_button)) {
            app_state.simulation_camera_mode = SimulationCameraMode::Free;
        }

        const bool simulation_active =
            app_state.workspace == Workspace::Simulation || app_state.show_simulation_window;

        if (simulation_active) {
            if (IsKeyPressed(KEY_SPACE)) {
                simulation_runtime.paused = !simulation_runtime.paused;
            }
            if (IsKeyPressed(KEY_R)) {
                resetSimulationRuntime(vehicle, simulation_runtime);
            }
            if (IsKeyPressed(KEY_K)) {
                cycleMissionKeyframePreview(simulation_runtime);
            }
        }

        updateViewportModelingInteraction(
            app_state,
            vehicle,
            motor_editor,
            mesh_generator,
            simulation_runtime,
            camera);

        const float frame_time_s = GetFrameTime();
        updateReplayTimeline(simulation_runtime, frame_time_s);

        if (!simulation_runtime.paused && simulation_active && !simulation_runtime.replay_active) {
            stepSimulationRuntime(
                simulation_runtime,
                vehicle,
                environment,
                dt_s,
                frame_time_s);
        }

        const FlightState modeling_preview_state = buildModelingPreviewState(vehicle);
        const FlightState replay_or_live_state = currentRenderState(simulation_runtime);
        const double render_time_s = currentRenderTime(simulation_runtime);
        const FlightState& view_state =
            app_state.workspace == Workspace::Modeling ? modeling_preview_state : replay_or_live_state;

        updateWorkspaceCamera(app_state, simulation_runtime, camera, view_state);

        const ForceResult modeling_force_result = computeForces(modeling_preview_state, vehicle, environment, 0.0);
        const ForceResult simulation_force_result = computeForces(view_state, vehicle, environment, render_time_s);
        app_state.cfd_field.update(
            view_state,
            vehicle,
            environment,
            render_time_s,
            frame_time_s);
        const rocket::CfdFrameData& cfd_frame = app_state.cfd_field.frame();
        const rocket::SimulationSnapshot modeling_snapshot =
            buildSnapshot(modeling_preview_state, vehicle, modeling_force_result, environment, mesh_generator, 0.0, 0.0);
        const rocket::SimulationSnapshot simulation_snapshot = buildSnapshot(
            view_state,
            vehicle,
            simulation_force_result,
            environment,
            mesh_generator,
            render_time_s,
            simulation_runtime.max_altitude_m,
            cfd_frame.solver_particle_count,
            cfd_frame.rendered_particle_count);

        if (app_state.show_simulation_window) {
            simulation_monitor.publish(buildMonitorState(
                simulation_snapshot,
                simulation_runtime,
                vehicle,
                environment));
        }

        BeginDrawing();
        ClearBackground(Color {6, 10, 18, 255});
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color {18, 25, 39, 255}, Color {7, 11, 19, 255});
        DrawCircle(GetScreenWidth() - 220, 144, 142.0f, Color {110, 164, 255, 16});
        DrawCircle(GetScreenWidth() - 220, 144, 84.0f, Color {219, 233, 255, 12});

        drawWorkspaceBar(app_state, simulation_runtime);

        camera.BeginMode();
        DrawPlane(::Vector3 {0.0f, 0.0f, 0.0f}, ::Vector2 {140.0f, 140.0f}, Color {18, 24, 38, 255});
        DrawGrid(app_state.workspace == Workspace::Modeling ? 30 : 40, app_state.workspace == Workspace::Modeling ? 1.0f : 2.0f);
        if (app_state.workspace == Workspace::Modeling) {
            drawSelectedComponentGrid(app_state, vehicle);
            drawReferenceBlueprintInViewport(app_state, vehicle);
        }

        if (app_state.workspace == Workspace::Simulation) {
            drawTrajectory(simulation_runtime.trajectory_history);
            if (app_state.show_flight_markers) {
                drawFlightMarkers3D(simulation_runtime);
            }
            drawReplayGhost(simulation_runtime);
        }

        mesh_generator.draw(view_state);
        if (app_state.workspace == Workspace::Modeling && app_state.show_wireframe_hint) {
            mesh_generator.drawWireframe(view_state);
        }
        drawSelectedComponentEditCage(app_state, vehicle, motor_editor);
        drawModelingHandles3D(app_state, vehicle, motor_editor, mesh_generator);

        if (app_state.show_metrics_overlay) {
            drawStabilityMarkers(
                vehicle,
                view_state,
                app_state.workspace == Workspace::Modeling ? modeling_force_result : simulation_force_result);
        }

        if (app_state.show_snap_points && app_state.workspace == Workspace::Modeling) {
            DrawSphere(::Vector3 {0.0f, 0.0f, static_cast<float>(vehicle.geometry.body_length_m * 0.5)}, 0.06f, Color {125, 211, 252, 255});
            DrawSphere(::Vector3 {0.0f, 0.0f, static_cast<float>(-vehicle.geometry.body_length_m * 0.5)}, 0.06f, Color {125, 211, 252, 255});
        }

        if (app_state.workspace == Workspace::Simulation) {
            DrawLine3D(
                toRaylib(view_state.position_m),
                ::Vector3 {static_cast<float>(view_state.position_m.x), 0.0f, static_cast<float>(view_state.position_m.y)},
                Color {251, 191, 36, 120});
        }
        camera.EndMode();
        drawModelingHandlesOverlay(app_state, vehicle, motor_editor, mesh_generator, camera);
        drawModelingHandleLabels(app_state, vehicle, motor_editor, mesh_generator, camera);
        if (app_state.workspace == Workspace::Simulation && app_state.show_flight_markers) {
            drawFlightMarkerLabels(simulation_runtime, camera);
        }

        if (app_state.workspace == Workspace::Modeling) {
            updateFloatingWindowDrag(app_state.layout.modeling_toolbar);
            updateFloatingWindowDrag(app_state.layout.modeling_outliner);
            updateFloatingWindowDrag(app_state.layout.modeling_library);
            updateFloatingWindowDrag(app_state.layout.modeling_properties);
            updateFloatingWindowDrag(app_state.layout.modeling_status);
            updateFloatingWindowDrag(app_state.layout.modeling_reference);

            drawModelingToolbar(app_state, app_state.layout.modeling_toolbar.bounds);
            drawModelingOutliner(app_state, app_state.layout.modeling_outliner.bounds);
            drawPieceLibrary(app_state, vehicle, motor_editor, mesh_generator, simulation_runtime, app_state.layout.modeling_library.bounds);
            handleGeometryEdits(app_state, vehicle, motor_editor, mesh_generator, simulation_runtime, app_state.layout.modeling_properties.bounds);
            drawModelingStatusBar(app_state.layout.modeling_status.bounds, app_state, modeling_snapshot, vehicle);
            drawModelingReferenceWindow(app_state, app_state.layout.modeling_reference.bounds);
            drawModelingViewportHeader(app_state, vehicle);
        } else if (app_state.workspace == Workspace::Simulation) {
            updateFloatingWindowDrag(app_state.layout.sim_telemetry);
            updateFloatingWindowDrag(app_state.layout.sim_events);
            updateFloatingWindowDrag(app_state.layout.sim_wind_tunnel);
            updateFloatingWindowDrag(app_state.layout.sim_overview);
            updateFloatingWindowDrag(app_state.layout.sim_timeline);
            updateFloatingWindowDrag(app_state.layout.sim_scenario);
            if (simulation_runtime.keyframe_preview_active) {
                updateFloatingWindowDrag(app_state.layout.sim_keyframe);
            }

            drawSimulationTelemetry(app_state.layout.sim_telemetry.bounds, simulation_snapshot);
            drawSimulationEvents(app_state.layout.sim_events.bounds, simulation_runtime, vehicle, simulation_snapshot);
            drawWindTunnelPanel(app_state.layout.sim_wind_tunnel.bounds, app_state, simulation_snapshot, vehicle);
            drawTrajectoryOverview(app_state.layout.sim_overview.bounds, simulation_runtime);
            drawSimulationScenario(app_state.layout.sim_scenario.bounds, app_state, simulation_runtime, vehicle, environment);
            drawSimulationTimeline(app_state.layout.sim_timeline.bounds, simulation_runtime, vehicle);
            if (simulation_runtime.keyframe_preview_active) {
                drawMissionKeyframePreview(app_state.layout.sim_keyframe.bounds, simulation_runtime, simulation_snapshot);
            }
        }

        DrawFPS(GetScreenWidth() - 100, 18);
        EndDrawing();
    }

    simulation_monitor.stop();

    return 0;
}
