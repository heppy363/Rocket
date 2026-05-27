#include "RocketAppImGuiHelpers.hpp"

#include <algorithm>

namespace rocket::app {

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
    const bool changed = ImGui::DragFloat(
        label,
        &working_value,
        static_cast<float>(speed),
        static_cast<float>(min_value),
        static_cast<float>(max_value),
        format);
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

void setWindowRect(const ::Rectangle& bounds) {
    ImGui::SetNextWindowPos(ImVec2(bounds.x, bounds.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bounds.width, bounds.height), ImGuiCond_Always);
}

void primeWindowRect(const ::Rectangle& bounds) {
    ImGui::SetNextWindowPos(ImVec2(bounds.x, bounds.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(bounds.width, bounds.height), ImGuiCond_FirstUseEver);
}

void drawMetricLabelValue(const char* label, const std::string& value, const ImVec4& accent) {
    ImGui::BeginChild(
        label,
        ImVec2(0.0f, 54.0f),
        ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextColored(accent, "%s", label);
    ImGui::Text("%s", value.c_str());
    ImGui::EndChild();
}

void drawStatCardImGui(const char* label, const std::string& value, const ImVec4& accent, float height) {
    ImGui::BeginChild(
        label,
        ImVec2(0.0f, height),
        ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
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

}  // namespace rocket::app
