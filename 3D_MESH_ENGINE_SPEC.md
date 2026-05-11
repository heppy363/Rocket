# 3D Mesh Engine Spec

## Obiettivo

Il motore di modellazione 3D deve supportare una mesh persistente per ogni componente del razzo, non solo una generazione procedurale usa-e-getta.

## Requisiti Implementati

- ogni pezzo principale (`nose`, `body`, `transition`, `fins`, `payload`, `motor mount`) espone ora una topologia dedicata
- la topologia e rappresentata con:
  - `Vertex Buffer`: posizioni, normali, coordinate UV
  - `Index Buffer`: triangoli indicizzati
- il runtime puo accedere alla mesh tramite `MeshGenerator::componentMesh(...)`
- il runtime puo accedere anche a cache derivate di `edge` e `face` tramite `componentEdges(...)` e `componentFaces(...)`
- il runtime puo leggere:
  - numero di vertici
  - numero di edge unici
  - numero di facce
  - numero di triangoli
  - posizione dei vertici nel body frame
- il runtime puo aggiornare i vertici singolarmente con `setComponentVertexPosition(...)`
- il runtime puo applicare primi operatori topologici diretti con:
  - `extrudeComponentFace(...)`
  - `bevelComponentFace(...)`
  - `loopCutComponentEdge(...)`
- il viewport puo mostrare il wireframe triangolare reale del componente

## Pipeline Corrente

1. La geometria procedurale genera la topologia base del componente.
2. La topologia viene salvata in una struttura persistente per-componente.
3. Gli eventuali `vertex modifiers` vengono applicati ai vertici della topologia.
4. Dalla topologia vengono ricostruite cache derivate per `face normals`, `face centers` ed `edge adjacency`.
5. Le normali vengono ricalcolate dalla rete triangolare.
6. La mesh GPU viene caricata da vertex/index buffers aggiornati.

## Stato Attuale

- supporto a vertex dragging diretto
- supporto al wireframe reale
- accesso alla rete triangolare, agli edge unici e alle facce dal runtime e dall'inspector
- selezione `Vertex / Edge / Face` dal workspace `Modelazione`
- `extrude` iniziale per facce triangolari
- `bevel` iniziale per facce triangolari tramite inset + ring
- `loop cut` iniziale locale sull'edge selezionato

## Limiti Correnti

- il `loop cut` e ancora locale al segmento selezionato, non percorre un intero edge loop complesso
- le operazioni topologiche vivono nella mesh runtime del componente e non sono ancora serializzate nel progetto
- non esistono ancora `multi-select`, `soft selection`, `edge slide`, `inset` esplicito o `bevel` edge-to-edge completo

## Prossimi Passi

- manipolatori 3D piu robusti per assi locali/world
- soft selection e falloff
- operazioni topologiche piu complete (`extrude` multi-face, `bevel` edge/vertex, `subdivide`, `loop cut` ad anello)
- serializzazione della topologia editata nel formato progetto
