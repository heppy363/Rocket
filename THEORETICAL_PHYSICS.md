# Theoretical Physics & Math Reference

## 1. Equation of Motion (6-DOF)
Il moto e governato dalla seconda legge di Newton estesa:
$$F_{tot} = F_{thrust} + F_{drag} + F_{gravity}$$
$$a = \frac{F_{tot}}{m(t)}$$

Nota: `m(t)` diminuisce durante la combustione.

## 2. Aerodynamic Drag Force
Calcolo della forza frenante:
$$F_D = \frac{1}{2} \cdot \rho(h) \cdot v^2 \cdot C_D \cdot A$$

- $\rho(h)$: densita dell'aria variabile con l'altezza `h`
- $C_D$: coefficiente di resistenza, variabile in base al Mach

## 3. Stability (The Static Margin)
Il razzo e stabile se:
$$X_{cp} - X_{cg} > 1.0 \cdot \text{Diametro}$$

Il simulatore deve calcolare il centro di gravita (`X_cg`) come media pesata delle masse e il centro di pressione (`X_cp`) usando una base tipo Barrowman.

## 3B. Wind Tunnel Interpretation
La modalita `F3` deve aiutare a leggere il comportamento locale della geometria:

- pressione piu alta in prossimita del punto di ristagno sull'ogiva
- crescita del drag distribuito lungo il corpo con l'aumento di `q`
- maggiore sensibilita delle pinne al crescere di `AoA`
- variazione del wake in coda per `transition` e `motor mount`

## 4. Integrazione RK4
Per ogni step temporale `dt`:

`k1 = f(t, y)`

`k2 = f(t + dt/2, y + k1 * dt/2)`

`k3 = f(t + dt/2, y + k2 * dt/2)`

`k4 = f(t + dt, y + k3 * dt)`

`y_next = y + (dt / 6) * (k1 + 2*k2 + 2*k3 + k4)`
