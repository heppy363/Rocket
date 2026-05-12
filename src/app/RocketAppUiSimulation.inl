void drawSimulationTelemetry(const ::Rectangle& bounds, const rocket::SimulationSnapshot& snapshot) {
    drawPanel(bounds, "Telemetria Live");
    drawSectionCaption(
        Rectangle {bounds.x + 16.0f, bounds.y + 46.0f, bounds.width - 32.0f, 30.0f},
        "Lettura immediata",
        "I valori principali sono separati in card per ridurre il rumore visivo.");
    const float col_w = (bounds.width - 44.0f) * 0.5f;
    const float row_h = 40.0f;

    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 82.0f, col_w, row_h}, "Tempo", std::format("{:.2f} s", snapshot.time_s), Color {84, 136, 199, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 82.0f, col_w, row_h}, "Quota", std::format("{:.1f} m", snapshot.state.position_m.z), Color {120, 187, 255, 255});
    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 130.0f, col_w, row_h}, "Velocita verticale", std::format("{:.1f} m/s", snapshot.state.velocity_mps.z), Color {93, 209, 150, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 130.0f, col_w, row_h}, "Velocita aria", std::format("{:.1f} m/s", snapshot.relative_air_speed_mps), Color {84, 180, 224, 255});
    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 178.0f, col_w, row_h}, "Mach", std::format("{:.2f}", snapshot.mach_number), Color {146, 165, 215, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 178.0f, col_w, row_h}, "AoA", std::format("{:.2f} deg", snapshot.angle_of_attack_deg), Color {223, 148, 86, 255});
    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 226.0f, col_w, row_h}, "Densita", std::format("{:.3f} kg/m3", snapshot.air_density_kgpm3), Color {96, 165, 250, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 226.0f, col_w, row_h}, "P statica", std::format("{:.0f} Pa", snapshot.static_pressure_pa), Color {56, 189, 248, 255});
    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 274.0f, col_w, row_h}, "q consigliata", std::format("{:.0f} kPa", snapshot.recommended_max_dynamic_pressure_pa / 1000.0), Color {251, 191, 36, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 274.0f, col_w, row_h}, "Safety Factor", std::format("{:.2f}x", snapshot.dynamic_pressure_safety_factor), snapshot.dynamic_pressure_safety_factor >= 1.35 ? Color {74, 222, 128, 255} : snapshot.dynamic_pressure_safety_factor >= 1.0 ? Color {251, 191, 36, 255} : Color {248, 113, 113, 255});
}

void drawMissionKeyframePreview(
    const ::Rectangle& bounds,
    const SimulationRuntime& runtime,
    const rocket::SimulationSnapshot& snapshot) {
    drawPanel(bounds, "Analisi Keyframe");

    MissionKeyframe keyframe;
    if (!tryGetSelectedMissionKeyframe(runtime, keyframe)) {
        drawInfoCard(
            Rectangle {bounds.x + 14.0f, bounds.y + 44.0f, bounds.width - 28.0f, 58.0f},
            "Keyframe preview inattivo",
            "Premi K per scorrere launch, burnout, apogee e impact con analisi puntuale.",
            Color {56, 189, 248, 255});
        return;
    }

    drawStatusChip(
        Rectangle {bounds.x + 14.0f, bounds.y + 44.0f, 118.0f, 28.0f},
        keyframe.label,
        Color {30, 41, 59, 230},
        keyframe.accent);
    drawStatusChip(
        Rectangle {bounds.x + 140.0f, bounds.y + 44.0f, 138.0f, 28.0f},
        std::format("t = {:.2f} s", keyframe.time_s),
        Color {15, 23, 42, 230},
        Color {100, 116, 139, 220});

    drawMetricCard(Rectangle {bounds.x + 14.0f, bounds.y + 84.0f, 126.0f, 44.0f}, "Quota", std::format("{:.1f} m", snapshot.state.position_m.z), keyframe.accent);
    drawMetricCard(Rectangle {bounds.x + 150.0f, bounds.y + 84.0f, 126.0f, 44.0f}, "Velocita aria", std::format("{:.1f} m/s", snapshot.relative_air_speed_mps), Color {56, 189, 248, 255});
    drawMetricCard(Rectangle {bounds.x + 14.0f, bounds.y + 136.0f, 126.0f, 44.0f}, "Mach", std::format("{:.2f}", snapshot.mach_number), Color {168, 85, 247, 255});
    drawMetricCard(Rectangle {bounds.x + 150.0f, bounds.y + 136.0f, 126.0f, 44.0f}, "AoA", std::format("{:.2f} deg", snapshot.angle_of_attack_deg), Color {249, 115, 22, 255});

    DrawText(std::format("Pressione q   {:.0f} Pa", snapshot.dynamic_pressure_pa).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 194, 14, Color {226, 232, 240, 255});
    DrawText(std::format("P0 totale    {:.0f} Pa", snapshot.total_pressure_pa).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 214, 14, Color {226, 232, 240, 255});
    DrawText(std::format("Margine      {:.2f} cal", snapshot.static_margin_calibers).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 234, 14, Color {226, 232, 240, 255});
    DrawText(std::format("CG / CP      {:.2f} / {:.2f} m", snapshot.cg_from_nose_m, snapshot.cp_from_nose_m).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 254, 14, Color {226, 232, 240, 255});
    DrawText(std::format("q rec        {:.0f} kPa", snapshot.recommended_max_dynamic_pressure_pa / 1000.0).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 274, 14, Color {226, 232, 240, 255});
    drawInlineHint(
        Rectangle {bounds.x + 14.0f, bounds.y + bounds.height - 30.0f, bounds.width - 28.0f, 24.0f},
        "Premi K di nuovo per passare al keyframe successivo. Dopo l'ultimo torni alla vista live.",
        keyframe.accent);
}

Color blendColor(Color a, Color b, float t) {
    const float clamped = std::clamp(t, 0.0f, 1.0f);
    const auto lerp_channel = [clamped](unsigned char lhs, unsigned char rhs) {
        return static_cast<unsigned char>(static_cast<float>(lhs) + (static_cast<float>(rhs) - static_cast<float>(lhs)) * clamped);
    };
    return {
        lerp_channel(a.r, b.r),
        lerp_channel(a.g, b.g),
        lerp_channel(a.b, b.b),
        lerp_channel(a.a, b.a)
    };
}

double airDynamicViscosityPaS(double temperature_k) {
    const double safe_temperature_k = std::max(temperature_k, 180.0);
    return 1.458e-6 * std::pow(safe_temperature_k, 1.5) / (safe_temperature_k + 110.4);
}

double reynoldsNumber(const rocket::SimulationSnapshot& snapshot, double reference_length_m) {
    const double mu = airDynamicViscosityPaS(snapshot.air_temperature_k);
    return (snapshot.air_density_kgpm3 * std::max(snapshot.relative_air_speed_mps, 0.0) * std::max(reference_length_m, 0.001)) /
           std::max(mu, 1e-7);
}

float pressureRatio01(double pressure_pa, double reference_pa) {
    return std::clamp(static_cast<float>(pressure_pa / std::max(reference_pa, 1.0)), 0.0f, 1.0f);
}

bool pointInsideRect(const ::Vector2& point, const ::Rectangle& rect, float margin = 0.0f) {
    return point.x >= rect.x - margin &&
           point.x <= rect.x + rect.width + margin &&
           point.y >= rect.y - margin &&
           point.y <= rect.y + rect.height + margin;
}

::Vector2 sampleWindTunnelFlowPoint(
    const ::Rectangle& tunnel,
    float center_y,
    float nose_tip_x,
    float nose_base_x,
    float body_end_x,
    float tail_end_x,
    float total_length,
    float body_radius,
    float aoa_shift,
    float turbulence,
    float compressibility,
    float pressure_bias,
    float lane_t,
    float progress_t,
    float phase_t) {
    const float y_seed = tunnel.y + 20.0f + lane_t * (tunnel.height - 40.0f);
    const float relative_to_body = (y_seed - center_y) / std::max(body_radius * 1.8f, 1.0f);
    const float body_influence = std::exp(-relative_to_body * relative_to_body);
    const float x = tunnel.x + 12.0f + progress_t * (tunnel.width - 24.0f);
    float y = y_seed + aoa_shift * (0.12f + 0.72f * lane_t) * progress_t;
    if (x >= nose_tip_x - 24.0f && x <= tail_end_x + 24.0f) {
        const float body_center_x = nose_tip_x + total_length * 0.45f;
        const float x_norm = (x - body_center_x) / std::max(total_length * 0.42f, 1.0f);
        y += body_influence * std::exp(-(x_norm * x_norm) * (3.8f + compressibility * 1.2f)) * (relative_to_body * 22.0f + aoa_shift * 0.15f);
        y += std::sin((progress_t * 9.0f + lane_t * 1.8f + phase_t) * pi) * turbulence * (2.2f + compressibility * 1.2f);
        if (progress_t > 0.1f && x > nose_base_x && x < body_end_x && compressibility > 0.66f) {
            y += std::sin((progress_t * 15.0f + lane_t * 2.0f + phase_t * 0.6f) * pi) * (1.4f + pressure_bias * 1.8f);
        }
    }
    return ::Vector2 {x, y};
}

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

std::string flowRegimeLabel(const rocket::SimulationSnapshot& snapshot) {
    if (snapshot.parachute_deployed) {
        return "Recovery flow";
    }
    if (snapshot.mach_number < 0.8) {
        return "Subsonic";
    }
    if (snapshot.mach_number < 1.2) {
        return "Transonic";
    }
    return "Supersonic";
}

void drawTunnelMetric(
    const ::Rectangle& bounds,
    const std::string& label,
    const std::string& value,
    Color accent) {
    DrawRectangleRounded(bounds, 0.16f, 8, Color {18, 28, 44, 240});
    DrawRectangleRoundedLinesEx(bounds, 0.16f, 8, 1.2f, Color {51, 65, 85, 255});
    DrawRectangle(static_cast<int>(bounds.x) + 1, static_cast<int>(bounds.y) + 1, 4, static_cast<int>(bounds.height) - 2, accent);
    DrawText(label.c_str(), static_cast<int>(bounds.x) + 12, static_cast<int>(bounds.y) + 7, 13, Color {148, 163, 184, 255});
    DrawText(value.c_str(), static_cast<int>(bounds.x) + 12, static_cast<int>(bounds.y) + 24, 16, Color {241, 245, 249, 255});
}

void drawWindTunnelPanel(
    const ::Rectangle& bounds,
    AppState& app_state,
    const rocket::SimulationSnapshot& snapshot,
    const rocket::VehicleModel& vehicle) {
    drawPanel(bounds, "Camera del Vento");

    const float header_y = bounds.y + 40.0f;
    const float metric_w = (bounds.width - 54.0f) / 5.0f;
    const Color regime_color =
        snapshot.mach_number < 0.8 ? Color {34, 197, 94, 255}
        : snapshot.mach_number < 1.2 ? Color {251, 191, 36, 255}
                                     : Color {239, 68, 68, 255};
    drawTunnelMetric(
        Rectangle {bounds.x + 14.0f, header_y, metric_w, 48.0f},
        "Regime",
        flowRegimeLabel(snapshot),
        regime_color);
    drawTunnelMetric(
        Rectangle {bounds.x + 20.0f + metric_w, header_y, metric_w, 48.0f},
        "Velocita aria",
        std::format("{:.1f} m/s", snapshot.relative_air_speed_mps),
        Color {56, 189, 248, 255});
    drawTunnelMetric(
        Rectangle {bounds.x + 26.0f + metric_w * 2.0f, header_y, metric_w, 48.0f},
        "AoA",
        std::format("{:.2f} deg", snapshot.angle_of_attack_deg),
        Color {168, 85, 247, 255});
    drawTunnelMetric(
        Rectangle {bounds.x + 32.0f + metric_w * 3.0f, header_y, metric_w, 48.0f},
        "Pressione q",
        std::format("{:.0f} Pa", snapshot.dynamic_pressure_pa),
        Color {249, 115, 22, 255});
    drawTunnelMetric(
        Rectangle {bounds.x + 38.0f + metric_w * 4.0f, header_y, metric_w, 48.0f},
        "Densita",
        std::format("{:.3f} kg/m3", snapshot.air_density_kgpm3),
        Color {125, 211, 252, 255});

    constexpr std::array<ComponentSelection, 6> focus_components {
        ComponentSelection::NoseCone,
        ComponentSelection::BodyTube,
        ComponentSelection::Transition,
        ComponentSelection::FinSet,
        ComponentSelection::Payload,
        ComponentSelection::MotorMount
    };

    for (int index = 0; index < static_cast<int>(focus_components.size()); ++index) {
        const auto focus = focus_components[static_cast<std::size_t>(index)];
        const float column = static_cast<float>(index % 3);
        const float row = static_cast<float>(index / 3);
        const ::Rectangle button {
            bounds.x + 14.0f + column * 144.0f,
            bounds.y + 94.0f + row * 32.0f,
            136.0f,
            24.0f
        };
        if (drawButton(button, componentSelectionLabel(focus), app_state.wind_tunnel_focus == focus, componentSelectionAccent(focus))) {
            app_state.wind_tunnel_focus = focus;
        }
    }

    const ::Rectangle tunnel {
        bounds.x + 14.0f,
        bounds.y + 164.0f,
        bounds.width - 218.0f,
        bounds.height - 180.0f
    };
    const ::Rectangle focus_card {
        bounds.x + bounds.width - 190.0f,
        bounds.y + 164.0f,
        176.0f,
        bounds.height - 180.0f
    };
    DrawRectangleRounded(tunnel, 0.04f, 8, Color {8, 15, 26, 246});
    DrawRectangleRoundedLinesEx(tunnel, 0.04f, 8, 1.0f, Color {45, 64, 89, 255});
    DrawRectangleRounded(focus_card, 0.08f, 8, Color {11, 20, 33, 236});
    DrawRectangleRoundedLinesEx(focus_card, 0.08f, 8, 1.0f, Color {45, 64, 89, 255});

    const bool low_flow_preview =
        snapshot.time_s < 0.05 &&
        snapshot.relative_air_speed_mps < 0.5 &&
        snapshot.wind_speed_mps < 0.5;
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
    const float aft_radius =
        std::max(7.0f, static_cast<float>(vehicle.geometry.transition_aft_diameter_m * scale * 0.48));
    const float nose_tip_x = rocket_left;
    const float nose_base_x = nose_tip_x + nose_length;
    const float body_end_x = nose_base_x + body_length;
    const float tail_end_x = body_end_x + transition_length;
    const float fin_front_x = rocket_left + static_cast<float>(vehicle.geometry.fin_front_from_nose_m * scale);
    const float fin_root = std::max(18.0f, static_cast<float>(vehicle.geometry.fin_root_chord_m * scale));
    const float fin_tip = std::max(12.0f, static_cast<float>(vehicle.geometry.fin_tip_chord_m * scale));
    const float fin_span = std::max(12.0f, static_cast<float>(vehicle.geometry.fin_span_m * vehicle.geometry.fin_controls.span_scale * scale * 0.7f));
    const float fin_tip_front =
        fin_front_x + static_cast<float>((vehicle.geometry.fin_sweep_length_m + vehicle.geometry.fin_controls.tip_le_offset_m) * scale);
    const float fin_tip_back = fin_tip_front + fin_tip;
    const float body_top = center_y - body_radius;
    const float body_bottom = center_y + body_radius;
    const float aoa_shift = recovery_flow
        ? std::clamp(static_cast<float>(snapshot.angle_of_attack_deg) * 0.45f, -8.0f, 8.0f)
        : std::clamp(static_cast<float>(snapshot.angle_of_attack_deg) * 2.4f, -22.0f, 22.0f);
    const float turbulence = pressureRatio01(snapshot.dynamic_pressure_pa, 24000.0);
    const float compressibility = recovery_flow ? 0.0f : std::clamp(static_cast<float>(snapshot.mach_number / 1.4), 0.0f, 1.0f);
    const float pressure_bias = pressureRatio01(snapshot.total_pressure_pa, 160000.0);
    const int streamline_count = recovery_flow ? 7 : 9 + static_cast<int>(std::round(compressibility * 4.0f));
    const float flow_phase = static_cast<float>(GetTime()) * (0.45f + std::clamp(snapshot.relative_air_speed_mps / 180.0f, 0.12, 1.4));
    const rocket::CfdFrameData& cfd_frame = app_state.cfd_field.frame();

    BeginScissorMode(
        static_cast<int>(tunnel.x) + 1,
        static_cast<int>(tunnel.y) + 1,
        std::max(1, static_cast<int>(tunnel.width) - 2),
        std::max(1, static_cast<int>(tunnel.height) - 2));

    for (int i = 0; i < streamline_count; ++i) {
        const float t = streamline_count <= 1 ? 0.5f : static_cast<float>(i) / static_cast<float>(streamline_count - 1);
        const Color flow_color = blendColor(Color {56, 189, 248, 170}, regime_color, 0.28f + 0.52f * std::max(turbulence, compressibility));
        const float line_thickness = recovery_flow ? 1.15f : 1.2f + turbulence * 0.8f + compressibility * 0.7f;

        ::Vector2 previous = sampleWindTunnelFlowPoint(
            tunnel,
            center_y,
            nose_tip_x,
            nose_base_x,
            body_end_x,
            tail_end_x,
            total_length,
            body_radius,
            aoa_shift,
            turbulence,
            compressibility,
            pressure_bias,
            t,
            0.0f,
            flow_phase);
        for (int segment = 1; segment <= 42; ++segment) {
            const float s = static_cast<float>(segment) / 42.0f;
            const ::Vector2 current = sampleWindTunnelFlowPoint(
                tunnel,
                center_y,
                nose_tip_x,
                nose_base_x,
                body_end_x,
                tail_end_x,
                total_length,
                body_radius,
                aoa_shift,
                turbulence,
                compressibility,
                pressure_bias,
                t,
                s,
                flow_phase);
            DrawLineEx(previous, current, line_thickness, flow_color);
            previous = current;
        }
    }

    if (!low_flow_preview) {
        for (const auto& particle : cfd_frame.render_particles) {
            const ::Vector2 previous {
                tunnel.x + 12.0f + static_cast<float>(particle.prev_x_norm) * (tunnel.width - 24.0f),
                tunnel.y + 12.0f + static_cast<float>(particle.prev_y_norm) * (tunnel.height - 24.0f)
            };
            const ::Vector2 current {
                tunnel.x + 12.0f + static_cast<float>(particle.x_norm) * (tunnel.width - 24.0f),
                tunnel.y + 12.0f + static_cast<float>(particle.y_norm) * (tunnel.height - 24.0f)
            };
            if (!pointInsideRect(previous, tunnel, 20.0f) && !pointInsideRect(current, tunnel, 20.0f)) {
                continue;
            }
            const float energy_ratio = pressureRatio01(particle.kinetic_energy, std::max(snapshot.dynamic_pressure_pa * 2.0, 1.0));
            const Color particle_color = blendColor(Color {191, 219, 254, 130}, regime_color, energy_ratio * 0.8f + compressibility * 0.2f);
            DrawLineEx(previous, current, recovery_flow ? 0.9f : 1.0f + energy_ratio * 1.4f, Color {particle_color.r, particle_color.g, particle_color.b, static_cast<unsigned char>(recovery_flow ? 70 : 90)});
            DrawCircleV(current, recovery_flow ? 1.4f : 1.3f + energy_ratio * 1.5f, Color {particle_color.r, particle_color.g, particle_color.b, static_cast<unsigned char>(recovery_flow ? 110 : 120 + energy_ratio * 100)});
        }
    }

    DrawLineEx(
        ::Vector2 {tunnel.x + 14.0f, center_y},
        ::Vector2 {tunnel.x + tunnel.width - 14.0f, center_y},
        1.0f,
        Color {51, 65, 85, 160});

    EndScissorMode();

    const auto focused_fill = [&](ComponentSelection selection, Color fallback) {
        return app_state.wind_tunnel_focus == selection ? componentSelectionAccent(selection) : fallback;
    };
    const auto focused_alpha = [&](ComponentSelection selection, unsigned char alpha) {
        Color color = componentSelectionAccent(selection);
        color.a = alpha;
        return color;
    };

    DrawTriangle(
        ::Vector2 {nose_tip_x, center_y},
        ::Vector2 {nose_base_x, body_top},
        ::Vector2 {nose_base_x, body_bottom},
        focused_fill(ComponentSelection::NoseCone, Color {214, 222, 233, 255}));
    DrawRectangleRounded(
        Rectangle {nose_base_x, body_top, body_length, body_radius * 2.0f},
        0.18f,
        8,
        focused_fill(ComponentSelection::BodyTube, Color {214, 222, 233, 255}));
    if (transition_length > 1.0f) {
        DrawTriangle(
            ::Vector2 {body_end_x, body_top},
            ::Vector2 {tail_end_x, center_y - aft_radius},
            ::Vector2 {body_end_x, body_bottom},
            focused_fill(ComponentSelection::Transition, Color {192, 203, 219, 255}));
        DrawTriangle(
            ::Vector2 {body_end_x, body_bottom},
            ::Vector2 {tail_end_x, center_y - aft_radius},
            ::Vector2 {tail_end_x, center_y + aft_radius},
            focused_fill(ComponentSelection::MotorMount, focused_fill(ComponentSelection::Transition, Color {192, 203, 219, 255})));
    }

    const float payload_length = std::max(18.0f, static_cast<float>(vehicle.geometry.payload_length_m * scale * 0.9f));
    const float payload_end_x = std::min(nose_base_x + payload_length, nose_base_x + body_length * 0.5f);
    DrawRectangleRounded(
        Rectangle {nose_base_x + 4.0f, body_top + 4.0f, std::max(payload_end_x - nose_base_x - 8.0f, 8.0f), std::max(body_radius * 2.0f - 8.0f, 8.0f)},
        0.18f,
        8,
        app_state.wind_tunnel_focus == ComponentSelection::Payload ? focused_alpha(ComponentSelection::Payload, 220) : Color {255, 255, 255, 26});

    DrawTriangle(
        ::Vector2 {fin_front_x, body_bottom - 1.0f},
        ::Vector2 {fin_tip_front, body_bottom + fin_span},
        ::Vector2 {fin_front_x + fin_root, body_bottom - 1.0f},
        focused_fill(ComponentSelection::FinSet, Color {255, 176, 64, 255}));
    DrawTriangle(
        ::Vector2 {fin_tip_front, body_bottom + fin_span},
        ::Vector2 {fin_tip_back, body_bottom + fin_span},
        ::Vector2 {fin_front_x + fin_root, body_bottom - 1.0f},
        focused_fill(ComponentSelection::FinSet, Color {255, 176, 64, 255}));

    DrawTriangle(
        ::Vector2 {fin_front_x, body_top + 1.0f},
        ::Vector2 {fin_tip_front, body_top - fin_span},
        ::Vector2 {fin_front_x + fin_root, body_top + 1.0f},
        app_state.wind_tunnel_focus == ComponentSelection::FinSet ? focused_alpha(ComponentSelection::FinSet, 220) : Color {255, 176, 64, 190});
    DrawTriangle(
        ::Vector2 {fin_tip_front, body_top - fin_span},
        ::Vector2 {fin_tip_back, body_top - fin_span},
        ::Vector2 {fin_front_x + fin_root, body_top + 1.0f},
        app_state.wind_tunnel_focus == ComponentSelection::FinSet ? focused_alpha(ComponentSelection::FinSet, 220) : Color {255, 176, 64, 190});

    if (!recovery_flow) {
        const float stagnation_radius = std::clamp(10.0f + turbulence * 10.0f + compressibility * 9.0f, 10.0f, 30.0f);
        DrawCircleGradient(
            ::Vector2 {nose_tip_x, center_y},
            stagnation_radius,
            Color {251, 191, 36, 120},
            Color {251, 191, 36, 0});
    }

    if (!recovery_flow && snapshot.mach_number >= 0.92 && snapshot.mach_number <= 1.18) {
        const float shock_x = nose_base_x + body_length * 0.18f;
        DrawLineEx(
            ::Vector2 {shock_x, body_top - 18.0f},
            ::Vector2 {shock_x + 24.0f, body_bottom + 18.0f},
            1.6f,
            Color {251, 191, 36, 180});
        DrawLineEx(
            ::Vector2 {shock_x, body_bottom + 18.0f},
            ::Vector2 {shock_x + 24.0f, body_top - 18.0f},
            1.6f,
            Color {251, 191, 36, 120});
    }

    DrawText(
        std::format(
            "Vento {:.1f} m/s  |  Mach {:.2f}  |  rho {:.3f} kg/m3  |  P0 {:.0f} Pa  |  CFD {} / {}",
            snapshot.wind_speed_mps,
            snapshot.mach_number,
            snapshot.air_density_kgpm3,
            snapshot.total_pressure_pa,
            snapshot.cfd_render_particle_count,
            snapshot.cfd_solver_particle_count).c_str(),
        static_cast<int>(tunnel.x) + 12,
        static_cast<int>(tunnel.y + tunnel.height - 22.0f),
        14,
        Color {148, 163, 184, 255});

    if (low_flow_preview) {
        drawInfoCard(
            Rectangle {tunnel.x + 22.0f, tunnel.y + 18.0f, tunnel.width - 44.0f, 52.0f},
            "Flusso quasi nullo",
            "Avvia la simulazione o aumenta il vento nello scenario per leggere separazione e pressione.",
            Color {56, 189, 248, 255});
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
    const double focused_reynolds = reynoldsNumber(snapshot, std::sqrt(std::max(focused_area_m2, 1e-4)));
    const auto focused_material = rocket::materialDefinition(focusedComponentMaterial(vehicle.geometry, app_state.wind_tunnel_focus));
    const double focused_q_limit_pa =
        rocket::estimateComponentDynamicPressureLimitPa(componentTypeFromSelection(app_state.wind_tunnel_focus), vehicle.geometry);
    const double focused_safety_factor = focused_q_limit_pa / std::max(focused_pressure_pa, snapshot.dynamic_pressure_pa * focused_load_factor * 0.65 + 1.0);
    const Color focus_accent = componentSelectionAccent(app_state.wind_tunnel_focus);

    drawStatusChip(
        Rectangle {focus_card.x + 12.0f, focus_card.y + 12.0f, focus_card.width - 24.0f, 28.0f},
        componentSelectionLabel(app_state.wind_tunnel_focus),
        Color {15, 23, 42, 230},
        focus_accent);
    DrawText("Focus locale", static_cast<int>(focus_card.x) + 12, static_cast<int>(focus_card.y) + 52, 13, Color {148, 163, 184, 255});
    drawMetricCard(Rectangle {focus_card.x + 12.0f, focus_card.y + 72.0f, focus_card.width - 24.0f, 42.0f}, "Carico stimato", std::format("{:.0f} N", focused_load_n), focus_accent);
    drawMetricCard(Rectangle {focus_card.x + 12.0f, focus_card.y + 122.0f, focus_card.width - 24.0f, 42.0f}, "Area efficace", std::format("{:.3f} m2", focused_area_m2), Color {56, 189, 248, 255});
    drawMetricCard(Rectangle {focus_card.x + 12.0f, focus_card.y + 172.0f, focus_card.width - 24.0f, 42.0f}, "Sensibilita AoA", std::format("{:.2f}x", sensitivity), Color {249, 115, 22, 255});
    DrawText(std::format("P impatto {:.0f} Pa", focused_pressure_pa).c_str(), static_cast<int>(focus_card.x) + 12, static_cast<int>(focus_card.y) + 226, 14, Color {226, 232, 240, 255});
    DrawText(std::format("{} | E {:.1f} GPa", focused_material.label, focused_material.youngs_modulus_gpa).c_str(), static_cast<int>(focus_card.x) + 12, static_cast<int>(focus_card.y) + 246, 14, Color {226, 232, 240, 255});
    DrawText(std::format("q rec {:.0f} kPa | SF {:.2f}", focused_q_limit_pa / 1000.0, focused_safety_factor).c_str(), static_cast<int>(focus_card.x) + 12, static_cast<int>(focus_card.y) + 266, 14, focused_safety_factor >= 1.2 ? Color {74, 222, 128, 255} : focused_safety_factor >= 1.0 ? Color {251, 191, 36, 255} : Color {248, 113, 113, 255});
    DrawText(std::format("Shock {:.2f} | Re {:.2e}", snapshot.shockwave_intensity, focused_reynolds).c_str(), static_cast<int>(focus_card.x) + 12, static_cast<int>(focus_card.y) + 286, 14, Color {226, 232, 240, 255});
    DrawText(focusedComponentInsight(vehicle.geometry, app_state.wind_tunnel_focus), static_cast<int>(focus_card.x) + 12, static_cast<int>(focus_card.y) + 310, 13, Color {148, 163, 184, 255});
    drawInlineHint(
        Rectangle {focus_card.x + 12.0f, focus_card.y + focus_card.height - 30.0f, focus_card.width - 24.0f, 24.0f},
        "Cambia componente per isolare il tratto di razzo che vuoi leggere.",
        focus_accent);
}

void drawSimulationScenario(
    const ::Rectangle& bounds,
    AppState& app_state,
    SimulationRuntime& runtime,
    rocket::VehicleModel& vehicle,
    rocket::Environment& environment) {
    drawPanel(bounds, "Scenario di Volo");

    drawSectionCaption(
        Rectangle {bounds.x + 18.0f, bounds.y + 50.0f, bounds.width - 36.0f, 30.0f},
        "Controlli missione",
        "L'azione principale resta visivamente dominante.");

    const ::Rectangle play_button {bounds.x + 18.0f, bounds.y + 92.0f, bounds.width - 36.0f, 38.0f};
    if (drawButton(play_button, runtime.paused ? "Avvia / Riprendi" : "Pausa", !runtime.paused, Color {25, 165, 124, 224}, ButtonStyle::Contained)) {
        runtime.paused = !runtime.paused;
    }

    const ::Rectangle reset_button {bounds.x + 18.0f, bounds.y + 140.0f, bounds.width - 36.0f, 34.0f};
    if (drawButton(reset_button, "Reset missione", false, Color {84, 136, 199, 224}, ButtonStyle::Outlined)) {
        resetSimulationRuntime(vehicle, runtime);
    }

    const ::Rectangle replay_button {bounds.x + 18.0f, bounds.y + 182.0f, bounds.width - 36.0f, 34.0f};
    if (drawButton(replay_button, runtime.replay_active ? "Ferma replay rotta" : "Avvia replay rotta", runtime.replay_active, Color {84, 136, 199, 224}, runtime.replay_active ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
        runtime.replay_active = !runtime.replay_active;
        runtime.replay_time_s = 0.0;
    }

    const ::Rectangle marker_button {bounds.x + 18.0f, bounds.y + 224.0f, bounds.width - 36.0f, 34.0f};
    if (drawButton(marker_button, app_state.show_flight_markers ? "Marker visibili" : "Marker nascosti", app_state.show_flight_markers, Color {84, 136, 199, 224}, ButtonStyle::Outlined)) {
        app_state.show_flight_markers = !app_state.show_flight_markers;
    }

    auto site = environment.launchSite();
    auto weather = environment.surfaceWeather();

    float row_y = bounds.y + 284.0f;
    auto apply_environment_step = [&](double& value, double step, const char* label, const char* unit) {
        const int delta = drawStepper(
            Rectangle {bounds.x + 18.0f, row_y, bounds.width - 36.0f, 30.0f},
            label,
            std::format("{:.2f} {}", value, unit));
        if (delta != 0) {
            value += step * static_cast<double>(delta);
            row_y += 40.0f;
            return true;
        }
        row_y += 40.0f;
        return false;
    };

    DrawText("Sito di lancio", static_cast<int>(bounds.x) + 18, static_cast<int>(row_y) - 6, 18, Color {226, 232, 240, 255});
    row_y += 24.0f;
    bool environment_changed = false;
    environment_changed = apply_environment_step(site.latitude_deg, 0.05, "Latitude", "deg") || environment_changed;
    environment_changed = apply_environment_step(site.longitude_deg, 0.05, "Longitude", "deg") || environment_changed;
    environment_changed = apply_environment_step(site.elevation_m, 5.0, "Elevation", "m") || environment_changed;

    DrawText("Meteo al suolo", static_cast<int>(bounds.x) + 18, static_cast<int>(row_y) + 2, 18, Color {226, 232, 240, 255});
    row_y += 28.0f;
    environment_changed = apply_environment_step(weather.temperature_c, 1.0, "Temp", "C") || environment_changed;
    environment_changed = apply_environment_step(weather.pressure_hpa, 1.5, "Pressure", "hPa") || environment_changed;
    environment_changed = apply_environment_step(weather.humidity_percent, 2.0, "Humidity", "%") || environment_changed;
    environment_changed = apply_environment_step(weather.wind_speed_mps, 0.5, "Wind Avg", "m/s") || environment_changed;
    environment_changed = apply_environment_step(weather.wind_direction_deg, 5.0, "Wind Dir", "deg") || environment_changed;
    environment_changed = apply_environment_step(weather.wind_gust_mps, 0.5, "Wind Gust", "m/s") || environment_changed;

    DrawText("Recovery", static_cast<int>(bounds.x) + 18, static_cast<int>(row_y) + 2, 18, Color {226, 232, 240, 255});
    row_y += 28.0f;
    auto& recovery = vehicle.recovery_system;
    const int deploy_alt_delta = drawStepper(
        Rectangle {bounds.x + 18.0f, row_y, bounds.width - 36.0f, 30.0f},
        "Deploy Alt",
        std::format("{:.0f} m", recovery.deployment_altitude_m));
    if (deploy_alt_delta != 0) {
        recovery.deployment_altitude_m += 10.0 * static_cast<double>(deploy_alt_delta);
        environment_changed = true;
    }
    row_y += 40.0f;
    const int chute_area_delta = drawStepper(
        Rectangle {bounds.x + 18.0f, row_y, bounds.width - 36.0f, 30.0f},
        "Chute Area",
        std::format("{:.2f} m2", recovery.parachute_area_m2));
    if (chute_area_delta != 0) {
        recovery.parachute_area_m2 += 0.05 * static_cast<double>(chute_area_delta);
        environment_changed = true;
    }
    row_y += 40.0f;
    const int deploy_delay_delta = drawStepper(
        Rectangle {bounds.x + 18.0f, row_y, bounds.width - 36.0f, 30.0f},
        "Deploy Delay",
        std::format("{:.1f} s", recovery.deployment_delay_s));
    if (deploy_delay_delta != 0) {
        recovery.deployment_delay_s += 0.2 * static_cast<double>(deploy_delay_delta);
        environment_changed = true;
    }
    row_y += 46.0f;

    if (environment_changed) {
        site.latitude_deg = std::clamp(site.latitude_deg, 35.0, 47.5);
        site.longitude_deg = std::clamp(site.longitude_deg, 6.0, 19.0);
        site.elevation_m = std::clamp(site.elevation_m, -50.0, 3500.0);
        weather.temperature_c = std::clamp(weather.temperature_c, -25.0, 45.0);
        weather.pressure_hpa = std::clamp(weather.pressure_hpa, 850.0, 1055.0);
        weather.humidity_percent = std::clamp(weather.humidity_percent, 0.0, 100.0);
        weather.wind_speed_mps = std::clamp(weather.wind_speed_mps, 0.0, 35.0);
        weather.wind_direction_deg = std::fmod(weather.wind_direction_deg + 360.0, 360.0);
        weather.wind_gust_mps = std::clamp(weather.wind_gust_mps, 0.0, 45.0);
        recovery.deployment_altitude_m = std::clamp(recovery.deployment_altitude_m, 30.0, 1200.0);
        recovery.parachute_area_m2 = std::clamp(recovery.parachute_area_m2, 0.1, 8.0);
        recovery.deployment_delay_s = std::clamp(recovery.deployment_delay_s, 0.0, 15.0);
        environment.setLaunchSite(site);
        environment.setSurfaceWeather(weather);
        resetSimulationRuntime(vehicle, runtime);
    }

    const ::Rectangle manual_button {bounds.x + 18.0f, row_y + 4.0f, bounds.width - 36.0f, 30.0f};
    if (drawButton(
            manual_button,
            environment.weatherDataSource() == rocket::WeatherDataSource::OpenMeteoReady
                ? "Weather Source: Open-Meteo Ready"
                : environment.weatherDataSource() == rocket::WeatherDataSource::OpenWeatherMapReady
                      ? "Weather Source: OpenWeather Ready"
                      : "Weather Source: Manual",
            true,
            Color {84, 136, 199, 224},
            ButtonStyle::Outlined)) {
        const auto next_source =
            environment.weatherDataSource() == rocket::WeatherDataSource::Manual
                ? rocket::WeatherDataSource::OpenMeteoReady
                : environment.weatherDataSource() == rocket::WeatherDataSource::OpenMeteoReady
                      ? rocket::WeatherDataSource::OpenWeatherMapReady
                      : rocket::WeatherDataSource::Manual;
        environment.setWeatherDataSource(next_source);
    }

    DrawText("Cluster motori", static_cast<int>(bounds.x) + 18, static_cast<int>(row_y), 18, Color {226, 232, 240, 255});
    for (std::size_t index = 0; index < vehicle.cluster.motorCount(); ++index) {
        const bool failed = vehicle.cluster.motorFailed(index);
        const ::Rectangle motor_button {
            bounds.x + 18.0f,
            row_y + 30.0f + static_cast<float>(index) * 40.0f,
            bounds.width - 36.0f,
            32.0f
        };
        const std::string label = failed ? std::format("Motor {} FAILED", index + 1) : std::format("Motor {} OK", index + 1);
        if (drawButton(motor_button, label, !failed, failed ? Color {198, 82, 82, 224} : Color {84, 136, 199, 224}, failed ? ButtonStyle::Outlined : ButtonStyle::Contained)) {
            vehicle.cluster.setMotorFailed(index, !failed);
            resetSimulationRuntime(vehicle, runtime);
        }
    }

    DrawText(
        environment.weatherDataSource() == rocket::WeatherDataSource::OpenMeteoReady
            ? "Source  Open-Meteo"
            : environment.weatherDataSource() == rocket::WeatherDataSource::OpenWeatherMapReady
                  ? "Source  OpenWeatherMap"
                  : "Source  Manual",
        static_cast<int>(bounds.x) + 18,
        static_cast<int>(bounds.y + bounds.height - 34.0f),
        14,
        Color {148, 163, 184, 255});
}

void drawSimulationTimeline(const ::Rectangle& bounds, const SimulationRuntime& runtime, const rocket::VehicleModel& vehicle) {
    drawPanel(bounds, "Sequenza Missione");
    const double burn_time_s = 2.4;
    const double normalized_progress = std::clamp(runtime.time_s / 12.0, 0.0, 1.0);
    const double burn_progress = std::clamp(runtime.time_s / burn_time_s, 0.0, 1.0);

    DrawText("Progressione volo", static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 36, 13, Color {148, 163, 184, 255});

    const ::Rectangle bar {bounds.x + 18.0f, bounds.y + 54.0f, bounds.width - 36.0f, 18.0f};
    DrawRectangleRounded(bar, 0.4f, 6, Color {30, 41, 59, 255});
    DrawRectangleRounded(
        Rectangle {bar.x, bar.y, static_cast<float>(bar.width * normalized_progress), bar.height},
        0.4f,
        6,
        Color {14, 165, 233, 220});

    DrawText("Fase boost", static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 76, 13, Color {148, 163, 184, 255});
    const ::Rectangle burn_bar {bounds.x + 18.0f, bounds.y + 92.0f, bounds.width - 36.0f, 12.0f};
    DrawRectangleRounded(burn_bar, 0.4f, 6, Color {39, 39, 42, 255});
    DrawRectangleRounded(
        Rectangle {burn_bar.x, burn_bar.y, static_cast<float>(burn_bar.width * burn_progress), burn_bar.height},
        0.4f,
        6,
        vehicle.cluster.isBurning(runtime.time_s) ? Color {249, 115, 22, 230} : Color {82, 82, 91, 230});

    DrawText(std::format("Tempo missione {:.2f} s", runtime.time_s).c_str(), static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 118, 16, Color {226, 232, 240, 255});
    if (runtime.time_s < 0.01) {
        DrawText("La timeline si popola quando la missione entra in volo.", static_cast<int>(bounds.x) + 230, static_cast<int>(bounds.y) + 118, 13, Color {148, 163, 184, 255});
    }
}

void drawTrajectoryOverview(const ::Rectangle& bounds, const SimulationRuntime& runtime) {
    drawPanel(bounds, "Panoramica Traiettoria");

    if (runtime.trajectory_history.size() < 2) {
        drawInfoCard(
            Rectangle {bounds.x + 18.0f, bounds.y + 48.0f, bounds.width - 36.0f, 62.0f},
            "Traiettoria non disponibile",
            "Avvia il volo per generare il profilo completo e i marker di burnout, apogeo e impatto.",
            Color {56, 189, 248, 255});
        return;
    }

    const ::Rectangle profile {
        bounds.x + 18.0f,
        bounds.y + 48.0f,
        bounds.width - 220.0f,
        bounds.height - 66.0f
    };
    const ::Rectangle summary {
        bounds.x + bounds.width - 188.0f,
        bounds.y + 48.0f,
        170.0f,
        bounds.height - 66.0f
    };

    DrawRectangleRounded(profile, 0.04f, 8, Color {10, 18, 30, 244});
    DrawRectangleRoundedLinesEx(profile, 0.04f, 8, 1.0f, Color {45, 64, 89, 255});
    DrawRectangleRounded(summary, 0.08f, 8, Color {11, 20, 33, 236});
    DrawRectangleRoundedLinesEx(summary, 0.08f, 8, 1.0f, Color {45, 64, 89, 255});

    double max_range_m = 1.0;
    double max_altitude_m = std::max(runtime.max_altitude_m, 1.0);
    for (const auto& sample : runtime.trajectory_history) {
        max_range_m = std::max(max_range_m, horizontalRangeM(sample.state.position_m));
        max_altitude_m = std::max(max_altitude_m, sample.state.position_m.z);
    }

    for (int index = 0; index <= 4; ++index) {
        const float t = static_cast<float>(index) / 4.0f;
        const float y = profile.y + 12.0f + t * (profile.height - 28.0f);
        DrawLineEx(
            ::Vector2 {profile.x + 10.0f, y},
            ::Vector2 {profile.x + profile.width - 10.0f, y},
            1.0f,
            index == 4 ? Color {71, 85, 105, 180} : Color {51, 65, 85, 100});
        const double altitude_label = max_altitude_m * (1.0 - static_cast<double>(t));
        DrawText(std::format("{:.0f} m", altitude_label).c_str(), static_cast<int>(profile.x) + 12, static_cast<int>(y) - 16, 13, Color {100, 116, 139, 255});
    }

    auto profile_point = [&](const rocket::Vector3& position_m) {
        const double range_ratio = std::clamp(horizontalRangeM(position_m) / max_range_m, 0.0, 1.0);
        const double altitude_ratio = std::clamp(position_m.z / max_altitude_m, 0.0, 1.0);
        return ::Vector2 {
            profile.x + 18.0f + static_cast<float>(range_ratio) * (profile.width - 36.0f),
            profile.y + profile.height - 18.0f - static_cast<float>(altitude_ratio) * (profile.height - 36.0f)
        };
    };

    for (std::size_t index = 1; index < runtime.trajectory_history.size(); ++index) {
        const auto a = profile_point(runtime.trajectory_history[index - 1].state.position_m);
        const auto b = profile_point(runtime.trajectory_history[index].state.position_m);
        const unsigned char alpha = static_cast<unsigned char>(70 + (170 * static_cast<int>(index)) / static_cast<int>(runtime.trajectory_history.size()));
        DrawLineEx(a, b, 2.0f, Color {64, 196, 255, alpha});
    }

    auto draw_marker = [&](const rocket::Vector3& position_m, const char* label, Color color) {
        const ::Vector2 point = profile_point(position_m);
        DrawCircleV(point, 5.0f, color);
        DrawText(label, static_cast<int>(point.x) + 8, static_cast<int>(point.y) - 10, 13, color);
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

    const rocket::FlightState render_state = rocket::currentRenderState(runtime);
    const ::Vector2 current_point = profile_point(render_state.position_m);
    const bool keyframe_active = runtime.keyframe_preview_active;
    DrawCircleV(current_point, 6.0f, keyframe_active ? Color {244, 114, 182, 255} : runtime.replay_active ? Color {251, 191, 36, 255} : Color {125, 211, 252, 255});
    DrawText(keyframe_active ? "Keyframe" : runtime.replay_active ? "Replay" : "Current", static_cast<int>(current_point.x) + 8, static_cast<int>(current_point.y) + 4, 13, keyframe_active ? Color {244, 114, 182, 255} : runtime.replay_active ? Color {251, 191, 36, 255} : Color {125, 211, 252, 255});

    DrawText("0 m", static_cast<int>(profile.x) + 18, static_cast<int>(profile.y + profile.height - 16.0f), 13, Color {100, 116, 139, 255});
    DrawText(std::format("{:.0f} m range", max_range_m).c_str(), static_cast<int>(profile.x + profile.width - 120.0f), static_cast<int>(profile.y + profile.height - 16.0f), 13, Color {100, 116, 139, 255});

    float row_y = summary.y + 12.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Modalita", runtime.replay_active ? "Replay" : "Live");
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Tempo volo", std::format("{:.2f} s", runtime.time_s));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Quota attuale", std::format("{:.1f} m", render_state.position_m.z));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Downrange", std::format("{:.1f} m", horizontalRangeM(render_state.position_m)));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Quota max", std::format("{:.1f} m", runtime.apogee_point_m.z));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Range impatto", std::format("{:.1f} m", horizontalRangeM(runtime.impact_point_m)));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Campioni", std::format("{}", runtime.trajectory_history.size()));
    row_y += 34.0f;

    DrawText("Eventi chiave", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 16, Color {226, 232, 240, 255});
    row_y += 24.0f;
    DrawText("Green  Launch", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 14, Color {34, 197, 94, 255});
    row_y += 20.0f;
    DrawText("Orange Burnout", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 14, Color {249, 115, 22, 255});
    row_y += 20.0f;
    DrawText("Purple Apogee", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 14, Color {168, 85, 247, 255});
    row_y += 20.0f;
    DrawText("Red    Impact", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 14, Color {239, 68, 68, 255});
}

void drawSimulationEvents(const ::Rectangle& bounds, const SimulationRuntime& runtime, const rocket::VehicleModel& vehicle, const rocket::SimulationSnapshot& snapshot) {
    drawPanel(bounds, "Stato Missione");

    std::string primary_event = "Awaiting launch";
    if (runtime.keyframe_preview_active) {
        primary_event = "Keyframe analysis mode";
    } else if (runtime.time_s > 0.0 && vehicle.cluster.isBurning(runtime.time_s)) {
        primary_event = "Boost phase active";
    } else if (runtime.time_s > 2.4 && snapshot.state.position_m.z > 0.0 && snapshot.state.velocity_mps.z > 0.0) {
        primary_event = "Coast to apogee";
    } else if (snapshot.parachute_deployed) {
        primary_event = "Recovery descent";
    } else if (snapshot.state.position_m.z > 0.0 && snapshot.state.velocity_mps.z < -0.5) {
        primary_event = "Ballistic descent";
    } else if (snapshot.max_altitude_m > 0.0 && std::abs(snapshot.state.velocity_mps.z) < 0.5) {
        primary_event = "Apogee window";
    } else if (snapshot.state.position_m.z <= 0.0 && runtime.time_s > 0.5) {
        primary_event = "Ground contact";
    }

    std::string next_action = "Prossimo passo: premi Avvia / Riprendi per iniziare la missione.";
    if (runtime.keyframe_preview_active) {
        next_action = "Analisi puntuale attiva: premi K per il prossimo keyframe oppure SPACE per restare in pausa.";
    } else if (runtime.time_s > 0.0 && vehicle.cluster.isBurning(runtime.time_s)) {
        next_action = "Monitoraggio: osserva telemetria, marker e traiettoria mentre il boost e attivo.";
    } else if (runtime.replay_active) {
        next_action = "Replay attivo: usa la panoramica per leggere eventi chiave e range.";
    } else if (snapshot.parachute_deployed) {
        next_action = "Recovery in corso: confronta quota, rateo di discesa e zona di impatto.";
    } else if (runtime.impact_recorded) {
        next_action = "Missione completata: avvia un replay o modifica lo scenario per il prossimo test.";
    }

    DrawText(primary_event.c_str(), static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 48, 22, Color {248, 250, 252, 255});
    DrawText(
        snapshot.static_margin_calibers >= 1.0 ? "Stabilita nominale" : "Margine statico basso",
        static_cast<int>(bounds.x) + 18,
        static_cast<int>(bounds.y) + 82,
        16,
        snapshot.static_margin_calibers >= 1.0 ? Color {74, 222, 128, 255} : Color {248, 113, 113, 255});
    drawMetricCard(
        Rectangle {bounds.x + 18.0f, bounds.y + 112.0f, bounds.width - 36.0f, 42.0f},
        "Profilo ogiva",
        snapshot.nose_shape_label,
        Color {56, 189, 248, 255});
    drawMetricCard(
        Rectangle {bounds.x + 18.0f, bounds.y + 160.0f, bounds.width - 36.0f, 42.0f},
        "Profilo pinne",
        snapshot.fin_shape_label,
        Color {168, 85, 247, 255});
    DrawText(
        snapshot.parachute_deployed ? "Recovery dispiegato" : "Recovery ancora non dispiegato",
        static_cast<int>(bounds.x) + 18,
        static_cast<int>(bounds.y) + 214,
        15,
        snapshot.parachute_deployed ? Color {74, 222, 128, 255} : Color {148, 163, 184, 255});
    drawInlineHint(
        Rectangle {bounds.x + 18.0f, bounds.y + 236.0f, bounds.width - 36.0f, 26.0f},
        next_action,
        Color {251, 191, 36, 255});
}


