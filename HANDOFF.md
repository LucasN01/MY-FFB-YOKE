\# HANDOFF entre agentes



\## Reglas

1\. Un solo agente edita a la vez. Cada tarea va en su propia rama: `git checkout -b tarea/<nombre>`.

2\. Al terminar, el agente completa la sección "Última entrega" y hace commit.

3\. El otro agente revisa el diff (`git diff master...HEAD`) sin editar, y anota hallazgos en "Revisión".

4\. Nada se integra a `master` sin que el usuario valide en hardware.

5\. Antes de afirmar algo del código, indicar la fuente: \[Grafo], \[INO] o \[Código], con archivo y línea. Las aristas INFERRED se tratan como hipótesis.

6\. Tras editar codigo, ejecutar `graphify update .` (o `herramientas/flujo/graphify-sync.cmd`, que hace lo mismo).

7\. No modificar constantes de seguridad (límites de PWM, `MOTOR\_PWM\_HARD\_LIMIT`, `SIMCONNECT\_EXTRA\_LIMIT`) sin aprobación explícita del usuario.

8\. Nunca usar `git add .` ni `git add -A`: agregar archivos por nombre.

9\. Un hallazgo de "posible bug" se verifica leyendo el código involucrado antes de proponer cambios. Informar la evidencia (archivo y línea), no solo la sospecha.

10\. Las reglas de versionado, documentacion, ahorro de tokens y pruebas estan en AGENTS.md (Codex) y CLAUDE.md (Claude); no se repiten aca.

11\. "Ultima entrega" y "Revision" contienen solo la tarea en curso: al empezar una tarea nueva se reemplazan (lo anterior queda en el historial de git).

\## Plan para Codex

Tarea: sin trabajo pendiente para Codex

(Claude, 2026-11-06.) La revision de la calibracion de topes termino aprobada en la ronda 2.

\## Última entrega

- Calibracion de topes con 4 finales de carrera implementada por Claude en `mejora/limite-topes`, sobre V2.9. Bandera `YOKE_ENDSTOP_CALIBRATION`, 0 por defecto; modo de posicion sin cambios (1, simulacion). Diseno: `LEEME md/DISENO_calibracion_topes.md`.
- Limitador de topes (V2.9): diseno `LEEME md/DISENO_limite_topes.md`, revision de Codex de dos rondas, tesis actualizada.
- Tesis actualizada (`tesis/capitulos/09_damper.tex`, seccion `sec:calibracion_topes`) y PDF generado en `tesis/main.pdf`.
- Version V3.0 creada. La carpeta del usuario `modelizacion motor 775/` (`tacometro_lm393.ino`) queda fuera de git solo en este equipo, por `.git/info/exclude`, a pedido del usuario: `git status` no la muestra y no entra en las versiones.
- Pendiente del usuario: ensayos F-calibracion y F-tope con hardware (`LEEME md/ENSAYOS.md`), polaridad de los sensores y tiempo de ciclo.
- Sigue pendiente de la tarea anterior: ensayos H0-H13 de la migracion a Pico SDK (detalle en `LEEME md/DISENO_migracion_pico_sdk.md` seccion 7).

\## Revisión

Ronda 1 de Codex (2026-11-06): requiere cambios. H1 (sensor reaccionado durante el antirrebote habilitaba la fuerza): aplicado. H2 (cruce entre ejes con movimientos combinados): limite aceptado y documentado.
Ronda 2 de Codex (2026-11-06): **aprobado**, con evidencia en los 21 puntos de la lista. H2 no se pudo refutar. El punto 16 queda parcial solo por la medicion fisica, pendiente del usuario.

\## Ensayos del usuario

- Protocolo V1 y R1-R3 (usuario, 2026-10-13, V2.3, motores apagados). Carpetas `ensayos/20261013-191138_arranque` a `20261013-192018_arranque`. Análisis de Claude.
  - **V1 (HardFault provocado): el instrumento funciona de punta a punta.**
    - `BOOT reset=WDT wd=0x1 etapa=15:PRUEBA falla=1 exc=0 pc=0x10007d06 lr=0x100060bd cfsr=0x00010000`.
    - `FALLA.ps1` lleva el PC a `loop` (`MiYoke_PD.ino:568` = `__builtin_trap()`) y el LR a `StageTimer::enter` (la última llamada, `markStage(TestHang)`).
    - CFSR 0x00010000 = UFSR.UNDEFINSTR, que corresponde a la instrucción UDF de `__builtin_trap`. `exc=0` (hilo). Todo coherente.
  - **R1 y R3 (carga de UF2 sin desenchufar):** `reset=SOFT wd=0x1`, sin falla. Es el reinicio del bootrom después de copiar el UF2.
  - **R2 y R3 (desenchufar y enchufar, 4 veces):** `reset=ENCENDIDO`, sin falla.
  - **El HardFault del 13/10 (`reset=WDT falla=1 etapa=0`) no se reprodujo en 6 arranques.** No es un efecto de la carga del UF2, que da `SOFT`: fue una falla real en ejecución.
  - **HALLAZGO: stack libre mínimo muy bajo.** De 2048 B quedaron libres 448, 544, 544, 560, 568 y 688 B, en sesiones de solo ~15 s de replay en monitor.
    - En cada sesión el valor **baja con el tiempo**, por ejemplo 824 → 800 → 776 → 560 B en 11 s.
    - Está por debajo del umbral de 512 B del diseño (§5): **es evidencia a favor de H-b, desborde de stack**. En un vuelo largo, con menú, LCD y todas las líneas de diagnóstico, el uso puede ser mayor.
    - Un desborde corrompe la RAM contigua sin aviso (MSPLIM=0) y explicaría una falla posterior fuera del lazo (`etapa=0`).
  - **Siguiente paso propuesto (tarea por Flujo, pendiente de aprobación del usuario):**
    1. medir qué consume el stack (locales grandes en `loop()`, `snprintf` de newlib, interrupciones anidadas);
    2. ver si el core permite agrandar el stack del núcleo 0 o activar MSPLIM, para que un desborde falle enseguida y con el PC exacto;
    3. bajar el consumo o agrandar el stack, con criterio medido: stack libre ≥ 1 KB en un vuelo completo con `ENSAYO.cmd`.

- (vacío: tarea nueva. El ensayo CICLOS y el hallazgo del HardFault están en `master`, commit `bccd1b7`.)
