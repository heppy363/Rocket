# Guida Modellazione 3D - The Rocket Lab

## 📐 Nuovo Sistema di Modellazione

Il workspace `Modelazione` supporta ora una modellazione 3D completamente interattiva con supporto sia per parametri procedurali che per modificazione libera per-vertex e primi strumenti topologici tipo Blender.

---

## 🎯 Workflow Principale

### 1. Selezione Componente
- Clicca direttamente sul razzo nella viewport per selezionare un componente
- Oppure usa i pulsanti nel pannello `Inspector Componente`
- Componenti disponibili:
  - **Nose Cone** (Ogiva) - con 4 profili: Conical, Tangent Ogive, Parabolic, LD-Haack
  - **Body Tube** (Fusoliera) - cilindro principale del veicolo
  - **Transition** (Transizione) - raccordo tra fusoliera e motori
  - **Fin Set** (Pinne) - stabilizzatori con 3 profili: Trapezoidal, Elliptical, Airfoil
  - **Motor Mount** (Cluster motori) - configurazione della spinta
  - **Payload** (Carico) - sezione di payload

### 2. Editing Parametrico
Nel pannello `Inspector Componente` puoi modificare:
- **Geometria**: lunghezza, diametro, spessore, angoli
- **Materiale**: PLA-CF, Alluminio 6061, PVC, Fiberglass, Carbon Fiber, Betulla Aircraft, Phenolic Tube
- **Control Vertices**: scale factors per deformazione procedurale liscia
  - Ad esempio: `Body Mid Radius` per controllare il profilo dell'ogiva

Ogni materiale mostra ora anche:
- densita
- modulo elastico `E`
- resistenza di snervamento
- temperatura massima di servizio
- `q` dinamica consigliata per il componente attivo

### 3. Editing Mesh Diretto (Nuovo!)
Dopo aver selezionato un componente:

**Abilitare l'editing libero:**
1. Scorri fino a "Vertici Liberi" nel pannello `Inspector Componente`
2. Clicca "Editing Disabilitato" → "Editing Abilitato"
3. Ora la mesh entra in modalita editabile e puoi passare tra `Vertex`, `Edge` e `Face`

**Modalita Vertex:**
1. Clicca su un vertice verde in viewport per selezionarlo
2. Premi [2] per attivare il tool **Move**
3. Trascina il vertice nella viewport per deformarlo liberamente
4. L'offset del vertice viene tracciato e visualizzato nell'inspector
5. Attiva `Wireframe` per leggere la rete triangolare del pezzo
6. I vertici ora sono mostrati come marker piccoli in overlay 2D, non come sfere grandi nel mondo 3D

**Modalita Edge:**
1. Passa a `Edge` nell'inspector
2. Clicca un marker azzurro sulla mesh
3. Usa `Loop Cut Edge` per inserire un nuovo vertice sul segmento selezionato e spezzare localmente le facce adiacenti

**Modalita Face:**
1. Passa a `Face` nell'inspector
2. Clicca un marker arancione al centro della faccia
3. Regola `Extrude Dist`, `Bevel Ratio` e `Bevel Offset`
4. Usa `Extrude Face` per creare volume locale oppure `Bevel Face` per smussare la faccia con un inset triangolare

**Topologia mesh disponibile:**
- ogni pezzo principale mantiene ora una mesh editabile con `vertex buffer` e `index buffer`
- l'inspector mostra il numero di vertici, edge e facce del componente attivo
- il drag dei vertici aggiorna direttamente la mesh persistente del componente, non solo i parametri procedurali
- il runtime mantiene anche cache topologiche per `edge` e `face`, usate sia dal picking che dagli operatori diretti

**Resettare i vertici:**
- Clicca "Resetta Vertici" per annullare tutte le modifiche al componente
- I parametri procedurali restano intatti

---

## 🎨 Overlay e Visualizzazione

### Griglia Locale
- Attiva con il toggle "Local Grid"
- **Passo**: distanza tra le linee della griglia (modificabile da 0.005 m a 0.250 m)
- **Estensione**: numero di celle visualizzate (da 4 a 40)
- La griglia si centra automaticamente sul componente selezionato

### Handle di Controllo
- **Handle ON/OFF**: visualizza i punti di controllo parametrici
- I colori indicano il tipo di modifica:
  - Giallo/Arancione: Ogiva
  - Azzurro: Fusoliera
  - Viola: Transizione
  - Rosa: Pinne
  - Verde: Vertici liberi modificati

### Viewport
- **Snap ON**: allinea automaticamente i parametri alla griglia (utile per precisione)
- **Metrics**: mostra CG, CP e margine di stabilità statica
- **Wireframe**: visualizza la mesh in wireframe per vedere la struttura interna
- **Mesh Mode**: passa rapidamente tra selezione `Vertex`, `Edge` e `Face`
- **Diagnostics**: info avanzate di rendering e geometria
- **Testo pannelli**: hint, card e status bar restano ora contenuti nei riquadri con wrapping e clipping
- **Handle puliti**: i punti di editing e i control point parametrici sono mostrati come marker overlay compatti, non come sfere 3D grandi

---

## 🛠️ Strumenti (Shortcuts)

| Tasto | Tool | Descrizione |
|-------|------|------------|
| **[1]** | Select | Seleziona componenti e handle direttamente in viewport |
| **[2]** | Move | Trascina handle o vertici per modificare la forma |
| **[3]** | Rotate | Visualizza in diverse prospettive (non modifica geometria) |
| **[4]** | Scale | Modifica le scale dei control vertices |
| **[5]** | Add Part | Aggiunge preset di progetto o componente |
| **[6]** | Measure | Legge lunghezze e distanze nel contesto 3D |
| **[TAB]** | CAD Camera | Abilita navigazione tipo CAD con MMB orbita |

---

## 📚 Libreria Preset

### Preset di Progetto (5 configurazioni complete)
- Research Starter - per esperimenti didattici
- Sport Trainer - modello di allenamento versatile
- High Altitude - ottimizzato per quota massima
- Minimum Diameter - configurazione compatta
- Heavy Lift - per carichi massimi

### Preset di Componente
Ogni componente selezionato mostra preset specifici:

**Ogiva:**
- Conical Sprint / Long Range
- Haack Low Drag
- Ogive Stable
- Parabolic Payload

**Fusoliera:**
- Minimum Diameter
- Avionics Bay
- Long Burner
- Heavy Structure
- Wide Lift Core

**Transizione:**
- Straight Interstage
- Boat Tail
- Conical Coupler
- Aggressive Tail
- Payload Shoulder

**Pinne:**
- Sport Trapezoid
- Supersonic Airfoil
- Clipped Delta
- Stable Elliptical
- Heavy Lift Canards

**Motori:**
- Single Sustainer
- Twin Booster
- 3-Motor Cluster
- 4-Motor Cluster
- 5-Motor Ring

**Payload:**
- Compact Payload
- Avionics + Camera
- Dual Deploy Bay
- Science Probe
- Heavy Recovery Stack

---

## 📊 Inspector Componente

Quando selezioni un componente, l'inspector mostra:

1. **Nome componente** e hint contextuale
2. **Tool attivo** con scorciatoia e descrizione
3. **Parametri numerici** con stepper (+/- per modificare)
4. **Scelta materiale** con anteprima colore
5. **Control Vertices** per deformazione procedurale liscia
6. **Profilo geometrico** (per Ogiva e Pinne)
7. **Sezione Vertici Liberi** per editing mesh `Vertex / Edge / Face`
8. **Info CG/CP** nel pannello riepilogo inferiore

---

## 🎯 Workflow Esempio: Modificare un Profilo di Pinne

1. Premi **F1** per attivare `Modelazione` (se non già attivo)
2. Clicca sulle pinne nel modello 3D
3. Nel pannello destro vedrai i parametri delle pinne
4. Usa i stepper per modificare:
   - **Fin Front**: posizione longitudinale
   - **Fin Span**: lunghezza delle pinne
   - **Tip LE/TE Offset**: curvatura della punta
5. Clicca su un preset (es. "Clipped Delta") per applicare una configurazione
6. Scorri giù fino a "Vertici Liberi"
7. Clicca "Editing Abilitato" per entrare nella modalita mesh
8. Usa `Vertex` per rifinire i punti oppure `Face` per estrudere e smussare localmente
9. Se serve piu densita topologica, passa a `Edge` e applica un `Loop Cut Edge`
10. I cambiamenti si vedono live nella viewport

---

## ⚠️ Validazione Geometria

Il sistema valida automaticamente:
- ❌ Razzo impossibile (es. pinne senza fusoliera)
- ❌ Auto-intersezioni tra componenti
- ❌ Parametri fuori range (es. diametro negativo)
- ✅ Aggiornamento automatico di CG, CP e margine statico

---

## 💾 Esportazione e Simulazione

Una volta soddisfatto della geometria:

1. Premi **F2** per passare a `Simulazione`
2. Configura il lanciatore (quota, vento, rampa)
3. Premi **Spazio** per avviare la simulazione
4. Osserva la traiettoria, l'assetto e i dati di volo in tempo reale
5. Premi **K** per analizzare i keyframe principali della missione
6. Premi **F3** per aprire la camera del vento e analizzare l'aerodinamica

---

## 🔧 Tips e Trucchi

- **Precisione**: attiva "Snap ON" per allinearti alla griglia
- **Vista multipla**: usa [3] Rotate per alternare Front/Side/Top/Perspective
- **Grid dinamica**: modifica il "Passo" della griglia in base alla scala di dettaglio
- **Live feedback**: tutti i cambiamenti si riflettono istantaneamente in CG, CP e massa stimata
- **Material feedback**: la scelta del materiale modifica anche damping, inerzia secca e budget strutturale di `q`
- **Undo geometria**: resetta un componente applicando un preset diverso e poi il precedente
- **Workflow veloce**: prediligi i preset per forma grossolana, poi affina con control vertices
- **Workflow mesh**: inizia con `Vertex`, poi usa `Face` per volume locale e `Edge` per aggiungere dettaglio dove serve

---

## 🚀 Prossime Funzionalità Pianificate

- Manipolazione gizmo 3D completa (move/rotate/scale di vertici, edge e facce)
- Smooth deformation con falloff su influence radius
- Libreria geometrica estesa con dati reali
- Import/export di profili di componenti custom
- Hit test migliorato per selezione precisa di vertici e loop cut esteso a veri edge loop

---

**Versione**: 2026-05-11 | **Stato**: Beta con editing mesh `Vertex / Edge / Face` in corso di sviluppo
