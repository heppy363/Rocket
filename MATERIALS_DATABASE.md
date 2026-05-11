# Materials & Components DB

## Materiali Strutturali

| Materiale | Densita (kg/m3) | E (GPa) | Yield (MPa) | Tmax (C) | Uso tipico |
| :--- | ---: | ---: | ---: | ---: | :--- |
| **PLA-CF** | 1240 | 11.0 | 68 | 55 | Ogive prototipali, pinne stampate |
| **Alluminio 6061** | 2700 | 68.0 | 276 | 150 | Struttura principale, tail section |
| **PVC** | 1400 | 3.0 | 52 | 60 | Supporti interni e payload economico |
| **Fiberglass** | 1850 | 24.0 | 210 | 120 | Fusoliere composite general purpose |
| **Carbon Fiber** | 1600 | 70.0 | 600 | 135 | Body tube e pinne high-performance |
| **Betulla Aircraft** | 680 | 10.5 | 95 | 90 | Pinne leggere e componenti semplici |
| **Phenolic Tube** | 1420 | 16.0 | 140 | 180 | Tubi e avionics bay resistenti al calore |

## Note di Modellazione

- Il simulatore usa ora queste proprieta per stimare massa strutturale, modulo equivalente e `q` dinamica consigliata.
- Le proprieta materiali influenzano anche `Cd` base, `CNalpha`, damping rotazionale e distribuzione delle inerzie secche.
- La `payload_mass_kg` resta il contenuto utile; il materiale payload influisce soprattutto sul bay/canister strutturale.
- La `q` consigliata e una stima ingegneristica preliminare, utile per confronto tra layout, non sostituisce analisi FEM o flutter dedicate.

## Effetti Dinamici Attuali

- Materiali piu lisci e rigidi come `Carbon Fiber` e `Fiberglass` riducono il drag base e mantengono meglio l'efficacia delle pinne.
- Materiali piu cedevoli o grezzi come `PVC`, `PLA-CF` e `Betulla Aircraft` aumentano leggermente il drag e possono ridurre l'efficacia aero laterale.
- Il simulatore calcola ora una distribuzione d'inerzia secca per componenti (`nose`, `payload bay`, `body`, `transition`, `fins`) invece di usare solo una stima cilindrica globale.

## Componenti Predefiniti
- **Paracadute:** Coefficiente di drag $C_d \approx 1.5$ - $1.75$.
- **Shock Cord:** Modellata come molla elastica (Hooke's Law).
