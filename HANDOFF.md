\# HANDOFF entre agentes



\## Reglas

1\. Un solo agente edita a la vez. Cada tarea va en su propia rama: `git checkout -b tarea/<nombre>`.

2\. Al terminar, el agente completa la sección "Última entrega" y hace commit.

3\. El otro agente revisa el diff (`git diff master...HEAD`) sin editar, y anota hallazgos en "Revisión".

4\. Nada se integra a `master` sin que el usuario valide en hardware.

5\. Antes de afirmar algo del código, indicar la fuente: \[Grafo], \[INO] o \[Código], con archivo y línea. Las aristas INFERRED se tratan como hipótesis.

6\. Tras editar `MiYoke\_PD.ino`, ejecutar `herramientas/flujo/graphify-sync.cmd`.

7\. No modificar constantes de seguridad (límites de PWM, `MOTOR\_PWM\_HARD\_LIMIT`, `SIMCONNECT\_EXTRA\_LIMIT`) sin aprobación explícita del usuario.

8\. Nunca usar `git add .` ni `git add -A`: la carpeta graphify-ino/ no debe versionarse. Agregar archivos por nombre.

9\. Un hallazgo de "posible bug" se verifica leyendo el código involucrado antes de proponer cambios. Informar la evidencia (archivo y línea), no solo la sospecha.

10\. Las reglas de versionado, documentacion, ahorro de tokens y pruebas estan en AGENTS.md (Codex) y CLAUDE.md (Claude); no se repiten aca.

11\. "Ultima entrega" y "Revision" contienen solo la tarea en curso: al empezar una tarea nueva se reemplazan (lo anterior queda en el historial de git).

\## Plan para Codex

Tarea: Encoders magneticos AS5600 como fuente de posicion 3

(Claude, 2026-10-22.) Rama: `mejora/encoder-as5600` (ya creada desde `master`, con el diseño commiteado). Diseño cerrado: `LEEME md/DISENO_encoder_as5600.md`. Aplicarlo tal cual; si algo no coincide con el código real, no improvisar: anotar `BLOQUEO:` en "Última entrega" y terminar.

Dos partes (archivos nuevos, lógica con estado, renombre en muchos archivos y 16 pruebas: riesgo de agotar el contexto en una sola sesión).

Para todas las partes:
- Orientarse con `graphify query`/`explain`; en archivos grandes leer solo las líneas ubicadas con grep. En el diseño, leer solo las secciones indicadas.
- No tocar `tesis/` ni `graphify-ino/` (hay cambios del usuario sin commit en `tesis/main.tex` y archivos sin seguimiento en `tesis/`: no agregarlos). Commits por nombre, nunca `git add .`/`-A`.
- No modificar constantes de seguridad.
- Pruebas solo con `powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1 ...`; copiar en HANDOFF solo las líneas OK/FALLO.

\### Parte 1 - Renombre, lógica pura AS5600 y pruebas

Leer del diseño: secciones 1, 2 (solo la lista de registros), 4.1, 4.2, 4.6 (solo los puntos de SimEffects/tests, Build-Pico2, probar.ps1), 5 y 6.
1. Línea base: `probar.ps1 -Todo` antes de editar.
2. Renombre `YOKE_USE_SIMULATION` → `YOKE_POSITION_SOURCE` en `mis_defines.h`, `MiYoke_PD.ino`, `CalibrationControls.h`, `SimEffects.h`, `tests/simconnect_checks.cpp`, `tests/sim_position_friction_checks.cpp`, `herramientas/compilacion/Build-Pico2.ps1` (solo renombre en .ino y CalibrationControls en esta parte).
3. `mis_defines.h`: bloque de 4.1 completo (bandera nueva con `#error` de renombre, bloque AS5600 y `static_assert`).
4. Nuevo `As5600Position.h` (4.2) y `tests/as5600_checks.h` con los 16 casos de la sección 5, incluido y llamado en `tests/simconnect_checks.cpp`.
5. Agregar el switch `-As5600` a `Build-Pico2.ps1` y `probar.ps1` (4.6), pero **no ejecutarlo en esta parte**: el modo 3 no compila hasta que la Parte 2 agregue el driver (es lo esperado). `-Todo` todavía no debe incluir `-As5600` en esta parte; la Parte 2 lo agrega.
6. Pruebas: `probar.ps1 -Todo` OK. Comprobación de renombre de la sección 5: solo las líneas `#ifdef`/`#error` de `mis_defines.h`.
7. Commit por nombre: `Renombra YOKE_USE_SIMULATION a YOKE_POSITION_SOURCE y agrega logica pura AS5600 con pruebas`. Si se tocó `MiYoke_PD.ino`: `herramientas/flujo/graphify-sync.cmd`.
8. No ejecutar `nueva-version.ps1`. Completar "Última entrega" (Parte 1): commit, líneas OK/FALLO base y final, desvíos.

\### Parte 2 - Driver I2C, integración en el firmware, documentación y versión

Leer del diseño: secciones 3, 4.3, 4.4, 4.5, 4.6 (LoopSupervisor, CalibrationControls, health_checks), 5 (compilaciones y ensayo), 6, 8 y 9. Precondición: Parte 1 commiteada y verde.
1. Nuevo `YokeI2C.h` y cambio en `LcdTelemetry.h` (4.3; la rama `WIRE_HAS_TIMEOUT` queda como está, solo cambia el literal).
2. Nuevo `AxisAs5600.h` (4.4).
3. `MiYoke_PD.ino` (4.5), `LoopSupervisor.h`, `CalibrationControls.h`, `tests/health_checks.h` (4.6). Incluir `-As5600` en `-Todo` de `probar.ps1`.
4. Pruebas: `probar.ps1 -Todo -As5600` OK; informar programa/RAM del build AS5600 frente al configurado. Comprobación de renombre (sección 5) sin resultados fuera de `mis_defines.h`.
5. Documentación (sección 8, sin `tesis/`): nuevo `LEEME md/LEEME_AS5600.md` (con la sección 9 y el ensayo con hardware de la sección 5) y los LEEME/INVENTARIO/AGENTS/CLAUDE/SKILL listados, solo las partes afectadas.
6. Commit por nombre (código + documentación): `Agrega fuente de posicion 3: encoders magneticos AS5600 por I2C`. Luego `herramientas/flujo/graphify-sync.cmd`.
7. Versión: `powershell -ExecutionPolicy Bypass -File herramientas/flujo/nueva-version.ps1 -Mensaje "Agrega fuente de posicion 3 (encoders AS5600 I2C) y renombra YOKE_USE_SIMULATION a YOKE_POSITION_SOURCE"`; verificar "Version Vx.y creada" y el ACTUALIZACION.
8. Completar "Última entrega" (Parte 2): commits, líneas OK/FALLO, tamaños, versión creada, y lo que el usuario debe ensayar cuando tenga los sensores.

\## Última entrega

- Parte 2 completada (Codex, 2026-10-22), sin bloqueos de implementacion. Parte 1 OK segun el usuario; revision de Parte 2 pendiente de Claude.
- Codigo: nuevos `YokeI2C.h` y `AxisAs5600.h`; modificados `LcdTelemetry.h`, `MiYoke_PD.ino`, `LoopSupervisor.h`, `CalibrationControls.h`, `tests/health_checks.h`, `herramientas/compilacion/probar.ps1` y `.gitignore`.
- Implementado segun diseno: buses compartido/propio, CONF volatil verificada y comprobada cada segundo alternando ejes, lectura de STATUS/RAW y AGC, estado serie, etapa SENSOR_I2C=16, excusa del hueco de flash, fuerza cero si cualquier eje no esta sano y reinicio del estimador de velocidad. Constantes de seguridad sin cambios. `-Todo` incluye AS5600.
- Documentacion: nuevo `LEEME md/LEEME_AS5600.md`; actualizados `AJUSTE_FUERZAS_Y_MOVIMIENTO.md`, `LEEME_POTENCIOMETROS.md`, `LEEME_SIMCONNECT.md`, `MENU_CALIBRACION.md`, `MOBIFLIGHT_PICO_Y_UNO.md`, `LEEME_PICO2.md`, `LEEME_ENCODERS.md`, `CENTRADO_Y_RODAJE.md` e `INVENTARIO_CODIGO.md` (todos en `LEEME md/`), mas switches de `AGENTS.md`, `CLAUDE.md` y `.claude/skills/verificacion-antes-de-terminar/SKILL.md`.
- Tesis pendiente de Claude al aprobar: `tesis/capitulos/03_centrado.tex`, `09_damper.tex`, `10_friccion.tex` y `tesis/fundamento_efectos/` segun seccion 8 del diseno. No se abrio ni modifico tesis/; sus cambios previos y archivos sin seguimiento se conservan.
- Pruebas: `powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1 -Todo -As5600`, salida 0:
  - OK suite simconnect_checks - PASS: 143769 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
  - OK comparar_ensayo
  - OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
  - OK Pico 2 (configurado) - Sketch uses 105404 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22920 bytes (4%) of dynamic memory, leaving 501368 bytes for local variables. Maximum is 524288 bytes.
  - OK Pico 2 (encoders) - Sketch uses 106556 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23256 bytes (4%) of dynamic memory, leaving 501032 bytes for local variables. Maximum is 524288 bytes.
  - OK Pico 2 (potenciometros) - Sketch uses 106388 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23016 bytes (4%) of dynamic memory, leaving 501272 bytes for local variables. Maximum is 524288 bytes.
  - OK Pico 2 (AS5600) - Sketch uses 106940 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23084 bytes (4%) of dynamic memory, leaving 501204 bytes for local variables. Maximum is 524288 bytes.
  - OK puente SimConnect
- AS5600 frente a configurado: +1536 bytes de programa y +164 bytes de RAM. Sin advertencias nuevas de compilacion.
- Renombre: fuera de tesis/ y documentos DISENO_*, solo quedan las dos lineas de compatibilidad de `mis_defines.h` y las menciones literales del Plan para Codex en este HANDOFF (pasos/commit/mensaje historicos); ningun otro archivo conserva la bandera anterior.
- `git diff --check`: sin errores. `.gitignore` requirio agregar `build-pico2-as5600/` porque enumeraba builds concretos; `git check-ignore` confirmado. Sin cambios fuera del alcance.
- Grafo: `herramientas/flujo/graphify-sync.cmd` y `graphify update .` ejecutados; copia .ino verificada por SHA256. La copia inicial dio acceso denegado en sandbox; la sincronizacion con permisos ampliados lo resolvio.
- Commit de implementacion: `1b29d0e` (`Agrega fuente de posicion 3: encoders magneticos AS5600 por I2C`). El cierre de este HANDOFF se commitea aparte, solo documentacion.
- Version: V2.6 creada por `nueva-version.ps1`, salida "Version V2.6 creada"; verificados etiqueta sobre `1b29d0e` y `../MiYoke_PD_VERSIONES/V2_6/ACTUALIZACION_V2_6.txt` (rama mejora/encoder-as5600). Se ejecuto en un worktree temporal limpio del mismo commit y rama, junto al repositorio, para conservar los cambios previos de tesis sin tocarlos. Worktree temporal retirado tras verificar la version.
- Ensayo pendiente del usuario cuando tenga sensores: s=OK y g cerca de 64 en ambos ejes; a= suave, sentido/magnitud correctos en todo el recorrido; desconexion SDA corta fuerza, reconexion recupera con centro absoluto o BLQ hasta reiniciar con centro al encender y hueco >40 ms. Comparar loopWorstUs/ciclos lentos con modo 2 sin aumento. Detalle en `LEEME md/LEEME_AS5600.md`. No se cargo firmware ni se hizo ensayo con hardware.

\## Revisión

- Parte 1: OK informado por el usuario (2026-10-22). Parte 2: pendiente de Claude.

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
