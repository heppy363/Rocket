void updateReplayTimeline(SimulationRuntime& runtime, float frame_time_s) {
    if (runtime.replay_active && !runtime.trajectory_history.empty()) {
        runtime.replay_time_s += static_cast<double>(frame_time_s) * 0.9;
        const double end_time_s = runtime.trajectory_history.back().time_s;
        if (runtime.replay_time_s > end_time_s) {
            runtime.replay_time_s = 0.0;
        }
    }
}

void updateModelingCadCamera(AppState& app_state, raylib::Camera3D& camera) {
    const bool mouse_over_ui = isMouseOverModelingUi(app_state);

    if (!mouse_over_ui) {
        const float wheel = GetMouseWheelMove();
        if (std::abs(wheel) > 1e-5f) {
            app_state.modeling_camera_distance =
                std::clamp(app_state.modeling_camera_distance - wheel * 1.2f, 2.5f, 80.0f);
        }
    }

    if (app_state.camera_orbit && !mouse_over_ui) {
        const ::Vector2 delta = GetMouseDelta();
        const bool middle_down = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);
        const bool shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        if (middle_down && !shift_down) {
            app_state.modeling_camera_yaw_rad -= delta.x * 0.0085f;
            app_state.modeling_camera_pitch_rad =
                std::clamp(app_state.modeling_camera_pitch_rad - delta.y * 0.0065f, -1.35f, 1.35f);
        } else if (middle_down && shift_down) {
            const float pan_scale = 0.0022f * app_state.modeling_camera_distance;
            const float cos_yaw = std::cos(app_state.modeling_camera_yaw_rad);
            const float sin_yaw = std::sin(app_state.modeling_camera_yaw_rad);
            const ::Vector3 right {cos_yaw, 0.0f, -sin_yaw};
            const ::Vector3 up {0.0f, 1.0f, 0.0f};

            app_state.modeling_camera_target.x += (-delta.x * pan_scale) * right.x + (delta.y * pan_scale) * up.x;
            app_state.modeling_camera_target.y += (-delta.x * pan_scale) * right.y + (delta.y * pan_scale) * up.y;
            app_state.modeling_camera_target.z += (-delta.x * pan_scale) * right.z + (delta.y * pan_scale) * up.z;
        }
    }

    const float cos_pitch = std::cos(app_state.modeling_camera_pitch_rad);
    const ::Vector3 offset {
        std::cos(app_state.modeling_camera_yaw_rad) * cos_pitch * app_state.modeling_camera_distance,
        std::sin(app_state.modeling_camera_pitch_rad) * app_state.modeling_camera_distance,
        std::sin(app_state.modeling_camera_yaw_rad) * cos_pitch * app_state.modeling_camera_distance
    };

    camera.position = {
        app_state.modeling_camera_target.x + offset.x,
        app_state.modeling_camera_target.y + offset.y,
        app_state.modeling_camera_target.z + offset.z
    };
    camera.target = app_state.modeling_camera_target;
    camera.up = {0.0f, 1.0f, 0.0f};
}

void updateWorkspaceCamera(
    AppState& app_state,
    SimulationRuntime& simulation_runtime,
    raylib::Camera3D& camera,
    const rocket::FlightState& view_state) {
    if (app_state.workspace == Workspace::Modeling && app_state.modeling_view != ModelingView::Perspective) {
        app_state.camera_orbit = false;
        const float front_distance = 6.0f;
        switch (app_state.modeling_view) {
        case ModelingView::Perspective:
            break;
        case ModelingView::Front:
            camera.position = {0.0f, 0.0f, front_distance};
            camera.target = {0.0f, 0.0f, 0.0f};
            camera.up = {0.0f, 1.0f, 0.0f};
            break;
        case ModelingView::Side:
            camera.position = {front_distance, 0.0f, 0.0f};
            camera.target = {0.0f, 0.0f, 0.0f};
            camera.up = {0.0f, 1.0f, 0.0f};
            break;
        case ModelingView::Top:
            camera.position = {0.0f, front_distance, 0.001f};
            camera.target = {0.0f, 0.0f, 0.0f};
            camera.up = {0.0f, 0.0f, -1.0f};
            break;
        }
    } else if (app_state.workspace == Workspace::Modeling) {
        updateModelingCadCamera(app_state, camera);
    } else if (app_state.workspace == Workspace::Simulation) {
        const rocket::Vector3 body_axis = rocket::rotateVector(view_state.attitude_body_to_world, {0.0, 0.0, 1.0}).normalized();
        const rocket::Vector3 lateral_axis = rocket::rotateVector(view_state.attitude_body_to_world, {1.0, 0.0, 0.0}).normalized();
        const rocket::Vector3 fallback_axis = body_axis.magnitude() > 1e-6 ? body_axis : rocket::Vector3 {0.0, 0.0, 1.0};
        const rocket::Vector3 focus_point = view_state.position_m + 0.8 * fallback_axis;

        switch (app_state.simulation_camera_mode) {
        case SimulationCameraMode::Fixed:
            camera.position = {18.0f, 11.0f, 18.0f};
            camera.target = {0.0f, 3.0f, 0.0f};
            camera.up = {0.0f, 1.0f, 0.0f};
            break;
        case SimulationCameraMode::Follow: {
            const rocket::Vector3 offset = (fallback_axis * -12.0) + (lateral_axis * 3.2) + rocket::Vector3 {0.0, 0.0, 2.8};
            camera.position = toRaylib(focus_point + offset);
            camera.target = toRaylib(focus_point);
            camera.up = {0.0f, 1.0f, 0.0f};
            break;
        }
        case SimulationCameraMode::Free:
            camera.Update(CAMERA_FREE);
            break;
        }
    } else {
        camera.Update(CAMERA_FREE);
    }
}

rocket::SimulationMonitorState buildMonitorState(
    const rocket::SimulationSnapshot& snapshot,
    const SimulationRuntime& runtime,
    const rocket::VehicleModel& vehicle,
    const rocket::Environment& environment) {
    return {
        .snapshot = snapshot,
        .geometry = vehicle.geometry,
        .recovery_system = vehicle.recovery_system,
        .launch_site = environment.launchSite(),
        .surface_weather = environment.surfaceWeather(),
        .weather_source = environment.weatherDataSource(),
        .paused = runtime.paused,
        .motor_burning = vehicle.cluster.isBurning(runtime.time_s),
        .motor_count = vehicle.cluster.motorCount()
    };
}

void appendRingHandles(
    std::vector<HandleSpec>& handles,
    HandleKind kind,
    double radius_m,
    double z_m,
    Color color,
    const char* label,
    int segments = 8) {
    const double safe_radius_m = std::max(radius_m, 0.001);
    for (int index = 0; index < segments; ++index) {
        const double angle = (2.0 * pi * static_cast<double>(index)) / static_cast<double>(segments);
        handles.push_back({
            kind,
            {safe_radius_m * std::cos(angle), safe_radius_m * std::sin(angle), z_m},
            color,
            label
        });
    }
}

std::vector<::Vector3> buildComponentHitSamples(
    const rocket::VehicleModel& vehicle,
    const MotorEditorState& motor_editor,
    ComponentSelection selection) {
    const auto& geometry = vehicle.geometry;
    const double lift_z = modelingPreviewLiftM(geometry);
    const double nose_tip_z = 0.5 * geometry.body_length_m + lift_z;
    const double nose_base_z = nose_tip_z - geometry.nose_length_m;
    const double body_start_z = -0.5 * geometry.body_length_m + lift_z;
    const double body_end_z = nose_base_z;
    const double body_radius = geometry.body_diameter_m * 0.5;
    const double aft_radius = geometry.transition_aft_diameter_m * 0.5;
    const double fin_front_z = body_start_z + geometry.fin_front_from_nose_m;
    const double fin_tip_front_z = fin_front_z + geometry.fin_sweep_length_m + geometry.fin_controls.tip_le_offset_m;
    const double fin_tip_back_z = fin_tip_front_z + geometry.fin_tip_chord_m + geometry.fin_controls.tip_te_offset_m;
    const double fin_outer_radius = body_radius + geometry.fin_span_m * geometry.fin_controls.span_scale;

    std::vector<::Vector3> samples;
    const auto add_ring = [&](double radius_m, double z_m, int count = 8) {
        for (int i = 0; i < count; ++i) {
            const double angle = (2.0 * pi * static_cast<double>(i)) / static_cast<double>(count);
            samples.push_back(toRaylib({radius_m * std::cos(angle), radius_m * std::sin(angle), z_m}));
        }
    };

    switch (selection) {
    case ComponentSelection::NoseCone:
        samples.push_back(toRaylib({0.0, 0.0, nose_tip_z}));
        add_ring(body_radius * geometry.nose_controls.shoulder_radius_scale, nose_base_z + geometry.nose_length_m * 0.1);
        add_ring(body_radius * geometry.nose_controls.mid_radius_scale, nose_base_z + geometry.nose_length_m * 0.55);
        break;
    case ComponentSelection::BodyTube:
        add_ring(body_radius * geometry.body_controls.fore_radius_scale, body_end_z - 0.12);
        add_ring(body_radius * geometry.body_controls.mid_radius_scale, lift_z);
        add_ring(body_radius * geometry.body_controls.aft_radius_scale, body_start_z + geometry.transition_length_m + 0.08);
        break;
    case ComponentSelection::Transition:
        add_ring(body_radius, body_end_z - 0.02);
        add_ring(aft_radius * geometry.transition_controls.mid_radius_scale, body_start_z + geometry.transition_length_m * 0.5);
        add_ring(aft_radius, body_start_z + 0.02);
        break;
    case ComponentSelection::FinSet:
        for (int fin_index = 0; fin_index < geometry.fin_count; ++fin_index) {
            const double angle = (2.0 * pi * static_cast<double>(fin_index)) / static_cast<double>(geometry.fin_count);
            const double cs = std::cos(angle);
            const double sn = std::sin(angle);
            samples.push_back(toRaylib({body_radius * cs, body_radius * sn, fin_front_z}));
            samples.push_back(toRaylib({fin_outer_radius * cs, fin_outer_radius * sn, fin_tip_front_z}));
            samples.push_back(toRaylib({fin_outer_radius * cs, fin_outer_radius * sn, fin_tip_back_z}));
            samples.push_back(toRaylib({body_radius * cs, body_radius * sn, fin_front_z + geometry.fin_root_chord_m}));
        }
        break;
    case ComponentSelection::MotorMount: {
        const int count = std::max(motor_editor.motor_count, 1);
        const double cluster_radius = std::max(motor_editor.mount_radius_m, 0.0);
        const double mount_z = body_start_z + 0.08;
        if (count == 1) {
            samples.push_back(toRaylib({0.0, 0.0, mount_z}));
        } else {
            for (int i = 0; i < count; ++i) {
                const double angle = (2.0 * pi * static_cast<double>(i)) / static_cast<double>(count);
                samples.push_back(toRaylib({cluster_radius * std::cos(angle), cluster_radius * std::sin(angle), mount_z}));
            }
        }
        break;
    }
    case ComponentSelection::Payload: {
        const double payload_end_z = nose_base_z - geometry.payload_length_m;
        add_ring(std::max(body_radius * 0.82, 0.02), nose_base_z - geometry.payload_length_m * 0.5, 6);
        add_ring(std::max(body_radius * 0.82, 0.02), payload_end_z + 0.02, 6);
        break;
    }
    }

    return samples;
}

ComponentSelection pickComponentSelection(
    const rocket::VehicleModel& vehicle,
    const MotorEditorState& motor_editor,
    const raylib::Camera3D& camera,
    ComponentSelection fallback) {
    const ::Vector2 mouse = GetMousePosition();
    float best_distance = 28.0f;
    ComponentSelection best = fallback;

    constexpr std::array<ComponentSelection, 6> selections {
        ComponentSelection::NoseCone,
        ComponentSelection::BodyTube,
        ComponentSelection::Transition,
        ComponentSelection::FinSet,
        ComponentSelection::MotorMount,
        ComponentSelection::Payload
    };

    for (const auto selection : selections) {
        for (const auto& point : buildComponentHitSamples(vehicle, motor_editor, selection)) {
            const ::Vector2 screen = GetWorldToScreen(point, camera);
            const float dx = screen.x - mouse.x;
            const float dy = screen.y - mouse.y;
            const float distance = std::sqrt(dx * dx + dy * dy);
            if (distance < best_distance) {
                best_distance = distance;
                best = selection;
            }
        }
    }

    return best;
}

std::vector<HandleSpec> buildHandleSpecs(
    const AppState& app_state,
    const rocket::VehicleModel& vehicle,
    const MotorEditorState& motor_editor,
    const rocket::MeshGenerator& mesh_generator) {
    const auto& geometry = vehicle.geometry;
    const double lift_z = modelingPreviewLiftM(geometry);
    const double nose_tip_z = 0.5 * geometry.body_length_m;
    const double nose_base_z = nose_tip_z - geometry.nose_length_m;
    const double body_start_z = -0.5 * geometry.body_length_m;
    const double transition_end_z = body_start_z + geometry.transition_length_m;
    const double fin_front_z = -0.5 * geometry.body_length_m + geometry.fin_front_from_nose_m;
    const double fin_tip_z = fin_front_z + geometry.fin_sweep_length_m + geometry.fin_controls.tip_le_offset_m;
    const double body_radius = geometry.body_diameter_m * 0.5;
    const double fin_outer_radius = body_radius + geometry.fin_span_m * geometry.fin_controls.span_scale;

    std::vector<HandleSpec> handles;
    switch (app_state.selection) {
    case ComponentSelection::NoseCone:
        handles.push_back({HandleKind::NoseLength, {0.0, 0.0, nose_base_z + lift_z}, Color {251, 191, 36, 255}, "Nose Length"});
        appendRingHandles(handles, HandleKind::NoseShoulder, body_radius * geometry.nose_controls.shoulder_radius_scale, nose_base_z + geometry.nose_length_m * 0.1 + lift_z, Color {245, 158, 11, 255}, "Shoulder", 10);
        appendRingHandles(handles, HandleKind::NoseMidRadius, body_radius * geometry.nose_controls.mid_radius_scale, nose_base_z + geometry.nose_length_m * 0.55 + lift_z, Color {249, 115, 22, 255}, "Nose Mid", 10);
        break;
    case ComponentSelection::BodyTube:
        handles.push_back({HandleKind::BodyLength, {0.0, 0.0, body_start_z + lift_z}, Color {56, 189, 248, 255}, "Body Length"});
        appendRingHandles(handles, HandleKind::BodyDiameter, body_radius, lift_z, Color {96, 165, 250, 255}, "Diameter", 12);
        appendRingHandles(handles, HandleKind::BodyForeRadius, body_radius * geometry.body_controls.fore_radius_scale, nose_base_z - 0.1 + lift_z, Color {59, 130, 246, 255}, "Fore Radius", 10);
        appendRingHandles(handles, HandleKind::BodyMidRadius, body_radius * geometry.body_controls.mid_radius_scale, lift_z, Color {59, 130, 246, 255}, "Mid Radius", 10);
        appendRingHandles(handles, HandleKind::BodyAftRadius, body_radius * geometry.body_controls.aft_radius_scale, body_start_z + geometry.transition_length_m + 0.08 + lift_z, Color {59, 130, 246, 255}, "Aft Radius", 10);
        break;
    case ComponentSelection::Transition:
        handles.push_back({HandleKind::TransitionLength, {0.0, 0.0, transition_end_z + lift_z}, Color {168, 85, 247, 255}, "Transition Length"});
        appendRingHandles(handles, HandleKind::TransitionMidRadius, 0.5 * geometry.transition_aft_diameter_m * geometry.transition_controls.mid_radius_scale, body_start_z + geometry.transition_length_m * 0.5 + lift_z, Color {216, 180, 254, 255}, "Transition Mid", 10);
        appendRingHandles(handles, HandleKind::TransitionAftDiameter, 0.5 * geometry.transition_aft_diameter_m, body_start_z + 0.02 + lift_z, Color {192, 132, 252, 255}, "Aft Diameter", 10);
        break;
    case ComponentSelection::FinSet:
        for (int fin_index = 0; fin_index < geometry.fin_count; ++fin_index) {
            const double angle = (2.0 * pi * static_cast<double>(fin_index)) / static_cast<double>(geometry.fin_count);
            const double cs = std::cos(angle);
            const double sn = std::sin(angle);
            handles.push_back({HandleKind::FinFront, {body_radius * cs, body_radius * sn, fin_front_z + lift_z}, Color {236, 72, 153, 255}, "Fin Front"});
            handles.push_back({HandleKind::FinSpan, {fin_outer_radius * cs, fin_outer_radius * sn, fin_front_z + geometry.fin_root_chord_m * 0.45 + lift_z}, Color {244, 114, 182, 255}, "Fin Span"});
            handles.push_back({HandleKind::FinTipLead, {fin_outer_radius * cs, fin_outer_radius * sn, fin_tip_z + lift_z}, Color {190, 24, 93, 255}, "Tip LE"});
            handles.push_back({HandleKind::FinTipTrail, {fin_outer_radius * cs, fin_outer_radius * sn, fin_tip_z + geometry.fin_tip_chord_m + geometry.fin_controls.tip_te_offset_m + lift_z}, Color {157, 23, 77, 255}, "Tip TE"});
        }
        break;
    case ComponentSelection::MotorMount:
        appendRingHandles(handles, HandleKind::MotorMountRadius, std::max(motor_editor.mount_radius_m, 0.02), body_start_z + 0.08 + lift_z, Color {34, 197, 94, 255}, "Cluster Radius", std::max(motor_editor.motor_count, 6));
        break;
    case ComponentSelection::Payload:
        handles.push_back({HandleKind::PayloadLength, {0.0, 0.0, nose_base_z - geometry.payload_length_m + lift_z}, Color {125, 211, 252, 255}, "Payload Length"});
        appendRingHandles(handles, HandleKind::PayloadLength, std::max(body_radius * 0.82, 0.02), nose_base_z - geometry.payload_length_m * 0.5 + lift_z, Color {125, 211, 252, 255}, "Payload Length", 8);
        break;
    }

    auto* active_modifiers = vehicle.geometry.getActiveComponentModifiers(
        static_cast<rocket::ComponentType>(app_state.selection));
    const auto component_type = static_cast<rocket::ComponentType>(app_state.selection);

    if (active_modifiers != nullptr && active_modifiers->is_active) {
        if (app_state.mesh_selection_mode == MeshSelectionMode::Vertex) {
            for (const auto& vertex : active_modifiers->modified_vertices) {
                rocket::Vector3 vertex_world_pos = vertex.base_position_m + vertex.offset_m;
                vertex_world_pos.z += lift_z;
                const Color vertex_color = app_state.selected_vertex_id == vertex.vertex_id
                    ? Color {34, 197, 94, 255}
                    : Color {34, 197, 94, 180};

                handles.push_back({
                    HandleKind::VertexFree,
                    vertex_world_pos,
                    vertex_color,
                    std::format("Vertex #{}", vertex.vertex_id),
                    vertex.vertex_id
                });
            }
        } else if (app_state.mesh_selection_mode == MeshSelectionMode::Edge) {
            if (const auto* edges = mesh_generator.componentEdges(component_type)) {
                for (std::size_t edge_index = 0; edge_index < edges->size(); ++edge_index) {
                    rocket::Vector3 edge_world_pos = (*edges)[edge_index].center_body_m;
                    edge_world_pos.z += lift_z;
                    handles.push_back({
                        HandleKind::EdgeFree,
                        edge_world_pos,
                        app_state.selected_edge_id == static_cast<int>(edge_index)
                            ? Color {56, 189, 248, 255}
                            : Color {56, 189, 248, 170},
                        std::format("Edge #{}", edge_index),
                        static_cast<int>(edge_index)
                    });
                }
            }
        } else if (app_state.mesh_selection_mode == MeshSelectionMode::Face) {
            if (const auto* faces = mesh_generator.componentFaces(component_type)) {
                for (std::size_t face_index = 0; face_index < faces->size(); ++face_index) {
                    rocket::Vector3 face_world_pos = (*faces)[face_index].center_body_m;
                    face_world_pos.z += lift_z;
                    handles.push_back({
                        HandleKind::FaceFree,
                        face_world_pos,
                        app_state.selected_face_id == static_cast<int>(face_index)
                            ? Color {251, 146, 60, 255}
                            : Color {251, 146, 60, 170},
                        std::format("Face #{}", face_index),
                        static_cast<int>(face_index)
                    });
                }
            }
        }
    }

    return handles;
}

int pickHandleIndex(
    const std::vector<HandleSpec>& handles,
    const raylib::Camera3D& camera) {
    const ::Vector2 mouse = GetMousePosition();
    float best_distance = 18.0f;
    int best_index = -1;
    for (int index = 0; index < static_cast<int>(handles.size()); ++index) {
        const auto& handle = handles[static_cast<std::size_t>(index)];
        const ::Vector2 screen = GetWorldToScreen(toRaylib(handle.world_position), camera);
        const float dx = screen.x - mouse.x;
        const float dy = screen.y - mouse.y;
        const float distance = std::sqrt(dx * dx + dy * dy);
        if (distance < best_distance) {
            best_distance = distance;
            best_index = index;
        }
    }
    return best_index;
}

bool applyHandleDrag(
    HandleKind handle,
    rocket::VehicleModel& vehicle,
    MotorEditorState& motor_editor,
    float mouse_dx,
    float mouse_dy) {
    const double drag = static_cast<double>(mouse_dx - mouse_dy) * 0.004;
    if (std::abs(drag) <= 1e-9) {
        return false;
    }

    switch (handle) {
    case HandleKind::NoseLength:
        vehicle.geometry.nose_length_m -= drag * 0.5;
        return true;
    case HandleKind::NoseMidRadius:
        vehicle.geometry.nose_controls.mid_radius_scale += drag;
        return true;
    case HandleKind::NoseShoulder:
        vehicle.geometry.nose_controls.shoulder_radius_scale += drag;
        return true;
    case HandleKind::BodyLength:
        vehicle.geometry.body_length_m -= drag * 0.7;
        return true;
    case HandleKind::BodyDiameter:
        vehicle.geometry.body_diameter_m += drag * 0.08;
        return true;
    case HandleKind::BodyForeRadius:
        vehicle.geometry.body_controls.fore_radius_scale += drag;
        return true;
    case HandleKind::BodyMidRadius:
        vehicle.geometry.body_controls.mid_radius_scale += drag;
        return true;
    case HandleKind::BodyAftRadius:
        vehicle.geometry.body_controls.aft_radius_scale += drag;
        return true;
    case HandleKind::TransitionLength:
        vehicle.geometry.transition_length_m += drag * 0.4;
        return true;
    case HandleKind::TransitionAftDiameter:
        vehicle.geometry.transition_aft_diameter_m += drag * 0.07;
        return true;
    case HandleKind::TransitionMidRadius:
        vehicle.geometry.transition_controls.mid_radius_scale += drag;
        return true;
    case HandleKind::FinFront:
        vehicle.geometry.fin_front_from_nose_m += drag * 0.55;
        return true;
    case HandleKind::FinSpan:
        vehicle.geometry.fin_controls.span_scale += drag;
        return true;
    case HandleKind::FinTipLead:
        vehicle.geometry.fin_controls.tip_le_offset_m += drag * 0.18;
        return true;
    case HandleKind::FinTipTrail:
        vehicle.geometry.fin_controls.tip_te_offset_m += drag * 0.18;
        return true;
    case HandleKind::PayloadLength:
        vehicle.geometry.payload_length_m -= drag * 0.45;
        return true;
    case HandleKind::MotorMountRadius:
        motor_editor.mount_radius_m += drag * 0.08;
        return true;
    case HandleKind::VertexFree:
    case HandleKind::EdgeFree:
    case HandleKind::FaceFree:
        return false;
    case HandleKind::None:
        return false;
    }
    return false;
}

rocket::ComponentType selectedComponentType(const AppState& app_state) {
    return static_cast<rocket::ComponentType>(app_state.selection);
}

void clearMeshSelection(AppState& app_state) {
    app_state.selected_vertex_id = -1;
    app_state.selected_edge_id = -1;
    app_state.selected_face_id = -1;
}

void syncActiveMeshVertices(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    const rocket::MeshGenerator& mesh_generator) {
    auto* active_modifiers = vehicle.geometry.getActiveComponentModifiers(selectedComponentType(app_state));
    if (active_modifiers == nullptr || !active_modifiers->is_active) {
        return;
    }

    const auto* topology = mesh_generator.componentMesh(selectedComponentType(app_state));
    if (topology == nullptr) {
        return;
    }

    active_modifiers->modified_vertices.clear();
    active_modifiers->modified_vertices.reserve(topology->vertices.size());
    for (std::size_t index = 0; index < topology->vertices.size(); ++index) {
        active_modifiers->modified_vertices.push_back(rocket::FreeControlVertex {
            .vertex_id = static_cast<int>(index),
            .base_position_m = topology->vertices[index].position_body_m,
            .offset_m = {},
            .influence_radius_m = 0.02,
            .locked = false
        });
    }
    clearMeshSelection(app_state);
}

bool applyVertexDrag(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    rocket::MeshGenerator& mesh_generator,
    float mouse_dx,
    float mouse_dy) {
    auto* active_modifiers = vehicle.geometry.getActiveComponentModifiers(selectedComponentType(app_state));
    if (active_modifiers == nullptr || !active_modifiers->is_active || app_state.selected_vertex_id < 0) {
        return false;
    }

    for (auto& vertex : active_modifiers->modified_vertices) {
        if (vertex.vertex_id != app_state.selected_vertex_id || vertex.locked) {
            continue;
        }

        vertex.offset_m.x += static_cast<double>(mouse_dx) * 0.0025;
        vertex.offset_m.z -= static_cast<double>(mouse_dy) * 0.0025;

        if (app_state.snap_to_grid && app_state.grid_spacing_m > 1e-9) {
            const auto snap = [&](double value) {
                return std::round(value / app_state.grid_spacing_m) * app_state.grid_spacing_m;
            };
            vertex.offset_m.x = snap(vertex.offset_m.x);
            vertex.offset_m.y = snap(vertex.offset_m.y);
            vertex.offset_m.z = snap(vertex.offset_m.z);
        }

        const bool updated = mesh_generator.setComponentVertexPosition(
            selectedComponentType(app_state),
            vertex.vertex_id,
            vertex.base_position_m + vertex.offset_m);
        return updated;
    }

    return false;
}

bool applyTopologyOperation(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    rocket::MeshGenerator& mesh_generator,
    const std::string& operation) {
    auto* active_modifiers = vehicle.geometry.getActiveComponentModifiers(selectedComponentType(app_state));
    if (active_modifiers == nullptr || !active_modifiers->is_active) {
        return false;
    }

    bool changed = false;
    if (operation == "extrude" && app_state.selected_face_id >= 0) {
        changed = mesh_generator.extrudeComponentFace(
            selectedComponentType(app_state),
            app_state.selected_face_id,
            app_state.mesh_extrude_distance_m);
    } else if (operation == "bevel" && app_state.selected_face_id >= 0) {
        changed = mesh_generator.bevelComponentFace(
            selectedComponentType(app_state),
            app_state.selected_face_id,
            app_state.mesh_bevel_ratio,
            app_state.mesh_bevel_offset_m);
    } else if (operation == "loop_cut" && app_state.selected_edge_id >= 0) {
        changed = mesh_generator.loopCutComponentEdge(
            selectedComponentType(app_state),
            app_state.selected_edge_id);
    }

    if (changed) {
        syncActiveMeshVertices(app_state, vehicle, mesh_generator);
    }
    return changed;
}

double snapToGrid(double value, const AppState& app_state) {
    if (!app_state.snap_to_grid || app_state.grid_spacing_m <= 1e-9) {
        return value;
    }
    return std::round(value / app_state.grid_spacing_m) * app_state.grid_spacing_m;
}

void applyGridSnapToSelection(
    const AppState& app_state,
    rocket::VehicleModel& vehicle,
    MotorEditorState& motor_editor) {
    switch (app_state.selection) {
    case ComponentSelection::NoseCone:
        vehicle.geometry.nose_length_m = snapToGrid(vehicle.geometry.nose_length_m, app_state);
        break;
    case ComponentSelection::BodyTube:
        vehicle.geometry.body_length_m = snapToGrid(vehicle.geometry.body_length_m, app_state);
        vehicle.geometry.body_diameter_m = snapToGrid(vehicle.geometry.body_diameter_m, app_state);
        break;
    case ComponentSelection::Transition:
        vehicle.geometry.transition_length_m = snapToGrid(vehicle.geometry.transition_length_m, app_state);
        vehicle.geometry.transition_aft_diameter_m = snapToGrid(vehicle.geometry.transition_aft_diameter_m, app_state);
        break;
    case ComponentSelection::FinSet:
        vehicle.geometry.fin_front_from_nose_m = snapToGrid(vehicle.geometry.fin_front_from_nose_m, app_state);
        vehicle.geometry.fin_tip_chord_m = snapToGrid(vehicle.geometry.fin_tip_chord_m, app_state);
        vehicle.geometry.fin_span_m = snapToGrid(vehicle.geometry.fin_span_m, app_state);
        vehicle.geometry.fin_controls.tip_le_offset_m = snapToGrid(vehicle.geometry.fin_controls.tip_le_offset_m, app_state);
        vehicle.geometry.fin_controls.tip_te_offset_m = snapToGrid(vehicle.geometry.fin_controls.tip_te_offset_m, app_state);
        break;
    case ComponentSelection::MotorMount:
        motor_editor.mount_radius_m = snapToGrid(motor_editor.mount_radius_m, app_state);
        break;
    case ComponentSelection::Payload:
        vehicle.geometry.payload_length_m = snapToGrid(vehicle.geometry.payload_length_m, app_state);
        break;
    }
}

double selectedComponentCenterZ(
    const AppState& app_state,
    const rocket::VehicleModel& vehicle) {
    const auto& geometry = vehicle.geometry;
    const double lift_z = modelingPreviewLiftM(geometry);
    const double nose_tip_z = 0.5 * geometry.body_length_m;
    const double nose_base_z = nose_tip_z - geometry.nose_length_m;
    const double body_start_z = -0.5 * geometry.body_length_m;
    switch (app_state.selection) {
    case ComponentSelection::NoseCone:
        return nose_base_z + geometry.nose_length_m * 0.5 + lift_z;
    case ComponentSelection::BodyTube:
        return lift_z;
    case ComponentSelection::Transition:
        return body_start_z + geometry.transition_length_m * 0.5 + lift_z;
    case ComponentSelection::FinSet:
        return body_start_z + geometry.fin_front_from_nose_m + geometry.fin_root_chord_m * 0.5 + lift_z;
    case ComponentSelection::MotorMount:
        return body_start_z + 0.08 + lift_z;
    case ComponentSelection::Payload:
        return nose_base_z - geometry.payload_length_m * 0.5 + lift_z;
    }
    return 0.0;
}

void drawSelectedComponentGrid(
    const AppState& app_state,
    const rocket::VehicleModel& vehicle) {
    if (!app_state.show_local_grid) {
        return;
    }

    const double center_z = selectedComponentCenterZ(app_state, vehicle);
    const int cells = std::clamp(app_state.grid_extent_cells, 4, 40);
    const float spacing = static_cast<float>(std::max(app_state.grid_spacing_m, 0.005));
    const float half = spacing * static_cast<float>(cells);
    const float z_center = static_cast<float>(center_z);

    for (int i = -cells; i <= cells; ++i) {
        const float offset = spacing * static_cast<float>(i);
        const Color axis_color = i == 0 ? Color {248, 250, 252, 170} : (std::abs(i) % 5 == 0 ? Color {71, 85, 105, 130} : Color {51, 65, 85, 90});
        DrawLine3D(
            ::Vector3 {offset, 0.0f, z_center - half},
            ::Vector3 {offset, 0.0f, z_center + half},
            axis_color);
        DrawLine3D(
            ::Vector3 {-half, 0.0f, z_center + offset},
            ::Vector3 {half, 0.0f, z_center + offset},
            axis_color);
    }
}

void updateViewportModelingInteraction(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    MotorEditorState& motor_editor,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime,
    const raylib::Camera3D& camera) {
    if (app_state.workspace != Workspace::Modeling) {
        app_state.dragging_handle = false;
        app_state.active_handle = HandleKind::None;
        return;
    }

    if (!app_state.dragging_handle && isMouseOverModelingUi(app_state)) {
        app_state.active_handle = HandleKind::None;
        return;
    }

    const std::vector<HandleSpec> handles = buildHandleSpecs(app_state, vehicle, motor_editor, mesh_generator);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        const int picked_index = pickHandleIndex(handles, camera);
        if (picked_index >= 0) {
            const auto& picked_handle = handles[static_cast<std::size_t>(picked_index)];
            app_state.active_handle = picked_handle.kind;
            app_state.dragging_handle = true;
            if (picked_handle.kind == HandleKind::VertexFree) {
                clearMeshSelection(app_state);
                app_state.selected_vertex_id = picked_handle.topology_id;
            } else if (picked_handle.kind == HandleKind::EdgeFree) {
                clearMeshSelection(app_state);
                app_state.selected_edge_id = picked_handle.topology_id;
            } else if (picked_handle.kind == HandleKind::FaceFree) {
                clearMeshSelection(app_state);
                app_state.selected_face_id = picked_handle.topology_id;
            }
        } else {
            app_state.selection = pickComponentSelection(vehicle, motor_editor, camera, app_state.selection);
            app_state.active_handle = HandleKind::None;
            app_state.dragging_handle = false;
            clearMeshSelection(app_state);
        }
    }

    if (app_state.dragging_handle && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        const ::Vector2 delta = GetMouseDelta();
        if (app_state.active_handle == HandleKind::VertexFree) {
            applyVertexDrag(app_state, vehicle, mesh_generator, delta.x, delta.y);
        } else if (app_state.active_handle != HandleKind::EdgeFree && app_state.active_handle != HandleKind::FaceFree &&
                   applyHandleDrag(app_state.active_handle, vehicle, motor_editor, delta.x, delta.y)) {
            applyGridSnapToSelection(app_state, vehicle, motor_editor);
            if (app_state.selection == ComponentSelection::MotorMount) {
                rebuildMotorCluster(vehicle, motor_editor, mesh_generator, runtime);
            } else {
                rebuildVehicle(vehicle, mesh_generator, runtime);
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        app_state.dragging_handle = false;
    }
}

bool isRectangleClicked(const ::Rectangle& bounds) {
    const ::Vector2 mouse_position = GetMousePosition();
    return IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse_position, bounds);
}

enum class ButtonStyle {
    Contained,
    Outlined,
    Subtle
};

Color mixUiColor(Color a, Color b, float t) {
    const float clamped = std::clamp(t, 0.0f, 1.0f);
    const auto mix_channel = [clamped](unsigned char lhs, unsigned char rhs) {
        return static_cast<unsigned char>(static_cast<float>(lhs) + (static_cast<float>(rhs) - static_cast<float>(lhs)) * clamped);
    };
    return {
        mix_channel(a.r, b.r),
        mix_channel(a.g, b.g),
        mix_channel(a.b, b.b),
        mix_channel(a.a, b.a)
    };
}

void updateFloatingWindowDrag(FloatingWindowState& window) {
    if (!window.visible) {
        return;
    }

    const ::Rectangle drag_bar {window.bounds.x, window.bounds.y, window.bounds.width, 32.0f};
    const ::Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, drag_bar)) {
        window.dragging = true;
        window.drag_offset = {mouse.x - window.bounds.x, mouse.y - window.bounds.y};
    }

    if (window.dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        window.bounds.x = mouse.x - window.drag_offset.x;
        window.bounds.y = mouse.y - window.drag_offset.y;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        window.dragging = false;
    }

    const float max_x = std::max(0.0f, static_cast<float>(GetScreenWidth()) - window.bounds.width);
    const float max_y = std::max(64.0f, static_cast<float>(GetScreenHeight()) - window.bounds.height);
    window.bounds.x = std::clamp(window.bounds.x, 0.0f, max_x);
    window.bounds.y = std::clamp(window.bounds.y, 64.0f, max_y);
}

bool pointInWindow(const FloatingWindowState& window, ::Vector2 point) {
    return window.visible && CheckCollisionPointRec(point, window.bounds);
}

bool isMouseOverModelingUi(const AppState& app_state) {
    const ::Vector2 mouse = GetMousePosition();
    return pointInWindow(app_state.layout.modeling_toolbar, mouse) ||
           pointInWindow(app_state.layout.modeling_outliner, mouse) ||
           pointInWindow(app_state.layout.modeling_library, mouse) ||
           pointInWindow(app_state.layout.modeling_properties, mouse) ||
           pointInWindow(app_state.layout.modeling_status, mouse) ||
           pointInWindow(app_state.layout.modeling_reference, mouse);
}

bool drawButton(
    const ::Rectangle& bounds,
    const std::string& text,
    bool active,
    Color accent = Color {61, 135, 245, 228},
    ButtonStyle style = ButtonStyle::Outlined) {
    const ::Vector2 mouse_position = GetMousePosition();
    const bool hovered = CheckCollisionPointRec(mouse_position, bounds);
    const float roundness = 0.28f;

    Color background {};
    Color border {};
    Color text_color {233, 239, 247, 255};

    switch (style) {
    case ButtonStyle::Contained:
        background = active ? accent : (hovered ? mixUiColor(accent, Color {255, 255, 255, 255}, 0.12f) : accent);
        border = active ? Color {244, 247, 251, 220} : mixUiColor(accent, Color {255, 255, 255, 255}, hovered ? 0.20f : 0.10f);
        text_color = Color {248, 250, 252, 255};
        break;
    case ButtonStyle::Subtle:
        background = active ? Color {30, 64, 107, 188} : (hovered ? Color {29, 41, 62, 214} : Color {17, 24, 39, 178});
        border = active ? accent : (hovered ? Color {99, 123, 157, 220} : Color {66, 79, 99, 170});
        text_color = active ? Color {244, 247, 251, 255} : Color {216, 224, 234, 255};
        break;
    case ButtonStyle::Outlined:
    default:
        background = active ? Color {22, 41, 67, 214} : (hovered ? Color {23, 34, 53, 214} : Color {14, 21, 33, 196});
        border = active ? accent : (hovered ? Color {111, 143, 188, 232} : Color {78, 93, 116, 196});
        text_color = active ? Color {242, 246, 250, 255} : Color {221, 229, 238, 255};
        break;
    }

    DrawRectangleRounded(bounds, roundness, 10, background);
    DrawRectangleRoundedLinesEx(bounds, roundness, 10, 1.4f, border);
    const int font_size = 16;
    const int text_width = MeasureText(text.c_str(), font_size);
    DrawText(
        text.c_str(),
        static_cast<int>(bounds.x + (bounds.width - static_cast<float>(text_width)) * 0.5f),
        static_cast<int>(bounds.y + (bounds.height - static_cast<float>(font_size)) * 0.5f),
        font_size,
        text_color);
    return isRectangleClicked(bounds);
}
