#pragma once

#include <string>

#include "rocket/Aerodynamics.hpp"
#include "rocket/CfdModule.hpp"
#include "rocket/Environment.hpp"
#include "rocket/SimulationCaches.hpp"
#include "rocket/SimulationMonitor.hpp"

namespace rocket {

struct DebugTelemetrySnapshot {
    double sample_age_s {};
    double fps {};
    double frame_time_ms {};
    double process_cpu_percent {};
    double system_cpu_percent {};
    double process_working_set_mb {};
    double process_private_mb {};
    double process_commit_mb {};
    double total_physical_mb {};
    double used_physical_mb {};
    double total_commit_mb {};
    double used_commit_mb {};
    double gpu_local_used_mb {};
    double gpu_local_budget_mb {};
    double gpu_shared_used_mb {};
    double gpu_shared_budget_mb {};
    bool gpu_metrics_available {};
    std::string gpu_adapter_name {"Unavailable"};
    AerodynamicsCacheStats aerodynamics_cache {};
    CfdCacheStats cfd_cache {};
    SimulationCacheStats simulation_cache {};
    EnvironmentCacheStats environment_cache {};
    SimulationSnapshot snapshot {};
    std::string console_text {};
};

class DebugTelemetryCollector {
public:
    DebugTelemetryCollector();

    [[nodiscard]] DebugTelemetrySnapshot capture(
        const Environment& environment,
        const SimulationSnapshot& snapshot,
        float frame_time_s) noexcept;

private:
    unsigned long processor_count_ {1};
    unsigned long long previous_sample_tick_ms_ {};
    unsigned long long previous_system_idle_ {};
    unsigned long long previous_system_kernel_ {};
    unsigned long long previous_system_user_ {};
    unsigned long long previous_process_kernel_ {};
    unsigned long long previous_process_user_ {};
    bool cpu_baseline_ready_ {false};
};

}  // namespace rocket
