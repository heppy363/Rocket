#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "rocket/CfdModule.hpp"
#include "rocket/FlightState.hpp"
#include "rocket/Propulsion.hpp"
#include "rocket/VehicleModel.hpp"

struct Material;
struct Mesh;
struct Matrix;
struct Vector3;

namespace rocket {

struct MeshVertexData {
    Vector3 position_body_m {};
    Vector3 normal_body {};
    double u {};
    double v {};
};

struct IndexedMeshData {
    std::vector<MeshVertexData> vertices {};
    std::vector<unsigned int> indices {};
};

struct MeshEdgeData {
    unsigned int vertex_a {};
    unsigned int vertex_b {};
    int face_a {-1};
    int face_b {-1};
    Vector3 center_body_m {};
};

struct MeshFaceData {
    unsigned int vertex_a {};
    unsigned int vertex_b {};
    unsigned int vertex_c {};
    Vector3 center_body_m {};
    Vector3 normal_body {};
};

class MeshGenerator {
public:
    MeshGenerator(const VehicleGeometry& geometry, const MotorCluster& cluster);
    ~MeshGenerator();

    MeshGenerator(const MeshGenerator&) = delete;
    MeshGenerator& operator=(const MeshGenerator&) = delete;
    MeshGenerator(MeshGenerator&&) = delete;
    MeshGenerator& operator=(MeshGenerator&&) = delete;

    void rebuild(const VehicleGeometry& geometry, const MotorCluster& cluster);
    void draw(const FlightState& state) const;
    void drawWireframe(const FlightState& state) const;
    void setPressureOverlay(
        const std::array<double, static_cast<std::size_t>(CfdComponentBand::Count)>& component_pressure_pa,
        double reference_pressure_pa,
        bool enabled) noexcept;

    [[nodiscard]] const IndexedMeshData* componentMesh(ComponentType component) const noexcept;
    [[nodiscard]] const std::vector<MeshEdgeData>* componentEdges(ComponentType component) const noexcept;
    [[nodiscard]] const std::vector<MeshFaceData>* componentFaces(ComponentType component) const noexcept;
    [[nodiscard]] std::size_t componentVertexCount(ComponentType component) const noexcept;
    [[nodiscard]] bool componentVertexPosition(ComponentType component, int vertex_id, Vector3& out_position_body_m) const noexcept;
    [[nodiscard]] bool setComponentVertexPosition(ComponentType component, int vertex_id, const Vector3& position_body_m);
    [[nodiscard]] bool extrudeComponentFace(ComponentType component, int face_id, double distance_m);
    [[nodiscard]] bool bevelComponentFace(ComponentType component, int face_id, double inset_ratio, double normal_offset_m);
    [[nodiscard]] bool loopCutComponentEdge(ComponentType component, int edge_id);

    [[nodiscard]] NoseConeShape noseConeShape() const noexcept;
    [[nodiscard]] std::string noseConeShapeLabel() const;
    [[nodiscard]] std::string finShapeLabel() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace rocket
