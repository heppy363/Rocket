# Materials & Components Database

Ultimo aggiornamento: `2026-05-12`

This table reflects the material definitions currently hard-coded in `src/DesignLibrary.cpp`.

## Structural Materials

| Materiale | Densita kg/m3 | E GPa | Yield MPa | Tmax C | Costo Relativo | Uso Tipico |
| :--- | ---: | ---: | ---: | ---: | ---: | :--- |
| `PLA-CF` | 1240 | 11.0 | 68 | 55 | 1.2 | Ogive prototipali, pinne stampate |
| `Alluminio 6061` | 2700 | 68.0 | 276 | 150 | 2.4 | Struttura principale, tail section |
| `PVC` | 1400 | 3.0 | 52 | 60 | 0.9 | Supporti interni, payload economico |
| `Fiberglass` | 1850 | 24.0 | 210 | 120 | 1.8 | Fusoliere composite general purpose |
| `Carbon Fiber` | 1600 | 70.0 | 600 | 135 | 3.6 | Body tube e pinne high-performance |
| `Betulla Aircraft` | 680 | 10.5 | 95 | 90 | 1.0 | Pinne leggere e componenti semplici |
| `Phenolic Tube` | 1420 | 16.0 | 140 | 180 | 1.7 | Tubi e avionics bay resistenti al calore |

## How Materials Affect The Current Simulation

Materials are not cosmetic only. They currently influence:

- structural mass estimate
- dry mass estimate
- equivalent density and equivalent modulus
- recommended maximum dynamic pressure
- base drag bias
- normal-force slope estimate
- rotational damping estimate
- principal inertia estimate

## Structural Outputs Already Used By The App

The app computes:

- per-component structural mass breakdown
- equivalent material assessment
- component dynamic pressure limit
- global recommended dynamic pressure limit
- dynamic pressure safety factor

## Important Modeling Notes

- `payload_mass_kg` represents useful payload mass, not only shell mass
- payload material mostly affects the payload bay shell estimate
- recommended `q` values are engineering heuristics for comparison
- these limits are useful for design iteration, not substitutes for FEM or flutter validation

## Recovery Defaults In Code

- parachute drag coefficient: `1.55`
- default parachute area: `0.85 m2`
- default deployment altitude: `250 m`
- default deployment delay: `1.2 s`
