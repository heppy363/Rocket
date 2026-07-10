# TODO Backlog

Ultimo aggiornamento: `2026-05-26`

## Highest Priority

- [x] Collegare davvero i provider meteo al runtime con fetch HTTP reale e parsing risposta.
- [x] Rendere persistenti e serializzabili preset, geometria e topology edits.
- [~] Portare i moduli `src/app/*.inl` verso coppie `.hpp/.cpp` piu pulite.
  Primo passo completato: `RocketAppUiTrajectory.inl` migrato a `src/app/RocketAppTrajectory.hpp/.cpp`.
  Secondo passo completato: helper aero/visuali del `Wind Tunnel` migrati a `src/app/RocketAppAeroHelpers.hpp/.cpp`.
  Terzo passo completato: helper Dear ImGui generici migrati a `src/app/RocketAppImGuiHelpers.hpp/.cpp`.
  Quarto passo completato: logica attiva del pannello `History` migrata a `src/app/RocketAppHistory.hpp/.cpp`.
- [ ] Migliorare il workflow mesh con gizmo piu chiari, selezione piu precisa e operazioni topologiche meno fragili.

## Modeling

- [~] Consolidare l'editing diretto `Vertex / Edge / Face`.
- [ ] Estendere `Loop Cut` da split locale a veri edge loop continui.
- [ ] Estendere `Bevel` oltre la faccia triangolare di base.
- [ ] Introdurre soft selection e falloff su deformazioni locali.
- [ ] Aggiungere undo/redo del modeling.
- [ ] Aggiungere docking o layout ridimensionabile reale.

## Simulation

- [x] Rifinire replay e review keyframe.
- [x] Estendere il sistema di cache software `L1/L2` a piu sottosistemi hot-path con metriche di cache hit.
- [x] Aggiungere scrub manuale e confronto fra keyframe.
- [x] Introdurre grafici storici per quota, Mach, `q`, `rho`, `P`, `P0`, `Re`.
- [~] Migliorare il profilo vento con shear multilayer.
  Primo pass completato: il runtime usa ora bande di quota con shear e directional veer, mantenendo compatibilita con i dati meteo esistenti.
- [~] Aggiungere fase di launch rail / rail guidance.
  Primo pass completato: rail verticale con lunghezza configurabile, vincolo iniziale di guida e rilascio automatico a fine rail.
- [ ] Aggiungere mappe di dispersione e landing analysis.

## Aero / CFD

- [~] Continuare a rendere piu leggibile e coerente il pannello `Wind Tunnel`.
  Ultimo passo: monitor esterno `F3` riallineato al tema Dark Space con clipping testo e pannelli coerenti, senza modifiche a simulazione o dati.
  Refinement successivo: padding aumentato e copertura dark dello sfondo rafforzata per migliorare leggibilita di chip, telemetria e card.
- [x] Portare la heatmap di pressione direttamente sulla mesh 3D del razzo.
- [~] Raffinare il solver particellare con neighborhood search piu robusta.
- [ ] Evolvere shockwave e aeroelastic metrics da euristiche a modelli piu fisici.
- [x] Chiarire in UI la differenza tra diagnostica CFD-style e solver CFD reale.

## Data & Project Workflow

- [x] Definire un formato progetto `.rlab`.
- [x] Salvare configurazione veicolo, materiali, scenario e topology edits.
- [x] Preparare export di report e dati simulazione.

## Nice To Have

- [ ] Libreria motori reale con preset piu ricchi.
- [ ] Import/export mesh o profili.
- [ ] Confronto side-by-side di configurazioni.
- [ ] Migliore onboarding in-app per utenti nuovi.
