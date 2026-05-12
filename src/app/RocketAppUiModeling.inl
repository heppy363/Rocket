void drawPieceLibrary(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    MotorEditorState& motor_editor,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime,
    const ::Rectangle& bounds) {
    drawPanel(bounds, "Libreria Preset");
    drawSectionCaption(
        Rectangle {bounds.x + 16.0f, bounds.y + 50.0f, bounds.width - 32.0f, 34.0f},
        "Preset progetto",
        "Configurazioni complete per iniziare piu rapidamente.");

    constexpr std::array<rocket::RocketPreset, 5> presets {
        rocket::RocketPreset::ResearchStarter,
        rocket::RocketPreset::SportTrainer,
        rocket::RocketPreset::HighAltitude,
        rocket::RocketPreset::MinimumDiameter,
        rocket::RocketPreset::HeavyLift
    };

    for (int index = 0; index < static_cast<int>(presets.size()); ++index) {
        const auto preset = presets[static_cast<std::size_t>(index)];
        const ::Rectangle button {bounds.x + 16.0f, bounds.y + 92.0f + static_cast<float>(index) * 34.0f, bounds.width - 32.0f, 28.0f};
        if (drawButton(button, std::string(rocket::rocketPresetLabel(preset)), app_state.active_preset == preset, Color {61, 135, 245, 228}, app_state.active_preset == preset ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
            app_state.active_preset = preset;
            vehicle.geometry = rocket::makePresetGeometry(preset);
            rebuildVehicle(vehicle, mesh_generator, runtime);
        }
    }

    float row_y = bounds.y + 286.0f;
    drawSectionCaption(
        Rectangle {bounds.x + 16.0f, row_y, bounds.width - 32.0f, 30.0f},
        "Preset componente",
        "Varianti contestuali basate sulla parte selezionata.");
    row_y += 42.0f;

    auto preset_button = [&](const char* label, const auto& fn) {
        const ::Rectangle button {bounds.x + 16.0f, row_y, bounds.width - 32.0f, 28.0f};
        if (drawButton(button, label, false, Color {84, 136, 199, 224}, ButtonStyle::Outlined)) {
            fn();
            rebuildVehicle(vehicle, mesh_generator, runtime);
        }
        row_y += 34.0f;
    };

    const char* library_note = "I preset restano procedurali e si adattano alla struttura corrente del razzo.";

    switch (app_state.selection) {
    case ComponentSelection::NoseCone:
        library_note = "Ogive presets trade drag, volume and static stability using realistic slenderness choices.";
        preset_button("Conical Sprint", [&] {
            vehicle.geometry.nose_length_m = 0.28;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::Conical;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 0.88, .shoulder_radius_scale = 1.0};
        });
        preset_button("Conical Long Range", [&] {
            vehicle.geometry.nose_length_m = 0.42;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::Conical;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 0.82, .shoulder_radius_scale = 0.98};
        });
        preset_button("Haack Low Drag", [&] {
            vehicle.geometry.nose_length_m = 0.52;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::LdHaack;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 0.94, .shoulder_radius_scale = 1.02};
        });
        preset_button("Ogive Stable", [&] {
            vehicle.geometry.nose_length_m = 0.44;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::TangentOgive;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 1.02, .shoulder_radius_scale = 1.0};
        });
        preset_button("Parabolic Payload", [&] {
            vehicle.geometry.nose_length_m = 0.38;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::Parabolic;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 1.08, .shoulder_radius_scale = 1.03};
        });
        break;
    case ComponentSelection::BodyTube:
        library_note = "Body presets change volume, sectional density, wall mass and inertial stiffness.";
        preset_button("Minimum Diameter", [&] {
            vehicle.geometry.body_length_m = 3.2;
            vehicle.geometry.body_diameter_m = 0.092;
            vehicle.geometry.wall_thickness_m = 0.0023;
            vehicle.geometry.body_controls = {.fore_radius_scale = 0.98, .mid_radius_scale = 1.0, .aft_radius_scale = 0.98};
        });
        preset_button("Avionics Bay", [&] {
            vehicle.geometry.body_length_m = 2.4;
            vehicle.geometry.body_diameter_m = 0.104;
            vehicle.geometry.wall_thickness_m = 0.0030;
            vehicle.geometry.body_controls = {.fore_radius_scale = 1.0, .mid_radius_scale = 1.02, .aft_radius_scale = 1.0};
        });
        preset_button("Long Burner", [&] {
            vehicle.geometry.body_length_m = 3.8;
            vehicle.geometry.body_diameter_m = 0.110;
            vehicle.geometry.wall_thickness_m = 0.0032;
            vehicle.geometry.body_controls = {.fore_radius_scale = 0.97, .mid_radius_scale = 1.0, .aft_radius_scale = 0.99};
        });
        preset_button("Heavy Structure", [&] {
            vehicle.geometry.body_length_m = 2.9;
            vehicle.geometry.body_diameter_m = 0.125;
            vehicle.geometry.wall_thickness_m = 0.0038;
            vehicle.geometry.body_controls = {.fore_radius_scale = 1.0, .mid_radius_scale = 1.02, .aft_radius_scale = 0.98};
        });
        preset_button("Wide Lift Core", [&] {
            vehicle.geometry.body_length_m = 3.3;
            vehicle.geometry.body_diameter_m = 0.140;
            vehicle.geometry.wall_thickness_m = 0.0040;
            vehicle.geometry.body_controls = {.fore_radius_scale = 1.0, .mid_radius_scale = 1.04, .aft_radius_scale = 1.0};
        });
        break;
    case ComponentSelection::Transition:
        library_note = "Transition presets affect base drag, aft closure efficiency and fin airflow quality.";
        preset_button("Straight Interstage", [&] {
            vehicle.geometry.transition_length_m = 0.08;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.94, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Conical;
            vehicle.geometry.transition_controls.mid_radius_scale = 1.0;
        });
        preset_button("Boat Tail", [&] {
            vehicle.geometry.transition_length_m = 0.22;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.72, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Curved;
            vehicle.geometry.transition_controls.mid_radius_scale = 0.86;
        });
        preset_button("Conical Coupler", [&] {
            vehicle.geometry.transition_length_m = 0.16;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.82, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Conical;
            vehicle.geometry.transition_controls.mid_radius_scale = 1.0;
        });
        preset_button("Aggressive Tail", [&] {
            vehicle.geometry.transition_length_m = 0.24;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.64, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Curved;
            vehicle.geometry.transition_controls.mid_radius_scale = 0.78;
        });
        preset_button("Payload Shoulder", [&] {
            vehicle.geometry.transition_length_m = 0.18;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.90, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Curved;
            vehicle.geometry.transition_controls.mid_radius_scale = 1.08;
        });
        break;
    case ComponentSelection::FinSet:
        library_note = "Fin presets alter restoring force, damping and transonic drag in a physically meaningful way.";
        preset_button("Sport Trapezoid", [&] {
            vehicle.geometry.fin_shape = rocket::FinShape::Trapezoidal;
            vehicle.geometry.fin_root_chord_m = 0.26;
            vehicle.geometry.fin_tip_chord_m = 0.13;
            vehicle.geometry.fin_span_m = 0.14;
            vehicle.geometry.fin_sweep_length_m = 0.06;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = 0.0, .tip_te_offset_m = 0.0, .span_scale = 1.0, .thickness_scale = 1.0};
        });
        preset_button("Supersonic Airfoil", [&] {
            vehicle.geometry.fin_shape = rocket::FinShape::Airfoil;
            vehicle.geometry.fin_root_chord_m = 0.24;
            vehicle.geometry.fin_tip_chord_m = 0.09;
            vehicle.geometry.fin_span_m = 0.14;
            vehicle.geometry.fin_sweep_length_m = 0.12;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = 0.03, .tip_te_offset_m = -0.01, .span_scale = 0.96, .thickness_scale = 0.88};
        });
        preset_button("Clipped Delta", [&] {
            vehicle.geometry.fin_shape = rocket::FinShape::Trapezoidal;
            vehicle.geometry.fin_root_chord_m = 0.30;
            vehicle.geometry.fin_tip_chord_m = 0.07;
            vehicle.geometry.fin_span_m = 0.13;
            vehicle.geometry.fin_sweep_length_m = 0.15;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = 0.02, .tip_te_offset_m = -0.02, .span_scale = 0.95, .thickness_scale = 0.9};
        });
        preset_button("Stable Elliptical", [&] {
            vehicle.geometry.fin_shape = rocket::FinShape::Elliptical;
            vehicle.geometry.fin_root_chord_m = 0.34;
            vehicle.geometry.fin_tip_chord_m = 0.17;
            vehicle.geometry.fin_span_m = 0.20;
            vehicle.geometry.fin_sweep_length_m = 0.05;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = 0.0, .tip_te_offset_m = 0.02, .span_scale = 1.04, .thickness_scale = 1.05};
        });
        preset_button("Heavy Lift Canards", [&] {
            vehicle.geometry.fin_shape = rocket::FinShape::Airfoil;
            vehicle.geometry.fin_root_chord_m = 0.38;
            vehicle.geometry.fin_tip_chord_m = 0.18;
            vehicle.geometry.fin_span_m = 0.24;
            vehicle.geometry.fin_sweep_length_m = 0.08;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = -0.01, .tip_te_offset_m = 0.03, .span_scale = 1.10, .thickness_scale = 1.12};
        });
        break;
    case ComponentSelection::MotorMount:
        library_note = "Motor presets map to real thrust, burn time, propellant mass and cluster radius.";
        preset_button("Single Sustainer", [&] {
            motor_editor.motor_count = 1;
            motor_editor.mount_radius_m = 0.0;
            motor_editor.max_thrust_n = 420.0;
            motor_editor.burn_time_s = 2.8;
            motor_editor.propellant_mass_kg = 0.54;
            motor_editor.cant_angle_deg = 0.0;
            rebuildMotorCluster(vehicle, motor_editor, mesh_generator, runtime);
        });
        preset_button("Twin Booster", [&] {
            motor_editor.motor_count = 2;
            motor_editor.mount_radius_m = 0.026;
            motor_editor.max_thrust_n = 210.0;
            motor_editor.burn_time_s = 2.2;
            motor_editor.propellant_mass_kg = 0.20;
            motor_editor.cant_angle_deg = 0.0;
            rebuildMotorCluster(vehicle, motor_editor, mesh_generator, runtime);
        });
        preset_button("3-Motor Cluster", [&] {
            motor_editor.motor_count = 3;
            motor_editor.mount_radius_m = 0.032;
            motor_editor.max_thrust_n = 220.0;
            motor_editor.burn_time_s = 2.1;
            motor_editor.propellant_mass_kg = 0.22;
            motor_editor.cant_angle_deg = 0.0;
            rebuildMotorCluster(vehicle, motor_editor, mesh_generator, runtime);
        });
        preset_button("4-Motor Cluster", [&] {
            motor_editor.motor_count = 4;
            motor_editor.mount_radius_m = 0.038;
            motor_editor.max_thrust_n = 240.0;
            motor_editor.burn_time_s = 2.5;
            motor_editor.propellant_mass_kg = 0.26;
            motor_editor.cant_angle_deg = 0.0;
            rebuildMotorCluster(vehicle, motor_editor, mesh_generator, runtime);
        });
        preset_button("5-Motor Ring", [&] {
            motor_editor.motor_count = 5;
            motor_editor.mount_radius_m = 0.040;
            motor_editor.max_thrust_n = 205.0;
            motor_editor.burn_time_s = 2.9;
            motor_editor.propellant_mass_kg = 0.23;
            motor_editor.cant_angle_deg = 1.5;
            rebuildMotorCluster(vehicle, motor_editor, mesh_generator, runtime);
        });
        break;
    case ComponentSelection::Payload:
        library_note = "Payload presets set real mass and bay length, shifting CG and inertial response.";
        preset_button("Compact Payload", [&] {
            vehicle.geometry.payload_length_m = 0.14;
            vehicle.geometry.payload_mass_kg = 0.45;
        });
        preset_button("Avionics + Camera", [&] {
            vehicle.geometry.payload_length_m = 0.26;
            vehicle.geometry.payload_mass_kg = 1.15;
        });
        preset_button("Dual Deploy Bay", [&] {
            vehicle.geometry.payload_length_m = 0.22;
            vehicle.geometry.payload_mass_kg = 0.72;
        });
        preset_button("Science Probe", [&] {
            vehicle.geometry.payload_length_m = 0.30;
            vehicle.geometry.payload_mass_kg = 1.35;
        });
        preset_button("Heavy Recovery Stack", [&] {
            vehicle.geometry.payload_length_m = 0.28;
            vehicle.geometry.payload_mass_kg = 1.85;
        });
        break;
    }

    drawInlineHint(
        Rectangle {bounds.x + 16.0f, bounds.y + bounds.height - 44.0f, bounds.width - 32.0f, 28.0f},
        library_note,
        Color {84, 136, 199, 224});
}

void drawModelingOverlays(AppState& app_state, const ::Rectangle& bounds) {
    drawPanel(bounds, "Viewport & Grid");

    const float col_w = (bounds.width - 34.0f) * 0.5f;
    const ::Rectangle metrics_button {bounds.x + 14.0f, bounds.y + 50.0f, col_w, 28.0f};
    const ::Rectangle snap_button {bounds.x + 20.0f + col_w, bounds.y + 50.0f, col_w, 28.0f};
    const ::Rectangle local_grid_button {bounds.x + 14.0f, bounds.y + 84.0f, col_w, 28.0f};
    const ::Rectangle handles_button {bounds.x + 20.0f + col_w, bounds.y + 84.0f, col_w, 28.0f};
    const ::Rectangle wireframe_button {bounds.x + 14.0f, bounds.y + 118.0f, col_w, 28.0f};
    const ::Rectangle diagnostics_button {bounds.x + 20.0f + col_w, bounds.y + 118.0f, col_w, 28.0f};

    if (drawButton(metrics_button, "Metrics", app_state.show_metrics_overlay, Color {14, 116, 144, 220})) {
        app_state.show_metrics_overlay = !app_state.show_metrics_overlay;
    }
    if (drawButton(snap_button, app_state.snap_to_grid ? "Snap ON" : "Snap OFF", app_state.snap_to_grid, Color {14, 116, 144, 220})) {
        app_state.snap_to_grid = !app_state.snap_to_grid;
    }
    if (drawButton(local_grid_button, app_state.show_local_grid ? "Local Grid" : "Grid Hidden", app_state.show_local_grid, Color {14, 116, 144, 220})) {
        app_state.show_local_grid = !app_state.show_local_grid;
    }
    if (drawButton(handles_button, app_state.show_snap_points ? "Handles ON" : "Handles OFF", app_state.show_snap_points, Color {14, 116, 144, 220})) {
        app_state.show_snap_points = !app_state.show_snap_points;
    }
    if (drawButton(wireframe_button, "Wireframe", app_state.show_wireframe_hint, Color {14, 116, 144, 220})) {
        app_state.show_wireframe_hint = !app_state.show_wireframe_hint;
    }
    if (drawButton(diagnostics_button, "Diagnostics", app_state.show_diagnostics, Color {14, 116, 144, 220})) {
        app_state.show_diagnostics = !app_state.show_diagnostics;
    }

    drawSectionCaption(
        Rectangle {bounds.x + 16.0f, bounds.y + 158.0f, bounds.width - 32.0f, 30.0f},
        "Griglia locale",
        "Definisce la precisione visuale del componente attivo.");
    const int spacing_delta = drawStepper(
        Rectangle {bounds.x + 14.0f, bounds.y + 180.0f, bounds.width - 28.0f, 30.0f},
        "Passo",
        std::format("{:.3f} m", app_state.grid_spacing_m));
    if (spacing_delta != 0) {
        app_state.grid_spacing_m = std::clamp(app_state.grid_spacing_m + 0.005 * static_cast<double>(spacing_delta), 0.005, 0.25);
    }

    const int extent_delta = drawStepper(
        Rectangle {bounds.x + 14.0f, bounds.y + 214.0f, bounds.width - 28.0f, 30.0f},
        "Estensione",
        std::format("{} cells", app_state.grid_extent_cells));
    if (extent_delta != 0) {
        app_state.grid_extent_cells = std::clamp(app_state.grid_extent_cells + extent_delta, 4, 40);
    }

}

void handleGeometryEdits(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    MotorEditorState& motor_editor,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime,
    const ::Rectangle& bounds) {
    drawPanel(bounds, "Inspector Componente");

    DrawText("Componente", static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 50, 15, Color {148, 163, 184, 255});
    constexpr std::array<const char*, 6> selection_names {
        "Nose Cone",
        "Body Tube",
        "Transition",
        "Fin Set",
        "Motor Mount",
        "Payload Section"
    };
    DrawText(selection_names[static_cast<std::size_t>(app_state.selection)], static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 72, 20, Color {226, 232, 240, 255});

    const auto selection_hint = [&]() -> const char* {
        switch (app_state.selection) {
        case ComponentSelection::NoseCone:
            return "Clicca l'ogiva in viewport, poi lavora sulla sua gabbia di editing e rifinisci i parametri qui.";
        case ComponentSelection::BodyTube:
            return "Seleziona il corpo direttamente sul modello e scolpisci volume, rigidita e massa dalla gabbia visibile.";
        case ComponentSelection::Transition:
            return "Clicca la transizione sul razzo per richiamare la sua edit cage e controllare raccordo e diametro di coda.";
        case ComponentSelection::FinSet:
            return "Le pinne ora si selezionano in viewport: cliccale e modifica posizione, span e profilo dalla gabbia.";
        case ComponentSelection::MotorMount:
            return "Seleziona la zona motore direttamente sul modello per aprire il controllo del cluster e del raggio.";
        case ComponentSelection::Payload:
            return "Clicca la payload section nel modello per editarne gabbia, massa e lunghezza senza uscire dalla viewport.";
        }
        return "Modifica il componente attivo e osserva gli effetti in viewport.";
    }();

    drawInlineHint(
        Rectangle {bounds.x + 18.0f, bounds.y + 104.0f, bounds.width - 36.0f, 44.0f},
        selection_hint,
        Color {56, 189, 248, 255});
    drawInfoCard(
        Rectangle {bounds.x + 18.0f, bounds.y + 154.0f, bounds.width - 36.0f, 76.0f},
        std::format("Tool {} {}", modelingToolLabel(app_state.modeling_tool), modelingToolShortcut(app_state.modeling_tool)),
        app_state.active_handle == HandleKind::None
            ? std::format("{} Nessun handle attivo: clicca un pezzo del razzo in viewport per selezionarlo, poi scegli un punto della sua gabbia.", modelingToolHint(app_state.modeling_tool))
            : std::format("{} Handle in focus: {}.", modelingToolHint(app_state.modeling_tool), handleKindLabel(app_state.active_handle)),
        Color {251, 191, 36, 255});

    bool changed = false;
    float row_y = bounds.y + 246.0f;
    const float row_height = 38.0f;
    const float row_width = bounds.width - 36.0f;

    auto apply_step = [&](double& value, double step, int direction) {
        if (direction != 0) {
            value += step * static_cast<double>(direction);
            changed = true;
        }
    };

    auto apply_material = [&](const char* title, rocket::ComponentMaterial& material) {
        changed = drawMaterialChooser(
                      Rectangle {bounds.x + 18.0f, row_y, row_width, 138.0f},
                      title,
                      material) ||
                  changed;
        const auto definition = rocket::materialDefinition(material);
        const auto selected_component = static_cast<rocket::ComponentType>(app_state.selection);
        const double component_q_limit_pa = rocket::estimateComponentDynamicPressureLimitPa(selected_component, vehicle.geometry);
        drawInfoCard(
            Rectangle {bounds.x + 18.0f, row_y + 108.0f, row_width, 62.0f},
            std::format(
                "{}  |  rho {:.0f} kg/m3  |  E {:.1f} GPa",
                definition.label,
                definition.density_kg_per_m3,
                definition.youngs_modulus_gpa),
            std::format(
                "Yield {:.0f} MPa  |  Tmax {:.0f} C  |  q rec {:.0f} kPa. {}",
                definition.yield_strength_mpa,
                definition.max_service_temperature_c,
                component_q_limit_pa / 1000.0,
                definition.note),
            Color {96, 165, 250, 255});
        row_y += 178.0f;
    };

    switch (app_state.selection) {
    case ComponentSelection::NoseCone: {
        apply_step(
            vehicle.geometry.nose_length_m,
            0.02,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Length",
                std::format("{:.2f} m", vehicle.geometry.nose_length_m)));
        row_y += row_height;
        apply_material("Material", vehicle.geometry.nose_material);
        apply_step(
            vehicle.geometry.nose_controls.mid_radius_scale,
            0.05,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Mid Radius",
                std::format("{:.2f}", vehicle.geometry.nose_controls.mid_radius_scale)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.nose_controls.shoulder_radius_scale,
            0.05,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Shoulder",
                std::format("{:.2f}", vehicle.geometry.nose_controls.shoulder_radius_scale)));
        row_y += row_height;

        constexpr std::array<rocket::NoseConeShape, 4> nose_shapes {
            rocket::NoseConeShape::Conical,
            rocket::NoseConeShape::TangentOgive,
            rocket::NoseConeShape::Parabolic,
            rocket::NoseConeShape::LdHaack
        };
        constexpr std::array<const char*, 4> labels {"Conical", "Tangent Ogive", "Parabolic", "LD-Haack"};
        for (int index = 0; index < 4; ++index) {
            const ::Rectangle button {
                bounds.x + 16.0f + static_cast<float>(index % 2) * ((row_width - 12.0f) * 0.5f + 12.0f),
                row_y + static_cast<float>(index / 2) * 40.0f,
                (row_width - 12.0f) * 0.5f,
                32.0f
            };
            if (drawButton(button, labels[static_cast<std::size_t>(index)], vehicle.geometry.nose_cone_shape == nose_shapes[static_cast<std::size_t>(index)], Color {61, 135, 245, 228}, vehicle.geometry.nose_cone_shape == nose_shapes[static_cast<std::size_t>(index)] ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
                vehicle.geometry.nose_cone_shape = nose_shapes[static_cast<std::size_t>(index)];
                changed = true;
            }
        }
        break;
    }
    case ComponentSelection::BodyTube: {
        apply_step(
            vehicle.geometry.body_length_m,
            0.05,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Body Length",
                std::format("{:.2f} m", vehicle.geometry.body_length_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.body_diameter_m,
            0.005,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Diameter",
                std::format("{:.3f} m", vehicle.geometry.body_diameter_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.wall_thickness_m,
            0.0005,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Wall Thickness",
                std::format("{:.4f} m", vehicle.geometry.wall_thickness_m)));
        row_y += row_height;
        apply_material("Material", vehicle.geometry.body_material);
        apply_step(
            vehicle.geometry.body_controls.fore_radius_scale,
            0.03,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Fore Radius",
                std::format("{:.2f}", vehicle.geometry.body_controls.fore_radius_scale)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.body_controls.mid_radius_scale,
            0.03,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Mid Radius",
                std::format("{:.2f}", vehicle.geometry.body_controls.mid_radius_scale)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.body_controls.aft_radius_scale,
            0.03,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Aft Radius",
                std::format("{:.2f}", vehicle.geometry.body_controls.aft_radius_scale)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.structure_cg_from_nose_m,
            0.02,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Structure CG",
                std::format("{:.2f} m", vehicle.geometry.structure_cg_from_nose_m)));
        break;
    }
    case ComponentSelection::Transition: {
        apply_step(
            vehicle.geometry.transition_length_m,
            0.01,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Length",
                std::format("{:.2f} m", vehicle.geometry.transition_length_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.transition_aft_diameter_m,
            0.004,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Aft Diameter",
                std::format("{:.3f} m", vehicle.geometry.transition_aft_diameter_m)));
        row_y += row_height;
        apply_material("Material", vehicle.geometry.transition_material);
        apply_step(
            vehicle.geometry.transition_controls.mid_radius_scale,
            0.04,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Mid Radius",
                std::format("{:.2f}", vehicle.geometry.transition_controls.mid_radius_scale)));
        row_y += row_height;

        constexpr std::array<rocket::TransitionShape, 2> transition_shapes {
            rocket::TransitionShape::Conical,
            rocket::TransitionShape::Curved
        };
        for (int index = 0; index < 2; ++index) {
            const auto shape = transition_shapes[static_cast<std::size_t>(index)];
            const ::Rectangle button {
                bounds.x + 16.0f + static_cast<float>(index) * ((row_width - 10.0f) * 0.5f + 10.0f),
                row_y,
                (row_width - 10.0f) * 0.5f,
                32.0f
            };
            if (drawButton(button, std::string(rocket::transitionShapeLabel(shape)), vehicle.geometry.transition_shape == shape, Color {61, 135, 245, 228}, vehicle.geometry.transition_shape == shape ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
                vehicle.geometry.transition_shape = shape;
                changed = true;
            }
        }
        break;
    }
    case ComponentSelection::FinSet: {
        apply_step(
            vehicle.geometry.fin_front_from_nose_m,
            0.03,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Front Station",
                std::format("{:.2f} m", vehicle.geometry.fin_front_from_nose_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.fin_root_chord_m,
            0.02,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Root Chord",
                std::format("{:.2f} m", vehicle.geometry.fin_root_chord_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.fin_tip_chord_m,
            0.02,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Tip Chord",
                std::format("{:.2f} m", vehicle.geometry.fin_tip_chord_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.fin_span_m,
            0.01,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Span",
                std::format("{:.2f} m", vehicle.geometry.fin_span_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.fin_sweep_length_m,
            0.01,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Sweep",
                std::format("{:.2f} m", vehicle.geometry.fin_sweep_length_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.fin_thickness_m,
            0.0005,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Thickness",
                std::format("{:.4f} m", vehicle.geometry.fin_thickness_m)));
        row_y += row_height;

        int fin_count_delta = drawStepper(
            Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
            "Fin Count",
            std::format("{}", vehicle.geometry.fin_count));
        if (fin_count_delta != 0) {
            vehicle.geometry.fin_count += fin_count_delta;
            changed = true;
        }
        row_y += row_height;
        apply_material("Material", vehicle.geometry.fin_material);
        apply_step(
            vehicle.geometry.fin_controls.tip_le_offset_m,
            0.01,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Tip LE",
                std::format("{:.2f} m", vehicle.geometry.fin_controls.tip_le_offset_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.fin_controls.tip_te_offset_m,
            0.01,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Tip TE",
                std::format("{:.2f} m", vehicle.geometry.fin_controls.tip_te_offset_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.fin_controls.span_scale,
            0.05,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Span Scale",
                std::format("{:.2f}", vehicle.geometry.fin_controls.span_scale)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.fin_controls.thickness_scale,
            0.05,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Ctrl Thickness",
                std::format("{:.2f}", vehicle.geometry.fin_controls.thickness_scale)));
        row_y += row_height + 8.0f;

        constexpr std::array<rocket::FinShape, 3> fin_shapes {
            rocket::FinShape::Trapezoidal,
            rocket::FinShape::Elliptical,
            rocket::FinShape::Airfoil
        };
        constexpr std::array<const char*, 3> labels {"Trapezoidal", "Elliptical", "Airfoil"};
        for (int index = 0; index < 3; ++index) {
            const ::Rectangle button {bounds.x + 16.0f, row_y + static_cast<float>(index) * 40.0f, row_width, 32.0f};
            if (drawButton(button, labels[static_cast<std::size_t>(index)], vehicle.geometry.fin_shape == fin_shapes[static_cast<std::size_t>(index)], Color {61, 135, 245, 228}, vehicle.geometry.fin_shape == fin_shapes[static_cast<std::size_t>(index)] ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
                vehicle.geometry.fin_shape = fin_shapes[static_cast<std::size_t>(index)];
                changed = true;
            }
        }
        break;
    }
    case ComponentSelection::MotorMount: {
        apply_step(
            vehicle.geometry.propellant_cg_from_nose_m,
            0.02,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Propellant CG",
                std::format("{:.2f} m", vehicle.geometry.propellant_cg_from_nose_m)));
        row_y += row_height;
        int motor_count_delta = drawStepper(
            Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
            "Motor Count",
            std::format("{}", motor_editor.motor_count));
        if (motor_count_delta != 0) {
            motor_editor.motor_count += motor_count_delta;
            changed = true;
        }
        row_y += row_height;
        apply_step(
            motor_editor.max_thrust_n,
            10.0,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Thrust Each",
                std::format("{:.0f} N", motor_editor.max_thrust_n)));
        row_y += row_height;
        apply_step(
            motor_editor.burn_time_s,
            0.1,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Burn Time",
                std::format("{:.1f} s", motor_editor.burn_time_s)));
        row_y += row_height;
        apply_step(
            motor_editor.propellant_mass_kg,
            0.02,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Prop Mass Each",
                std::format("{:.2f} kg", motor_editor.propellant_mass_kg)));
        row_y += row_height;
        apply_step(
            motor_editor.mount_radius_m,
            0.004,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Cluster Radius",
                std::format("{:.3f} m", motor_editor.mount_radius_m)));
        row_y += row_height;
        apply_step(
            motor_editor.cant_angle_deg,
            0.5,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Cant Angle",
                std::format("{:.1f} deg", motor_editor.cant_angle_deg)));
        row_y += row_height;
        break;
    }
    case ComponentSelection::Payload: {
        apply_step(
            vehicle.geometry.payload_length_m,
            0.01,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Payload Length",
                std::format("{:.2f} m", vehicle.geometry.payload_length_m)));
        row_y += row_height;
        apply_step(
            vehicle.geometry.payload_mass_kg,
            0.05,
            drawStepper(
                Rectangle {bounds.x + 16.0f, row_y, row_width, 30.0f},
                "Payload Mass",
                std::format("{:.2f} kg", vehicle.geometry.payload_mass_kg)));
        row_y += row_height;
        apply_material("Material", vehicle.geometry.payload_material);
        break;
    }
    }

    if (changed) {
        motor_editor.motor_count = std::clamp(motor_editor.motor_count, 1, 6);
        motor_editor.max_thrust_n = rocket::secure::clampFinite(motor_editor.max_thrust_n, 180.0, 20.0, 2500.0);
        motor_editor.burn_time_s = rocket::secure::clampFinite(motor_editor.burn_time_s, 2.4, 0.4, 20.0);
        motor_editor.propellant_mass_kg = rocket::secure::clampFinite(motor_editor.propellant_mass_kg, 0.24, 0.02, 8.0);
        motor_editor.mount_radius_m = rocket::secure::clampFinite(motor_editor.mount_radius_m, 0.04, 0.0, vehicle.geometry.body_diameter_m * 0.42);
        motor_editor.cant_angle_deg = rocket::secure::clampFinite(motor_editor.cant_angle_deg, 0.0, 0.0, 12.0);
        if (app_state.selection == ComponentSelection::MotorMount) {
            rebuildMotorCluster(vehicle, motor_editor, mesh_generator, runtime);
        } else {
            rebuildVehicle(vehicle, mesh_generator, runtime);
        }
    }

    // Sezione di editing per i vertici liberi del componente selezionato
    drawSectionCaption(
        Rectangle {bounds.x + 16.0f, row_y + 24.0f, bounds.width - 32.0f, 30.0f},
        "Vertici Liberi",
        "Modifica la forma dei singoli punti della mesh senza perdere la geometria procedurale.");
    row_y += 56.0f;

    auto* active_modifiers = vehicle.geometry.getActiveComponentModifiers(
        static_cast<rocket::ComponentType>(app_state.selection));

    if (active_modifiers != nullptr) {
        const auto component_type = static_cast<rocket::ComponentType>(app_state.selection);
        const auto* topology = mesh_generator.componentMesh(component_type);
        const auto* edges = mesh_generator.componentEdges(component_type);
        const auto* faces = mesh_generator.componentFaces(component_type);
        if (topology != nullptr) {
            DrawText(
                std::format(
                    "Mesh editabile: {} vertici  |  {} edge  |  {} facce",
                    topology->vertices.size(),
                    edges != nullptr ? edges->size() : 0,
                    faces != nullptr ? faces->size() : topology->indices.size() / 3)
                    .c_str(),
                static_cast<int>(bounds.x) + 18,
                static_cast<int>(row_y),
                13,
                Color {148, 163, 184, 255});
            row_y += 22.0f;
        }

        const ::Rectangle vertex_mod_toggle {
            bounds.x + 16.0f, row_y, bounds.width - 32.0f, 30.0f};
        if (drawButton(
                vertex_mod_toggle,
                active_modifiers->is_active ? "Editing Abilitato" : "Editing Disabilitato",
                active_modifiers->is_active,
                Color {99, 102, 241, 220},
                active_modifiers->is_active ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
            active_modifiers->is_active = !active_modifiers->is_active;
            if (active_modifiers->is_active) {
                mesh_generator.rebuild(vehicle.geometry, vehicle.cluster);
                syncActiveMeshVertices(app_state, vehicle, mesh_generator);
            }
        }
        row_y += 36.0f;

        if (active_modifiers->is_active) {
            DrawText(
                std::format(
                    "Modo mesh: {}  |  Vertici disponibili: {}",
                    meshSelectionModeLabel(app_state.mesh_selection_mode),
                    active_modifiers->modified_vertices.size())
                    .c_str(),
                static_cast<int>(bounds.x) + 18,
                static_cast<int>(row_y),
                14,
                Color {148, 163, 184, 255});
            row_y += 24.0f;

            const float mode_width = (bounds.width - 44.0f) / 3.0f;
            constexpr std::array<MeshSelectionMode, 3> selection_modes {
                MeshSelectionMode::Vertex,
                MeshSelectionMode::Edge,
                MeshSelectionMode::Face
            };
            for (int index = 0; index < 3; ++index) {
                const auto mode = selection_modes[static_cast<std::size_t>(index)];
                const ::Rectangle button {
                    bounds.x + 16.0f + static_cast<float>(index) * (mode_width + 6.0f),
                    row_y,
                    mode_width,
                    28.0f
                };
                if (drawButton(
                        button,
                        meshSelectionModeLabel(mode),
                        app_state.mesh_selection_mode == mode,
                        Color {56, 189, 248, 220},
                        app_state.mesh_selection_mode == mode ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
                    app_state.mesh_selection_mode = mode;
                    clearMeshSelection(app_state);
                }
            }
            row_y += 36.0f;

            const ::Rectangle reset_button {
                bounds.x + 16.0f, row_y, bounds.width - 32.0f, 28.0f};
            if (drawButton(reset_button, "Resetta Vertici", false, Color {239, 68, 68, 220}, ButtonStyle::Outlined)) {
                clearComponentEdits(vehicle, component_type);
                mesh_generator.rebuild(vehicle.geometry, vehicle.cluster);
                syncActiveMeshVertices(app_state, vehicle, mesh_generator);
            }
            row_y += 36.0f;

            if (app_state.mesh_selection_mode == MeshSelectionMode::Face) {
                DrawText(
                    std::format("Face selezionata: {}", app_state.selected_face_id >= 0 ? std::to_string(app_state.selected_face_id) : std::string("nessuna")).c_str(),
                    static_cast<int>(bounds.x) + 18,
                    static_cast<int>(row_y),
                    12,
                    Color {203, 213, 225, 255});
                row_y += 22.0f;

                const int extrude_delta = drawStepper(
                    Rectangle {bounds.x + 16.0f, row_y, bounds.width - 32.0f, 28.0f},
                    "Extrude Dist",
                    std::format("{:.3f} m", app_state.mesh_extrude_distance_m));
                if (extrude_delta != 0) {
                    app_state.mesh_extrude_distance_m =
                        std::clamp(app_state.mesh_extrude_distance_m + 0.005 * static_cast<double>(extrude_delta), 0.005, 0.20);
                }
                row_y += 34.0f;

                const int bevel_ratio_delta = drawStepper(
                    Rectangle {bounds.x + 16.0f, row_y, bounds.width - 32.0f, 28.0f},
                    "Bevel Ratio",
                    std::format("{:.2f}", app_state.mesh_bevel_ratio));
                if (bevel_ratio_delta != 0) {
                    app_state.mesh_bevel_ratio =
                        std::clamp(app_state.mesh_bevel_ratio + 0.04 * static_cast<double>(bevel_ratio_delta), 0.05, 0.82);
                }
                row_y += 34.0f;

                const int bevel_offset_delta = drawStepper(
                    Rectangle {bounds.x + 16.0f, row_y, bounds.width - 32.0f, 28.0f},
                    "Bevel Offset",
                    std::format("{:.3f} m", app_state.mesh_bevel_offset_m));
                if (bevel_offset_delta != 0) {
                    app_state.mesh_bevel_offset_m =
                        std::clamp(app_state.mesh_bevel_offset_m + 0.002 * static_cast<double>(bevel_offset_delta), -0.03, 0.08);
                }
                row_y += 34.0f;

                const float op_width = (bounds.width - 42.0f) * 0.5f;
                if (drawButton(
                        Rectangle {bounds.x + 16.0f, row_y, op_width, 28.0f},
                        "Extrude Face",
                        false,
                        Color {251, 146, 60, 220},
                        ButtonStyle::Outlined)) {
                    applyTopologyOperation(app_state, vehicle, mesh_generator, "extrude");
                }
                if (drawButton(
                        Rectangle {bounds.x + 26.0f + op_width, row_y, op_width, 28.0f},
                        "Bevel Face",
                        false,
                        Color {245, 158, 11, 220},
                        ButtonStyle::Outlined)) {
                    applyTopologyOperation(app_state, vehicle, mesh_generator, "bevel");
                }
                row_y += 36.0f;
            } else if (app_state.mesh_selection_mode == MeshSelectionMode::Edge) {
                DrawText(
                    std::format("Edge selezionato: {}", app_state.selected_edge_id >= 0 ? std::to_string(app_state.selected_edge_id) : std::string("nessuno")).c_str(),
                    static_cast<int>(bounds.x) + 18,
                    static_cast<int>(row_y),
                    12,
                    Color {203, 213, 225, 255});
                row_y += 22.0f;
                if (drawButton(
                        Rectangle {bounds.x + 16.0f, row_y, bounds.width - 32.0f, 28.0f},
                        "Loop Cut Edge",
                        false,
                        Color {56, 189, 248, 220},
                        ButtonStyle::Outlined)) {
                    applyTopologyOperation(app_state, vehicle, mesh_generator, "loop_cut");
                }
                row_y += 36.0f;
            } else {
                for (size_t i = 0; i < active_modifiers->modified_vertices.size() && i < 5; ++i) {
                    const auto& vertex = active_modifiers->modified_vertices[i];
                    DrawText(
                        std::format(
                            "V#{}: offset ({:.3f}, {:.3f}, {:.3f}) m",
                            vertex.vertex_id,
                            vertex.offset_m.x,
                            vertex.offset_m.y,
                            vertex.offset_m.z)
                            .c_str(),
                        static_cast<int>(bounds.x) + 24,
                        static_cast<int>(row_y),
                        11,
                        Color {203, 213, 225, 255});
                    row_y += 20.0f;
                }

                if (active_modifiers->modified_vertices.size() > 5) {
                    DrawText(
                        std::format("... e {} altri", active_modifiers->modified_vertices.size() - 5).c_str(),
                        static_cast<int>(bounds.x) + 24,
                        static_cast<int>(row_y),
                        11,
                        Color {100, 116, 139, 255});
                    row_y += 20.0f;
                }
            }

            drawInlineHint(
                Rectangle {bounds.x + 18.0f, row_y + 8.0f, bounds.width - 36.0f, 44.0f},
                app_state.mesh_selection_mode == MeshSelectionMode::Vertex
                    ? "Seleziona i vertici in viewport e usa [2] Move per deformare la mesh direttamente."
                    : (app_state.mesh_selection_mode == MeshSelectionMode::Edge
                           ? "Clicca un edge marker nel wireframe e applica un loop cut locale per aumentare il dettaglio."
                           : "Clicca una faccia della mesh per estruderla o smussarla con bevel direttamente dalla viewport."),
                Color {34, 197, 94, 220});
        }
    }
}

void drawModelingStatusBar(
    const ::Rectangle& bounds,
    const AppState& app_state,
    const rocket::SimulationSnapshot& snapshot,
    const rocket::VehicleModel& vehicle) {
    drawPanel(bounds, "Riepilogo Progetto", Color {10, 16, 28, 235});
    const auto breakdown = rocket::estimateStructureMassBreakdown(vehicle.geometry);
    const auto structural = rocket::estimateStructuralMaterialAssessment(vehicle.geometry);
    drawKeyValueLine(Rectangle {bounds.x + 18.0f, bounds.y + 50.0f, 220.0f, 20.0f}, "Preset", std::string(rocket::rocketPresetLabel(app_state.active_preset)));
    drawKeyValueLine(Rectangle {bounds.x + 18.0f, bounds.y + 74.0f, 220.0f, 20.0f}, "Stabilita", std::format("{:.2f} cal", snapshot.static_margin_calibers));
    drawKeyValueLine(Rectangle {bounds.x + 268.0f, bounds.y + 50.0f, 220.0f, 20.0f}, "Massa", std::format("{:.2f} kg", snapshot.state.mass_kg));
    drawKeyValueLine(Rectangle {bounds.x + 268.0f, bounds.y + 74.0f, 220.0f, 20.0f}, "Struttura", std::format("{:.2f} kg", breakdown.total_mass_kg));
    drawKeyValueLine(Rectangle {bounds.x + 508.0f, bounds.y + 50.0f, 260.0f, 20.0f}, "CG / CP", std::format("{:.2f} / {:.2f} m", snapshot.cg_from_nose_m, snapshot.cp_from_nose_m));
    drawKeyValueLine(Rectangle {bounds.x + 508.0f, bounds.y + 74.0f, 260.0f, 20.0f}, "q consigliata", std::format("{:.0f} kPa", structural.recommended_max_dynamic_pressure_pa / 1000.0));
    drawKeyValueLine(Rectangle {bounds.x + 786.0f, bounds.y + 50.0f, 240.0f, 20.0f}, "E equivalente", std::format("{:.1f} GPa", structural.equivalent_modulus_gpa));
    drawKeyValueLine(Rectangle {bounds.x + 786.0f, bounds.y + 74.0f, 240.0f, 20.0f}, "Densita eq.", std::format("{:.0f} kg/m3", structural.equivalent_density_kg_per_m3));
    drawSingleLineClippedText(
        Rectangle {bounds.x + 1048.0f, bounds.y + 62.0f, bounds.width - 1062.0f, 14.0f},
        std::format(
            "Tool {} {}  |  Handle {}  |  Mesh {}  |  {}",
            modelingToolLabel(app_state.modeling_tool),
            modelingToolShortcut(app_state.modeling_tool),
            handleKindLabel(app_state.active_handle),
            meshSelectionModeLabel(app_state.mesh_selection_mode),
            app_state.snap_to_grid
                ? "Snap attivo: scolpisci con gli handle e usa gli stepper per rifinire."
                : "Snap libero: scolpisci in viewport e riallinea se serve precisione."),
        12,
        Color {148, 163, 184, 255});
}
