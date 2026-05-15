#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <deque>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

#include "Camera3D.hpp"
#include "Window.hpp"
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"
#include "rocket/Aerodynamics.hpp"
#include "rocket/CfdModule.hpp"
#include "rocket/DesignLibrary.hpp"
#include "rocket/DebugTelemetry.hpp"
#include "rocket/Forces.hpp"
#include "rocket/MeshGenerator.hpp"
#include "rocket/NativeFileDialog.hpp"
#include "rocket/PhysicalConstants.hpp"
#include "rocket/ProjectIO.hpp"
#include "rocket/RocketApp.hpp"
#include "rocket/RungeKutta4.hpp"
#include "rocket/SecureValidation.hpp"
#include "rocket/SimulationCaches.hpp"
#include "rocket/SimulationCore.hpp"
#include "rocket/SimulationEngine.hpp"
#include "rocket/SimulationMonitor.hpp"
#include "rocket/SimulationRuntime.hpp"
#include "rocket/VehicleModel.hpp"
#include "rocket/WeatherProvider.hpp"

namespace {

#include "app/RocketAppState.inl"
#include "app/RocketAppInteraction.inl"
#include "app/RocketAppUiTrajectory.inl"
#include "app/RocketAppUiCommon.inl"
#include "app/RocketAppUiWindTunnel.inl"
#include "app/RocketAppUiModeling.inl"
#include "app/RocketAppUiSimulation.inl"
#include "app/RocketAppImGui.inl"

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
    DebugTelemetryCollector debug_telemetry;
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
    rlImGuiSetup(true);
    applyDarkSpaceTheme();

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
        const bool f4_pressed = IsKeyPressed(KEY_F4);

        if (IsKeyPressed(KEY_F1)) {
            app_state.workspace = Workspace::Modeling;
        }
        if (IsKeyPressed(KEY_F2)) {
            app_state.workspace = Workspace::Simulation;
        }
        if (f3_pressed || app_state.request_simulation_window_toggle) {
            app_state.show_simulation_window = !app_state.show_simulation_window;
            if (app_state.show_simulation_window) {
                simulation_monitor.start();
            } else {
                simulation_monitor.stop();
            }
        }
        app_state.request_simulation_window_toggle = false;
        if (f4_pressed || app_state.request_debug_terminal_toggle) {
            app_state.show_debug_terminal = !app_state.show_debug_terminal;
        }
        app_state.request_debug_terminal_toggle = false;
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

        const bool simulation_active =
            app_state.workspace == Workspace::Simulation || app_state.show_simulation_window;

        if (simulation_active) {
            if (IsKeyPressed(KEY_SPACE)) {
                if (simulation_runtime.paused) {
                    simulation_runtime.keyframe_preview_active = false;
                    simulation_runtime.keyframe_preview_index = -1;
                    simulation_runtime.keyframe_preview_time_s = 0.0;
                    rocket::clearScrubPreview(simulation_runtime);
                }
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
        rocket::updateReplayTimeline(simulation_runtime, frame_time_s);

        if (!simulation_runtime.paused && simulation_active && !simulation_runtime.replay_active) {
            if (const auto step_result = rocket::stepSimulationRuntime(
                simulation_runtime,
                vehicle,
                environment,
                rocket::Seconds {dt_s},
                frame_time_s);
                !step_result) {
                simulation_runtime.paused = true;
                setTransientStatus(
                    app_state,
                    std::format("Simulazione in pausa: {}", step_result.error().message),
                    6.0);
            }
        }

        const FlightState modeling_preview_state = buildModelingPreviewState(vehicle);
        const FlightState live_simulation_state = simulation_runtime.state;
        const FlightState replay_or_live_state = rocket::currentRenderState(simulation_runtime);
        const double render_time_s = rocket::currentRenderTime(simulation_runtime);
        const FlightState& view_state =
            app_state.workspace == Workspace::Modeling ? modeling_preview_state : replay_or_live_state;

        updateWorkspaceCamera(app_state, simulation_runtime, camera, view_state);

        const ForceResult modeling_force_result = computeForces(modeling_preview_state, vehicle, environment, 0.0);
        const ForceResult live_simulation_force_result = computeForces(
            live_simulation_state,
            vehicle,
            environment,
            simulation_runtime.time_s);
        const ForceResult simulation_force_result = computeForces(view_state, vehicle, environment, render_time_s);
        app_state.cfd_field.update(
            view_state,
            vehicle,
            environment,
            render_time_s,
            frame_time_s);
        const rocket::CfdFrameData& cfd_frame = app_state.cfd_field.frame();
        const rocket::SimulationSnapshot modeling_snapshot =
            buildSnapshot(modeling_preview_state, vehicle, modeling_force_result, environment, 0.0, 0.0);
        const rocket::SimulationSnapshot live_simulation_snapshot = buildSnapshot(
            live_simulation_state,
            vehicle,
            live_simulation_force_result,
            environment,
            simulation_runtime.time_s,
            simulation_runtime.max_altitude_m,
            cfd_frame.solver_particle_count,
            cfd_frame.rendered_particle_count);
        const rocket::SimulationSnapshot simulation_snapshot = buildSnapshot(
            view_state,
            vehicle,
            simulation_force_result,
            environment,
            render_time_s,
            simulation_runtime.max_altitude_m,
            cfd_frame.solver_particle_count,
            cfd_frame.rendered_particle_count);
        rocket::SimulationSnapshot keyframe_snapshot = simulation_snapshot;
        if (simulation_runtime.keyframe_preview_active && !simulation_runtime.trajectory_history.empty()) {
            const FlightState keyframe_state = rocket::sampleTrajectoryState(
                simulation_runtime.trajectory_history,
                simulation_runtime.keyframe_preview_time_s);
            const ForceResult keyframe_force_result = computeForces(
                keyframe_state,
                vehicle,
                environment,
                simulation_runtime.keyframe_preview_time_s);
            keyframe_snapshot = buildSnapshot(
                keyframe_state,
                vehicle,
                keyframe_force_result,
                environment,
                simulation_runtime.keyframe_preview_time_s,
                simulation_runtime.max_altitude_m,
                cfd_frame.solver_particle_count,
                cfd_frame.rendered_particle_count);
        }
        const rocket::SimulationSnapshot& active_snapshot =
            app_state.workspace == Workspace::Modeling ? modeling_snapshot : simulation_snapshot;
        const rocket::DebugTelemetrySnapshot debug_snapshot =
            debug_telemetry.capture(environment, active_snapshot, frame_time_s);
        mesh_generator.setPressureOverlay(
            cfd_frame.component_pressure_pa,
            std::max(active_snapshot.dynamic_pressure_pa, active_snapshot.recommended_max_dynamic_pressure_pa),
            app_state.show_cfd_pressure_overlay);

        const auto compute_failure_intensity = [&](ComponentType component, CfdComponentBand band) {
            const double limit_pa = estimateComponentDynamicPressureLimitPa(component, vehicle.geometry);
            const double component_pa = cfd_frame.component_pressure_pa[static_cast<std::size_t>(band)];
            const double ratio = component_pa / std::max(limit_pa, 1.0);
            return static_cast<float>(std::clamp((ratio - 1.0) / 0.6, 0.0, 1.0));
        };
        mesh_generator.setComponentFailureOverlay(
            std::array<float, static_cast<std::size_t>(CfdComponentBand::Count)>{
                compute_failure_intensity(ComponentType::NoseCone, CfdComponentBand::NoseCone),
                compute_failure_intensity(ComponentType::BodyTube, CfdComponentBand::BodyTube),
                compute_failure_intensity(ComponentType::Transition, CfdComponentBand::Transition),
                compute_failure_intensity(ComponentType::FinSet, CfdComponentBand::FinSet),
                compute_failure_intensity(ComponentType::Payload, CfdComponentBand::Payload),
                compute_failure_intensity(ComponentType::MotorMount, CfdComponentBand::MotorMount)
            },
            app_state.workspace == Workspace::Simulation);

        if (app_state.show_simulation_window) {
            simulation_monitor.publish(buildMonitorState(simulation_snapshot, simulation_runtime, vehicle, environment));
        }

        const bool ctrl_down = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        const bool shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        if (ctrl_down && shift_down && IsKeyPressed(KEY_S)) {
            app_state.request_project_save_as = true;
        } else if (ctrl_down && IsKeyPressed(KEY_S)) {
            app_state.request_project_save = true;
        }
        if (ctrl_down && IsKeyPressed(KEY_O)) {
            app_state.request_project_load = true;
        }
        if (ctrl_down && IsKeyPressed(KEY_L)) {
            app_state.request_project_load = true;
        }
        if (ctrl_down && IsKeyPressed(KEY_E)) {
            app_state.request_project_export = true;
        }

        const auto save_project = [&](const std::filesystem::path& target_path) {
            const rocket::ProjectDocument project_document =
                buildProjectDocument(app_state, vehicle, motor_editor, environment);
            std::string error_message;
            if (rocket::saveProjectDocument(target_path, project_document, error_message)) {
                app_state.current_project_path = target_path;
                setTransientStatus(
                    app_state,
                    std::format("Progetto salvato in {}", app_state.current_project_path.string()));
            } else {
                setTransientStatus(app_state, std::format("Salvataggio fallito: {}", error_message), 5.0);
            }
        };

        if (app_state.request_project_save_as) {
            std::string error_message;
            std::filesystem::path chosen_path = app_state.current_project_path;
            if (rocket::promptSaveFilePath(
                    "Save Rocket Lab Project",
                    "Rocket Lab Project (*.rlab)",
                    "*.rlab",
                    chosen_path,
                    error_message)) {
                save_project(chosen_path);
            } else if (!error_message.empty()) {
                setTransientStatus(app_state, std::format("Dialog save fallito: {}", error_message), 5.0);
            }
            app_state.request_project_save_as = false;
        } else if (app_state.request_project_save) {
            save_project(app_state.current_project_path);
            app_state.request_project_save = false;
        }

        if (app_state.request_project_load) {
            rocket::ProjectDocument loaded_document;
            std::string error_message;
            std::filesystem::path chosen_path = app_state.current_project_path;
            if (rocket::promptOpenFilePath(
                    "Load Rocket Lab Project",
                    "Rocket Lab Project (*.rlab)",
                    "*.rlab",
                    chosen_path,
                    error_message)) {
                if (rocket::loadProjectDocument(chosen_path, loaded_document, error_message)) {
                    app_state.current_project_path = chosen_path;
                    applyProjectDocument(
                        app_state,
                        loaded_document,
                        vehicle,
                        motor_editor,
                        environment,
                        mesh_generator,
                        simulation_runtime);
                    setTransientStatus(
                        app_state,
                        std::format("Progetto caricato da {}", app_state.current_project_path.string()));
                    app_state.request_project_load = false;
                    continue;
                }
                setTransientStatus(app_state, std::format("Caricamento fallito: {}", error_message), 5.0);
            } else if (!error_message.empty()) {
                setTransientStatus(app_state, std::format("Dialog load fallito: {}", error_message), 5.0);
            }
            app_state.request_project_load = false;
        }

        if (app_state.request_project_export) {
            const rocket::ProjectDocument project_document =
                buildProjectDocument(app_state, vehicle, motor_editor, environment);
            std::string error_message;
            std::filesystem::path report_path = app_state.current_report_path;
            std::filesystem::path csv_path = app_state.current_trajectory_csv_path;
            if (!rocket::promptSaveFilePath(
                    "Export Simulation Report",
                    "Text Report (*.txt)",
                    "*.txt",
                    report_path,
                    error_message)) {
                if (!error_message.empty()) {
                    setTransientStatus(app_state, std::format("Dialog export report fallito: {}", error_message), 5.0);
                }
                app_state.request_project_export = false;
            } else if (!rocket::promptSaveFilePath(
                           "Export Trajectory CSV",
                           "CSV File (*.csv)",
                           "*.csv",
                           csv_path,
                           error_message)) {
                if (!error_message.empty()) {
                    setTransientStatus(app_state, std::format("Dialog export CSV fallito: {}", error_message), 5.0);
                }
                app_state.request_project_export = false;
            } else {
                app_state.current_report_path = report_path;
                app_state.current_trajectory_csv_path = csv_path;
                const rocket::ProjectExportSummary export_summary =
                    buildProjectExportSummary(simulation_runtime);
                const std::vector<rocket::TrajectoryRecord> trajectory_records =
                    buildTrajectoryRecords(simulation_runtime);
                const bool report_ok =
                    rocket::exportProjectReport(report_path, project_document, active_snapshot, export_summary, error_message);
                const bool csv_ok =
                    rocket::exportTrajectoryCsv(csv_path, trajectory_records, error_message);
                if (report_ok && csv_ok) {
                    setTransientStatus(
                        app_state,
                        std::format("Export completato: {} e {}", report_path.string(), csv_path.string()));
                } else {
                    setTransientStatus(app_state, std::format("Export fallito: {}", error_message), 5.0);
                }
                app_state.request_project_export = false;
            }
        }

        BeginDrawing();
        ClearBackground(Color {6, 10, 18, 255});
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color {18, 25, 39, 255}, Color {7, 11, 19, 255});
        DrawCircle(GetScreenWidth() - 220, 144, 142.0f, Color {110, 164, 255, 16});
        DrawCircle(GetScreenWidth() - 220, 144, 84.0f, Color {219, 233, 255, 12});

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

        rlImGuiBegin();
        renderDearImGuiUi(
            app_state,
            vehicle,
            motor_editor,
            mesh_generator,
            simulation_runtime,
            environment,
            modeling_snapshot,
            keyframe_snapshot,
            live_simulation_snapshot,
            simulation_snapshot,
            debug_snapshot);
        rlImGuiEnd();

        DrawFPS(GetScreenWidth() - 100, 18);
        EndDrawing();
    }

    simulation_monitor.stop();
    rlImGuiShutdown();

    return 0;
}
