[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/eiNgq3fR)
[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-2e0aaae1b6195c2367325f4f02e2d04e9abb55f0b24a779b69b11b9e10269abc.svg)](https://classroom.github.com/online_ide?assignment_repo_id=22048053&assignment_repo_type=AssignmentRepo)
# Dwell Time Transmilenio
<!-- Cambiar el titulo "Proyecto" por el nombre del proyecto -->


# Integrantes
- Tomás Esteban Cuartas Feliciano  
- Salomón Velasco Rueda

Indice:
1. [Descripción](#descripción)
2. [Informe](#informe)
3. [Implementación](#implementación)
4. [Lista de anexos](#anexos)

## Descripción

Este proyecto implementa un **sistema de guía adaptativa de puertas** (A y B) para reducir el *dwell time* en estaciones tipo BRT (TransMilenio), orientando a los usuarios hacia la puerta con menor ocupación.

### Idea general
- Se usan **3 sensores ultrasónicos**:
  - 1 sensor para detectar si el **bus está cerca** (habilita/deshabilita todo el sistema).
  - 2 sensores apuntando al interior del bus (uno por puerta) para estimar **ocupación por puerta**.
- Actuadores:
  - **2 buzzers** (uno por puerta) para atraer usuarios.
  - **2 LEDs RGB** en forma de flecha (por puerta). Se usan solo canales **Rojo** y **Verde** para obtener: rojo, verde y amarillo (rojo+verde).

> Nota importante (consistencia con la entrega final): aunque el concepto original incluye **3 botones por puerta** para estimar gente afuera, en la versión final integrada la decisión se basa en los **sensores ultrasónicos** (por restricciones de integración/pines).

### Lógica de decisión (resumen)
1. Si el bus **no** está cerca → todo apagado.
2. Si el bus está cerca → se clasifica cada puerta en 3 niveles: **0 libre**, **1 medio**, **2 lleno**.
3. Reglas:
- **Ambas libres (0,0):** suenan ambos buzzers.
- **Una puerta más libre que la otra:**  
  - La flecha (verde) indica redirección desde la puerta más congestionada hacia la más libre.  
  - En la puerta más libre se muestra el estado de la otra: **amarillo** si está en “medio”, **rojo** si está “llena”.  
  - El buzzer solo suena en la puerta **libre** (para no atraer hacia congestión).
- **Iguales:**  
  - (1,1) → amarillo en ambas.  
  - (2,2) → todo apagado (no hay “mejor opción”).

### Plataforma / herramientas
- **Vivado**: Block Design con Zynq PS + AXI GPIO.
- **Vitis (C)**: aplicación `main.c` que lee ECHOs, genera TRIGs y controla LEDs/buzzers por registros mapeados.

## Informe

- [Informe final (PDF)](./src/ENTREGA_FINAL_PROYECTO.pdf)
- [Block Design (PDF)](./src/bd_proyecto_tm.pdf)

## Implementación

- [Video de funcionamiento (Drive)](https://drive.google.com/file/d/1HAOiNshg6l43BvAlzujdxhfv5xr37RkS/view?usp=drivesdk)

## Anexos

Archivos (ubicados en `src/`):
- [Código principal en C](./src/main.c)
- [Block Design exportado](./src/bd_proyecto_tm.pdf)
- [Informe final](./src/ENTREGA_FINAL_PROYECTO.pdf)
