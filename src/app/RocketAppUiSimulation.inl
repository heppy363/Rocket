void drawSimulationTelemetry(const ::Rectangle& bounds, const rocket::SimulationSnapshot& snapshot) {
    drawPanel(bounds, "Telemetria Live");
    drawSectionCaption(
        Rectangle {bounds.x + 16.0f, bounds.y + 46.0f, bounds.width - 32.0f, 30.0f},
        "Lettura immediata",
        "I valori principali sono separati in card per ridurre il rumore visivo.");
    const float col_w = (bounds.width - 44.0f) * 0.5f;
    const float row_h = 40.0f;

    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 82.0f, col_w, row_h}, "Tempo", std::format("{:.2f} s", snapshot.time_s), Color {84, 136, 199, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 82.0f, col_w, row_h}, "Quota", std::format("{:.1f} m", snapshot.state.position_m.z), Color {120, 187, 255, 255});
    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 130.0f, col_w, row_h}, "Velocita verticale", std::format("{:.1f} m/s", snapshot.state.velocity_mps.z), Color {93, 209, 150, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 130.0f, col_w, row_h}, "Velocita aria", std::format("{:.1f} m/s", snapshot.relative_air_speed_mps), Color {84, 180, 224, 255});
    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 178.0f, col_w, row_h}, "Mach", std::format("{:.2f}", snapshot.mach_number), Color {146, 165, 215, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 178.0f, col_w, row_h}, "AoA", std::format("{:.2f} deg", snapshot.angle_of_attack_deg), Color {223, 148, 86, 255});
    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 226.0f, col_w, row_h}, "Densita", std::format("{:.3f} kg/m3", snapshot.air_density_kgpm3), Color {96, 165, 250, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 226.0f, col_w, row_h}, "P statica", std::format("{:.0f} Pa", snapshot.static_pressure_pa), Color {56, 189, 248, 255});
    drawMetricCard(Rectangle {bounds.x + 16.0f, bounds.y + 274.0f, col_w, row_h}, "q consigliata", std::format("{:.0f} kPa", snapshot.recommended_max_dynamic_pressure_pa / 1000.0), Color {251, 191, 36, 255});
    drawMetricCard(Rectangle {bounds.x + 28.0f + col_w, bounds.y + 274.0f, col_w, row_h}, "Safety Factor", std::format("{:.2f}x", snapshot.dynamic_pressure_safety_factor), snapshot.dynamic_pressure_safety_factor >= 1.35 ? Color {74, 222, 128, 255} : snapshot.dynamic_pressure_safety_factor >= 1.0 ? Color {251, 191, 36, 255} : Color {248, 113, 113, 255});
}

void drawMissionKeyframePreview(
    const ::Rectangle& bounds,
    const SimulationRuntime& runtime,
    const rocket::SimulationSnapshot& keyframe_snapshot,
    const rocket::SimulationSnapshot& inspection_snapshot,
    const rocket::SimulationSnapshot& live_snapshot) {
    drawPanel(bounds, "Analisi Keyframe");

    MissionKeyframe keyframe;
    if (!tryGetSelectedMissionKeyframe(runtime, keyframe)) {
        drawInfoCard(
            Rectangle {bounds.x + 14.0f, bounds.y + 44.0f, bounds.width - 28.0f, 58.0f},
            "Keyframe preview inattivo",
            "Premi K per scorrere launch, burnout, apogee e impact con analisi puntuale.",
            Color {56, 189, 248, 255});
        return;
    }

    drawStatusChip(
        Rectangle {bounds.x + 14.0f, bounds.y + 44.0f, 118.0f, 28.0f},
        keyframe.label,
        Color {30, 41, 59, 230},
        keyframe.accent);
    drawStatusChip(
        Rectangle {bounds.x + 140.0f, bounds.y + 44.0f, 138.0f, 28.0f},
        std::format("t = {:.2f} s", keyframe.time_s),
        Color {15, 23, 42, 230},
        Color {100, 116, 139, 220});

    drawMetricCard(Rectangle {bounds.x + 14.0f, bounds.y + 84.0f, 126.0f, 44.0f}, "Quota", std::format("{:.1f} m", keyframe_snapshot.state.position_m.z), keyframe.accent);
    drawMetricCard(Rectangle {bounds.x + 150.0f, bounds.y + 84.0f, 126.0f, 44.0f}, "Velocita aria", std::format("{:.1f} m/s", keyframe_snapshot.relative_air_speed_mps), Color {56, 189, 248, 255});
    drawMetricCard(Rectangle {bounds.x + 14.0f, bounds.y + 136.0f, 126.0f, 44.0f}, "Mach", std::format("{:.2f}", keyframe_snapshot.mach_number), Color {168, 85, 247, 255});
    drawMetricCard(Rectangle {bounds.x + 150.0f, bounds.y + 136.0f, 126.0f, 44.0f}, "AoA", std::format("{:.2f} deg", keyframe_snapshot.angle_of_attack_deg), Color {249, 115, 22, 255});

    DrawText(std::format("Pressione q   {:.0f} Pa", keyframe_snapshot.dynamic_pressure_pa).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 194, 14, Color {226, 232, 240, 255});
    DrawText(std::format("P0 totale    {:.0f} Pa", keyframe_snapshot.total_pressure_pa).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 214, 14, Color {226, 232, 240, 255});
    DrawText(std::format("Margine      {:.2f} cal", keyframe_snapshot.static_margin_calibers).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 234, 14, Color {226, 232, 240, 255});
    DrawText(std::format("CG / CP      {:.2f} / {:.2f} m", keyframe_snapshot.cg_from_nose_m, keyframe_snapshot.cp_from_nose_m).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 254, 14, Color {226, 232, 240, 255});
    DrawText(std::format("q rec        {:.0f} kPa", keyframe_snapshot.recommended_max_dynamic_pressure_pa / 1000.0).c_str(), static_cast<int>(bounds.x) + 14, static_cast<int>(bounds.y) + 274, 14, Color {226, 232, 240, 255});

    const rocket::SimulationSnapshot& comparison_snapshot =
        runtime.scrub_preview_active ? inspection_snapshot : live_snapshot;
    const std::string compare_label =
        runtime.scrub_preview_active ? "Confronto con scrub" : "Confronto con live";
    const auto signed_delta = [](double value, int precision, const char* unit = "") {
        if (precision <= 0) {
            return std::format("{:+.0f}{}", value, unit);
        }
        if (precision == 1) {
            return std::format("{:+.1f}{}", value, unit);
        }
        if (precision == 2) {
            return std::format("{:+.2f}{}", value, unit);
        }
        return std::format("{:+.3f}{}", value, unit);
    };
    drawStatusChip(
        Rectangle {bounds.x + 14.0f, bounds.y + 300.0f, 144.0f, 26.0f},
        compare_label,
        Color {17, 24, 39, 230},
        keyframe.accent);
    drawKeyValueLine(Rectangle {bounds.x + 14.0f, bounds.y + 332.0f, bounds.width - 28.0f, 18.0f}, "Delta quota", signed_delta(comparison_snapshot.state.position_m.z - keyframe_snapshot.state.position_m.z, 1, " m"));
    drawKeyValueLine(Rectangle {bounds.x + 14.0f, bounds.y + 354.0f, bounds.width - 28.0f, 18.0f}, "Delta V aria", signed_delta(comparison_snapshot.relative_air_speed_mps - keyframe_snapshot.relative_air_speed_mps, 1, " m/s"));
    drawKeyValueLine(Rectangle {bounds.x + 14.0f, bounds.y + 376.0f, bounds.width - 28.0f, 18.0f}, "Delta q", signed_delta(comparison_snapshot.dynamic_pressure_pa - keyframe_snapshot.dynamic_pressure_pa, 0, " Pa"));
    drawKeyValueLine(Rectangle {bounds.x + 14.0f, bounds.y + 398.0f, bounds.width - 28.0f, 18.0f}, "Delta margine", signed_delta(comparison_snapshot.static_margin_calibers - keyframe_snapshot.static_margin_calibers, 2, " cal"));
    drawInlineHint(
        Rectangle {bounds.x + 14.0f, bounds.y + bounds.height - 42.0f, bounds.width - 28.0f, 36.0f},
        runtime.scrub_preview_active ? "Trascina la timeline per confrontare questo keyframe con qualsiasi istante della missione." : "Premi K di nuovo per passare al keyframe successivo. Dopo l'ultimo torni alla vista live.",
        keyframe.accent);
}

void drawSimulationScenario(
    const ::Rectangle& bounds,
    AppState& app_state,
    SimulationRuntime& runtime,
    rocket::VehicleModel& vehicle,
    rocket::Environment& environment) {
    drawPanel(bounds, "Scenario di Volo");

    drawSectionCaption(
        Rectangle {bounds.x + 18.0f, bounds.y + 50.0f, bounds.width - 36.0f, 30.0f},
        "Controlli missione",
        "L'azione principale resta visivamente dominante.");

    const ::Rectangle play_button {bounds.x + 18.0f, bounds.y + 92.0f, bounds.width - 36.0f, 38.0f};
    if (drawButton(play_button, runtime.paused ? "Avvia / Riprendi" : "Pausa", !runtime.paused, Color {25, 165, 124, 224}, ButtonStyle::Contained)) {
        if (runtime.paused) {
            runtime.keyframe_preview_active = false;
            runtime.keyframe_preview_index = -1;
            runtime.keyframe_preview_time_s = 0.0;
            rocket::clearScrubPreview(runtime);
        }
        runtime.paused = !runtime.paused;
    }

    const ::Rectangle reset_button {bounds.x + 18.0f, bounds.y + 140.0f, bounds.width - 36.0f, 34.0f};
    if (drawButton(reset_button, "Reset missione", false, Color {84, 136, 199, 224}, ButtonStyle::Outlined)) {
        resetSimulationRuntime(vehicle, runtime);
    }

    const ::Rectangle replay_button {bounds.x + 18.0f, bounds.y + 182.0f, bounds.width - 36.0f, 34.0f};
    if (drawButton(replay_button, runtime.replay_active ? "Ferma replay rotta" : "Avvia replay rotta", runtime.replay_active, Color {84, 136, 199, 224}, runtime.replay_active ? ButtonStyle::Contained : ButtonStyle::Outlined)) {
        runtime.replay_active = !runtime.replay_active;
        runtime.replay_time_s = 0.0;
        runtime.keyframe_preview_active = false;
        runtime.keyframe_preview_index = -1;
        runtime.keyframe_preview_time_s = 0.0;
        rocket::clearScrubPreview(runtime);
    }

    const ::Rectangle marker_button {bounds.x + 18.0f, bounds.y + 224.0f, bounds.width - 36.0f, 34.0f};
    if (drawButton(marker_button, app_state.show_flight_markers ? "Marker visibili" : "Marker nascosti", app_state.show_flight_markers, Color {84, 136, 199, 224}, ButtonStyle::Outlined)) {
        app_state.show_flight_markers = !app_state.show_flight_markers;
    }

    auto site = environment.launchSite();
    auto weather = environment.surfaceWeather();

    float row_y = bounds.y + 284.0f;
    auto apply_environment_step = [&](double& value, double step, const char* label, const char* unit) {
        const int delta = drawStepper(
            Rectangle {bounds.x + 18.0f, row_y, bounds.width - 36.0f, 30.0f},
            label,
            std::format("{:.2f} {}", value, unit));
        if (delta != 0) {
            value += step * static_cast<double>(delta);
            row_y += 40.0f;
            return true;
        }
        row_y += 40.0f;
        return false;
    };

    DrawText("Sito di lancio", static_cast<int>(bounds.x) + 18, static_cast<int>(row_y) - 6, 18, Color {226, 232, 240, 255});
    row_y += 24.0f;
    bool environment_changed = false;
    environment_changed = apply_environment_step(site.latitude_deg, 0.05, "Latitude", "deg") || environment_changed;
    environment_changed = apply_environment_step(site.longitude_deg, 0.05, "Longitude", "deg") || environment_changed;
    environment_changed = apply_environment_step(site.elevation_m, 5.0, "Elevation", "m") || environment_changed;

    DrawText("Meteo al suolo", static_cast<int>(bounds.x) + 18, static_cast<int>(row_y) + 2, 18, Color {226, 232, 240, 255});
    row_y += 28.0f;
    environment_changed = apply_environment_step(weather.temperature_c, 1.0, "Temp", "C") || environment_changed;
    environment_changed = apply_environment_step(weather.pressure_hpa, 1.5, "Pressure", "hPa") || environment_changed;
    environment_changed = apply_environment_step(weather.humidity_percent, 2.0, "Humidity", "%") || environment_changed;
    environment_changed = apply_environment_step(weather.wind_speed_mps, 0.5, "Wind Avg", "m/s") || environment_changed;
    environment_changed = apply_environment_step(weather.wind_direction_deg, 5.0, "Wind Dir", "deg") || environment_changed;
    environment_changed = apply_environment_step(weather.wind_gust_mps, 0.5, "Wind Gust", "m/s") || environment_changed;

    DrawText("Recovery", static_cast<int>(bounds.x) + 18, static_cast<int>(row_y) + 2, 18, Color {226, 232, 240, 255});
    row_y += 28.0f;
    auto& recovery = vehicle.recovery_system;
    const int deploy_alt_delta = drawStepper(
        Rectangle {bounds.x + 18.0f, row_y, bounds.width - 36.0f, 30.0f},
        "Deploy Alt",
        std::format("{:.0f} m", recovery.deployment_altitude_m));
    if (deploy_alt_delta != 0) {
        recovery.deployment_altitude_m += 10.0 * static_cast<double>(deploy_alt_delta);
        environment_changed = true;
    }
    row_y += 40.0f;
    const int chute_area_delta = drawStepper(
        Rectangle {bounds.x + 18.0f, row_y, bounds.width - 36.0f, 30.0f},
        "Chute Area",
        std::format("{:.2f} m2", recovery.parachute_area_m2));
    if (chute_area_delta != 0) {
        recovery.parachute_area_m2 += 0.05 * static_cast<double>(chute_area_delta);
        environment_changed = true;
    }
    row_y += 40.0f;
    const int deploy_delay_delta = drawStepper(
        Rectangle {bounds.x + 18.0f, row_y, bounds.width - 36.0f, 30.0f},
        "Deploy Delay",
        std::format("{:.1f} s", recovery.deployment_delay_s));
    if (deploy_delay_delta != 0) {
        recovery.deployment_delay_s += 0.2 * static_cast<double>(deploy_delay_delta);
        environment_changed = true;
    }
    row_y += 46.0f;

    if (environment_changed) {
        site.latitude_deg = std::clamp(site.latitude_deg, 35.0, 47.5);
        site.longitude_deg = std::clamp(site.longitude_deg, 6.0, 19.0);
        site.elevation_m = std::clamp(site.elevation_m, -50.0, 3500.0);
        weather.temperature_c = std::clamp(weather.temperature_c, -25.0, 45.0);
        weather.pressure_hpa = std::clamp(weather.pressure_hpa, 850.0, 1055.0);
        weather.humidity_percent = std::clamp(weather.humidity_percent, 0.0, 100.0);
        weather.wind_speed_mps = std::clamp(weather.wind_speed_mps, 0.0, 35.0);
        weather.wind_direction_deg = std::fmod(weather.wind_direction_deg + 360.0, 360.0);
        weather.wind_gust_mps = std::clamp(weather.wind_gust_mps, 0.0, 45.0);
        recovery.deployment_altitude_m = std::clamp(recovery.deployment_altitude_m, 30.0, 1200.0);
        recovery.parachute_area_m2 = std::clamp(recovery.parachute_area_m2, 0.1, 8.0);
        recovery.deployment_delay_s = std::clamp(recovery.deployment_delay_s, 0.0, 15.0);
        environment.setLaunchSite(site);
        environment.setSurfaceWeather(weather);
        resetSimulationRuntime(vehicle, runtime);
    }

    const ::Rectangle manual_button {bounds.x + 18.0f, row_y + 4.0f, bounds.width - 36.0f, 30.0f};
    if (drawButton(
            manual_button,
            std::format("Weather Source: {}", rocket::weatherSourceLabel(environment.weatherDataSource())),
            true,
            Color {84, 136, 199, 224},
            ButtonStyle::Outlined)) {
        const auto next_source =
            environment.weatherDataSource() == rocket::WeatherDataSource::Manual
                ? rocket::WeatherDataSource::OpenMeteoReady
                : environment.weatherDataSource() == rocket::WeatherDataSource::OpenMeteoReady
                      ? rocket::WeatherDataSource::OpenWeatherMapReady
                      : rocket::WeatherDataSource::Manual;
        environment.setWeatherDataSource(next_source);
    }
    row_y += 42.0f;

    if (environment.weatherDataSource() != rocket::WeatherDataSource::Manual) {
        const ::Rectangle fetch_weather_button {bounds.x + 18.0f, row_y, bounds.width - 36.0f, 32.0f};
        if (drawButton(
                fetch_weather_button,
                "Fetch Weather Now",
                false,
                Color {25, 165, 124, 224},
                ButtonStyle::Outlined)) {
            const auto fetched = rocket::refreshEnvironmentWeather(environment);
            if (fetched) {
                resetSimulationRuntime(vehicle, runtime);
                setTransientStatus(
                    app_state,
                    std::format(
                        "Meteo aggiornato da {}: {:.1f} C, {:.1f} hPa, vento {:.1f} m/s",
                        fetched->provider_name,
                        fetched->weather.temperature_c,
                        fetched->weather.pressure_hpa,
                        fetched->weather.wind_speed_mps),
                    5.0);
            } else {
                setTransientStatus(
                    app_state,
                    std::format("Fetch meteo fallita: {}", fetched.error()),
                    6.0);
            }
        }
        row_y += 40.0f;
    }

    DrawText("Cluster motori", static_cast<int>(bounds.x) + 18, static_cast<int>(row_y), 18, Color {226, 232, 240, 255});
    for (std::size_t index = 0; index < vehicle.cluster.motorCount(); ++index) {
        const bool failed = vehicle.cluster.motorFailed(index);
        const ::Rectangle motor_button {
            bounds.x + 18.0f,
            row_y + 30.0f + static_cast<float>(index) * 40.0f,
            bounds.width - 36.0f,
            32.0f
        };
        const std::string label = failed ? std::format("Motor {} FAILED", index + 1) : std::format("Motor {} OK", index + 1);
        if (drawButton(motor_button, label, !failed, failed ? Color {198, 82, 82, 224} : Color {84, 136, 199, 224}, failed ? ButtonStyle::Outlined : ButtonStyle::Contained)) {
            vehicle.cluster.setMotorFailed(index, !failed);
            resetSimulationRuntime(vehicle, runtime);
        }
    }

    DrawText(
        std::format("Source  {}", rocket::weatherSourceLabel(environment.weatherDataSource())).c_str(),
        static_cast<int>(bounds.x) + 18,
        static_cast<int>(bounds.y + bounds.height - 34.0f),
        14,
        Color {148, 163, 184, 255});
}

void drawSimulationTimeline(const ::Rectangle& bounds, SimulationRuntime& runtime, const rocket::VehicleModel& vehicle) {
    drawPanel(bounds, "Sequenza Missione");
    const double armed_burn_time_s = vehicle.cluster.maxBurnTimeS();
    const bool boost_available = armed_burn_time_s > 1e-6;
    const double burn_time_s = boost_available ? armed_burn_time_s : 1.0;
    const double timeline_end_s = std::max(
        {
            12.0,
            runtime.time_s,
            runtime.trajectory_history.empty() ? 0.0 : runtime.trajectory_history.back().time_s,
            runtime.burnout_time_s,
            runtime.apogee_time_s,
            runtime.impact_time_s
        });
    const double normalized_progress = std::clamp(runtime.time_s / timeline_end_s, 0.0, 1.0);
    const double burn_progress =
        boost_available ? std::clamp(runtime.time_s / burn_time_s, 0.0, 1.0) : 0.0;

    DrawText("Progressione volo", static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 36, 13, Color {148, 163, 184, 255});

    const ::Rectangle bar {bounds.x + 18.0f, bounds.y + 54.0f, bounds.width - 36.0f, 18.0f};
    DrawRectangleRounded(bar, 0.4f, 6, Color {30, 41, 59, 255});
    DrawRectangleRounded(
        Rectangle {bar.x, bar.y, static_cast<float>(bar.width * normalized_progress), bar.height},
        0.4f,
        6,
        Color {14, 165, 233, 220});

    DrawText(
        boost_available ? "Fase boost" : "Boost non armato",
        static_cast<int>(bounds.x) + 18,
        static_cast<int>(bounds.y) + 76,
        13,
        Color {148, 163, 184, 255});
    const ::Rectangle burn_bar {bounds.x + 18.0f, bounds.y + 92.0f, bounds.width - 36.0f, 12.0f};
    DrawRectangleRounded(burn_bar, 0.4f, 6, Color {39, 39, 42, 255});
    DrawRectangleRounded(
        Rectangle {burn_bar.x, burn_bar.y, static_cast<float>(burn_bar.width * burn_progress), burn_bar.height},
        0.4f,
        6,
        !boost_available ? Color {71, 85, 105, 220}
        : vehicle.cluster.isBurning(runtime.time_s) ? Color {249, 115, 22, 230}
                                                    : Color {82, 82, 91, 230});

    const ::Rectangle scrub_bar {bounds.x + 18.0f, bounds.y + 112.0f, bounds.width - 176.0f, 16.0f};
    DrawRectangleRounded(scrub_bar, 0.45f, 8, Color {17, 24, 39, 250});
    DrawRectangleRoundedLinesEx(scrub_bar, 0.45f, 8, 1.0f, Color {71, 85, 105, 220});

    for (const auto& keyframe : buildMissionKeyframes(runtime)) {
        const float marker_t =
            timeline_end_s <= 1e-6 ? 0.0f : static_cast<float>(std::clamp(keyframe.time_s / timeline_end_s, 0.0, 1.0));
        const float marker_x = scrub_bar.x + scrub_bar.width * marker_t;
        DrawLineEx(
            ::Vector2 {marker_x, scrub_bar.y - 4.0f},
            ::Vector2 {marker_x, scrub_bar.y + scrub_bar.height + 4.0f},
            1.4f,
            keyframe.accent);
    }

    if (runtime.trajectory_history.size() > 1) {
        const ::Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, scrub_bar) &&
            (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonDown(MOUSE_LEFT_BUTTON))) {
            const double ratio =
                std::clamp((mouse.x - scrub_bar.x) / std::max(scrub_bar.width, 1.0f), 0.0f, 1.0f);
            rocket::setScrubPreviewTime(runtime, ratio * timeline_end_s);
        }
    }

    const double inspection_time_s = rocket::currentRenderTime(runtime);
    const float inspection_t =
        timeline_end_s <= 1e-6 ? 0.0f : static_cast<float>(std::clamp(inspection_time_s / timeline_end_s, 0.0, 1.0));
    const float inspection_x = scrub_bar.x + scrub_bar.width * inspection_t;
    DrawCircleV(
        ::Vector2 {inspection_x, scrub_bar.y + scrub_bar.height * 0.5f},
        6.0f,
        runtime.scrub_preview_active ? Color {244, 114, 182, 255}
        : runtime.keyframe_preview_active ? Color {168, 85, 247, 255}
        : runtime.replay_active ? Color {251, 191, 36, 255}
                                : Color {125, 211, 252, 255});

    const ::Rectangle live_button {bounds.x + bounds.width - 146.0f, bounds.y + 104.0f, 128.0f, 28.0f};
    if (drawButton(
            live_button,
            runtime.scrub_preview_active ? "Torna live" : "Scrub pronto",
            runtime.scrub_preview_active,
            Color {236, 72, 153, 220},
            runtime.scrub_preview_active ? ButtonStyle::Contained : ButtonStyle::Outlined) &&
        runtime.scrub_preview_active) {
        rocket::clearScrubPreview(runtime);
    }

    const char* inspection_mode =
        runtime.scrub_preview_active ? "Scrub"
        : runtime.keyframe_preview_active ? "Keyframe"
        : runtime.replay_active ? "Replay"
                                : "Live";
    DrawText(std::format("Tempo missione {:.2f} s | Vista {} {:.2f} / {:.2f} s", runtime.time_s, inspection_mode, inspection_time_s, timeline_end_s).c_str(), static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 138, 16, Color {226, 232, 240, 255});
    if (runtime.time_s < 0.01) {
        DrawText("La timeline si popola quando la missione entra in volo.", static_cast<int>(bounds.x) + 230, static_cast<int>(bounds.y) + 138, 13, Color {148, 163, 184, 255});
    }
}

void drawTrajectoryOverview(const ::Rectangle& bounds, const SimulationRuntime& runtime) {
    drawPanel(bounds, "Panoramica Traiettoria");

    if (runtime.trajectory_history.size() < 2) {
        drawInfoCard(
            Rectangle {bounds.x + 18.0f, bounds.y + 48.0f, bounds.width - 36.0f, 62.0f},
            "Traiettoria non disponibile",
            "Avvia il volo per generare il profilo completo e i marker di burnout, apogeo e impatto.",
            Color {56, 189, 248, 255});
        return;
    }

    const ::Rectangle profile {
        bounds.x + 18.0f,
        bounds.y + 48.0f,
        bounds.width - 220.0f,
        bounds.height - 66.0f
    };
    const ::Rectangle summary {
        bounds.x + bounds.width - 188.0f,
        bounds.y + 48.0f,
        170.0f,
        bounds.height - 66.0f
    };

    DrawRectangleRounded(profile, 0.04f, 8, Color {10, 18, 30, 244});
    DrawRectangleRoundedLinesEx(profile, 0.04f, 8, 1.0f, Color {45, 64, 89, 255});
    DrawRectangleRounded(summary, 0.08f, 8, Color {11, 20, 33, 236});
    DrawRectangleRoundedLinesEx(summary, 0.08f, 8, 1.0f, Color {45, 64, 89, 255});

    double max_range_m = 1.0;
    double max_altitude_m = std::max(runtime.max_altitude_m, 1.0);
    for (const auto& sample : runtime.trajectory_history) {
        max_range_m = std::max(max_range_m, horizontalRangeM(sample.state.position_m));
        max_altitude_m = std::max(max_altitude_m, sample.state.position_m.z);
    }

    for (int index = 0; index <= 4; ++index) {
        const float t = static_cast<float>(index) / 4.0f;
        const float y = profile.y + 12.0f + t * (profile.height - 28.0f);
        DrawLineEx(
            ::Vector2 {profile.x + 10.0f, y},
            ::Vector2 {profile.x + profile.width - 10.0f, y},
            1.0f,
            index == 4 ? Color {71, 85, 105, 180} : Color {51, 65, 85, 100});
        const double altitude_label = max_altitude_m * (1.0 - static_cast<double>(t));
        DrawText(std::format("{:.0f} m", altitude_label).c_str(), static_cast<int>(profile.x) + 12, static_cast<int>(y) - 16, 13, Color {100, 116, 139, 255});
    }

    auto profile_point = [&](const rocket::Vector3& position_m) {
        const double range_ratio = std::clamp(horizontalRangeM(position_m) / max_range_m, 0.0, 1.0);
        const double altitude_ratio = std::clamp(position_m.z / max_altitude_m, 0.0, 1.0);
        return ::Vector2 {
            profile.x + 18.0f + static_cast<float>(range_ratio) * (profile.width - 36.0f),
            profile.y + profile.height - 18.0f - static_cast<float>(altitude_ratio) * (profile.height - 36.0f)
        };
    };

    for (std::size_t index = 1; index < runtime.trajectory_history.size(); ++index) {
        const auto a = profile_point(runtime.trajectory_history[index - 1].state.position_m);
        const auto b = profile_point(runtime.trajectory_history[index].state.position_m);
        const unsigned char alpha = static_cast<unsigned char>(70 + (170 * static_cast<int>(index)) / static_cast<int>(runtime.trajectory_history.size()));
        DrawLineEx(a, b, 2.0f, Color {64, 196, 255, alpha});
    }

    auto draw_marker = [&](const rocket::Vector3& position_m, const char* label, Color color) {
        const ::Vector2 point = profile_point(position_m);
        DrawCircleV(point, 5.0f, color);
        DrawText(label, static_cast<int>(point.x) + 8, static_cast<int>(point.y) - 10, 13, color);
    };

    draw_marker({0.0, 0.0, 0.0}, "Launch", Color {34, 197, 94, 255});
    if (runtime.burnout_recorded) {
        draw_marker(runtime.burnout_point_m, "Burnout", Color {249, 115, 22, 255});
    }
    if (runtime.apogee_recorded) {
        draw_marker(runtime.apogee_point_m, "Apogee", Color {168, 85, 247, 255});
    }
    if (runtime.impact_recorded) {
        draw_marker(runtime.impact_point_m, "Impact", Color {239, 68, 68, 255});
    }

    const rocket::FlightState render_state = rocket::currentRenderState(runtime);
    const ::Vector2 current_point = profile_point(render_state.position_m);
    const char* inspection_mode =
        runtime.scrub_preview_active ? "Scrub"
        : runtime.keyframe_preview_active ? "Keyframe"
        : runtime.replay_active ? "Replay"
                                : "Current";
    const Color inspection_color =
        runtime.scrub_preview_active ? Color {244, 114, 182, 255}
        : runtime.keyframe_preview_active ? Color {168, 85, 247, 255}
        : runtime.replay_active ? Color {251, 191, 36, 255}
                                : Color {125, 211, 252, 255};
    DrawCircleV(current_point, 6.0f, inspection_color);
    DrawText(
        inspection_mode,
        static_cast<int>(current_point.x) + 8,
        static_cast<int>(current_point.y) + 4,
        13,
        inspection_color);

    DrawText("0 m", static_cast<int>(profile.x) + 18, static_cast<int>(profile.y + profile.height - 16.0f), 13, Color {100, 116, 139, 255});
    DrawText(std::format("{:.0f} m range", max_range_m).c_str(), static_cast<int>(profile.x + profile.width - 120.0f), static_cast<int>(profile.y + profile.height - 16.0f), 13, Color {100, 116, 139, 255});

    float row_y = summary.y + 12.0f;
    drawKeyValueLine(
        Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f},
        "Modalita",
        runtime.scrub_preview_active ? "Scrub"
        : runtime.keyframe_preview_active ? "Keyframe"
        : runtime.replay_active ? "Replay"
                                : "Live");
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Tempo volo", std::format("{:.2f} s", runtime.time_s));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Quota attuale", std::format("{:.1f} m", render_state.position_m.z));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Downrange", std::format("{:.1f} m", horizontalRangeM(render_state.position_m)));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Quota max", std::format("{:.1f} m", runtime.apogee_point_m.z));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Range impatto", std::format("{:.1f} m", horizontalRangeM(runtime.impact_point_m)));
    row_y += 24.0f;
    drawKeyValueLine(Rectangle {summary.x + 12.0f, row_y, summary.width - 24.0f, 20.0f}, "Campioni", std::format("{}", runtime.trajectory_history.size()));
    row_y += 34.0f;

    DrawText("Eventi chiave", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 16, Color {226, 232, 240, 255});
    row_y += 24.0f;
    DrawText("Green  Launch", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 14, Color {34, 197, 94, 255});
    row_y += 20.0f;
    DrawText("Orange Burnout", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 14, Color {249, 115, 22, 255});
    row_y += 20.0f;
    DrawText("Purple Apogee", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 14, Color {168, 85, 247, 255});
    row_y += 20.0f;
    DrawText("Red    Impact", static_cast<int>(summary.x) + 12, static_cast<int>(row_y), 14, Color {239, 68, 68, 255});
}

void drawSimulationEvents(const ::Rectangle& bounds, const SimulationRuntime& runtime, const rocket::VehicleModel& vehicle, const rocket::SimulationSnapshot& snapshot) {
    drawPanel(bounds, "Stato Missione");
    const double burn_time_s = vehicle.cluster.maxBurnTimeS();
    const bool boost_available = burn_time_s > 1e-6;

    std::string primary_event = "Awaiting launch";
    if (runtime.scrub_preview_active) {
        primary_event = "Scrub inspection mode";
    } else if (runtime.keyframe_preview_active) {
        primary_event = "Keyframe analysis mode";
    } else if (runtime.time_s > 0.0 && vehicle.cluster.isBurning(runtime.time_s)) {
        primary_event = "Boost phase active";
    } else if (boost_available && runtime.time_s > burn_time_s && snapshot.state.position_m.z > 0.0 && snapshot.state.velocity_mps.z > 0.0) {
        primary_event = "Coast to apogee";
    } else if (snapshot.parachute_deployed) {
        primary_event = "Recovery descent";
    } else if (snapshot.state.position_m.z > 0.0 && snapshot.state.velocity_mps.z < -0.5) {
        primary_event = "Ballistic descent";
    } else if (snapshot.max_altitude_m > 0.0 && std::abs(snapshot.state.velocity_mps.z) < 0.5) {
        primary_event = "Apogee window";
    } else if (snapshot.state.position_m.z <= 0.0 && runtime.time_s > 0.5) {
        primary_event = "Ground contact";
    }

    std::string next_action = "Prossimo passo: premi Avvia / Riprendi per iniziare la missione.";
    if (runtime.scrub_preview_active) {
        next_action = "Scrub attivo: trascina la timeline per confrontare quota, eventi e assetto in qualsiasi istante.";
    } else if (runtime.keyframe_preview_active) {
        next_action = "Analisi puntuale attiva: premi K per il prossimo keyframe oppure SPACE per restare in pausa.";
    } else if (!boost_available && runtime.time_s <= 0.01) {
        next_action = "Nessun motore armato: abilita almeno un motore dal pannello Scenario prima di avviare la missione.";
    } else if (runtime.time_s > 0.0 && vehicle.cluster.isBurning(runtime.time_s)) {
        next_action = "Monitoraggio: osserva telemetria, marker e traiettoria mentre il boost e attivo.";
    } else if (runtime.replay_active) {
        next_action = "Replay attivo: usa la panoramica per leggere eventi chiave e range.";
    } else if (snapshot.parachute_deployed) {
        next_action = "Recovery in corso: confronta quota, rateo di discesa e zona di impatto.";
    } else if (runtime.impact_recorded) {
        next_action = "Missione completata: avvia un replay o modifica lo scenario per il prossimo test.";
    }

    DrawText(primary_event.c_str(), static_cast<int>(bounds.x) + 18, static_cast<int>(bounds.y) + 48, 22, Color {248, 250, 252, 255});
    DrawText(
        snapshot.static_margin_calibers >= 1.0 ? "Stabilita nominale" : "Margine statico basso",
        static_cast<int>(bounds.x) + 18,
        static_cast<int>(bounds.y) + 82,
        16,
        snapshot.static_margin_calibers >= 1.0 ? Color {74, 222, 128, 255} : Color {248, 113, 113, 255});
    drawMetricCard(
        Rectangle {bounds.x + 18.0f, bounds.y + 112.0f, bounds.width - 36.0f, 42.0f},
        "Profilo ogiva",
        snapshot.nose_shape_label,
        Color {56, 189, 248, 255});
    drawMetricCard(
        Rectangle {bounds.x + 18.0f, bounds.y + 160.0f, bounds.width - 36.0f, 42.0f},
        "Profilo pinne",
        snapshot.fin_shape_label,
        Color {168, 85, 247, 255});
    DrawText(
        snapshot.parachute_deployed ? "Recovery dispiegato" : "Recovery ancora non dispiegato",
        static_cast<int>(bounds.x) + 18,
        static_cast<int>(bounds.y) + 214,
        15,
        snapshot.parachute_deployed ? Color {74, 222, 128, 255} : Color {148, 163, 184, 255});
    drawInlineHint(
        Rectangle {bounds.x + 18.0f, bounds.y + 236.0f, bounds.width - 36.0f, 26.0f},
        next_action,
        Color {251, 191, 36, 255});
}


