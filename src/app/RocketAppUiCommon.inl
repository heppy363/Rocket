void drawPanel(const ::Rectangle& bounds, const std::string& title, Color fill = Color {15, 23, 42, 228}) {
    DrawRectangleRounded(bounds, 0.14f, 10, fill);
    DrawRectangleRoundedLinesEx(bounds, 0.14f, 10, 1.2f, Color {66, 79, 99, 220});
    DrawRectangleRounded(Rectangle {bounds.x + 1.5f, bounds.y + 1.5f, bounds.width - 3.0f, 38.0f}, 0.14f, 10, Color {24, 32, 48, 220});
    DrawRectangleGradientH(
        static_cast<int>(bounds.x) + 18,
        static_cast<int>(bounds.y) + 15,
        static_cast<int>(bounds.width) - 36,
        2,
        Color {120, 172, 255, 190},
        Color {120, 172, 255, 18});
    DrawText(title.c_str(), static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 11, 18, Color {239, 243, 248, 255});
}
std::vector<std::string> wrapTextLines(
    const std::string& text,
    int font_size,
    float max_width,
    int max_lines = 0) {
    if (text.empty() || max_width <= 4.0f) {
        return {};
    }

    std::vector<std::string> lines;
    std::string current;
    std::string word;

    const auto flush_word = [&](bool force_newline = false) {
        if (word.empty() && !force_newline) {
            return;
        }

        if (force_newline) {
            if (!current.empty()) {
                lines.push_back(current);
                current.clear();
            } else {
                lines.emplace_back();
            }
            word.clear();
            return;
        }

        const std::string candidate = current.empty() ? word : (current + " " + word);
        if (MeasureText(candidate.c_str(), font_size) <= max_width) {
            current = candidate;
        } else {
            if (!current.empty()) {
                lines.push_back(current);
                current = word;
            } else {
                std::string trimmed = word;
                while (!trimmed.empty() && MeasureText((trimmed + "...").c_str(), font_size) > max_width) {
                    trimmed.pop_back();
                }
                current = trimmed.empty() ? word.substr(0, 1) : (trimmed + "...");
            }
        }
        word.clear();
    };

    for (const char ch : text) {
        if (ch == '\n') {
            flush_word();
            flush_word(true);
        } else if (std::isspace(static_cast<unsigned char>(ch))) {
            flush_word();
        } else {
            word.push_back(ch);
        }
    }
    flush_word();
    if (!current.empty()) {
        lines.push_back(current);
    }

    if (max_lines > 0 && static_cast<int>(lines.size()) > max_lines) {
        lines.resize(static_cast<std::size_t>(max_lines));
        std::string& last = lines.back();
        while (!last.empty() && MeasureText((last + "...").c_str(), font_size) > max_width) {
            last.pop_back();
        }
        last += "...";
    }

    return lines;
}

void drawWrappedText(
    const ::Rectangle& bounds,
    const std::string& text,
    int font_size,
    Color color,
    int max_lines = 0,
    float line_spacing = 3.0f) {
    if (text.empty()) {
        return;
    }

    const auto lines = wrapTextLines(text, font_size, bounds.width, max_lines);
    float y = bounds.y;
    for (const auto& line : lines) {
        if (y + static_cast<float>(font_size) > bounds.y + bounds.height) {
            break;
        }
        DrawText(line.c_str(), static_cast<int>(bounds.x), static_cast<int>(y), font_size, color);
        y += static_cast<float>(font_size) + line_spacing;
    }
}
void drawSingleLineClippedText(
    const ::Rectangle& bounds,
    const std::string& text,
    int font_size,
    Color color) {
    auto lines = wrapTextLines(text, font_size, bounds.width, 1);
    if (!lines.empty()) {
        DrawText(lines.front().c_str(), static_cast<int>(bounds.x), static_cast<int>(bounds.y), font_size, color);
    }
}
void drawKeyValueLine(const ::Rectangle& bounds, const std::string& label, const std::string& value) {
    const float split_x = bounds.x + std::min(126.0f, bounds.width * 0.42f);
    drawSingleLineClippedText(
        Rectangle {bounds.x, bounds.y, split_x - bounds.x - 8.0f, bounds.height},
        label,
        16,
        Color {148, 163, 184, 255});
    drawSingleLineClippedText(
        Rectangle {split_x, bounds.y, bounds.width - (split_x - bounds.x), bounds.height},
        value,
        16,
        Color {226, 232, 240, 255});
}
void drawSectionCaption(const ::Rectangle& bounds, const std::string& title, const std::string& subtitle = {}) {
    drawSingleLineClippedText(
        Rectangle {bounds.x, bounds.y, bounds.width, 18.0f},
        title,
        15,
        Color {230, 236, 243, 255});
    if (!subtitle.empty()) {
        drawWrappedText(
            Rectangle {bounds.x, bounds.y + 20.0f, bounds.width, std::max(bounds.height - 20.0f, 14.0f)},
            subtitle,
            13,
            Color {136, 149, 167, 255},
            2);
    }
}

void drawMetricCard(const ::Rectangle& bounds, const std::string& label, const std::string& value, Color accent) {
    DrawRectangleRounded(bounds, 0.24f, 8, Color {18, 26, 40, 232});
    DrawRectangleRoundedLinesEx(bounds, 0.24f, 8, 1.0f, Color {67, 81, 102, 230});
    DrawRectangle(static_cast<int>(bounds.x) + 1, static_cast<int>(bounds.y) + 1, 4, static_cast<int>(bounds.height) - 2, accent);
    DrawText(label.c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 8, 13, Color {149, 161, 178, 255});
    DrawText(value.c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 29, 18, Color {241, 245, 249, 255});
}

void drawStatusChip(const ::Rectangle& bounds, const std::string& text, Color fill, Color border) {
    DrawRectangleRounded(bounds, 0.35f, 8, fill);
    DrawRectangleRoundedLinesEx(bounds, 0.35f, 8, 1.1f, border);
    drawSingleLineClippedText(
        Rectangle {bounds.x + 10.0f, bounds.y + 6.0f, bounds.width - 20.0f, bounds.height - 12.0f},
        text,
        14,
        Color {241, 245, 249, 255});
}

void drawInlineHint(const ::Rectangle& bounds, const std::string& text, Color accent) {
    DrawRectangleRounded(bounds, 0.28f, 8, Color {18, 26, 38, 220});
    DrawRectangleRoundedLinesEx(bounds, 0.28f, 8, 1.0f, Color {65, 80, 102, 210});
    DrawRectangle(static_cast<int>(bounds.x) + 1, static_cast<int>(bounds.y) + 1, 4, static_cast<int>(bounds.height) - 2, accent);
    drawWrappedText(
        Rectangle {bounds.x + 12.0f, bounds.y + 6.0f, bounds.width - 22.0f, bounds.height - 10.0f},
        text,
        13,
        Color {203, 213, 225, 255},
        3);
}

void drawInfoCard(const ::Rectangle& bounds, const std::string& title, const std::string& body, Color accent) {
    DrawRectangleRounded(bounds, 0.18f, 10, Color {16, 24, 38, 232});
    DrawRectangleRoundedLinesEx(bounds, 0.18f, 10, 1.2f, Color {65, 80, 100, 226});
    DrawRectangle(static_cast<int>(bounds.x) + 1, static_cast<int>(bounds.y) + 1, 5, static_cast<int>(bounds.height) - 2, accent);
    drawSingleLineClippedText(
        Rectangle {bounds.x + 14.0f, bounds.y + 10.0f, bounds.width - 24.0f, 18.0f},
        title,
        15,
        Color {241, 245, 249, 255});
    drawWrappedText(
        Rectangle {bounds.x + 14.0f, bounds.y + 30.0f, bounds.width - 24.0f, bounds.height - 38.0f},
        body,
        13,
        Color {148, 163, 184, 255},
        3);
}

int drawStepper(const ::Rectangle& bounds, const std::string& label, const std::string& value) {
    const float right_buttons_w = 72.0f;
    const float value_x = bounds.x + std::min(144.0f, bounds.width * 0.46f);
    drawSingleLineClippedText(
        Rectangle {bounds.x, bounds.y + 6.0f, value_x - bounds.x - 8.0f, 18.0f},
        label,
        16,
        Color {148, 163, 184, 255});
    drawSingleLineClippedText(
        Rectangle {value_x, bounds.y + 6.0f, bounds.width - (value_x - bounds.x) - right_buttons_w - 8.0f, 18.0f},
        value,
        16,
        Color {241, 245, 249, 255});

    const ::Rectangle minus_button {bounds.x + bounds.width - 68.0f, bounds.y, 28.0f, 28.0f};
    const ::Rectangle plus_button {bounds.x + bounds.width - 34.0f, bounds.y, 28.0f, 28.0f};
    int result = 0;
    if (drawButton(minus_button, "-", false, Color {71, 85, 105, 220})) {
        result = -1;
    }
    if (drawButton(plus_button, "+", false, Color {71, 85, 105, 220})) {
        result = 1;
    }
    return result;
}

void drawStabilityMarkers(
    const rocket::VehicleModel& vehicle,
    const rocket::FlightState& state,
    const rocket::ForceResult& force_result) {
    const rocket::Vector3 cg_world = worldPointFromNoseStation(vehicle, state, force_result.center_of_gravity_from_nose_m);
    const rocket::Vector3 cp_world = worldPointFromNoseStation(vehicle, state, force_result.center_of_pressure_from_nose_m);
    const rocket::Vector3 body_axis_tip = state.position_m + rocket::rotateVector(
        state.attitude_body_to_world,
        {0.0, 0.0, vehicle.geometry.body_length_m * 0.6});

    DrawSphere(toRaylib(cg_world), 0.055f, Color {34, 197, 94, 210});
    DrawSphereWires(toRaylib(cg_world), 0.085f, 8, 8, Color {34, 197, 94, 180});
    DrawSphere(toRaylib(cp_world), 0.055f, Color {239, 68, 68, 210});
    DrawSphereWires(toRaylib(cp_world), 0.085f, 8, 8, Color {239, 68, 68, 180});
    DrawLine3D(toRaylib(cg_world), toRaylib(cp_world), Color {251, 191, 36, 255});
    DrawLine3D(toRaylib(state.position_m), toRaylib(body_axis_tip), Color {96, 165, 250, 255});
}

void drawRingCage(double radius_m, double z_m, Color color, int segments = 18) {
    const float radius = static_cast<float>(std::max(radius_m, 0.001));
    const float z = static_cast<float>(z_m);
    for (int index = 0; index < segments; ++index) {
        const float a0 = static_cast<float>((2.0 * pi * static_cast<double>(index)) / static_cast<double>(segments));
        const float a1 = static_cast<float>((2.0 * pi * static_cast<double>(index + 1)) / static_cast<double>(segments));
        DrawLine3D(
            ::Vector3 {radius * std::cos(a0), radius * std::sin(a0), z},
            ::Vector3 {radius * std::cos(a1), radius * std::sin(a1), z},
            color);
    }
}

void drawSelectedComponentEditCage(
    const AppState& app_state,
    const rocket::VehicleModel& vehicle,
    const MotorEditorState& motor_editor) {
    if (app_state.workspace != Workspace::Modeling || !app_state.show_snap_points) {
        return;
    }

    const auto& geometry = vehicle.geometry;
    const double lift_z = modelingPreviewLiftM(geometry);
    const double nose_tip_z = 0.5 * geometry.body_length_m + lift_z;
    const double nose_base_z = nose_tip_z - geometry.nose_length_m;
    const double body_start_z = -0.5 * geometry.body_length_m + lift_z;
    const double body_radius = geometry.body_diameter_m * 0.5;
    const double body_end_z = nose_base_z;
    const double aft_radius = geometry.transition_aft_diameter_m * 0.5;
    const double fin_front_z = body_start_z + geometry.fin_front_from_nose_m;
    const double fin_back_z = fin_front_z + geometry.fin_root_chord_m;
    const double fin_tip_front_z = fin_front_z + geometry.fin_sweep_length_m + geometry.fin_controls.tip_le_offset_m;
    const double fin_tip_back_z = fin_tip_front_z + geometry.fin_tip_chord_m + geometry.fin_controls.tip_te_offset_m;
    const double fin_outer_radius = body_radius + geometry.fin_span_m * geometry.fin_controls.span_scale;

    const auto cage_color = [&]() {
        switch (app_state.selection) {
        case ComponentSelection::NoseCone:
            return Color {251, 191, 36, 210};
        case ComponentSelection::BodyTube:
            return Color {96, 165, 250, 210};
        case ComponentSelection::Transition:
            return Color {192, 132, 252, 210};
        case ComponentSelection::FinSet:
            return Color {244, 114, 182, 210};
        case ComponentSelection::MotorMount:
            return Color {74, 222, 128, 210};
        case ComponentSelection::Payload:
            return Color {125, 211, 252, 210};
        }
        return Color {241, 245, 249, 210};
    }();

    switch (app_state.selection) {
    case ComponentSelection::NoseCone: {
        drawRingCage(body_radius * geometry.nose_controls.shoulder_radius_scale, nose_base_z + geometry.nose_length_m * 0.1, cage_color, 16);
        drawRingCage(body_radius * geometry.nose_controls.mid_radius_scale, nose_base_z + geometry.nose_length_m * 0.55, cage_color, 16);
        for (int i = 0; i < 8; ++i) {
            const double angle = (2.0 * pi * static_cast<double>(i)) / 8.0;
            const double cs = std::cos(angle);
            const double sn = std::sin(angle);
            DrawLine3D(
                ::Vector3 {0.0f, 0.0f, static_cast<float>(nose_tip_z)},
                ::Vector3 {
                    static_cast<float>(body_radius * geometry.nose_controls.shoulder_radius_scale * cs),
                    static_cast<float>(body_radius * geometry.nose_controls.shoulder_radius_scale * sn),
                    static_cast<float>(nose_base_z + geometry.nose_length_m * 0.1)},
                cage_color);
        }
        break;
    }
    case ComponentSelection::BodyTube: {
        const double fore_radius = body_radius * geometry.body_controls.fore_radius_scale;
        const double mid_radius = body_radius * geometry.body_controls.mid_radius_scale;
        const double aft_radius_local = body_radius * geometry.body_controls.aft_radius_scale;
        const double fore_z = body_end_z - 0.12;
        const double mid_z = lift_z;
        const double aft_z = body_start_z + geometry.transition_length_m + 0.08;
        drawRingCage(fore_radius, fore_z, cage_color, 18);
        drawRingCage(mid_radius, mid_z, cage_color, 18);
        drawRingCage(aft_radius_local, aft_z, cage_color, 18);
        for (int i = 0; i < 12; ++i) {
            const double angle = (2.0 * pi * static_cast<double>(i)) / 12.0;
            const double cs = std::cos(angle);
            const double sn = std::sin(angle);
            DrawLine3D(
                ::Vector3 {static_cast<float>(fore_radius * cs), static_cast<float>(fore_radius * sn), static_cast<float>(fore_z)},
                ::Vector3 {static_cast<float>(mid_radius * cs), static_cast<float>(mid_radius * sn), static_cast<float>(mid_z)},
                cage_color);
            DrawLine3D(
                ::Vector3 {static_cast<float>(mid_radius * cs), static_cast<float>(mid_radius * sn), static_cast<float>(mid_z)},
                ::Vector3 {static_cast<float>(aft_radius_local * cs), static_cast<float>(aft_radius_local * sn), static_cast<float>(aft_z)},
                cage_color);
        }
        break;
    }
    case ComponentSelection::Transition: {
        const double mid_radius = aft_radius * geometry.transition_controls.mid_radius_scale;
        const double mid_z = body_start_z + geometry.transition_length_m * 0.5;
        drawRingCage(body_radius, body_end_z - 0.02, cage_color, 16);
        drawRingCage(mid_radius, mid_z, cage_color, 16);
        drawRingCage(aft_radius, body_start_z + 0.02, cage_color, 16);
        for (int i = 0; i < 10; ++i) {
            const double angle = (2.0 * pi * static_cast<double>(i)) / 10.0;
            const double cs = std::cos(angle);
            const double sn = std::sin(angle);
            DrawLine3D(
                ::Vector3 {static_cast<float>(body_radius * cs), static_cast<float>(body_radius * sn), static_cast<float>(body_end_z - 0.02)},
                ::Vector3 {static_cast<float>(mid_radius * cs), static_cast<float>(mid_radius * sn), static_cast<float>(mid_z)},
                cage_color);
            DrawLine3D(
                ::Vector3 {static_cast<float>(mid_radius * cs), static_cast<float>(mid_radius * sn), static_cast<float>(mid_z)},
                ::Vector3 {static_cast<float>(aft_radius * cs), static_cast<float>(aft_radius * sn), static_cast<float>(body_start_z + 0.02)},
                cage_color);
        }
        break;
    }
    case ComponentSelection::FinSet: {
        for (int fin_index = 0; fin_index < geometry.fin_count; ++fin_index) {
            const double angle = (2.0 * pi * static_cast<double>(fin_index)) / static_cast<double>(geometry.fin_count);
            const double cs = std::cos(angle);
            const double sn = std::sin(angle);
            const ::Vector3 root_front {static_cast<float>(body_radius * cs), static_cast<float>(body_radius * sn), static_cast<float>(fin_front_z)};
            const ::Vector3 root_back {static_cast<float>(body_radius * cs), static_cast<float>(body_radius * sn), static_cast<float>(fin_back_z)};
            const ::Vector3 tip_front {static_cast<float>(fin_outer_radius * cs), static_cast<float>(fin_outer_radius * sn), static_cast<float>(fin_tip_front_z)};
            const ::Vector3 tip_back {static_cast<float>(fin_outer_radius * cs), static_cast<float>(fin_outer_radius * sn), static_cast<float>(fin_tip_back_z)};
            DrawLine3D(root_front, root_back, cage_color);
            DrawLine3D(root_front, tip_front, cage_color);
            DrawLine3D(root_back, tip_back, cage_color);
            DrawLine3D(tip_front, tip_back, cage_color);
        }
        break;
    }
    case ComponentSelection::MotorMount: {
        const double mount_z = body_start_z + 0.08;
        drawRingCage(std::max(motor_editor.mount_radius_m, 0.02), mount_z, cage_color, std::max(motor_editor.motor_count, 8));
        break;
    }
    case ComponentSelection::Payload: {
        const double payload_radius = std::max(body_radius * 0.82, 0.02);
        const double payload_mid_z = nose_base_z - geometry.payload_length_m * 0.5;
        const double payload_end_z = nose_base_z - geometry.payload_length_m + 0.02;
        drawRingCage(payload_radius, payload_mid_z, cage_color, 14);
        drawRingCage(payload_radius, payload_end_z, cage_color, 14);
        for (int i = 0; i < 8; ++i) {
            const double angle = (2.0 * pi * static_cast<double>(i)) / 8.0;
            const double cs = std::cos(angle);
            const double sn = std::sin(angle);
            DrawLine3D(
                ::Vector3 {static_cast<float>(payload_radius * cs), static_cast<float>(payload_radius * sn), static_cast<float>(payload_mid_z)},
                ::Vector3 {static_cast<float>(payload_radius * cs), static_cast<float>(payload_radius * sn), static_cast<float>(payload_end_z)},
                cage_color);
        }
        break;
    }
    }
}

void drawModelingHandles3D(
    const AppState& app_state,
    const rocket::VehicleModel& vehicle,
    const MotorEditorState& motor_editor,
    const rocket::MeshGenerator& mesh_generator) {
    if (app_state.workspace != Workspace::Modeling || !app_state.show_snap_points) {
        return;
    }

    const double lift_z = modelingPreviewLiftM(vehicle.geometry);
    const auto component_type = selectedComponentType(app_state);
    if (app_state.mesh_selection_mode == MeshSelectionMode::Edge &&
        app_state.selected_edge_id >= 0) {
        const auto* topology = mesh_generator.componentMesh(component_type);
        const auto* edges = mesh_generator.componentEdges(component_type);
        if (topology != nullptr && edges != nullptr &&
            static_cast<std::size_t>(app_state.selected_edge_id) < edges->size()) {
            const auto& edge = (*edges)[static_cast<std::size_t>(app_state.selected_edge_id)];
            rocket::Vector3 a = topology->vertices[edge.vertex_a].position_body_m;
            rocket::Vector3 b = topology->vertices[edge.vertex_b].position_body_m;
            a.z += lift_z;
            b.z += lift_z;
            DrawLine3D(toRaylib(a), toRaylib(b), Color {125, 211, 252, 255});
        }
    }
    if (app_state.mesh_selection_mode == MeshSelectionMode::Face &&
        app_state.selected_face_id >= 0) {
        const auto* topology = mesh_generator.componentMesh(component_type);
        const auto* faces = mesh_generator.componentFaces(component_type);
        if (topology != nullptr && faces != nullptr &&
            static_cast<std::size_t>(app_state.selected_face_id) < faces->size()) {
            const auto& face = (*faces)[static_cast<std::size_t>(app_state.selected_face_id)];
            rocket::Vector3 a = topology->vertices[face.vertex_a].position_body_m;
            rocket::Vector3 b = topology->vertices[face.vertex_b].position_body_m;
            rocket::Vector3 c = topology->vertices[face.vertex_c].position_body_m;
            a.z += lift_z;
            b.z += lift_z;
            c.z += lift_z;
            DrawLine3D(toRaylib(a), toRaylib(b), Color {251, 191, 36, 255});
            DrawLine3D(toRaylib(b), toRaylib(c), Color {251, 191, 36, 255});
            DrawLine3D(toRaylib(c), toRaylib(a), Color {251, 191, 36, 255});
        }
    }
}

void drawModelingHandlesOverlay(
    const AppState& app_state,
    const rocket::VehicleModel& vehicle,
    const MotorEditorState& motor_editor,
    const rocket::MeshGenerator& mesh_generator,
    const raylib::Camera3D& camera) {
    if (app_state.workspace != Workspace::Modeling || !app_state.show_snap_points) {
        return;
    }

    for (const auto& handle : app_state.modeling_handle_cache) {
        const ::Vector2 screen = GetWorldToScreen(toRaylib(handle.world_position), camera);
        const bool selected =
            (handle.kind == HandleKind::VertexFree && handle.topology_id == app_state.selected_vertex_id) ||
            (handle.kind == HandleKind::EdgeFree && handle.topology_id == app_state.selected_edge_id) ||
            (handle.kind == HandleKind::FaceFree && handle.topology_id == app_state.selected_face_id) ||
            (handle.kind != HandleKind::VertexFree &&
             handle.kind != HandleKind::EdgeFree &&
             handle.kind != HandleKind::FaceFree &&
             app_state.active_handle == handle.kind);

        if (handle.kind == HandleKind::VertexFree) {
            const float radius = selected ? 5.0f : 3.2f;
            DrawCircleV(screen, radius, ColorAlpha(handle.color, selected ? 0.98f : 0.88f));
            DrawCircleLines(
                static_cast<int>(screen.x),
                static_cast<int>(screen.y),
                radius + 1.0f,
                Color {241, 245, 249, static_cast<unsigned char>(selected ? 255 : 170)});
        } else if (handle.kind == HandleKind::EdgeFree) {
            const float radius = selected ? 5.8f : 4.0f;
            DrawRectanglePro(
                Rectangle {screen.x - radius, screen.y - radius, radius * 2.0f, radius * 2.0f},
                Vector2 {0.0f, 0.0f},
                45.0f,
                ColorAlpha(handle.color, selected ? 0.98f : 0.82f));
            DrawRectangleLinesEx(
                Rectangle {screen.x - radius - 1.0f, screen.y - radius - 1.0f, radius * 2.0f + 2.0f, radius * 2.0f + 2.0f},
                1.0f,
                Color {241, 245, 249, static_cast<unsigned char>(selected ? 240 : 150)});
        } else if (handle.kind == HandleKind::FaceFree) {
            const float radius = selected ? 6.2f : 4.6f;
            DrawPoly(screen, 3, radius, -90.0f, ColorAlpha(handle.color, selected ? 0.98f : 0.82f));
            DrawPolyLines(
                screen,
                3,
                radius + 1.2f,
                -90.0f,
                Color {241, 245, 249, static_cast<unsigned char>(selected ? 240 : 150)});
        } else {
            const float half = selected ? 5.2f : 4.0f;
            const float arm = selected ? 7.0f : 5.0f;
            const Color fill = ColorAlpha(handle.color, selected ? 0.96f : 0.78f);
            DrawRectangleV(
                Vector2 {screen.x - half, screen.y - half},
                Vector2 {half * 2.0f, half * 2.0f},
                fill);
            DrawRectangleLinesEx(
                Rectangle {screen.x - half - 1.0f, screen.y - half - 1.0f, half * 2.0f + 2.0f, half * 2.0f + 2.0f},
                1.0f,
                Color {241, 245, 249, static_cast<unsigned char>(selected ? 235 : 150)});
            DrawLineEx(
                Vector2 {screen.x - arm, screen.y},
                Vector2 {screen.x + arm, screen.y},
                selected ? 2.0f : 1.0f,
                Color {241, 245, 249, static_cast<unsigned char>(selected ? 215 : 120)});
            DrawLineEx(
                Vector2 {screen.x, screen.y - arm},
                Vector2 {screen.x, screen.y + arm},
                selected ? 2.0f : 1.0f,
                Color {241, 245, 249, static_cast<unsigned char>(selected ? 215 : 120)});
        }
    }
}

void drawModelingHandleLabels(
    const AppState& app_state,
    const rocket::VehicleModel& vehicle,
    const MotorEditorState& motor_editor,
    const rocket::MeshGenerator& mesh_generator,
    const raylib::Camera3D& camera) {
    if (app_state.workspace != Workspace::Modeling || !app_state.show_snap_points) {
        return;
    }

    for (const auto& handle : app_state.modeling_handle_cache) {
        const bool selected =
            (handle.kind == HandleKind::VertexFree && handle.topology_id == app_state.selected_vertex_id) ||
            (handle.kind == HandleKind::EdgeFree && handle.topology_id == app_state.selected_edge_id) ||
            (handle.kind == HandleKind::FaceFree && handle.topology_id == app_state.selected_face_id) ||
            (handle.kind != HandleKind::VertexFree && handle.kind != HandleKind::EdgeFree &&
             handle.kind != HandleKind::FaceFree && app_state.active_handle != HandleKind::None &&
             app_state.active_handle == handle.kind);
        if (!selected) {
            continue;
        }
        const ::Vector2 screen = GetWorldToScreen(toRaylib(handle.world_position), camera);
        drawSingleLineClippedText(
            Rectangle {screen.x + 10.0f, screen.y - 8.0f, 160.0f, 16.0f},
            handle.label,
            15,
            Color {248, 250, 252, 255});
    }
}

void drawModelingViewportHeader(
    const AppState& app_state,
    const rocket::VehicleModel& vehicle) {
    const ::Rectangle header {444.0f, 84.0f, 664.0f, 74.0f};
    DrawRectangleRounded(header, 0.14f, 8, Color {11, 18, 30, 224});
    DrawRectangleRoundedLinesEx(header, 0.14f, 8, 1.2f, Color {68, 82, 103, 225});

    constexpr std::array<const char*, 6> selection_names {
        "Nose Cone",
        "Body Tube",
        "Transition",
        "Fin Set",
        "Motor Mount",
        "Payload Section"
    };
    constexpr std::array<const char*, 4> view_names {
        "Perspective",
        "Front",
        "Side",
        "Top"
    };

    drawSingleLineClippedText(
        Rectangle {header.x + 18.0f, header.y + 12.0f, 194.0f, 22.0f},
        selection_names[static_cast<std::size_t>(app_state.selection)],
        20,
        Color {248, 250, 252, 255});
    drawSingleLineClippedText(
        Rectangle {header.x + 18.0f, header.y + 38.0f, 194.0f, 14.0f},
        "Elemento attivo nella viewport di modellazione",
        13,
        Color {124, 139, 160, 255});

    drawStatusChip(
        Rectangle {header.x + 232.0f, header.y + 14.0f, 118.0f, 28.0f},
        std::format("Tool  {}", modelingToolLabel(app_state.modeling_tool)),
        Color {21, 56, 91, 220},
        Color {100, 169, 255, 220});
    drawStatusChip(
        Rectangle {header.x + 362.0f, header.y + 14.0f, 118.0f, 28.0f},
        std::format("View  {}", view_names[static_cast<std::size_t>(app_state.modeling_view)]),
        Color {31, 48, 72, 220},
        Color {128, 167, 216, 220});
    drawStatusChip(
        Rectangle {header.x + 492.0f, header.y + 14.0f, 114.0f, 28.0f},
        std::format("Grid  {:.3f} m", app_state.grid_spacing_m),
        Color {37, 43, 54, 220},
        Color {109, 118, 132, 220});
    drawStatusChip(
        Rectangle {header.x + 618.0f, header.y + 14.0f, 84.0f, 28.0f},
        app_state.snap_to_grid ? "Snap" : "Free",
        app_state.snap_to_grid ? Color {22, 66, 46, 220} : Color {82, 38, 38, 220},
        app_state.snap_to_grid ? Color {97, 210, 138, 220} : Color {241, 136, 136, 220});
    drawStatusChip(
        Rectangle {header.x + 714.0f, header.y + 14.0f, 92.0f, 28.0f},
        std::format("Mesh  {}", meshSelectionModeLabel(app_state.mesh_selection_mode)),
        Color {38, 55, 78, 220},
        Color {125, 211, 252, 220});
    drawSingleLineClippedText(
        Rectangle {header.x + 232.0f, header.y + 48.0f, header.width - 250.0f, 14.0f},
        std::format(
            "{} {}  |  Handle {}  |  {}  |  Corpo {:.2f} m x {:.3f} m",
            modelingToolLabel(app_state.modeling_tool),
            modelingToolShortcut(app_state.modeling_tool),
            handleKindLabel(app_state.active_handle),
            app_state.camera_orbit ? "CAD Nav TAB" : "CAD Nav OFF",
            vehicle.geometry.body_length_m,
            vehicle.geometry.body_diameter_m),
        13,
        Color {149, 161, 178, 255});
}

void drawReferenceBlueprintInViewport(const AppState& app_state, const rocket::VehicleModel& vehicle) {
    if (app_state.workspace != Workspace::Modeling || !app_state.show_reference_blueprint) {
        return;
    }

    const float z_plane = static_cast<float>(app_state.reference_board_offset_m);
    const float board_half_width = static_cast<float>(std::max(vehicle.geometry.body_diameter_m * app_state.reference_board_scale_m, 0.6));
    const float board_half_height = static_cast<float>(std::max(vehicle.geometry.body_length_m * 0.55, 1.2));
    const float body_radius = static_cast<float>(vehicle.geometry.body_diameter_m * 0.5);
    const float tail_radius = static_cast<float>(vehicle.geometry.transition_aft_diameter_m * 0.5);
    const float nose_base_y = static_cast<float>(0.5 * vehicle.geometry.body_length_m - vehicle.geometry.nose_length_m);
    const float nose_tip_y = static_cast<float>(0.5 * vehicle.geometry.body_length_m);
    const float body_start_y = static_cast<float>(-0.5 * vehicle.geometry.body_length_m + vehicle.geometry.transition_length_m);
    const float fin_front_y = static_cast<float>(-0.5 * vehicle.geometry.body_length_m + vehicle.geometry.fin_front_from_nose_m);
    const float fin_back_y = fin_front_y + static_cast<float>(vehicle.geometry.fin_root_chord_m);
    const float fin_tip_y = fin_front_y + static_cast<float>(vehicle.geometry.fin_sweep_length_m + vehicle.geometry.fin_controls.tip_le_offset_m);
    const float fin_tip_back_y = fin_tip_y + static_cast<float>(vehicle.geometry.fin_tip_chord_m + vehicle.geometry.fin_controls.tip_te_offset_m);
    const float fin_outer = body_radius + static_cast<float>(vehicle.geometry.fin_span_m * vehicle.geometry.fin_controls.span_scale);

    const Color board_color {45, 78, 112, 38};
    const Color guide_color {83, 139, 196, 120};
    const Color sketch_color {140, 210, 255, 180};

    DrawCubeV(
        ::Vector3 {0.0f, 0.0f, z_plane},
        ::Vector3 {board_half_width * 2.0f, board_half_height * 2.0f, 0.01f},
        board_color);

    for (int i = -4; i <= 4; ++i) {
        const float x = board_half_width * static_cast<float>(i) / 4.0f;
        DrawLine3D(
            ::Vector3 {x, -board_half_height, z_plane},
            ::Vector3 {x, board_half_height, z_plane},
            guide_color);
    }
    for (int i = -6; i <= 6; ++i) {
        const float y = board_half_height * static_cast<float>(i) / 6.0f;
        DrawLine3D(
            ::Vector3 {-board_half_width, y, z_plane},
            ::Vector3 {board_half_width, y, z_plane},
            guide_color);
    }

    DrawLine3D(::Vector3 {0.0f, -board_half_height, z_plane}, ::Vector3 {0.0f, board_half_height, z_plane}, Color {120, 170, 220, 210});
    DrawLine3D(::Vector3 {-board_half_width, 0.0f, z_plane}, ::Vector3 {board_half_width, 0.0f, z_plane}, Color {120, 170, 220, 150});

    DrawLine3D(::Vector3 {0.0f, nose_tip_y, z_plane}, ::Vector3 {-body_radius, nose_base_y, z_plane}, sketch_color);
    DrawLine3D(::Vector3 {0.0f, nose_tip_y, z_plane}, ::Vector3 {body_radius, nose_base_y, z_plane}, sketch_color);
    DrawLine3D(::Vector3 {-body_radius, nose_base_y, z_plane}, ::Vector3 {-body_radius, body_start_y, z_plane}, sketch_color);
    DrawLine3D(::Vector3 {body_radius, nose_base_y, z_plane}, ::Vector3 {body_radius, body_start_y, z_plane}, sketch_color);
    DrawLine3D(::Vector3 {-body_radius, body_start_y, z_plane}, ::Vector3 {-tail_radius, -0.5f * static_cast<float>(vehicle.geometry.body_length_m), z_plane}, sketch_color);
    DrawLine3D(::Vector3 {body_radius, body_start_y, z_plane}, ::Vector3 {tail_radius, -0.5f * static_cast<float>(vehicle.geometry.body_length_m), z_plane}, sketch_color);
    DrawLine3D(::Vector3 {-fin_outer, fin_tip_y, z_plane}, ::Vector3 {-body_radius, fin_front_y, z_plane}, sketch_color);
    DrawLine3D(::Vector3 {-fin_outer, fin_tip_back_y, z_plane}, ::Vector3 {-body_radius, fin_back_y, z_plane}, sketch_color);
    DrawLine3D(::Vector3 {-fin_outer, fin_tip_y, z_plane}, ::Vector3 {-fin_outer, fin_tip_back_y, z_plane}, sketch_color);
    DrawLine3D(::Vector3 {fin_outer, fin_tip_y, z_plane}, ::Vector3 {body_radius, fin_front_y, z_plane}, sketch_color);
    DrawLine3D(::Vector3 {fin_outer, fin_tip_back_y, z_plane}, ::Vector3 {body_radius, fin_back_y, z_plane}, sketch_color);
    DrawLine3D(::Vector3 {fin_outer, fin_tip_y, z_plane}, ::Vector3 {fin_outer, fin_tip_back_y, z_plane}, sketch_color);
}

void drawModelingReferenceWindow(AppState& app_state, const ::Rectangle& bounds) {
    drawPanel(bounds, "Vista & Overlay");
    drawSectionCaption(
        Rectangle {bounds.x + 16.0f, bounds.y + 48.0f, bounds.width - 32.0f, 34.0f},
        "Allineamento camera",
        "Passa tra viste ortogonali e prospettiva CAD con navigazione intenzionale.");

    constexpr std::array<const char*, 4> view_labels {"Perspective", "Front", "Side", "Top"};
    constexpr std::array<ModelingView, 4> views {
        ModelingView::Perspective,
        ModelingView::Front,
        ModelingView::Side,
        ModelingView::Top
    };

    for (int index = 0; index < 4; ++index) {
        const ::Rectangle button {
            bounds.x + 16.0f + static_cast<float>(index % 2) * ((bounds.width - 42.0f) * 0.5f + 10.0f),
            bounds.y + 86.0f + static_cast<float>(index / 2) * 36.0f,
            (bounds.width - 42.0f) * 0.5f,
            28.0f
        };
        if (drawButton(button, view_labels[static_cast<std::size_t>(index)], app_state.modeling_view == views[static_cast<std::size_t>(index)], Color {61, 135, 245, 228}, app_state.modeling_view == views[static_cast<std::size_t>(index)] ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
            app_state.modeling_view = views[static_cast<std::size_t>(index)];
            if (app_state.modeling_view != ModelingView::Perspective) {
                app_state.camera_orbit = false;
            }
        }
    }

    drawInlineHint(
        Rectangle {bounds.x + 16.0f, bounds.y + 166.0f, bounds.width - 32.0f, 28.0f},
        app_state.camera_orbit
            ? "TAB attivo: MMB orbita, Shift+MMB pan, wheel zoom. Sopra i pannelli la camera resta ferma."
            : "TAB disattivo: la camera resta stabile. Attivalo solo quando vuoi navigare la viewport.",
        Color {84, 136, 199, 224});

    const ::Rectangle blueprint_button {bounds.x + 16.0f, bounds.y + 202.0f, bounds.width - 32.0f, 30.0f};
    if (drawButton(
            blueprint_button,
            app_state.show_reference_blueprint ? "Blueprint visibile" : "Blueprint nascosto",
            app_state.show_reference_blueprint,
            Color {84, 136, 199, 224},
            ButtonStyle::Outlined)) {
        app_state.show_reference_blueprint = !app_state.show_reference_blueprint;
    }

    const int scale_delta = drawStepper(
        Rectangle {bounds.x + 16.0f, bounds.y + 242.0f, bounds.width - 32.0f, 28.0f},
        "Scala tavola",
        std::format("{:.1f}x", app_state.reference_board_scale_m));
    if (scale_delta != 0) {
        app_state.reference_board_scale_m = std::clamp(app_state.reference_board_scale_m + 0.1 * static_cast<double>(scale_delta), 1.0, 6.0);
    }

    const int offset_delta = drawStepper(
        Rectangle {bounds.x + 16.0f, bounds.y + 276.0f, bounds.width - 32.0f, 28.0f},
        "Profondita",
        std::format("{:.2f} m", app_state.reference_board_offset_m));
    if (offset_delta != 0) {
        app_state.reference_board_offset_m =
            std::clamp(app_state.reference_board_offset_m + 0.05 * static_cast<double>(offset_delta), -1.2, 1.2);
    }

    drawSectionCaption(
        Rectangle {bounds.x + 16.0f, bounds.y + 316.0f, bounds.width - 32.0f, 32.0f},
        "Overlay rapidi",
        "Raggruppa i toggle che cambiano la leggibilita della viewport.");

    const float col_w = (bounds.width - 42.0f) * 0.5f;
    const ::Rectangle metrics_button {bounds.x + 16.0f, bounds.y + 352.0f, col_w, 28.0f};
    const ::Rectangle snap_button {bounds.x + 26.0f + col_w, bounds.y + 352.0f, col_w, 28.0f};
    const ::Rectangle grid_button {bounds.x + 16.0f, bounds.y + 388.0f, col_w, 28.0f};
    const ::Rectangle handles_button {bounds.x + 26.0f + col_w, bounds.y + 388.0f, col_w, 28.0f};

    if (drawButton(metrics_button, "Metrics", app_state.show_metrics_overlay, Color {84, 136, 199, 224}, ButtonStyle::Outlined)) {
        app_state.show_metrics_overlay = !app_state.show_metrics_overlay;
    }
    if (drawButton(snap_button, app_state.snap_to_grid ? "Snap ON" : "Snap OFF", app_state.snap_to_grid, Color {84, 136, 199, 224}, ButtonStyle::Outlined)) {
        app_state.snap_to_grid = !app_state.snap_to_grid;
    }
    if (drawButton(grid_button, app_state.show_local_grid ? "Grid locale" : "Grid nascosta", app_state.show_local_grid, Color {84, 136, 199, 224}, ButtonStyle::Outlined)) {
        app_state.show_local_grid = !app_state.show_local_grid;
    }
    if (drawButton(handles_button, app_state.show_snap_points ? "Handles ON" : "Handles OFF", app_state.show_snap_points, Color {84, 136, 199, 224}, ButtonStyle::Outlined)) {
        app_state.show_snap_points = !app_state.show_snap_points;
    }
}

void drawWorkspaceBar(AppState& app_state, const SimulationRuntime& runtime) {
    const ::Rectangle top_bar {0.0f, 0.0f, static_cast<float>(GetScreenWidth()), 64.0f};
    DrawRectangleGradientH(0, 0, GetScreenWidth(), 64, Color {10, 14, 22, 246}, Color {19, 26, 38, 246});
    DrawLineEx(Vector2 {0.0f, top_bar.height}, Vector2 {top_bar.width, top_bar.height}, 1.0f, Color {60, 72, 89, 230});
    DrawText("The Rocket Lab", 24, 14, 24, Color {244, 247, 251, 255});
    DrawText("Fluent workspace for rocket design and flight analysis", 24, 39, 11, Color {124, 139, 160, 255});
    if (drawButton(
            Rectangle {168.0f, 14.0f, 106.0f, 36.0f},
            app_state.show_project_workflow_panel ? "Project ON" : "Project OFF",
            app_state.show_project_workflow_panel,
            Color {84, 136, 199, 224},
            app_state.show_project_workflow_panel ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
        app_state.show_project_workflow_panel = !app_state.show_project_workflow_panel;
    }

    const ::Rectangle modeling_button {286.0f, 14.0f, 176.0f, 36.0f};
    const ::Rectangle simulation_button {472.0f, 14.0f, 176.0f, 36.0f};
    drawButton(modeling_button, "Modelazione [F1]", app_state.workspace == Workspace::Modeling, Color {61, 135, 245, 228}, app_state.workspace == Workspace::Modeling ? ButtonStyle::Contained : ButtonStyle::Outlined);
    drawButton(simulation_button, "Simulazione [F2]", app_state.workspace == Workspace::Simulation, Color {25, 165, 124, 224}, app_state.workspace == Workspace::Simulation ? ButtonStyle::Contained : ButtonStyle::Outlined);

    DrawText(
        app_state.workspace == Workspace::Modeling ? "Workspace di modellazione" : "Control room di simulazione",
        674,
        18,
        18,
        Color {220, 227, 236, 255});
    DrawText(
        app_state.workspace == Workspace::Modeling ? "Viewport centrale, libreria contestuale e inspector del componente." : "Telemetria live, scenario, timeline e vista traiettoria.",
        674,
        38,
        12,
        Color {124, 139, 160, 255});

    const bool has_transient_status =
        !app_state.transient_status_message.empty() &&
        GetTime() <= app_state.transient_status_expire_time_s;
    const std::string workflow_hint = has_transient_status
        ? app_state.transient_status_message
        : "Ctrl+S salva  |  Ctrl+Shift+S Save As  |  Ctrl+O carica  |  Ctrl+E esporta";
    drawSingleLineClippedText(
        Rectangle {674.0f, 51.0f, 620.0f, 12.0f},
        workflow_hint,
        11,
        has_transient_status ? Color {191, 219, 254, 255} : Color {124, 139, 160, 255});

    if (app_state.workspace == Workspace::Simulation) {
        drawButton(
            Rectangle {982.0f, 14.0f, 176.0f, 36.0f},
            runtime.replay_active ? "Stop Replay" : "Ripercorri Rotta",
            runtime.replay_active,
            Color {189, 108, 30, 224},
            runtime.replay_active ? ButtonStyle::Contained : ButtonStyle::Outlined);
        drawButton(
            Rectangle {1172.0f, 14.0f, 148.0f, 36.0f},
            app_state.show_flight_markers ? "Markers ON" : "Markers OFF",
            app_state.show_flight_markers,
            Color {92, 129, 196, 224},
            ButtonStyle::Subtle);
        drawButton(
            Rectangle {1334.0f, 14.0f, 84.0f, 36.0f},
            "Fixed",
            app_state.simulation_camera_mode == SimulationCameraMode::Fixed,
            Color {84, 136, 199, 224},
            app_state.simulation_camera_mode == SimulationCameraMode::Fixed ? ButtonStyle::Contained : ButtonStyle::Outlined);
        drawButton(
            Rectangle {1428.0f, 14.0f, 84.0f, 36.0f},
            "Follow",
            app_state.simulation_camera_mode == SimulationCameraMode::Follow,
            Color {84, 136, 199, 224},
            app_state.simulation_camera_mode == SimulationCameraMode::Follow ? ButtonStyle::Contained : ButtonStyle::Outlined);
        drawButton(
            Rectangle {1522.0f, 14.0f, 58.0f, 36.0f},
            "Free",
            app_state.simulation_camera_mode == SimulationCameraMode::Free,
            Color {84, 136, 199, 224},
            app_state.simulation_camera_mode == SimulationCameraMode::Free ? ButtonStyle::Contained : ButtonStyle::Outlined);
        DrawText("F3 apre la camera del vento esterna", 1336, 51, 11, Color {124, 139, 160, 255});
    }
}

void drawProjectWorkflowPanel(AppState& app_state) {
    if (!app_state.show_project_workflow_panel) {
        return;
    }

    const ::Rectangle bounds {610.0f, 92.0f, 414.0f, 220.0f};
    drawPanel(bounds, "Project & Export");

    drawSectionCaption(
        Rectangle {bounds.x + 16.0f, bounds.y + 46.0f, bounds.width - 32.0f, 30.0f},
        "Percorsi attivi",
        "Usa i pulsanti per scegliere file reali, poi salva, carica o esporta senza dipendere solo dalle scorciatoie.");

    drawSingleLineClippedText(
        Rectangle {bounds.x + 16.0f, bounds.y + 82.0f, bounds.width - 32.0f, 16.0f},
        std::format("Project  {}", app_state.current_project_path.string()),
        13,
        Color {203, 213, 225, 255});
    drawSingleLineClippedText(
        Rectangle {bounds.x + 16.0f, bounds.y + 102.0f, bounds.width - 32.0f, 16.0f},
        std::format("Report   {}", app_state.current_report_path.string()),
        13,
        Color {148, 163, 184, 255});
    drawSingleLineClippedText(
        Rectangle {bounds.x + 16.0f, bounds.y + 122.0f, bounds.width - 32.0f, 16.0f},
        std::format("CSV      {}", app_state.current_trajectory_csv_path.string()),
        13,
        Color {148, 163, 184, 255});

    const float row_w = (bounds.width - 44.0f) * 0.5f;
    if (drawButton(
            Rectangle {bounds.x + 16.0f, bounds.y + 150.0f, row_w, 30.0f},
            "Save Project",
            false,
            Color {61, 135, 245, 228},
            ButtonStyle::Contained)) {
        app_state.request_project_save = true;
    }
    if (drawButton(
            Rectangle {bounds.x + 28.0f + row_w, bounds.y + 150.0f, row_w, 30.0f},
            "Save As...",
            false,
            Color {84, 136, 199, 224},
            ButtonStyle::Outlined)) {
        app_state.request_project_save_as = true;
    }
    if (drawButton(
            Rectangle {bounds.x + 16.0f, bounds.y + 186.0f, row_w, 30.0f},
            "Load...",
            false,
            Color {84, 136, 199, 224},
            ButtonStyle::Outlined)) {
        app_state.request_project_load = true;
    }
    if (drawButton(
            Rectangle {bounds.x + 28.0f + row_w, bounds.y + 186.0f, row_w, 30.0f},
            "Export Report+CSV",
            false,
            Color {25, 165, 124, 224},
            ButtonStyle::Contained)) {
        app_state.request_project_export = true;
    }
}

void drawModelingToolbar(AppState& app_state, const ::Rectangle& bounds) {
    drawPanel(bounds, "Strumenti Rapidi");
    constexpr std::array<ModelingTool, 6> tools {
        ModelingTool::Select,
        ModelingTool::Move,
        ModelingTool::Rotate,
        ModelingTool::Scale,
        ModelingTool::AddPart,
        ModelingTool::Measure
    };
    for (int index = 0; index < static_cast<int>(tools.size()); ++index) {
        const auto tool = tools[static_cast<std::size_t>(index)];
        const ::Rectangle button {
            bounds.x + 14.0f,
            bounds.y + 50.0f + static_cast<float>(index) * 44.0f,
            bounds.width - 28.0f,
            30.0f
        };
        const std::string label = std::format("{} {}", modelingToolLabel(tool), modelingToolShortcut(tool));
        if (drawButton(button, label, app_state.modeling_tool == tool, Color {61, 135, 245, 228}, app_state.modeling_tool == tool ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
            app_state.modeling_tool = tool;
        }
    }

    drawInfoCard(
        Rectangle {bounds.x + 14.0f, bounds.y + bounds.height - 86.0f, bounds.width - 28.0f, 60.0f},
        std::format("Tool attivo  {}", modelingToolLabel(app_state.modeling_tool)),
        modelingToolHint(app_state.modeling_tool),
        Color {56, 189, 248, 255});
}

void drawModelingOutliner(AppState& app_state, const ::Rectangle& bounds) {
    drawPanel(bounds, "Struttura Veicolo");
    constexpr std::array<const char*, 6> component_labels {
        "Nose Cone",
        "Body Tube",
        "Transition",
        "Fin Set",
        "Motor Mount",
        "Payload Section"
    };

    for (int index = 0; index < static_cast<int>(component_labels.size()); ++index) {
        const ::Rectangle button {
            bounds.x + 16.0f,
            bounds.y + 54.0f + static_cast<float>(index) * 36.0f,
            bounds.width - 32.0f,
            28.0f
        };
        if (drawButton(button, component_labels[static_cast<std::size_t>(index)], app_state.selection == static_cast<ComponentSelection>(index), Color {61, 135, 245, 228}, app_state.selection == static_cast<ComponentSelection>(index) ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
            app_state.selection = static_cast<ComponentSelection>(index);
        }
    }
}

bool drawMaterialChooser(
    const ::Rectangle& bounds,
    const char* title,
    rocket::ComponentMaterial& material) {
    DrawText(title, static_cast<int>(bounds.x), static_cast<int>(bounds.y), 18, Color {148, 163, 184, 255});
    bool changed = false;
    const auto& materials = rocket::availableComponentMaterials();
    constexpr int columns = 3;
    for (int index = 0; index < static_cast<int>(materials.size()); ++index) {
        const auto current = materials[static_cast<std::size_t>(index)];
        const float cell_w = (bounds.width - 12.0f) / 3.0f;
        const int column = index % columns;
        const int row = index / columns;
        const ::Rectangle button {
            bounds.x + static_cast<float>(column) * (cell_w + 6.0f),
            bounds.y + 26.0f + static_cast<float>(row) * 38.0f,
            cell_w,
            32.0f
        };
        if (drawButton(
                button,
                std::string(rocket::materialDefinition(current).label),
                material == current,
                Color {84, 136, 199, 224},
                material == current ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
            material = current;
            changed = true;
        }
    }
    return changed;
}


