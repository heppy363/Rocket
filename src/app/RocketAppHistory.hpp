#pragma once

#include "rocket/SimulationCore.hpp"
#include "rocket/SimulationRuntime.hpp"
#include "rocket/VehicleModel.hpp"

namespace rocket::app {

void drawSimulationHistoryPanelImGui(
    const rocket::SimulationRuntime& runtime,
    const rocket::VehicleModel& vehicle,
    const rocket::SimulationSnapshot& snapshot);

}  // namespace rocket::app
