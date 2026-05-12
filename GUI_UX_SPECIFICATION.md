# GUI & UX Specification - Current Direction

Ultimo aggiornamento: `2026-05-12`

## Purpose

This document now describes the active UX direction of the current `raylib` desktop application, not the old or alternate frontend branches.

## Active UI Architecture

The current primary interface is:

- `src/RocketApp.cpp`
- `src/app/RocketAppState.inl`
- `src/app/RocketAppInteraction.inl`
- `src/app/RocketAppUiCommon.inl`
- `src/app/RocketAppUiModeling.inl`
- `src/app/RocketAppUiSimulation.inl`

The Slint files still present in the repository are reference/prototype assets and are not the active UI path.

## Product Shape

The app is organized into two main workspaces:

- `Modelazione`
- `Simulazione`

Design intent:

- modeling should feel like a lightweight CAD/editor
- simulation should feel like a compact control room
- the 3D viewport remains central in both modes

## Implemented UX Today

### Modeling Workspace

Implemented:

- top workspace switch
- viewport-first layout
- floating desktop-like panels
- direct viewport component selection
- contextual handles
- local grid and snap
- wireframe toggle
- viewport reference board
- active tool hints
- compact overlay markers for vertices, edges, faces

Current tool set:

- `Select`
- `Move`
- `Rotate`
- `Scale`
- `Add Part`
- `Measure`

Current mental model:

- select component
- adjust procedural dimensions
- optionally refine local mesh
- watch mass/stability update immediately

### Simulation Workspace

Implemented:

- mission control actions
- telemetry panels
- scenario controls
- trajectory rendering
- event markers
- replay mode
- keyframe review with `K`
- simulation camera modes
- per-component stress and failure visualization in the viewport
- integrated wind-tunnel analysis panel

### External Monitor

Implemented:

- `F3` opens a specialized external aerodynamic monitor
- it is not a duplicate of the whole app
- it focuses on flow regime, pressures, component loading, and structural response cues

## UX Rules We Should Preserve

### Modeling

- viewport remains the primary surface
- panels support the scene and should not compete with it
- interaction must stay fast and visually readable
- topology markers must stay compact and precise
- metrics should be near the active work, not buried in separate menus

### Simulation

- the launch/replay loop must remain simple
- state, telemetry, and scenario should remain legible at a glance
- wind/aero diagnostics should stay close to mission context
- key events should be easy to review without leaving the workspace

## Features Implemented Vs Planned

### Implemented

- workspace split
- modeling shortcuts
- CAD-like orbit mode
- floating panels
- component presets
- material selection
- vertex/edge/face mesh mode
- first topology tools
- mission keyframes
- external aerodynamic monitor

### Planned / Incomplete

- resizable/dockable layout system
- richer timeline graphs
- true transform gizmos for topology editing
- project save/load UX
- direct mesh heatmap rendering on the 3D model
- comparison workflows across multiple configurations

## Copy & Terminology Guidance

Use these names consistently in the docs and UI:

- `Modelazione`
- `Simulazione`
- `Wind Tunnel`
- `Mission Control`
- `Telemetry`
- `Scenario`
- `Keyframe`
- `Nose Cone`
- `Body Tube`
- `Transition`
- `Fin Set`
- `Motor Mount`
- `Payload`

Avoid describing the current app as:

- Slint-first
- web-like
- multi-window simulation suite

That no longer matches the active build.

## UX Debt Still Visible

- `src/app/*.inl` still reflects a transitional refactor
- some UI controls exist as ad-hoc overlays rather than reusable widgets
- the topology editing flow is functional but not yet polished enough to feel fully tool-grade

## Near-Term UX Goals

1. strengthen the modeling inspector and topology workflow
2. add richer simulation history views without overloading the layout
3. keep documentation aligned with the actual `raylib` build path
