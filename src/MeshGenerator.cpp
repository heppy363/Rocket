#include "rocket/MeshGenerator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "raylib.h"
#include "raymath.h"
#include "rocket/DesignLibrary.hpp"

namespace rocket {

namespace {

constexpr double pi = 3.14159265358979323846;

struct TriangleVertex {
    Vector3 body_position {};
    double u {};
    double v {};
};

struct MeshBuffers {
    IndexedMeshData topology {};
    std::unordered_map<std::string, unsigned int> position_lookup {};
};

Vector3 bodyPosition(double x, double y, double z) {
    return {x, y, z};
}

::Vector3 toRaylibLocal(const Vector3& position) {
    return {
        static_cast<float>(position.x),
        static_cast<float>(position.z),
        static_cast<float>(position.y)
    };
}

Vector3 fromRaylibLocal(const ::Vector3& position) {
    return {
        static_cast<double>(position.x),
        static_cast<double>(position.z),
        static_cast<double>(position.y)
    };
}

::Vector3 toRaylibQuaternionVector(const rocket::Quaternion& quaternion) {
    return {
        static_cast<float>(quaternion.x),
        static_cast<float>(quaternion.z),
        static_cast<float>(quaternion.y)
    };
}

::Quaternion toRaylibQuaternion(const rocket::Quaternion& quaternion) {
    return {
        static_cast<float>(quaternion.x),
        static_cast<float>(quaternion.z),
        static_cast<float>(quaternion.y),
        static_cast<float>(quaternion.w)
    };
}

::Matrix bodyToWorldTransform(const FlightState& state) {
    const ::Matrix rotation = QuaternionToMatrix(toRaylibQuaternion(state.attitude_body_to_world));
    const ::Matrix translation = MatrixTranslate(
        static_cast<float>(state.position_m.x),
        static_cast<float>(state.position_m.z),
        static_cast<float>(state.position_m.y));
    return MatrixMultiply(rotation, translation);
}

void drawIndexedMeshWireOverlay(const IndexedMeshData& topology, const FlightState& state, Color color) {
    for (std::size_t triangle = 0; triangle + 2 < topology.indices.size(); triangle += 3) {
        const Vector3 a_body = topology.vertices[topology.indices[triangle]].position_body_m;
        const Vector3 b_body = topology.vertices[topology.indices[triangle + 1]].position_body_m;
        const Vector3 c_body = topology.vertices[topology.indices[triangle + 2]].position_body_m;
        const Vector3 a_world = state.position_m + rotateVector(state.attitude_body_to_world, a_body);
        const Vector3 b_world = state.position_m + rotateVector(state.attitude_body_to_world, b_body);
        const Vector3 c_world = state.position_m + rotateVector(state.attitude_body_to_world, c_body);
        DrawLine3D(toRaylibLocal(a_world), toRaylibLocal(b_world), color);
        DrawLine3D(toRaylibLocal(b_world), toRaylibLocal(c_world), color);
        DrawLine3D(toRaylibLocal(c_world), toRaylibLocal(a_world), color);
    }
}

Color materialBaseColor(ComponentMaterial material) noexcept {
    switch (material) {
    case ComponentMaterial::PlaCf:
        return Color {255, 176, 64, 255};
    case ComponentMaterial::Aluminum6061:
        return Color {214, 222, 233, 255};
    case ComponentMaterial::Pvc:
        return Color {88, 176, 255, 255};
    case ComponentMaterial::Fiberglass:
        return Color {166, 204, 150, 255};
    case ComponentMaterial::CarbonFiber:
        return Color {72, 78, 92, 255};
    case ComponentMaterial::BirchPlywood:
        return Color {205, 164, 104, 255};
    case ComponentMaterial::Phenolic:
        return Color {163, 99, 72, 255};
    }
    return Color {200, 200, 200, 255};
}

Color lerpColor(Color a, Color b, float t) noexcept {
    const float clamped_t = std::clamp(t, 0.0f, 1.0f);
    const auto blend = [clamped_t](unsigned char lhs, unsigned char rhs) {
        return static_cast<unsigned char>(
            static_cast<float>(lhs) + (static_cast<float>(rhs) - static_cast<float>(lhs)) * clamped_t);
    };
    return {blend(a.r, b.r), blend(a.g, b.g), blend(a.b, b.b), blend(a.a, b.a)};
}

Color pressureHeatColor(float normalized_pressure) noexcept {
    const float clamped = std::clamp(normalized_pressure, 0.0f, 1.0f);
    if (clamped < 0.33f) {
        return lerpColor(Color {56, 189, 248, 255}, Color {34, 197, 94, 255}, clamped / 0.33f);
    }
    if (clamped < 0.66f) {
        return lerpColor(Color {34, 197, 94, 255}, Color {251, 191, 36, 255}, (clamped - 0.33f) / 0.33f);
    }
    return lerpColor(Color {251, 191, 36, 255}, Color {239, 68, 68, 255}, (clamped - 0.66f) / 0.34f);
}

double blend3(double t, double a, double b, double c) noexcept {
    const double clamped_t = std::clamp(t, 0.0, 1.0);
    if (clamped_t < 0.5) {
        const double local = clamped_t * 2.0;
        return a + (b - a) * local;
    }
    const double local = (clamped_t - 0.5) * 2.0;
    return b + (c - b) * local;
}

double bodyRadiusScaleAt(const VehicleGeometry& geometry, double t) noexcept {
    return std::max(
        0.45,
        blend3(
            t,
            geometry.body_controls.fore_radius_scale,
            geometry.body_controls.mid_radius_scale,
            geometry.body_controls.aft_radius_scale));
}

double noseRadiusScaleAt(const VehicleGeometry& geometry, double t) noexcept {
    return std::max(
        0.35,
        blend3(t, 1.0, geometry.nose_controls.mid_radius_scale, geometry.nose_controls.shoulder_radius_scale));
}

double transitionRadiusScaleAt(const VehicleGeometry& geometry, double t) noexcept {
    return std::max(0.35, blend3(t, 1.0, geometry.transition_controls.mid_radius_scale, 1.0));
}

double nacaThicknessFraction(double x) noexcept {
    const double xt = std::clamp(x, 0.0, 1.0);
    return 5.0 * (0.2969 * std::sqrt(xt) - 0.1260 * xt - 0.3516 * xt * xt +
                  0.2843 * xt * xt * xt - 0.1015 * xt * xt * xt * xt);
}

std::string quantizedKey(const Vector3& position) {
    return std::to_string(static_cast<long long>(std::llround(position.x * 100000.0))) + "|" +
           std::to_string(static_cast<long long>(std::llround(position.y * 100000.0))) + "|" +
           std::to_string(static_cast<long long>(std::llround(position.z * 100000.0)));
}

unsigned int appendVertex(MeshBuffers& buffers, const TriangleVertex& vertex) {
    const std::string key = quantizedKey(vertex.body_position);
    if (const auto found = buffers.position_lookup.find(key); found != buffers.position_lookup.end()) {
        return found->second;
    }

    const unsigned int index = static_cast<unsigned int>(buffers.topology.vertices.size());
    buffers.topology.vertices.push_back(MeshVertexData {
        .position_body_m = vertex.body_position,
        .normal_body = {},
        .u = vertex.u,
        .v = vertex.v
    });
    buffers.position_lookup.emplace(key, index);
    return index;
}

void pushTriangle(MeshBuffers& buffers, const TriangleVertex& a, const TriangleVertex& b, const TriangleVertex& c) {
    buffers.topology.indices.push_back(appendVertex(buffers, a));
    buffers.topology.indices.push_back(appendVertex(buffers, b));
    buffers.topology.indices.push_back(appendVertex(buffers, c));
}

void appendQuad(
    MeshBuffers& buffers,
    const Vector3& a,
    const Vector3& b,
    const Vector3& c,
    const Vector3& d,
    double u0 = 0.0,
    double u1 = 1.0,
    double v0 = 0.0,
    double v1 = 1.0) {
    pushTriangle(
        buffers,
        TriangleVertex {.body_position = a, .u = u0, .v = v0},
        TriangleVertex {.body_position = b, .u = u1, .v = v0},
        TriangleVertex {.body_position = c, .u = u1, .v = v1});
    pushTriangle(
        buffers,
        TriangleVertex {.body_position = a, .u = u0, .v = v0},
        TriangleVertex {.body_position = c, .u = u1, .v = v1},
        TriangleVertex {.body_position = d, .u = u0, .v = v1});
}

void appendDisc(MeshBuffers& buffers, double z_m, double radius_m, bool forward, int segments) {
    if (radius_m <= 1e-6) {
        return;
    }
    const Vector3 center = bodyPosition(0.0, 0.0, z_m);
    for (int segment = 0; segment < segments; ++segment) {
        const double a0 = (2.0 * pi * static_cast<double>(segment)) / static_cast<double>(segments);
        const double a1 = (2.0 * pi * static_cast<double>(segment + 1)) / static_cast<double>(segments);
        const Vector3 p0 = bodyPosition(radius_m * std::cos(a0), radius_m * std::sin(a0), z_m);
        const Vector3 p1 = bodyPosition(radius_m * std::cos(a1), radius_m * std::sin(a1), z_m);
        if (forward) {
            pushTriangle(buffers, {center, 0.5, 0.5}, {p0, 0.0, 0.0}, {p1, 1.0, 0.0});
        } else {
            pushTriangle(buffers, {center, 0.5, 0.5}, {p1, 1.0, 0.0}, {p0, 0.0, 0.0});
        }
    }
}

void appendRevolvedBand(
    MeshBuffers& buffers,
    double z0_m,
    double radius0_m,
    double z1_m,
    double radius1_m,
    int segments) {
    for (int segment = 0; segment < segments; ++segment) {
        const double a0 = (2.0 * pi * static_cast<double>(segment)) / static_cast<double>(segments);
        const double a1 = (2.0 * pi * static_cast<double>(segment + 1)) / static_cast<double>(segments);
        appendQuad(
            buffers,
            bodyPosition(radius0_m * std::cos(a0), radius0_m * std::sin(a0), z0_m),
            bodyPosition(radius1_m * std::cos(a0), radius1_m * std::sin(a0), z1_m),
            bodyPosition(radius1_m * std::cos(a1), radius1_m * std::sin(a1), z1_m),
            bodyPosition(radius0_m * std::cos(a1), radius0_m * std::sin(a1), z0_m),
            static_cast<double>(segment) / static_cast<double>(segments),
            static_cast<double>(segment + 1) / static_cast<double>(segments));
    }
}

void recomputeNormals(IndexedMeshData& topology) {
    for (auto& vertex : topology.vertices) {
        vertex.normal_body = {};
    }
    for (std::size_t triangle = 0; triangle + 2 < topology.indices.size(); triangle += 3) {
        const unsigned int ia = topology.indices[triangle];
        const unsigned int ib = topology.indices[triangle + 1];
        const unsigned int ic = topology.indices[triangle + 2];
        const ::Vector3 a = toRaylibLocal(topology.vertices[ia].position_body_m);
        const ::Vector3 b = toRaylibLocal(topology.vertices[ib].position_body_m);
        const ::Vector3 c = toRaylibLocal(topology.vertices[ic].position_body_m);
        const Vector3 normal = fromRaylibLocal(Vector3Normalize(Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a))));
        topology.vertices[ia].normal_body += normal;
        topology.vertices[ib].normal_body += normal;
        topology.vertices[ic].normal_body += normal;
    }
    for (auto& vertex : topology.vertices) {
        vertex.normal_body = vertex.normal_body.normalized();
    }
}

std::vector<MeshFaceData> buildFaceCache(const IndexedMeshData& topology) {
    std::vector<MeshFaceData> faces;
    faces.reserve(topology.indices.size() / 3);
    for (std::size_t triangle = 0; triangle + 2 < topology.indices.size(); triangle += 3) {
        const unsigned int ia = topology.indices[triangle];
        const unsigned int ib = topology.indices[triangle + 1];
        const unsigned int ic = topology.indices[triangle + 2];
        if (ia >= topology.vertices.size() || ib >= topology.vertices.size() || ic >= topology.vertices.size()) {
            continue;
        }

        const Vector3 a = topology.vertices[ia].position_body_m;
        const Vector3 b = topology.vertices[ib].position_body_m;
        const Vector3 c = topology.vertices[ic].position_body_m;
        faces.push_back(MeshFaceData {
            .vertex_a = ia,
            .vertex_b = ib,
            .vertex_c = ic,
            .center_body_m = (a + b + c) / 3.0,
            .normal_body = cross(b - a, c - a).normalized()
        });
    }
    return faces;
}

std::uint64_t edgeKey(unsigned int a, unsigned int b) {
    const auto [lo, hi] = std::minmax(a, b);
    return (static_cast<std::uint64_t>(lo) << 32U) | static_cast<std::uint64_t>(hi);
}

std::vector<MeshEdgeData> buildEdgeCache(
    const IndexedMeshData& topology,
    const std::vector<MeshFaceData>& faces) {
    std::vector<MeshEdgeData> edges;
    std::unordered_map<std::uint64_t, std::size_t> lookup;
    lookup.reserve(faces.size() * 3);

    const auto register_edge = [&](unsigned int a, unsigned int b, int face_id) {
        const std::uint64_t key = edgeKey(a, b);
        const auto found = lookup.find(key);
        if (found == lookup.end()) {
            const auto [lo, hi] = std::minmax(a, b);
            const Vector3 center =
                (topology.vertices[lo].position_body_m + topology.vertices[hi].position_body_m) * 0.5;
            lookup.emplace(key, edges.size());
            edges.push_back(MeshEdgeData {
                .vertex_a = lo,
                .vertex_b = hi,
                .face_a = face_id,
                .face_b = -1,
                .center_body_m = center
            });
            return;
        }
        auto& edge = edges[found->second];
        if (edge.face_a < 0) {
            edge.face_a = face_id;
        } else if (edge.face_b < 0) {
            edge.face_b = face_id;
        }
    };

    for (int face_id = 0; face_id < static_cast<int>(faces.size()); ++face_id) {
        const auto& face = faces[static_cast<std::size_t>(face_id)];
        register_edge(face.vertex_a, face.vertex_b, face_id);
        register_edge(face.vertex_b, face.vertex_c, face_id);
        register_edge(face.vertex_c, face.vertex_a, face_id);
    }

    return edges;
}

::Mesh uploadMesh(const IndexedMeshData& topology) {
    ::Mesh mesh {};
    mesh.vertexCount = static_cast<int>(topology.vertices.size());
    mesh.triangleCount = static_cast<int>(topology.indices.size() / 3);
    mesh.vertices = static_cast<float*>(MemAlloc(sizeof(float) * topology.vertices.size() * 3));
    mesh.normals = static_cast<float*>(MemAlloc(sizeof(float) * topology.vertices.size() * 3));
    mesh.texcoords = static_cast<float*>(MemAlloc(sizeof(float) * topology.vertices.size() * 2));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(sizeof(unsigned short) * topology.indices.size()));

    for (std::size_t index = 0; index < topology.vertices.size(); ++index) {
        const auto& vertex = topology.vertices[index];
        const ::Vector3 position = toRaylibLocal(vertex.position_body_m);
        const ::Vector3 normal = toRaylibLocal(vertex.normal_body);
        mesh.vertices[index * 3] = position.x;
        mesh.vertices[index * 3 + 1] = position.y;
        mesh.vertices[index * 3 + 2] = position.z;
        mesh.normals[index * 3] = normal.x;
        mesh.normals[index * 3 + 1] = normal.y;
        mesh.normals[index * 3 + 2] = normal.z;
        mesh.texcoords[index * 2] = static_cast<float>(vertex.u);
        mesh.texcoords[index * 2 + 1] = static_cast<float>(vertex.v);
    }
    for (std::size_t index = 0; index < topology.indices.size(); ++index) {
        mesh.indices[index] = static_cast<unsigned short>(topology.indices[index]);
    }

    UploadMesh(&mesh, false);
    return mesh;
}

void syncGpuMeshVertexArrays(::Mesh& mesh, const IndexedMeshData& topology) {
    if (mesh.vertices == nullptr || mesh.normals == nullptr) {
        return;
    }

    for (std::size_t index = 0; index < topology.vertices.size(); ++index) {
        const auto& vertex = topology.vertices[index];
        const ::Vector3 position = toRaylibLocal(vertex.position_body_m);
        const ::Vector3 normal = toRaylibLocal(vertex.normal_body);
        mesh.vertices[index * 3] = position.x;
        mesh.vertices[index * 3 + 1] = position.y;
        mesh.vertices[index * 3 + 2] = position.z;
        mesh.normals[index * 3] = normal.x;
        mesh.normals[index * 3 + 1] = normal.y;
        mesh.normals[index * 3 + 2] = normal.z;
    }
}

float noseRadiusAt(
    double distance_from_base_m,
    double base_radius_m,
    double length_m,
    NoseConeShape shape,
    const NoseControlVertices& controls) {
    if (length_m <= 1e-9) {
        return 0.0f;
    }

    const double x = std::clamp(distance_from_base_m, 0.0, length_m);
    switch (shape) {
    case NoseConeShape::Conical:
        return static_cast<float>(base_radius_m * (1.0 - x / length_m) * noseRadiusScaleAt(VehicleGeometry {.nose_controls = controls}, x / length_m));
    case NoseConeShape::Parabolic: {
        const double xi = x / length_m;
        return static_cast<float>(base_radius_m * (1.0 - xi * xi) * noseRadiusScaleAt(VehicleGeometry {.nose_controls = controls}, xi));
    }
    case NoseConeShape::TangentOgive: {
        const double rho = (base_radius_m * base_radius_m + length_m * length_m) / (2.0 * base_radius_m);
        const double y = std::sqrt(std::max(0.0, rho * rho - (length_m - x) * (length_m - x))) + base_radius_m - rho;
        return static_cast<float>(y * noseRadiusScaleAt(VehicleGeometry {.nose_controls = controls}, x / length_m));
    }
    case NoseConeShape::LdHaack: {
        const double s = length_m - x;
        const double theta = std::acos(std::clamp(1.0 - (2.0 * s / length_m), -1.0, 1.0));
        const double area_term = std::max(0.0, (theta - std::sin(2.0 * theta) / 2.0) / pi);
        return static_cast<float>(base_radius_m * std::sqrt(area_term) * noseRadiusScaleAt(VehicleGeometry {.nose_controls = controls}, x / length_m));
    }
    }
    return 0.0f;
}

IndexedMeshData buildBodyTopology(const VehicleGeometry& geometry) {
    MeshBuffers buffers;
    const double radius_m = geometry.body_diameter_m * 0.5;
    const double body_start_z_m = -0.5 * geometry.body_length_m;
    const double body_end_z_m = 0.5 * geometry.body_length_m - geometry.nose_length_m - geometry.transition_length_m;
    constexpr int axial_segments = 10;
    constexpr int radial_segments = 28;

    for (int axial = 0; axial < axial_segments; ++axial) {
        const double t0 = static_cast<double>(axial) / static_cast<double>(axial_segments);
        const double t1 = static_cast<double>(axial + 1) / static_cast<double>(axial_segments);
        const double z0 = body_start_z_m + (body_end_z_m - body_start_z_m) * t0;
        const double z1 = body_start_z_m + (body_end_z_m - body_start_z_m) * t1;
        const double r0 = radius_m * bodyRadiusScaleAt(geometry, t0);
        const double r1 = radius_m * bodyRadiusScaleAt(geometry, t1);
        for (int segment = 0; segment < radial_segments; ++segment) {
            const double a0 = (2.0 * pi * static_cast<double>(segment)) / static_cast<double>(radial_segments);
            const double a1 = (2.0 * pi * static_cast<double>(segment + 1)) / static_cast<double>(radial_segments);
            appendQuad(
                buffers,
                bodyPosition(r0 * std::cos(a0), r0 * std::sin(a0), z0),
                bodyPosition(r1 * std::cos(a0), r1 * std::sin(a0), z1),
                bodyPosition(r1 * std::cos(a1), r1 * std::sin(a1), z1),
                bodyPosition(r0 * std::cos(a1), r0 * std::sin(a1), z0),
                static_cast<double>(segment) / static_cast<double>(radial_segments),
                static_cast<double>(segment + 1) / static_cast<double>(radial_segments),
                t0,
                t1);
        }
    }

    recomputeNormals(buffers.topology);
    return std::move(buffers.topology);
}

IndexedMeshData buildNoseTopology(const VehicleGeometry& geometry) {
    MeshBuffers buffers;
    const double radius_m = geometry.body_diameter_m * 0.5;
    const double body_end_z_m = 0.5 * geometry.body_length_m - geometry.nose_length_m;
    constexpr int axial_segments = 26;
    constexpr int radial_segments = 28;

    for (int axial = 0; axial < axial_segments; ++axial) {
        const double x0 = (static_cast<double>(axial) / static_cast<double>(axial_segments)) * geometry.nose_length_m;
        const double x1 = (static_cast<double>(axial + 1) / static_cast<double>(axial_segments)) * geometry.nose_length_m;
        const double z0 = body_end_z_m + x0;
        const double z1 = body_end_z_m + x1;
        const double r0 = noseRadiusAt(x0, radius_m, geometry.nose_length_m, geometry.nose_cone_shape, geometry.nose_controls);
        const double r1 = noseRadiusAt(x1, radius_m, geometry.nose_length_m, geometry.nose_cone_shape, geometry.nose_controls);

        for (int radial = 0; radial < radial_segments; ++radial) {
            const double a0 = (2.0 * pi * static_cast<double>(radial)) / static_cast<double>(radial_segments);
            const double a1 = (2.0 * pi * static_cast<double>(radial + 1)) / static_cast<double>(radial_segments);
            appendQuad(
                buffers,
                bodyPosition(r0 * std::cos(a0), r0 * std::sin(a0), z0),
                bodyPosition(r1 * std::cos(a0), r1 * std::sin(a0), z1),
                bodyPosition(r1 * std::cos(a1), r1 * std::sin(a1), z1),
                bodyPosition(r0 * std::cos(a1), r0 * std::sin(a1), z0));
        }
    }

    recomputeNormals(buffers.topology);
    return std::move(buffers.topology);
}

IndexedMeshData buildTransitionTopology(const VehicleGeometry& geometry) {
    MeshBuffers buffers;
    if (geometry.transition_length_m <= 1e-6) {
        return {};
    }

    constexpr int axial_segments = 14;
    constexpr int radial_segments = 28;
    const double body_radius_m = geometry.body_diameter_m * 0.5 * bodyRadiusScaleAt(geometry, 1.0);
    const double aft_radius_m = std::max(geometry.transition_aft_diameter_m * 0.5, 0.01);
    const double z0 = -0.5 * geometry.body_length_m;
    const double z1 = z0 + geometry.transition_length_m;

    for (int axial = 0; axial < axial_segments; ++axial) {
        const double t0 = static_cast<double>(axial) / static_cast<double>(axial_segments);
        const double t1 = static_cast<double>(axial + 1) / static_cast<double>(axial_segments);
        const double blend0 = geometry.transition_shape == TransitionShape::Curved ? std::sin(t0 * pi * 0.5) : t0;
        const double blend1 = geometry.transition_shape == TransitionShape::Curved ? std::sin(t1 * pi * 0.5) : t1;
        const double radius0 = (body_radius_m + (aft_radius_m - body_radius_m) * blend0) * transitionRadiusScaleAt(geometry, t0);
        const double radius1 = (body_radius_m + (aft_radius_m - body_radius_m) * blend1) * transitionRadiusScaleAt(geometry, t1);
        const double ring_z0 = z0 + (z1 - z0) * t0;
        const double ring_z1 = z0 + (z1 - z0) * t1;

        for (int radial = 0; radial < radial_segments; ++radial) {
            const double a0 = (2.0 * pi * static_cast<double>(radial)) / static_cast<double>(radial_segments);
            const double a1 = (2.0 * pi * static_cast<double>(radial + 1)) / static_cast<double>(radial_segments);
            appendQuad(
                buffers,
                bodyPosition(radius0 * std::cos(a0), radius0 * std::sin(a0), ring_z0),
                bodyPosition(radius1 * std::cos(a0), radius1 * std::sin(a0), ring_z1),
                bodyPosition(radius1 * std::cos(a1), radius1 * std::sin(a1), ring_z1),
                bodyPosition(radius0 * std::cos(a1), radius0 * std::sin(a1), ring_z0));
        }
    }

    recomputeNormals(buffers.topology);
    return std::move(buffers.topology);
}

IndexedMeshData buildPayloadTopology(const VehicleGeometry& geometry) {
    MeshBuffers buffers;
    constexpr int radial_segments = 28;
    const double body_radius_m = geometry.body_diameter_m * 0.5;
    const double nose_base_z_m = 0.5 * geometry.body_length_m - geometry.nose_length_m;
    const double payload_aft_z_m = nose_base_z_m - geometry.payload_length_m;
    const double body_scale = bodyRadiusScaleAt(geometry, 0.08);
    const double radial_offset_m = std::clamp(std::max(geometry.wall_thickness_m * 0.4, 0.0015), 0.0015, body_radius_m * 0.04);
    const double payload_radius_m = std::max(body_radius_m * body_scale + radial_offset_m, body_radius_m * 0.55);

    appendRevolvedBand(buffers, payload_aft_z_m, payload_radius_m, nose_base_z_m, payload_radius_m, radial_segments);
    appendDisc(buffers, payload_aft_z_m, payload_radius_m, false, radial_segments);
    appendDisc(buffers, nose_base_z_m, payload_radius_m, true, radial_segments);
    recomputeNormals(buffers.topology);
    return std::move(buffers.topology);
}

IndexedMeshData buildMotorTopology(const VehicleGeometry& geometry, const MotorCluster& cluster) {
    MeshBuffers buffers;
    constexpr int radial_segments = 20;

    const double body_radius_m = geometry.body_diameter_m * 0.5;
    const double aft_z_m = -0.5 * geometry.body_length_m;
    const double casing_length_m = std::clamp(geometry.body_diameter_m * 0.9, 0.06, 0.18);
    const double nozzle_length_m = std::clamp(geometry.body_diameter_m * 0.45, 0.03, 0.09);
    const auto& motors = cluster.mountedMotors();
    if (motors.empty()) {
        return {};
    }

    double furthest_mount_m = 0.0;
    for (const auto& mounted_motor : motors) {
        furthest_mount_m = std::max(furthest_mount_m, std::sqrt(mounted_motor.mount_position_m.x * mounted_motor.mount_position_m.x + mounted_motor.mount_position_m.y * mounted_motor.mount_position_m.y));
    }

    const double available_radius_m = std::clamp(body_radius_m - furthest_mount_m, body_radius_m * 0.08, body_radius_m * 0.28);
    const double motor_radius_m = std::clamp(available_radius_m * 0.75, 0.008, body_radius_m * 0.22);
    const double throat_radius_m = std::max(motor_radius_m * 0.55, 0.005);

    for (const auto& mounted_motor : motors) {
        const double x_m = mounted_motor.mount_position_m.x;
        const double y_m = mounted_motor.mount_position_m.y;
        const double casing_fore_z_m = aft_z_m + geometry.transition_length_m + 0.03;
        const double casing_aft_z_m = casing_fore_z_m - casing_length_m;
        const double nozzle_aft_z_m = casing_aft_z_m - nozzle_length_m;

        for (int segment = 0; segment < radial_segments; ++segment) {
            const double a0 = (2.0 * pi * static_cast<double>(segment)) / static_cast<double>(radial_segments);
            const double a1 = (2.0 * pi * static_cast<double>(segment + 1)) / static_cast<double>(radial_segments);
            appendQuad(
                buffers,
                bodyPosition(x_m + motor_radius_m * std::cos(a0), y_m + motor_radius_m * std::sin(a0), casing_aft_z_m),
                bodyPosition(x_m + motor_radius_m * std::cos(a0), y_m + motor_radius_m * std::sin(a0), casing_fore_z_m),
                bodyPosition(x_m + motor_radius_m * std::cos(a1), y_m + motor_radius_m * std::sin(a1), casing_fore_z_m),
                bodyPosition(x_m + motor_radius_m * std::cos(a1), y_m + motor_radius_m * std::sin(a1), casing_aft_z_m));
            appendQuad(
                buffers,
                bodyPosition(x_m + throat_radius_m * std::cos(a0), y_m + throat_radius_m * std::sin(a0), nozzle_aft_z_m),
                bodyPosition(x_m + motor_radius_m * std::cos(a0), y_m + motor_radius_m * std::sin(a0), casing_aft_z_m),
                bodyPosition(x_m + motor_radius_m * std::cos(a1), y_m + motor_radius_m * std::sin(a1), casing_aft_z_m),
                bodyPosition(x_m + throat_radius_m * std::cos(a1), y_m + throat_radius_m * std::sin(a1), nozzle_aft_z_m));
        }
    }

    recomputeNormals(buffers.topology);
    return std::move(buffers.topology);
}

IndexedMeshData buildFinTopology(const VehicleGeometry& geometry) {
    MeshBuffers buffers;
    const double radius_m = geometry.body_diameter_m * 0.5;
    const double thickness_m = geometry.fin_thickness_m > 0.0
                                   ? geometry.fin_thickness_m * geometry.fin_controls.thickness_scale
                                   : std::max(geometry.body_diameter_m * 0.045, 0.004);
    const double fin_front_z_m = -0.5 * geometry.body_length_m + geometry.fin_front_from_nose_m;
    const double fin_back_z_m = fin_front_z_m + geometry.fin_root_chord_m;
    const double fin_tip_front_z_m = fin_front_z_m + geometry.fin_sweep_length_m + geometry.fin_controls.tip_le_offset_m;
    const double fin_tip_back_z_m = fin_tip_front_z_m + geometry.fin_tip_chord_m + geometry.fin_controls.tip_te_offset_m;
    const double fin_outer_radius_m = radius_m + geometry.fin_span_m * geometry.fin_controls.span_scale;
    const int fin_count = std::max(geometry.fin_count, 1);

    for (int fin_index = 0; fin_index < fin_count; ++fin_index) {
        const double angle_rad = (2.0 * pi * static_cast<double>(fin_index)) / static_cast<double>(fin_count);
        const double cos_a = std::cos(angle_rad);
        const double sin_a = std::sin(angle_rad);
        const Vector3 radial = bodyPosition(cos_a, sin_a, 0.0);
        const Vector3 tangent = bodyPosition(-sin_a, cos_a, 0.0);

        auto makePoint = [&](double radius, double z, double tangential_offset) -> Vector3 {
            return radius * radial + tangential_offset * tangent + bodyPosition(0.0, 0.0, z);
        };

        if (geometry.fin_shape == FinShape::Trapezoidal) {
            const Vector3 a0 = makePoint(radius_m, fin_front_z_m, -thickness_m * 0.5);
            const Vector3 b0 = makePoint(fin_outer_radius_m, fin_tip_front_z_m, -thickness_m * 0.5);
            const Vector3 c0 = makePoint(fin_outer_radius_m, fin_tip_back_z_m, -thickness_m * 0.5);
            const Vector3 d0 = makePoint(radius_m, fin_back_z_m, -thickness_m * 0.5);
            const Vector3 a1 = makePoint(radius_m, fin_front_z_m, thickness_m * 0.5);
            const Vector3 b1 = makePoint(fin_outer_radius_m, fin_tip_front_z_m, thickness_m * 0.5);
            const Vector3 c1 = makePoint(fin_outer_radius_m, fin_tip_back_z_m, thickness_m * 0.5);
            const Vector3 d1 = makePoint(radius_m, fin_back_z_m, thickness_m * 0.5);
            appendQuad(buffers, a0, b0, c0, d0);
            appendQuad(buffers, a1, d1, c1, b1);
            appendQuad(buffers, a0, a1, b1, b0);
            appendQuad(buffers, b0, b1, c1, c0);
            appendQuad(buffers, c0, c1, d1, d0);
            appendQuad(buffers, d0, d1, a1, a0);
            continue;
        }

        const int arc_segments = geometry.fin_shape == FinShape::Airfoil ? 18 : 14;
        std::vector<Vector3> left_face;
        std::vector<Vector3> right_face;
        left_face.reserve(static_cast<std::size_t>(arc_segments) + 2);
        right_face.reserve(static_cast<std::size_t>(arc_segments) + 2);

        left_face.push_back(makePoint(radius_m, fin_front_z_m, -thickness_m * 0.5));
        right_face.push_back(makePoint(radius_m, fin_front_z_m, thickness_m * 0.5));

        for (int segment = 0; segment <= arc_segments; ++segment) {
            const double t = static_cast<double>(segment) / static_cast<double>(arc_segments);
            const double arc_angle = pi * t;
            const double radius_interp = radius_m + geometry.fin_span_m * geometry.fin_controls.span_scale * std::sin(arc_angle);
            const double chord_interp = geometry.fin_root_chord_m + (geometry.fin_tip_chord_m - geometry.fin_root_chord_m) * std::sin(arc_angle);
            const double z = fin_front_z_m + chord_interp * t;
            const double local_thickness = geometry.fin_shape == FinShape::Airfoil
                                               ? thickness_m * nacaThicknessFraction(t) * 0.5
                                               : thickness_m * 0.5;
            left_face.push_back(makePoint(radius_interp, z, -local_thickness));
            right_face.push_back(makePoint(radius_interp, z, local_thickness));
        }

        left_face.push_back(makePoint(radius_m, fin_back_z_m, -thickness_m * 0.5));
        right_face.push_back(makePoint(radius_m, fin_back_z_m, thickness_m * 0.5));

        for (std::size_t index = 1; index + 1 < left_face.size(); ++index) {
            pushTriangle(buffers, {left_face[0], 0.0, 0.0}, {left_face[index], 1.0, 0.0}, {left_face[index + 1], 1.0, 1.0});
            pushTriangle(buffers, {right_face[0], 0.0, 0.0}, {right_face[index + 1], 1.0, 1.0}, {right_face[index], 1.0, 0.0});
        }
        for (std::size_t index = 0; index + 1 < left_face.size(); ++index) {
            appendQuad(buffers, left_face[index], right_face[index], right_face[index + 1], left_face[index + 1]);
        }
    }

    recomputeNormals(buffers.topology);
    return std::move(buffers.topology);
}

IndexedMeshData buildOverrideTopology(const ComponentTopologyOverride& override_data) {
    IndexedMeshData topology;
    if (!override_data.is_active ||
        override_data.vertex_positions_body_m.size() < 3 ||
        override_data.indices.size() < 3 ||
        override_data.indices.size() % 3 != 0) {
        return topology;
    }

    topology.vertices.reserve(override_data.vertex_positions_body_m.size());
    for (const auto& position : override_data.vertex_positions_body_m) {
        topology.vertices.push_back(MeshVertexData {
            .position_body_m = position,
            .normal_body = {},
            .u = 0.0,
            .v = 0.0
        });
    }

    topology.indices = override_data.indices;
    const bool valid_indices = std::all_of(topology.indices.begin(), topology.indices.end(), [&](unsigned int index) {
        return static_cast<std::size_t>(index) < topology.vertices.size();
    });
    if (!valid_indices) {
        return {};
    }

    recomputeNormals(topology);
    return topology;
}

void applyVertexModifiers(ComponentType component, const VehicleGeometry& geometry, IndexedMeshData& topology) {
    const auto* modifiers = geometry.getActiveComponentModifiers(component);
    if (modifiers == nullptr) {
        return;
    }
    for (const auto& modifier : modifiers->modified_vertices) {
        if (modifier.vertex_id < 0 || static_cast<std::size_t>(modifier.vertex_id) >= topology.vertices.size()) {
            continue;
        }
        topology.vertices[static_cast<std::size_t>(modifier.vertex_id)].position_body_m =
            modifier.base_position_m + modifier.offset_m;
    }
    recomputeNormals(topology);
}

}  // namespace

struct MeshGenerator::Impl {
    struct ComponentMeshGpu {
        IndexedMeshData topology {};
        std::vector<MeshEdgeData> edges {};
        std::vector<MeshFaceData> faces {};
        ::Mesh gpu_mesh {};
    };

    VehicleGeometry geometry {};
    MotorCluster cluster {std::vector<MountedMotor> {}};
    ComponentMeshGpu body {};
    ComponentMeshGpu nose {};
    ComponentMeshGpu transition {};
    ComponentMeshGpu fin {};
    ComponentMeshGpu payload {};
    ComponentMeshGpu motor {};
    ::Material body_material {};
    ::Material nose_material {};
    ::Material transition_material {};
    ::Material fin_material {};
    ::Material payload_material {};
    ::Material motor_material {};
    std::array<double, static_cast<std::size_t>(CfdComponentBand::Count)> pressure_overlay_pa {};
    std::array<float, static_cast<std::size_t>(CfdComponentBand::Count)> failure_overlay_intensity {};
    double pressure_reference_pa {1.0};
    bool pressure_overlay_enabled {false};
    bool failure_overlay_enabled {false};
    bool built {false};

    void unloadComponent(ComponentMeshGpu& component) {
        if (component.gpu_mesh.vertexCount > 0) {
            UnloadMesh(component.gpu_mesh);
            component.gpu_mesh = {};
        }
        component.topology = {};
        component.edges.clear();
        component.faces.clear();
    }

    void unload() {
        unloadComponent(body);
        unloadComponent(nose);
        unloadComponent(transition);
        unloadComponent(fin);
        unloadComponent(payload);
        unloadComponent(motor);
        if (built) {
            UnloadMaterial(body_material);
            UnloadMaterial(nose_material);
            UnloadMaterial(transition_material);
            UnloadMaterial(fin_material);
            UnloadMaterial(payload_material);
            UnloadMaterial(motor_material);
        }
        body_material = {};
        nose_material = {};
        transition_material = {};
        fin_material = {};
        payload_material = {};
        motor_material = {};
        built = false;
    }

    ComponentMeshGpu* component(ComponentType type) noexcept {
        switch (type) {
        case ComponentType::NoseCone:
            return &nose;
        case ComponentType::BodyTube:
            return &body;
        case ComponentType::Transition:
            return &transition;
        case ComponentType::FinSet:
            return &fin;
        case ComponentType::MotorMount:
            return &motor;
        case ComponentType::Payload:
            return &payload;
        }
        return nullptr;
    }

    const ComponentMeshGpu* component(ComponentType type) const noexcept {
        switch (type) {
        case ComponentType::NoseCone:
            return &nose;
        case ComponentType::BodyTube:
            return &body;
        case ComponentType::Transition:
            return &transition;
        case ComponentType::FinSet:
            return &fin;
        case ComponentType::MotorMount:
            return &motor;
        case ComponentType::Payload:
            return &payload;
        }
        return nullptr;
    }

    void refreshComponent(ComponentMeshGpu& component) {
        recomputeNormals(component.topology);
        component.faces = buildFaceCache(component.topology);
        component.edges = buildEdgeCache(component.topology, component.faces);
        if (component.gpu_mesh.vertexCount > 0) {
            UnloadMesh(component.gpu_mesh);
        }
        component.gpu_mesh = uploadMesh(component.topology);
    }

    void refreshComponentSurface(ComponentMeshGpu& component) {
        recomputeNormals(component.topology);
        if (component.gpu_mesh.vertexCount != static_cast<int>(component.topology.vertices.size()) ||
            component.gpu_mesh.triangleCount != static_cast<int>(component.topology.indices.size() / 3) ||
            component.gpu_mesh.vertices == nullptr ||
            component.gpu_mesh.normals == nullptr ||
            component.gpu_mesh.vboId == nullptr) {
            if (component.gpu_mesh.vertexCount > 0) {
                UnloadMesh(component.gpu_mesh);
            }
            component.gpu_mesh = uploadMesh(component.topology);
            return;
        }

        syncGpuMeshVertexArrays(component.gpu_mesh, component.topology);
        UpdateMeshBuffer(
            component.gpu_mesh,
            SHADER_LOC_VERTEX_POSITION,
            component.gpu_mesh.vertices,
            component.gpu_mesh.vertexCount * 3 * static_cast<int>(sizeof(float)),
            0);
        UpdateMeshBuffer(
            component.gpu_mesh,
            SHADER_LOC_VERTEX_NORMAL,
            component.gpu_mesh.normals,
            component.gpu_mesh.vertexCount * 3 * static_cast<int>(sizeof(float)),
            0);
    }

    void applyPressureOverlay() {
        const auto component_color = [&](ComponentMaterial material, CfdComponentBand band) {
            const Color base = materialBaseColor(material);
            if (!pressure_overlay_enabled) {
                return base;
            }
            const double ratio =
                pressure_overlay_pa[static_cast<std::size_t>(band)] / std::max(pressure_reference_pa, 1.0);
            const float normalized = std::clamp(static_cast<float>(std::sqrt(std::max(ratio, 0.0))), 0.0f, 1.0f);
            return lerpColor(base, pressureHeatColor(normalized), 0.72f * normalized);
        };

        const auto apply_failure_overlay = [&](ComponentMaterial material, CfdComponentBand band) {
            const Color base = materialBaseColor(material);
            const float failure = std::clamp(failure_overlay_intensity[static_cast<std::size_t>(band)], 0.0f, 1.0f);
            const double ratio = pressure_overlay_pa[static_cast<std::size_t>(band)] / std::max(pressure_reference_pa, 1.0);
            const float normalized = std::clamp(static_cast<float>(std::sqrt(std::max(ratio, 0.0))), 0.0f, 1.0f);
            Color result = base;
            if (failure > 0.0f) {
                result = lerpColor(result, Color {239, 68, 68, 255}, failure);
            }
            if (pressure_overlay_enabled) {
                result = lerpColor(result, pressureHeatColor(normalized), 0.72f * normalized);
            }
            return result;
        };

        body_material.maps[MATERIAL_MAP_DIFFUSE].color =
            apply_failure_overlay(geometry.body_material, CfdComponentBand::BodyTube);
        nose_material.maps[MATERIAL_MAP_DIFFUSE].color =
            apply_failure_overlay(geometry.nose_material, CfdComponentBand::NoseCone);
        transition_material.maps[MATERIAL_MAP_DIFFUSE].color =
            apply_failure_overlay(geometry.transition_material, CfdComponentBand::Transition);
        fin_material.maps[MATERIAL_MAP_DIFFUSE].color =
            apply_failure_overlay(geometry.fin_material, CfdComponentBand::FinSet);
        payload_material.maps[MATERIAL_MAP_DIFFUSE].color =
            apply_failure_overlay(geometry.payload_material, CfdComponentBand::Payload);
        motor_material.maps[MATERIAL_MAP_DIFFUSE].color =
            apply_failure_overlay(ComponentMaterial::Aluminum6061, CfdComponentBand::MotorMount);
    }
};

MeshGenerator::MeshGenerator(const VehicleGeometry& geometry, const MotorCluster& cluster)
    : impl_(std::make_unique<Impl>()) {
    rebuild(geometry, cluster);
}

MeshGenerator::~MeshGenerator() {
    impl_->unload();
}

void MeshGenerator::rebuild(const VehicleGeometry& geometry, const MotorCluster& cluster) {
    impl_->unload();
    impl_->geometry = geometry;
    impl_->cluster = cluster;

    impl_->body.topology = buildBodyTopology(geometry);
    impl_->nose.topology = buildNoseTopology(geometry);
    impl_->transition.topology = buildTransitionTopology(geometry);
    impl_->fin.topology = buildFinTopology(geometry);
    impl_->payload.topology = buildPayloadTopology(geometry);
    impl_->motor.topology = buildMotorTopology(geometry, cluster);

    if (const auto* override_data = geometry.getActiveTopologyOverride(ComponentType::BodyTube);
        override_data != nullptr && override_data->is_active) {
        if (IndexedMeshData override_topology = buildOverrideTopology(*override_data);
            !override_topology.vertices.empty()) {
            impl_->body.topology = std::move(override_topology);
        }
    }
    if (const auto* override_data = geometry.getActiveTopologyOverride(ComponentType::NoseCone);
        override_data != nullptr && override_data->is_active) {
        if (IndexedMeshData override_topology = buildOverrideTopology(*override_data);
            !override_topology.vertices.empty()) {
            impl_->nose.topology = std::move(override_topology);
        }
    }
    if (const auto* override_data = geometry.getActiveTopologyOverride(ComponentType::Transition);
        override_data != nullptr && override_data->is_active) {
        if (IndexedMeshData override_topology = buildOverrideTopology(*override_data);
            !override_topology.vertices.empty()) {
            impl_->transition.topology = std::move(override_topology);
        }
    }
    if (const auto* override_data = geometry.getActiveTopologyOverride(ComponentType::FinSet);
        override_data != nullptr && override_data->is_active) {
        if (IndexedMeshData override_topology = buildOverrideTopology(*override_data);
            !override_topology.vertices.empty()) {
            impl_->fin.topology = std::move(override_topology);
        }
    }
    if (const auto* override_data = geometry.getActiveTopologyOverride(ComponentType::Payload);
        override_data != nullptr && override_data->is_active) {
        if (IndexedMeshData override_topology = buildOverrideTopology(*override_data);
            !override_topology.vertices.empty()) {
            impl_->payload.topology = std::move(override_topology);
        }
    }
    if (const auto* override_data = geometry.getActiveTopologyOverride(ComponentType::MotorMount);
        override_data != nullptr && override_data->is_active) {
        if (IndexedMeshData override_topology = buildOverrideTopology(*override_data);
            !override_topology.vertices.empty()) {
            impl_->motor.topology = std::move(override_topology);
        }
    }

    applyVertexModifiers(ComponentType::BodyTube, geometry, impl_->body.topology);
    applyVertexModifiers(ComponentType::NoseCone, geometry, impl_->nose.topology);
    applyVertexModifiers(ComponentType::Transition, geometry, impl_->transition.topology);
    applyVertexModifiers(ComponentType::FinSet, geometry, impl_->fin.topology);
    applyVertexModifiers(ComponentType::Payload, geometry, impl_->payload.topology);
    applyVertexModifiers(ComponentType::MotorMount, geometry, impl_->motor.topology);

    impl_->refreshComponent(impl_->body);
    impl_->refreshComponent(impl_->nose);
    impl_->refreshComponent(impl_->transition);
    impl_->refreshComponent(impl_->fin);
    impl_->refreshComponent(impl_->payload);
    impl_->refreshComponent(impl_->motor);

    impl_->body_material = LoadMaterialDefault();
    impl_->nose_material = LoadMaterialDefault();
    impl_->transition_material = LoadMaterialDefault();
    impl_->fin_material = LoadMaterialDefault();
    impl_->payload_material = LoadMaterialDefault();
    impl_->motor_material = LoadMaterialDefault();
    impl_->body_material.maps[MATERIAL_MAP_DIFFUSE].color = materialBaseColor(geometry.body_material);
    impl_->nose_material.maps[MATERIAL_MAP_DIFFUSE].color = materialBaseColor(geometry.nose_material);
    impl_->transition_material.maps[MATERIAL_MAP_DIFFUSE].color = materialBaseColor(geometry.transition_material);
    impl_->fin_material.maps[MATERIAL_MAP_DIFFUSE].color = materialBaseColor(geometry.fin_material);
    impl_->payload_material.maps[MATERIAL_MAP_DIFFUSE].color = materialBaseColor(geometry.payload_material);
    impl_->motor_material.maps[MATERIAL_MAP_DIFFUSE].color = Color {190, 200, 214, 255};
    impl_->applyPressureOverlay();
    impl_->built = true;
}

void MeshGenerator::draw(const FlightState& state) const {
    if (!impl_->built) {
        return;
    }

    impl_->applyPressureOverlay();
    const ::Matrix transform = bodyToWorldTransform(state);
    const auto draw_component = [&](const Impl::ComponentMeshGpu& component, const ::Material& material, CfdComponentBand band) {
        if (component.gpu_mesh.vertexCount == 0) {
            return;
        }
        DrawMesh(component.gpu_mesh, material, transform);
        if (impl_->failure_overlay_enabled) {
            const float intensity = std::clamp(impl_->failure_overlay_intensity[static_cast<std::size_t>(band)], 0.0f, 1.0f);
            if (intensity > 0.001f) {
                const Color wire_color = {
                    255,
                    static_cast<unsigned char>(120.0f + 120.0f * (1.0f - intensity)),
                    static_cast<unsigned char>(120.0f + 120.0f * (1.0f - intensity)),
                    static_cast<unsigned char>(140.0f + 115.0f * intensity)};
                drawIndexedMeshWireOverlay(component.topology, state, wire_color);
            }
        }
    };

    draw_component(impl_->body, impl_->body_material, CfdComponentBand::BodyTube);
    draw_component(impl_->nose, impl_->nose_material, CfdComponentBand::NoseCone);
    if (impl_->transition.gpu_mesh.vertexCount > 0) {
        draw_component(impl_->transition, impl_->transition_material, CfdComponentBand::Transition);
    }
    draw_component(impl_->fin, impl_->fin_material, CfdComponentBand::FinSet);
    if (impl_->payload.gpu_mesh.vertexCount > 0) {
        draw_component(impl_->payload, impl_->payload_material, CfdComponentBand::Payload);
    }
    if (impl_->motor.gpu_mesh.vertexCount > 0) {
        draw_component(impl_->motor, impl_->motor_material, CfdComponentBand::MotorMount);
    }
}

void MeshGenerator::setPressureOverlay(
    const std::array<double, static_cast<std::size_t>(CfdComponentBand::Count)>& component_pressure_pa,
    double reference_pressure_pa,
    bool enabled) noexcept {
    impl_->pressure_overlay_pa = component_pressure_pa;
    impl_->pressure_reference_pa = std::max(reference_pressure_pa, 1.0);
    impl_->pressure_overlay_enabled = enabled;
}

void MeshGenerator::setComponentFailureOverlay(
    const std::array<float, static_cast<std::size_t>(CfdComponentBand::Count)>& component_failure_intensity,
    bool enabled) noexcept {
    impl_->failure_overlay_intensity = component_failure_intensity;
    impl_->failure_overlay_enabled = enabled;
}

void MeshGenerator::drawWireframe(const FlightState& state) const {
    if (!impl_->built) {
        return;
    }

    const auto draw_component = [&](const Impl::ComponentMeshGpu& component) {
        if (component.topology.indices.empty()) {
            return;
        }
        drawIndexedMeshWireOverlay(component.topology, state, Color {241, 245, 249, 120});
    };

    draw_component(impl_->body);
    draw_component(impl_->nose);
    draw_component(impl_->transition);
    draw_component(impl_->fin);
    draw_component(impl_->payload);
    draw_component(impl_->motor);
}

const IndexedMeshData* MeshGenerator::componentMesh(ComponentType component) const noexcept {
    const auto* mesh = impl_->component(component);
    return mesh != nullptr ? &mesh->topology : nullptr;
}

const std::vector<MeshEdgeData>* MeshGenerator::componentEdges(ComponentType component) const noexcept {
    const auto* mesh = impl_->component(component);
    return mesh != nullptr ? &mesh->edges : nullptr;
}

const std::vector<MeshFaceData>* MeshGenerator::componentFaces(ComponentType component) const noexcept {
    const auto* mesh = impl_->component(component);
    return mesh != nullptr ? &mesh->faces : nullptr;
}

std::size_t MeshGenerator::componentVertexCount(ComponentType component) const noexcept {
    const auto* mesh = componentMesh(component);
    return mesh != nullptr ? mesh->vertices.size() : 0;
}

bool MeshGenerator::componentVertexPosition(ComponentType component, int vertex_id, Vector3& out_position_body_m) const noexcept {
    const auto* mesh = componentMesh(component);
    if (mesh == nullptr || vertex_id < 0 || static_cast<std::size_t>(vertex_id) >= mesh->vertices.size()) {
        return false;
    }
    out_position_body_m = mesh->vertices[static_cast<std::size_t>(vertex_id)].position_body_m;
    return true;
}

bool MeshGenerator::setComponentVertexPosition(
    ComponentType component,
    int vertex_id,
    const Vector3& position_body_m,
    bool rebuild_topology_caches) {
    auto* mesh = impl_->component(component);
    if (mesh == nullptr || vertex_id < 0 || static_cast<std::size_t>(vertex_id) >= mesh->topology.vertices.size()) {
        return false;
    }

    auto& vertex = mesh->topology.vertices[static_cast<std::size_t>(vertex_id)];
    if ((vertex.position_body_m - position_body_m).magnitude() <= 1e-9) {
        return false;
    }

    vertex.position_body_m = position_body_m;
    if (rebuild_topology_caches) {
        impl_->refreshComponent(*mesh);
    } else {
        impl_->refreshComponentSurface(*mesh);
    }
    return true;
}

void MeshGenerator::finalizeComponentMeshEdits(ComponentType component) {
    if (auto* mesh = impl_->component(component); mesh != nullptr) {
        impl_->refreshComponent(*mesh);
    }
}

bool MeshGenerator::extrudeComponentFace(ComponentType component, int face_id, double distance_m) {
    auto* mesh = impl_->component(component);
    if (mesh == nullptr || face_id < 0 || static_cast<std::size_t>(face_id) >= mesh->faces.size() || std::abs(distance_m) <= 1e-9) {
        return false;
    }

    const auto face = mesh->faces[static_cast<std::size_t>(face_id)];
    const auto& a = mesh->topology.vertices[face.vertex_a];
    const auto& b = mesh->topology.vertices[face.vertex_b];
    const auto& c = mesh->topology.vertices[face.vertex_c];
    const Vector3 normal = face.normal_body.magnitude() > 1e-9 ? face.normal_body.normalized() : Vector3 {0.0, 0.0, 1.0};

    const unsigned int a2 = static_cast<unsigned int>(mesh->topology.vertices.size());
    const unsigned int b2 = a2 + 1;
    const unsigned int c2 = a2 + 2;
    mesh->topology.vertices.push_back(MeshVertexData {.position_body_m = a.position_body_m + normal * distance_m, .normal_body = normal, .u = a.u, .v = a.v});
    mesh->topology.vertices.push_back(MeshVertexData {.position_body_m = b.position_body_m + normal * distance_m, .normal_body = normal, .u = b.u, .v = b.v});
    mesh->topology.vertices.push_back(MeshVertexData {.position_body_m = c.position_body_m + normal * distance_m, .normal_body = normal, .u = c.u, .v = c.v});

    auto& indices = mesh->topology.indices;
    indices.push_back(a2);
    indices.push_back(b2);
    indices.push_back(c2);

    const auto append_side = [&](unsigned int p0, unsigned int p1, unsigned int q0, unsigned int q1) {
        indices.push_back(p0);
        indices.push_back(p1);
        indices.push_back(q1);
        indices.push_back(p0);
        indices.push_back(q1);
        indices.push_back(q0);
    };

    append_side(face.vertex_a, face.vertex_b, a2, b2);
    append_side(face.vertex_b, face.vertex_c, b2, c2);
    append_side(face.vertex_c, face.vertex_a, c2, a2);

    impl_->refreshComponent(*mesh);
    return true;
}

bool MeshGenerator::bevelComponentFace(ComponentType component, int face_id, double inset_ratio, double normal_offset_m) {
    auto* mesh = impl_->component(component);
    if (mesh == nullptr || face_id < 0 || static_cast<std::size_t>(face_id) >= mesh->faces.size()) {
        return false;
    }

    const auto face = mesh->faces[static_cast<std::size_t>(face_id)];
    const double clamped_inset = std::clamp(inset_ratio, 0.05, 0.82);
    const auto& a = mesh->topology.vertices[face.vertex_a];
    const auto& b = mesh->topology.vertices[face.vertex_b];
    const auto& c = mesh->topology.vertices[face.vertex_c];
    const Vector3 center = face.center_body_m;
    const Vector3 normal = face.normal_body.magnitude() > 1e-9 ? face.normal_body.normalized() : Vector3 {0.0, 0.0, 1.0};

    const unsigned int a2 = static_cast<unsigned int>(mesh->topology.vertices.size());
    const unsigned int b2 = a2 + 1;
    const unsigned int c2 = a2 + 2;
    mesh->topology.vertices.push_back(MeshVertexData {
        .position_body_m = a.position_body_m + (center - a.position_body_m) * clamped_inset + normal * normal_offset_m,
        .normal_body = normal,
        .u = a.u,
        .v = a.v
    });
    mesh->topology.vertices.push_back(MeshVertexData {
        .position_body_m = b.position_body_m + (center - b.position_body_m) * clamped_inset + normal * normal_offset_m,
        .normal_body = normal,
        .u = b.u,
        .v = b.v
    });
    mesh->topology.vertices.push_back(MeshVertexData {
        .position_body_m = c.position_body_m + (center - c.position_body_m) * clamped_inset + normal * normal_offset_m,
        .normal_body = normal,
        .u = c.u,
        .v = c.v
    });

    auto& indices = mesh->topology.indices;
    const std::size_t base = static_cast<std::size_t>(face_id) * 3;
    indices[base] = a2;
    indices[base + 1] = b2;
    indices[base + 2] = c2;

    const auto append_ring = [&](unsigned int p0, unsigned int p1, unsigned int q0, unsigned int q1) {
        indices.push_back(p0);
        indices.push_back(p1);
        indices.push_back(q1);
        indices.push_back(p0);
        indices.push_back(q1);
        indices.push_back(q0);
    };

    append_ring(face.vertex_a, face.vertex_b, a2, b2);
    append_ring(face.vertex_b, face.vertex_c, b2, c2);
    append_ring(face.vertex_c, face.vertex_a, c2, a2);

    impl_->refreshComponent(*mesh);
    return true;
}

bool MeshGenerator::loopCutComponentEdge(ComponentType component, int edge_id) {
    auto* mesh = impl_->component(component);
    if (mesh == nullptr || edge_id < 0 || static_cast<std::size_t>(edge_id) >= mesh->edges.size()) {
        return false;
    }

    const auto edge = mesh->edges[static_cast<std::size_t>(edge_id)];
    if (edge.vertex_a >= mesh->topology.vertices.size() || edge.vertex_b >= mesh->topology.vertices.size()) {
        return false;
    }

    const auto& va = mesh->topology.vertices[edge.vertex_a];
    const auto& vb = mesh->topology.vertices[edge.vertex_b];
    const unsigned int midpoint = static_cast<unsigned int>(mesh->topology.vertices.size());
    mesh->topology.vertices.push_back(MeshVertexData {
        .position_body_m = (va.position_body_m + vb.position_body_m) * 0.5,
        .normal_body = (va.normal_body + vb.normal_body).normalized(),
        .u = 0.5 * (va.u + vb.u),
        .v = 0.5 * (va.v + vb.v)
    });

    const auto split_face = [&](int face_id) {
        if (face_id < 0) {
            return false;
        }
        const std::size_t base = static_cast<std::size_t>(face_id) * 3;
        if (base + 2 >= mesh->topology.indices.size()) {
            return false;
        }

        auto& indices = mesh->topology.indices;
        const unsigned int i0 = indices[base];
        const unsigned int i1 = indices[base + 1];
        const unsigned int i2 = indices[base + 2];
        const unsigned int a = edge.vertex_a;
        const unsigned int b = edge.vertex_b;

        if ((i0 == a && i1 == b) || (i0 == b && i1 == a)) {
            indices[base] = i0;
            indices[base + 1] = midpoint;
            indices[base + 2] = i2;
            indices.push_back(midpoint);
            indices.push_back(i1);
            indices.push_back(i2);
            return true;
        }
        if ((i1 == a && i2 == b) || (i1 == b && i2 == a)) {
            indices[base] = i1;
            indices[base + 1] = midpoint;
            indices[base + 2] = i0;
            indices.push_back(midpoint);
            indices.push_back(i2);
            indices.push_back(i0);
            return true;
        }
        if ((i2 == a && i0 == b) || (i2 == b && i0 == a)) {
            indices[base] = i2;
            indices[base + 1] = midpoint;
            indices[base + 2] = i1;
            indices.push_back(midpoint);
            indices.push_back(i0);
            indices.push_back(i1);
            return true;
        }
        return false;
    };

    bool changed = split_face(edge.face_a);
    changed = split_face(edge.face_b) || changed;
    if (!changed) {
        return false;
    }

    impl_->refreshComponent(*mesh);
    return true;
}

NoseConeShape MeshGenerator::noseConeShape() const noexcept {
    return impl_->geometry.nose_cone_shape;
}

std::string MeshGenerator::noseConeShapeLabel() const {
    return std::string(rocket::noseConeShapeLabel(impl_->geometry.nose_cone_shape));
}

std::string MeshGenerator::finShapeLabel() const {
    return std::string(rocket::finShapeLabel(impl_->geometry.fin_shape));
}

}  // namespace rocket
