#pragma once

#include <deque>

#include "Camera3D.hpp"
#include "rocket/FlightState.hpp"
#include "rocket/SimulationRuntime.hpp"

double horizontalRangeM(const rocket::Vector3& point);
void drawTrajectory(const std::deque<rocket::TrajectorySample>& history);
void drawReplayGhost(const rocket::SimulationRuntime& runtime);
void drawFlightMarkers3D(const rocket::SimulationRuntime& runtime);
void drawFlightMarkerLabels(const rocket::SimulationRuntime& runtime, const raylib::Camera3D& camera);
