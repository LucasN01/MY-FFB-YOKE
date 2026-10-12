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
  2. Crear `tests/comparar_ensayo.h` (lógica), `tests/comparar_ensayo.cpp` (main) y `tests/comparar_ensayo.vcxproj` (§4, §7).
  3. Casos 7-8 de §7 en `tests/picolog_checks.h`.
  4. `probar.ps1`: compilar también `tests\comparar_ensayo.vcxproj` en el paso 1 y reportar `OK comparar_ensayo`.
  5. `graphify update .`.
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

- Parte 1b completada (2026-10-12) en `mejora/registro-ensayo-pico`, sobre `af146fd`. Hallazgos H1-H4 aplicados; pendiente de revision de Claude.
- H1 aplicado: cada ciclo con revision modelada 0 reinicia `lastLoggedRev` ([INO] `MiYoke_PD.ino:338`), para no contar revisiones sin modelo tras pausa, desarme, slew o dt>50.
- H2 aplicado: funcion pura inline `YokeLog::nextDrop` ([Codigo] `PicoLog.h:15`), resta el aumento de skip sin bajar de cero y satura en 65535. El firmware usa la funcion y actualiza `lastSkip` al escribir M ([INO] `MiYoke_PD.ino:344,369`).
- H3 aplicado: dos Effects con las mismas tramas y los mismos ticks de control, uno modela las muestras en secuencia y otro saltea la intermedia. En la muestra comun, se comprueba igualdad de command con centrado, posicion .5 y presion >0, y salida distinta de cero ([Codigo] `tests/picolog_checks.h:81-101`).
- H4 aplicado: caso de formato verifica tambien seq, t/rev de 32 bits, PWM positivo/negativo y drop/skip en 65535 ([Codigo] `tests/picolog_checks.h:31-33`).
- Pruebas de nextDrop: 5,0,0,9 sin perdidas; 5->8 suma 2 sin skip y 1 con skip +1; skip mayor que el salto suma 0; saturacion y revision repetida.
- Archivos modificados: `PicoLog.h`, `MiYoke_PD.ino`, `tests/picolog_checks.h`, `HANDOFF.md`. Documentacion actualizada: este HANDOFF; el diseno §2.3 ya fue corregido por Claude. Tesis y cambios previos del usuario preservados y fuera del commit.
- Verificacion final: `powershell -ExecutionPolicy Bypass -File probar.ps1 -Pico -Potentiometers -Encoders` (codigo 0):
  - OK suite simconnect_checks - PASS: 141150 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
  - OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
  - OK Pico 2 (configurado) - Sketch uses 104492 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22768 bytes (4%) of dynamic memory, leaving 501520 bytes for local variables. Maximum is 524288 bytes.
  - OK Pico 2 (encoders) - Sketch uses 104668 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23008 bytes (4%) of dynamic memory, leaving 501280 bytes for local variables. Maximum is 524288 bytes.
  - OK Pico 2 (potenciometros) - Sketch uses 104492 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22768 bytes (4%) of dynamic memory, leaving 501520 bytes for local variables. Maximum is 524288 bytes.
- Tamano frente a Parte 1: +24 B configurado/potenciometros, +48 B encoders. Ejecutados graphify-sync.cmd y graphify update .; git diff --check sin errores.
- Commit por nombre, mensaje: `Pico: contador de lineas perdidas sin falsos tras pausa ni doble conteo de muestras salteadas`. Sin version, segun Parte 1b; la version queda para Parte 3. Sin desvios pendientes ni cambios en fuerzas/constantes de seguridad.
- Sigue: revision de Claude de Parte 1b. El usuario NO carga el UF2 hasta que Claude apruebe. Despues: vuelo corto de 2 min y registrar en Ensayos del usuario la linea Fin: de bridge.log y las 3 primeras filas de pico.csv antes de Parte 2.

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

\## Ensayos del usuario

- (el usuario o Codex anotan acá: resultado del vuelo corto, confirmación "fuente de motores apagada" con fecha, y la carpeta de cada ensayo en `ensayos\`)
