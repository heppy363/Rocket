#pragma once

#include <vector>
#include "rocket/Propulsion.hpp"

namespace rocket {

enum class NoseConeShape {
    Conical,
    TangentOgive,
    Parabolic,
    LdHaack
};

enum class FinShape {
    Trapezoidal,
    Elliptical,
    Airfoil
};

enum class TransitionShape {
    Conical,
    Curved
};

enum class ComponentMaterial {
    PlaCf,
    Aluminum6061,
    Pvc,
    Fiberglass,
    CarbonFiber,
    BirchPlywood,
    Phenolic
};

enum class RocketPreset {
    ResearchStarter,
    SportTrainer,
    HighAltitude,
    MinimumDiameter,
    HeavyLift
};

enum class ComponentType {
    NoseCone,
    BodyTube,
    Transition,
    FinSet,
    MotorMount,
    Payload
};

// Struttura per un vertice di controllo libero in coordinate locali al componente
struct FreeControlVertex {
    int vertex_id {};           // Indice univoco del vertice nella mesh procedurale
    Vector3 base_position_m {}; // Posizione locale originale del vertice
    Vector3 offset_m {};        // Offset dalla posizione procedurale originale
    double influence_radius_m {0.02};  // Raggio di influenza per smooth deformation
    bool locked {false};        // Se true, il vertice è pinned e non verrà modificato
};

// Tracciamento delle modifiche libere per componente
struct ComponentVertexModifiers {
    ComponentType component_type {};
    std::vector<FreeControlVertex> modified_vertices {};
    bool is_active {false};    // Se true, le modifiche sono abilitate per questo componente
};

struct NoseControlVertices {
    double mid_radius_scale {1.0};
    double shoulder_radius_scale {1.0};
};

struct BodyControlVertices {
    double fore_radius_scale {1.0};
    double mid_radius_scale {1.0};
    double aft_radius_scale {1.0};
};

struct TransitionControlVertices {
    double mid_radius_scale {1.0};
};

struct FinControlVertices {
    double tip_le_offset_m {};
    double tip_te_offset_m {};
    double span_scale {1.0};
    double thickness_scale {1.0};
};

struct VehicleGeometry {
    double body_length_m {};
    double body_diameter_m {};
    double wall_thickness_m {0.003};
    double nose_length_m {};
    NoseConeShape nose_cone_shape {NoseConeShape::Conical};
    ComponentMaterial nose_material {ComponentMaterial::PlaCf};

    double transition_length_m {};
    double transition_aft_diameter_m {};
    TransitionShape transition_shape {TransitionShape::Conical};
    ComponentMaterial transition_material {ComponentMaterial::Aluminum6061};

    ComponentMaterial body_material {ComponentMaterial::Aluminum6061};

    double fin_front_from_nose_m {};
    double fin_root_chord_m {};
    double fin_tip_chord_m {};
    double fin_span_m {};
    double fin_sweep_length_m {};
    double fin_thickness_m {};
    FinShape fin_shape {FinShape::Trapezoidal};
    ComponentMaterial fin_material {ComponentMaterial::PlaCf};
    int fin_count {};

    double payload_length_m {0.22};
    double payload_mass_kg {0.9};
    ComponentMaterial payload_material {ComponentMaterial::Pvc};

    NoseControlVertices nose_controls {};
    BodyControlVertices body_controls {};
    TransitionControlVertices transition_controls {};
    FinControlVertices fin_controls {};

    double structure_cg_from_nose_m {};
    double propellant_cg_from_nose_m {};

    // Tracciamento delle modifiche libere per componente
    ComponentVertexModifiers nose_vertex_mods {};
    ComponentVertexModifiers body_vertex_mods {};
    ComponentVertexModifiers transition_vertex_mods {};
    ComponentVertexModifiers fin_vertex_mods {};
    ComponentVertexModifiers motor_vertex_mods {};
    ComponentVertexModifiers payload_vertex_mods {};

    // Metodo utility per accedere ai modifiers del componente attivo
    ComponentVertexModifiers* getActiveComponentModifiers(ComponentType component) {
        switch (component) {
        case ComponentType::NoseCone:
            return &nose_vertex_mods;
        case ComponentType::BodyTube:
            return &body_vertex_mods;
        case ComponentType::Transition:
            return &transition_vertex_mods;
        case ComponentType::FinSet:
            return &fin_vertex_mods;
        case ComponentType::MotorMount:
            return &motor_vertex_mods;
        case ComponentType::Payload:
            return &payload_vertex_mods;
        }
        return nullptr;
    }

    // Versione const della stessa funzione
    const ComponentVertexModifiers* getActiveComponentModifiers(ComponentType component) const {
        switch (component) {
        case ComponentType::NoseCone:
            return &nose_vertex_mods;
        case ComponentType::BodyTube:
            return &body_vertex_mods;
        case ComponentType::Transition:
            return &transition_vertex_mods;
        case ComponentType::FinSet:
            return &fin_vertex_mods;
        case ComponentType::MotorMount:
            return &motor_vertex_mods;
        case ComponentType::Payload:
            return &payload_vertex_mods;
        }
        return nullptr;
    }
};

struct AerodynamicCoefficients {
    double drag_coefficient {};
    double normal_force_slope_per_rad {};
    double rotational_damping_coefficient {};
};

struct RecoverySystem {
    double parachute_drag_coefficient {1.55};
    double parachute_area_m2 {0.85};
    double deployment_altitude_m {250.0};
    double deployment_delay_s {1.2};
};

struct VehicleModel {
    double dry_mass_kg {};
    double reference_area_m2 {};
    Vector3 principal_inertia_kgm2 {};
    VehicleGeometry geometry {};
    AerodynamicCoefficients aerodynamic_coefficients {};
    RecoverySystem recovery_system {};
    MotorCluster cluster;
};

}  // namespace rocket
