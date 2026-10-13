\# HANDOFF entre agentes



\## Reglas

1\. Un solo agente edita a la vez. Cada tarea va en su propia rama: `git checkout -b tarea/<nombre>`.

2\. Al terminar, el agente completa la sección "Última entrega" y hace commit.

3\. El otro agente revisa el diff (`git diff master...HEAD`) sin editar, y anota hallazgos en "Revisión".

4\. Nada se integra a `master` sin que el usuario valide en hardware.

5\. Antes de afirmar algo del código, indicar la fuente: \[Grafo], \[INO] o \[Código], con archivo y línea. Las aristas INFERRED se tratan como hipótesis.

6\. Tras editar `MiYoke\_PD.ino`, ejecutar `graphify-sync.cmd`.

7\. No modificar constantes de seguridad (límites de PWM, `MOTOR\_PWM\_HARD\_LIMIT`, `SIMCONNECT\_EXTRA\_LIMIT`) sin aprobación explícita del usuario.

8\. Nunca usar `git add .` ni `git add -A`: la carpeta graphify-ino/ no debe versionarse. Agregar archivos por nombre.

9\. Un hallazgo de "posible bug" se verifica leyendo el código involucrado antes de proponer cambios. Informar la evidencia (archivo y línea), no solo la sospecha.

10\. Las reglas de versionado, documentacion, ahorro de tokens y pruebas estan en AGENTS.md (Codex) y CLAUDE.md (Claude); no se repiten aca.

11\. "Ultima entrega" y "Revision" contienen solo la tarea en curso: al empezar una tarea nueva se reemplazan (lo anterior queda en el historial de git).

\## Plan para Codex

Tarea: registro CSV de lo que calcula la Pico 2, banco sin simulador y rutina de ensayo (Claude, 2026-10-12).
Rama: `mejora/registro-ensayo-pico` (ya creada; contiene el diseño). Diseño: `LEEME md/DISENO_registro_y_ensayo_pico.md`.
Se divide en **3 partes** (una sesión de Codex por parte): toca firmware con estado, puente, una herramienta nueva, scripts y documentación; entre la Parte 1 y la 2 el usuario carga el UF2 y comprueba que salen los registros.

\### Precondiciones (todas las partes)
- `git checkout mejora/registro-ensayo-pico` y `git status`: ignorar `graphify-ino/`, `tesis/main.tex`, `tesis/capitulos/11_maquinas_estado.tex` y `tesis/caracterizacion_motores_775.tex` (cambios del usuario: no tocarlos ni commitearlos).
- Orientarse con `graphify query/explain` antes de abrir código; leer solo las líneas indicadas.
- No tocar constantes de seguridad (regla 7). No usar `git add .` ni `git add -A`.
- Pruebas solo con `powershell -ExecutionPolicy Bypass -File probar.ps1 [...]`; en HANDOFF copiar solo las líneas OK/FALLO.
- Si algo del diseño contradice el código, detenerse y anotarlo en "Última entrega"; no improvisar.

\### Parte 1 — Firmware + `pico.csv`/`ganancias.csv` en el puente (sin versión)
- Leer del diseño: §1.2, §2 completo, §3.1, §3.2 puntos 1 a 4, §7 casos 1-6 y 9, §9.
- Código a leer: `MiYoke_PD.ino:290-340`, `SimEffects.h:45-71` y `:130-205`, `GainSettings.h:30-45`, `SimTelemetry.h:30-52` y `:114-117`, `mis_defines.h:215-230`, `SimConnectBridge/SerialLink.h:50-67`, `SimConnectBridge/main.cpp` (completo, 270 líneas), `tests/simconnect_checks.cpp:50-62`.
- Alcance:
  1. Crear `PicoLog.h` (§2.1), puro e `inline`.
  2. `SimEffects.h`: accesores y `skipCount` de §2.2. `SimTelemetry.h`: accesor `isPaused()` de §2.2b (autorizado; solo eso).
  3. `mis_defines.h`: `YOKE_MODEL_LOG 1` (§2.4).
  4. `MiYoke_PD.ino`: §2.3 puntos 1 a 7.
  5. Crear `SimConnectBridge/PicoCsv.h` (§3.1) y aplicar §3.2 puntos 1 a 4 en `SimConnectBridge/main.cpp` (incluye `PollPort`).
  6. Crear `tests/picolog_checks.h` con los casos 1-6 y 9 de §7 e incluirlo desde `tests/simconnect_checks.cpp`.
  7. `graphify-sync.cmd` (se editó el `.ino`) y `graphify update .`.
- Pruebas: `probar.ps1 -Pico -Potentiometers -Encoders -Bridge`. Todo OK. Anotar el tamaño del programa frente a V1.8 (103148 B) y compilar una vez con `YOKE_MODEL_LOG 0` (solo verificar y volver a 1).
- Commit por nombre: `PicoLog.h SimEffects.h SimTelemetry.h mis_defines.h MiYoke_PD.ino SimConnectBridge/PicoCsv.h SimConnectBridge/main.cpp tests/picolog_checks.h tests/simconnect_checks.cpp HANDOFF.md`.
  Mensaje: "Pico y puente: registro pico.csv y ganancias.csv con lo que calcula la Pico".
- **No** ejecutar `nueva-version.ps1`.
- "Última entrega": hecho, líneas OK/FALLO, tamaño, desvíos. Avisar al usuario que cargue el UF2 de `build-pico2-potentiometers` (o el que use) y haga un vuelo corto de 2 min (§8 punto 3), y que pegue en HANDOFF la línea `Fin:` de `bridge.log` y las 3 primeras filas de `pico.csv`.

\### Parte 2 — `--replay` + comparador (sin versión)
- Precondición: Parte 1 commiteada y el usuario anotó en HANDOFF el resultado del vuelo corto (si no, detenerse).
- Leer del diseño: §3.2 punto 5, §4 completo, §7 casos 7-8.
- Código a leer: `SimConnectBridge/main.cpp` (completo), `tests/raw_checks.h:1-20`, `SimTelemetry.h:30-110`, `PicoForceModels.h`, `tests/simconnect_checks.vcxproj`, `probar.ps1:50-65`.
- Alcance:
  1. Crear `SimConnectBridge/Replay.h` y agregar `--replay` en `SimConnectBridge/main.cpp` (§3.2.5); enlazar `winmm.lib` en `SimConnectBridge/YokeSimConnectBridge.vcxproj`.
     `parseRow` acepta `nan`, `-nan(ind)`, `inf` y `-inf` (§3.2.5); agregar a `tests/picolog_checks.h` un caso con `spoiler_left=-nan(ind)` que se lea como NaN y uno con `abc` que se rechace.
  2. Crear `tests/comparar_ensayo.h` (lógica), `tests/comparar_ensayo.cpp` (main) y `tests/comparar_ensayo.vcxproj` (§4, §7).
  3. Casos 7-8 de §7 en `tests/picolog_checks.h`.
  4. `probar.ps1`: compilar también `tests\comparar_ensayo.vcxproj` en el paso 1 y reportar `OK comparar_ensayo`.
  5. Ajustes menores de la revisión de la Parte 1b (firmware):
     - `MiYoke_PD.ino:339`: sincronizar también `lastSkip` en la sincronización por dtr: `if(!logSynced){ lastLoggedRev=r; lastSkip=simEffects.skipped(); logSynced=true; }`.
     - `tests/picolog_checks.h` (prueba H3): dar a la trama 3 una presión distinta de la trama 2, para detectar un error que modele la muestra salteada en lugar de la última.
     - Agregar `MiYoke_PD.ino` al commit de la Parte 2, ejecutar `graphify-sync.cmd` y compilar también con `-Pico -Potentiometers -Encoders`.
  6. `graphify update .`.
- Pruebas: `probar.ps1 -Bridge`. Todo OK. Si existe `SimConnectBridge\bin\Win32\RealismoSC8\telemetry.csv` del vuelo corto, copiar esa carpeta de registros a `tmp\vuelo_corto\` y ejecutar `tests\bin\comparar_ensayo.exe tmp\vuelo_corto`; anotar solo las líneas `A`, `B1..B9` y `RESULTADO`.
- Commit por nombre: `SimConnectBridge/Replay.h SimConnectBridge/main.cpp SimConnectBridge/YokeSimConnectBridge.vcxproj tests/comparar_ensayo.h tests/comparar_ensayo.cpp tests/comparar_ensayo.vcxproj tests/picolog_checks.h probar.ps1 HANDOFF.md`.
  Mensaje: "Puente: modo replay sin simulador y comparador de ensayos contra el modelo de la Pico".
- **No** ejecutar `nueva-version.ps1`.

\### Parte 3 — Scripts + documentación + versión
- Leer del diseño: §5, §6, §8, §10.
- Código a leer: `Iniciar-Simulador.ps1:1-20` y `:130-145`, `.gitignore`.
- Alcance:
  1. Crear `ENSAYO.ps1`, `ENSAYO.cmd` (igual estilo que `PROBAR REALISMO.cmd`) y `BANCO.ps1` (§5), con la tabla de fases de §6 dentro de `ENSAYO.ps1`.
  2. Agregar `ensayos/` a `.gitignore`.
  3. Documentación: aplicar los reemplazos exactos de §10 (buscar el TEXTO ACTUAL literal; si no aparece, anotarlo y seguir) y crear `LEEME md/ENSAYOS.md`.
  4. Banco en monitor (sin armar): con la Pico conectada, Prepar3D cerrado y la entrada del vuelo corto: `BANCO.ps1 -Entrada tmp\vuelo_corto\telemetry.csv`. Anotar sus líneas `B` y `RESULTADO` (A dará `SIN DATOS`). **No usar `-Armar`** salvo que HANDOFF tenga la confirmación del usuario "fuente de motores apagada" con fecha de ese día; en ese caso, `-Armar -Confirmado`.
  5. `graphify update .`.
- Pruebas: `probar.ps1 -Pico -Potentiometers -Encoders -Bridge` (todo OK). La tesis no se toca en esta tarea.
- Commit por nombre: `ENSAYO.ps1 ENSAYO.cmd BANCO.ps1 .gitignore "LEEME md/CALCULO_PICO_SC8.md" "LEEME md/LEEME_SIMCONNECT.md" "LEEME md/ENSAYOS.md" HANDOFF.md`.
  Mensaje: "Ensayos: rutina de vuelo, banco sin simulador y documentacion de registros de la Pico".
- Versión: `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Registro pico.csv de lo que calcula la Pico, modo replay, comparador y rutina de ensayo"`. Verificar "Version Vx.y creada" y `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt`.
- "Última entrega": hecho, líneas OK/FALLO, resultado del banco, versión creada, desvíos.
- Luego: revisión de Claude, banco armado por el usuario y vuelo con `ENSAYO.cmd`. No integrar a `master`.

\## Última entrega

- Parte 2 implementada (2026-10-12), rama `mejora/registro-ensayo-pico`, sobre `48cc38e`. Precondiciones cumplidas: Parte 1b aprobada y vuelo corto registrado por Claude.
- Replay: lector puro de 34 campos y cabecera exacta, con nan / -nan(ind) / inf / -inf sin distinguir mayusculas; rechazo de texto no numerico. Puente --replay con --port, ajustes desde cada fila, solo sent=1, reloj original sample_time_ms y programacion por host_time_ms, PollPort, salida por Ctrl+C y tres Send(false) separados 50 ms. Sin SimConnect_Open en replay; proteccion de entrada contra telemetry.csv del EXE, rechazo de opciones incompatibles y enlace winmm.lib.
- Comparador: receptor SC8 y modelos compartidos con firmware, emparejamiento por sample_time_ms y deduplicacion por revision, exclusiones por skip y tolerancias A, B1-B9, datos C, fases automaticas y marcas opcionales. Lee M de pico.log si falta pico.csv (B6 SIN DATOS). Escribe informe.txt y resumen_fases.csv.
- Pruebas nuevas: casos 7-8, lectura de -nan(ind) y abc, ensayo sintetico OK, desvio de ground de 0.01 detectado, exclusion de tramo con skip sin penalizar A, monitor sin filas armadas da SIN DATOS.
- Ajustes menores de Claude aplicados: sincronizacion de lastSkip junto a lastLoggedRev en dtr ([INO] `MiYoke_PD.ino:339`); trama 3 de H3 con presion 150 frente a 100 en trama 2 ([Codigo] `tests/picolog_checks.h:149`).
- Archivos modificados: `SimConnectBridge/Replay.h`, `SimConnectBridge/main.cpp`, `SimConnectBridge/YokeSimConnectBridge.vcxproj`, `tests/comparar_ensayo.h`, `tests/comparar_ensayo.cpp`, `tests/comparar_ensayo.vcxproj`, `tests/picolog_checks.h`, `probar.ps1`, `MiYoke_PD.ino`, `HANDOFF.md`.
- Documentacion actualizada: este HANDOFF; LEEME quedan para Parte 3. No se toco tesis ni constantes de seguridad. Cambios previos del usuario preservados.
- Verificacion: corrida `probar.ps1 -Pico -Potentiometers -Encoders -Bridge` y corrida final `probar.ps1` (suite/comparador/lanzador). La primera termino codigo 1 porque al iniciar faltaba el include directo de mis_defines.h en el comparador standalone; se corrigio, y la corrida final sin flags termino codigo 0. No se repitieron compilaciones de firmware/puente que ya dieron OK y cuyo codigo no cambio. Resultados finales:
  - OK suite simconnect_checks - PASS: 141283 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
  - OK comparar_ensayo
  - OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
  - OK Pico 2 (configurado) - Sketch uses 104500 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22768 bytes (4%) of dynamic memory, leaving 501520 bytes for local variables. Maximum is 524288 bytes.
  - OK Pico 2 (encoders) - Sketch uses 104676 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23008 bytes (4%) of dynamic memory, leaving 501280 bytes for local variables. Maximum is 524288 bytes.
  - OK Pico 2 (potenciometros) - Sketch uses 104500 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22768 bytes (4%) of dynamic memory, leaving 501520 bytes for local variables. Maximum is 524288 bytes.
  - OK puente SimConnect
- Registros actuales del vuelo corto copiados a `tmp/vuelo_corto/`: telemetry.csv, pico.csv, ganancias.csv, pico.log y bridge.log. Ejecutado comparador final sobre esa carpeta (codigo 1); informes generados alli:
  - A fidelidad: OK emparejadas=100.000% q=100.000% gust=100.000% buffet=100.000% ground=100.000% env=100.000% peor=q@91235750:0.000050 excluidas=0 esperadas=4376
  - B1: OK violaciones=0
  - B2: OK violaciones=0
  - B3: OK violaciones=0
  - B4: OK violaciones=0
  - B5: OK violaciones=0
  - B6: FALLO violaciones=1 comprobaciones=4
  - B7: OK violaciones=0 pico=0.000% puente=0.000% dropped=0.000
  - B8: OK violaciones=0
  - B9: OK violaciones=0 lineas=4533 malas=0
  - RESULTADO: FALLO
- Dato para revision de Claude: B6 se aplica literalmente como §4.2. La pausa inicial de telemetry.csv es host 91043093 (sample 0, nunca armado aun); despues hay armado en 91043187. La primera fila pico con host >=91043243 es 91043265, flags 179 y cmd -0.0007 / 0.0017, por eso B6 falla. Pico ya habia emitido cmd=0 en 91043109, pero se rearmo antes de +150 ms. Las otras tres transiciones comprobadas tienen cmd=0. No se cambio el criterio B6 ni se oculto el resultado; Claude debe evaluar el tratamiento del arranque/rearmado antes del umbral.
- Ejecutados graphify-sync.cmd y graphify update . despues de los cambios correspondientes; git diff --check sin errores. No se ejecuto replay hacia hardware; el banco corresponde a Parte 3.
- Commit por nombre, mensaje: `Puente: modo replay sin simulador y comparador de ensayos contra el modelo de la Pico`. Sin version, segun el plan; la version queda para Parte 3. No integrado a master.
- Sigue: revision de Claude de Parte 2 y evaluacion del resultado B6; luego Parte 3 (scripts, documentacion y version).

\## Revisión

- Revisor: Claude (2026-10-12), con Code Reviewer. **Parte 1 (`63c5715`): REQUIERE CAMBIOS** (uno bloqueante, originado en el diseño, no en la implementación).
  - **Fidelidad al diseño:** correcta. Los 26 campos de `M` tienen el orden y la escala pedidos (`PicoLog.h:21-29`), con recorte antes de `lroundf` y `NonFinite`. `Menu` está bajo `YOKE_CALIBRATION_ENABLED`. Una sola `M`/`G` por ciclo, `availableForWrite` va antes de formatear y dentro de `UsbLock`, y `mark(SerialTx)` está antes de cada write. `skipCount` y los accesores también están bien. En el puente: validación estricta de 26/14 enteros, cabeceras, rotación de 5 archivos, buffer de 4096, `PollPort` equivalente al bucle anterior, flush y cierre en todas las salidas, contadores en `Fin:`. No cambia ninguna fuerza ni constante de seguridad.
  - **H1 (bloqueante): `drop` falso tras pausa, desarme, slew o reinicio por `dt>50`.**
    - Con el modelo reiniciado, `modeledRevision()` vale 0, pero `rx.revision` sigue subiendo con cada trama de pausa/desarme (`SimTelemetry.h:75,103`). Como `lastLoggedRev` queda con la última revisión modelada (`MiYoke_PD.ino:337-346`), al volver a modelar `d` es grande y `drop` suma cientos de pérdidas que no existieron. B7 daría FALLO en cualquier ensayo con pausa.
    - Corrección (diseño §2.3 punto 4, ya actualizado): `if(r==0) lastLoggedRev=0;` cada ciclo, justo después de leer `r`.
  - **H2 (bloqueante, mismo bloque): `drop` cuenta dos veces las muestras no modeladas.** Si la Pico salteó una revisión (`skip`), `d` también salta y se suma a `drop`. Corrección (§2.3 punto 4): restar el aumento de `skipped()` desde la última línea escrita (`lastSkip`), sin bajar de 0, con la función pura `YokeLog::nextDrop(...)` en `PicoLog.h`.
  - **H3 (menor): prueba débil en el caso 9.** `tests/picolog_checks.h:70`: `command()==0` sobre pista lisa a velocidad 0 da 0 haya salto o no. Reemplazar por dos `Effects` alimentados con las mismas tramas, uno con salto y otro sin salto, y comparar `command()` en un caso con salida ≠ 0 (centrado con posición ≠ 0 y presión > 0): deben coincidir en la muestra común.
  - **H4 (menor): caso 1 incompleto.** `tests/picolog_checks.h:15-17`: verificar también `values[0]==65535`, `values[1]==values[2]==4294967295`, `values[20]==255`, `values[21]==-255` y `values[24]==values[25]==65535`.
  - **Para Codex: Parte 1b**, en la misma rama y antes de la Parte 2:
    1. `PicoLog.h`: `nextDrop` según §2.3 punto 4.
    2. `MiYoke_PD.ino`: `if(r==0) lastLoggedRev=0;` después de `const uint32_t r=...`; usar `YokeLog::nextDrop`; `static uint16_t lastSkip`, que se actualiza al escribir `M`.
    3. `tests/picolog_checks.h`:
       - casos de `nextDrop`: secuencia r=5,0,0,9 → `drop` 0; r=5→8 sin skip → +2; r=5→8 con skip +1 → +1; con skip mayor que el salto → +0; saturación en 65535;
       - además H3 y H4.
    4. Pruebas: `probar.ps1 -Pico -Potentiometers -Encoders` (copiar solo las líneas OK/FALLO).
    5. Commit por nombre `PicoLog.h MiYoke_PD.ino tests/picolog_checks.h HANDOFF.md`, con el mensaje "Pico: contador de lineas perdidas sin falsos tras pausa ni doble conteo de muestras salteadas".
    6. `graphify-sync.cmd` y `graphify update .`. Sin versión.
    7. Completar "Última entrega".
  - El usuario **no** carga el UF2 hasta que Claude apruebe la Parte 1b.

- Revisor: Claude (2026-10-12), con Code Reviewer. **Parte 1b (`fd46a9d`): APROBADA.**
  - H1-H4 resueltos:
    - `YokeLog::nextDrop` (`PicoLog.h:15-24`) cumple §2.3 punto 4.
    - `if(r==0) lastLoggedRev=0;` evita falsos tras pausa/desarme.
    - La prueba H3 compara `command()` con y sin salto (salida ≠ 0); H4 completa.
    - Suite (141150 comprobaciones) y las 3 variantes de firmware OK según Codex; no se recompiló porque el código no cambió.
  - Observaciones menores (no bloquean; pasan a la Parte 2, punto 5):
    - tras reconectar, `lastSkip` no se sincroniza y puede ocultar alguna pérdida real (nunca crea falsos);
    - la prueba H3 usa la misma presión en las 3 tramas.
  - Limitación conocida: con `skip` saturado en 65535, las muestras salteadas nuevas se cuentan en `drop`.
  - **Usuario:** ya puede cargar `build-pico2-potentiometers/MiYoke_PD.ino.uf2` (12/10 20:44) y hacer el vuelo corto de 2 min, con la fuente de motores apagada.

\## Ensayos del usuario

- Vuelo corto (usuario, 2026-10-12 20:52; firmware `fd46a9d`, fuente de motores apagada). Análisis de Claude con Test Results Analyzer, sobre los registros de `SimConnectBridge/bin/Win32/RealismoSC8/`:
  - **Datos del vuelo:**
    - 225 s; rodaje, despegue a 70,8 s y vuelo hasta 156 kt; 2 pausas, la segunda al cerrar.
    - Bridge `Fin: muestras=21395 error=1 reconexiones=0 lineas_pico=4533 lineas_pico_malas=0`. `error=1` se debe al `ERROR Dispatch 0xC00000B0` (pipe desconectado) al cerrar el simulador, después de la pausa y de `running=0`: es el cierre normal.
  - **pico.csv:**
    - 4487 filas, de las cuales 4382 están modeladas;
    - `dropped=0`, `skipped=0`, sin saltos de `seq` ni NonFinite;
    - unas 20 filas/s;
    - todas las filas modeladas tienen su `sample_time_ms` en `telemetry.csv`.
  - **ganancias.csv:** 46 filas, sin cambios.
  - **Invariantes:**
    - B1-B5 sin violaciones; B6: comando a 0 a los 47 ms y a los 62 ms de cada pausa; `dropped` sigue en 0 al reanudar.
    - Presión: 100 % dentro de 2e-4 (peor diferencia 5,2e-5).
    - Suelo solo en rodaje (envolvente hasta 1,0; peak 23/4 PWM roll/pitch).
    - buffet_stall hasta 0,68 solo en vuelo con AoA de 11,7 a 18,2° y 45-53 kt; buffet de tren con el tren extendido; ráfagas 0 (sin viento).
  - **Tiempos (SC8):** us=286, lp=2412, gap=2429 µs (máximos acumulados de todo el vuelo). El vuelo V1.8 del 11/10 tenía us=186 y lp=38948, en condiciones distintas, así que el criterio de "+100 µs" no se puede atribuir: el registro M/G no está dentro de `Effects::update` (`us`). `lp` > 2000 µs una vez: queda como dato a seguir en el banco.
  - **Observaciones:**
    - `spoiler_left/right` = `-nan(ind)` en casi todo `telemetry.csv`. Es esperado (C182 sin spoilers; `RawTelemetry.h:48` lo admite y la Pico da buffet_spoiler=0), pero el lector de la Parte 2 tiene que aceptarlo: se agregó a §3.2.5 y a la Parte 2, punto 1.
    - Corte de armado de 0,3 s a los 25 s, en tierra y detenido, sin pausa: la Pico respondió con cmd=0. Compatible con el desarme del puente cuando no llegan datos por más de 250 ms (`main.cpp:240`), por ejemplo durante una carga del simulador.
  - **Conclusión:** los registros nuevos funcionan. Se puede hacer la Parte 2. Los registros de este vuelo sirven como entrada del comparador y del banco: copiarlos a `tmp\vuelo_corto\` (lo indica la Parte 2).

- (el usuario o Codex anotan acá: resultado del vuelo corto, confirmación "fuente de motores apagada" con fecha, y la carpeta de cada ensayo en `ensayos\`)
