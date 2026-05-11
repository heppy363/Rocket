# Ballistics & Trajectory Physics Engine

Il simulatore deve risolvere iterativamente le equazioni del moto usando integrazione numerica RK4, ma la precisione finale dipende soprattutto dalla qualita dei modelli fisici che alimentano le derivate di stato.

## 1. Modello Atmosferico Dinamico
Le condizioni del sito di lancio devono partire da dati meteo legati a coordinate geografiche:

- pressione al suolo `P0`
- temperatura al suolo `T0`
- umidita relativa
- vento medio
- raffiche
- quota del sito

Da queste grandezze si costruisce il profilo atmosferico locale lungo la quota.

### A. Temperatura e Pressione in Troposfera
La temperatura varia con il lapse rate standard:

`T(h) = T0 - L * h`

dove `L = 0.0065 K/m`

La pressione viene aggiornata con la relazione barometrica:

`P(h) = P0 * (1 - L*h/T0)^((g*M)/(R*L))`

### B. Densita dell'Aria con Correzione di Umidita
La densita non deve essere trattata come semplice esponenziale fissa se abbiamo dati meteo reali.

`rho = Pd / (Rd * T) + Pv / (Rv * T)`

dove:

- `Pd` e la pressione parziale dell'aria secca
- `Pv` e la pressione del vapore
- `Rd` e `Rv` sono le costanti specifiche per aria secca e vapor d'acqua

## 2. Equazione Vettoriale del Moto
Per ogni step temporale:

`a = (Fthrust + Fdrag + Flift + Fgravity + Frecovery) / m(t)`

La velocita da usare nei termini aerodinamici non e quella assoluta del razzo, ma quella relativa al vento:

`Vrel = Vrocket - Vwind`

## 3. Drag Aerodinamico
Il drag dipende da densita, velocita relativa, area di riferimento e numero di Mach:

`Fdrag = -0.5 * rho * |Vrel|^2 * Cd(M) * A * Vrel_hat`

### A. Numero di Mach
`M = |Vrel| / a_sound`

con:

`a_sound = sqrt(gamma * R * T(h))`

### B. Nota Modellistica
Un modello realistico non deve usare un solo `Cd` costante su tutto il volo. Serve almeno una variazione semplificata tra:

- subsonico
- transonico
- supersonico iniziale

## 4. Forza Normale, Lift e Momento Ripristinante
La componente normale aerodinamica e fondamentale sia per la traiettoria sia per la stabilita:

`Flift = 0.5 * rho * |Vrel|^2 * A * CNalpha * alpha`

dove:

- `alpha` e l'angolo di attacco tra asse del razzo e aria relativa
- `CNalpha` deriva dal modello geometrico del razzo, ad esempio tramite Barrowman

Il momento restaurativo dipende dalla distanza `CP - CG` e dalla forza aerodinamica trasformata nel frame del corpo.

## 5. Massa Variabile
Durante la combustione:

- `m(t) = mdry + mpropellant(t)`
- `dm/dt < 0`

Se il motore e descritto solo con massa propellente e burn time, la variazione puo essere modellata come mass flow medio. Se sono disponibili spinta e impulso specifico, la forma piu fisica e:

`dm/dt = -T / (g0 * Isp)`

## 6. Vento, Gust e Weather Cocking
Il vento non deve solo traslare il punto di impatto: modifica `Vrel`, quindi drag, angolo di attacco e momento di imbardata/beccheggio.

Profilo minimo richiesto:

- vento medio al suolo
- shear verticale semplice
- contributo di raffica dipendente dal tempo

## 7. Gravita Locale
Per simulazioni piu fedeli:

- la gravita puo dipendere da latitudine
- la gravita puo essere corretta con la quota del sito e con l'altitudine di volo

## 8. Fase di Discesa
La fisica non termina all'apogeo.

### A. Discesa Balistica
Se nessun sistema di recovery e attivo, il corpo rientra con:

- peso
- drag aerodinamico del corpo
- vento

### B. Recovery Descent
Se il paracadute o il recovery system viene aperto:

`Frecovery = -0.5 * rho * |Vrel|^2 * Cd_recovery * A_recovery * Vrel_hat`

La logica di deploy puo dipendere da:

- quota
- ritardo temporale
- evento di missione

## 9. Output Balistici Attesi
La simulazione deve permettere di leggere almeno:

- traiettoria di salita
- apogeo
- velocita massima
- Mach massimo
- pressione dinamica massima
- deriva laterale
- traiettoria di discesa
- punto stimato di atterraggio

## 9B. Output Aero Locali per Camera del Vento
La modalita `F3` deve sintetizzare anche una lettura locale, non solo globale, del comportamento aerodinamico:

- risposta della `Nose Cone` nella zona di ristagno
- contributo del `Body Tube` a skin drag e sviluppo dello strato limite
- influenza della `Transition` sul recupero di pressione in coda
- sensibilita del `Fin Set` a `AoA`, forza normale e damping
- effetto di `Payload` e volume anteriore sull'equilibrio aero
- impronta di `Motor Mount` e regione nozzle su wake e base drag

## 10. Algoritmo di Integrazione
Pseudo-codice:

```cpp
State computeNextState(State current, double dt) {
    auto k1 = derivatives(current);
    auto k2 = derivatives(current + k1 * (dt / 2));
    auto k3 = derivatives(current + k2 * (dt / 2));
    auto k4 = derivatives(current + k3 * dt);
    return current + (k1 + k2 * 2 + k3 * 2 + k4) * (dt / 6);
}
```
