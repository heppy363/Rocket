# Ballistics & Trajectory Physics Core

Ultimo aggiornamento: `2026-05-12`

This document now distinguishes between what is already implemented and what is still planned.

## Implemented Physics Model

### State

The simulation state currently includes:

- position
- velocity
- attitude quaternion
- angular velocity
- mass

Time integration is performed with RK4.

### Forces Already In The Runtime

The solver currently sums:

- thrust
- aerodynamic drag
- aerodynamic normal force
- recovery drag
- gravity
- CFD-inspired augmentation force

### Moments Already In The Runtime

The solver currently uses:

- thrust moment from clustered motors
- aerodynamic restoring moment from `CP - CG`
- rotational damping moment
- CFD-inspired augmentation moment

### Propulsion

Clustered motors already support:

- multiple mounted motors
- per-motor mount position
- per-motor thrust direction
- failed motor toggles
- thrust moment asymmetry
- propellant mass flow

### Atmosphere

The `Environment` model already provides:

- site elevation
- surface pressure, temperature, humidity
- wind speed, gust, direction
- altitude-based pressure
- altitude-based temperature
- altitude-based density
- speed of sound
- gravity with altitude dependence
- wind-relative velocity field used by the flight model

### Aerodynamics

The current aerodynamic layer already computes:

- relative airspeed
- Mach number
- dynamic pressure
- angle of attack
- aerodynamic drag with a simple Mach-dependent drag rise
- aerodynamic normal force using `CNalpha`
- static margin from CG and CP

### Recovery

The current recovery model already supports:

- deployment delay
- deployment altitude
- descent drag from parachute area and parachute `Cd`

## Diagnostics Already Exposed To The App

The application already exposes:

- time
- altitude
- airspeed
- Mach
- AoA
- dynamic pressure
- CG / CP
- static margin
- max altitude
- wind speed
- pressure and density derived from the environment
- shockwave and aeroelastic synthetic indicators

## Current Model Limits

The simulation is useful and coherent, but still approximate.

Not yet implemented:

- launch rail guidance
- staged propulsion
- true Barrowman-grade full-detail CP breakdown per component
- controller/autopilot logic
- dispersion maps
- high-fidelity transonic/supersonic aero database

Implemented in first pass:

- layered wind profile by altitude bands with directional veer and shear derived from the current surface weather state
- launch rail guidance with constrained early-flight translation/rotation until rail release

## CFD Layer: What It Is Today

The `CfdModule` is currently a real-time diagnostic augmentation layer, not a full CFD solver.

It currently provides:

- persistent particle field
- component pressure buckets
- heuristic force and moment augmentation
- render particle samples
- synthetic shockwave and aeroelastic response cues

It does not yet provide:

- converged volumetric CFD
- mesh pressure field export
- Navier-Stokes-grade resolution

## Near-Term Physics Priorities

1. connect live weather input to the existing environment model
2. improve wind profiling and launch-phase realism
3. deepen aerodynamic validation around transonic behavior
4. preserve coherence between flight model and diagnostic CFD layer
