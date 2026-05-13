ImVec4 toImVec4(Color color) {
    return ImVec4(
        static_cast<float>(color.r) / 255.0f,
        static_cast<float>(color.g) / 255.0f,
        static_cast<float>(color.b) / 255.0f,
        static_cast<float>(color.a) / 255.0f);
}

void applyDarkSpaceTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.ChildRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.WindowPadding = ImVec2(14.0f, 12.0f);
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.95f, 0.99f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.49f, 0.56f, 0.66f, 1.0f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.07f, 0.12f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.10f, 0.16f, 0.78f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.09f, 0.14f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.23f, 0.31f, 0.44f, 0.85f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.13f, 0.20f, 0.98f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.30f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.25f, 0.38f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.08f, 0.13f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.11f, 0.17f, 1.0f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.05f, 0.09f, 0.14f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.03f, 0.05f, 0.08f, 0.75f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.31f, 0.44f, 0.9f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.31f, 0.43f, 0.62f, 0.95f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.56f, 0.80f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.57f, 0.83f, 1.0f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.44f, 0.67f, 0.98f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.69f, 0.85f, 1.0f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.10f, 0.18f, 0.29f, 0.96f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.17f, 0.30f, 0.46f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.44f, 0.65f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.10f, 0.18f, 0.28f, 0.92f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.29f, 0.44f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.40f, 0.59f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.33f, 0.47f, 0.86f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.33f, 0.46f, 0.65f, 0.55f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.42f, 0.60f, 0.86f, 0.90f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.56f, 0.76f, 1.0f, 1.0f);
    colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.13f, 0.20f, 0.95f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.28f, 0.43f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.14f, 0.23f, 0.35f, 1.0f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.18f, 0.30f, 0.46f, 1.0f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.53f, 0.79f, 1.0f, 1.0f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.98f, 0.64f, 0.22f, 1.0f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.49f, 0.86f, 0.62f, 1.0f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.98f, 0.78f, 0.28f, 1.0f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.49f, 0.77f, 0.35f);
}

bool dragDouble(const char* label, double& value, double speed, double min_value, double max_value, const char* format) {
    float working_value = static_cast<float>(value);
    const bool changed = ImGui::DragFloat(label, &working_value, static_cast<float>(speed), static_cast<float>(min_value), static_cast<float>(max_value), format);
    if (changed) {
        value = std::clamp(static_cast<double>(working_value), min_value, max_value);
    }
    return changed;
}

bool dragInt(const char* label, int& value, float speed, int min_value, int max_value) {
    int working_value = value;
    const bool changed = ImGui::DragInt(label, &working_value, speed, min_value, max_value);
    if (changed) {
        value = std::clamp(working_value, min_value, max_value);
    }
    return changed;
}

bool comboComponentMaterial(const char* label, rocket::ComponentMaterial& material) {
    const auto& materials = rocket::availableComponentMaterials();
    int current_index = 0;
    for (int index = 0; index < static_cast<int>(materials.size()); ++index) {
        if (materials[static_cast<std::size_t>(index)] == material) {
            current_index = index;
            break;
        }
    }

    if (!ImGui::BeginCombo(label, std::string(rocket::materialDefinition(material).label).c_str())) {
        return false;
    }

    bool changed = false;
    for (int index = 0; index < static_cast<int>(materials.size()); ++index) {
        const auto current = materials[static_cast<std::size_t>(index)];
        const bool selected = current == material;
        if (ImGui::Selectable(std::string(rocket::materialDefinition(current).label).c_str(), selected)) {
            material = current;
            changed = true;
        }
        if (selected) {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndCombo();
    return changed;
}

template <typename EnumT, std::size_t N>
bool comboEnum(
    const char* label,
    EnumT& current_value,
    const std::array<EnumT, N>& values,
    const std::array<const char*, N>& labels) {
    std::size_t selected_index = 0;
    for (std::size_t index = 0; index < N; ++index) {
        if (values[index] == current_value) {
            selected_index = index;
            break;
        }
    }

    if (!ImGui::BeginCombo(label, labels[selected_index])) {
        return false;
    }

    bool changed = false;
    for (std::size_t index = 0; index < N; ++index) {
        const bool selected = values[index] == current_value;
        if (ImGui::Selectable(labels[index], selected)) {
            current_value = values[index];
            changed = true;
        }
        if (selected) {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndCombo();
    return changed;
}

void setWindowRect(const ::Rectangle& bounds) {
    ImGui::SetNextWindowPos(ImVec2(bounds.x, bounds.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bounds.width, bounds.height), ImGuiCond_Always);
}

void primeWindowRect(const ::Rectangle& bounds) {
    ImGui::SetNextWindowPos(ImVec2(bounds.x, bounds.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(bounds.width, bounds.height), ImGuiCond_FirstUseEver);
}

::Rectangle syncWindowRect(FloatingWindowState& state) {
    const ImVec2 position = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();
    state.bounds = ::Rectangle {position.x, position.y, size.x, size.y};

    const ImVec2 min_region = ImGui::GetWindowContentRegionMin();
    const ImVec2 max_region = ImGui::GetWindowContentRegionMax();
    return ::Rectangle {
        position.x + min_region.x,
        position.y + min_region.y,
        std::max(0.0f, max_region.x - min_region.x),
        std::max(0.0f, max_region.y - min_region.y)
    };
}

void drawTransientStatusInline(const AppState& app_state) {
    const bool has_transient_status =
        !app_state.transient_status_message.empty() &&
        GetTime() <= app_state.transient_status_expire_time_s;
    if (has_transient_status) {
        ImGui::TextColored(ImVec4(0.74f, 0.88f, 1.0f, 1.0f), "%s", app_state.transient_status_message.c_str());
    } else {
        ImGui::TextDisabled("Ctrl+S Save  |  Ctrl+Shift+S Save As  |  Ctrl+O Load  |  Ctrl+E Export");
    }
}

void drawMetricLabelValue(const char* label, const std::string& value, const ImVec4& accent) {
    ImGui::BeginChild(label, ImVec2(0.0f, 54.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextColored(accent, "%s", label);
    ImGui::Text("%s", value.c_str());
    ImGui::EndChild();
}

void drawStatCardImGui(const char* label, const std::string& value, const ImVec4& accent, float height = 58.0f) {
    ImGui::BeginChild(label, ImVec2(0.0f, height), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextColored(accent, "%s", label);
    ImGui::Spacing();
    ImGui::Text("%s", value.c_str());
    ImGui::EndChild();
}

void drawKeyValueTableImGui(const char* id, std::initializer_list<std::pair<const char*, std::string>> rows) {
    if (ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV)) {
        for (const auto& [label, value] : rows) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("%s", label);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", value.c_str());
        }
        ImGui::EndTable();
    }
}

void applyComponentQuickPreset(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    MotorEditorState& motor_editor,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime,
    int preset_index) {
    switch (app_state.selection) {
    case ComponentSelection::NoseCone:
        switch (preset_index) {
        case 0:
            vehicle.geometry.nose_length_m = 0.28;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::Conical;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 0.88, .shoulder_radius_scale = 1.0};
            break;
        case 1:
            vehicle.geometry.nose_length_m = 0.42;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::Conical;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 0.82, .shoulder_radius_scale = 0.98};
            break;
        case 2:
            vehicle.geometry.nose_length_m = 0.52;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::LdHaack;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 0.94, .shoulder_radius_scale = 1.02};
            break;
        case 3:
            vehicle.geometry.nose_length_m = 0.44;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::TangentOgive;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 1.02, .shoulder_radius_scale = 1.0};
            break;
        case 4:
            vehicle.geometry.nose_length_m = 0.38;
            vehicle.geometry.nose_cone_shape = rocket::NoseConeShape::Parabolic;
            vehicle.geometry.nose_controls = {.mid_radius_scale = 1.08, .shoulder_radius_scale = 1.03};
            break;
        default:
            break;
        }
        rebuildVehicle(vehicle, mesh_generator, runtime);
        break;
    case ComponentSelection::BodyTube:
        switch (preset_index) {
        case 0:
            vehicle.geometry.body_length_m = 3.2;
            vehicle.geometry.body_diameter_m = 0.092;
            vehicle.geometry.wall_thickness_m = 0.0023;
            vehicle.geometry.body_controls = {.fore_radius_scale = 0.98, .mid_radius_scale = 1.0, .aft_radius_scale = 0.98};
            break;
        case 1:
            vehicle.geometry.body_length_m = 2.4;
            vehicle.geometry.body_diameter_m = 0.104;
            vehicle.geometry.wall_thickness_m = 0.0030;
            vehicle.geometry.body_controls = {.fore_radius_scale = 1.0, .mid_radius_scale = 1.02, .aft_radius_scale = 1.0};
            break;
        case 2:
            vehicle.geometry.body_length_m = 3.8;
            vehicle.geometry.body_diameter_m = 0.110;
            vehicle.geometry.wall_thickness_m = 0.0032;
            vehicle.geometry.body_controls = {.fore_radius_scale = 0.97, .mid_radius_scale = 1.0, .aft_radius_scale = 0.99};
            break;
        case 3:
            vehicle.geometry.body_length_m = 2.9;
            vehicle.geometry.body_diameter_m = 0.125;
            vehicle.geometry.wall_thickness_m = 0.0038;
            vehicle.geometry.body_controls = {.fore_radius_scale = 1.0, .mid_radius_scale = 1.02, .aft_radius_scale = 0.98};
            break;
        case 4:
            vehicle.geometry.body_length_m = 3.3;
            vehicle.geometry.body_diameter_m = 0.140;
            vehicle.geometry.wall_thickness_m = 0.0040;
            vehicle.geometry.body_controls = {.fore_radius_scale = 1.0, .mid_radius_scale = 1.04, .aft_radius_scale = 1.0};
            break;
        default:
            break;
        }
        rebuildVehicle(vehicle, mesh_generator, runtime);
        break;
    case ComponentSelection::Transition:
        switch (preset_index) {
        case 0:
            vehicle.geometry.transition_length_m = 0.08;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.94, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Conical;
            vehicle.geometry.transition_controls.mid_radius_scale = 1.0;
            break;
        case 1:
            vehicle.geometry.transition_length_m = 0.22;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.72, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Curved;
            vehicle.geometry.transition_controls.mid_radius_scale = 0.86;
            break;
        case 2:
            vehicle.geometry.transition_length_m = 0.16;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.82, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Conical;
            vehicle.geometry.transition_controls.mid_radius_scale = 1.0;
            break;
        case 3:
            vehicle.geometry.transition_length_m = 0.24;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.64, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Curved;
            vehicle.geometry.transition_controls.mid_radius_scale = 0.78;
            break;
        case 4:
            vehicle.geometry.transition_length_m = 0.18;
            vehicle.geometry.transition_aft_diameter_m = std::max(vehicle.geometry.body_diameter_m * 0.90, 0.05);
            vehicle.geometry.transition_shape = rocket::TransitionShape::Curved;
            vehicle.geometry.transition_controls.mid_radius_scale = 1.08;
            break;
        default:
            break;
        }
        rebuildVehicle(vehicle, mesh_generator, runtime);
        break;
    case ComponentSelection::FinSet:
        switch (preset_index) {
        case 0:
            vehicle.geometry.fin_shape = rocket::FinShape::Trapezoidal;
            vehicle.geometry.fin_root_chord_m = 0.26;
            vehicle.geometry.fin_tip_chord_m = 0.13;
            vehicle.geometry.fin_span_m = 0.14;
            vehicle.geometry.fin_sweep_length_m = 0.06;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = 0.0, .tip_te_offset_m = 0.0, .span_scale = 1.0, .thickness_scale = 1.0};
            break;
        case 1:
            vehicle.geometry.fin_shape = rocket::FinShape::Airfoil;
            vehicle.geometry.fin_root_chord_m = 0.24;
            vehicle.geometry.fin_tip_chord_m = 0.09;
            vehicle.geometry.fin_span_m = 0.14;
            vehicle.geometry.fin_sweep_length_m = 0.12;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = 0.03, .tip_te_offset_m = -0.01, .span_scale = 0.96, .thickness_scale = 0.88};
            break;
        case 2:
            vehicle.geometry.fin_shape = rocket::FinShape::Trapezoidal;
            vehicle.geometry.fin_root_chord_m = 0.30;
            vehicle.geometry.fin_tip_chord_m = 0.07;
            vehicle.geometry.fin_span_m = 0.13;
            vehicle.geometry.fin_sweep_length_m = 0.15;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = 0.02, .tip_te_offset_m = -0.02, .span_scale = 0.95, .thickness_scale = 0.9};
            break;
        case 3:
            vehicle.geometry.fin_shape = rocket::FinShape::Elliptical;
            vehicle.geometry.fin_root_chord_m = 0.34;
            vehicle.geometry.fin_tip_chord_m = 0.17;
            vehicle.geometry.fin_span_m = 0.20;
            vehicle.geometry.fin_sweep_length_m = 0.05;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = 0.0, .tip_te_offset_m = 0.02, .span_scale = 1.04, .thickness_scale = 1.05};
            break;
        case 4:
            vehicle.geometry.fin_shape = rocket::FinShape::Airfoil;
            vehicle.geometry.fin_root_chord_m = 0.38;
            vehicle.geometry.fin_tip_chord_m = 0.18;
            vehicle.geometry.fin_span_m = 0.24;
            vehicle.geometry.fin_sweep_length_m = 0.08;
            vehicle.geometry.fin_controls = {.tip_le_offset_m = -0.01, .tip_te_offset_m = 0.03, .span_scale = 1.10, .thickness_scale = 1.12};
            break;
        default:
            break;
        }
        rebuildVehicle(vehicle, mesh_generator, runtime);
        break;
    case ComponentSelection::MotorMount:
        switch (preset_index) {
        case 0:
            motor_editor.motor_count = 1;
            motor_editor.mount_radius_m = 0.0;
            motor_editor.max_thrust_n = 420.0;
            motor_editor.burn_time_s = 2.8;
            motor_editor.propellant_mass_kg = 0.54;
            motor_editor.cant_angle_deg = 0.0;
            break;
        case 1:
            motor_editor.motor_count = 2;
            motor_editor.mount_radius_m = 0.026;
            motor_editor.max_thrust_n = 210.0;
            motor_editor.burn_time_s = 2.2;
            motor_editor.propellant_mass_kg = 0.20;
            motor_editor.cant_angle_deg = 0.0;
            break;
        case 2:
            motor_editor.motor_count = 3;
            motor_editor.mount_radius_m = 0.032;
            motor_editor.max_thrust_n = 220.0;
            motor_editor.burn_time_s = 2.1;
            motor_editor.propellant_mass_kg = 0.22;
            motor_editor.cant_angle_deg = 0.0;
            break;
        case 3:
            motor_editor.motor_count = 4;
            motor_editor.mount_radius_m = 0.038;
            motor_editor.max_thrust_n = 240.0;
            motor_editor.burn_time_s = 2.5;
            motor_editor.propellant_mass_kg = 0.26;
            motor_editor.cant_angle_deg = 0.0;
            break;
        case 4:
            motor_editor.motor_count = 5;
            motor_editor.mount_radius_m = 0.040;
            motor_editor.max_thrust_n = 205.0;
            motor_editor.burn_time_s = 2.9;
            motor_editor.propellant_mass_kg = 0.23;
            motor_editor.cant_angle_deg = 1.5;
            break;
        default:
            break;
        }
        rebuildMotorCluster(vehicle, motor_editor, mesh_generator, runtime);
        break;
    case ComponentSelection::Payload:
        switch (preset_index) {
        case 0:
            vehicle.geometry.payload_length_m = 0.14;
            vehicle.geometry.payload_mass_kg = 0.45;
            break;
        case 1:
            vehicle.geometry.payload_length_m = 0.26;
            vehicle.geometry.payload_mass_kg = 1.15;
            break;
        case 2:
            vehicle.geometry.payload_length_m = 0.22;
            vehicle.geometry.payload_mass_kg = 0.72;
            break;
        case 3:
            vehicle.geometry.payload_length_m = 0.30;
            vehicle.geometry.payload_mass_kg = 1.35;
            break;
        case 4:
            vehicle.geometry.payload_length_m = 0.28;
            vehicle.geometry.payload_mass_kg = 1.85;
            break;
        default:
            break;
        }
        rebuildVehicle(vehicle, mesh_generator, runtime);
        break;
    }
}

const char* quickPresetLabel(ComponentSelection selection, int index) {
    static constexpr std::array<const char*, 5> nose {
        "Conical Sprint", "Conical Long Range", "Haack Low Drag", "Ogive Stable", "Parabolic Payload"
    };
    static constexpr std::array<const char*, 5> body {
        "Minimum Diameter", "Avionics Bay", "Long Burner", "Heavy Structure", "Wide Lift Core"
    };
    static constexpr std::array<const char*, 5> transition {
        "Straight Interstage", "Boat Tail", "Conical Coupler", "Aggressive Tail", "Payload Shoulder"
    };
    static constexpr std::array<const char*, 5> fins {
        "Sport Trapezoid", "Supersonic Airfoil", "Clipped Delta", "Stable Elliptical", "Heavy Lift Canards"
    };
    static constexpr std::array<const char*, 5> motors {
        "Single Sustainer", "Twin Booster", "3-Motor Cluster", "4-Motor Cluster", "5-Motor Ring"
    };
    static constexpr std::array<const char*, 5> payload {
        "Compact Payload", "Avionics + Camera", "Dual Deploy Bay", "Science Probe", "Heavy Recovery Stack"
    };

    const auto& labels =
        selection == ComponentSelection::NoseCone ? nose :
        selection == ComponentSelection::BodyTube ? body :
        selection == ComponentSelection::Transition ? transition :
        selection == ComponentSelection::FinSet ? fins :
        selection == ComponentSelection::MotorMount ? motors :
        payload;
    return labels[static_cast<std::size_t>(index)];
}

void drawTopBarImGui(AppState& app_state, SimulationRuntime& runtime) {
    const float width = static_cast<float>(GetScreenWidth());
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, 82.0f), ImGuiCond_Always);
    ImGui::Begin(
        "##topbar",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::Columns(3, nullptr, false);
    ImGui::SetColumnWidth(0, 340.0f);
    ImGui::TextColored(ImVec4(0.82f, 0.91f, 1.0f, 1.0f), "The Rocket Lab");
    ImGui::TextDisabled("Dear ImGui Dark Space shell on top of the active raylib workspace");
    drawTransientStatusInline(app_state);

    ImGui::NextColumn();
    if (ImGui::Button("Modelazione [F1]", ImVec2(160.0f, 32.0f))) {
        app_state.workspace = Workspace::Modeling;
    }
    ImGui::SameLine();
    if (ImGui::Button("Simulazione [F2]", ImVec2(160.0f, 32.0f))) {
        app_state.workspace = Workspace::Simulation;
        app_state.camera_orbit = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(app_state.show_project_workflow_panel ? "Project ON" : "Project OFF", ImVec2(120.0f, 32.0f))) {
        app_state.show_project_workflow_panel = !app_state.show_project_workflow_panel;
    }

    if (app_state.workspace == Workspace::Simulation) {
        int camera_mode = static_cast<int>(app_state.simulation_camera_mode);
        if (ImGui::Button(runtime.replay_active ? "Stop Replay" : "Replay Route", ImVec2(120.0f, 28.0f))) {
            runtime.replay_active = !runtime.replay_active;
            runtime.replay_time_s = 0.0;
        }
        ImGui::SameLine();
        ImGui::Checkbox("Markers", &app_state.show_flight_markers);
        ImGui::SameLine();
        ImGui::RadioButton("Fixed", &camera_mode, static_cast<int>(SimulationCameraMode::Fixed));
        ImGui::SameLine();
        ImGui::RadioButton("Follow", &camera_mode, static_cast<int>(SimulationCameraMode::Follow));
        ImGui::SameLine();
        ImGui::RadioButton("Free", &camera_mode, static_cast<int>(SimulationCameraMode::Free));
        app_state.simulation_camera_mode = static_cast<SimulationCameraMode>(camera_mode);
    } else {
        ImGui::Checkbox("Orbit Camera [TAB]", &app_state.camera_orbit);
        ImGui::SameLine();
        ImGui::Checkbox("Metrics", &app_state.show_metrics_overlay);
        ImGui::SameLine();
        ImGui::Checkbox("Wireframe", &app_state.show_wireframe_hint);
    }

    ImGui::NextColumn();
    if (ImGui::Button("Save", ImVec2(84.0f, 30.0f))) {
        app_state.request_project_save = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save As", ImVec2(84.0f, 30.0f))) {
        app_state.request_project_save_as = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Load", ImVec2(84.0f, 30.0f))) {
        app_state.request_project_load = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Export", ImVec2(84.0f, 30.0f))) {
        app_state.request_project_export = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(app_state.show_simulation_window ? "F3 Monitor ON" : "F3 Monitor OFF", ImVec2(140.0f, 30.0f))) {
        app_state.request_simulation_window_toggle = true;
    }
    if (app_state.workspace == Workspace::Simulation) {
        if (ImGui::Button(
                app_state.show_wind_tunnel_panel ? "Wind Tunnel ON" : "Wind Tunnel OFF",
                ImVec2(140.0f, 26.0f))) {
            app_state.show_wind_tunnel_panel = !app_state.show_wind_tunnel_panel;
        }
    }
    ImGui::TextDisabled("%s", app_state.workspace == Workspace::Modeling ? "CAD-like modeling focus" : "Flight control and diagnostics focus");
    ImGui::TextDisabled("Project: %s", app_state.current_project_path.string().c_str());
    ImGui::Columns(1);
    ImGui::End();
}

void drawProjectPanelImGui(AppState& app_state) {
    if (!app_state.show_project_workflow_panel) {
        return;
    }

    const ::Rectangle bounds {static_cast<float>(GetScreenWidth()) - 396.0f, 96.0f, 380.0f, 170.0f};
    setWindowRect(bounds);
    ImGui::Begin("Project & Export", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextWrapped("Percorsi attivi e trigger rapidi per save/load/export.");
    ImGui::Separator();
    ImGui::Text("Project");
    ImGui::TextWrapped("%s", app_state.current_project_path.string().c_str());
    ImGui::Text("Report");
    ImGui::TextWrapped("%s", app_state.current_report_path.string().c_str());
    ImGui::Text("CSV");
    ImGui::TextWrapped("%s", app_state.current_trajectory_csv_path.string().c_str());
    ImGui::End();
}

void drawModelingSidebarImGui(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    MotorEditorState& motor_editor,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime) {
    const ::Rectangle bounds {16.0f, 96.0f, 320.0f, static_cast<float>(GetScreenHeight()) - 112.0f};
    setWindowRect(bounds);
    ImGui::Begin("Modeling Control", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

    if (ImGui::BeginTabBar("modeling-tabs")) {
        if (ImGui::BeginTabItem("Tools")) {
            constexpr std::array<ModelingTool, 6> tools {
                ModelingTool::Select, ModelingTool::Move, ModelingTool::Rotate,
                ModelingTool::Scale, ModelingTool::AddPart, ModelingTool::Measure
            };
            for (const auto tool : tools) {
                if (ImGui::Selectable(std::format("{} {}", modelingToolLabel(tool), modelingToolShortcut(tool)).c_str(), app_state.modeling_tool == tool)) {
                    app_state.modeling_tool = tool;
                }
            }
            ImGui::Separator();
            constexpr std::array<const char*, 6> component_labels {
                "Nose Cone", "Body Tube", "Transition", "Fin Set", "Motor Mount", "Payload Section"
            };
            for (int index = 0; index < static_cast<int>(component_labels.size()); ++index) {
                if (ImGui::Selectable(component_labels[static_cast<std::size_t>(index)], app_state.selection == static_cast<ComponentSelection>(index))) {
                    app_state.selection = static_cast<ComponentSelection>(index);
                }
            }
            ImGui::Separator();
            ImGui::Checkbox("Show metrics", &app_state.show_metrics_overlay);
            ImGui::Checkbox("Show handles", &app_state.show_snap_points);
            ImGui::Checkbox("Show local grid", &app_state.show_local_grid);
            ImGui::Checkbox("Snap to grid", &app_state.snap_to_grid);
            ImGui::Checkbox("Wireframe", &app_state.show_wireframe_hint);
            ImGui::Checkbox("Diagnostics", &app_state.show_diagnostics);
            dragDouble("Grid spacing", app_state.grid_spacing_m, 0.001, 0.005, 0.25, "%.3f m");
            dragInt("Grid extent", app_state.grid_extent_cells, 0.25f, 4, 40);
            ImGui::TextWrapped("%s", modelingToolHint(app_state.modeling_tool));
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Presets")) {
            constexpr std::array<rocket::RocketPreset, 5> presets {
                rocket::RocketPreset::ResearchStarter,
                rocket::RocketPreset::SportTrainer,
                rocket::RocketPreset::HighAltitude,
                rocket::RocketPreset::MinimumDiameter,
                rocket::RocketPreset::HeavyLift
            };
            for (const auto preset : presets) {
                if (ImGui::Button(std::string(rocket::rocketPresetLabel(preset)).c_str(), ImVec2(-1.0f, 0.0f))) {
                    app_state.active_preset = preset;
                    vehicle.geometry = rocket::makePresetGeometry(preset);
                    rebuildVehicle(vehicle, mesh_generator, runtime);
                }
            }
            ImGui::SeparatorText("Quick Component Library");
            for (int index = 0; index < 5; ++index) {
                if (ImGui::Button(quickPresetLabel(app_state.selection, index), ImVec2(-1.0f, 0.0f))) {
                    applyComponentQuickPreset(app_state, vehicle, motor_editor, mesh_generator, runtime, index);
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Reference")) {
            ImGui::Text("Selection: %s", componentSelectionLabel(app_state.selection));
            ImGui::Text("Handle: %s", handleKindLabel(app_state.active_handle));
            ImGui::Text("Mesh mode: %s", meshSelectionModeLabel(app_state.mesh_selection_mode));
            ImGui::Separator();
            ImGui::TextWrapped("Viewport-first workflow: scegli il componente in 3D, rifinisci parametri qui, poi continua con mesh editing e simulazione.");
            ImGui::TextWrapped("F1/F2 cambiano workspace, F3 apre il monitor esterno, K scorre i keyframe.");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void drawModelingInspectorImGui(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    MotorEditorState& motor_editor,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime,
    const rocket::SimulationSnapshot& snapshot) {
    const ::Rectangle bounds {static_cast<float>(GetScreenWidth()) - 432.0f, 280.0f, 416.0f, static_cast<float>(GetScreenHeight()) - 296.0f};
    setWindowRect(bounds);
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("Componente: %s", componentSelectionLabel(app_state.selection));
    ImGui::TextDisabled("Tool attivo: %s", modelingToolLabel(app_state.modeling_tool));
    ImGui::Separator();

    bool changed = false;
    bool cluster_changed = false;
    auto& geometry = vehicle.geometry;

    switch (app_state.selection) {
    case ComponentSelection::NoseCone: {
        changed |= dragDouble("Length", geometry.nose_length_m, 0.005, 0.12, geometry.body_length_m * 0.45, "%.3f m");
        changed |= dragDouble("Mid radius scale", geometry.nose_controls.mid_radius_scale, 0.01, 0.55, 1.45, "%.2f");
        changed |= dragDouble("Shoulder scale", geometry.nose_controls.shoulder_radius_scale, 0.01, 0.70, 1.25, "%.2f");
        changed |= comboComponentMaterial("Material", geometry.nose_material);
        constexpr std::array<rocket::NoseConeShape, 4> values {
            rocket::NoseConeShape::Conical,
            rocket::NoseConeShape::TangentOgive,
            rocket::NoseConeShape::Parabolic,
            rocket::NoseConeShape::LdHaack
        };
        constexpr std::array<const char*, 4> labels {"Conical", "Tangent Ogive", "Parabolic", "LD-Haack"};
        changed |= comboEnum("Shape", geometry.nose_cone_shape, values, labels);
        break;
    }
    case ComponentSelection::BodyTube:
        changed |= dragDouble("Body length", geometry.body_length_m, 0.01, 0.8, 8.0, "%.3f m");
        changed |= dragDouble("Body diameter", geometry.body_diameter_m, 0.002, 0.04, 0.35, "%.3f m");
        changed |= dragDouble("Wall thickness", geometry.wall_thickness_m, 0.0002, 0.0015, 0.01, "%.4f m");
        changed |= dragDouble("Fore scale", geometry.body_controls.fore_radius_scale, 0.01, 0.70, 1.30, "%.2f");
        changed |= dragDouble("Mid scale", geometry.body_controls.mid_radius_scale, 0.01, 0.70, 1.30, "%.2f");
        changed |= dragDouble("Aft scale", geometry.body_controls.aft_radius_scale, 0.01, 0.55, 1.30, "%.2f");
        changed |= dragDouble("Structure CG", geometry.structure_cg_from_nose_m, 0.01, 0.2, geometry.body_length_m - 0.1, "%.3f m");
        changed |= comboComponentMaterial("Material", geometry.body_material);
        break;
    case ComponentSelection::Transition: {
        changed |= dragDouble("Length", geometry.transition_length_m, 0.005, 0.0, geometry.body_length_m * 0.25, "%.3f m");
        changed |= dragDouble("Aft diameter", geometry.transition_aft_diameter_m, 0.002, 0.03, geometry.body_diameter_m, "%.3f m");
        changed |= dragDouble("Mid scale", geometry.transition_controls.mid_radius_scale, 0.01, 0.55, 1.35, "%.2f");
        changed |= comboComponentMaterial("Material", geometry.transition_material);
        constexpr std::array<rocket::TransitionShape, 2> values {
            rocket::TransitionShape::Conical, rocket::TransitionShape::Curved
        };
        constexpr std::array<const char*, 2> labels {"Conical", "Curved"};
        changed |= comboEnum("Shape", geometry.transition_shape, values, labels);
        break;
    }
    case ComponentSelection::FinSet: {
        changed |= dragDouble("Front from nose", geometry.fin_front_from_nose_m, 0.01, geometry.nose_length_m + 0.05, geometry.body_length_m - geometry.transition_length_m - geometry.fin_root_chord_m - 0.02, "%.3f m");
        changed |= dragDouble("Root chord", geometry.fin_root_chord_m, 0.005, 0.08, 1.5, "%.3f m");
        changed |= dragDouble("Tip chord", geometry.fin_tip_chord_m, 0.005, 0.04, 1.2, "%.3f m");
        changed |= dragDouble("Span", geometry.fin_span_m, 0.005, 0.04, 0.8, "%.3f m");
        changed |= dragDouble("Sweep", geometry.fin_sweep_length_m, 0.005, 0.0, geometry.fin_root_chord_m * 0.95, "%.3f m");
        changed |= dragDouble("Tip LE offset", geometry.fin_controls.tip_le_offset_m, 0.002, -0.12, 0.12, "%.3f m");
        changed |= dragDouble("Tip TE offset", geometry.fin_controls.tip_te_offset_m, 0.002, -0.12, 0.12, "%.3f m");
        changed |= dragDouble("Span scale", geometry.fin_controls.span_scale, 0.01, 0.55, 1.45, "%.2f");
        changed |= dragDouble("Thickness scale", geometry.fin_controls.thickness_scale, 0.01, 0.55, 1.60, "%.2f");
        changed |= dragInt("Fin count", geometry.fin_count, 0.2f, 3, 6);
        changed |= comboComponentMaterial("Material", geometry.fin_material);
        constexpr std::array<rocket::FinShape, 3> values {
            rocket::FinShape::Trapezoidal, rocket::FinShape::Elliptical, rocket::FinShape::Airfoil
        };
        constexpr std::array<const char*, 3> labels {"Trapezoidal", "Elliptical", "Airfoil"};
        changed |= comboEnum("Shape", geometry.fin_shape, values, labels);
        break;
    }
    case ComponentSelection::MotorMount:
        cluster_changed |= dragInt("Motor count", motor_editor.motor_count, 0.2f, 1, 8);
        cluster_changed |= dragDouble("Mount radius", motor_editor.mount_radius_m, 0.001, 0.0, 0.20, "%.3f m");
        cluster_changed |= dragDouble("Max thrust", motor_editor.max_thrust_n, 1.0, 20.0, 1200.0, "%.1f N");
        cluster_changed |= dragDouble("Burn time", motor_editor.burn_time_s, 0.02, 0.2, 10.0, "%.2f s");
        cluster_changed |= dragDouble("Propellant mass", motor_editor.propellant_mass_kg, 0.01, 0.01, 5.0, "%.2f kg");
        cluster_changed |= dragDouble("Cant angle", motor_editor.cant_angle_deg, 0.1, 0.0, 8.0, "%.1f deg");
        break;
    case ComponentSelection::Payload:
        changed |= dragDouble("Payload length", geometry.payload_length_m, 0.005, 0.08, geometry.body_length_m * 0.25, "%.3f m");
        changed |= dragDouble("Payload mass", geometry.payload_mass_kg, 0.02, 0.1, 8.0, "%.2f kg");
        changed |= comboComponentMaterial("Material", geometry.payload_material);
        break;
    }

    if (changed) {
        rebuildVehicle(vehicle, mesh_generator, runtime);
    }
    if (cluster_changed) {
        rebuildMotorCluster(vehicle, motor_editor, mesh_generator, runtime);
    }

    ImGui::SeparatorText("Mesh Editing");
    auto* active_modifiers = vehicle.geometry.getActiveComponentModifiers(static_cast<rocket::ComponentType>(app_state.selection));
    if (active_modifiers != nullptr) {
        int mesh_mode = static_cast<int>(app_state.mesh_selection_mode);
        ImGui::Checkbox("Editable mesh", &active_modifiers->is_active);
        if (ImGui::IsItemDeactivatedAfterEdit() && active_modifiers->is_active) {
            mesh_generator.rebuild(vehicle.geometry, vehicle.cluster);
            syncActiveMeshVertices(app_state, vehicle, mesh_generator);
        }

        constexpr std::array<MeshSelectionMode, 3> modes {
            MeshSelectionMode::Vertex, MeshSelectionMode::Edge, MeshSelectionMode::Face
        };
        for (const auto mode : modes) {
            if (ImGui::RadioButton(meshSelectionModeLabel(mode), &mesh_mode, static_cast<int>(mode))) {
                clearMeshSelection(app_state);
            }
            ImGui::SameLine();
        }
        app_state.mesh_selection_mode = static_cast<MeshSelectionMode>(mesh_mode);
        ImGui::NewLine();
        if (ImGui::Button("Reset Component Mesh", ImVec2(-1.0f, 0.0f))) {
            clearComponentEdits(vehicle, static_cast<rocket::ComponentType>(app_state.selection));
            mesh_generator.rebuild(vehicle.geometry, vehicle.cluster);
            syncActiveMeshVertices(app_state, vehicle, mesh_generator);
        }
        dragDouble("Extrude distance", app_state.mesh_extrude_distance_m, 0.001, 0.005, 0.20, "%.3f m");
        dragDouble("Bevel ratio", app_state.mesh_bevel_ratio, 0.01, 0.05, 0.82, "%.2f");
        dragDouble("Bevel offset", app_state.mesh_bevel_offset_m, 0.001, -0.03, 0.08, "%.3f m");

        if (app_state.mesh_selection_mode == MeshSelectionMode::Face) {
            ImGui::Text("Selected face: %d", app_state.selected_face_id);
            if (ImGui::Button("Extrude Face", ImVec2(-1.0f, 0.0f))) {
                applyTopologyOperation(app_state, vehicle, mesh_generator, "extrude");
            }
            if (ImGui::Button("Bevel Face", ImVec2(-1.0f, 0.0f))) {
                applyTopologyOperation(app_state, vehicle, mesh_generator, "bevel");
            }
        } else if (app_state.mesh_selection_mode == MeshSelectionMode::Edge) {
            ImGui::Text("Selected edge: %d", app_state.selected_edge_id);
            if (ImGui::Button("Loop Cut Edge", ImVec2(-1.0f, 0.0f))) {
                applyTopologyOperation(app_state, vehicle, mesh_generator, "loop_cut");
            }
        } else {
            ImGui::Text("Selected vertex: %d", app_state.selected_vertex_id);
            ImGui::TextWrapped("Muovi i vertici dal viewport con tool Move per scolpire la mesh.");
        }
    }

    ImGui::SeparatorText("Live Summary");
    ImGui::Text("Static margin: %.2f cal", snapshot.static_margin_calibers);
    ImGui::Text("Mass: %.2f kg", snapshot.state.mass_kg);
    ImGui::Text("CG / CP: %.2f / %.2f m", snapshot.cg_from_nose_m, snapshot.cp_from_nose_m);
    ImGui::End();
}

void drawSimulationSidebarImGui(
    AppState& app_state,
    const rocket::SimulationSnapshot& snapshot,
    const SimulationRuntime& runtime,
    const rocket::VehicleModel& vehicle) {
    const ::Rectangle bounds {16.0f, 96.0f, 332.0f, static_cast<float>(GetScreenHeight()) - 112.0f};
    setWindowRect(bounds);
    ImGui::Begin("Flight Overview", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

    if (ImGui::BeginTabBar("sim-tabs")) {
        if (ImGui::BeginTabItem("Telemetry")) {
            drawMetricLabelValue("Time", std::format("{:.2f} s", snapshot.time_s), ImVec4(0.53f, 0.79f, 1.0f, 1.0f));
            drawMetricLabelValue("Altitude", std::format("{:.1f} m", snapshot.state.position_m.z), ImVec4(0.49f, 0.86f, 0.62f, 1.0f));
            drawMetricLabelValue("Air speed", std::format("{:.1f} m/s", snapshot.relative_air_speed_mps), ImVec4(0.54f, 0.88f, 1.0f, 1.0f));
            drawMetricLabelValue("Mach", std::format("{:.2f}", snapshot.mach_number), ImVec4(0.78f, 0.69f, 1.0f, 1.0f));
            drawMetricLabelValue("AoA", std::format("{:.2f} deg", snapshot.angle_of_attack_deg), ImVec4(0.99f, 0.67f, 0.38f, 1.0f));
            drawMetricLabelValue("Safety", std::format("{:.2f}x", snapshot.dynamic_pressure_safety_factor), snapshot.dynamic_pressure_safety_factor >= 1.35 ? ImVec4(0.49f, 0.86f, 0.62f, 1.0f) : snapshot.dynamic_pressure_safety_factor >= 1.0 ? ImVec4(0.99f, 0.82f, 0.39f, 1.0f) : ImVec4(0.99f, 0.49f, 0.49f, 1.0f));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Events")) {
            ImGui::BulletText("Launch at %.2f s", 0.0);
            if (runtime.burnout_recorded) {
                ImGui::BulletText("Burnout at %.2f s", runtime.burnout_time_s);
            }
            if (runtime.apogee_recorded) {
                ImGui::BulletText("Apogee at %.2f s", runtime.apogee_time_s);
            }
            if (runtime.impact_recorded) {
                ImGui::BulletText("Impact at %.2f s", runtime.impact_time_s);
            }
            ImGui::Separator();
            ImGui::Text("Motor cluster: %zu", vehicle.cluster.motorCount());
            for (std::size_t index = 0; index < vehicle.cluster.motorCount(); ++index) {
                ImGui::BulletText("Motor %zu: %s", index + 1, vehicle.cluster.motorFailed(index) ? "FAILED" : "OK");
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Keyframes")) {
            MissionKeyframe keyframe;
            if (tryGetSelectedMissionKeyframe(runtime, keyframe)) {
                ImGui::TextColored(toImVec4(keyframe.accent), "%s", keyframe.label);
                ImGui::Text("Time: %.2f s", keyframe.time_s);
                ImGui::Text("Altitude: %.1f m", snapshot.state.position_m.z);
                ImGui::Text("q: %.0f Pa", snapshot.dynamic_pressure_pa);
                ImGui::Text("CG / CP: %.2f / %.2f m", snapshot.cg_from_nose_m, snapshot.cp_from_nose_m);
            } else {
                ImGui::TextWrapped("Premi K per scorrere launch, burnout, apogee e impact.");
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void drawSimulationScenarioImGui(
    AppState& app_state,
    SimulationRuntime& runtime,
    rocket::VehicleModel& vehicle,
    rocket::Environment& environment) {
    primeWindowRect(app_state.layout.sim_scenario.bounds);
    ImGui::Begin("Scenario", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
    syncWindowRect(app_state.layout.sim_scenario);

    if (ImGui::Button(runtime.paused ? "Avvia / Riprendi" : "Pausa", ImVec2(-1.0f, 0.0f))) {
        runtime.paused = !runtime.paused;
    }
    if (ImGui::Button("Reset missione", ImVec2(-1.0f, 0.0f))) {
        resetSimulationRuntime(vehicle, runtime);
    }
    if (ImGui::Button(runtime.replay_active ? "Ferma replay rotta" : "Avvia replay rotta", ImVec2(-1.0f, 0.0f))) {
        runtime.replay_active = !runtime.replay_active;
        runtime.replay_time_s = 0.0;
    }

    auto site = environment.launchSite();
    auto weather = environment.surfaceWeather();
    bool environment_changed = false;

    ImGui::SeparatorText("Launch Site");
    environment_changed |= dragDouble("Latitude", site.latitude_deg, 0.02, 35.0, 47.5, "%.2f deg");
    environment_changed |= dragDouble("Longitude", site.longitude_deg, 0.02, 6.0, 19.0, "%.2f deg");
    environment_changed |= dragDouble("Elevation", site.elevation_m, 1.0, -50.0, 3500.0, "%.0f m");

    ImGui::SeparatorText("Weather");
    environment_changed |= dragDouble("Temp", weather.temperature_c, 0.2, -25.0, 45.0, "%.1f C");
    environment_changed |= dragDouble("Pressure", weather.pressure_hpa, 0.5, 850.0, 1055.0, "%.1f hPa");
    environment_changed |= dragDouble("Humidity", weather.humidity_percent, 0.5, 0.0, 100.0, "%.1f %%");
    environment_changed |= dragDouble("Wind avg", weather.wind_speed_mps, 0.1, 0.0, 35.0, "%.1f m/s");
    environment_changed |= dragDouble("Wind dir", weather.wind_direction_deg, 0.5, 0.0, 360.0, "%.1f deg");
    environment_changed |= dragDouble("Wind gust", weather.wind_gust_mps, 0.1, 0.0, 45.0, "%.1f m/s");

    ImGui::SeparatorText("Recovery");
    auto& recovery = vehicle.recovery_system;
    environment_changed |= dragDouble("Deploy altitude", recovery.deployment_altitude_m, 2.0, 30.0, 1200.0, "%.0f m");
    environment_changed |= dragDouble("Chute area", recovery.parachute_area_m2, 0.01, 0.1, 8.0, "%.2f m2");
    environment_changed |= dragDouble("Deploy delay", recovery.deployment_delay_s, 0.05, 0.0, 15.0, "%.1f s");

    if (environment_changed) {
        site.latitude_deg = std::clamp(site.latitude_deg, 35.0, 47.5);
        site.longitude_deg = std::clamp(site.longitude_deg, 6.0, 19.0);
        weather.wind_direction_deg = std::fmod(weather.wind_direction_deg + 360.0, 360.0);
        environment.setLaunchSite(site);
        environment.setSurfaceWeather(weather);
        resetSimulationRuntime(vehicle, runtime);
    }

    ImGui::SeparatorText("Weather Source");
    if (ImGui::Button(std::format("Source: {}", rocket::weatherSourceLabel(environment.weatherDataSource())).c_str(), ImVec2(-1.0f, 0.0f))) {
        const auto next_source =
            environment.weatherDataSource() == rocket::WeatherDataSource::Manual
                ? rocket::WeatherDataSource::OpenMeteoReady
                : environment.weatherDataSource() == rocket::WeatherDataSource::OpenMeteoReady
                      ? rocket::WeatherDataSource::OpenWeatherMapReady
                      : rocket::WeatherDataSource::Manual;
        environment.setWeatherDataSource(next_source);
    }
    if (environment.weatherDataSource() != rocket::WeatherDataSource::Manual &&
        ImGui::Button("Fetch Weather Now", ImVec2(-1.0f, 0.0f))) {
        const auto fetched = rocket::refreshEnvironmentWeather(environment);
        if (fetched) {
            resetSimulationRuntime(vehicle, runtime);
            setTransientStatus(
                app_state,
                std::format(
                    "Meteo aggiornato da {}: {:.1f} C, {:.1f} hPa, vento {:.1f} m/s",
                    fetched->provider_name,
                    fetched->weather.temperature_c,
                    fetched->weather.pressure_hpa,
                    fetched->weather.wind_speed_mps),
                5.0);
        } else {
            setTransientStatus(app_state, std::format("Fetch meteo fallita: {}", fetched.error()), 6.0);
        }
    }

    ImGui::SeparatorText("Cluster Motori");
    for (std::size_t index = 0; index < vehicle.cluster.motorCount(); ++index) {
        bool armed = !vehicle.cluster.motorFailed(index);
        if (ImGui::Checkbox(std::format("Motor {} armed", index + 1).c_str(), &armed)) {
            vehicle.cluster.setMotorFailed(index, !armed);
            resetSimulationRuntime(vehicle, runtime);
        }
    }

    ImGui::End();
}

void drawSimulationTelemetryWindowImGui(FloatingWindowState& state, const rocket::SimulationSnapshot& snapshot) {
    primeWindowRect(state.bounds);
    ImGui::Begin("Live Telemetry", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
    syncWindowRect(state);

    ImGui::TextWrapped("Lettura live organizzata per stato di volo, atmosfera e stabilita.");
    ImGui::Separator();

    if (ImGui::BeginTable("telemetry-top", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        drawStatCardImGui("Mission Time", std::format("{:.2f} s", snapshot.time_s), ImVec4(0.53f, 0.79f, 1.0f, 1.0f));
        ImGui::TableNextColumn();
        drawStatCardImGui("Altitude", std::format("{:.1f} m", snapshot.state.position_m.z), ImVec4(0.49f, 0.86f, 0.62f, 1.0f));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        drawStatCardImGui("Air Speed", std::format("{:.1f} m/s", snapshot.relative_air_speed_mps), ImVec4(0.54f, 0.88f, 1.0f, 1.0f));
        ImGui::TableNextColumn();
        const ImVec4 safety_color =
            snapshot.dynamic_pressure_safety_factor >= 1.35 ? ImVec4(0.49f, 0.86f, 0.62f, 1.0f)
            : snapshot.dynamic_pressure_safety_factor >= 1.0 ? ImVec4(0.99f, 0.82f, 0.39f, 1.0f)
                                                             : ImVec4(0.99f, 0.49f, 0.49f, 1.0f);
        drawStatCardImGui("Safety Factor", std::format("{:.2f}x", snapshot.dynamic_pressure_safety_factor), safety_color);
        ImGui::EndTable();
    }

    ImGui::SeparatorText("Flight");
    drawKeyValueTableImGui(
        "telemetry-flight",
        {
            {"Vertical velocity", std::format("{:.1f} m/s", snapshot.state.velocity_mps.z)},
            {"Mach", std::format("{:.2f}", snapshot.mach_number)},
            {"AoA", std::format("{:.2f} deg", snapshot.angle_of_attack_deg)},
            {"Wind speed", std::format("{:.1f} m/s", snapshot.wind_speed_mps)}
        });

    ImGui::SeparatorText("Atmosphere");
    drawKeyValueTableImGui(
        "telemetry-atmo",
        {
            {"Dynamic pressure", std::format("{:.0f} Pa", snapshot.dynamic_pressure_pa)},
            {"Static pressure", std::format("{:.0f} Pa", snapshot.static_pressure_pa)},
            {"Total pressure", std::format("{:.0f} Pa", snapshot.total_pressure_pa)},
            {"Air density", std::format("{:.3f} kg/m3", snapshot.air_density_kgpm3)},
            {"Air temperature", std::format("{:.1f} K", snapshot.air_temperature_k)},
            {"Speed of sound", std::format("{:.1f} m/s", snapshot.speed_of_sound_mps)}
        });

    ImGui::SeparatorText("Vehicle");
    drawKeyValueTableImGui(
        "telemetry-vehicle",
        {
            {"CG / CP", std::format("{:.2f} / {:.2f} m", snapshot.cg_from_nose_m, snapshot.cp_from_nose_m)},
            {"Static margin", std::format("{:.2f} cal", snapshot.static_margin_calibers)},
            {"q recommended", std::format("{:.0f} kPa", snapshot.recommended_max_dynamic_pressure_pa / 1000.0)},
            {"Struct modulus", std::format("{:.1f} GPa", snapshot.equivalent_structural_modulus_gpa)},
            {"Struct density", std::format("{:.0f} kg/m3", snapshot.equivalent_structural_density_kg_per_m3)}
        });

    ImGui::End();
}

void drawSimulationEventsWindowImGui(
    FloatingWindowState& state,
    const SimulationRuntime& runtime,
    const rocket::VehicleModel& vehicle,
    const rocket::SimulationSnapshot& snapshot) {
    primeWindowRect(state.bounds);
    ImGui::Begin("Mission Events", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
    syncWindowRect(state);

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

    const ImVec4 state_color =
        snapshot.parachute_deployed ? ImVec4(0.49f, 0.86f, 0.62f, 1.0f)
        : vehicle.cluster.isBurning(runtime.time_s) ? ImVec4(0.99f, 0.67f, 0.38f, 1.0f)
                                                    : ImVec4(0.75f, 0.84f, 0.98f, 1.0f);
    ImGui::TextColored(state_color, "%s", primary_event.c_str());
    ImGui::TextDisabled("%s", snapshot.static_margin_calibers >= 1.0 ? "Stabilita nominale" : "Margine statico basso");
    ImGui::Separator();

    if (ImGui::BeginTable("event-overview", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        drawStatCardImGui("Nose Profile", snapshot.nose_shape_label, ImVec4(0.53f, 0.79f, 1.0f, 1.0f), 54.0f);
        ImGui::TableNextColumn();
        drawStatCardImGui("Fin Profile", snapshot.fin_shape_label, ImVec4(0.78f, 0.69f, 1.0f, 1.0f), 54.0f);
        ImGui::EndTable();
    }

    ImGui::SeparatorText("Timeline");
    ImGui::BulletText("Launch: 0.00 s");
    if (runtime.burnout_recorded) {
        ImGui::BulletText("Burnout: %.2f s", runtime.burnout_time_s);
    }
    if (runtime.apogee_recorded) {
        ImGui::BulletText("Apogee: %.2f s", runtime.apogee_time_s);
    }
    if (runtime.impact_recorded) {
        ImGui::BulletText("Impact: %.2f s", runtime.impact_time_s);
    }

    ImGui::SeparatorText("Mission Flags");
    drawKeyValueTableImGui(
        "event-flags",
        {
            {"Replay", runtime.replay_active ? "Active" : "Off"},
            {"Keyframe preview", runtime.keyframe_preview_active ? "Active" : "Off"},
            {"Recovery", snapshot.parachute_deployed ? "Deployed" : "Stowed"},
            {"Ground contact", runtime.impact_recorded ? "Recorded" : "No"}
        });

    ImGui::SeparatorText("Motor Cluster");
    for (std::size_t index = 0; index < vehicle.cluster.motorCount(); ++index) {
        const bool armed = !vehicle.cluster.motorFailed(index);
        ImGui::BulletText("Motor %zu: %s", index + 1, armed ? "ARMED" : "FAILED");
    }

    ImGui::End();
}

::Rectangle beginOverlayHostWindow(FloatingWindowState& state, const char* title) {
    primeWindowRect(state.bounds);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.03f, 0.05f, 0.05f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(
        title,
        nullptr,
        ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);
    ::Rectangle content_bounds = syncWindowRect(state);
    ImGui::Dummy(ImVec2(std::max(0.0f, content_bounds.width), std::max(0.0f, content_bounds.height)));
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    return content_bounds;
}

void renderDearImGuiUi(
    AppState& app_state,
    rocket::VehicleModel& vehicle,
    MotorEditorState& motor_editor,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime,
    rocket::Environment& environment,
    const rocket::SimulationSnapshot& modeling_snapshot,
    const rocket::SimulationSnapshot& simulation_snapshot) {
    drawTopBarImGui(app_state, runtime);
    drawProjectPanelImGui(app_state);

    if (app_state.workspace == Workspace::Modeling) {
        drawModelingSidebarImGui(app_state, vehicle, motor_editor, mesh_generator, runtime);
        drawModelingInspectorImGui(app_state, vehicle, motor_editor, mesh_generator, runtime, modeling_snapshot);
    } else {
        drawSimulationScenarioImGui(app_state, runtime, vehicle, environment);
        drawSimulationTelemetryWindowImGui(app_state.layout.sim_telemetry, simulation_snapshot);
        drawSimulationEventsWindowImGui(app_state.layout.sim_events, runtime, vehicle, simulation_snapshot);

        const ::Rectangle overview_bounds = beginOverlayHostWindow(app_state.layout.sim_overview, "Trajectory");
        const ::Rectangle timeline_bounds = beginOverlayHostWindow(app_state.layout.sim_timeline, "Mission Timeline");
        ::Rectangle wind_tunnel_bounds {};
        if (app_state.show_wind_tunnel_panel) {
            wind_tunnel_bounds = beginOverlayHostWindow(app_state.layout.sim_wind_tunnel, "Wind Tunnel");
        }

        if (app_state.show_wind_tunnel_panel) {
            drawWindTunnelPanel(wind_tunnel_bounds, app_state, simulation_snapshot, vehicle);
        }
        drawTrajectoryOverview(overview_bounds, runtime);
        drawSimulationTimeline(timeline_bounds, runtime, vehicle);

        if (runtime.keyframe_preview_active) {
            const ::Rectangle keyframe_bounds = beginOverlayHostWindow(app_state.layout.sim_keyframe, "Keyframe");
            drawMissionKeyframePreview(keyframe_bounds, runtime, simulation_snapshot);
        }
    }
}
