#include "rocket/SimulationMonitor.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "rocket/DesignLibrary.hpp"
#include "rocket/PhysicalConstants.hpp"
#include "rocket/SimulationCaches.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <dwmapi.h>
#include <uxtheme.h>
#include <windows.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
#endif

namespace rocket {

namespace {

constexpr double kPi = 3.14159265358979323846;

double airDynamicViscosityPaS(double temperature_k) {
    const double safe_temperature_k = std::max(temperature_k, 180.0);
    return 1.458e-6 * std::pow(safe_temperature_k, 1.5) / (safe_temperature_k + 110.4);
}

double reynoldsNumber(const SimulationSnapshot& snapshot, double reference_length_m) {
    const double mu = airDynamicViscosityPaS(snapshot.air_temperature_k);
    return (snapshot.air_density_kgpm3 * std::max(snapshot.relative_air_speed_mps, 0.0) * std::max(reference_length_m, 0.001)) /
           std::max(mu, 1e-7);
}

double airTemperatureFromSurfaceK(const SurfaceWeather& weather, double altitude_m) {
    const double surface_temperature_k = weather.temperature_c + 273.15;
    return std::max(180.0, surface_temperature_k - physical_constants::lapse_rate_k_per_m * std::max(altitude_m, 0.0));
}

double airPressureFromSurfacePa(const SurfaceWeather& weather, double altitude_m) {
    const double relative_altitude_m = std::max(altitude_m, 0.0);
    const double surface_temperature_k = weather.temperature_c + 273.15;
    const double base = 1.0 - (physical_constants::lapse_rate_k_per_m * relative_altitude_m) / surface_temperature_k;
    const double exponent =
        (physical_constants::gravitational_acceleration * physical_constants::molar_mass_dry_air) /
        (physical_constants::universal_gas_constant * physical_constants::lapse_rate_k_per_m);
    return std::max(weather.pressure_hpa * 100.0, 1000.0) * std::pow(std::max(base, 1e-6), exponent);
}

double airDensityFromSurfaceKgPerM3(const SurfaceWeather& weather, double altitude_m) {
    const double temperature_k = airTemperatureFromSurfaceK(weather, altitude_m);
    const double pressure_pa = airPressureFromSurfacePa(weather, altitude_m);
    return pressure_pa / (physical_constants::specific_gas_constant_dry_air * temperature_k);
}

POINT sampleProfileFlowPoint(
    const RECT& profile,
    int center_y,
    int nose_tip_x,
    int nose_base_x,
    int body_end_x,
    int tail_end_x,
    int body_radius,
    double total_length,
    double aoa_deg,
    double mach_number,
    double dynamic_pressure_pa,
    double total_pressure_pa,
    double lane_t,
    double progress_t,
    double phase_t) {
    const double y_seed = profile.top + 20.0 + lane_t * static_cast<double>(profile.bottom - profile.top - 40);
    const double relative_to_body = (y_seed - static_cast<double>(center_y)) / std::max(body_radius * 1.8, 1.0);
    const double body_influence = std::exp(-relative_to_body * relative_to_body);
    const double total_width = static_cast<double>(profile.right - profile.left - 28);
    const double x = profile.left + 14.0 + progress_t * total_width;
    const double compressibility = std::clamp(mach_number / 1.4, 0.0, 1.0);
    const double turbulence = std::clamp(dynamic_pressure_pa / 24000.0, 0.0, 1.0);
    const double pressure_bias = std::clamp(total_pressure_pa / 160000.0, 0.0, 1.0);
    double y = y_seed + progress_t * aoa_deg * 1.6;
    if (x >= nose_tip_x - 18 && x <= tail_end_x + 24) {
        const double x_norm = (x - (nose_tip_x + total_length * 0.45)) / std::max(total_length * 0.42, 1.0);
        y += std::exp(-(x_norm * x_norm) * (6.2 + mach_number * 1.8)) * (relative_to_body * 14.0);
        y += std::sin((progress_t * 9.0 + lane_t * 1.8 + phase_t) * kPi) * turbulence * (2.2 + compressibility * 1.2);
        if (mach_number >= 0.92 && mach_number <= 1.18 && x > nose_base_x && x < body_end_x) {
            y += std::sin((progress_t * 16.0 + lane_t * 2.0 + phase_t * 0.6) * kPi) * (1.6 + pressure_bias * 2.2);
        }
        y += body_influence * aoa_deg * 0.15;
    }
    return POINT {static_cast<LONG>(x), static_cast<LONG>(y)};
}

bool pointInsideRect(const POINT& point, const RECT& rect, int margin = 0) {
    return point.x >= rect.left - margin &&
           point.x <= rect.right + margin &&
           point.y >= rect.top - margin &&
           point.y <= rect.bottom + margin;
}

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

std::wstring weatherSourceLabel(WeatherDataSource source) {
    switch (source) {
    case WeatherDataSource::OpenMeteoReady:
        return L"Open-Meteo Ready";
    case WeatherDataSource::OpenWeatherMapReady:
        return L"OpenWeatherMap Ready";
    case WeatherDataSource::Manual:
    default:
        return L"Manual";
    }
}

std::wstring flowRegimeLabel(const SimulationSnapshot& snapshot) {
    if (snapshot.parachute_deployed) {
        return L"Recovery";
    }
    if (snapshot.mach_number < 0.8) {
        return L"Subsonic";
    }
    if (snapshot.mach_number < 1.2) {
        return L"Transonic";
    }
    return L"Supersonic";
}

void applyDarkWindowTheme(HWND hwnd) {
    const BOOL enabled = TRUE;
    const COLORREF background = RGB(7, 14, 24);
    const COLORREF caption = RGB(241, 245, 249);
    SetWindowTheme(hwnd, L"DarkMode_Explorer", nullptr);
    DwmSetWindowAttribute(hwnd, 20, &enabled, sizeof(enabled));
    DwmSetWindowAttribute(hwnd, 19, &enabled, sizeof(enabled));
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &background, sizeof(background));
    DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &caption, sizeof(caption));
    DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &background, sizeof(background));
}

#endif

}  // namespace

class SimulationMonitor::Impl {
public:
    Impl() = default;

    ~Impl() {
        stop();
    }

    void start() {
#ifdef _WIN32
        bool expected = false;
        if (!running_requested_.compare_exchange_strong(expected, true)) {
            return;
        }

        if (worker_.joinable()) {
            worker_.join();
        }
        worker_ = std::thread([this] { threadMain(); });
#endif
    }

    void stop() {
#ifdef _WIN32
        if (!running_requested_.exchange(false)) {
            return;
        }

        const auto windows_copy = windowsSnapshot();
        for (HWND hwnd : windows_copy) {
            if (hwnd != nullptr) {
                PostMessageW(hwnd, WM_CLOSE, 0, 0);
            }
        }

        if (thread_id_ != 0) {
            PostThreadMessageW(thread_id_, WM_QUIT, 0, 0);
        }

        if (worker_.joinable()) {
            worker_.join();
        }

        thread_id_ = 0;
        running_.store(false);
#endif
    }

    void publish(const SimulationMonitorState& state) {
        {
            std::lock_guard lock(state_mutex_);
            latest_state_ = state;
        }

#ifdef _WIN32
        const auto windows_copy = windowsSnapshot();
        for (HWND hwnd : windows_copy) {
            if (hwnd != nullptr) {
                InvalidateRect(hwnd, nullptr, FALSE);
            }
        }
#endif
    }

    [[nodiscard]] bool isRunning() const noexcept {
#ifdef _WIN32
        return running_.load();
#else
        return false;
#endif
    }

private:
#ifdef _WIN32
    enum class PanelKind {
        WindTunnel
    };

    struct PanelWindow {
        Impl* owner {};
        PanelKind kind {};
        std::wstring title {};
        HWND hwnd {};
    };

    static constexpr wchar_t kWindowClassName[] = L"RocketSimulationMonitorPanel";

    void threadMain() {
        thread_id_ = GetCurrentThreadId();
        registerWindowClass();
        createWindows();
        running_.store(true);

        MSG message {};
        while (GetMessageW(&message, nullptr, 0, 0) > 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        destroyWindows();
        running_requested_.store(false);
        running_.store(false);
    }

    void registerWindowClass() {
        WNDCLASSEXW wc {};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc = &Impl::wndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = kWindowClassName;
        RegisterClassExW(&wc);
    }

    void createWindows() {
        const RECT rect {220, 120, 1400, 820};
        panels_[0] = PanelWindow {this, PanelKind::WindTunnel, L" "};

        const int width = rect.right - rect.left;
        const int height = rect.bottom - rect.top;
        panels_[0].hwnd = CreateWindowExW(
            WS_EX_APPWINDOW,
            kWindowClassName,
            panels_[0].title.c_str(),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            rect.left,
            rect.top,
            width,
            height,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            &panels_[0]);
        applyDarkWindowTheme(panels_[0].hwnd);
        windows_[0].store(panels_[0].hwnd);
    }

    void destroyWindows() {
        for (auto& window : windows_) {
            const HWND hwnd = window.exchange(nullptr);
            if (hwnd != nullptr && IsWindow(hwnd)) {
                DestroyWindow(hwnd);
            }
        }
    }

    std::array<HWND, 1> windowsSnapshot() const {
        std::array<HWND, 1> result {};
        for (std::size_t index = 0; index < windows_.size(); ++index) {
            result[index] = windows_[index].load();
        }
        return result;
    }

    static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        auto* panel = reinterpret_cast<PanelWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        if (message == WM_NCCREATE) {
            auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
            panel = static_cast<PanelWindow*>(create->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(panel));
            if (panel != nullptr) {
                panel->hwnd = hwnd;
            }
        }

        if (panel == nullptr || panel->owner == nullptr) {
            return DefWindowProcW(hwnd, message, wparam, lparam);
        }

        switch (message) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            panel->owner->paintPanel(hwnd, panel->kind);
            return 0;
        case WM_DESTROY:
            panel->owner->clearWindow(hwnd);
            if (!panel->owner->hasOpenWindows()) {
                PostQuitMessage(0);
            }
            return 0;
        default:
            return DefWindowProcW(hwnd, message, wparam, lparam);
        }
    }

    void clearWindow(HWND hwnd) {
        for (auto& window : windows_) {
            if (window.load() == hwnd) {
                window.store(nullptr);
            }
        }
    }

    [[nodiscard]] bool hasOpenWindows() const {
        for (const auto& window : windows_) {
            if (window.load() != nullptr) {
                return true;
            }
        }
        return false;
    }

    void paintPanel(HWND hwnd, PanelKind kind) {
        PAINTSTRUCT ps {};
        HDC window_dc = BeginPaint(hwnd, &ps);

        RECT client {};
        GetClientRect(hwnd, &client);

        HDC hdc = CreateCompatibleDC(window_dc);
        HBITMAP buffer = CreateCompatibleBitmap(window_dc, std::max(1L, client.right), std::max(1L, client.bottom));
        HGDIOBJ old_bitmap = SelectObject(hdc, buffer);

        fillRect(hdc, client, RGB(5, 10, 18));
        fillRect(hdc, RECT {0, 0, client.right, client.bottom}, RGB(7, 14, 24));
        HPEN frame_pen = CreatePen(PS_SOLID, 1, RGB(51, 65, 85));
        HGDIOBJ old_pen = SelectObject(hdc, frame_pen);
        HGDIOBJ old_brush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
        Rectangle(hdc, 0, 0, client.right, client.bottom);
        SelectObject(hdc, old_pen);
        SelectObject(hdc, old_brush);
        DeleteObject(frame_pen);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(241, 245, 249));

        HFONT title_font = CreateFontW(24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HFONT body_font = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HFONT old_font = static_cast<HFONT>(SelectObject(hdc, title_font));

        SelectObject(hdc, body_font);

        SimulationMonitorState state {};
        {
            std::lock_guard lock(state_mutex_);
            state = latest_state_;
        }

        switch (kind) {
        case PanelKind::WindTunnel:
            paintWindTunnel(hdc, client, state);
            break;
        }

        SelectObject(hdc, old_font);
        DeleteObject(title_font);
        DeleteObject(body_font);
        BitBlt(window_dc, 0, 0, client.right, client.bottom, hdc, 0, 0, SRCCOPY);
        SelectObject(hdc, old_bitmap);
        DeleteObject(buffer);
        DeleteDC(hdc);
        EndPaint(hwnd, &ps);
    }

    [[nodiscard]] std::wstring panelTitle(PanelKind kind) const {
        switch (kind) {
        case PanelKind::WindTunnel:
            return L"Wind Tunnel Analysis";
        }
        return L"Panel";
    }

    void drawLine(HDC hdc, int x, int y, const std::wstring& text, COLORREF color = RGB(191, 219, 254)) {
        SetTextColor(hdc, color);
        TextOutW(hdc, x, y, text.c_str(), static_cast<int>(text.size()));
    }

    void fillRect(HDC hdc, const RECT& rect, COLORREF color) {
        HBRUSH brush = CreateSolidBrush(color);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }

    void strokeRect(HDC hdc, const RECT& rect, COLORREF color) {
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HGDIOBJ old_pen = SelectObject(hdc, pen);
        HGDIOBJ old_brush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
        SelectObject(hdc, old_pen);
        SelectObject(hdc, old_brush);
        DeleteObject(pen);
    }

    void fillRoundedRect(HDC hdc, const RECT& rect, COLORREF fill, COLORREF border, int radius = 14) {
        HBRUSH brush = CreateSolidBrush(fill);
        HPEN pen = CreatePen(PS_SOLID, 1, border);
        HGDIOBJ old_brush = SelectObject(hdc, brush);
        HGDIOBJ old_pen = SelectObject(hdc, pen);
        RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
        SelectObject(hdc, old_brush);
        SelectObject(hdc, old_pen);
        DeleteObject(brush);
        DeleteObject(pen);
    }

    void drawTextRect(
        HDC hdc,
        const RECT& rect,
        const std::wstring& text,
        COLORREF color,
        UINT format = DT_LEFT | DT_TOP | DT_NOPREFIX | DT_END_ELLIPSIS) {
        RECT draw_rect = rect;
        SetTextColor(hdc, color);
        DrawTextW(hdc, text.c_str(), static_cast<int>(text.size()), &draw_rect, format);
    }

    void drawMetricChip(
        HDC hdc,
        const RECT& rect,
        const std::wstring& label,
        const std::wstring& value,
        COLORREF accent) {
        fillRoundedRect(hdc, rect, RGB(13, 22, 36), RGB(45, 64, 89), 18);
        RECT label_rect {rect.left + 14, rect.top + 8, rect.right - 14, rect.top + 24};
        RECT value_rect {rect.left + 14, rect.top + 24, rect.right - 14, rect.bottom - 8};
        drawTextRect(hdc, label_rect, label, RGB(134, 146, 166), DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        drawTextRect(hdc, value_rect, value, accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }

    void drawKeyValueRow(
        HDC hdc,
        const RECT& rect,
        const std::wstring& label,
        const std::wstring& value,
        COLORREF value_color = RGB(226, 232, 240)) {
        const int split_x = rect.left + 112;
        drawTextRect(
            hdc,
            RECT {rect.left, rect.top, split_x - 8, rect.bottom},
            label,
            RGB(148, 163, 184),
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        drawTextRect(
            hdc,
            RECT {split_x, rect.top, rect.right, rect.bottom},
            value,
            value_color,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }

    void paintWindTunnel(HDC hdc, const RECT& client, const SimulationMonitorState& state) {
        const COLORREF shell_bg = RGB(7, 14, 24);
        const COLORREF surface_bg = RGB(10, 18, 30);
        const COLORREF raised_bg = RGB(14, 24, 39);
        const COLORREF border = RGB(45, 64, 89);
        const COLORREF title = RGB(241, 245, 249);
        const COLORREF muted = RGB(136, 149, 167);
        fillRect(hdc, client, shell_bg);
        fillRoundedRect(hdc, RECT {8, 8, client.right - 8, client.bottom - 8}, RGB(8, 15, 26), RGB(18, 30, 48), 26);

        RECT header_band {18, 18, client.right - 18, 92};
        fillRoundedRect(hdc, header_band, RGB(9, 18, 31), border, 22);
        drawTextRect(
            hdc,
            RECT {header_band.left + 16, header_band.top + 12, header_band.left + 280, header_band.top + 30},
            L"External CFD monitor",
            muted,
            DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);

        const int chip_y = header_band.top + 8;
        const int chip_h = 46;
        const int chip_gap = 10;
        const int chip_w = 146;
        int chip_x = header_band.left + 240;
        drawMetricChip(hdc, RECT {chip_x, chip_y, chip_x + chip_w, chip_y + chip_h}, L"Flow Regime", flowRegimeLabel(state.snapshot), RGB(226, 232, 240));
        chip_x += chip_w + chip_gap;
        drawMetricChip(hdc, RECT {chip_x, chip_y, chip_x + chip_w, chip_y + chip_h}, L"Air Speed", formatNumber(state.snapshot.relative_air_speed_mps, 1) + L" m/s", RGB(56, 189, 248));
        chip_x += chip_w + chip_gap;
        drawMetricChip(hdc, RECT {chip_x, chip_y, chip_x + chip_w, chip_y + chip_h}, L"AoA", formatNumber(state.snapshot.angle_of_attack_deg, 2) + L" deg", RGB(168, 85, 247));
        chip_x += chip_w + chip_gap;
        drawMetricChip(hdc, RECT {chip_x, chip_y, chip_x + chip_w, chip_y + chip_h}, L"q Dynamic", formatNumber(state.snapshot.dynamic_pressure_pa, 0) + L" Pa", RGB(249, 115, 22));
        chip_x += chip_w + chip_gap;
        drawMetricChip(hdc, RECT {chip_x, chip_y, chip_x + chip_w, chip_y + chip_h}, L"Mach", formatNumber(state.snapshot.mach_number, 2), RGB(226, 232, 240));
        chip_x += chip_w + chip_gap;
        drawMetricChip(hdc, RECT {chip_x, chip_y, chip_x + chip_w, chip_y + chip_h}, L"Density", formatNumber(state.snapshot.air_density_kgpm3, 3) + L" kg/m3", RGB(125, 211, 252));
        chip_x += chip_w + chip_gap;
        drawMetricChip(hdc, RECT {chip_x, chip_y, chip_x + chip_w, chip_y + chip_h}, L"Total P", formatNumber(state.snapshot.total_pressure_pa, 0) + L" Pa", RGB(148, 163, 184));

        RECT analysis_panel {18, 118, client.right - 388, client.bottom - 24};
        RECT part_panel {client.right - 352, 118, client.right - 18, client.bottom - 24};
        RECT heatmap {analysis_panel.left, analysis_panel.top, analysis_panel.right, analysis_panel.top + 318};
        RECT profile {analysis_panel.left, analysis_panel.top + 332, analysis_panel.right, analysis_panel.bottom};
        fillRoundedRect(hdc, analysis_panel, surface_bg, border, 18);
        fillRoundedRect(hdc, part_panel, surface_bg, border, 18);
        fillRoundedRect(hdc, heatmap, raised_bg, border, 16);
        fillRoundedRect(hdc, profile, raised_bg, border, 16);

        fillRect(hdc, RECT {heatmap.left + 1, heatmap.top + 1, heatmap.right - 1, heatmap.top + 38}, RGB(13, 23, 39));
        fillRect(hdc, RECT {profile.left + 1, profile.top + 1, profile.right - 1, profile.top + 38}, RGB(13, 23, 39));
        drawTextRect(hdc, RECT {heatmap.left + 14, heatmap.top + 11, heatmap.right - 14, heatmap.top + 31}, L"Altitude / Velocity Load Envelope", title, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        drawTextRect(hdc, RECT {profile.left + 14, profile.top + 11, profile.right - 14, profile.top + 31}, L"Pressure Recovery, Streamlines & Structural Response", title, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        drawTextRect(hdc, RECT {part_panel.left + 14, part_panel.top + 11, part_panel.right - 14, part_panel.top + 31}, L"Telemetry & Component Response", title, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);

        const auto pressure_color = [](double pressure_ratio) {
            const double clamped = std::clamp(pressure_ratio, 0.0, 1.0);
            const int red = static_cast<int>(36.0 + clamped * 216.0);
            const int green = static_cast<int>(96.0 + (1.0 - std::abs(clamped - 0.45) * 1.9) * 132.0);
            const int blue = static_cast<int>(246.0 - clamped * 218.0);
            return RGB(std::clamp(red, 0, 255), std::clamp(green, 0, 255), std::clamp(blue, 0, 255));
        };

        const auto risk_color = [](double risk) {
            const double clamped = std::clamp(risk, 0.0, 1.0);
            if (clamped < 0.34) {
                return RGB(74, 222, 128);
            }
            if (clamped < 0.67) {
                return RGB(251, 191, 36);
            }
            return RGB(248, 113, 113);
        };

        const double air_speed = std::max(state.snapshot.relative_air_speed_mps, 1.0);
        const double altitude = std::max(state.snapshot.state.position_m.z, 0.0);
        const double q_current = std::max(state.snapshot.dynamic_pressure_pa, 0.0);
        const double aoa_abs = std::abs(state.snapshot.angle_of_attack_deg);
        const double q_reference = 35000.0;
        const double max_altitude = std::max(state.snapshot.max_altitude_m, 6000.0);
        const double max_speed = std::max(air_speed * 1.45, 420.0);
        const int cols = 22;
        const int rows = 14;
        const int cell_w = (heatmap.right - heatmap.left - 72) / cols;
        const int cell_h = (heatmap.bottom - heatmap.top - 70) / rows;
        const int grid_left = heatmap.left + 48;
        const int grid_top = heatmap.top + 38;

        for (int row = 0; row < rows; ++row) {
            const double alt_ratio = static_cast<double>(row) / static_cast<double>(rows - 1);
            const double sample_altitude = max_altitude * (1.0 - alt_ratio);
            for (int col = 0; col < cols; ++col) {
                const double speed_ratio = static_cast<double>(col) / static_cast<double>(cols - 1);
                const double sample_speed = max_speed * speed_ratio;
                const double sample_density = airDensityFromSurfaceKgPerM3(state.surface_weather, sample_altitude);
                const double q_sample = 0.5 * sample_density * sample_speed * sample_speed;
                const double load_ratio = std::clamp(q_sample / q_reference, 0.0, 1.0);
                RECT cell {
                    grid_left + col * cell_w,
                    grid_top + row * cell_h,
                    grid_left + (col + 1) * cell_w - 1,
                    grid_top + (row + 1) * cell_h - 1
                };
                fillRect(hdc, cell, pressure_color(load_ratio));
            }
        }

        for (int index = 0; index <= 4; ++index) {
            const int x = grid_left + index * ((cols * cell_w) / 4);
            drawLine(hdc, x, grid_top - 4, std::to_wstring(static_cast<int>(max_speed * static_cast<double>(index) / 4.0)) + L" m/s", RGB(148, 163, 184));
        }
        for (int index = 0; index <= 4; ++index) {
            const int y = grid_top + index * ((rows * cell_h) / 4);
            drawLine(hdc, heatmap.left + 10, y - 8, std::to_wstring(static_cast<int>(max_altitude * (1.0 - static_cast<double>(index) / 4.0))) + L" m", RGB(148, 163, 184));
        }
        drawLine(hdc, heatmap.left + 16, heatmap.bottom - 22, L"Altitude", RGB(100, 116, 139));
        drawLine(hdc, heatmap.right - 130, heatmap.bottom - 22, L"Airspeed / q", RGB(100, 116, 139));

        const int marker_x = grid_left + static_cast<int>(std::clamp(air_speed / max_speed, 0.0, 1.0) * static_cast<double>(cols * cell_w - 1));
        const int marker_y = grid_top + static_cast<int>(std::clamp(1.0 - altitude / max_altitude, 0.0, 1.0) * static_cast<double>(rows * cell_h - 1));
        HBRUSH marker_brush = CreateSolidBrush(RGB(248, 250, 252));
        HGDIOBJ marker_old = SelectObject(hdc, marker_brush);
        Ellipse(hdc, marker_x - 6, marker_y - 6, marker_x + 6, marker_y + 6);
        SelectObject(hdc, marker_old);
        DeleteObject(marker_brush);
        drawLine(hdc, marker_x + 10, marker_y - 8, L"Current state", RGB(248, 250, 252));

        const LONG usable_width = std::max<LONG>(180, profile.right - profile.left - 96);
        const double profile_scale = usable_width / std::max(state.geometry.body_length_m, 0.4);
        const int center_y = profile.top + (profile.bottom - profile.top) / 2 + 12;
        const int nose_tip_x = profile.left + 56;
        const int nose_base_x = nose_tip_x + static_cast<int>(state.geometry.nose_length_m * profile_scale);
        const int body_end_x = profile.right - 72 - static_cast<int>(state.geometry.transition_length_m * profile_scale);
        const int body_radius = std::max(14, static_cast<int>(state.geometry.body_diameter_m * profile_scale * 0.48));
        const int aft_radius = std::max(9, static_cast<int>(state.geometry.transition_aft_diameter_m * profile_scale * 0.48));
        const int body_top = center_y - body_radius;
        const int body_bottom = center_y + body_radius;
        const int fin_front_x = nose_tip_x + static_cast<int>(state.geometry.fin_front_from_nose_m * profile_scale);
        const int fin_root = std::max(20, static_cast<int>(state.geometry.fin_root_chord_m * profile_scale));
        const int fin_tip = std::max(14, static_cast<int>(state.geometry.fin_tip_chord_m * profile_scale));
        const int fin_span = std::max(16, static_cast<int>(state.geometry.fin_span_m * state.geometry.fin_controls.span_scale * profile_scale * 0.72));
        const int fin_tip_front = fin_front_x + static_cast<int>((state.geometry.fin_sweep_length_m + state.geometry.fin_controls.tip_le_offset_m) * profile_scale);
        const int fin_tip_back = fin_tip_front + fin_tip;
        const int tail_end_x = profile.right - 42;
        const double animation_phase = static_cast<double>(GetTickCount64()) * 0.001 *
            (0.45 + std::clamp(state.snapshot.relative_air_speed_mps / 180.0, 0.12, 1.4));

        const double body_radius_m = std::max(state.geometry.body_diameter_m * 0.5, 1e-6);
        const double fin_area =
            0.5 * (state.geometry.fin_root_chord_m + state.geometry.fin_tip_chord_m) *
            (state.geometry.fin_span_m * state.geometry.fin_controls.span_scale) *
            static_cast<double>(std::max(state.geometry.fin_count, 1));
        const double nose_factor = std::clamp((state.geometry.nose_length_m / std::max(state.geometry.body_diameter_m, 1e-6)) * 0.22 + aoa_abs * 0.03, 0.0, 2.5);
        const double body_factor = std::clamp((state.geometry.body_length_m / std::max(state.geometry.body_diameter_m, 1e-6)) * 0.05 + aoa_abs * 0.02, 0.0, 2.5);
        const double transition_factor = std::clamp((state.geometry.transition_length_m / std::max(state.geometry.body_diameter_m, 1e-6)) * 0.18 + aoa_abs * 0.015, 0.0, 2.0);
        const double fin_factor = std::clamp((fin_area / std::max(kPi * body_radius_m * body_radius_m, 1e-6)) * 0.32 + aoa_abs * 0.06, 0.0, 3.2);
        const double payload_factor = std::clamp((state.geometry.payload_length_m / std::max(state.geometry.body_length_m, 1e-6)) * 0.35 + aoa_abs * 0.012, 0.0, 1.4);
        const double motor_factor = std::clamp((state.motor_count * 0.18) + aoa_abs * 0.01, 0.0, 1.6);

        const auto fillPolygonColor = [&](const POINT* points, int count, COLORREF color) {
            HBRUSH brush = CreateSolidBrush(color);
            HGDIOBJ old_brush = SelectObject(hdc, brush);
            Polygon(hdc, points, count);
            SelectObject(hdc, old_brush);
            DeleteObject(brush);
        };

        const auto localPressureRatio = [&](double station_t, double vertical_t, double component_factor, bool fin_region) {
            double ratio = (0.34 + 0.46 * (1.0 - station_t)) * component_factor;
            ratio += std::exp(-std::pow((vertical_t - 0.5) * 2.0, 2.0) * 2.2) * 0.16;
            ratio += std::clamp(state.snapshot.shockwave_intensity, 0.0, 1.0) * 0.22 * std::exp(-std::pow(station_t - 0.18, 2.0) * 18.0);
            if (fin_region) {
                ratio += 0.18 + 0.14 * std::abs(vertical_t - 0.5);
            }
            return std::clamp(ratio, 0.0, 1.0);
        };

        const int saved_heatmap_dc = SaveDC(hdc);
        IntersectClipRect(hdc, profile.left + 1, profile.top + 1, profile.right - 1, profile.bottom - 1);

        const int nose_slices = 28;
        for (int slice = 0; slice < nose_slices; ++slice) {
            const double t0 = static_cast<double>(slice) / static_cast<double>(nose_slices);
            const double t1 = static_cast<double>(slice + 1) / static_cast<double>(nose_slices);
            const int x0 = nose_tip_x + static_cast<int>((nose_base_x - nose_tip_x) * t0);
            const int x1 = nose_tip_x + static_cast<int>((nose_base_x - nose_tip_x) * t1);
            const int y0_top = center_y - static_cast<int>((body_radius) * t0);
            const int y0_bottom = center_y + static_cast<int>((body_radius) * t0);
            const int y1_top = center_y - static_cast<int>((body_radius) * t1);
            const int y1_bottom = center_y + static_cast<int>((body_radius) * t1);
            POINT quad[4] {{x0, y0_top}, {x1, y1_top}, {x1, y1_bottom}, {x0, y0_bottom}};
            const double pressure_ratio = localPressureRatio(t0, 0.5, std::clamp(nose_factor / 2.5, 0.12, 1.0), false);
            fillPolygonColor(quad, 4, pressure_color(pressure_ratio));
        }

        const int body_columns = 54;
        const int body_rows = 6;
        for (int col = 0; col < body_columns; ++col) {
            const double tx0 = static_cast<double>(col) / static_cast<double>(body_columns);
            const double tx1 = static_cast<double>(col + 1) / static_cast<double>(body_columns);
            const int x0 = nose_base_x + static_cast<int>((body_end_x - nose_base_x) * tx0);
            const int x1 = nose_base_x + static_cast<int>((body_end_x - nose_base_x) * tx1);
            for (int row = 0; row < body_rows; ++row) {
                const double ty0 = static_cast<double>(row) / static_cast<double>(body_rows);
                const double ty1 = static_cast<double>(row + 1) / static_cast<double>(body_rows);
                const int y0 = body_top + static_cast<int>((body_bottom - body_top) * ty0);
                const int y1 = body_top + static_cast<int>((body_bottom - body_top) * ty1);
                const double pressure_ratio = localPressureRatio(tx0, 0.5 * (ty0 + ty1), std::clamp(body_factor / 2.5, 0.15, 1.0), false);
                fillRect(hdc, RECT {x0, y0, std::max(x1, x0 + 1), std::max(y1, y0 + 1)}, pressure_color(pressure_ratio));
            }
        }
        strokeRect(hdc, RECT {nose_base_x, body_top, body_end_x, body_bottom}, RGB(214, 222, 233));

        if (state.geometry.transition_length_m > 1e-6) {
            const int tail_slices = 18;
            for (int slice = 0; slice < tail_slices; ++slice) {
                const double t0 = static_cast<double>(slice) / static_cast<double>(tail_slices);
                const double t1 = static_cast<double>(slice + 1) / static_cast<double>(tail_slices);
                const int x0 = body_end_x + static_cast<int>((tail_end_x - body_end_x) * t0);
                const int x1 = body_end_x + static_cast<int>((tail_end_x - body_end_x) * t1);
                const int top0 = center_y - body_radius + static_cast<int>((body_radius - aft_radius) * t0);
                const int top1 = center_y - body_radius + static_cast<int>((body_radius - aft_radius) * t1);
                const int bottom0 = center_y + body_radius - static_cast<int>((body_radius - aft_radius) * t0);
                const int bottom1 = center_y + body_radius - static_cast<int>((body_radius - aft_radius) * t1);
                POINT quad[4] {{x0, top0}, {x1, top1}, {x1, bottom1}, {x0, bottom0}};
                const double pressure_ratio = localPressureRatio(t0, 0.5, std::clamp(transition_factor / 2.0, 0.10, 1.0), false);
                fillPolygonColor(quad, 4, pressure_color(pressure_ratio));
            }
        }

        const int fin_columns = 16;
        for (int col = 0; col < fin_columns; ++col) {
            const double t0 = static_cast<double>(col) / static_cast<double>(fin_columns);
            const double t1 = static_cast<double>(col + 1) / static_cast<double>(fin_columns);
            const int root_x0 = fin_front_x + static_cast<int>(fin_root * t0);
            const int root_x1 = fin_front_x + static_cast<int>(fin_root * t1);
            const int tip_x0 = fin_tip_front + static_cast<int>(fin_tip * t0);
            const int tip_x1 = fin_tip_front + static_cast<int>(fin_tip * t1);
            const double pressure_ratio = localPressureRatio(t0, 0.82, std::clamp(fin_factor / 3.2, 0.18, 1.0), true);

            POINT lower_quad[4] {
                {root_x0, body_bottom},
                {tip_x0, body_bottom + fin_span},
                {tip_x1, body_bottom + fin_span},
                {root_x1, body_bottom}
            };
            POINT upper_quad[4] {
                {root_x0, body_top},
                {tip_x0, body_top - fin_span},
                {tip_x1, body_top - fin_span},
                {root_x1, body_top}
            };
            fillPolygonColor(lower_quad, 4, pressure_color(pressure_ratio));
            fillPolygonColor(upper_quad, 4, pressure_color(pressure_ratio));
        }

        RestoreDC(hdc, saved_heatmap_dc);

        RECT legend_box {profile.right - 154, profile.top + 18, profile.right - 24, profile.top + 64};
        fillRect(hdc, legend_box, RGB(14, 24, 39));
        strokeRect(hdc, legend_box, RGB(51, 65, 85));
        const int legend_left = legend_box.left + 10;
        const int legend_top = legend_box.top + 24;
        const int legend_width = legend_box.right - legend_box.left - 20;
        for (int step = 0; step < legend_width; ++step) {
            const double t = static_cast<double>(step) / std::max(1, legend_width - 1);
            fillRect(hdc, RECT {legend_left + step, legend_top, legend_left + step + 1, legend_top + 10}, pressure_color(t));
        }
        drawLine(hdc, legend_box.left + 10, legend_box.top + 8, L"Surface Pressure Heatmap", RGB(226, 232, 240));
        drawLine(hdc, legend_left, legend_top + 14, L"low", RGB(148, 163, 184));
        drawLine(hdc, legend_box.right - 34, legend_top + 14, L"high", RGB(248, 250, 252));

        POINT nose[3] {{nose_tip_x, center_y}, {nose_base_x, body_top}, {nose_base_x, body_bottom}};
        HPEN shell_pen = CreatePen(PS_SOLID, 1, RGB(214, 222, 233));
        HGDIOBJ old_pen = SelectObject(hdc, shell_pen);
        HGDIOBJ old_brush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
        Polygon(hdc, nose, 3);
        if (state.geometry.transition_length_m > 1e-6) {
            POINT tail_outline[4] {
                {body_end_x, body_top},
                {tail_end_x, center_y - aft_radius},
                {tail_end_x, center_y + aft_radius},
                {body_end_x, body_bottom}
            };
            Polygon(hdc, tail_outline, 4);
        }
        POINT lower_fin[4] {
            {fin_front_x, body_bottom},
            {fin_tip_front, body_bottom + fin_span},
            {fin_tip_back, body_bottom + fin_span},
            {fin_front_x + fin_root, body_bottom}
        };
        POINT upper_fin[4] {
            {fin_front_x, body_top},
            {fin_tip_front, body_top - fin_span},
            {fin_tip_back, body_top - fin_span},
            {fin_front_x + fin_root, body_top}
        };
        Polygon(hdc, lower_fin, 4);
        Polygon(hdc, upper_fin, 4);
        SelectObject(hdc, old_pen);
        SelectObject(hdc, old_brush);
        DeleteObject(shell_pen);

        const bool recovery_flow = state.snapshot.parachute_deployed;

        const auto draw_streamline = [&](double y_bias, double thickness) {
            const double lane_y = center_y + y_bias;
            const double lane_t = std::clamp((lane_y - (profile.top + 20.0)) / std::max(1.0, static_cast<double>(profile.bottom - profile.top - 40)), 0.0, 1.0);
            POINT previous = sampleProfileFlowPoint(
                profile,
                center_y,
                nose_tip_x,
                nose_base_x,
                body_end_x,
                tail_end_x,
                body_radius,
                static_cast<double>(body_end_x - nose_tip_x),
                state.snapshot.angle_of_attack_deg,
                state.snapshot.mach_number,
                state.snapshot.dynamic_pressure_pa,
                state.snapshot.total_pressure_pa,
                lane_t,
                0.0,
                animation_phase);
            for (int segment = 1; segment <= 44; ++segment) {
                const double s = static_cast<double>(segment) / 44.0;
                POINT current = sampleProfileFlowPoint(
                    profile,
                    center_y,
                    nose_tip_x,
                    nose_base_x,
                    body_end_x,
                    tail_end_x,
                    body_radius,
                    static_cast<double>(body_end_x - nose_tip_x),
                    state.snapshot.angle_of_attack_deg,
                    state.snapshot.mach_number,
                    state.snapshot.dynamic_pressure_pa,
                    state.snapshot.total_pressure_pa,
                    lane_t,
                    s,
                    animation_phase);
                const double x_local = static_cast<double>(current.x);
                const double y_local = static_cast<double>(current.y);
                double pressure_ratio = 0.18;
                if (x_local < nose_base_x) {
                    pressure_ratio = std::clamp((1.0 - (x_local - nose_tip_x) / std::max(12.0, static_cast<double>(nose_base_x - nose_tip_x))) * (nose_factor / 2.5), 0.0, 1.0);
                } else if (x_local <= body_end_x) {
                    pressure_ratio = std::clamp((0.35 + 0.25 * std::exp(-std::pow((y_local - center_y) / std::max(12.0, static_cast<double>(body_radius)), 2.0))) * (body_factor / 2.5), 0.0, 1.0);
                } else {
                    pressure_ratio = std::clamp((0.28 + 0.18 * std::exp(-std::pow((y_local - center_y) / std::max(12.0, static_cast<double>(aft_radius)), 2.0))) * (transition_factor / 2.0), 0.0, 1.0);
                }
                if (x_local >= fin_front_x - 10 && x_local <= fin_tip_back + 10) {
                    const double fin_influence =
                        std::exp(-std::pow((x_local - (fin_front_x + fin_root * 0.5)) / 54.0, 2.0)) *
                        std::exp(-std::pow(std::abs(y_local - center_y) - body_radius - fin_span * 0.38, 2.0) / 420.0);
                    pressure_ratio = std::clamp(pressure_ratio + fin_influence * (fin_factor / 3.2), 0.0, 1.0);
                }
                HPEN pen = CreatePen(PS_SOLID, static_cast<int>(thickness), pressure_color(pressure_ratio));
                HGDIOBJ old_pen = SelectObject(hdc, pen);
                MoveToEx(hdc, previous.x, previous.y, nullptr);
                LineTo(hdc, current.x, current.y);
                SelectObject(hdc, old_pen);
                DeleteObject(pen);
                previous = current;
            }
        };

        const int streamline_extent = 5 + static_cast<int>(std::round(std::clamp(state.snapshot.mach_number, 0.0, 1.6) * 2.0));
        for (int line = -streamline_extent; line <= streamline_extent; ++line) {
            const double thickness = line == 0 ? 3.2 : 1.8 + std::clamp(state.snapshot.mach_number * 0.4, 0.0, 0.8);
            draw_streamline(static_cast<double>(line) * 16.0, thickness);
        }

        const int particle_lanes = streamline_extent * 2 + 5;
        const int particles_per_lane = 5 + static_cast<int>(std::round(std::clamp(state.snapshot.dynamic_pressure_pa / 24000.0, 0.0, 1.0) * 5.0 + std::clamp(state.snapshot.mach_number / 1.4, 0.0, 1.0) * 4.0));
        for (int lane = 0; lane < particle_lanes; ++lane) {
            const double lane_t = particle_lanes <= 1 ? 0.5 : static_cast<double>(lane) / static_cast<double>(particle_lanes - 1);
            for (int particle = 0; particle < particles_per_lane; ++particle) {
                const double phase_offset = (static_cast<double>(particle) / std::max(1, particles_per_lane)) + lane_t * 0.19;
                const double progress = std::fmod(phase_offset + animation_phase * (0.18 + std::clamp(state.snapshot.dynamic_pressure_pa / 24000.0, 0.0, 1.0) * 0.16 + std::clamp(state.snapshot.mach_number / 1.4, 0.0, 1.0) * 0.12), 1.0);
                const POINT point = sampleProfileFlowPoint(
                    profile,
                    center_y,
                    nose_tip_x,
                    nose_base_x,
                    body_end_x,
                    tail_end_x,
                    body_radius,
                    static_cast<double>(body_end_x - nose_tip_x),
                    state.snapshot.angle_of_attack_deg,
                    state.snapshot.mach_number,
                    state.snapshot.dynamic_pressure_pa,
                    state.snapshot.total_pressure_pa,
                    lane_t,
                    progress,
                    animation_phase);
                const double brightness = 0.45 + 0.55 * (1.0 - progress);
                COLORREF particle_color = pressure_color(std::clamp(0.24 + state.snapshot.mach_number * 0.28 + state.snapshot.dynamic_pressure_pa / 90000.0, 0.0, 1.0));
                HBRUSH particle_brush = CreateSolidBrush(particle_color);
                HGDIOBJ previous_brush = SelectObject(hdc, particle_brush);
                const int radius = static_cast<int>(std::round(1.5 + brightness * 1.4));
                Ellipse(hdc, point.x - radius, point.y - radius, point.x + radius, point.y + radius);
                SelectObject(hdc, previous_brush);
                DeleteObject(particle_brush);
            }
        }

        const int saved_dc = SaveDC(hdc);
        IntersectClipRect(hdc, profile.left + 1, profile.top + 1, profile.right - 1, profile.bottom - 1);

        const int airflow_lanes = recovery_flow ? 34 : 42;
        const int airflow_traces_per_lane = recovery_flow ? 30 : 28;
        const int airflow_segment_span = recovery_flow ? 4 : 5;
        const int airflow_line_count = airflow_lanes * airflow_traces_per_lane;
        for (int lane = 0; lane < airflow_lanes; ++lane) {
            const double lane_t = airflow_lanes <= 1 ? 0.5 : static_cast<double>(lane) / static_cast<double>(airflow_lanes - 1);
            for (int trace = 0; trace < airflow_traces_per_lane; ++trace) {
                const double trace_phase = static_cast<double>(trace) / static_cast<double>(airflow_traces_per_lane);
                const double start_progress = std::fmod(trace_phase + lane_t * 0.137 + animation_phase * (recovery_flow ? 0.06 : 0.11), 1.0);
                const double end_progress = std::min(start_progress + static_cast<double>(airflow_segment_span) / 240.0, 1.0);
                const POINT a = sampleProfileFlowPoint(
                    profile,
                    center_y,
                    nose_tip_x,
                    nose_base_x,
                    body_end_x,
                    tail_end_x,
                    body_radius,
                    static_cast<double>(body_end_x - nose_tip_x),
                    recovery_flow ? state.snapshot.angle_of_attack_deg * 0.35 : state.snapshot.angle_of_attack_deg,
                    recovery_flow ? std::min(state.snapshot.mach_number, 0.35) : state.snapshot.mach_number,
                    state.snapshot.dynamic_pressure_pa,
                    state.snapshot.total_pressure_pa,
                    lane_t,
                    start_progress,
                    animation_phase);
                const POINT b = sampleProfileFlowPoint(
                    profile,
                    center_y,
                    nose_tip_x,
                    nose_base_x,
                    body_end_x,
                    tail_end_x,
                    body_radius,
                    static_cast<double>(body_end_x - nose_tip_x),
                    recovery_flow ? state.snapshot.angle_of_attack_deg * 0.35 : state.snapshot.angle_of_attack_deg,
                    recovery_flow ? std::min(state.snapshot.mach_number, 0.35) : state.snapshot.mach_number,
                    state.snapshot.dynamic_pressure_pa,
                    state.snapshot.total_pressure_pa,
                    lane_t,
                    end_progress,
                    animation_phase);
                if (!pointInsideRect(a, profile, 6) && !pointInsideRect(b, profile, 6)) {
                    continue;
                }

                const double energy_ratio = std::clamp(
                    (0.22 + state.snapshot.mach_number * 0.18 + state.snapshot.dynamic_pressure_pa / 90000.0) *
                        (1.0 - start_progress * 0.32),
                    0.0,
                    1.0);
                const COLORREF dense_color = pressure_color(energy_ratio);
                HPEN dense_pen = CreatePen(
                    PS_SOLID,
                    recovery_flow ? 1 : (trace % 6 == 0 ? 2 : 1),
                    dense_color);
                HGDIOBJ old_pen = SelectObject(hdc, dense_pen);
                MoveToEx(hdc, a.x, a.y, nullptr);
                LineTo(hdc, b.x, b.y);
                SelectObject(hdc, old_pen);
                DeleteObject(dense_pen);
            }
        }

        RestoreDC(hdc, saved_dc);

        drawLine(
            hdc,
            profile.left + 14,
            profile.bottom - 26,
            L"Airflow field uses >=1000 streak lines plus surface heatmap to reveal impact, deflection and deformation zones.",
            RGB(148, 163, 184));

        const auto deformation_index = [&](double factor, double span_m, double thickness_m, ComponentMaterial material) {
            const double pressure_load = q_current * factor;
            const double slenderness = span_m / std::max(thickness_m, 0.0015);
            const double rigidity = materialDefinition(material).youngs_modulus_gpa;
            return std::clamp((pressure_load / 18000.0) * (slenderness / 24.0) * (16.0 / rigidity), 0.0, 1.0);
        };

        const double nose_deform = deformation_index(nose_factor, state.geometry.nose_length_m, state.geometry.wall_thickness_m, state.geometry.nose_material);
        const double body_deform = deformation_index(body_factor, state.geometry.body_length_m, state.geometry.wall_thickness_m, state.geometry.body_material);
        const double transition_deform = deformation_index(std::max(transition_factor, 0.1), std::max(state.geometry.transition_length_m, 0.08), state.geometry.wall_thickness_m, state.geometry.transition_material);
        const double fin_deform = deformation_index(fin_factor, state.geometry.fin_span_m, state.geometry.fin_thickness_m * state.geometry.fin_controls.thickness_scale, state.geometry.fin_material);
        const double payload_deform = deformation_index(std::max(payload_factor, 0.1), state.geometry.payload_length_m, state.geometry.wall_thickness_m, state.geometry.payload_material);
        const double motor_deform = deformation_index(std::max(motor_factor, 0.1), std::max(state.geometry.transition_length_m, 0.12), state.geometry.wall_thickness_m, state.geometry.body_material);
        const double peak_surface_pressure_kpa =
            q_current * std::max(
                {
                    std::clamp(nose_factor / 2.5, 0.12, 1.0),
                    std::clamp(body_factor / 2.5, 0.15, 1.0),
                    std::clamp(transition_factor / 2.0, 0.10, 1.0),
                    std::clamp(fin_factor / 3.2, 0.18, 1.0)
                }) / 1000.0;

        auto draw_part_card = [&](int y, const wchar_t* title, ComponentType component, ComponentMaterial material, double aero_factor, double deform_factor, const wchar_t* role) {
            RECT card {part_panel.left + 12, y, part_panel.right - 12, y + 86};
            fillRoundedRect(hdc, card, RGB(16, 27, 45), RGB(51, 65, 85), 14);
            const int content_width = card.right - card.left - 24;
            const int aero_width = static_cast<int>(std::clamp(aero_factor / 3.2, 0.0, 1.0) * static_cast<double>(content_width));
            const int deform_width = static_cast<int>(std::clamp(deform_factor, 0.0, 1.0) * static_cast<double>(content_width));
            const double q_limit_pa = estimateComponentDynamicPressureLimitPa(component, state.geometry);
            const double safety_factor = q_limit_pa / std::max(q_current * std::max(aero_factor, 0.22), 1.0);
            const auto definition = materialDefinition(material);
            fillRect(hdc, RECT {card.left + 12, card.top + 46, card.left + 12 + aero_width, card.top + 58}, pressure_color(std::clamp(aero_factor / 3.2, 0.0, 1.0)));
            fillRect(hdc, RECT {card.left + 12, card.top + 62, card.left + 12 + deform_width, card.top + 74}, risk_color(deform_factor));
            fillRect(hdc, RECT {card.left + 1, card.top + 1, card.left + 6, card.bottom - 1}, pressure_color(std::clamp(aero_factor / 3.2, 0.0, 1.0)));
            drawTextRect(
                hdc,
                RECT {card.left + 12, card.top + 10, card.left + 126, card.top + 30},
                title,
                RGB(248, 250, 252),
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            drawTextRect(
                hdc,
                RECT {card.left + 128, card.top + 10, card.right - 12, card.top + 30},
                L"Load " + formatNumber(q_current * aero_factor / 1000.0, 1) + L" kPa",
                pressure_color(std::clamp(aero_factor / 3.2, 0.0, 1.0)),
                DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            drawTextRect(
                hdc,
                RECT {card.left + 12, card.top + 28, card.right - 12, card.top + 44},
                widen(std::string(definition.label)) + L" | q rec " + formatNumber(q_limit_pa / 1000.0, 0) + L" kPa",
                RGB(148, 163, 184),
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            drawTextRect(
                hdc,
                RECT {card.left + 12, card.top + 74, card.right - 12, card.bottom - 8},
                L"SF " + formatNumber(safety_factor, 2) + L" | " + role,
                risk_color(deform_factor),
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        };

        RECT telemetry_box {part_panel.left + 12, part_panel.top + 42, part_panel.right - 12, part_panel.top + 306};
        fillRoundedRect(hdc, telemetry_box, RGB(16, 27, 45), RGB(51, 65, 85), 14);

        int ty = telemetry_box.top + 12;
        drawTextRect(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + 18}, L"Telemetry", title, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        ty += 26;
        const int row_height = 20;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"Time", formatNumber(state.snapshot.time_s, 2) + L" s");
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"Altitude", formatNumber(state.snapshot.state.position_m.z, 1) + L" m");
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"Vertical V", formatNumber(state.snapshot.state.velocity_mps.z, 1) + L" m/s");
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"Air Speed", formatNumber(state.snapshot.relative_air_speed_mps, 1) + L" m/s", RGB(56, 189, 248));
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"Mach", formatNumber(state.snapshot.mach_number, 2));
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"rho / Ps", formatNumber(state.snapshot.air_density_kgpm3, 3) + L" / " + formatNumber(state.snapshot.static_pressure_pa, 0));
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"AoA", formatNumber(state.snapshot.angle_of_attack_deg, 2) + L" deg", RGB(168, 85, 247));
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"q / P0", formatNumber(state.snapshot.dynamic_pressure_pa, 0) + L" / " + formatNumber(state.snapshot.total_pressure_pa, 0), RGB(249, 115, 22));
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"a / Re", formatNumber(state.snapshot.speed_of_sound_mps, 1) + L" / " + formatNumber(reynoldsNumber(state.snapshot, std::max(state.geometry.body_length_m, 0.1)), 0));
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"CFD Lines", std::to_wstring(airflow_line_count));
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"Peak Surf", formatNumber(peak_surface_pressure_kpa, 1) + L" kPa");
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"q Struct", formatNumber(cachedRecommendedMaxDynamicPressurePa(state.geometry) / 1000.0, 1) + L" kPa");
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"Apogee", formatNumber(state.snapshot.max_altitude_m, 1) + L" m");
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"CG / CP", formatNumber(state.snapshot.cg_from_nose_m, 2) + L" / " + formatNumber(state.snapshot.cp_from_nose_m, 2) + L" m");
        ty += row_height + 4;
        drawKeyValueRow(hdc, RECT {telemetry_box.left + 12, ty, telemetry_box.right - 12, ty + row_height}, L"Static M", formatNumber(state.snapshot.static_margin_calibers, 2) + L" cal");

        const int card_start_y = telemetry_box.bottom + 12;
        const int card_step = 94;
        draw_part_card(card_start_y + card_step * 0, L"Nose Cone", ComponentType::NoseCone, state.geometry.nose_material, nose_factor, nose_deform, L"Stagnation and forebody pressure rise.");
        draw_part_card(card_start_y + card_step * 1, L"Body Tube", ComponentType::BodyTube, state.geometry.body_material, body_factor, body_deform, L"Distributed skin drag and shell bending.");
        draw_part_card(card_start_y + card_step * 2, L"Transition", ComponentType::Transition, state.geometry.transition_material, transition_factor, transition_deform, L"Tail pressure recovery and aft load.");
        draw_part_card(card_start_y + card_step * 3, L"Fin Set", ComponentType::FinSet, state.geometry.fin_material, fin_factor, fin_deform, L"AoA-driven lift, flutter and bending risk.");
        draw_part_card(card_start_y + card_step * 4, L"Payload", ComponentType::Payload, state.geometry.payload_material, payload_factor, payload_deform, L"Fore volume affects local pressure balance.");
        draw_part_card(card_start_y + card_step * 5, L"Motor Mount", ComponentType::MotorMount, state.geometry.body_material, motor_factor, motor_deform, L"Wake, base drag and tail structural stress.");
    }

    [[nodiscard]] std::wstring formatNumber(double value, int precision) const {
        wchar_t buffer[64];
        const wchar_t* format = precision == 0 ? L"%.0f" : precision == 1 ? L"%.1f" : precision == 2 ? L"%.2f" : L"%.3f";
        swprintf_s(buffer, format, value);
        return buffer;
    }
#endif

    std::mutex state_mutex_ {};
    SimulationMonitorState latest_state_ {};

#ifdef _WIN32
    std::thread worker_ {};
    std::atomic<bool> running_requested_ {false};
    std::atomic<bool> running_ {false};
    std::array<std::atomic<HWND>, 1> windows_ {};
    std::array<PanelWindow, 1> panels_ {};
    DWORD thread_id_ {};
#endif
};

SimulationMonitor::SimulationMonitor()
    : impl_(std::make_unique<Impl>()) {}

SimulationMonitor::~SimulationMonitor() = default;

SimulationMonitor::SimulationMonitor(SimulationMonitor&&) noexcept = default;

SimulationMonitor& SimulationMonitor::operator=(SimulationMonitor&&) noexcept = default;

void SimulationMonitor::start() {
    impl_->start();
}

void SimulationMonitor::stop() {
    impl_->stop();
}

void SimulationMonitor::publish(const SimulationMonitorState& state) {
    impl_->publish(state);
}

bool SimulationMonitor::isRunning() const noexcept {
    return impl_->isRunning();
}

}  // namespace rocket
