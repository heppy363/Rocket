# Context & Progress Database

Ultimo aggiornamento: `2026-05-25`

## Executive Summary

The project has now clearly consolidated around the `raylib` desktop application.

Current reality:

- the active executable is `rocket_sim`
- the primary UI is the `raylib` 3D workspace in `src/RocketApp.cpp`
- the active control shell is now `Dear ImGui` via `external/imgui` + `external/rlImGui`
- procedural modeling, simulation, and aero diagnostics are all wired together

## Module Status

### Application Layer

- `RocketApp.cpp`: implemented and active
- `src/app/*.inl`: implemented and actively used by the main app
- `src/app/RocketAppUiTrajectory.inl`: extracted from common UI helpers to keep files bounded and cleaner
- `src/app/RocketAppImGui.inl`: active Dear ImGui shell for the desktop workflow
- `SimulationMonitor.cpp`: implemented; external monitor available on Windows
- F2 analytics now use resizable Dear ImGui hosts while preserving the richer legacy telemetry, trajectory, wind-tunnel, and mission panels
- live telemetry and mission events in F2 now use cleaner native Dear ImGui layout sections, while trajectory and wind tunnel keep the denser visual simulation layer
- a dedicated F2 history window now tracks compact mission traces for altitude, Mach, `q`, `rho`, `P`, `P0`, and Reynolds, reusing runtime trajectory telemetry
- the F2 mission timeline and event labeling now derive boost/coast timing from the real armed motor-cluster burn window, not a hard-coded `2.4 s`
- trajectory overview labels in F2 now stay coherent with the actual inspection mode, including `Scrub`, `Keyframe`, `Replay`, and `Live`
- mission events panels now explicitly switch into `Scrub` inspection language instead of pretending the user is in live flight phase

### Modeling

- dual workspace shell: implemented
- viewport component picking: implemented
- parametric editing of major components: implemented
- presets for full rockets: implemented
- per-component materials: implemented
- local grid and snap: implemented
- wireframe overlay: implemented
- direct mesh vertex dragging: implemented
- mesh selection modes `Vertex / Edge / Face`: implemented
- topology operators `Extrude Face / Bevel Face / Loop Cut Edge`: first implementation completed
- true gizmo-style transform tools for topology editing: not yet implemented
- serialization of topology edits: implemented

### Mesh System

- persistent mesh storage per component: implemented
- face and edge cache generation: implemented
- GPU mesh refresh after edits: implemented
- support for `Nose`, `Body`, `Transition`, `Fin`, `Payload`, `Motor Mount`: implemented
- smooth influence/falloff deformation: not yet implemented
- full loop-cut over continuous edge loops: not yet implemented

### Flight Simulation

- RK4 integrator: implemented
- software `L1/L2` cache layer for repeated core calculations: implemented
- aerodynamic geometry `L1/L2` cache for repeated `CP` lookups: implemented
- CFD geometry `L1/L2` cache for repeated component area / band analysis: implemented
- variable mass during burn: implemented
- thrust force and thrust moment from clustered motors: implemented
- motor failure toggles: implemented
- drag with simple Mach response: implemented
- aerodynamic normal force and restoring moment: implemented
- damping term: implemented
- recovery drag and parachute deployment logic: implemented
- per-component structural failure visualization during F2 simulation: implemented
- key mission events and replay: implemented
- manual timeline scrub with keyframe delta comparison: implemented
- compact historical graphs for altitude, Mach, `q`, `rho`, `P`, `P0`, `Re`: implemented
- rail guidance / launch rod phase: not yet implemented
- layered wind profile and dispersion map: not yet implemented

### Atmosphere & Weather

- launch site and surface weather model: implemented
- atmosphere derived from site and altitude: implemented
- atmosphere `L1/L2` cache for repeated altitude queries: implemented
- live cache hit counters now exposed for geometry, aerodynamics, CFD, atmosphere, and wind in the debug terminal: implemented
- wind-relative velocity: implemented
- provider enum and query URL helper: implemented
- live weather fetch service: implemented
- `Open-Meteo` runtime fetch + parsing: implemented
- `OpenWeatherMap` runtime fetch + parsing: implemented when `OPENWEATHERMAP_API_KEY` is present
- auto refresh / async polling / forecast blending: not yet implemented

### Data & Project Workflow

- `.rlab` project format: first implementation completed
- save/load of geometry, materials, scenario, recovery, motor settings, cluster state: implemented
- save/load of free vertex modifiers: implemented
- save/load of topology overrides for edited meshes: implemented
- text report export: implemented
- trajectory CSV export: implemented
- file picker workflow: implemented

### Aero / CFD Diagnostics

- in-workspace wind tunnel panel: implemented
- wind tunnel panel readability pass with dedicated module split: implemented
- external `F3` wind-tunnel monitor: implemented on Windows
- external `F3` wind-tunnel monitor style pass aligned to the Dear ImGui Dark Space shell without changing CFD logic or data: implemented
- external `F3` wind-tunnel monitor readability pass with stronger padding and darker shell background coverage: implemented
- component-oriented aero cards: implemented
- synthetic shockwave and aeroelastic metrics: implemented
- real-time particle field for CFD-style diagnostics: implemented
- local neighborhood smoothing for particle density/alignment: first robustness pass implemented
- CFD force/moment augmentation into flight model: implemented
- pressure heatmap directly on 3D mesh: implemented
- full CFD solver with convergence model: not yet implemented

## Current User-Facing Capabilities

### Ready To Use

- build and launch desktop app
- choose a preset rocket
- reshape geometry and materials
- inspect CG, CP, static margin, and mass effects live
- configure clustered motors and failures
- simulate launch, burnout, apogee, impact, and replay
- inspect aerodynamic behavior through the simulation workspace and `F3`

### Partially Ready

- topology editing is usable, but still first-generation
- CFD diagnostics are informative, but still heuristic
- pressure heatmap is now available on the 3D mesh, but it is driven by component-band pressure estimates rather than per-face CFD
- weather sources are connected to live providers, but fetch is still manual and synchronous
- mission review now supports manual scrub, keyframe-vs-live deltas, and compact historical plots for the main aero/atmospheric signals

### Not Yet Productized

- project save/load
- export pipeline is available and usable, but still not rich enough for full analysis workflows
- persistent undo/redo
- background online weather sync
- graph-heavy simulation history UI

## Architectural Decisions Locked In

- the primary app path is `raylib`
- the active UI shell is `Dear ImGui`, not the older custom 2D panel renderer
- `main.cpp` must remain a minimal bootstrap
- the physics core stays reusable and separated from the UI shell
- performance optimizations should prefer deterministic software caches before more invasive approximation layers
- mesh generation and simulation remain coupled through `VehicleModel`, not duplicated per frontend
- the external monitor is specialized for aerodynamic diagnostics, not a duplicate full app window

## Immediate Risks / Technical Debt

- the `src/app/*.inl` split is better than the old monolith, but still transitional
- part of the older custom panel code still exists in-tree while the Dear ImGui migration is being completed
- the new `L1/L2` cache layer is exact-keyed, so it avoids drift but does not yet exploit approximate reuse across nearby states
- topology edits are serialized, but a full procedural rebuild can still replace local edits
- the CFD naming can suggest higher fidelity than the current heuristic model actually provides

## Next High-Value Moves

- move `src/app/*.inl` into stronger `.hpp/.cpp` modules
- deepen topology editing beyond the current first operator set
- continue refining CFD particle neighborhoods and pressure visualization fidelity
- add historical simulation graphs and richer comparison tools
- keep expanding comparison tooling now that the first historical graph window is in place
