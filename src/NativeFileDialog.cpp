#include "rocket/NativeFileDialog.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#endif

namespace rocket {

namespace {

#ifdef _WIN32
std::wstring widen(const std::string& text) {
    if (text.empty()) {
        return {};
    }

    const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (size <= 1) {
        return std::wstring(text.begin(), text.end());
    }

    std::wstring wide(static_cast<std::size_t>(size - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), size);
    return wide;
}

std::wstring buildFilter(const std::string& label, const std::string& pattern) {
    std::wstring filter = widen(label);
    filter.push_back(L'\0');
    std::wstring wide_pattern = widen(pattern);
    filter += wide_pattern;
    filter.push_back(L'\0');
    filter.push_back(L'\0');
    return filter;
}

std::string narrowLossy(const std::wstring& text) {
    std::string result;
    result.reserve(text.size());
    for (wchar_t ch : text) {
        result.push_back(ch >= 0 && ch <= 127 ? static_cast<char>(ch) : '?');
    }
    return result;
}

std::wstring extractDefaultExtension(const std::string& filter_pattern) {
    const std::size_t dot = filter_pattern.find_last_of('.');
    if (dot == std::string::npos) {
        return L"";
    }

    std::string extension = filter_pattern.substr(dot + 1);
    extension.erase(std::remove(extension.begin(), extension.end(), '*'), extension.end());
    extension.erase(std::remove(extension.begin(), extension.end(), '.'), extension.end());
    extension.erase(std::remove(extension.begin(), extension.end(), ';'), extension.end());
    return widen(extension);
}

std::wstring commonDialogErrorLabel(DWORD code) {
    switch (code) {
    case CDERR_DIALOGFAILURE:
        return L"CDERR_DIALOGFAILURE";
    case CDERR_FINDRESFAILURE:
        return L"CDERR_FINDRESFAILURE";
    case CDERR_INITIALIZATION:
        return L"CDERR_INITIALIZATION";
    case CDERR_LOADRESFAILURE:
        return L"CDERR_LOADRESFAILURE";
    case CDERR_LOADSTRFAILURE:
        return L"CDERR_LOADSTRFAILURE";
    case CDERR_LOCKRESFAILURE:
        return L"CDERR_LOCKRESFAILURE";
    case CDERR_MEMALLOCFAILURE:
        return L"CDERR_MEMALLOCFAILURE";
    case CDERR_MEMLOCKFAILURE:
        return L"CDERR_MEMLOCKFAILURE";
    case CDERR_NOHINSTANCE:
        return L"CDERR_NOHINSTANCE";
    case CDERR_NOHOOK:
        return L"CDERR_NOHOOK";
    case CDERR_NOTEMPLATE:
        return L"CDERR_NOTEMPLATE";
    case CDERR_STRUCTSIZE:
        return L"CDERR_STRUCTSIZE";
    case FNERR_BUFFERTOOSMALL:
        return L"FNERR_BUFFERTOOSMALL";
    case FNERR_INVALIDFILENAME:
        return L"FNERR_INVALIDFILENAME";
    case FNERR_SUBCLASSFAILURE:
        return L"FNERR_SUBCLASSFAILURE";
    default:
        return L"UNKNOWN_DIALOG_ERROR";
    }
}

bool promptFilePathCommon(
    bool save_mode,
    const std::string& title,
    const std::string& filter_label,
    const std::string& filter_pattern,
    std::filesystem::path& selected_path,
    std::string& error_message) {
    std::vector<wchar_t> buffer(4096, L'\0');

    std::filesystem::path initial_path = selected_path;
    if (!initial_path.empty() && initial_path.is_relative()) {
        initial_path = std::filesystem::current_path() / initial_path;
    }

    std::filesystem::path initial_dir = std::filesystem::current_path();
    if (!initial_path.empty()) {
        if (initial_path.has_parent_path() && std::filesystem::exists(initial_path.parent_path())) {
            initial_dir = initial_path.parent_path();
        }
        const std::wstring initial_name = initial_path.filename().wstring();
        const std::size_t copy_size = std::min(initial_name.size(), buffer.size() - 1);
        std::copy_n(initial_name.begin(), copy_size, buffer.begin());
    }

    std::wstring filter = buildFilter(filter_label, filter_pattern);
    const std::wstring wide_title = widen(title);
    const std::wstring wide_initial_dir = initial_dir.wstring();
    const std::wstring default_extension = extractDefaultExtension(filter_pattern);

    OPENFILENAMEW dialog {};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetForegroundWindow();
    dialog.lpstrFile = buffer.data();
    dialog.nMaxFile = static_cast<DWORD>(buffer.size());
    dialog.lpstrFilter = filter.c_str();
    dialog.lpstrTitle = wide_title.c_str();
    dialog.lpstrInitialDir = wide_initial_dir.empty() ? nullptr : wide_initial_dir.c_str();
    dialog.lpstrDefExt = default_extension.empty() ? nullptr : default_extension.c_str();
    dialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (save_mode) {
        dialog.Flags |= OFN_OVERWRITEPROMPT;
    } else {
        dialog.Flags |= OFN_FILEMUSTEXIST;
    }

    const BOOL result = save_mode ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog);
    if (result == FALSE) {
        const DWORD dialog_error = CommDlgExtendedError();
        if (dialog_error == 0) {
            error_message.clear();
            return false;
        }
        const std::wstring error_label = commonDialogErrorLabel(dialog_error);
        error_message = std::format(
            "Native file dialog failed: {} (code {}).",
            narrowLossy(error_label),
            dialog_error);
        return false;
    }

    selected_path = std::filesystem::path(buffer.data()).lexically_normal();
    return true;
}
#endif

}  // namespace

bool promptOpenFilePath(
    const std::string& title,
    const std::string& filter_label,
    const std::string& filter_pattern,
    std::filesystem::path& selected_path,
    std::string& error_message) {
#ifdef _WIN32
    return promptFilePathCommon(false, title, filter_label, filter_pattern, selected_path, error_message);
#else
    error_message = "Native file dialogs are only implemented on Windows in this build.";
    return false;
#endif
}

bool promptSaveFilePath(
    const std::string& title,
    const std::string& filter_label,
    const std::string& filter_pattern,
    std::filesystem::path& selected_path,
    std::string& error_message) {
#ifdef _WIN32
    return promptFilePathCommon(true, title, filter_label, filter_pattern, selected_path, error_message);
#else
    error_message = "Native file dialogs are only implemented on Windows in this build.";
    return false;
#endif
}

}  // namespace rocket
