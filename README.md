# Rocket Simulator

Desktop C++20 prototype for procedural rocket modeling and ballistic flight simulation.

The project combines:

- a raylib-based 3D desktop workspace focused on modeling and flight analysis
- procedural rocket modeling and simulation controls wired to the existing C++ core
- editable component presets for nose, body, transition, fins, payload, and motor cluster
- per-component editable polygonal meshes with vertex/index buffers and optional wireframe overlay
- direct mesh topology editing with `Vertex / Edge / Face` selection, face `extrude`, face `bevel`, and local edge `loop cut`
- a physics core with variable mass, drag, wind-relative velocity, stability markers, and RK4 integration
- material-aware structural estimates that now feed dry mass, inertia, damping, and recommended dynamic-pressure limits
- a dual workspace UI for `Modelazione` and `Simulazione`
- a dedicated `F3` wind-tunnel window focused on airflow, pressure, and aerodynamic response of individual rocket components
- a Fluent-inspired visual language with clearer card grouping, larger whitespace, and cleaner primary-vs-secondary actions

## Current Status

This repository is an active prototype focused on:

- consolidating the 3D workspace as the only primary UI
- modeling workflow cleanup
- procedural component library expansion
- ballistics and aerodynamic realism
- simulation and UI iteration
- consolidation of the wind-analysis workflow around a single focused `F3` tool

## Features

- 3D desktop shell in raylib with modeling and simulation workspaces
- Prefab component presets with physical impact on mass and aero coefficients
- Extended construction-material library with density, stiffness, yield, service temperature, and structural guidance
- Multi-motor cluster layout with asymmetric failure support
- Basic atmospheric and aerodynamic modeling
- Recovery system and descent logic
- Real-time desktop viewport and simulation panels
- External wind-tunnel analysis window with single-view aerodynamic diagnostics
- Keyboard-driven mission keyframe review with focused in-app analysis window
- In-workspace wind-tunnel component focus for local aerodynamic inspection
- Modeling tool shortcuts with contextual hints for viewport-driven editing
- Mesh inspector controls for topology mode switching and direct polygon editing inside the modeling workspace

## Tech Stack

- C++20
- CMake
- [raylib](https://www.raylib.com/)
- [raylib-cpp](https://github.com/RobLoach/raylib-cpp)

## Build

### Windows / Visual Studio

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

Executable output:

```text
build/Debug/rocket_sim.exe
```

## Project Layout

```text
include/rocket/   Public headers for the simulation core and app entrypoint
src/              Application and physics sources
external/         Vendored rendering dependencies
build/            Local build output (ignored)
```

## Main Documents

- `BALLISTICS_PHYSICS_CORE.md`
- `GUI_UX_SPECIFICATION.md`
- `PROGRESS_DB.md`
- `TODO_BACKLOG.md`
- `THEORETICAL_PHYSICS.md`

## Notes

- The repository currently vendors external dependencies in `external/`.
- The current primary application path is the 3D raylib workspace in `src/RocketApp.cpp`.
- Legacy Slint files may remain in the tree for reference, but they are no longer part of the build.
- Build output and generated IDE files are excluded via `.gitignore`.
- The UI and simulation are still under active development and not yet production-stable.

## Roadmap

- Cleaner UI architecture
- Richer component library
- Stronger aerodynamic and ballistic validation
- Better telemetry and simulation tooling
- Stronger coupling between material selection and flight dynamics
- Live weather integration

## Contributing

See `CONTRIBUTING.md`.
