# Rocket Simulator / The Rocket Lab

Desktop prototype in C++23 for procedural rocket modeling, 3D inspection, and flight simulation.

## Current Project State

The current primary application is the `raylib` desktop workspace built from `src/RocketApp.cpp`, with a `Dear ImGui` "Dark Space" control shell layered on top of the live 3D viewport.

Today the project already includes:

- a `Dear ImGui` control shell for the active desktop UI
- a dual workspace desktop app: `Modelazione` and `Simulazione`
- procedural rocket geometry with presets for complete vehicles
- editable components: `Nose Cone`, `Body Tube`, `Transition`, `Fin Set`, `Motor Mount`, `Payload`
- persistent per-component polygonal meshes with vertex/index buffers
- direct mesh editing with `Vertex`, `Edge`, and `Face` selection
- first topology tools: `Extrude Face`, `Bevel Face`, `Loop Cut Edge`
- 6-DOF style runtime state with variable mass, attitude, angular velocity, and RK4 integration
- software `L1/L2` caches in the core for repeated atmosphere, aerodynamic geometry, CFD geometry, and vehicle-derived calculations
- clustered motors with asymmetric failure support and thrust moment generation
- atmosphere, wind-relative velocity, Mach, dynamic pressure, and recovery drag
- material-aware dry mass, inertia, damping, and recommended structural `q` limits
- in-app CFD-inspired flow diagnostics plus a dedicated external `F3` wind-tunnel window on Windows
- mission replay, keyframe review, and manual timeline scrub for `Launch`, `Burnout`, `Apogee`, and `Impact`
- burn timeline and mission-phase labeling now follow the real configured motor-cluster burn duration instead of a fixed boost window
- the active simulation `History` ImGui panel is now isolated in `src/app/RocketAppHistory.hpp/.cpp` for safer gradual UI refactors

## What Is In Build

Built by `CMake` today:

- `rocket_core` static library
- `rocket_sim` target, exported as the single user-facing executable `RocketLab.exe`
- dependencies vendored in `external/raylib` and `external/raylib-cpp`
- UI stack vendored in `external/imgui` and `external/rlImGui`

## Main Features

### Modeling

- project presets: `Research Starter`, `Sport Trainer`, `High Altitude`, `Minimum Diameter`, `Heavy Lift`
- nose profiles: `Conical`, `Tangent Ogive`, `Parabolic`, `LD-Haack`
- fin profiles: `Trapezoidal`, `Elliptical`, `Airfoil`
- transition profiles: `Conical`, `Curved`
- live rebuild of geometry, mass, inertia, drag, and stability markers
- component picking directly in viewport
- local component grid, snap, wireframe, metrics overlay, and reference board
- draggable floating windows for toolbar, outliner, library, properties, status, and reference

### Simulation

- launch/replay/reset controls
- manual timeline scrub with direct comparison against the selected keyframe
- dedicated simulation history window with compact traces for altitude, Mach, `q`, `rho`, `P`, `P0`, and Reynolds
- first vertical `Launch Rail` guidance phase with configurable rail length from the simulation scenario
- fixed, follow, and free simulation camera
- trajectory history and event markers
- clustered motors with failure toggles
- configurable scenario inputs for launch elevation, wind, gust, direction, and recovery
- live fetch of surface weather from `Open-Meteo`
- live fetch path for `OpenWeatherMap` with `OPENWEATHERMAP_API_KEY`
- first multilayer wind profile pass with altitude-band shear and directional veer derived from the active surface weather
- live telemetry for altitude, velocity, Mach, AoA, dynamic pressure, CG/CP, and static margin
- historical telemetry traces in the simulation workspace for altitude, Mach, `q`, density, static/total pressure, and Reynolds trend review
- in-app debug terminal with live cache hit metrics for geometry, aero, CFD, atmosphere, and wind

### Wind Tunnel / Aero Diagnostics

- integrated CFD-inspired view in the simulation workspace
- pressure heatmap projected directly on the 3D rocket mesh
- external `F3` monitor window on Windows only
- flow regime readout, pressure envelope, streamline rendering, component load cards
- per-component structural failure highlighting when stress limits are exceeded
- synthetic shockwave and aeroelastic indicators
- solver/render particle counters from the real-time CFD module

## Controls

### Global

- `F1`: workspace `Modelazione`
- `F2`: workspace `Simulazione`
- `F3`: toggle external wind-tunnel monitor

### Modeling

- `TAB`: enable intentional CAD-like orbit camera
- `1` to `6`: `Select`, `Move`, `Rotate`, `Scale`, `Add Part`, `Measure`
- mouse wheel: zoom
- `MMB`: orbit when CAD camera is active
- `Shift + MMB`: pan when CAD camera is active

### Simulation

- `Space`: play/pause
- `R`: reset mission
- `K`: cycle mission keyframes
- `Ctrl+S`: save current project to `.rlab`
- `Ctrl+Shift+S`: choose save path and save project
- `Ctrl+O`: load a `.rlab` project
- `Ctrl+E`: export text report and trajectory CSV
- simulation scenario panel: cycle weather source and use `Fetch Weather Now`

## Build

### Windows / Visual Studio

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

Canonical output executable:

```text
artifacts/Debug/RocketLab.exe
```

Note:

- `build/` and `build-gha/` remain only as CMake work directories
- runtime tests are emitted separately under `artifacts/tests/Debug/`

## Repository Layout

```text
include/rocket/   Public headers for physics, vehicle, mesh, and app entrypoint
src/              Application, simulation core, mesh generation, monitor window
src/app/          Internal UI/state/interaction modules used by the raylib + Dear ImGui frontend
external/         Vendored dependencies
```

## Documentation Map

- `PROGRESS_DB.md`: current implementation status and decisions
- `MODELING_GUIDE.md`: practical guide for the modeling workspace
- `GUI_UX_SPECIFICATION.md`: current UX direction and implemented behaviors
- `BALLISTICS_PHYSICS_CORE.md`: implemented physics model and next physics gaps
- `MATERIALS_DATABASE.md`: materials used by the simulation and structural estimates
- `RLAB_FORMAT.md`: current project file format and persistence scope
- `TODO_BACKLOG.md`: prioritized technical backlog
- `sttuttra.md`: codebase structure map

## Current Limitations

- the external monitor window is Windows-specific
- only `Open-Meteo` works zero-config today; `OpenWeatherMap` requires `OPENWEATHERMAP_API_KEY`
- weather fetch is manual from the scenario panel, not yet auto-refresh or background sync
- the CFD layer is diagnostic and heuristic, not a full volumetric solver
- the particle CFD neighborhood model is improved but still approximate, not a real Navier-Stokes solver
- the `L1/L2` acceleration is a software cache layer in the simulation core, not direct control of hardware CPU caches
- the Dear ImGui frontend is now the active shell, but some legacy custom panel helpers still remain in the source tree as migration support

## Contributing

See `CONTRIBUTING.md`.
