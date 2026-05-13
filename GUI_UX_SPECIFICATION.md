# GUI & UX Specification - Current Direction

Ultimo aggiornamento: `2026-05-13`

## Purpose

This document now describes the active UX direction of the current `raylib` desktop application, not the old or alternate frontend branches.

## Active UI Architecture

The current primary interface is:

- `src/RocketApp.cpp`
- `src/app/RocketAppImGui.inl`
- `src/app/RocketAppState.inl`
- `src/app/RocketAppInteraction.inl`
- `src/app/RocketAppUiCommon.inl`
- `src/app/RocketAppUiModeling.inl`
- `src/app/RocketAppUiSimulation.inl`

## Product Shape

The app is organized into two main workspaces:

- `Modelazione`
- `Simulazione`

Design intent:

- modeling should feel like a lightweight CAD/editor inside a Dark Space control shell
- simulation should feel like a compact control room with denser, faster-to-scan telemetry
- the 3D viewport remains central in both modes
- Dear ImGui windows should coordinate the workflow, not fight with the viewport

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
- resizable Dear ImGui host windows wrapped around the richer simulation diagnostics panels
- native Dear ImGui telemetry/events windows for cleaner scan order without removing the denser visual trajectory and wind-tunnel views
- wind-tunnel panel visually cleaned to preserve all CFD-style data without label overflow or layout collisions
- F2 now allows the integrated wind-tunnel panel to be toggled on/off directly from the top-right control area, independently from the external `F3` monitor

### External Monitor

Implemented:

- `F3` opens a specialized external aerodynamic monitor
- it is not a duplicate of the whole app
- it focuses on flow regime, pressures, component loading, and structural response cues
- the `F3` shell now follows the same Dark Space visual language as the Dear ImGui control room, with clipped labels, denser metric chips, and no white high-contrast breakpoints
- the `F3` shell also now uses larger padding and stronger dark background coverage so captions and side telemetry stay readable at a glance

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

- Dear ImGui Dark Space shell
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

- web-like
- multi-window simulation suite
- flat generic ImGui tooling

That no longer matches the active build.

## UX Debt Still Visible

- `src/app/*.inl` still reflects a transitional refactor
- some UI controls still exist as older custom overlays while the Dear ImGui migration is being consolidated
- the topology editing flow is functional but not yet polished enough to feel fully tool-grade

## Near-Term UX Goals

1. deepen the Dear ImGui shell until all workflow-critical panels feel cohesive
2. strengthen the modeling inspector and topology workflow
3. add richer simulation history views without overloading the layout
