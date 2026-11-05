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

(Sin trabajo pendiente para Codex.)

\## Última entrega

- Limitador de topes implementado por Claude en la rama `mejora/limite-topes`, sobre `firmware/pico-sdk`, para los cuatro modos de posicion. Diseno: `LEEME md/DISENO_limite_topes.md`.
- `probar.ps1 -Todo -PruebaFalla` (Claude y Codex, mismo codigo): OK simconnect_checks (1990580), OK comparar_ensayo, OK hal_checks (694), OK launcher_port_checks (19), OK Pico 2 configurado 65692/8608, encoders 66972/8924, potenciometros 66540/8708, AS5600 67644/8772, prueba falla 66580/8708 (flash/RAM), OK puente SimConnect.
- Pendiente: ensayo F-tope con hardware (`LEEME md/ENSAYOS.md`) en cada modo de posicion que se use; tiempo de ciclo. Tesis actualizada por Claude (`09_damper.tex`, seccion `sec:limite_topes`, y `03_centrado.tex`). Version V2.9 creada.
- Sigue pendiente de la tarea anterior: ensayos H0-H13 de la migracion a Pico SDK (detalle en `LEEME md/DISENO_migracion_pico_sdk.md` seccion 7).

\## Revisión

Dos rondas de Codex con lista de control (`tmp/revision_codex.md`, no versionado).

- H1 (el damper puede aumentar una orden hacia el centro): objetado por Claude; Codex intento refutar la objecion y no pudo. Se conserva la ley; I3 y documentacion corregidos, prueba de regresion agregada.
- H2 (la frase sobre saturacion de I3 era falsa con saturacion parcial): aplicado con el texto exacto de Codex, solo documentacion y un comentario de prueba, sin tercera ronda.
- Veredicto formal de la ronda 2: "requiere cambios" solo por H2, sin cambios pedidos al firmware. Puntos 10 y 12 cumplen; punto 9 parcial hasta tener un ensayo real; punto 16 parcial por H2, ya aplicado. Tiempo de ejecucion y sensacion mecanica sin medir.

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
