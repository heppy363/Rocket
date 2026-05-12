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
- serialization of topology edits: not yet implemented

### Mesh System

- persistent mesh storage per component: implemented
- face and edge cache generation: implemented
- GPU mesh refresh after edits: implemented
- support for `Nose`, `Body`, `Transition`, `Fin`, `Payload`, `Motor Mount`: implemented
- smooth influence/falloff deformation: not yet implemented
- full loop-cut over continuous edge loops: not yet implemented

### Flight Simulation

- RK4 integrator: implemented
- variable mass during burn: implemented
- thrust force and thrust moment from clustered motors: implemented
- motor failure toggles: implemented
- drag with simple Mach response: implemented
- aerodynamic normal force and restoring moment: implemented
- damping term: implemented
- recovery drag and parachute deployment logic: implemented
- key mission events and replay: implemented
- rail guidance / launch rod phase: not yet implemented
- layered wind profile and dispersion map: not yet implemented

### Atmosphere & Weather

- launch site and surface weather model: implemented
- atmosphere derived from site and altitude: implemented
- wind-relative velocity: implemented
- provider enum and query URL helper: implemented
- live weather fetch: not yet implemented

### Aero / CFD Diagnostics

- in-workspace wind tunnel panel: implemented
- external `F3` wind-tunnel monitor: implemented on Windows
- component-oriented aero cards: implemented
- synthetic shockwave and aeroelastic metrics: implemented
- real-time particle field for CFD-style diagnostics: implemented
- CFD force/moment augmentation into flight model: implemented
- pressure heatmap directly on 3D mesh: not yet implemented
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
- weather sources are modeled in code, but not yet connected to network providers

### Not Yet Productized

- project save/load
- export pipeline
- persistent undo/redo
- live online weather
- graph-heavy simulation history UI

## Architectural Decisions Locked In

- the primary app path is `raylib`, not Slint
- `main.cpp` must remain a minimal bootstrap
- the physics core stays reusable and separated from the UI shell
- mesh generation and simulation remain coupled through `VehicleModel`, not duplicated per frontend
- the external monitor is specialized for aerodynamic diagnostics, not a duplicate full app window

## Immediate Risks / Technical Debt

- the `src/app/*.inl` split is better than the old monolith, but still transitional
- topology edits reset with full procedural rebuilds and are not serialized
- some documentation previously described Slint as primary even though the build no longer does
- the CFD naming can suggest higher fidelity than the current heuristic model actually provides

## Next High-Value Moves

- connect live weather providers
- persist project state and topology edits
- move `src/app/*.inl` into stronger `.hpp/.cpp` modules
- deepen topology editing beyond the current first operator set
- add historical simulation graphs and richer comparison tools
