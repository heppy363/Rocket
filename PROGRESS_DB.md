# Context & Progress Database

Ultimo aggiornamento: `2026-05-12`

## Executive Summary

The project has now clearly consolidated around the `raylib` desktop application.

Current reality:

- the active executable is `rocket_sim`
- the primary UI is the 3D workspace in `src/RocketApp.cpp`
- procedural modeling, simulation, and aero diagnostics are all wired together
- the Slint UI branch remains in the tree as reference/prototype, not as the active build

## Module Status

### Application Layer

- `RocketApp.cpp`: implemented and active
- `src/app/*.inl`: implemented and actively used by the main app
- `src/app/RocketAppUiTrajectory.inl`: extracted from common UI helpers to keep files bounded and cleaner
- `SimulationMonitor.cpp`: implemented; external monitor available on Windows
- `RocketSlintApp.cpp`: present but not compiled in current `CMake`
- `ui/rocket_lab.slint`: reference/prototype only

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
- variable mass during burn: implemented
- thrust force and thrust moment from clustered motors: implemented
- motor failure toggles: implemented
- drag with simple Mach response: implemented
- aerodynamic normal force and restoring moment: implemented
- damping term: implemented
- recovery drag and parachute deployment logic: implemented
- per-component structural failure visualization during F2 simulation: implemented
- key mission events and replay: implemented
- rail guidance / launch rod phase: not yet implemented
- layered wind profile and dispersion map: not yet implemented

### Atmosphere & Weather

- launch site and surface weather model: implemented
- atmosphere derived from site and altitude: implemented
- atmosphere `L1/L2` cache for repeated altitude queries: implemented
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

### Not Yet Productized

- project save/load
- export pipeline is available and usable, but still not rich enough for full analysis workflows
- persistent undo/redo
- background online weather sync
- graph-heavy simulation history UI

## Architectural Decisions Locked In

- the primary app path is `raylib`, not Slint
- `main.cpp` must remain a minimal bootstrap
- the physics core stays reusable and separated from the UI shell
- performance optimizations should prefer deterministic software caches before more invasive approximation layers
- mesh generation and simulation remain coupled through `VehicleModel`, not duplicated per frontend
- the external monitor is specialized for aerodynamic diagnostics, not a duplicate full app window

## Immediate Risks / Technical Debt

- the `src/app/*.inl` split is better than the old monolith, but still transitional
- the new `L1/L2` cache layer is exact-keyed, so it avoids drift but does not yet exploit approximate reuse across nearby states
- topology edits are serialized, but a full procedural rebuild can still replace local edits
- some documentation previously described Slint as primary even though the build no longer does
- the CFD naming can suggest higher fidelity than the current heuristic model actually provides

## Next High-Value Moves

- connect live weather providers
- move `src/app/*.inl` into stronger `.hpp/.cpp` modules
- deepen topology editing beyond the current first operator set
- continue refining CFD particle neighborhoods and pressure visualization fidelity
- add historical simulation graphs and richer comparison tools
