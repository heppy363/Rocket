# Sttuttra Del Progetto

Questo file serve come promemoria operativo della struttura del progetto: cosa fa ogni file principale e perche ci serve nel tempo.

## Root

- `README.md`: panoramica veloce del progetto, feature correnti, build e layout repository.
- `PROGRESS_DB.md`: memoria dello stato reale di sviluppo e delle decisioni gia prese.
- `TODO_BACKLOG.md`: coda dei lavori tecnici e UX ancora aperti.
- `GUI_UX_SPECIFICATION.md`: direzione di prodotto per workspace, pannelli, scorciatoie e comportamento UX.
- `BALLISTICS_PHYSICS_CORE.md`: riferimento del modello fisico e balistico.
- `THEORETICAL_PHYSICS.md`: appunti teorici di supporto per la parte simulativa.
- `MATERIALS_DATABASE.md`: base dati compatta dei materiali usati dal modello.
- `CONTRIBUTING.md`: linee guida di collaborazione.
- `sttuttra.md`: mappa interna del codice per ricordare ruoli e motivazioni architetturali.

## Public Headers

- `include/rocket/RocketApp.hpp`: entrypoint pubblico dell'app desktop. Serve per lasciare `main.cpp` minimale.
- `include/rocket/Aerodynamics.hpp`: interfaccia del modello aerodinamico.
- `include/rocket/DesignLibrary.hpp`: preset geometrici e librerie di configurazione veicolo.
- `include/rocket/Environment.hpp`: atmosfera, sito di lancio, meteo e sorgenti dati weather.
- `include/rocket/Forces.hpp`: calcolo delle forze e grandezze derivate.
- `include/rocket/FlightState.hpp`: stato dinamico del razzo.
- `include/rocket/MathTypes.hpp`: vettori, quaternioni e tipi matematici di base.
- `include/rocket/MeshGenerator.hpp`: generazione e gestione delle mesh procedurali.
- `include/rocket/PhysicalConstants.hpp`: costanti globali della simulazione.
- `include/rocket/RungeKutta4.hpp`: integratore numerico.
- `include/rocket/SecureValidation.hpp`: clamp e validazioni centralizzate per input e geometria.
- `include/rocket/SimulationMonitor.hpp`: supporto alla finestra esterna e al monitor realtime.
- `include/rocket/VehicleModel.hpp`: definizione completa del veicolo e della sua geometria.

## Application Sources

- `src/main.cpp`: bootstrap minimo. Deve restare il piu pulito possibile e limitarsi a chiamare l'app.
- `src/RocketSlintApp.cpp`: nuova orchestrazione principale dell'applicazione desktop con frontend Slint, callback UI e collegamento al core esistente senza modificare le funzioni del simulatore.
- `src/RocketApp.cpp`: orchestratore storico basato su Raylib. Rimane come riferimento tecnico del vecchio frontend, ma non e piu il punto di ingresso principale della UI.

## UI Sources

- `ui/rocket_lab.slint`: definizione del nuovo frontend Slint. Qui vivono struttura visuale, card, pannelli, proprieta bindate e callback dell'interfaccia.

## Legacy Raylib UI Modules

- `src/app/RocketAppState.inl`: tipi applicativi locali, runtime della missione, layout finestre, helper di stato e utilita per keyframe/ricostruzione veicolo.
Serve per concentrare lo stato interno del vecchio frontend in un unico punto invece di spargerlo nel loop principale.

- `src/app/RocketAppInteraction.inl`: logica di interazione utente non puramente grafica.
Qui stanno camera, replay, handle 3D, picking, snap su griglia e manipolazione diretta dei componenti.

- `src/app/RocketAppUiCommon.inl`: primitive UI condivise del vecchio frontend Raylib.
Contiene pannelli, button, card metriche, hint, interpolazione replay e utility comuni al rendering dei workspace.

- `src/app/RocketAppUiModeling.inl`: UI legacy del workspace `Modelazione`.
Gestisce toolbar, outliner, libreria preset, inspector, reference board e riepilogo di progetto.
Nel layout attuale raggruppa anche i controlli `Vista & Overlay` per evitare toggle dispersi.

- `src/app/RocketAppUiSimulation.inl`: UI legacy del workspace `Simulazione`.
Gestisce telemetria, camera del vento, scenario, timeline, panoramica traiettoria, stato missione, review keyframe con `K` e focus per-componente nella `Wind Tunnel`.
Qui si concentra la control room: azioni missione, parametri ambientali e diagnostica sono separati per gruppi logici.

Nota:
- Questi moduli `.inl` esistono perche arriviamo da un refactor di emergenza del vecchio `main.cpp` monolitico.
- In futuro il passo successivo consigliato e spostarli gradualmente in coppie `.hpp/.cpp` piu tipizzate.

## Physics And Core Sources

- `src/Aerodynamics.cpp`: implementazione dei coefficienti e del comportamento aerodinamico.
- `src/DesignLibrary.cpp`: preset di razzo e geometrie di partenza.
- `src/Environment.cpp`: logica del contesto atmosferico e dei parametri ambientali.
- `src/Forces.cpp`: forze applicate al razzo e grandezze di volo derivate.
- `src/RungeKutta4.cpp`: integrazione numerica dello stato.
- `src/MeshGenerator.cpp`: generazione mesh procedurale dei componenti e disegno del veicolo.
- `src/SimulationMonitor.cpp`: finestra esterna e monitor dedicato alla simulazione/vento.

## External

- `external/raylib/`: dipendenza grafica C ancora presente per supporto grafico/rendering e parti storiche del progetto.
- `external/raylib-cpp/`: wrapper C++ usato per semplificare integrazione con Raylib.

## Build Output

- `build-slint/`: build directory di riferimento per la nuova UI Slint.

## Perche Questa Struttura Ci Serve

- Evita di ricadere in un `main.cpp` ingestibile.
- Rende piu semplice capire dove mettere una modifica nuova.
- Permette di evolvere design Slint, orchestration C++ e core fisico come layer separati.
- Riduce il rischio di rompere logica fisica quando stiamo lavorando solo sulla UI.
- Ci lascia una base chiara per il refactor successivo verso moduli ancora piu puliti.
