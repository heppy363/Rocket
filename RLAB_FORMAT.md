# RLAB Project Format

Ultimo aggiornamento: `2026-05-12`

## Overview

`.rlab` is the project format used by the current desktop app to persist:

- vehicle geometry
- materials
- recovery settings
- launch site and surface weather
- weather source selection
- motor editor settings
- current motor cluster layout and failure state
- per-component free vertex modifiers
- per-component topology overrides for edited meshes

## Encoding

- plain text
- line-oriented
- section-based
- human-readable

The file starts with:

```text
rocket_lab_project
format_version=1
```

## Section Model

The current writer uses sections like:

- `[project]`
- `[geometry]`
- `[recovery]`
- `[launch_site]`
- `[surface_weather]`
- `[weather]`
- `[motor_editor]`
- `[cluster]`
- `[modifier.nose]`
- `[modifier.body]`
- `[modifier.transition]`
- `[modifier.fins]`
- `[modifier.motor_mount]`
- `[modifier.payload]`
- `[topology.nose]`
- `[topology.body]`
- `[topology.transition]`
- `[topology.fins]`
- `[topology.motor_mount]`
- `[topology.payload]`

## Persistence Strategy

### Parametric State

Procedural geometry is saved explicitly as scalar fields.

### Direct Mesh Edits

Two layers are saved:

- free vertex modifier data
- topology override data

The topology override is what allows edited meshes to survive project save/load even after operations such as:

- vertex dragging
- face extrusion
- face bevel
- edge loop cut

## Current Limits

- format versioning exists but migration logic is still minimal
- reports and trajectory exports are separate files, not embedded in `.rlab`
- the format is still line-oriented and custom, not schema-driven
- there is no embedded binary payload for large mesh assets or replay history

## Current Default Paths In App

- initial project path: `projects/rocket_lab_current.rlab`
- initial report path: `exports/rocket_lab_report.txt`
- initial trajectory path: `exports/rocket_lab_trajectory.csv`

These are default suggestions only. The active app now exposes native file dialogs for save, save-as, load, report export, and CSV export.
