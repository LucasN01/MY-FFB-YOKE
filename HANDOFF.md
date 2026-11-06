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

Tarea: Revisar calibracion de topes con finales de carrera - ronda 2

(Claude, 2026-11-06.) Segunda y ultima ronda de revision. **Codex solo revisa, no edita ni commitea** (se permite crear copias y salida en `tmp/`). Rama `mejora/limite-topes`, sin version: la crea Claude despues del aprobado.

1. Leer `tmp/respuesta_claude_ronda1.md`, `tmp/revision_codex.md` (tu ronda 1) y `tmp/lista_control_calibracion_topes.md`. Ver el cambio con `git diff c03caaf..HEAD`.
2. H1: comprobar la correccion con tu reproduccion y en una copia en `tmp/` quitar `|| pressed[i]` para ver que la regresion falla.
3. H2: intentar refutar la decision con el criterio escrito en `tmp/respuesta_claude_ronda1.md`. Si no se puede refutar, marcarlo como limite aceptado y no como defecto.
4. Puntos 11, 12 y 13: verificar la evidencia y mirar el desensamblado del modo 1 (`tmp/comparador_v29/`, `tmp/v29_fuente`). No hace falta repetir todo si la evidencia alcanza.
5. Buscar defectos nuevos que hayan entrado con este cambio. Volver a marcar cada punto de la lista como cumple, parcial o no, con evidencia; lo fisico de ensayo queda como pendiente del usuario, no bloqueante. Pruebas: `powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1` (agregar `-Todo -PruebaFalla` solo si hace falta).
6. Escribir temprano y dejar el resultado en `tmp/revision_codex.md`, reemplazando su contenido: hallazgos, tabla por punto y veredicto "aprobado" o "requiere cambios". No tocar `tesis/`, `HANDOFF.md` ni crear versiones.

\## Última entrega

- Calibracion de topes con 4 finales de carrera implementada por Claude en `mejora/limite-topes`, sobre V2.9. Bandera `YOKE_ENDSTOP_CALIBRATION`, 0 por defecto; modo de posicion sin cambios (1, simulacion). Diseno: `LEEME md/DISENO_calibracion_topes.md`.
- Limitador de topes (V2.9): diseno `LEEME md/DISENO_limite_topes.md`, revision de Codex de dos rondas, tesis actualizada.
- Pendiente: revision de Codex, version, tesis y PDF (el usuario pidio generarlo al final), ensayos F-calibracion y F-tope con hardware (`LEEME md/ENSAYOS.md`) y tiempo de ciclo.
- Sigue pendiente de la tarea anterior: ensayos H0-H13 de la migracion a Pico SDK (detalle en `LEEME md/DISENO_migracion_pico_sdk.md` seccion 7).

\## Revisión

Ronda 1 de Codex (2026-11-06): requiere cambios. H1 (sensor reaccionado durante el antirrebote habilitaba la fuerza): aplicado. H2 (cruce entre ejes con movimientos combinados): decision de diseno, limite aceptado y documentado, a refutar por Codex. Puntos parciales 11, 12 y 13: evidencia en `tmp/respuesta_claude_ronda1.md`. Ronda 2 pendiente.

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
