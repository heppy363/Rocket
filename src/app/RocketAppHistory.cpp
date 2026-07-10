#include "RocketAppHistory.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <limits>
#include <string>
#include <vector>

#include "imgui.h"
#include "RocketAppAeroHelpers.hpp"

namespace rocket::app {

namespace {

bool trajectorySampleHasHistoricalMetrics(const rocket::TrajectorySample& sample) {
    return
        sample.time_s > 0.0 ||
        sample.static_pressure_pa > 0.0 ||
        sample.total_pressure_pa > 0.0 ||
        sample.air_density_kgpm3 > 0.0;
}

template <typename Accessor>
std::vector<float> buildHistorySeries(
    const rocket::SimulationRuntime& runtime,
    float& min_value,
    float& max_value,
    Accessor accessor) {
    std::vector<float> values;
    values.reserve(runtime.trajectory_history.size());
    min_value = std::numeric_limits<float>::max();
    max_value = std::numeric_limits<float>::lowest();

    for (const auto& sample : runtime.trajectory_history) {
        if (!trajectorySampleHasHistoricalMetrics(sample)) {
            continue;
        }
        const float value = static_cast<float>(accessor(sample));
        values.push_back(value);
        min_value = std::min(min_value, value);
        max_value = std::max(max_value, value);
    }

    if (values.empty()) {
        min_value = 0.0f;
        max_value = 1.0f;
        return values;
    }

    if (std::abs(max_value - min_value) < 1e-5f) {
        const float padding = std::max(1.0f, std::abs(max_value) * 0.08f);
        min_value -= padding;
        max_value += padding;
    }

    return values;
}

template <typename Accessor>
void drawHistoryMetricPlotImGui(
    const char* child_id,
    const char* label,
    const rocket::SimulationRuntime& runtime,
    double current_value,
    const std::string& current_label,
    const ImVec4& accent,
    Accessor accessor) {
    ImGui::BeginChild(
        child_id,
        ImVec2(0.0f, 88.0f),
        ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextColored(accent, "%s", label);
    ImGui::Text("Now %s", current_label.c_str());

    float min_value = 0.0f;
    float max_value = 1.0f;
    std::vector<float> values = buildHistorySeries(runtime, min_value, max_value, accessor);
    min_value = std::min(min_value, static_cast<float>(current_value));
    max_value = std::max(max_value, static_cast<float>(current_value));
    if (std::abs(max_value - min_value) < 1e-5f) {
        max_value += 1.0f;
        min_value -= 1.0f;
    }

    if (values.size() < 2) {
        ImGui::TextDisabled("Lo storico si popola durante il volo e il replay.");
        ImGui::EndChild();
        return;
    }

    ImGui::TextDisabled("Range %.3g .. %.3g", min_value, max_value);
    ImGui::PlotLines(
        std::format("##{}", child_id).c_str(),
        values.data(),
        static_cast<int>(values.size()),
        0,
        nullptr,
        min_value,
        max_value,
        ImVec2(-1.0f, 34.0f));
    ImGui::EndChild();
}

}  // namespace

void drawSimulationHistoryPanelImGui(
    const rocket::SimulationRuntime& runtime,
    const rocket::VehicleModel& vehicle,
    const rocket::SimulationSnapshot& snapshot) {
    const char* mode_label =
        runtime.scrub_preview_active ? "Scrub"
        : runtime.keyframe_preview_active ? "Keyframe"
        : runtime.replay_active ? "Replay"
                                : "Live";
    ImGui::TextWrapped("Storico compatto dei parametri missione per confronto rapido durante volo, replay e review.");
    ImGui::TextDisabled("Vista %s | Campioni %zu", mode_label, runtime.trajectory_history.size());
    ImGui::Separator();

    drawHistoryMetricPlotImGui(
        "history-altitude",
        "Altitude",
        runtime,
        snapshot.state.position_m.z,
        std::format("{:.1f} m", snapshot.state.position_m.z),
        ImVec4(0.49f, 0.86f, 0.62f, 1.0f),
        [](const rocket::TrajectorySample& sample) { return sample.altitude_m; });
    drawHistoryMetricPlotImGui(
        "history-mach",
        "Mach",
        runtime,
        snapshot.mach_number,
        std::format("{:.2f}", snapshot.mach_number),
        ImVec4(0.78f, 0.69f, 1.0f, 1.0f),
        [](const rocket::TrajectorySample& sample) { return sample.mach_number; });
    drawHistoryMetricPlotImGui(
        "history-q",
        "Dynamic Pressure q",
        runtime,
        snapshot.dynamic_pressure_pa,
        std::format("{:.0f} Pa", snapshot.dynamic_pressure_pa),
        ImVec4(0.99f, 0.67f, 0.38f, 1.0f),
        [](const rocket::TrajectorySample& sample) { return sample.dynamic_pressure_pa; });
    drawHistoryMetricPlotImGui(
        "history-rho",
        "Density rho",
        runtime,
        snapshot.air_density_kgpm3,
        std::format("{:.3f} kg/m3", snapshot.air_density_kgpm3),
        ImVec4(0.54f, 0.88f, 1.0f, 1.0f),
        [](const rocket::TrajectorySample& sample) { return sample.air_density_kgpm3; });
    drawHistoryMetricPlotImGui(
        "history-p",
        "Static Pressure P",
        runtime,
        snapshot.static_pressure_pa,
        std::format("{:.0f} Pa", snapshot.static_pressure_pa),
        ImVec4(0.53f, 0.79f, 1.0f, 1.0f),
        [](const rocket::TrajectorySample& sample) { return sample.static_pressure_pa; });
    drawHistoryMetricPlotImGui(
        "history-p0",
        "Total Pressure P0",
        runtime,
        snapshot.total_pressure_pa,
        std::format("{:.0f} Pa", snapshot.total_pressure_pa),
        ImVec4(0.99f, 0.82f, 0.39f, 1.0f),
        [](const rocket::TrajectorySample& sample) { return sample.total_pressure_pa; });
    drawHistoryMetricPlotImGui(
        "history-re",
        "Reynolds",
        runtime,
        reynoldsNumber(snapshot, vehicle.geometry.body_length_m),
        std::format("{:.2e}", reynoldsNumber(snapshot, vehicle.geometry.body_length_m)),
        ImVec4(0.93f, 0.63f, 0.98f, 1.0f),
        [](const rocket::TrajectorySample& sample) { return sample.reynolds_number; });
}

}  // namespace rocket::app
