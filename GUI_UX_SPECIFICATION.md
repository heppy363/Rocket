# GUI & UX Specification: "The Rocket Lab" Interface

## 1. Visione Generale
L'app deve essere divisa in due sezioni principali, sempre accessibili dalla barra superiore:

- `Modelazione`
- `Simulazione`

L'utente non deve percepire la GUI come un semplice viewer tecnico, ma come un vero ambiente di lavoro desktop per progettare e poi testare il razzo.

Linee guida di questa iterazione UI/UX:
- copy coerente e leggibile, evitando mix casuale tra etichette tecniche e messaggi generici
- gerarchia visiva piu chiara con header, card metriche e pannelli dal ruolo esplicito
- enfasi sul contesto attivo: componente selezionato, tool corrente, stato dello snap e stato della missione
- nessuna modifica alla logica applicativa in questa fase: il focus resta solo sulla presentazione e sulla comprensione del flusso
- shell 3D raylib con pannelli desktop integrati: superfici morbide, contenimento visivo, controlli primari pieni e secondari contornati
- piu whitespace tra pannelli e gruppi di controlli per evitare saturazione percettiva
- configurazioni e toggle raggruppati in card dedicate invece che sparsi in piu punti della UI

Nota architetturale:
- l'implementazione GUI non deve piu convergere in un unico `main.cpp`; anche i layer UI e orchestration devono restare modulari per mantenere la roadmap sostenibile
- la shell primaria vive ora nel workspace 3D raylib (`RocketApp.cpp` + moduli `src/app/`), mentre il C++ continua a gestire stato, simulazione e aggiornamento dei dati

---

## 2. Workspace Principali

### A. Workspace `Modelazione`
Questa sezione deve ispirarsi chiaramente a Blender per organizzazione mentale, densita dei controlli e flusso di lavoro.

Obiettivi UX:
- dare priorita alla viewport 3D
- permettere selezione, editing e ispezione rapida dei componenti
- mantenere strumenti e proprieta sempre vicini al contesto selezionato

Layout desiderato:
- **Top Bar:** selezione workspace, file, import/export, modalita vista, snapping, overlay.
- **Left Toolbar:** strumenti rapidi in stile Blender (`Select`, `Move`, `Rotate`, `Scale`, `Add Part`, `Measure`).
- **Center 3D Viewport:** scena principale del razzo con griglia, gizmo, assi, CG, CP e punti di snap.
- **Right Sidebar / Properties:** editor proprieta del componente selezionato.
- **Bottom Strip opzionale:** elenco rapido componenti, log operazioni, future timeline di editing.
- **Outliner / Project Tree:** gerarchia del razzo e dei sottocomponenti.

Indicazioni di rifinitura UI applicate:
- header viewport con riepilogo sintetico del contesto corrente
- toolbar e outliner con nomenclatura piu chiara e orientata all'azione
- inspector focalizzato sul componente attivo, senza ambiguita testuali
- libreria preset separata in `preset progetto` e `preset componente`
- hint contestuali direttamente nell'inspector per suggerire come intervenire sul componente attivo
- pannello `Vista & Overlay` unificato per view switching, blueprint e toggle rapidi della viewport
- status bar di progetto con piu respiro e allineamento piu leggibile delle metriche di sintesi

Comportamento stile Blender:
- selezione oggetti direttamente nella viewport
- gizmo di trasformazione visibile sul componente attivo
- focus sull'oggetto selezionato
- modalita wireframe, solid e diagnostic overlay
- supporto a mesh poligonali editabili con vertex selection diretta, rete triangolare visibile e manipolazione puntuale dei vertici
- supporto a modalita topologiche `Vertex`, `Edge`, `Face` con selezione contestuale in viewport
- pannelli collassabili e docking flessibile
- scorciatoie tastiera coerenti per navigazione e strumenti
- shortcut tool immediate nel workspace (`1-6`) con hint contestuali del tool attivo
- navigazione CAD in prospettiva attivabile con `TAB`, con orbita intenzionale, pan separato e blocco dei movimenti accidentali sopra i pannelli UI
- edit cage del componente selezionato direttamente sulla mesh visibile, con selezione del pezzo tramite click in viewport
- modalita `Wireframe` reale sopra la mesh del componente, non solo hint visivo, per leggere edge flow e topologia

Componenti modificabili nel workspace:
- `Nose Cone`
- `Body Tube`
- `Transition`
- `Fin Set`
- `Motor Mount`
- `Avionics Bay`
- `Payload Section`

Editor proprieta richiesto:
- parametri geometrici numerici
- materiale
- massa stimata
- vincoli di connessione
- posizione lungo asse del razzo
- lock read-only del componente
- statistiche mesh del componente (`vertex count`, `triangle count`, stato editabile)

Manipolazione diretta della geometria:
- ogni componente deve esporre una mesh persistente basata su `Vertex Buffer` + `Index Buffer`
- la GUI deve poter leggere la rete triangolare del pezzo selezionato
- l'utente deve poter selezionare e trascinare manualmente i vertici della mesh
- l'utente deve poter selezionare `edge` e `face` senza uscire dalla viewport
- i vertici modificati devono restare associati al componente e sopravvivere ai redraw
- il wireframe deve poter essere acceso o spento dalla viewport per controllare topologia e deformazioni locali
- la selezione di un vertice deve mostrare feedback chiaro nell'inspector e nella viewport
- la selezione di `edge` e `face` deve mostrare highlight dedicato nel wireframe e statistiche nel pannello proprieta
- l'inspector deve esporre operatori topologici minimi tipo `Extrude Face`, `Bevel Face` e `Loop Cut Edge`
- il passaggio tra modalita mesh deve essere rapido e persistente nel contesto del componente selezionato
- i marker topologici non devono essere sfere 3D ingombranti: `vertex`, `edge` e `face` devono usare indicatori piccoli e leggibili in overlay schermo
- testo e microcopy di hint, info card, header e status bar devono restare sempre contenuti nei pannelli tramite wrapping o clipping controllato

Forme procedurali supportate o pianificate:
- **Nose Cones:** `Conical`, `Tangent Ogive`, `Parabolic`, `LD-Haack`
- **Fins:** `Trapezoidal`, `Elliptical`, `Airfoil`
- **Transitions:** profili conici e successivamente profili piu raffinati

### B. Workspace `Simulazione`
Questa sezione deve essere piu vicina a una control room tecnica che a un CAD editor.

Obiettivi UX:
- lanciare scenari di simulazione in modo chiaro
- visualizzare traiettoria, assetto e metriche in tempo reale
- confrontare configurazioni geometriche e condizioni ambientali

Layout desiderato:
- **Viewport 3D Centrale:** volo del razzo, traiettoria, vettori forza, marker eventi.
- **Scenario Panel:** motori, failure, vento, rampa, massa, condizioni iniziali.
- **Telemetry Panel:** quota, velocita, accelerazione, assetto, AoA, CG, CP, margine statico.
- **Timeline & Graphs:** replay, pause, scrub temporale, grafici quota/velocita/assetto.
- **Events / Alerts Panel:** burnout, apogeo, instabilita, superamento limiti strutturali.
- **Flight Overview Panel:** panoramica sintetica dell'intera traiettoria dal lancio all'impatto con marker chiave e stato corrente/replay.

Indicazioni di rifinitura UI applicate:
- telemetria organizzata in card invece che in lista monospaziata
- pannello eventi orientato allo stato missione e non solo al dump di testo
- timeline resa piu leggibile distinguendo chiaramente progressione missione e fase boost
- top bar con hint persistente per `F3` come accesso alla camera del vento esterna
- stati vuoti e messaggi `next action` per aiutare la lettura del flusso prima del lancio e dopo l'impatto
- `Scenario di Volo` con azione principale evidenziata e azioni secondarie rese meno invasive
- palette piu coerente e meno rumorosa tra control room, telemetria, timeline e camera del vento

Funzioni richieste:
- play, pause, reset, replay
- confronto fra piu configurazioni
- attivazione failure motori
- overlay diagnostici aerodinamici
- esportazione report
- review dei keyframe principali con stepping da tastiera e finestra di analisi puntuale

Scorciatoia dedicata:
- `F3` deve aprire una sola finestra esterna di `Wind Tunnel`.
- Questa finestra non deve replicare tutta la simulazione.
- Deve mostrare soltanto profilo del corpo, vento relativo, pressione dinamica, regime di flusso e risposta aero dei singoli componenti.

Scorciatoia di analisi:
- `K` deve scorrere i keyframe principali della missione nel workspace `Simulazione`.
- Ogni pressione di `K` deve congelare o aggiornare la vista sul keyframe successivo.
- Per ogni keyframe deve comparire una finestra dedicata con statistiche puntuali del razzo in quell'istante.

---

## 3. Modellazione e Logica di Vincolo
Il software deve impedire la creazione di "razzi impossibili" tramite un sistema di validazione gerarchica.

### A. Vincoli Strutturali
- **Validazione Gerarchica:** non e possibile inserire pinne o motori senza un `Body Tube` valido.
- **Posizionamento Corretto:** l'ogiva deve essere il terminale superiore del veicolo.
- **Transitions Consistenti:** una `Transition` deve collegare solo sezioni con diametri compatibili.
- **Motor Mount Validation:** il supporto motore deve restare contenuto nel volume disponibile.

### B. Smart Assembly
- **Snap Points:** i componenti espongono punti di aggancio matematici.
- **Auto-Alignment:** i pezzi si allineano automaticamente sull'asse longitudinale del razzo.
- **Live Rebuild:** ogni modifica geometrica aggiorna subito mesh, CG, CP e massa stimata.
- **Locking:** i componenti possono essere bloccati per evitare modifiche accidentali.

---

## 4. Overlay Tecnici

### A. Overlay nel Workspace `Modelazione`
- griglia metrica
- assi locali e globali
- gizmo di trasformazione
- marker di `CG` e `CP`
- punti di snap
- quote principali del componente selezionato
- wireframe triangolare della mesh editabile
- vertici selezionabili del componente attivo con highlight dedicato
- edge e facce selezionate evidenziati sopra il wireframe con colori separati

### B. Overlay nel Workspace `Simulazione`
- traiettoria 3D
- vettore spinta
- vettore drag
- direzione velocita relativa
- warning di instabilita
- eventi chiave della missione

---

## 5. Analisi e Diagnostica

### A. Analisi Flussi
- visualizzazione streamline
- heatmap pressioni sulla mesh
- evidenza dei punti di ristagno
- lettura per-componente di carico aerodinamico su `nose`, `body`, `transition`, `fin set`, `payload`, `motor mount`

Stato attuale implementato:
- il pannello `Wind Tunnel` del workspace `Simulazione` consente gia una selezione attiva del componente e una lettura locale sintetica
- la diagnostica del flusso legge ora anche densita aria, pressione statica, pressione totale e velocita del suono derivandole dal modello atmosferico del simulatore
- streamline, cue di stagnazione e risposta transonica devono reagire in modo coerente all'aumento di `Mach`, `q`, pressione e densita, senza restare animazioni puramente decorative
- il modulo CFD in tempo reale deve poter mostrare anche un campo particellare persistente con budget dinamico, trail temporali e indicatori espliciti di shockwave e risposta aeroelastica

### B. Stress Test
- deformazione strutturale visuale
- thermal mapping
- warning sul superamento dei limiti del materiale

Queste analisi appartengono al workspace `Simulazione`, ma devono poter riutilizzare direttamente la geometria costruita in `Modelazione`.

---

## 6. Gestione File e Progetti
- **Formato Proprietario `.rlab`:** JSON compresso con geometria, materiali, parametri di simulazione e dati scenario.
- **Export 3D:** `.stl`, `.obj`
- **Export Analisi:** `.csv`, `.pdf`

---

## 7. Metriche Live
Durante la modellazione la GUI deve aggiornare costantemente:

- stabilita statica in calibri
- massa totale
- posizione `CG`
- posizione `CP`
- apogeo stimato

Durante la simulazione la GUI deve aggiornare costantemente:

- tempo
- quota
- velocita
- accelerazione
- angolo di attacco
- assetto
- stato motori

---

## 8. Roadmap GUI
- [x] Implementare una top bar con switch esplicito `Modelazione` / `Simulazione`
- [x] Consolidare la shell principale sul workspace 3D raylib mantenendo intatta la logica applicativa esistente
- [x] Rifinire copy, naming dei pannelli e gerarchia visiva dei workspace senza toccare la logica
- [x] Riallineare la shell desktop 3D a un linguaggio Fluent-like con primary buttons contained, secondary outlined e superfici piu ariose
- [x] Introdurre hint contestuali e stati vuoti piu guidati nei pannelli principali
- [x] Introdurre shortcut dei tool di modellazione e hint dipendenti dal tool attivo nel workspace 3D
- [x] Introdurre analisi keyframe con stepping preciso e finestra dedicata di diagnostica missione
- [ ] Evolvere il workspace `Modelazione` con viewport e strumenti piu vicini a un CAD leggero
- [~] Introdurre base mesh poligonale persistente con vertex/index buffers, wireframe, selezione topologica e primi operatori diretti
- [ ] Aggiungere outliner e property editor dei componenti ancora piu ricchi nel frontend 3D
- [ ] Espandere il workspace `Simulazione` con grafici e timeline piu avanzati
- [ ] Sviluppare shader heatmap per pressioni sulla mesh
- [ ] Scrivere parser salvataggio/caricamento `.rlab`
