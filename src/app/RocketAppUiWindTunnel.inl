const char* componentSelectionLabel(ComponentSelection selection) {
    switch (selection) {
    case ComponentSelection::NoseCone:
        return "Nose";
    case ComponentSelection::BodyTube:
        return "Body";
    case ComponentSelection::Transition:
        return "Transition";
    case ComponentSelection::FinSet:
        return "Fins";
    case ComponentSelection::MotorMount:
        return "Motor Mount";
    case ComponentSelection::Payload:
        return "Payload";
    }
    return "Component";
}

Color componentSelectionAccent(ComponentSelection selection) {
    switch (selection) {
    case ComponentSelection::NoseCone:
        return Color {56, 189, 248, 255};
    case ComponentSelection::BodyTube:
        return Color {125, 211, 252, 255};
    case ComponentSelection::Transition:
        return Color {168, 85, 247, 255};
    case ComponentSelection::FinSet:
        return Color {249, 115, 22, 255};
    case ComponentSelection::MotorMount:
        return Color {34, 197, 94, 255};
    case ComponentSelection::Payload:
        return Color {244, 114, 182, 255};
    }
    return Color {241, 245, 249, 255};
}

double focusedComponentAreaEstimateM2(const rocket::VehicleGeometry& geometry, ComponentSelection selection) {
    const double body_radius = geometry.body_diameter_m * 0.5;
    switch (selection) {
    case ComponentSelection::NoseCone:
        return std::max(0.04, geometry.nose_length_m * body_radius);
    case ComponentSelection::BodyTube:
        return std::max(0.05, geometry.body_length_m * geometry.body_diameter_m);
    case ComponentSelection::Transition:
        return std::max(0.02, geometry.transition_length_m * (geometry.body_diameter_m + geometry.transition_aft_diameter_m) * 0.5);
    case ComponentSelection::FinSet:
        return std::max(0.02, 0.5 * (geometry.fin_root_chord_m + geometry.fin_tip_chord_m) * geometry.fin_span_m * static_cast<double>(geometry.fin_count));
    case ComponentSelection::MotorMount:
        return std::max(0.01, geometry.transition_aft_diameter_m * geometry.transition_aft_diameter_m * 0.35);
    case ComponentSelection::Payload:
        return std::max(0.02, geometry.payload_length_m * geometry.body_diameter_m * 0.6);
    }
    return 0.02;
}

rocket::ComponentType componentTypeFromSelection(ComponentSelection selection) {
    switch (selection) {
    case ComponentSelection::NoseCone:
        return rocket::ComponentType::NoseCone;
    case ComponentSelection::BodyTube:
        return rocket::ComponentType::BodyTube;
    case ComponentSelection::Transition:
        return rocket::ComponentType::Transition;
    case ComponentSelection::FinSet:
        return rocket::ComponentType::FinSet;
    case ComponentSelection::MotorMount:
        return rocket::ComponentType::MotorMount;
    case ComponentSelection::Payload:
        return rocket::ComponentType::Payload;
    }
    return rocket::ComponentType::BodyTube;
}

rocket::ComponentMaterial focusedComponentMaterial(const rocket::VehicleGeometry& geometry, ComponentSelection selection) {
    switch (selection) {
    case ComponentSelection::NoseCone:
        return geometry.nose_material;
    case ComponentSelection::BodyTube:
        return geometry.body_material;
    case ComponentSelection::Transition:
        return geometry.transition_material;
    case ComponentSelection::FinSet:
        return geometry.fin_material;
    case ComponentSelection::MotorMount:
        return geometry.body_material;
    case ComponentSelection::Payload:
        return geometry.payload_material;
    }
    return geometry.body_material;
}

double focusedComponentLoadFactor(const rocket::VehicleGeometry& geometry, ComponentSelection selection) {
    switch (selection) {
    case ComponentSelection::NoseCone:
        return geometry.nose_cone_shape == rocket::NoseConeShape::LdHaack ? 0.82 : 0.94;
    case ComponentSelection::BodyTube:
        return 0.68;
    case ComponentSelection::Transition:
        return geometry.transition_shape == rocket::TransitionShape::Curved ? 0.74 : 0.84;
    case ComponentSelection::FinSet:
        return geometry.fin_shape == rocket::FinShape::Airfoil ? 1.04 : 1.18;
    case ComponentSelection::MotorMount:
        return 0.93;
    case ComponentSelection::Payload:
        return 0.72;
    }
    return 1.0;
}

const char* focusedComponentInsight(const rocket::VehicleGeometry& geometry, ComponentSelection selection) {
    switch (selection) {
    case ComponentSelection::NoseCone:
        return geometry.nose_cone_shape == rocket::NoseConeShape::LdHaack ? "Profilo efficiente: pressione piu distribuita sul muso." : "Il muso domina la stagnazione e l'innesco del carico frontale.";
    case ComponentSelection::BodyTube:
        return "Il corpo converte AoA in carico distribuito e drag longitudinale.";
    case ComponentSelection::Transition:
        return geometry.transition_shape == rocket::TransitionShape::Curved ? "Raccordo piu pulito: minore separazione nella coda." : "La coda conica e piu sensibile a separazione e drag di base.";
    case ComponentSelection::FinSet:
        return "Le pinne amplificano stabilita e carico laterale, soprattutto con AoA crescente.";
    case ComponentSelection::MotorMount:
        return "La zona motore sente soprattutto base drag, scia e sensibilita alla coda.";
    case ComponentSelection::Payload:
        return "La payload section influenza il flusso interno al volume frontale e il bilanciamento globale.";
    }
    return "Analisi locale del componente selezionato.";
}

void drawTunnelMetric(
    const ::Rectangle& bounds,
    const std::string& label,
    const std::string& value,
    Color accent) {
    DrawRectangleRounded(bounds, 0.16f, 8, Color {18, 28, 44, 240});
    DrawRectangleRoundedLinesEx(bounds, 0.16f, 8, 1.2f, Color {51, 65, 85, 255});
    DrawRectangle(static_cast<int>(bounds.x) + 1, static_cast<int>(bounds.y) + 1, 4, static_cast<int>(bounds.height) - 2, accent);
    drawSingleLineClippedText(
        Rectangle {bounds.x + 12.0f, bounds.y + 7.0f, bounds.width - 20.0f, 14.0f},
        label,
        13,
        Color {148, 163, 184, 255});
    drawSingleLineClippedText(
        Rectangle {bounds.x + 12.0f, bounds.y + 24.0f, bounds.width - 20.0f, 18.0f},
        value,
        16,
        Color {241, 245, 249, 255});
}

void drawWindTunnelInfoBlock(
    const ::Rectangle& bounds,
    const std::string& label,
    const std::string& value,
    Color accent) {
    DrawRectangleRounded(bounds, 0.18f, 8, Color {16, 24, 38, 236});
    DrawRectangleRoundedLinesEx(bounds, 0.18f, 8, 1.0f, Color {60, 76, 99, 220});
    DrawRectangle(static_cast<int>(bounds.x) + 1, static_cast<int>(bounds.y) + 1, 4, static_cast<int>(bounds.height) - 2, accent);
    drawSingleLineClippedText(
        Rectangle {bounds.x + 12.0f, bounds.y + 8.0f, bounds.width - 20.0f, 14.0f},
        label,
        13,
        Color {148, 163, 184, 255});
    drawWrappedText(
        Rectangle {bounds.x + 12.0f, bounds.y + 26.0f, bounds.width - 20.0f, bounds.height - 34.0f},
        value,
        15,
        Color {241, 245, 249, 255},
        2,
        2.0f);
}

void drawPressureHeatLegend(const ::Rectangle& bounds) {
    const std::array<Color, 4> stops {
        Color {56, 189, 248, 255},
        Color {34, 197, 94, 255},
        Color {251, 191, 36, 255},
        Color {239, 68, 68, 255}
    };

    const float section_w = bounds.width / static_cast<float>(stops.size());
    for (int index = 0; index < static_cast<int>(stops.size()); ++index) {
        DrawRectangle(
            static_cast<int>(bounds.x + section_w * static_cast<float>(index)),
            static_cast<int>(bounds.y),
            static_cast<int>(std::ceil(section_w)),
            static_cast<int>(bounds.height),
            stops[static_cast<std::size_t>(index)]);
    }
    DrawRectangleLinesEx(bounds, 1.0f, Color {71, 85, 105, 220});
    DrawText("low", static_cast<int>(bounds.x), static_cast<int>(bounds.y + bounds.height + 4.0f), 12, Color {148, 163, 184, 255});
    DrawText("high", static_cast<int>(bounds.x + bounds.width - 28.0f), static_cast<int>(bounds.y + bounds.height + 4.0f), 12, Color {148, 163, 184, 255});
}

void drawWindTunnelPanel(
    const ::Rectangle& bounds,
    AppState& app_state,
    const rocket::SimulationSnapshot& snapshot,
    const rocket::VehicleModel& vehicle) {
    drawPanel(bounds, "Camera del Vento");

    const float outer_padding = 14.0f;
    const float header_y = bounds.y + 40.0f;
    const float focus_card_w = std::clamp(bounds.width * 0.32f, 216.0f, 268.0f);
    const float metric_gap = 6.0f;
    const float metric_w = (bounds.width - outer_padding * 2.0f - metric_gap * 4.0f) / 5.0f;
    const Color regime_color =
        snapshot.mach_number < 0.8 ? Color {34, 197, 94, 255}
        : snapshot.mach_number < 1.2 ? Color {251, 191, 36, 255}
                                     : Color {239, 68, 68, 255};
    drawTunnelMetric(Rectangle {bounds.x + outer_padding, header_y, metric_w, 48.0f}, "Regime", rocket::app::flowRegimeLabel(snapshot), regime_color);
    drawTunnelMetric(Rectangle {bounds.x + outer_padding + (metric_w + metric_gap) * 1.0f, header_y, metric_w, 48.0f}, "Velocita aria", std::format("{:.1f} m/s", snapshot.relative_air_speed_mps), Color {56, 189, 248, 255});
    drawTunnelMetric(Rectangle {bounds.x + outer_padding + (metric_w + metric_gap) * 2.0f, header_y, metric_w, 48.0f}, "AoA", std::format("{:.2f} deg", snapshot.angle_of_attack_deg), Color {168, 85, 247, 255});
    drawTunnelMetric(Rectangle {bounds.x + outer_padding + (metric_w + metric_gap) * 3.0f, header_y, metric_w, 48.0f}, "Pressione q", std::format("{:.0f} Pa", snapshot.dynamic_pressure_pa), Color {249, 115, 22, 255});
    drawTunnelMetric(Rectangle {bounds.x + outer_padding + (metric_w + metric_gap) * 4.0f, header_y, metric_w, 48.0f}, "Densita", std::format("{:.3f} kg/m3", snapshot.air_density_kgpm3), Color {125, 211, 252, 255});

    constexpr std::array<ComponentSelection, 6> focus_components {
        ComponentSelection::NoseCone,
        ComponentSelection::BodyTube,
        ComponentSelection::Transition,
        ComponentSelection::FinSet,
        ComponentSelection::Payload,
        ComponentSelection::MotorMount
    };

    const float focus_section_y = bounds.y + 94.0f;
    const float left_controls_w = bounds.width - outer_padding * 3.0f - focus_card_w;
    const float selector_gap = 8.0f;
    const float selector_w = (left_controls_w - selector_gap * 2.0f) / 3.0f;
    for (int index = 0; index < static_cast<int>(focus_components.size()); ++index) {
        const auto focus = focus_components[static_cast<std::size_t>(index)];
        const float column = static_cast<float>(index % 3);
        const float row = static_cast<float>(index / 3);
        const ::Rectangle button {
            bounds.x + outer_padding + column * (selector_w + selector_gap),
            focus_section_y + row * 32.0f,
            selector_w,
            24.0f
        };
        if (drawButton(button, componentSelectionLabel(focus), app_state.wind_tunnel_focus == focus, componentSelectionAccent(focus))) {
            app_state.wind_tunnel_focus = focus;
        }
    }

    const float focus_card_x = bounds.x + bounds.width - outer_padding - focus_card_w;
    const ::Rectangle heatmap_button {focus_card_x, focus_section_y, focus_card_w, 24.0f};
    if (drawButton(
            heatmap_button,
            app_state.show_cfd_pressure_overlay ? "Mesh Heatmap ON" : "Mesh Heatmap OFF",
            app_state.show_cfd_pressure_overlay,
            Color {84, 136, 199, 224},
            ButtonStyle::Outlined)) {
        app_state.show_cfd_pressure_overlay = !app_state.show_cfd_pressure_overlay;
    }
    drawPressureHeatLegend(Rectangle {focus_card_x + 10.0f, focus_section_y + 34.0f, focus_card_w - 20.0f, 10.0f});

    const float content_y = focus_section_y + 64.0f;
    const ::Rectangle tunnel {
        bounds.x + outer_padding,
        content_y,
        bounds.width - outer_padding * 3.0f - focus_card_w,
        bounds.height - (content_y - bounds.y) - outer_padding
    };
    const ::Rectangle focus_card {focus_card_x, content_y, focus_card_w, tunnel.height};
    DrawRectangleRounded(tunnel, 0.04f, 8, Color {8, 15, 26, 246});
    DrawRectangleRoundedLinesEx(tunnel, 0.04f, 8, 1.0f, Color {45, 64, 89, 255});
    DrawRectangleRounded(focus_card, 0.08f, 8, Color {11, 20, 33, 236});
    DrawRectangleRoundedLinesEx(focus_card, 0.08f, 8, 1.0f, Color {45, 64, 89, 255});

    const bool low_flow_preview = snapshot.time_s < 0.05 && snapshot.relative_air_speed_mps < 0.5 && snapshot.wind_speed_mps < 0.5;
    const bool recovery_flow = snapshot.parachute_deployed;

    const float center_y = tunnel.y + tunnel.height * 0.54f;
    const float rocket_left = tunnel.x + 58.0f;
    const float usable_width = tunnel.width - 116.0f;
    const float scale = usable_width / static_cast<float>(std::max(vehicle.geometry.body_length_m, 0.4));
    const float total_length = static_cast<float>(vehicle.geometry.body_length_m * scale);
    const float nose_length = static_cast<float>(vehicle.geometry.nose_length_m * scale);
    const float transition_length = static_cast<float>(vehicle.geometry.transition_length_m * scale);
    const float body_length = std::max(total_length - nose_length - transition_length, 36.0f);
    const float body_radius = std::max(10.0f, static_cast<float>(vehicle.geometry.body_diameter_m * scale * 0.48));
    const float aft_radius = std::max(7.0f, static_cast<float>(vehicle.geometry.transition_aft_diameter_m * scale * 0.48));
    const float nose_tip_x = rocket_left;
    const float nose_base_x = nose_tip_x + nose_length;
    const float body_end_x = nose_base_x + body_length;
    const float tail_end_x = body_end_x + transition_length;
    const float fin_front_x = rocket_left + static_cast<float>(vehicle.geometry.fin_front_from_nose_m * scale);
    const float fin_root = std::max(18.0f, static_cast<float>(vehicle.geometry.fin_root_chord_m * scale));
    const float fin_tip = std::max(12.0f, static_cast<float>(vehicle.geometry.fin_tip_chord_m * scale));
    const float fin_span = std::max(12.0f, static_cast<float>(vehicle.geometry.fin_span_m * vehicle.geometry.fin_controls.span_scale * scale * 0.7f));
    const float fin_tip_front = fin_front_x + static_cast<float>((vehicle.geometry.fin_sweep_length_m + vehicle.geometry.fin_controls.tip_le_offset_m) * scale);
    const float fin_tip_back = fin_tip_front + fin_tip;
    const float body_top = center_y - body_radius;
    const float body_bottom = center_y + body_radius;
    const float aoa_shift = recovery_flow ? std::clamp(static_cast<float>(snapshot.angle_of_attack_deg) * 0.45f, -8.0f, 8.0f) : std::clamp(static_cast<float>(snapshot.angle_of_attack_deg) * 2.4f, -22.0f, 22.0f);
    const float turbulence = rocket::app::pressureRatio01(snapshot.dynamic_pressure_pa, 24000.0);
    const float compressibility = recovery_flow ? 0.0f : std::clamp(static_cast<float>(snapshot.mach_number / 1.4), 0.0f, 1.0f);
    const float pressure_bias = rocket::app::pressureRatio01(snapshot.total_pressure_pa, 160000.0);
    const int streamline_count = recovery_flow ? 7 : 9 + static_cast<int>(std::round(compressibility * 4.0f));
    const float flow_phase = static_cast<float>(GetTime()) * (0.45f + std::clamp(snapshot.relative_air_speed_mps / 180.0f, 0.12, 1.4));
    const rocket::CfdFrameData& cfd_frame = app_state.cfd_field.frame();

    BeginScissorMode(static_cast<int>(tunnel.x) + 1, static_cast<int>(tunnel.y) + 1, std::max(1, static_cast<int>(tunnel.width) - 2), std::max(1, static_cast<int>(tunnel.height) - 2));

    for (int i = 0; i < streamline_count; ++i) {
        const float t = streamline_count <= 1 ? 0.5f : static_cast<float>(i) / static_cast<float>(streamline_count - 1);
        const Color flow_color = rocket::app::blendColor(Color {56, 189, 248, 170}, regime_color, 0.28f + 0.52f * std::max(turbulence, compressibility));
        const float line_thickness = recovery_flow ? 1.15f : 1.2f + turbulence * 0.8f + compressibility * 0.7f;
        ::Vector2 previous = rocket::app::sampleWindTunnelFlowPoint(tunnel, center_y, nose_tip_x, nose_base_x, body_end_x, tail_end_x, total_length, body_radius, aoa_shift, turbulence, compressibility, pressure_bias, t, 0.0f, flow_phase);
        for (int segment = 1; segment <= 42; ++segment) {
            const float s = static_cast<float>(segment) / 42.0f;
            const ::Vector2 current = rocket::app::sampleWindTunnelFlowPoint(tunnel, center_y, nose_tip_x, nose_base_x, body_end_x, tail_end_x, total_length, body_radius, aoa_shift, turbulence, compressibility, pressure_bias, t, s, flow_phase);
            DrawLineEx(previous, current, line_thickness, flow_color);
            previous = current;
        }
    }

    if (!low_flow_preview) {
        for (const auto& particle : cfd_frame.render_particles) {
            const ::Vector2 previous {tunnel.x + 12.0f + static_cast<float>(particle.prev_x_norm) * (tunnel.width - 24.0f), tunnel.y + 12.0f + static_cast<float>(particle.prev_y_norm) * (tunnel.height - 24.0f)};
            const ::Vector2 current {tunnel.x + 12.0f + static_cast<float>(particle.x_norm) * (tunnel.width - 24.0f), tunnel.y + 12.0f + static_cast<float>(particle.y_norm) * (tunnel.height - 24.0f)};
            if (!rocket::app::pointInsideRect(previous, tunnel, 20.0f) &&
                !rocket::app::pointInsideRect(current, tunnel, 20.0f)) {
                continue;
            }
            const float energy_ratio =
                rocket::app::pressureRatio01(particle.kinetic_energy, std::max(snapshot.dynamic_pressure_pa * 2.0, 1.0));
            const Color particle_color = rocket::app::blendColor(
                Color {191, 219, 254, 130},
                regime_color,
                energy_ratio * 0.8f + compressibility * 0.2f);
            DrawLineEx(previous, current, recovery_flow ? 0.9f : 1.0f + energy_ratio * 1.4f, Color {particle_color.r, particle_color.g, particle_color.b, static_cast<unsigned char>(recovery_flow ? 70 : 90)});
            DrawCircleV(current, recovery_flow ? 1.4f : 1.3f + energy_ratio * 1.5f, Color {particle_color.r, particle_color.g, particle_color.b, static_cast<unsigned char>(recovery_flow ? 110 : 120 + energy_ratio * 100)});
        }
    }

    DrawLineEx(::Vector2 {tunnel.x + 14.0f, center_y}, ::Vector2 {tunnel.x + tunnel.width - 14.0f, center_y}, 1.0f, Color {51, 65, 85, 160});
    EndScissorMode();

    const auto focused_fill = [&](ComponentSelection selection, Color fallback) {
        return app_state.wind_tunnel_focus == selection ? componentSelectionAccent(selection) : fallback;
    };
    const auto focused_alpha = [&](ComponentSelection selection, unsigned char alpha) {
        Color color = componentSelectionAccent(selection);
        color.a = alpha;
        return color;
    };

    DrawTriangle(::Vector2 {nose_tip_x, center_y}, ::Vector2 {nose_base_x, body_top}, ::Vector2 {nose_base_x, body_bottom}, focused_fill(ComponentSelection::NoseCone, Color {214, 222, 233, 255}));
    DrawRectangleRounded(Rectangle {nose_base_x, body_top, body_length, body_radius * 2.0f}, 0.18f, 8, focused_fill(ComponentSelection::BodyTube, Color {214, 222, 233, 255}));
    if (transition_length > 1.0f) {
        DrawTriangle(::Vector2 {body_end_x, body_top}, ::Vector2 {tail_end_x, center_y - aft_radius}, ::Vector2 {body_end_x, body_bottom}, focused_fill(ComponentSelection::Transition, Color {192, 203, 219, 255}));
        DrawTriangle(::Vector2 {body_end_x, body_bottom}, ::Vector2 {tail_end_x, center_y - aft_radius}, ::Vector2 {tail_end_x, center_y + aft_radius}, focused_fill(ComponentSelection::MotorMount, focused_fill(ComponentSelection::Transition, Color {192, 203, 219, 255})));
    }

    const float payload_length = std::max(18.0f, static_cast<float>(vehicle.geometry.payload_length_m * scale * 0.9f));
    const float payload_end_x = std::min(nose_base_x + payload_length, nose_base_x + body_length * 0.5f);
    DrawRectangleRounded(Rectangle {nose_base_x + 4.0f, body_top + 4.0f, std::max(payload_end_x - nose_base_x - 8.0f, 8.0f), std::max(body_radius * 2.0f - 8.0f, 8.0f)}, 0.18f, 8, app_state.wind_tunnel_focus == ComponentSelection::Payload ? focused_alpha(ComponentSelection::Payload, 220) : Color {255, 255, 255, 26});

    DrawTriangle(::Vector2 {fin_front_x, body_bottom - 1.0f}, ::Vector2 {fin_tip_front, body_bottom + fin_span}, ::Vector2 {fin_front_x + fin_root, body_bottom - 1.0f}, focused_fill(ComponentSelection::FinSet, Color {255, 176, 64, 255}));
    DrawTriangle(::Vector2 {fin_tip_front, body_bottom + fin_span}, ::Vector2 {fin_tip_back, body_bottom + fin_span}, ::Vector2 {fin_front_x + fin_root, body_bottom - 1.0f}, focused_fill(ComponentSelection::FinSet, Color {255, 176, 64, 255}));
    DrawTriangle(::Vector2 {fin_front_x, body_top + 1.0f}, ::Vector2 {fin_tip_front, body_top - fin_span}, ::Vector2 {fin_front_x + fin_root, body_top + 1.0f}, app_state.wind_tunnel_focus == ComponentSelection::FinSet ? focused_alpha(ComponentSelection::FinSet, 220) : Color {255, 176, 64, 190});
    DrawTriangle(::Vector2 {fin_tip_front, body_top - fin_span}, ::Vector2 {fin_tip_back, body_top - fin_span}, ::Vector2 {fin_front_x + fin_root, body_top + 1.0f}, app_state.wind_tunnel_focus == ComponentSelection::FinSet ? focused_alpha(ComponentSelection::FinSet, 220) : Color {255, 176, 64, 190});

    if (!recovery_flow) {
        const float stagnation_radius = std::clamp(10.0f + turbulence * 10.0f + compressibility * 9.0f, 10.0f, 30.0f);
        DrawCircleGradient(::Vector2 {nose_tip_x, center_y}, stagnation_radius, Color {251, 191, 36, 120}, Color {251, 191, 36, 0});
    }
    if (!recovery_flow && snapshot.mach_number >= 0.92 && snapshot.mach_number <= 1.18) {
        const float shock_x = nose_base_x + body_length * 0.18f;
        DrawLineEx(::Vector2 {shock_x, body_top - 18.0f}, ::Vector2 {shock_x + 24.0f, body_bottom + 18.0f}, 1.6f, Color {251, 191, 36, 180});
        DrawLineEx(::Vector2 {shock_x, body_bottom + 18.0f}, ::Vector2 {shock_x + 24.0f, body_top - 18.0f}, 1.6f, Color {251, 191, 36, 120});
    }

    DrawRectangleRounded(
        Rectangle {tunnel.x + 8.0f, tunnel.y + tunnel.height - 34.0f, tunnel.width - 16.0f, 24.0f},
        0.18f,
        8,
        Color {11, 20, 33, 220});
    drawSingleLineClippedText(
        Rectangle {tunnel.x + 14.0f, tunnel.y + tunnel.height - 28.0f, tunnel.width - 28.0f, 14.0f},
        std::format(
            "Vento {:.1f} m/s  |  Mach {:.2f}  |  rho {:.3f} kg/m3  |  P0 {:.0f} Pa  |  CFD {} / {}",
            snapshot.wind_speed_mps,
            snapshot.mach_number,
            snapshot.air_density_kgpm3,
            snapshot.total_pressure_pa,
            snapshot.cfd_render_particle_count,
            snapshot.cfd_solver_particle_count),
        14,
        Color {148, 163, 184, 255});

    if (low_flow_preview) {
        drawInfoCard(Rectangle {tunnel.x + 22.0f, tunnel.y + 18.0f, tunnel.width - 44.0f, 52.0f}, "Flusso quasi nullo", "Avvia la simulazione o aumenta il vento nello scenario per leggere separazione e pressione.", Color {56, 189, 248, 255});
    }

    const double focused_area_m2 = focusedComponentAreaEstimateM2(vehicle.geometry, app_state.wind_tunnel_focus);
    const double focused_load_factor = focusedComponentLoadFactor(vehicle.geometry, app_state.wind_tunnel_focus);
    const std::size_t focused_band_index =
        app_state.wind_tunnel_focus == ComponentSelection::NoseCone ? static_cast<std::size_t>(rocket::CfdComponentBand::NoseCone)
        : app_state.wind_tunnel_focus == ComponentSelection::BodyTube ? static_cast<std::size_t>(rocket::CfdComponentBand::BodyTube)
        : app_state.wind_tunnel_focus == ComponentSelection::Transition ? static_cast<std::size_t>(rocket::CfdComponentBand::Transition)
        : app_state.wind_tunnel_focus == ComponentSelection::FinSet ? static_cast<std::size_t>(rocket::CfdComponentBand::FinSet)
        : app_state.wind_tunnel_focus == ComponentSelection::Payload ? static_cast<std::size_t>(rocket::CfdComponentBand::Payload)
                                                                    : static_cast<std::size_t>(rocket::CfdComponentBand::MotorMount);
    const double focused_pressure_pa = cfd_frame.component_pressure_pa[focused_band_index];
    const double focused_load_n = std::max(snapshot.dynamic_pressure_pa * focused_area_m2 * focused_load_factor, focused_pressure_pa * focused_area_m2);
    const double sensitivity = focused_load_factor * (1.0 + std::abs(snapshot.angle_of_attack_deg) / 12.0);
    const double focused_reynolds =
        rocket::app::reynoldsNumber(snapshot, std::sqrt(std::max(focused_area_m2, 1e-4)));
    const auto focused_material = rocket::materialDefinition(focusedComponentMaterial(vehicle.geometry, app_state.wind_tunnel_focus));
    const double focused_q_limit_pa = rocket::estimateComponentDynamicPressureLimitPa(componentTypeFromSelection(app_state.wind_tunnel_focus), vehicle.geometry);
    const double focused_safety_factor = focused_q_limit_pa / std::max(focused_pressure_pa, snapshot.dynamic_pressure_pa * focused_load_factor * 0.65 + 1.0);
    const Color focus_accent = componentSelectionAccent(app_state.wind_tunnel_focus);

    const float card_padding = 12.0f;
    const float card_inner_w = focus_card.width - card_padding * 2.0f;
    const float stat_gap = 10.0f;
    const float stat_w = (card_inner_w - stat_gap) * 0.5f;
    drawStatusChip(
        Rectangle {focus_card.x + card_padding, focus_card.y + 12.0f, card_inner_w, 28.0f},
        componentSelectionLabel(app_state.wind_tunnel_focus),
        Color {15, 23, 42, 230},
        focus_accent);
    drawSectionCaption(
        Rectangle {focus_card.x + card_padding, focus_card.y + 50.0f, card_inner_w, 24.0f},
        "Focus Locale",
        "Stessa lettura fisica, ordinata come una console.");

    drawWindTunnelInfoBlock(
        Rectangle {focus_card.x + card_padding, focus_card.y + 82.0f, stat_w, 58.0f},
        "Carico stimato",
        std::format("{:.0f} N", focused_load_n),
        focus_accent);
    drawWindTunnelInfoBlock(
        Rectangle {focus_card.x + card_padding + stat_w + stat_gap, focus_card.y + 82.0f, stat_w, 58.0f},
        "Area efficace",
        std::format("{:.3f} m2", focused_area_m2),
        Color {56, 189, 248, 255});
    drawWindTunnelInfoBlock(
        Rectangle {focus_card.x + card_padding, focus_card.y + 148.0f, stat_w, 58.0f},
        "Sensibilita AoA",
        std::format("{:.2f}x", sensitivity),
        Color {249, 115, 22, 255});
    drawWindTunnelInfoBlock(
        Rectangle {focus_card.x + card_padding + stat_w + stat_gap, focus_card.y + 148.0f, stat_w, 58.0f},
        "P impatto",
        std::format("{:.0f} Pa", focused_pressure_pa),
        Color {125, 211, 252, 255});

    drawSectionCaption(
        Rectangle {focus_card.x + card_padding, focus_card.y + 218.0f, card_inner_w, 22.0f},
        "Materiale & Struttura");
    drawKeyValueLine(Rectangle {focus_card.x + card_padding, focus_card.y + 244.0f, card_inner_w, 18.0f}, "Materiale", std::string(focused_material.label));
    drawKeyValueLine(Rectangle {focus_card.x + card_padding, focus_card.y + 266.0f, card_inner_w, 18.0f}, "Modulo E", std::format("{:.1f} GPa", focused_material.youngs_modulus_gpa));
    drawKeyValueLine(Rectangle {focus_card.x + card_padding, focus_card.y + 288.0f, card_inner_w, 18.0f}, "q rec", std::format("{:.0f} kPa", focused_q_limit_pa / 1000.0));
    drawKeyValueLine(Rectangle {focus_card.x + card_padding, focus_card.y + 310.0f, card_inner_w, 18.0f}, "Safety factor", std::format("{:.2f}", focused_safety_factor));
    drawKeyValueLine(Rectangle {focus_card.x + card_padding, focus_card.y + 332.0f, card_inner_w, 18.0f}, "Shock / Re", std::format("{:.2f} / {:.2e}", snapshot.shockwave_intensity, focused_reynolds));

    drawSectionCaption(
        Rectangle {focus_card.x + card_padding, focus_card.y + 362.0f, card_inner_w, 22.0f},
        "Insight");
    drawInfoCard(
        Rectangle {focus_card.x + card_padding, focus_card.y + 388.0f, card_inner_w, 74.0f},
        componentSelectionLabel(app_state.wind_tunnel_focus),
        focusedComponentInsight(vehicle.geometry, app_state.wind_tunnel_focus),
        focus_accent);
    drawInlineHint(
        Rectangle {focus_card.x + card_padding, focus_card.y + focus_card.height - 56.0f, card_inner_w, 46.0f},
        app_state.show_cfd_pressure_overlay ? "Heatmap attiva: la mesh 3D usa la stessa scala di pressione del tunnel." : "Attiva la heatmap per leggere la pressione direttamente sulla mesh 3D.",
        focus_accent);
}
