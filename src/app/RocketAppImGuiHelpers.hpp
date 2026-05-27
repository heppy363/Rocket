#pragma once

#include <array>
#include <initializer_list>
#include <string>
#include <utility>

#include "imgui.h"
#include "raylib.h"
#include "rocket/DesignLibrary.hpp"

namespace rocket::app {

ImVec4 toImVec4(Color color);
void applyDarkSpaceTheme();
bool dragDouble(const char* label, double& value, double speed, double min_value, double max_value, const char* format);
bool dragInt(const char* label, int& value, float speed, int min_value, int max_value);
bool comboComponentMaterial(const char* label, rocket::ComponentMaterial& material);

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

void setWindowRect(const ::Rectangle& bounds);
void primeWindowRect(const ::Rectangle& bounds);
void drawMetricLabelValue(const char* label, const std::string& value, const ImVec4& accent);
void drawStatCardImGui(const char* label, const std::string& value, const ImVec4& accent, float height = 58.0f);
void drawKeyValueTableImGui(const char* id, std::initializer_list<std::pair<const char*, std::string>> rows);

}  // namespace rocket::app
