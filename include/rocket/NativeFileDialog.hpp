#pragma once

#include <filesystem>
#include <string>

namespace rocket {

[[nodiscard]] bool promptOpenFilePath(
    const std::string& title,
    const std::string& filter_label,
    const std::string& filter_pattern,
    std::filesystem::path& selected_path,
    std::string& error_message);

[[nodiscard]] bool promptSaveFilePath(
    const std::string& title,
    const std::string& filter_label,
    const std::string& filter_pattern,
    std::filesystem::path& selected_path,
    std::string& error_message);

}  // namespace rocket
