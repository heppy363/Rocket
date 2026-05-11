# Approved Technology Stack

## Core Language
- **C++20** come standard minimo per tutto il codice di simulazione.

## Approved Libraries
- **Raylib (C++ Wrapper)** per rendering 3D desktop.
- **libcurl** per integrazione con API meteo.

## Build & Portability
- Toolchain supportate: **GCC**, **Clang**, **MSVC**.
- Dipendenze limitate al minimo: preferire STL quando sufficiente.

## Architectural Rules
- Separazione rigorosa tra `include/` e `src/`.
- `double` per fisica e integrazione numerica.
- `float` solo nei layer di rendering quando necessario.
