# Struttura Del Progetto

Ultimo aggiornamento: `2026-05-26`

Questo file descrive la struttura reale del repository nello stato attuale.

## Root

- `README.md`: panoramica generale e stato attuale del progetto.
- `PROGRESS_DB.md`: stato implementativo reale e decisioni architetturali.
- `MODELING_GUIDE.md`: guida pratica al workspace di modellazione.
- `GUI_UX_SPECIFICATION.md`: direzione UX del frontend attivo.
- `BALLISTICS_PHYSICS_CORE.md`: fotografia del modello fisico oggi in uso.
- `MATERIALS_DATABASE.md`: materiali hard-coded e impatto sul simulatore.
- `TODO_BACKLOG.md`: backlog aggiornato.
- `sttuttra.md`: questa mappa del codice.

## Build Attiva

La build attiva e` definita in `CMakeLists.txt` e produce:

- libreria statica `rocket_core`
- eseguibile `rocket_sim`

Percorso applicativo attivo:

- `src/main.cpp`
- `src/RocketApp.cpp`
- `src/SimulationMonitor.cpp`
- `src/MeshGenerator.cpp`

## Public Headers

- `include/rocket/RocketApp.hpp`: entrypoint pubblico dell'app.
- `include/rocket/VehicleModel.hpp`: geometria, materiali, preset, recovery e cluster motori.
- `include/rocket/Propulsion.hpp`: motori, cluster e failure state.
- `include/rocket/FlightState.hpp`: stato dinamico del razzo.
- `include/rocket/Environment.hpp`: sito, meteo e atmosfera.
- `include/rocket/Aerodynamics.hpp`: grandezze aero derivate come frame, CG, CP e static margin.
- `include/rocket/Forces.hpp`: somma di forze e momenti del runtime.
- `include/rocket/CfdModule.hpp`: diagnostica CFD-style e augmentation aero.
- `include/rocket/RungeKutta4.hpp`: integrazione numerica.
- `include/rocket/SimulationCore.hpp`: helper core per stato iniziale, snapshot e monitor state.
- `include/rocket/SimulationCaches.hpp`: cache software `L1/L2` per analisi geometriche e derivati ripetuti.
- `include/rocket/SimulationRuntime.hpp`: struttura dati runtime della missione.
- `include/rocket/SimulationEngine.hpp`: stepping, replay, scrub timeline e sampling del runtime.
- `include/rocket/Validation.hpp`: validazione esplicita con `std::expected`.
- `include/rocket/WeatherProvider.hpp`: provider meteo live e fetch HTTP.
- `include/rocket/MeshGenerator.hpp`: mesh procedurali, cache topologiche e editing base.
- `include/rocket/SimulationMonitor.hpp`: stato pubblicato alla finestra esterna.
- `include/rocket/SecureValidation.hpp`: clamp e validazioni di sicurezza.
- `include/rocket/Concepts.hpp`: concepts moderni per API core e template constrained.
- `include/rocket/Units.hpp`: strong-value type di base per alcune unita`.
- `include/rocket/MathTypes.hpp`: vettori, quaternioni e algebra base.
- `include/rocket/PhysicalConstants.hpp`: costanti globali.

## Core Sources

- `src/DesignLibrary.cpp`: preset, materiali, stime di massa e limiti strutturali.
- `src/Environment.cpp`: atmosfera, meteo e vento.
- `src/Aerodynamics.cpp`: CG, CP, frame aero e margine statico.
- `src/Forces.cpp`: drag, lift laterale, recovery, thrust e momenti.
- `src/CfdModule.cpp`: diagnostica realtime del flusso.
- `src/RungeKutta4.cpp`: stepping numerico.
- `src/SimulationCaches.cpp`: implementazione delle cache software `L1/L2`.
- `src/SimulationCore.cpp`: snapshot e adapter puri tra fisica e monitor.
- `src/SimulationEngine.cpp`: runtime della missione estratto dalla UI.
- `src/Validation.cpp`: validazione input e stato simulazione.
- `src/WeatherProvider.cpp`: fetch HTTP e parsing provider meteo.

## Application Sources

- `src/main.cpp`: bootstrap minimale.
- `src/RocketApp.cpp`: loop principale dell'app `raylib`.
- `src/SimulationMonitor.cpp`: finestra esterna `F3`, oggi specializzata sulla camera del vento.
- `src/MeshGenerator.cpp`: costruzione mesh, upload GPU, editing topologico di base.
- `src/app/RocketAppTrajectory.cpp`: modulo estratto per traiettoria, replay ghost e marker di volo.
- `src/app/RocketAppAeroHelpers.cpp`: helper aero/visuali condivisi per il pannello `Wind Tunnel`.

## Internal App Modules

Questi file sono inclusi da `src/RocketApp.cpp` e rappresentano la UI/interaction layer attiva:

- `src/app/RocketAppState.inl`: stato applicativo, runtime missione, helper di rebuild e snapshot.
- `src/app/RocketAppInteraction.inl`: camera, picking, handle, grid, mesh editing, replay.
- `src/app/RocketAppImGui.inl`: shell Dear ImGui "Dark Space" per toolbar, inspector, scenario e analytics.
- `src/app/RocketAppImGui.inl`: shell Dear ImGui "Dark Space" per toolbar, inspector, scenario, analytics e nuovo storico missione.
- `src/app/RocketAppUiCommon.inl`: primitive UI e utility condivise.
- `src/app/RocketAppUiWindTunnel.inl`: blocco dedicato al pannello `Wind Tunnel`, streamline, focus card e legenda heatmap.
- `src/app/RocketAppUiModeling.inl`: pannelli e workflow del workspace `Modelazione`.
- `src/app/RocketAppUiSimulation.inl`: pannelli e workflow del workspace `Simulazione`.

Nota:

- questi `.inl` restano parte della build attiva insieme ai primi moduli estratti `.hpp/.cpp`
- restano un passaggio intermedio di refactor, non la forma finale desiderata

## External Dependencies

- `external/raylib/`: libreria grafica vendorizzata.
- `external/raylib-cpp/`: wrapper C++ vendorizzato.
- `external/imgui/`: Dear ImGui vendorizzato.
- `external/rlImGui/`: bridge tra Dear ImGui e raylib.

## Output Directories

- `build/`: output CMake principale.
