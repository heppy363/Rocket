# TODO Backlog

Ultimo aggiornamento: `2026-05-13`

## Highest Priority

- [~] Collegare davvero i provider meteo al runtime con fetch HTTP reale e parsing risposta.
- [x] Rendere persistenti e serializzabili preset, geometria e topology edits.
- [~] Portare i moduli `src/app/*.inl` verso coppie `.hpp/.cpp` piu pulite.
- [ ] Migliorare il workflow mesh con gizmo piu chiari, selezione piu precisa e operazioni topologiche meno fragili.

## Modeling

- [~] Consolidare l'editing diretto `Vertex / Edge / Face`.
- [ ] Estendere `Loop Cut` da split locale a veri edge loop continui.
- [ ] Estendere `Bevel` oltre la faccia triangolare di base.
- [ ] Introdurre soft selection e falloff su deformazioni locali.
- [ ] Aggiungere undo/redo del modeling.
- [ ] Aggiungere docking o layout ridimensionabile reale.

## Simulation

- [~] Rifinire replay e review keyframe.
- [x] Estendere il sistema di cache software `L1/L2` a piu sottosistemi hot-path con metriche di cache hit.
- [ ] Aggiungere scrub manuale e confronto fra keyframe.
- [ ] Introdurre grafici storici per quota, Mach, `q`, `rho`, `P`, `P0`, `Re`.
- [ ] Migliorare il profilo vento con shear multilayer.
- [ ] Aggiungere fase di launch rail / rail guidance.
- [ ] Aggiungere mappe di dispersione e landing analysis.

## Aero / CFD

- [~] Continuare a rendere piu leggibile e coerente il pannello `Wind Tunnel`.
  Ultimo passo: monitor esterno `F3` riallineato al tema Dark Space con clipping testo e pannelli coerenti, senza modifiche a simulazione o dati.
  Refinement successivo: padding aumentato e copertura dark dello sfondo rafforzata per migliorare leggibilita di chip, telemetria e card.
- [x] Portare la heatmap di pressione direttamente sulla mesh 3D del razzo.
- [~] Raffinare il solver particellare con neighborhood search piu robusta.
- [ ] Evolvere shockwave e aeroelastic metrics da euristiche a modelli piu fisici.
- [ ] Chiarire in UI la differenza tra diagnostica CFD-style e solver CFD reale.

## Data & Project Workflow

- [x] Definire un formato progetto `.rlab`.
- [x] Salvare configurazione veicolo, materiali, scenario e topology edits.
- [x] Preparare export di report e dati simulazione.

## Nice To Have

- [ ] Libreria motori reale con preset piu ricchi.
- [ ] Import/export mesh o profili.
- [ ] Confronto side-by-side di configurazioni.
- [ ] Migliore onboarding in-app per utenti nuovi.
