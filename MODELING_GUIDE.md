# Modeling Guide - The Rocket Lab

Ultimo aggiornamento: `2026-05-12`

## Overview

The `Modelazione` workspace is the active 3D design environment of the current application.

It combines:

- procedural rocket components
- live mass and stability feedback
- direct viewport interaction
- first-generation polygon editing on per-component meshes

## Supported Components

- `Nose Cone`
- `Body Tube`
- `Transition`
- `Fin Set`
- `Motor Mount`
- `Payload`

## Supported Shape Families

### Nose

- `Conical`
- `Tangent Ogive`
- `Parabolic`
- `LD-Haack`

### Fins

- `Trapezoidal`
- `Elliptical`
- `Airfoil`

### Transition

- `Conical`
- `Curved`

## Main Workflow

### 1. Enter Modeling

- press `F1`
- or click the `Modelazione` workspace button

### 2. Select A Component

You can select a component by:

- clicking it directly in the viewport
- using the modeling panels

Selection drives:

- visible handles
- local grid position
- inspector content
- available mesh operations

### 3. Adjust Parametric Geometry

The current modeling flow supports direct adjustment of:

- body length and diameter
- nose length and profile family
- transition length and aft diameter
- fin position, span, tip offsets, and count
- payload length and payload mass
- motor count, thrust, burn time, propellant mass, cluster radius, cant angle

Every rebuild updates:

- dry mass estimate
- reference area
- aerodynamic coefficients
- inertia estimate
- CG / CP / static margin

### 4. Choose Materials

Each main component can use one of the structural materials from `MATERIALS_DATABASE.md`.

Materials currently affect:

- density and dry mass
- equivalent structural modulus
- recommended dynamic pressure limit
- drag bias
- normal-force effectiveness
- rotational damping and inertia estimation

### 5. Use Viewport Handles

The modeling workspace exposes component-specific handles for fast reshaping.

Examples:

- nose length and shoulder/mid radius
- body length and radius zones
- transition length and aft diameter
- fin front, span, tip lead, tip trail
- payload length
- motor cluster radius

## Camera & Navigation

### Perspective CAD Camera

- press `TAB` to enable the intentional orbit camera
- mouse wheel: zoom
- `MMB`: orbit
- `Shift + MMB`: pan

### Orthographic-Style Review Views

The workspace also supports fixed review views:

- `Perspective`
- `Front`
- `Side`
- `Top`

## Mesh Editing

The current mesh editing system is real and active, but still early-stage.

### What Exists Today

- persistent mesh per component
- cached vertices, faces, and edges
- direct vertex repositioning
- selection modes:
  - `Vertex`
  - `Edge`
  - `Face`
- wireframe overlay
- topology operations:
  - `Extrude Face`
  - `Bevel Face`
  - `Loop Cut Edge`

### Vertex Editing

Use vertex mode when you want local shape changes without changing the whole procedural profile.

Current behavior:

- select a component
- activate its free-edit mode from the UI
- click a vertex marker
- drag to move the vertex
- snap applies when enabled

### Edge Editing

In `Edge` mode you can select an edge marker and apply:

- `Loop Cut Edge`

Current implementation:

- splits the selected edge locally
- updates adjacent faces where possible

### Face Editing

In `Face` mode you can select a face marker and apply:

- `Extrude Face`
- `Bevel Face`

These are currently triangle-oriented first-pass tools, useful for quick experiments and local refinement.

## Overlays

### Available In Modeling

- local component grid
- snap-to-grid
- metrics overlay
- wireframe
- reference blueprint board
- compact handle markers

### Metrics You Can Read Live

- dry mass
- CG
- CP
- static margin
- drag coefficient estimate

## Presets

Current full-vehicle presets:

- `Research Starter`
- `Sport Trainer`
- `High Altitude`
- `Minimum Diameter`
- `Heavy Lift`

They are useful as starting points before local refinement.

## Practical Limits Of The Current System

- topology edits are serialized in `.rlab` projects
- a full procedural rebuild can still replace locally edited topology
- topology tools are intentionally minimal and not yet Blender-level
- there is no true undo stack yet

## Recommended Workflow

1. Start from a preset.
2. Adjust major procedural dimensions first.
3. Choose materials.
4. Validate CG/CP and mass direction.
5. Use local mesh editing only for final shaping or experiments.
6. Switch to `Simulazione` to evaluate the effect on flight.

## Related Documents

- `README.md`
- `PROGRESS_DB.md`
- `GUI_UX_SPECIFICATION.md`
- `MATERIALS_DATABASE.md`
