constexpr double pi = 3.14159265358979323846;

enum class Workspace {
    Modeling,
    Simulation
};

enum class ModelingTool {
    Select,
    Move,
    Rotate,
    Scale,
    AddPart,
    Measure
};

enum class ComponentSelection {
    NoseCone,
    BodyTube,
    Transition,
    FinSet,
    MotorMount,
    Payload
};

enum class ModelingView {
    Perspective,
    Front,
    Side,
    Top
};

enum class SimulationCameraMode {
    Fixed,
    Follow,
    Free
};

enum class MeshSelectionMode {
    Vertex,
    Edge,
    Face
};

enum class HandleKind {
    None,
    NoseLength,
    NoseMidRadius,
    NoseShoulder,
    BodyLength,
    BodyDiameter,
    BodyForeRadius,
    BodyMidRadius,
    BodyAftRadius,
    TransitionLength,
    TransitionAftDiameter,
    TransitionMidRadius,
    FinFront,
    FinSpan,
    FinTipLead,
    FinTipTrail,
    PayloadLength,
    MotorMountRadius,
    VertexFree,
    EdgeFree,
    FaceFree
};

const char* modelingToolLabel(ModelingTool tool) {
    switch (tool) {
    case ModelingTool::Select:
        return "Select";
    case ModelingTool::Move:
        return "Move";
    case ModelingTool::Rotate:
        return "Rotate";
    case ModelingTool::Scale:
        return "Scale";
    case ModelingTool::AddPart:
        return "Add";
    case ModelingTool::Measure:
        return "Measure";
    }
    return "Select";
}

const char* modelingToolShortcut(ModelingTool tool) {
    switch (tool) {
    case ModelingTool::Select:
        return "[1]";
    case ModelingTool::Move:
        return "[2]";
    case ModelingTool::Rotate:
        return "[3]";
    case ModelingTool::Scale:
        return "[4]";
    case ModelingTool::AddPart:
        return "[5]";
    case ModelingTool::Measure:
        return "[6]";
    }
    return "[1]";
}

const char* modelingToolHint(ModelingTool tool) {
    switch (tool) {
    case ModelingTool::Select:
        return "Seleziona il componente e porta in focus handle, griglia locale e inspector.";
    case ModelingTool::Move:
        return "Usa gli handle per spostare quote e station con feedback immediato in viewport.";
    case ModelingTool::Rotate:
        return "Modalita di review: controlla assetto, sweep e geometrie da piu viste prima di rifinire.";
    case ModelingTool::Scale:
        return "Lavora su diametri, span e thickness mantenendo il rapporto con la griglia locale.";
    case ModelingTool::AddPart:
        return "Applica preset di progetto o di componente per espandere velocemente il layout del veicolo.";
    case ModelingTool::Measure:
        return "Leggi lunghezze, margine statico e allineamento CG/CP direttamente nel contesto 3D.";
    }
    return "Controllo contestuale della modellazione.";
}

const char* handleKindLabel(HandleKind handle) {
    switch (handle) {
    case HandleKind::NoseLength:
        return "Nose Length";
    case HandleKind::NoseMidRadius:
        return "Nose Mid";
    case HandleKind::NoseShoulder:
        return "Shoulder";
    case HandleKind::BodyLength:
        return "Body Length";
    case HandleKind::BodyDiameter:
        return "Body Diameter";
    case HandleKind::BodyForeRadius:
        return "Fore Radius";
    case HandleKind::BodyMidRadius:
        return "Mid Radius";
    case HandleKind::BodyAftRadius:
        return "Aft Radius";
    case HandleKind::TransitionLength:
        return "Transition Length";
    case HandleKind::TransitionAftDiameter:
        return "Aft Diameter";
    case HandleKind::TransitionMidRadius:
        return "Transition Mid";
    case HandleKind::FinFront:
        return "Fin Front";
    case HandleKind::FinSpan:
        return "Fin Span";
    case HandleKind::FinTipLead:
        return "Tip LE";
    case HandleKind::FinTipTrail:
        return "Tip TE";
    case HandleKind::PayloadLength:
        return "Payload Length";
    case HandleKind::MotorMountRadius:
        return "Cluster Radius";
    case HandleKind::VertexFree:
        return "Free Vertex";
    case HandleKind::EdgeFree:
        return "Mesh Edge";
    case HandleKind::FaceFree:
        return "Mesh Face";
    case HandleKind::None:
        return "None";
    }
    return "None";
}

const char* meshSelectionModeLabel(MeshSelectionMode mode) {
    switch (mode) {
    case MeshSelectionMode::Vertex:
        return "Vertex";
    case MeshSelectionMode::Edge:
        return "Edge";
    case MeshSelectionMode::Face:
        return "Face";
    }
    return "Vertex";
}

struct HandleSpec {
    HandleKind kind {HandleKind::None};
    rocket::Vector3 world_position {};
    Color color {96, 165, 250, 255};
    std::string label {};
    int topology_id {-1};
};

struct MotorEditorState {
    int motor_count {2};
    double max_thrust_n {180.0};
    double burn_time_s {2.4};
    double propellant_mass_kg {0.24};
    double mount_radius_m {0.04};
    double cant_angle_deg {};
};

struct TrajectorySample {
    rocket::FlightState state {};
    double time_s {};
};

struct SimulationRuntime {
    rocket::FlightState initial_state {};
    rocket::FlightState state {};
    double time_s {};
    double accumulator_s {};
    double max_altitude_m {};
    bool paused {true};
    std::deque<TrajectorySample> trajectory_history {};
    bool burnout_recorded {false};
    bool apogee_recorded {false};
    bool impact_recorded {false};
    bool replay_active {false};
    double replay_time_s {};
    rocket::Vector3 burnout_point_m {};
    rocket::Vector3 apogee_point_m {};
    rocket::Vector3 impact_point_m {};
    double burnout_time_s {};
    double apogee_time_s {};
    double impact_time_s {};
    bool keyframe_preview_active {false};
    int keyframe_preview_index {-1};
    double keyframe_preview_time_s {};
};

struct MissionKeyframe {
    const char* label {""};
    double time_s {};
    Color accent {255, 255, 255, 255};
};

struct FloatingWindowState {
    ::Rectangle bounds {};
    bool visible {true};
    bool dragging {false};
    ::Vector2 drag_offset {};
};

struct LayoutState {
    FloatingWindowState modeling_toolbar {::Rectangle {20.0f, 84.0f, 128.0f, 324.0f}};
    FloatingWindowState modeling_outliner {::Rectangle {164.0f, 84.0f, 258.0f, 282.0f}};
    FloatingWindowState modeling_library {::Rectangle {164.0f, 384.0f, 258.0f, 404.0f}};
    FloatingWindowState modeling_properties {::Rectangle {1168.0f, 84.0f, 408.0f, 700.0f}};
    FloatingWindowState modeling_status {::Rectangle {164.0f, 806.0f, 952.0f, 104.0f}};
    FloatingWindowState modeling_reference {::Rectangle {444.0f, 84.0f, 272.0f, 452.0f}};

    FloatingWindowState sim_telemetry {::Rectangle {20.0f, 84.0f, 316.0f, 280.0f}};
    FloatingWindowState sim_events {::Rectangle {20.0f, 384.0f, 316.0f, 244.0f}};
    FloatingWindowState sim_wind_tunnel {::Rectangle {360.0f, 84.0f, 652.0f, 312.0f}};
    FloatingWindowState sim_overview {::Rectangle {360.0f, 420.0f, 652.0f, 248.0f}};
    FloatingWindowState sim_timeline {::Rectangle {360.0f, 692.0f, 652.0f, 128.0f}};
    FloatingWindowState sim_scenario {::Rectangle {1192.0f, 84.0f, 368.0f, 700.0f}};
    FloatingWindowState sim_keyframe {::Rectangle {20.0f, 648.0f, 316.0f, 296.0f}};
};

struct AppState {
    Workspace workspace {Workspace::Modeling};
    ModelingTool modeling_tool {ModelingTool::Select};
    ModelingView modeling_view {ModelingView::Perspective};
    SimulationCameraMode simulation_camera_mode {SimulationCameraMode::Fixed};
    ComponentSelection selection {ComponentSelection::NoseCone};
    rocket::RocketPreset active_preset {rocket::RocketPreset::ResearchStarter};
    HandleKind active_handle {HandleKind::None};
    bool dragging_handle {false};
    bool camera_orbit {false};
    bool show_metrics_overlay {true};
    bool show_snap_points {true};
    bool show_wireframe_hint {false};
    bool show_diagnostics {true};
    bool show_local_grid {true};
    bool snap_to_grid {true};
    bool show_simulation_window {false};
    bool show_reference_blueprint {true};
    bool simulation_multi_window_mode {true};
    bool show_flight_markers {true};
    ComponentSelection wind_tunnel_focus {ComponentSelection::NoseCone};
    double reference_board_scale_m {2.6};
    double reference_board_offset_m {};
    double grid_spacing_m {0.02};
    int grid_extent_cells {12};
    rocket::RealTimeCfdField cfd_field {};
    ::Vector3 modeling_camera_target {0.0f, 0.0f, 0.0f};
    float modeling_camera_distance {18.0f};
    float modeling_camera_yaw_rad {0.78f};
    float modeling_camera_pitch_rad {0.42f};
    LayoutState layout {};

    MeshSelectionMode mesh_selection_mode {MeshSelectionMode::Vertex};
    int selected_vertex_id {-1};
    int selected_edge_id {-1};
    int selected_face_id {-1};
    bool dragging_vertex {false};
    rocket::Vector3 vertex_drag_start_world {};
    double mesh_extrude_distance_m {0.03};
    double mesh_bevel_ratio {0.22};
    double mesh_bevel_offset_m {0.008};
};

void rebuildVehicle(
    rocket::VehicleModel& vehicle,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime);

bool isMouseOverModelingUi(const AppState& app_state);

::Vector3 toRaylib(const rocket::Vector3& vector) {
    return {
        static_cast<float>(vector.x),
        static_cast<float>(vector.z),
        static_cast<float>(vector.y)
    };
}

float toWorldY(double altitude_m) {
    return static_cast<float>(altitude_m);
}

rocket::FlightState buildRestState(const rocket::VehicleModel& vehicle) {
    return {
        .position_m = {0.0, 0.0, 0.0},
        .velocity_mps = {0.0, 0.0, 0.0},
        .attitude_body_to_world = {},
        .angular_velocity_body_radps = {0.0, 0.0, 0.0},
        .mass_kg = vehicle.dry_mass_kg + vehicle.cluster.totalPropellantMassKg()
    };
}

double modelingPreviewLiftM(const rocket::VehicleGeometry& geometry) {
    const double body_start_z_m = -0.5 * geometry.body_length_m;
    const double casing_length_m = std::clamp(geometry.body_diameter_m * 1.1, 0.08, 0.24);
    const double nozzle_length_m = std::clamp(geometry.body_diameter_m * 0.45, 0.03, 0.09);
    const double casing_fore_z_m = body_start_z_m + geometry.transition_length_m + 0.03;
    const double casing_aft_z_m = casing_fore_z_m - casing_length_m;
    const double nozzle_aft_z_m = casing_aft_z_m - nozzle_length_m;
    const double lowest_point_z_m = std::min(body_start_z_m, nozzle_aft_z_m);
    return std::max(-lowest_point_z_m, 0.0);
}

rocket::FlightState buildModelingPreviewState(const rocket::VehicleModel& vehicle) {
    rocket::FlightState state = buildRestState(vehicle);
    state.position_m.z = modelingPreviewLiftM(vehicle.geometry);
    return state;
}

double deriveBaseDragCoefficient(const rocket::VehicleGeometry& geometry) {
    const auto nose_material = rocket::materialDefinition(geometry.nose_material);
    const auto body_material = rocket::materialDefinition(geometry.body_material);
    const auto transition_material = rocket::materialDefinition(geometry.transition_material);
    const auto fin_material = rocket::materialDefinition(geometry.fin_material);

    const auto surface_drag_bias = [](const rocket::MaterialDefinition& material) {
        if (material.label == "Carbon Fiber") {
            return -0.022;
        }
        if (material.label == "Fiberglass") {
            return -0.014;
        }
        if (material.label == "Phenolic Tube") {
            return -0.010;
        }
        if (material.label == "Alluminio 6061") {
            return -0.006;
        }
        if (material.label == "PLA-CF") {
            return 0.010;
        }
        if (material.label == "Betulla Aircraft") {
            return 0.014;
        }
        return 0.018;
    };

    double drag = 0.46;

    switch (geometry.nose_cone_shape) {
    case rocket::NoseConeShape::Conical:
        drag += 0.08;
        break;
    case rocket::NoseConeShape::TangentOgive:
        drag += 0.03;
        break;
    case rocket::NoseConeShape::Parabolic:
        drag += 0.05;
        break;
    case rocket::NoseConeShape::LdHaack:
        drag += 0.0;
        break;
    }

    switch (geometry.fin_shape) {
    case rocket::FinShape::Trapezoidal:
        drag += 0.03;
        break;
    case rocket::FinShape::Elliptical:
        drag += 0.02;
        break;
    case rocket::FinShape::Airfoil:
        drag += 0.0;
        break;
    }

    if (geometry.transition_length_m > 1e-6) {
        drag += geometry.transition_shape == rocket::TransitionShape::Curved ? -0.03 : -0.01;
    }

    const double fineness_ratio = geometry.body_length_m / std::max(geometry.body_diameter_m, 1e-6);
    if (fineness_ratio > 18.0) {
        drag -= 0.02;
    } else if (fineness_ratio < 10.0) {
        drag += 0.03;
    }

    const double thickness_ratio =
        (geometry.fin_thickness_m * geometry.fin_controls.thickness_scale) /
        std::max(geometry.body_diameter_m, 1e-6);
    drag += std::clamp((thickness_ratio - 0.045) * 0.9, -0.015, 0.05);
    drag += 0.22 * surface_drag_bias(nose_material);
    drag += 0.36 * surface_drag_bias(body_material);
    drag += 0.14 * surface_drag_bias(transition_material);
    drag += 0.28 * surface_drag_bias(fin_material);

    return std::clamp(drag, 0.34, 0.82);
}

double deriveNormalForceSlopePerRad(const rocket::VehicleGeometry& geometry) {
    const auto fin_material = rocket::materialDefinition(geometry.fin_material);
    const auto body_material = rocket::materialDefinition(geometry.body_material);
    const double radius_m = geometry.body_diameter_m * 0.5;
    const double tip_chord_m = std::max(geometry.fin_tip_chord_m + geometry.fin_controls.tip_te_offset_m, 0.04);
    const double mean_chord_m = 0.5 * (geometry.fin_root_chord_m + tip_chord_m);
    const double effective_span_m = geometry.fin_span_m * geometry.fin_controls.span_scale + radius_m;
    const double aspect_ratio =
        (2.0 * effective_span_m * effective_span_m) /
        std::max((geometry.fin_root_chord_m + tip_chord_m) * effective_span_m, 1e-6);
    const double fin_term =
        (2.1 + 0.24 * static_cast<double>(geometry.fin_count)) *
        std::clamp(aspect_ratio / 2.3, 0.7, 1.8);

    double nose_term = 1.4;
    if (geometry.nose_cone_shape == rocket::NoseConeShape::TangentOgive) {
        nose_term = 1.55;
    } else if (geometry.nose_cone_shape == rocket::NoseConeShape::LdHaack) {
        nose_term = 1.48;
    }

    const double tail_bonus = geometry.transition_length_m > 1e-6 ? 0.12 : 0.0;
    const double chord_bias = std::clamp(mean_chord_m / std::max(geometry.body_diameter_m, 1e-6), 0.7, 3.0);
    const double fin_stiffness_factor = std::clamp(fin_material.youngs_modulus_gpa / 18.0, 0.78, 1.18);
    const double body_support_factor = std::clamp(body_material.youngs_modulus_gpa / 28.0, 0.88, 1.08);
    return std::clamp(
        nose_term + fin_term * 0.46 * fin_stiffness_factor + tail_bonus + 0.12 * chord_bias + 0.08 * (body_support_factor - 1.0),
        2.6,
        8.8);
}

double deriveRotationalDampingCoefficient(const rocket::VehicleGeometry& geometry, double dry_mass_kg) {
    const auto fin_material = rocket::materialDefinition(geometry.fin_material);
    const auto body_material = rocket::materialDefinition(geometry.body_material);
    const double reference_area_m2 = pi * std::pow(geometry.body_diameter_m * 0.5, 2.0);
    const double fin_area_m2 =
        0.5 * (geometry.fin_root_chord_m + geometry.fin_tip_chord_m) *
        (geometry.fin_span_m * geometry.fin_controls.span_scale) *
        static_cast<double>(geometry.fin_count);
    const double lever_arm_m =
        std::max(geometry.fin_front_from_nose_m - geometry.structure_cg_from_nose_m, 0.12);
    const double fin_damping_factor = std::clamp(std::sqrt(fin_material.youngs_modulus_gpa / 11.0), 0.82, 1.28);
    const double shell_damping_factor = std::clamp(body_material.density_kg_per_m3 / 1600.0, 0.82, 1.26);
    const double damping =
        0.42 + 2.8 * fin_area_m2 / std::max(reference_area_m2, 1e-6) * fin_damping_factor +
        0.18 * lever_arm_m + 0.025 * dry_mass_kg + 0.12 * (shell_damping_factor - 1.0);
    return std::clamp(damping, 0.7, 4.5);
}

rocket::Vector3 estimatePrincipalInertiaKgM2(const rocket::VehicleGeometry& geometry, double dry_mass_kg) {
    const auto breakdown = rocket::estimateStructureMassBreakdown(geometry);
    const double avionics_mass_kg = std::max(dry_mass_kg - breakdown.total_mass_kg - geometry.payload_mass_kg, 0.0);
    const double body_radius_m = geometry.body_diameter_m * 0.5;
    const double aft_radius_m = std::max(geometry.transition_aft_diameter_m * 0.5, 0.01);
    const double cylinder_length_m =
        std::max(geometry.body_length_m - geometry.nose_length_m - geometry.transition_length_m, 0.2);

    struct InertiaPart {
        double mass_kg;
        double station_from_nose_m;
        double axial_radius_m;
        double local_length_m;
    };

    const std::array<InertiaPart, 7> parts {{
        {breakdown.nose_mass_kg, geometry.nose_length_m * 0.45, body_radius_m * 0.72, geometry.nose_length_m},
        {breakdown.payload_bay_mass_kg, geometry.nose_length_m + geometry.payload_length_m * 0.5, body_radius_m * 0.85, geometry.payload_length_m},
        {geometry.payload_mass_kg, geometry.nose_length_m + geometry.payload_length_m * 0.5, body_radius_m * 0.55, geometry.payload_length_m * 0.8},
        {breakdown.body_mass_kg, geometry.nose_length_m + cylinder_length_m * 0.5, body_radius_m, cylinder_length_m},
        {breakdown.transition_mass_kg, geometry.body_length_m - geometry.transition_length_m * 0.5, 0.5 * (body_radius_m + aft_radius_m), geometry.transition_length_m},
        {breakdown.fin_mass_kg, geometry.fin_front_from_nose_m + geometry.fin_root_chord_m * 0.45, body_radius_m + geometry.fin_span_m * geometry.fin_controls.span_scale * 0.7, geometry.fin_root_chord_m},
        {avionics_mass_kg, geometry.nose_length_m + geometry.payload_length_m + cylinder_length_m * 0.18, body_radius_m * 0.45, std::max(geometry.payload_length_m, 0.10)}
    }};

    double cg_from_nose_m = 0.0;
    for (const auto& part : parts) {
        cg_from_nose_m += part.mass_kg * part.station_from_nose_m;
    }
    cg_from_nose_m /= std::max(dry_mass_kg, 1e-6);

    double i_transverse = 0.0;
    double i_axial = 0.0;
    for (const auto& part : parts) {
        const double local_transverse =
            (part.mass_kg / 12.0) *
            (3.0 * part.axial_radius_m * part.axial_radius_m + part.local_length_m * part.local_length_m);
        const double local_axial = 0.5 * part.mass_kg * part.axial_radius_m * part.axial_radius_m;
        const double lever_m = part.station_from_nose_m - cg_from_nose_m;
        i_transverse += local_transverse + part.mass_kg * lever_m * lever_m;
        i_axial += local_axial;
    }
    return {
        std::max(i_transverse, 0.02),
        std::max(i_transverse, 0.02),
        std::max(i_axial, 0.002)
    };
}

void syncVehicleDerivedValues(rocket::VehicleModel& vehicle) {
    using rocket::secure::clampFinite;
    using rocket::secure::clampFiniteInt;
    vehicle.geometry.body_length_m = clampFinite(vehicle.geometry.body_length_m, 2.5, 0.8, 8.0);
    vehicle.geometry.body_diameter_m = clampFinite(vehicle.geometry.body_diameter_m, 0.1, 0.04, 0.35);
    vehicle.geometry.wall_thickness_m = clampFinite(vehicle.geometry.wall_thickness_m, 0.003, 0.0015, 0.01);
    vehicle.geometry.nose_length_m = clampFinite(vehicle.geometry.nose_length_m, 0.4, 0.12, vehicle.geometry.body_length_m * 0.45);
    vehicle.geometry.transition_length_m = clampFinite(vehicle.geometry.transition_length_m, 0.12, 0.0, vehicle.geometry.body_length_m * 0.25);
    vehicle.geometry.transition_aft_diameter_m = clampFinite(vehicle.geometry.transition_aft_diameter_m, vehicle.geometry.body_diameter_m * 0.8, 0.03, vehicle.geometry.body_diameter_m);
    vehicle.geometry.fin_root_chord_m = clampFinite(vehicle.geometry.fin_root_chord_m, 0.24, 0.08, 1.5);
    vehicle.geometry.fin_tip_chord_m = clampFinite(vehicle.geometry.fin_tip_chord_m, 0.10, 0.04, 1.2);
    vehicle.geometry.fin_span_m = clampFinite(vehicle.geometry.fin_span_m, 0.12, 0.04, 0.8);
    vehicle.geometry.fin_sweep_length_m = clampFinite(vehicle.geometry.fin_sweep_length_m, 0.06, 0.0, vehicle.geometry.fin_root_chord_m * 0.95);
    vehicle.geometry.fin_front_from_nose_m = clampFinite(
        vehicle.geometry.fin_front_from_nose_m,
        vehicle.geometry.body_length_m * 0.72,
        vehicle.geometry.nose_length_m + 0.05,
        vehicle.geometry.body_length_m - vehicle.geometry.transition_length_m - vehicle.geometry.fin_root_chord_m - 0.02);
    vehicle.geometry.fin_count = clampFiniteInt(vehicle.geometry.fin_count, 4, 3, 6);
    vehicle.geometry.payload_length_m = clampFinite(vehicle.geometry.payload_length_m, 0.18, 0.08, vehicle.geometry.body_length_m * 0.25);
    vehicle.geometry.payload_mass_kg = clampFinite(vehicle.geometry.payload_mass_kg, 0.8, 0.1, 8.0);
    vehicle.geometry.nose_controls.mid_radius_scale = clampFinite(vehicle.geometry.nose_controls.mid_radius_scale, 1.0, 0.55, 1.45);
    vehicle.geometry.nose_controls.shoulder_radius_scale = clampFinite(vehicle.geometry.nose_controls.shoulder_radius_scale, 1.0, 0.7, 1.25);
    vehicle.geometry.body_controls.fore_radius_scale = clampFinite(vehicle.geometry.body_controls.fore_radius_scale, 1.0, 0.7, 1.3);
    vehicle.geometry.body_controls.mid_radius_scale = clampFinite(vehicle.geometry.body_controls.mid_radius_scale, 1.0, 0.7, 1.3);
    vehicle.geometry.body_controls.aft_radius_scale = clampFinite(vehicle.geometry.body_controls.aft_radius_scale, 1.0, 0.55, 1.3);
    vehicle.geometry.transition_controls.mid_radius_scale = clampFinite(vehicle.geometry.transition_controls.mid_radius_scale, 1.0, 0.55, 1.35);
    vehicle.geometry.fin_controls.tip_le_offset_m = clampFinite(vehicle.geometry.fin_controls.tip_le_offset_m, 0.0, -0.12, 0.12);
    vehicle.geometry.fin_controls.tip_te_offset_m = clampFinite(vehicle.geometry.fin_controls.tip_te_offset_m, 0.0, -0.12, 0.12);
    vehicle.geometry.fin_controls.span_scale = clampFinite(vehicle.geometry.fin_controls.span_scale, 1.0, 0.55, 1.45);
    vehicle.geometry.fin_controls.thickness_scale = clampFinite(vehicle.geometry.fin_controls.thickness_scale, 1.0, 0.55, 1.6);
    vehicle.geometry.structure_cg_from_nose_m = clampFinite(vehicle.geometry.structure_cg_from_nose_m, vehicle.geometry.body_length_m * 0.48, 0.2, vehicle.geometry.body_length_m - 0.1);
    vehicle.geometry.propellant_cg_from_nose_m = clampFinite(vehicle.geometry.propellant_cg_from_nose_m, vehicle.geometry.body_length_m * 0.75, vehicle.geometry.structure_cg_from_nose_m, vehicle.geometry.body_length_m - 0.05);
    vehicle.reference_area_m2 = pi * std::pow(vehicle.geometry.body_diameter_m * 0.5, 2.0);
    vehicle.dry_mass_kg = rocket::estimateDryMassKg(vehicle.geometry, 0.45);
    vehicle.aerodynamic_coefficients.drag_coefficient = deriveBaseDragCoefficient(vehicle.geometry);
    vehicle.aerodynamic_coefficients.normal_force_slope_per_rad = deriveNormalForceSlopePerRad(vehicle.geometry);
    vehicle.aerodynamic_coefficients.rotational_damping_coefficient =
        deriveRotationalDampingCoefficient(vehicle.geometry, vehicle.dry_mass_kg);
    vehicle.principal_inertia_kgm2 = estimatePrincipalInertiaKgM2(vehicle.geometry, vehicle.dry_mass_kg);
}

void resetSimulationRuntime(const rocket::VehicleModel& vehicle, SimulationRuntime& runtime) {
    runtime.initial_state = buildRestState(vehicle);
    runtime.state = runtime.initial_state;
    runtime.time_s = 0.0;
    runtime.accumulator_s = 0.0;
    runtime.max_altitude_m = 0.0;
    runtime.paused = true;
    runtime.burnout_recorded = false;
    runtime.apogee_recorded = false;
    runtime.impact_recorded = false;
    runtime.replay_active = false;
    runtime.replay_time_s = 0.0;
    runtime.burnout_point_m = runtime.state.position_m;
    runtime.apogee_point_m = runtime.state.position_m;
    runtime.impact_point_m = runtime.state.position_m;
    runtime.burnout_time_s = 0.0;
    runtime.apogee_time_s = 0.0;
    runtime.impact_time_s = 0.0;
    runtime.keyframe_preview_active = false;
    runtime.keyframe_preview_index = -1;
    runtime.keyframe_preview_time_s = 0.0;
    runtime.trajectory_history.clear();
    runtime.trajectory_history.push_back(TrajectorySample {.state = runtime.state, .time_s = 0.0});
}

std::vector<MissionKeyframe> buildMissionKeyframes(const SimulationRuntime& runtime) {
    std::vector<MissionKeyframe> keyframes;
    keyframes.push_back(MissionKeyframe {
        .label = "Launch",
        .time_s = 0.0,
        .accent = Color {34, 197, 94, 255}
    });
    if (runtime.burnout_recorded) {
        keyframes.push_back(MissionKeyframe {
            .label = "Burnout",
            .time_s = runtime.burnout_time_s,
            .accent = Color {249, 115, 22, 255}
        });
    }
    if (runtime.apogee_recorded) {
        keyframes.push_back(MissionKeyframe {
            .label = "Apogee",
            .time_s = runtime.apogee_time_s,
            .accent = Color {168, 85, 247, 255}
        });
    }
    if (runtime.impact_recorded) {
        keyframes.push_back(MissionKeyframe {
            .label = "Impact",
            .time_s = runtime.impact_time_s,
            .accent = Color {239, 68, 68, 255}
        });
    }
    return keyframes;
}

bool tryGetSelectedMissionKeyframe(const SimulationRuntime& runtime, MissionKeyframe& keyframe) {
    if (!runtime.keyframe_preview_active || runtime.keyframe_preview_index < 0) {
        return false;
    }

    const auto keyframes = buildMissionKeyframes(runtime);
    if (runtime.keyframe_preview_index >= static_cast<int>(keyframes.size())) {
        return false;
    }

    keyframe = keyframes[static_cast<std::size_t>(runtime.keyframe_preview_index)];
    return true;
}

void cycleMissionKeyframePreview(SimulationRuntime& runtime) {
    const auto keyframes = buildMissionKeyframes(runtime);
    if (keyframes.empty()) {
        runtime.keyframe_preview_active = false;
        runtime.keyframe_preview_index = -1;
        runtime.keyframe_preview_time_s = 0.0;
        return;
    }

    if (!runtime.keyframe_preview_active) {
        runtime.keyframe_preview_active = true;
        runtime.keyframe_preview_index = 0;
        runtime.keyframe_preview_time_s = keyframes.front().time_s;
        runtime.paused = true;
        return;
    }

    const int next_index = runtime.keyframe_preview_index + 1;
    if (next_index >= static_cast<int>(keyframes.size())) {
        runtime.keyframe_preview_active = false;
        runtime.keyframe_preview_index = -1;
        runtime.keyframe_preview_time_s = 0.0;
        return;
    }

    runtime.keyframe_preview_index = next_index;
    runtime.keyframe_preview_time_s = keyframes[static_cast<std::size_t>(next_index)].time_s;
    runtime.paused = true;
}

std::vector<rocket::MountedMotor> buildClusterLayout(const MotorEditorState& editor) {
    std::vector<rocket::MountedMotor> motors;
    const int count = std::max(editor.motor_count, 1);
    motors.reserve(static_cast<std::size_t>(count));

    for (int index = 0; index < count; ++index) {
        double angle_rad = 0.0;
        rocket::Vector3 mount_position {};
        if (count > 1) {
            angle_rad = (2.0 * pi * static_cast<double>(index)) / static_cast<double>(count);
            mount_position = {
                editor.mount_radius_m * std::cos(angle_rad),
                editor.mount_radius_m * std::sin(angle_rad),
                0.0
            };
        }

        const double cant_rad = editor.cant_angle_deg * pi / 180.0;
        const rocket::Vector3 radial =
            count > 1 ? rocket::Vector3 {std::cos(angle_rad), std::sin(angle_rad), 0.0} : rocket::Vector3 {1.0, 0.0, 0.0};
        const rocket::Vector3 thrust_direction =
            (std::cos(cant_rad) * rocket::Vector3 {0.0, 0.0, 1.0} + std::sin(cant_rad) * radial).normalized();

        motors.push_back(rocket::MountedMotor {
            .motor = rocket::Motor {
                .max_thrust_n = editor.max_thrust_n,
                .burn_time_s = editor.burn_time_s,
                .propellant_mass_kg = editor.propellant_mass_kg
            },
            .mount_position_m = mount_position,
            .thrust_direction_body = thrust_direction,
            .failed = false
        });
    }

    return motors;
}

void rebuildMotorCluster(
    rocket::VehicleModel& vehicle,
    const MotorEditorState& editor,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime) {
    vehicle.cluster = rocket::MotorCluster(buildClusterLayout(editor));
    vehicle.geometry.propellant_cg_from_nose_m =
        std::clamp(vehicle.geometry.body_length_m - vehicle.geometry.transition_length_m - 0.22, 0.4, vehicle.geometry.body_length_m - 0.05);
    rebuildVehicle(vehicle, mesh_generator, runtime);
}

void rebuildVehicle(
    rocket::VehicleModel& vehicle,
    rocket::MeshGenerator& mesh_generator,
    SimulationRuntime& runtime) {
    syncVehicleDerivedValues(vehicle);
    mesh_generator.rebuild(vehicle.geometry, vehicle.cluster);
    resetSimulationRuntime(vehicle, runtime);
}

rocket::Vector3 worldPointFromNoseStation(
    const rocket::VehicleModel& vehicle,
    const rocket::FlightState& state,
    double from_nose_m) {
    const rocket::Vector3 local_point {0.0, 0.0, (0.5 * vehicle.geometry.body_length_m) - from_nose_m};
    return state.position_m + rocket::rotateVector(state.attitude_body_to_world, local_point);
}

rocket::SimulationSnapshot buildSnapshot(
    const rocket::FlightState& state,
    const rocket::VehicleModel& vehicle,
    const rocket::ForceResult& force_result,
    const rocket::Environment& environment,
    const rocket::MeshGenerator& mesh_generator,
    double time_s,
    double max_altitude_m,
    int cfd_solver_particle_count = 0,
    int cfd_render_particle_count = 0) {
    const double altitude_m = std::max(state.position_m.z, 0.0);
    const double static_pressure_pa = environment.airPressurePa(altitude_m);
    const double air_density_kgpm3 = environment.airDensityKgPerM3(altitude_m);
    const double air_temperature_k = environment.airTemperatureK(altitude_m);
    const double speed_of_sound_mps = environment.speedOfSoundMps(altitude_m);
    return rocket::SimulationSnapshot {
        .time_s = time_s,
        .state = state,
        .max_altitude_m = max_altitude_m,
        .cg_from_nose_m = force_result.center_of_gravity_from_nose_m,
        .cp_from_nose_m = force_result.center_of_pressure_from_nose_m,
        .static_margin_calibers = force_result.static_margin_calibers,
        .angle_of_attack_deg = force_result.angle_of_attack_rad * (180.0 / pi),
        .dynamic_pressure_pa = force_result.dynamic_pressure_pa,
        .static_pressure_pa = static_pressure_pa,
        .total_pressure_pa = static_pressure_pa + std::max(force_result.dynamic_pressure_pa, 0.0),
        .air_density_kgpm3 = air_density_kgpm3,
        .air_temperature_k = air_temperature_k,
        .speed_of_sound_mps = speed_of_sound_mps,
        .mach_number = force_result.mach_number,
        .relative_air_speed_mps = force_result.relative_air_velocity_world_mps.magnitude(),
        .wind_speed_mps = force_result.wind_velocity_world_mps.magnitude(),
        .recommended_max_dynamic_pressure_pa = rocket::estimateRecommendedMaxDynamicPressurePa(vehicle.geometry),
        .dynamic_pressure_safety_factor =
            rocket::estimateDynamicPressureSafetyFactor(vehicle.geometry, force_result.dynamic_pressure_pa),
        .equivalent_structural_modulus_gpa =
            rocket::estimateStructuralMaterialAssessment(vehicle.geometry).equivalent_modulus_gpa,
        .equivalent_structural_density_kg_per_m3 =
            rocket::estimateStructuralMaterialAssessment(vehicle.geometry).equivalent_density_kg_per_m3,
        .shockwave_intensity = force_result.shockwave_intensity,
        .aeroelastic_response = force_result.aeroelastic_response,
        .cfd_solver_particle_count = cfd_solver_particle_count,
        .cfd_render_particle_count = cfd_render_particle_count,
        .parachute_deployed = force_result.parachute_deployed,
        .nose_shape_label = mesh_generator.noseConeShapeLabel(),
        .fin_shape_label = mesh_generator.finShapeLabel()
    };
}

void stepSimulationRuntime(
    SimulationRuntime& runtime,
    const rocket::VehicleModel& vehicle,
    const rocket::Environment& environment,
    double dt_s,
    float frame_time_s) {
    runtime.accumulator_s += frame_time_s;
    while (runtime.accumulator_s >= dt_s) {
        const bool was_burning = vehicle.cluster.isBurning(runtime.time_s);
        runtime.state = rocket::integrateRk4(
            runtime.state,
            vehicle,
            environment,
            runtime.time_s,
            dt_s);
        runtime.time_s += dt_s;
        runtime.accumulator_s -= dt_s;
        runtime.max_altitude_m = std::max(runtime.max_altitude_m, runtime.state.position_m.z);
        if (!runtime.apogee_recorded || runtime.state.position_m.z >= runtime.apogee_point_m.z) {
            runtime.apogee_recorded = true;
            runtime.apogee_point_m = runtime.state.position_m;
            runtime.apogee_time_s = runtime.time_s;
        }

        if (runtime.trajectory_history.empty() ||
            (runtime.state.position_m - runtime.trajectory_history.back().state.position_m).magnitude() > 0.2) {
            runtime.trajectory_history.push_back(TrajectorySample {
                .state = runtime.state,
                .time_s = runtime.time_s
            });
        }
        if (runtime.trajectory_history.size() > 2500) {
            runtime.trajectory_history.pop_front();
        }

        if (!runtime.burnout_recorded && was_burning && !vehicle.cluster.isBurning(runtime.time_s)) {
            runtime.burnout_recorded = true;
            runtime.burnout_point_m = runtime.state.position_m;
            runtime.burnout_time_s = runtime.time_s;
        }
        if (!runtime.impact_recorded && runtime.time_s > 0.5 && runtime.state.position_m.z <= 0.0 && runtime.state.velocity_mps.z < 0.0) {
            runtime.impact_recorded = true;
            runtime.impact_point_m = runtime.state.position_m;
            runtime.impact_time_s = runtime.time_s;
            runtime.replay_active = true;
            runtime.replay_time_s = 0.0;
        }
    }
}
