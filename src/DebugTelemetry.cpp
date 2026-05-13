#include "rocket/DebugTelemetry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <format>

#ifdef _WIN32
#include <windows.h>
#include <dxgi1_4.h>
#include <psapi.h>
#endif

namespace rocket {

namespace {

template <typename StatsT>
double cacheHitRatePercent(const StatsT& stats) noexcept {
    const double requests = static_cast<double>(stats.l1_hits + stats.l2_hits + stats.misses);
    if (requests <= 0.0) {
        return 0.0;
    }
    return (static_cast<double>(stats.l1_hits + stats.l2_hits) / requests) * 100.0;
}

#ifdef _WIN32
unsigned long long fileTimeToUint64(const FILETIME& value) noexcept {
    return (static_cast<unsigned long long>(value.dwHighDateTime) << 32ULL) |
           static_cast<unsigned long long>(value.dwLowDateTime);
}

struct GpuMemorySnapshot {
    double local_used_mb {};
    double local_budget_mb {};
    double shared_used_mb {};
    double shared_budget_mb {};
    bool available {};
    std::string adapter_name {"Unavailable"};
};

GpuMemorySnapshot queryGpuMemorySnapshot() noexcept {
    GpuMemorySnapshot snapshot;

    IDXGIFactory1* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        return snapshot;
    }

    IDXGIAdapter1* adapter = nullptr;
    for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index) {
        DXGI_ADAPTER_DESC1 desc {};
        if (FAILED(adapter->GetDesc1(&desc)) || (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) {
            adapter->Release();
            adapter = nullptr;
            continue;
        }
        break;
    }
    factory->Release();

    if (adapter == nullptr) {
        return snapshot;
    }

    DXGI_ADAPTER_DESC1 desc {};
    adapter->GetDesc1(&desc);
    char adapter_name[128] {};
    std::wcstombs(adapter_name, desc.Description, std::size(adapter_name) - 1U);
    snapshot.adapter_name = adapter_name;

    IDXGIAdapter3* adapter3 = nullptr;
    if (SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&adapter3)))) {
        DXGI_QUERY_VIDEO_MEMORY_INFO local_info {};
        DXGI_QUERY_VIDEO_MEMORY_INFO shared_info {};
        if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local_info)) &&
            SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &shared_info))) {
            snapshot.local_used_mb = static_cast<double>(local_info.CurrentUsage) / (1024.0 * 1024.0);
            snapshot.local_budget_mb = static_cast<double>(local_info.Budget) / (1024.0 * 1024.0);
            snapshot.shared_used_mb = static_cast<double>(shared_info.CurrentUsage) / (1024.0 * 1024.0);
            snapshot.shared_budget_mb = static_cast<double>(shared_info.Budget) / (1024.0 * 1024.0);
            snapshot.available = true;
        }
        adapter3->Release();
    }

    adapter->Release();
    return snapshot;
}
#endif

std::string buildConsoleText(const DebugTelemetrySnapshot& snapshot) {
    std::array<char, 8192> buffer {};
    const auto& geometry_cache = snapshot.simulation_cache.geometry;
    const auto& atmosphere_cache = snapshot.environment_cache.atmosphere;
    const auto& wind_cache = snapshot.environment_cache.wind;

    const int written = std::snprintf(
        buffer.data(),
        buffer.size(),
        "ROCKET DEBUG TERMINAL [LIVE]\n"
        "rocket@workstation> sample_age........... %6.3f s\n"
        "rocket@workstation> render.fps........... %6.1f fps\n"
        "rocket@workstation> render.frame_time.... %6.2f ms\n"
        "\n"
        "rocket@workstation> cpu.process......... %6.1f %%\n"
        "rocket@workstation> cpu.system.......... %6.1f %%\n"
        "rocket@workstation> ram.used............ %6.0f / %6.0f MB\n"
        "rocket@workstation> commit.used......... %6.0f / %6.0f MB\n"
        "rocket@workstation> proc.working_set.... %6.1f MB\n"
        "rocket@workstation> proc.private........ %6.1f MB\n"
        "rocket@workstation> proc.commit......... %6.1f MB\n"
        "\n"
        "rocket@workstation> gpu.adapter......... %s\n"
        "rocket@workstation> gpu.vram.local...... %6.0f / %6.0f MB\n"
        "rocket@workstation> gpu.vram.shared..... %6.0f / %6.0f MB\n"
        "\n"
        "rocket@workstation> cache.geometry...... L1=%zu L2=%zu/%zu MISS=%zu HIT=%5.1f%% WR=%zu\n"
        "rocket@workstation> cache.atmosphere.... L1=%zu L2=%zu/%zu MISS=%zu HIT=%5.1f%% WR=%zu\n"
        "rocket@workstation> cache.wind.......... L1=%zu L2=%zu/%zu MISS=%zu HIT=%5.1f%% WR=%zu\n"
        "\n"
        "rocket@workstation> sim.time........... %6.2f s\n"
        "rocket@workstation> sim.altitude....... %6.1f m\n"
        "rocket@workstation> sim.velocity....... %6.1f m/s\n"
        "rocket@workstation> sim.q_dynamic...... %6.0f Pa\n"
        "rocket@workstation> sim.mach........... %6.2f\n"
        "rocket@workstation> sim.cfd_solver..... %6d particles\n"
        "rocket@workstation> sim.cfd_render..... %6d particles\n",
        snapshot.sample_age_s,
        snapshot.fps,
        snapshot.frame_time_ms,
        snapshot.process_cpu_percent,
        snapshot.system_cpu_percent,
        snapshot.used_physical_mb,
        snapshot.total_physical_mb,
        snapshot.used_commit_mb,
        snapshot.total_commit_mb,
        snapshot.process_working_set_mb,
        snapshot.process_private_mb,
        snapshot.process_commit_mb,
        snapshot.gpu_adapter_name.c_str(),
        snapshot.gpu_local_used_mb,
        snapshot.gpu_local_budget_mb,
        snapshot.gpu_shared_used_mb,
        snapshot.gpu_shared_budget_mb,
        geometry_cache.l1_hits,
        geometry_cache.l2_valid_entries,
        geometry_cache.l2_capacity,
        geometry_cache.misses,
        cacheHitRatePercent(geometry_cache),
        geometry_cache.writes,
        atmosphere_cache.l1_hits,
        atmosphere_cache.l2_valid_entries,
        atmosphere_cache.l2_capacity,
        atmosphere_cache.misses,
        cacheHitRatePercent(atmosphere_cache),
        atmosphere_cache.writes,
        wind_cache.l1_hits,
        wind_cache.l2_valid_entries,
        wind_cache.l2_capacity,
        wind_cache.misses,
        cacheHitRatePercent(wind_cache),
        wind_cache.writes,
        snapshot.snapshot.time_s,
        snapshot.snapshot.state.position_m.z,
        snapshot.snapshot.relative_air_speed_mps,
        snapshot.snapshot.dynamic_pressure_pa,
        snapshot.snapshot.mach_number,
        snapshot.snapshot.cfd_solver_particle_count,
        snapshot.snapshot.cfd_render_particle_count);

    const std::size_t safe_size = written < 0 ? 0U : std::min<std::size_t>(static_cast<std::size_t>(written), buffer.size() - 1U);
    return std::string(buffer.data(), safe_size);
}

}  // namespace

DebugTelemetryCollector::DebugTelemetryCollector() {
#ifdef _WIN32
    SYSTEM_INFO system_info {};
    GetSystemInfo(&system_info);
    processor_count_ = system_info.dwNumberOfProcessors > 0 ? system_info.dwNumberOfProcessors : 1UL;
#endif
}

DebugTelemetrySnapshot DebugTelemetryCollector::capture(
    const Environment& environment,
    const SimulationSnapshot& snapshot,
    float frame_time_s) noexcept {
    DebugTelemetrySnapshot telemetry;
    telemetry.snapshot = snapshot;
    telemetry.frame_time_ms = static_cast<double>(frame_time_s) * 1000.0;
    telemetry.fps = frame_time_s > 0.0f ? 1.0 / static_cast<double>(frame_time_s) : 0.0;
    telemetry.simulation_cache = simulationCacheStats();
    telemetry.environment_cache = environment.cacheStats();

#ifdef _WIN32
    MEMORYSTATUSEX memory_status {};
    memory_status.dwLength = sizeof(memory_status);
    if (GlobalMemoryStatusEx(&memory_status) != 0) {
        telemetry.total_physical_mb = static_cast<double>(memory_status.ullTotalPhys) / (1024.0 * 1024.0);
        telemetry.used_physical_mb =
            static_cast<double>(memory_status.ullTotalPhys - memory_status.ullAvailPhys) / (1024.0 * 1024.0);
        telemetry.total_commit_mb = static_cast<double>(memory_status.ullTotalPageFile) / (1024.0 * 1024.0);
        telemetry.used_commit_mb =
            static_cast<double>(memory_status.ullTotalPageFile - memory_status.ullAvailPageFile) / (1024.0 * 1024.0);
    }

    PROCESS_MEMORY_COUNTERS_EX process_memory {};
    if (GetProcessMemoryInfo(
            GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&process_memory),
            sizeof(process_memory)) != 0) {
        telemetry.process_working_set_mb = static_cast<double>(process_memory.WorkingSetSize) / (1024.0 * 1024.0);
        telemetry.process_private_mb = static_cast<double>(process_memory.PrivateUsage) / (1024.0 * 1024.0);
        telemetry.process_commit_mb = static_cast<double>(process_memory.PagefileUsage) / (1024.0 * 1024.0);
    }

    FILETIME idle_time {};
    FILETIME kernel_time {};
    FILETIME user_time {};
    FILETIME process_creation_time {};
    FILETIME process_exit_time {};
    FILETIME process_kernel_time {};
    FILETIME process_user_time {};
    if (GetSystemTimes(&idle_time, &kernel_time, &user_time) != 0 &&
        GetProcessTimes(
            GetCurrentProcess(),
            &process_creation_time,
            &process_exit_time,
            &process_kernel_time,
            &process_user_time) != 0) {
        const unsigned long long idle = fileTimeToUint64(idle_time);
        const unsigned long long kernel = fileTimeToUint64(kernel_time);
        const unsigned long long user = fileTimeToUint64(user_time);
        const unsigned long long process_kernel = fileTimeToUint64(process_kernel_time);
        const unsigned long long process_user = fileTimeToUint64(process_user_time);
        const unsigned long long tick_ms = GetTickCount64();

        if (cpu_baseline_ready_) {
            const unsigned long long system_kernel_delta = kernel - previous_system_kernel_;
            const unsigned long long system_user_delta = user - previous_system_user_;
            const unsigned long long system_idle_delta = idle - previous_system_idle_;
            const unsigned long long process_kernel_delta = process_kernel - previous_process_kernel_;
            const unsigned long long process_user_delta = process_user - previous_process_user_;
            const unsigned long long total_delta = system_kernel_delta + system_user_delta;
            const unsigned long long busy_delta = total_delta > system_idle_delta ? total_delta - system_idle_delta : 0ULL;
            const unsigned long long process_delta = process_kernel_delta + process_user_delta;

            if (total_delta > 0ULL) {
                telemetry.system_cpu_percent =
                    std::clamp((static_cast<double>(busy_delta) / static_cast<double>(total_delta)) * 100.0, 0.0, 100.0);
                telemetry.process_cpu_percent =
                    std::clamp((static_cast<double>(process_delta) / static_cast<double>(total_delta)) * 100.0, 0.0, 100.0);
            }
            telemetry.sample_age_s =
                static_cast<double>(tick_ms - previous_sample_tick_ms_) / 1000.0;
        } else {
            cpu_baseline_ready_ = true;
        }

        previous_sample_tick_ms_ = tick_ms;
        previous_system_idle_ = idle;
        previous_system_kernel_ = kernel;
        previous_system_user_ = user;
        previous_process_kernel_ = process_kernel;
        previous_process_user_ = process_user;
    }

    const GpuMemorySnapshot gpu_snapshot = queryGpuMemorySnapshot();
    telemetry.gpu_metrics_available = gpu_snapshot.available;
    telemetry.gpu_adapter_name = gpu_snapshot.adapter_name;
    telemetry.gpu_local_used_mb = gpu_snapshot.local_used_mb;
    telemetry.gpu_local_budget_mb = gpu_snapshot.local_budget_mb;
    telemetry.gpu_shared_used_mb = gpu_snapshot.shared_used_mb;
    telemetry.gpu_shared_budget_mb = gpu_snapshot.shared_budget_mb;
#endif

    telemetry.console_text = buildConsoleText(telemetry);
    return telemetry;
}

}  // namespace rocket
