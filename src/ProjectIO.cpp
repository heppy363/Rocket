#include "rocket/ProjectIO.hpp"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <format>
#include <optional>
#include <sstream>
#include <string_view>
#include <unordered_map>

#include "rocket/DesignLibrary.hpp"
#include "rocket/SimulationCaches.hpp"

namespace rocket {

namespace {

struct ParsedDocument {
    std::unordered_map<std::string, std::string> values {};
};

std::string trim(std::string_view text) {
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos) {
        return {};
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    return std::string(text.substr(begin, end - begin + 1));
}

std::string componentKey(ComponentType component) {
    switch (component) {
    case ComponentType::NoseCone:
        return "nose";
    case ComponentType::BodyTube:
        return "body";
    case ComponentType::Transition:
        return "transition";
    case ComponentType::FinSet:
        return "fins";
    case ComponentType::MotorMount:
        return "motor_mount";
    case ComponentType::Payload:
        return "payload";
    }
    return "body";
}

std::optional<ComponentMaterial> parseComponentMaterial(std::string_view value) {
    if (value == "PLA-CF") {
        return ComponentMaterial::PlaCf;
    }
    if (value == "Alluminio 6061") {
        return ComponentMaterial::Aluminum6061;
    }
    if (value == "PVC") {
        return ComponentMaterial::Pvc;
    }
    if (value == "Fiberglass") {
        return ComponentMaterial::Fiberglass;
    }
    if (value == "Carbon Fiber") {
        return ComponentMaterial::CarbonFiber;
    }
    if (value == "Betulla Aircraft") {
        return ComponentMaterial::BirchPlywood;
    }
    if (value == "Phenolic Tube") {
        return ComponentMaterial::Phenolic;
    }
    return std::nullopt;
}

std::optional<NoseConeShape> parseNoseConeShape(std::string_view value) {
    if (value == "Conical") {
        return NoseConeShape::Conical;
    }
    if (value == "Tangent Ogive") {
        return NoseConeShape::TangentOgive;
    }
    if (value == "Parabolic") {
        return NoseConeShape::Parabolic;
    }
    if (value == "LD-Haack") {
        return NoseConeShape::LdHaack;
    }
    return std::nullopt;
}

std::optional<FinShape> parseFinShape(std::string_view value) {
    if (value == "Trapezoidal") {
        return FinShape::Trapezoidal;
    }
    if (value == "Elliptical") {
        return FinShape::Elliptical;
    }
    if (value == "Airfoil") {
        return FinShape::Airfoil;
    }
    return std::nullopt;
}

std::optional<TransitionShape> parseTransitionShape(std::string_view value) {
    if (value == "Conical") {
        return TransitionShape::Conical;
    }
    if (value == "Curved") {
        return TransitionShape::Curved;
    }
    return std::nullopt;
}

std::optional<RocketPreset> parseRocketPreset(std::string_view value) {
    if (value == "Research Starter") {
        return RocketPreset::ResearchStarter;
    }
    if (value == "Sport Trainer") {
        return RocketPreset::SportTrainer;
    }
    if (value == "High Altitude") {
        return RocketPreset::HighAltitude;
    }
    if (value == "Minimum Diameter") {
        return RocketPreset::MinimumDiameter;
    }
    if (value == "Heavy Lift") {
        return RocketPreset::HeavyLift;
    }
    return std::nullopt;
}

std::optional<WeatherDataSource> parseWeatherSource(std::string_view value) {
    if (value == "Manual") {
        return WeatherDataSource::Manual;
    }
    if (value == "Open-Meteo Ready") {
        return WeatherDataSource::OpenMeteoReady;
    }
    if (value == "OpenWeatherMap Ready") {
        return WeatherDataSource::OpenWeatherMapReady;
    }
    return std::nullopt;
}

template <typename T>
void writeScalar(std::ostream& stream, std::string_view key, const T& value) {
    stream << key << "=" << value << '\n';
}

void writeString(std::ostream& stream, std::string_view key, std::string_view value) {
    stream << key << "=" << value << '\n';
}

void writeSection(std::ostream& stream, std::string_view section) {
    stream << '\n' << "[" << section << "]" << '\n';
}

template <typename T>
std::optional<T> parseNumber(std::string_view text) {
    T value {};
    const auto* begin = text.data();
    const auto* end = text.data() + text.size();
    if constexpr (std::is_integral_v<T>) {
        const auto result = std::from_chars(begin, end, value);
        if (result.ec != std::errc() || result.ptr != end) {
            return std::nullopt;
        }
        return value;
    } else {
        std::string storage(text);
        char* parsed_end = nullptr;
        const double parsed = std::strtod(storage.c_str(), &parsed_end);
        if (parsed_end == storage.c_str() || *parsed_end != '\0') {
            return std::nullopt;
        }
        return static_cast<T>(parsed);
    }
}

std::optional<bool> parseBool(std::string_view text) {
    if (text == "1" || text == "true" || text == "True") {
        return true;
    }
    if (text == "0" || text == "false" || text == "False") {
        return false;
    }
    return std::nullopt;
}

std::optional<std::string> lookup(const ParsedDocument& parsed, const std::string& key) {
    const auto found = parsed.values.find(key);
    if (found == parsed.values.end()) {
        return std::nullopt;
    }
    return found->second;
}

template <typename T>
bool loadNumberIfPresent(const ParsedDocument& parsed, const std::string& key, T& target, std::string& error_message) {
    const auto value = lookup(parsed, key);
    if (!value.has_value()) {
        return true;
    }
    const auto parsed_value = parseNumber<T>(*value);
    if (!parsed_value.has_value()) {
        error_message = std::format("Invalid numeric value for key '{}'.", key);
        return false;
    }
    target = *parsed_value;
    return true;
}

bool loadBoolIfPresent(const ParsedDocument& parsed, const std::string& key, bool& target, std::string& error_message) {
    const auto value = lookup(parsed, key);
    if (!value.has_value()) {
        return true;
    }
    const auto parsed_value = parseBool(*value);
    if (!parsed_value.has_value()) {
        error_message = std::format("Invalid bool value for key '{}'.", key);
        return false;
    }
    target = *parsed_value;
    return true;
}

ParsedDocument parseProjectFile(std::istream& input) {
    ParsedDocument parsed;
    std::string line;
    std::string section;
    while (std::getline(input, line)) {
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed.starts_with('#')) {
            continue;
        }
        if (trimmed.front() == '[' && trimmed.back() == ']') {
            section = trim(std::string_view(trimmed).substr(1, trimmed.size() - 2));
            continue;
        }
        const auto equal_pos = trimmed.find('=');
        if (equal_pos == std::string::npos) {
            continue;
        }
        std::string key = trim(std::string_view(trimmed).substr(0, equal_pos));
        const std::string value = trim(std::string_view(trimmed).substr(equal_pos + 1));
        if (!section.empty()) {
            key = section + "." + key;
        }
        parsed.values[key] = value;
    }
    return parsed;
}

void writeVector3(std::ostream& stream, std::string_view prefix, const Vector3& value) {
    writeScalar(stream, std::format("{}.x", prefix), value.x);
    writeScalar(stream, std::format("{}.y", prefix), value.y);
    writeScalar(stream, std::format("{}.z", prefix), value.z);
}

bool readVector3(const ParsedDocument& parsed, std::string_view prefix, Vector3& out, std::string& error_message) {
    return loadNumberIfPresent(parsed, std::format("{}.x", prefix), out.x, error_message) &&
           loadNumberIfPresent(parsed, std::format("{}.y", prefix), out.y, error_message) &&
           loadNumberIfPresent(parsed, std::format("{}.z", prefix), out.z, error_message);
}

void writeVertexModifiers(std::ostream& stream, const std::string& component_name, const ComponentVertexModifiers& modifiers) {
    writeSection(stream, "modifier." + component_name);
    writeScalar(stream, "active", modifiers.is_active ? 1 : 0);
    writeScalar(stream, "count", modifiers.modified_vertices.size());
    for (std::size_t index = 0; index < modifiers.modified_vertices.size(); ++index) {
        const auto& vertex = modifiers.modified_vertices[index];
        writeScalar(stream, std::format("vertex{}.id", index), vertex.vertex_id);
        writeVector3(stream, std::format("vertex{}.base", index), vertex.base_position_m);
        writeVector3(stream, std::format("vertex{}.offset", index), vertex.offset_m);
        writeScalar(stream, std::format("vertex{}.influence_radius_m", index), vertex.influence_radius_m);
        writeScalar(stream, std::format("vertex{}.locked", index), vertex.locked ? 1 : 0);
    }
}

bool readVertexModifiers(
    const ParsedDocument& parsed,
    const std::string& component_name,
    ComponentType component_type,
    ComponentVertexModifiers& modifiers,
    std::string& error_message) {
    modifiers.component_type = component_type;
    if (!loadBoolIfPresent(parsed, "modifier." + component_name + ".active", modifiers.is_active, error_message)) {
        return false;
    }

    int count = 0;
    if (!loadNumberIfPresent(parsed, "modifier." + component_name + ".count", count, error_message)) {
        return false;
    }
    count = std::max(count, 0);
    modifiers.modified_vertices.clear();
    modifiers.modified_vertices.reserve(static_cast<std::size_t>(count));

    for (int index = 0; index < count; ++index) {
        FreeControlVertex vertex {};
        if (!loadNumberIfPresent(parsed, std::format("modifier.{}.vertex{}.id", component_name, index), vertex.vertex_id, error_message) ||
            !readVector3(parsed, std::format("modifier.{}.vertex{}.base", component_name, index), vertex.base_position_m, error_message) ||
            !readVector3(parsed, std::format("modifier.{}.vertex{}.offset", component_name, index), vertex.offset_m, error_message) ||
            !loadNumberIfPresent(parsed, std::format("modifier.{}.vertex{}.influence_radius_m", component_name, index), vertex.influence_radius_m, error_message) ||
            !loadBoolIfPresent(parsed, std::format("modifier.{}.vertex{}.locked", component_name, index), vertex.locked, error_message)) {
            return false;
        }
        modifiers.modified_vertices.push_back(vertex);
    }
    return true;
}

void writeTopologyOverride(std::ostream& stream, const std::string& component_name, const ComponentTopologyOverride& topology) {
    writeSection(stream, "topology." + component_name);
    writeScalar(stream, "active", topology.is_active ? 1 : 0);
    writeScalar(stream, "vertex_count", topology.vertex_positions_body_m.size());
    writeScalar(stream, "index_count", topology.indices.size());
    for (std::size_t index = 0; index < topology.vertex_positions_body_m.size(); ++index) {
        writeVector3(stream, std::format("vertex{}", index), topology.vertex_positions_body_m[index]);
    }
    for (std::size_t index = 0; index < topology.indices.size(); ++index) {
        writeScalar(stream, std::format("index{}", index), topology.indices[index]);
    }
}

bool readTopologyOverride(
    const ParsedDocument& parsed,
    const std::string& component_name,
    ComponentType component_type,
    ComponentTopologyOverride& topology,
    std::string& error_message) {
    topology.component_type = component_type;
    if (!loadBoolIfPresent(parsed, "topology." + component_name + ".active", topology.is_active, error_message)) {
        return false;
    }

    int vertex_count = 0;
    int index_count = 0;
    if (!loadNumberIfPresent(parsed, "topology." + component_name + ".vertex_count", vertex_count, error_message) ||
        !loadNumberIfPresent(parsed, "topology." + component_name + ".index_count", index_count, error_message)) {
        return false;
    }

    vertex_count = std::max(vertex_count, 0);
    index_count = std::max(index_count, 0);
    topology.vertex_positions_body_m.clear();
    topology.indices.clear();
    topology.vertex_positions_body_m.reserve(static_cast<std::size_t>(vertex_count));
    topology.indices.reserve(static_cast<std::size_t>(index_count));

    for (int index = 0; index < vertex_count; ++index) {
        Vector3 position {};
        if (!readVector3(parsed, std::format("topology.{}.vertex{}", component_name, index), position, error_message)) {
            return false;
        }
        topology.vertex_positions_body_m.push_back(position);
    }
    for (int index = 0; index < index_count; ++index) {
        unsigned int triangle_index {};
        if (!loadNumberIfPresent(parsed, std::format("topology.{}.index{}", component_name, index), triangle_index, error_message)) {
            return false;
        }
        topology.indices.push_back(triangle_index);
    }
    return true;
}

void writeMotorCluster(std::ostream& stream, const MotorCluster& cluster) {
    writeSection(stream, "cluster");
    writeScalar(stream, "count", cluster.motorCount());
    const auto& motors = cluster.mountedMotors();
    for (std::size_t index = 0; index < motors.size(); ++index) {
        const auto& mounted_motor = motors[index];
        writeScalar(stream, std::format("motor{}.max_thrust_n", index), mounted_motor.motor.max_thrust_n);
        writeScalar(stream, std::format("motor{}.burn_time_s", index), mounted_motor.motor.burn_time_s);
        writeScalar(stream, std::format("motor{}.propellant_mass_kg", index), mounted_motor.motor.propellant_mass_kg);
        writeVector3(stream, std::format("motor{}.mount_position_m", index), mounted_motor.mount_position_m);
        writeVector3(stream, std::format("motor{}.thrust_direction_body", index), mounted_motor.thrust_direction_body);
        writeScalar(stream, std::format("motor{}.failed", index), mounted_motor.failed ? 1 : 0);
    }
}

bool readMotorCluster(const ParsedDocument& parsed, MotorCluster& cluster, std::string& error_message) {
    int count = 0;
    if (!loadNumberIfPresent(parsed, "cluster.count", count, error_message)) {
        return false;
    }
    count = std::max(count, 0);
    std::vector<MountedMotor> motors;
    motors.reserve(static_cast<std::size_t>(count));
    for (int index = 0; index < count; ++index) {
        MountedMotor mounted_motor {};
        if (!loadNumberIfPresent(parsed, std::format("cluster.motor{}.max_thrust_n", index), mounted_motor.motor.max_thrust_n, error_message) ||
            !loadNumberIfPresent(parsed, std::format("cluster.motor{}.burn_time_s", index), mounted_motor.motor.burn_time_s, error_message) ||
            !loadNumberIfPresent(parsed, std::format("cluster.motor{}.propellant_mass_kg", index), mounted_motor.motor.propellant_mass_kg, error_message) ||
            !readVector3(parsed, std::format("cluster.motor{}.mount_position_m", index), mounted_motor.mount_position_m, error_message) ||
            !readVector3(parsed, std::format("cluster.motor{}.thrust_direction_body", index), mounted_motor.thrust_direction_body, error_message) ||
            !loadBoolIfPresent(parsed, std::format("cluster.motor{}.failed", index), mounted_motor.failed, error_message)) {
            return false;
        }
        motors.push_back(mounted_motor);
    }
    cluster = MotorCluster(std::move(motors));
    return true;
}

void ensureParentDirectory(const std::filesystem::path& path) {
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }
}

}  // namespace

bool saveProjectDocument(
    const std::filesystem::path& path,
    const ProjectDocument& document,
    std::string& error_message) {
    try {
        ensureParentDirectory(path);
        std::ofstream output(path, std::ios::binary);
        if (!output) {
            error_message = std::format("Cannot open '{}' for writing.", path.string());
            return false;
        }

        output << "rocket_lab_project\n";
        writeScalar(output, "format_version", 1);

        writeSection(output, "project");
        writeString(output, "active_preset", std::string(rocketPresetLabel(document.active_preset)));

        writeSection(output, "geometry");
        const auto& geometry = document.vehicle.geometry;
        writeScalar(output, "body_length_m", geometry.body_length_m);
        writeScalar(output, "body_diameter_m", geometry.body_diameter_m);
        writeScalar(output, "wall_thickness_m", geometry.wall_thickness_m);
        writeScalar(output, "nose_length_m", geometry.nose_length_m);
        writeString(output, "nose_cone_shape", std::string(noseConeShapeLabel(geometry.nose_cone_shape)));
        writeString(output, "nose_material", std::string(materialDefinition(geometry.nose_material).label));
        writeScalar(output, "transition_length_m", geometry.transition_length_m);
        writeScalar(output, "transition_aft_diameter_m", geometry.transition_aft_diameter_m);
        writeString(output, "transition_shape", std::string(transitionShapeLabel(geometry.transition_shape)));
        writeString(output, "transition_material", std::string(materialDefinition(geometry.transition_material).label));
        writeString(output, "body_material", std::string(materialDefinition(geometry.body_material).label));
        writeScalar(output, "fin_front_from_nose_m", geometry.fin_front_from_nose_m);
        writeScalar(output, "fin_root_chord_m", geometry.fin_root_chord_m);
        writeScalar(output, "fin_tip_chord_m", geometry.fin_tip_chord_m);
        writeScalar(output, "fin_span_m", geometry.fin_span_m);
        writeScalar(output, "fin_sweep_length_m", geometry.fin_sweep_length_m);
        writeScalar(output, "fin_thickness_m", geometry.fin_thickness_m);
        writeString(output, "fin_shape", std::string(finShapeLabel(geometry.fin_shape)));
        writeString(output, "fin_material", std::string(materialDefinition(geometry.fin_material).label));
        writeScalar(output, "fin_count", geometry.fin_count);
        writeScalar(output, "payload_length_m", geometry.payload_length_m);
        writeScalar(output, "payload_mass_kg", geometry.payload_mass_kg);
        writeString(output, "payload_material", std::string(materialDefinition(geometry.payload_material).label));
        writeScalar(output, "nose_mid_radius_scale", geometry.nose_controls.mid_radius_scale);
        writeScalar(output, "nose_shoulder_radius_scale", geometry.nose_controls.shoulder_radius_scale);
        writeScalar(output, "body_fore_radius_scale", geometry.body_controls.fore_radius_scale);
        writeScalar(output, "body_mid_radius_scale", geometry.body_controls.mid_radius_scale);
        writeScalar(output, "body_aft_radius_scale", geometry.body_controls.aft_radius_scale);
        writeScalar(output, "transition_mid_radius_scale", geometry.transition_controls.mid_radius_scale);
        writeScalar(output, "fin_tip_le_offset_m", geometry.fin_controls.tip_le_offset_m);
        writeScalar(output, "fin_tip_te_offset_m", geometry.fin_controls.tip_te_offset_m);
        writeScalar(output, "fin_span_scale", geometry.fin_controls.span_scale);
        writeScalar(output, "fin_thickness_scale", geometry.fin_controls.thickness_scale);
        writeScalar(output, "structure_cg_from_nose_m", geometry.structure_cg_from_nose_m);
        writeScalar(output, "propellant_cg_from_nose_m", geometry.propellant_cg_from_nose_m);

        writeSection(output, "recovery");
        writeScalar(output, "parachute_drag_coefficient", document.vehicle.recovery_system.parachute_drag_coefficient);
        writeScalar(output, "parachute_area_m2", document.vehicle.recovery_system.parachute_area_m2);
        writeScalar(output, "deployment_altitude_m", document.vehicle.recovery_system.deployment_altitude_m);
        writeScalar(output, "deployment_delay_s", document.vehicle.recovery_system.deployment_delay_s);

        writeSection(output, "launch_site");
        writeScalar(output, "latitude_deg", document.launch_site.latitude_deg);
        writeScalar(output, "longitude_deg", document.launch_site.longitude_deg);
        writeScalar(output, "elevation_m", document.launch_site.elevation_m);

        writeSection(output, "surface_weather");
        writeScalar(output, "pressure_hpa", document.surface_weather.pressure_hpa);
        writeScalar(output, "temperature_c", document.surface_weather.temperature_c);
        writeScalar(output, "humidity_percent", document.surface_weather.humidity_percent);
        writeScalar(output, "wind_speed_mps", document.surface_weather.wind_speed_mps);
        writeScalar(output, "wind_direction_deg", document.surface_weather.wind_direction_deg);
        writeScalar(output, "wind_gust_mps", document.surface_weather.wind_gust_mps);

        writeSection(output, "weather");
        switch (document.weather_source) {
        case WeatherDataSource::OpenMeteoReady:
            writeString(output, "source", "Open-Meteo Ready");
            break;
        case WeatherDataSource::OpenWeatherMapReady:
            writeString(output, "source", "OpenWeatherMap Ready");
            break;
        case WeatherDataSource::Manual:
        default:
            writeString(output, "source", "Manual");
            break;
        }

        writeSection(output, "motor_editor");
        writeScalar(output, "motor_count", document.motor_settings.motor_count);
        writeScalar(output, "max_thrust_n", document.motor_settings.max_thrust_n);
        writeScalar(output, "burn_time_s", document.motor_settings.burn_time_s);
        writeScalar(output, "propellant_mass_kg", document.motor_settings.propellant_mass_kg);
        writeScalar(output, "mount_radius_m", document.motor_settings.mount_radius_m);
        writeScalar(output, "cant_angle_deg", document.motor_settings.cant_angle_deg);

        writeMotorCluster(output, document.vehicle.cluster);

        const struct ModifierEntry {
            ComponentType type;
            const ComponentVertexModifiers* modifiers;
            const ComponentTopologyOverride* topology;
        } entries[] {
            {ComponentType::NoseCone, &geometry.nose_vertex_mods, &geometry.nose_topology_override},
            {ComponentType::BodyTube, &geometry.body_vertex_mods, &geometry.body_topology_override},
            {ComponentType::Transition, &geometry.transition_vertex_mods, &geometry.transition_topology_override},
            {ComponentType::FinSet, &geometry.fin_vertex_mods, &geometry.fin_topology_override},
            {ComponentType::MotorMount, &geometry.motor_vertex_mods, &geometry.motor_topology_override},
            {ComponentType::Payload, &geometry.payload_vertex_mods, &geometry.payload_topology_override}
        };

        for (const auto& entry : entries) {
            const std::string key = componentKey(entry.type);
            writeVertexModifiers(output, key, *entry.modifiers);
            writeTopologyOverride(output, key, *entry.topology);
        }
        return true;
    } catch (const std::exception& error) {
        error_message = error.what();
        return false;
    }
}

bool loadProjectDocument(
    const std::filesystem::path& path,
    ProjectDocument& document,
    std::string& error_message) {
    constexpr int supported_format_version = 1;

    try {
        std::ifstream input(path, std::ios::binary);
        if (!input) {
            error_message = std::format("Cannot open '{}' for reading.", path.string());
            return false;
        }

        std::string header;
        std::getline(input, header);
        if (trim(header) != "rocket_lab_project") {
            error_message = "Invalid .rlab header.";
            return false;
        }

        const ParsedDocument parsed = parseProjectFile(input);
        int format_version = supported_format_version;
        if (!loadNumberIfPresent(parsed, "format_version", format_version, error_message)) {
            return false;
        }
        if (format_version != supported_format_version) {
            error_message = std::format(
                "Unsupported .rlab format_version {}. Expected {}.",
                format_version,
                supported_format_version);
            return false;
        }

        ProjectDocument loaded {};

        if (const auto preset_value = lookup(parsed, "project.active_preset"); preset_value.has_value()) {
            const auto parsed_preset = parseRocketPreset(*preset_value);
            if (!parsed_preset.has_value()) {
                error_message = "Invalid rocket preset in project file.";
                return false;
            }
            loaded.active_preset = *parsed_preset;
        }

        auto& geometry = loaded.vehicle.geometry;
        if (!loadNumberIfPresent(parsed, "geometry.body_length_m", geometry.body_length_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.body_diameter_m", geometry.body_diameter_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.wall_thickness_m", geometry.wall_thickness_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.nose_length_m", geometry.nose_length_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.transition_length_m", geometry.transition_length_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.transition_aft_diameter_m", geometry.transition_aft_diameter_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_front_from_nose_m", geometry.fin_front_from_nose_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_root_chord_m", geometry.fin_root_chord_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_tip_chord_m", geometry.fin_tip_chord_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_span_m", geometry.fin_span_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_sweep_length_m", geometry.fin_sweep_length_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_thickness_m", geometry.fin_thickness_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_count", geometry.fin_count, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.payload_length_m", geometry.payload_length_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.payload_mass_kg", geometry.payload_mass_kg, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.nose_mid_radius_scale", geometry.nose_controls.mid_radius_scale, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.nose_shoulder_radius_scale", geometry.nose_controls.shoulder_radius_scale, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.body_fore_radius_scale", geometry.body_controls.fore_radius_scale, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.body_mid_radius_scale", geometry.body_controls.mid_radius_scale, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.body_aft_radius_scale", geometry.body_controls.aft_radius_scale, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.transition_mid_radius_scale", geometry.transition_controls.mid_radius_scale, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_tip_le_offset_m", geometry.fin_controls.tip_le_offset_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_tip_te_offset_m", geometry.fin_controls.tip_te_offset_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_span_scale", geometry.fin_controls.span_scale, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.fin_thickness_scale", geometry.fin_controls.thickness_scale, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.structure_cg_from_nose_m", geometry.structure_cg_from_nose_m, error_message) ||
            !loadNumberIfPresent(parsed, "geometry.propellant_cg_from_nose_m", geometry.propellant_cg_from_nose_m, error_message)) {
            return false;
        }

        if (const auto value = lookup(parsed, "geometry.nose_cone_shape"); value.has_value()) {
            const auto parsed_value = parseNoseConeShape(*value);
            if (!parsed_value.has_value()) {
                error_message = "Invalid nose_cone_shape.";
                return false;
            }
            geometry.nose_cone_shape = *parsed_value;
        }
        if (const auto value = lookup(parsed, "geometry.transition_shape"); value.has_value()) {
            const auto parsed_value = parseTransitionShape(*value);
            if (!parsed_value.has_value()) {
                error_message = "Invalid transition_shape.";
                return false;
            }
            geometry.transition_shape = *parsed_value;
        }
        if (const auto value = lookup(parsed, "geometry.fin_shape"); value.has_value()) {
            const auto parsed_value = parseFinShape(*value);
            if (!parsed_value.has_value()) {
                error_message = "Invalid fin_shape.";
                return false;
            }
            geometry.fin_shape = *parsed_value;
        }
        if (const auto value = lookup(parsed, "geometry.nose_material"); value.has_value()) {
            const auto parsed_value = parseComponentMaterial(*value);
            if (!parsed_value.has_value()) {
                error_message = "Invalid nose_material.";
                return false;
            }
            geometry.nose_material = *parsed_value;
        }
        if (const auto value = lookup(parsed, "geometry.transition_material"); value.has_value()) {
            const auto parsed_value = parseComponentMaterial(*value);
            if (!parsed_value.has_value()) {
                error_message = "Invalid transition_material.";
                return false;
            }
            geometry.transition_material = *parsed_value;
        }
        if (const auto value = lookup(parsed, "geometry.body_material"); value.has_value()) {
            const auto parsed_value = parseComponentMaterial(*value);
            if (!parsed_value.has_value()) {
                error_message = "Invalid body_material.";
                return false;
            }
            geometry.body_material = *parsed_value;
        }
        if (const auto value = lookup(parsed, "geometry.fin_material"); value.has_value()) {
            const auto parsed_value = parseComponentMaterial(*value);
            if (!parsed_value.has_value()) {
                error_message = "Invalid fin_material.";
                return false;
            }
            geometry.fin_material = *parsed_value;
        }
        if (const auto value = lookup(parsed, "geometry.payload_material"); value.has_value()) {
            const auto parsed_value = parseComponentMaterial(*value);
            if (!parsed_value.has_value()) {
                error_message = "Invalid payload_material.";
                return false;
            }
            geometry.payload_material = *parsed_value;
        }

        if (!loadNumberIfPresent(parsed, "recovery.parachute_drag_coefficient", loaded.vehicle.recovery_system.parachute_drag_coefficient, error_message) ||
            !loadNumberIfPresent(parsed, "recovery.parachute_area_m2", loaded.vehicle.recovery_system.parachute_area_m2, error_message) ||
            !loadNumberIfPresent(parsed, "recovery.deployment_altitude_m", loaded.vehicle.recovery_system.deployment_altitude_m, error_message) ||
            !loadNumberIfPresent(parsed, "recovery.deployment_delay_s", loaded.vehicle.recovery_system.deployment_delay_s, error_message)) {
            return false;
        }

        if (!loadNumberIfPresent(parsed, "launch_site.latitude_deg", loaded.launch_site.latitude_deg, error_message) ||
            !loadNumberIfPresent(parsed, "launch_site.longitude_deg", loaded.launch_site.longitude_deg, error_message) ||
            !loadNumberIfPresent(parsed, "launch_site.elevation_m", loaded.launch_site.elevation_m, error_message) ||
            !loadNumberIfPresent(parsed, "surface_weather.pressure_hpa", loaded.surface_weather.pressure_hpa, error_message) ||
            !loadNumberIfPresent(parsed, "surface_weather.temperature_c", loaded.surface_weather.temperature_c, error_message) ||
            !loadNumberIfPresent(parsed, "surface_weather.humidity_percent", loaded.surface_weather.humidity_percent, error_message) ||
            !loadNumberIfPresent(parsed, "surface_weather.wind_speed_mps", loaded.surface_weather.wind_speed_mps, error_message) ||
            !loadNumberIfPresent(parsed, "surface_weather.wind_direction_deg", loaded.surface_weather.wind_direction_deg, error_message) ||
            !loadNumberIfPresent(parsed, "surface_weather.wind_gust_mps", loaded.surface_weather.wind_gust_mps, error_message)) {
            return false;
        }

        if (const auto value = lookup(parsed, "weather.source"); value.has_value()) {
            const auto parsed_value = parseWeatherSource(*value);
            if (!parsed_value.has_value()) {
                error_message = "Invalid weather source.";
                return false;
            }
            loaded.weather_source = *parsed_value;
        }

        if (!loadNumberIfPresent(parsed, "motor_editor.motor_count", loaded.motor_settings.motor_count, error_message) ||
            !loadNumberIfPresent(parsed, "motor_editor.max_thrust_n", loaded.motor_settings.max_thrust_n, error_message) ||
            !loadNumberIfPresent(parsed, "motor_editor.burn_time_s", loaded.motor_settings.burn_time_s, error_message) ||
            !loadNumberIfPresent(parsed, "motor_editor.propellant_mass_kg", loaded.motor_settings.propellant_mass_kg, error_message) ||
            !loadNumberIfPresent(parsed, "motor_editor.mount_radius_m", loaded.motor_settings.mount_radius_m, error_message) ||
            !loadNumberIfPresent(parsed, "motor_editor.cant_angle_deg", loaded.motor_settings.cant_angle_deg, error_message)) {
            return false;
        }

        if (!readMotorCluster(parsed, loaded.vehicle.cluster, error_message)) {
            return false;
        }

        const struct Entry {
            ComponentType type;
            ComponentVertexModifiers* modifiers;
            ComponentTopologyOverride* topology;
        } entries[] {
            {ComponentType::NoseCone, &geometry.nose_vertex_mods, &geometry.nose_topology_override},
            {ComponentType::BodyTube, &geometry.body_vertex_mods, &geometry.body_topology_override},
            {ComponentType::Transition, &geometry.transition_vertex_mods, &geometry.transition_topology_override},
            {ComponentType::FinSet, &geometry.fin_vertex_mods, &geometry.fin_topology_override},
            {ComponentType::MotorMount, &geometry.motor_vertex_mods, &geometry.motor_topology_override},
            {ComponentType::Payload, &geometry.payload_vertex_mods, &geometry.payload_topology_override}
        };

        for (auto& entry : entries) {
            const std::string key = componentKey(entry.type);
            if (!readVertexModifiers(parsed, key, entry.type, *entry.modifiers, error_message) ||
                !readTopologyOverride(parsed, key, entry.type, *entry.topology, error_message)) {
                return false;
            }
        }

        document = std::move(loaded);
        return true;
    } catch (const std::exception& error) {
        error_message = error.what();
        return false;
    }
}

bool exportProjectReport(
    const std::filesystem::path& path,
    const ProjectDocument& document,
    const SimulationSnapshot& snapshot,
    const ProjectExportSummary& summary,
    std::string& error_message) {
    try {
        ensureParentDirectory(path);
        std::ofstream output(path, std::ios::binary);
        if (!output) {
            error_message = std::format("Cannot open '{}' for writing.", path.string());
            return false;
        }

        output << "# Rocket Lab Report\n\n";
        output << std::format("Preset: {}\n", rocketPresetLabel(document.active_preset));
        output << std::format("Body: {:.2f} m x {:.3f} m\n", document.vehicle.geometry.body_length_m, document.vehicle.geometry.body_diameter_m);
        output << std::format("Dry mass: {:.2f} kg\n", document.vehicle.dry_mass_kg);
        output << std::format("Reference area: {:.5f} m2\n", document.vehicle.reference_area_m2);
        output << std::format("Motors: {}\n", document.vehicle.cluster.motorCount());
        output << std::format("Launch site: lat {:.4f}, lon {:.4f}, elev {:.1f} m\n", document.launch_site.latitude_deg, document.launch_site.longitude_deg, document.launch_site.elevation_m);
        output << std::format("Weather source: {}\n\n",
            document.weather_source == WeatherDataSource::OpenMeteoReady ? "Open-Meteo Ready" :
            document.weather_source == WeatherDataSource::OpenWeatherMapReady ? "OpenWeatherMap Ready" :
            "Manual");

        output << std::format(
            "Surface weather: {:.1f} C, {:.1f} hPa, {:.0f}% RH, wind {:.1f} m/s @ {:.0f} deg, gust {:.1f} m/s\n\n",
            document.surface_weather.temperature_c,
            document.surface_weather.pressure_hpa,
            document.surface_weather.humidity_percent,
            document.surface_weather.wind_speed_mps,
            document.surface_weather.wind_direction_deg,
            document.surface_weather.wind_gust_mps);

        const auto structure = cachedStructureMassBreakdown(document.vehicle.geometry);
        const auto structural = cachedStructuralMaterialAssessment(document.vehicle.geometry);
        output << "## Vehicle Summary\n\n";
        output << std::format("Nose shape: {}\n", noseConeShapeLabel(document.vehicle.geometry.nose_cone_shape));
        output << std::format("Fin shape: {}\n", finShapeLabel(document.vehicle.geometry.fin_shape));
        output << std::format("Transition shape: {}\n", transitionShapeLabel(document.vehicle.geometry.transition_shape));
        output << std::format("Body material: {}\n", materialDefinition(document.vehicle.geometry.body_material).label);
        output << std::format("Nose material: {}\n", materialDefinition(document.vehicle.geometry.nose_material).label);
        output << std::format("Transition material: {}\n", materialDefinition(document.vehicle.geometry.transition_material).label);
        output << std::format("Fin material: {}\n", materialDefinition(document.vehicle.geometry.fin_material).label);
        output << std::format("Payload material: {}\n", materialDefinition(document.vehicle.geometry.payload_material).label);
        output << std::format("Structure mass: {:.2f} kg\n", structure.total_mass_kg);
        output << std::format("  nose: {:.2f} kg\n", structure.nose_mass_kg);
        output << std::format("  body: {:.2f} kg\n", structure.body_mass_kg);
        output << std::format("  transition: {:.2f} kg\n", structure.transition_mass_kg);
        output << std::format("  fins: {:.2f} kg\n", structure.fin_mass_kg);
        output << std::format("  payload bay: {:.2f} kg\n", structure.payload_bay_mass_kg);
        output << std::format("Equivalent modulus: {:.2f} GPa\n", structural.equivalent_modulus_gpa);
        output << std::format("Equivalent density: {:.1f} kg/m3\n", structural.equivalent_density_kg_per_m3);
        output << std::format("Recommended q max: {:.1f} Pa\n\n", structural.recommended_max_dynamic_pressure_pa);

        output << "## Snapshot\n\n";
        output << std::format("Time: {:.2f} s\n", snapshot.time_s);
        output << std::format("Altitude: {:.2f} m\n", snapshot.state.position_m.z);
        output << std::format("Downrange: {:.2f} m\n", std::sqrt(snapshot.state.position_m.x * snapshot.state.position_m.x + snapshot.state.position_m.y * snapshot.state.position_m.y));
        output << std::format("Air speed: {:.2f} m/s\n", snapshot.relative_air_speed_mps);
        output << std::format("Mach: {:.3f}\n", snapshot.mach_number);
        output << std::format("AoA: {:.3f} deg\n", snapshot.angle_of_attack_deg);
        output << std::format("Dynamic pressure: {:.1f} Pa\n", snapshot.dynamic_pressure_pa);
        output << std::format("Static pressure: {:.1f} Pa\n", snapshot.static_pressure_pa);
        output << std::format("Total pressure: {:.1f} Pa\n", snapshot.total_pressure_pa);
        output << std::format("Air density: {:.4f} kg/m3\n", snapshot.air_density_kgpm3);
        output << std::format("Air temperature: {:.2f} K\n", snapshot.air_temperature_k);
        output << std::format("CG / CP: {:.3f} / {:.3f} m\n", snapshot.cg_from_nose_m, snapshot.cp_from_nose_m);
        output << std::format("Static margin: {:.3f} cal\n", snapshot.static_margin_calibers);
        output << std::format("Recommended q max: {:.1f} Pa\n", snapshot.recommended_max_dynamic_pressure_pa);
        output << std::format("Safety factor: {:.3f}\n", snapshot.dynamic_pressure_safety_factor);
        output << std::format("Shockwave intensity: {:.3f}\n", snapshot.shockwave_intensity);
        output << std::format("Aeroelastic response: {:.3f}\n", snapshot.aeroelastic_response);
        output << std::format("CFD solver particles: {}\n", snapshot.cfd_solver_particle_count);
        output << std::format("CFD render particles: {}\n\n", snapshot.cfd_render_particle_count);

        output << "## Mission Summary\n\n";
        output << std::format("Mission time: {:.2f} s\n", summary.mission_time_s);
        output << std::format("Max altitude: {:.2f} m\n", summary.max_altitude_m);
        output << std::format("Max range: {:.2f} m\n", summary.max_range_m);
        output << std::format("Trajectory samples: {}\n", summary.trajectory_sample_count);
        output << std::format("Burnout recorded: {}\n", summary.burnout_recorded ? "yes" : "no");
        if (summary.burnout_recorded) {
            output << std::format("  burnout time: {:.2f} s\n", summary.burnout_time_s);
            output << std::format("  burnout point: ({:.2f}, {:.2f}, {:.2f}) m\n", summary.burnout_point_m.x, summary.burnout_point_m.y, summary.burnout_point_m.z);
        }
        output << std::format("Apogee recorded: {}\n", summary.apogee_recorded ? "yes" : "no");
        if (summary.apogee_recorded) {
            output << std::format("  apogee time: {:.2f} s\n", summary.apogee_time_s);
            output << std::format("  apogee point: ({:.2f}, {:.2f}, {:.2f}) m\n", summary.apogee_point_m.x, summary.apogee_point_m.y, summary.apogee_point_m.z);
        }
        output << std::format("Impact recorded: {}\n", summary.impact_recorded ? "yes" : "no");
        if (summary.impact_recorded) {
            output << std::format("  impact time: {:.2f} s\n", summary.impact_time_s);
            output << std::format("  impact point: ({:.2f}, {:.2f}, {:.2f}) m\n", summary.impact_point_m.x, summary.impact_point_m.y, summary.impact_point_m.z);
        }
        return true;
    } catch (const std::exception& error) {
        error_message = error.what();
        return false;
    }
}

bool exportTrajectoryCsv(
    const std::filesystem::path& path,
    std::span<const TrajectoryRecord> trajectory,
    std::string& error_message) {
    try {
        ensureParentDirectory(path);
        std::ofstream output(path, std::ios::binary);
        if (!output) {
            error_message = std::format("Cannot open '{}' for writing.", path.string());
            return false;
        }

        output << "time_s,pos_x_m,pos_y_m,pos_z_m,vel_x_mps,vel_y_mps,vel_z_mps,quat_w,quat_x,quat_y,quat_z,omega_x_radps,omega_y_radps,omega_z_radps,mass_kg\n";
        for (const auto& sample : trajectory) {
            output << std::format(
                "{:.6f},{:.6f},{:.6f},{:.6f},{:.6f},{:.6f},{:.6f},{:.9f},{:.9f},{:.9f},{:.9f},{:.6f},{:.6f},{:.6f},{:.6f}\n",
                sample.time_s,
                sample.state.position_m.x,
                sample.state.position_m.y,
                sample.state.position_m.z,
                sample.state.velocity_mps.x,
                sample.state.velocity_mps.y,
                sample.state.velocity_mps.z,
                sample.state.attitude_body_to_world.w,
                sample.state.attitude_body_to_world.x,
                sample.state.attitude_body_to_world.y,
                sample.state.attitude_body_to_world.z,
                sample.state.angular_velocity_body_radps.x,
                sample.state.angular_velocity_body_radps.y,
                sample.state.angular_velocity_body_radps.z,
                sample.state.mass_kg);
        }
        return true;
    } catch (const std::exception& error) {
        error_message = error.what();
        return false;
    }
}

}  // namespace rocket
