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
- Código a leer también: `tests/comparar_ensayo.h` (solo las funciones de B6, B7, skip, reset y fases) y `SimConnectBridge/Replay.h:30-40`, `SimConnectBridge/main.cpp:315-330`.
- Alcance:
  0. Correcciones de la revisión de la Parte 2 (sección "Revisión", hallazgos R1 a R7), con sus pruebas en `tests/picolog_checks.h`.
  1. Crear `ENSAYO.ps1`, `ENSAYO.cmd` (igual estilo que `PROBAR REALISMO.cmd`) y `BANCO.ps1` (§5), con la tabla de fases de §6 dentro de `ENSAYO.ps1`.
  2. Agregar `ensayos/` a `.gitignore`.
  3. Documentación: aplicar los reemplazos exactos de §10 (buscar el TEXTO ACTUAL literal; si no aparece, anotarlo y seguir) y crear `LEEME md/ENSAYOS.md`.
  4. Banco en monitor (sin armar): con la Pico conectada, Prepar3D cerrado y la entrada del vuelo corto: `BANCO.ps1 -Entrada tmp\vuelo_corto\telemetry.csv`. Anotar sus líneas `B` y `RESULTADO` (A dará `SIN DATOS`). **No usar `-Armar`** salvo que HANDOFF tenga la confirmación del usuario "fuente de motores apagada" con fecha de ese día; en ese caso, `-Armar -Confirmado`.
  5. `graphify update .`.
- Pruebas: `probar.ps1 -Pico -Potentiometers -Encoders -Bridge` (todo OK). La tesis no se toca en esta tarea.
- Antes del banco, volver a ejecutar `tests\bin\comparar_ensayo.exe tmp\vuelo_corto` y anotar las líneas `A`, `B1..B9` y `RESULTADO`: tiene que dar `RESULTADO: OK`.
- Commit por nombre: `tests/comparar_ensayo.h tests/picolog_checks.h SimConnectBridge/Replay.h SimConnectBridge/main.cpp ENSAYO.ps1 ENSAYO.cmd BANCO.ps1 .gitignore "LEEME md/CALCULO_PICO_SC8.md" "LEEME md/LEEME_SIMCONNECT.md" "LEEME md/ENSAYOS.md" HANDOFF.md`.
  Mensaje: "Ensayos: rutina de vuelo, banco sin simulador y documentacion de registros de la Pico".
- Versión: `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Registro pico.csv de lo que calcula la Pico, modo replay, comparador y rutina de ensayo"`. Verificar "Version Vx.y creada" y `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt`.
- "Última entrega": hecho, líneas OK/FALLO, resultado del banco, versión creada, desvíos.
- Luego: revisión de Claude, banco armado por el usuario y vuelo con `ENSAYO.cmd`. No integrar a `master`.

\## Última entrega

- Parte 3b implementada (2026-10-12), rama `mejora/registro-ensayo-pico`, sobre `0886956`. Solo S1–S6 de la revision.
- S1 aplicado: `ENSAYO.cmd` hace pause siempre, para mantener visible el informe con doble clic.
- S2 aplicado: `ENSAYO.ps1` ignora Enter vacio, permite marcas por numero y termina solo con FIN sin distinguir mayusculas. Si el puente sigue abierto, avisa antes de la espera existente de hasta 120 s.
- S3 aplicado: ambos scripts copian solamente los registros existentes y avisan `Falta <archivo>; se sigue sin el`. Ejecutan el comparador si se archivo telemetry.csv; si falta, avisan y salen con codigo 1.
- S4 aplicado: la prueba de pausa previa al primer armado pone paused=true y armed=false en tel[0].
- S5 aplicado: no_emparejadas_drop usa aritmetica con signo int64_t y max(0, diferencia).
- S6 aplicado: BANCO.ps1 cambia ErrorActionPreference a Continue solamente durante la llamada al lanzador y lo restaura en finally.
- Archivos modificados: `ENSAYO.cmd`, `ENSAYO.ps1`, `BANCO.ps1`, `tests/picolog_checks.h`, `tests/comparar_ensayo.h`, `HANDOFF.md` y `LEEME md/ENSAYOS.md`.
- Documentacion actualizada: solo la frase de ENSAYOS.md que indicaba Enter para terminar, reemplazada por escribir FIN. Sin cambios en firmware, puente ni tesis. Los cambios previos del usuario en tesis se preservan.
- Verificacion `powershell -ExecutionPolicy Bypass -File probar.ps1`:
  - OK suite simconnect_checks - PASS: 141297 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
  - OK comparar_ensayo
  - OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
  - OK sintaxis ENSAYO.ps1
  - OK sintaxis BANCO.ps1
- Comparador `tests\bin\comparar_ensayo.exe tmp\vuelo_corto`:
  - A fidelidad: OK emparejadas=100.000% q=100.000% gust=100.000% buffet=100.000% ground=100.000% env=100.000% peor=q@91235750:0.000050 excluidas=0 esperadas=4376 no_emparejadas_drop=0
  - B1: OK violaciones=0
  - B2: OK violaciones=0
  - B3: OK violaciones=0
  - B4: OK violaciones=0
  - B5: OK violaciones=0
  - B6: OK violaciones=0 transiciones=3 evaluadas=3 latencia_max=62ms
  - B7: OK violaciones=0 pico=0.000% puente=0.000% dropped=0.000
  - B8: OK violaciones=0
  - B9: OK violaciones=0 lineas=4533 malas=0
  - RESULTADO: OK
- `graphify update .` ejecutado (AST, sin API); `git diff --check` sin errores. No se repitieron compilaciones de firmware/puente ni banco, que no forman parte de 3b. La observacion del ciclo de 39 ms queda fuera de esta tarea, como pide la revision.
- Commit de implementacion por nombre: `caab4f0` — `Ensayos: ventana abierta al terminar, fin con FIN, copia tolerante y pruebas reforzadas`. Cierre documental posterior, solo HANDOFF.md: `HANDOFF: cierre de Parte 3b y version V2.0`.
- Version creada: **V2.0**, etiqueta sobre `caab4f0`. `nueva-version.ps1` ejecutado desde PowerShell sin modificarlo, despues del commit; salida `Version V2.0 creada`. Verificado `..\MiYoke_PD_VERSIONES\V2_0\ACTUALIZACION_V2_0.txt`. Desvio operativo igual a Parte 3: checkout temporal limpio y detached del mismo commit, en el mismo directorio padre, para preservar los cambios previos de tesis y mantener el destino MiYoke_PD_VERSIONES. Checkout retirado; rama de trabajo preservada. El campo RAMA del TXT generado queda vacio por el checkout detached; la rama de origen se indica arriba.
- Sigue: revision de Claude de Parte 3b. No integrado a master.

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

- Revisor: Claude (2026-10-12), con Code Reviewer. **Parte 2 (`963338a`): APROBADA, con correcciones a hacer al empezar la Parte 3** (punto 0).
  - **Bien:**
    - Replay integrado sin tocar el bucle de SimConnect. `timeBeginPeriod`/`timeEndPeriod` sin salidas intermedias; arma solo con `--enable`; rechaza opciones incompatibles y la entrada `telemetry.csv` del EXE.
    - El comparador usa el `Receiver::feed` real y los modelos del firmware. Sobre el vuelo corto: A=100 % en 4376 revisiones (peor 5e-5), B1-B5 y B7-B9 OK, 0,44 s de ejecución, O(n log n).
    - Ajustes menores de la Parte 1b aplicados. Suite (141283 comprobaciones), comparador, firmware y puente OK según Codex; no se recompiló.
  - **R1 (corrige un falso FALLO): B6 mal especificado por Claude.** Evaluaba la pausa de arranque, previa al primer armado, y no consideraba el rearmado dentro de la ventana. Nueva regla en la tabla §4.2 (fila B6): solo transiciones armado→desarmado, latencia hasta la primera fila con `cmd=0` ≤ 150 ms, rearmado dentro de la ventana = no evaluada. Prueba: pausa antes del primer armado no evaluada; transición con cmd=0 a los 60 ms → OK; a los 200 ms → FALLO; rearmado a los 100 ms → no evaluada.
  - **R2 (menor) B7 en vuelos largos** (`tests/comparar_ensayo.h:149`). `seq` es uint16 y se desborda a las 65 536 líneas (~55 min). Usar como denominador `filas pico + pérdida`; tratar el salto hacia atrás de `seq` como desborde (`uint16_t(seq-prev)-1`) o reinicio de la Pico, sin contarlo como pérdida enorme. Prueba con `seq` 65534→65535→0→1: pérdida 0.
  - **R3 (menor) exclusión falsa al inicio** (`:85,108`). Iniciar `previousSkip` con el `skipped` de la primera fila de pico.csv, no con 0. Prueba: pico.csv que empieza con `skipped=5` constante → 0 excluidas.
  - **R4 (menor) watchdog del comparador** (`:68,77`). Antes de cada `feed`: `if(!rx.fresh(uint32_t(r.host))) models.reset();`, en lugar del hueco entre filas `sent`.
  - **R5 (menor) cierre del replay** (`SimConnectBridge/main.cpp:319-327`). Poner `opened=false` antes de los 3 `Send(false)` finales, como en `main.cpp:347`, para que salgan con modo 0.
  - **R6 (menor) tokens NaN** (`SimConnectBridge/Replay.h:35`). Aceptar también `-nan`, `nan(snan)` y `-nan(snan)` (prefijo `[-]nan` con paréntesis opcional), sin distinguir mayúsculas. Prueba con los tres.
  - **R7 (menor) fases e informe** (`tests/comparar_ensayo.h:187-191,212`).
    - Fusionar de nuevo los tramos consecutivos con el mismo nombre tras unir los cortos.
    - Escribir el orden como `03 RODAJE` (§4.5).
    - Con respaldo en `pico.log` (`host=0`), escribir `SIN DATOS` en las métricas de la Pico por fase.
    - Informar en la línea `A` las revisiones no emparejadas por `drop` (`no_emparejadas_drop=<n>`).
  - Con estas correcciones, `comparar_ensayo.exe tmp\vuelo_corto` debe dar `RESULTADO: OK`.

- Revisor: Claude (2026-10-12), con Code Reviewer. **Parte 3 (`82b5c04`, V1.9): REQUIERE CAMBIOS** (solo en los scripts del ensayo; lo demás está aprobado).
  - **Bien:**
    - R1-R7 implementados con sus pruebas. `comparar_ensayo.exe tmp\vuelo_corto` da `RESULTADO: OK` (B6: 3 transiciones evaluadas, latencia máxima 62 ms; A 100 %).
    - BANCO.ps1 solo agrega `--enable` con `-Armar` más `APAGADA` exacto o `-Confirmado` (`BANCO.ps1:12,31`); no hay otro camino que arme.
    - Reemplazos de §10 literales; `LEEME md/ENSAYOS.md` coincide con el código; tesis sin tocar.
    - Tag V1.9 y `ACTUALIZACION_V1_9.txt` correctos.
  - **Hallazgos (verificados por Claude en el código):**
    - **S1 (medio)** `ENSAYO.cmd:5`: hace `pause` solo si hay error, así que con doble clic y resultado OK la ventana se cierra antes de leer el informe. Cambiar por `pause` siempre.
    - **S2 (medio)** `ENSAYO.ps1:46-53`: el mismo `Read-Host` sirve para marcar fases y para terminar; un Enter vacío dado por error cierra el ensayo y copia registros incompletos. Cambiar el bucle por:
      - prompt `Escribi un numero de fase para marcarla, o FIN para terminar (despues de cerrar el simulador)`;
      - el Enter vacío se ignora y se vuelve a preguntar;
      - `FIN` (sin distinguir mayúsculas) sale;
      - si al salir el puente sigue abierto, mostrar `Esperando a que se cierre el puente (cerra el simulador)... hasta 120 s` antes de la espera existente.
    - **S3 (bajo)** `ENSAYO.ps1:58-60` y `BANCO.ps1:34-36`: si falta un registro, `Copy-Item` corta con una excepción. Copiar solo los que existen y avisar en español: `Falta <archivo>; se sigue sin el`. El comparador se ejecuta igual si existe `telemetry.csv`; si no existe, avisar y salir con código 1.
    - **S4 (detalle)** `tests/picolog_checks.h:84-86`: la prueba de "pausa antes del primer armado" pasa con cualquier versión. Poner `paused=1` (y `armed=0`) en `tel[0]` para que exista de verdad una pausa previa al armado que no debe evaluarse.
    - **S5 (detalle)** `tests/comparar_ensayo.h:177`: `no_emparejadas_drop` es una resta sin signo; usar `max(0, ...)` con aritmética con signo.
    - **S6 (detalle)** `BANCO.ps1:20`: con `2>&1` bajo `ErrorActionPreference=Stop`, una línea de error del lanzador corta la detección. Rodear esa llamada con `$prev=$ErrorActionPreference; $ErrorActionPreference='Continue'` y restaurarlo después (patrón de `probar.ps1:44-49`).
  - **Observación aparte (fuera de esta tarea; no corregir ahora):** en el banco, la primera línea `SC8` ya traía `lp=39252 gap=39297` µs, con `us=286` igual que en el vuelo corto. La Pico no se reinició entre el vuelo corto (lp=2412) y el banco, así que un ciclo de unos 39 ms ocurrió **entre los dos**, sin puente activo (cierre de P3D/puente o detección de puerto). El vuelo V1.8 del 11/10 tuvo un valor parecido (lp=38948). Es una hipótesis a investigar con `depuracion-sistematica` en una tarea propia, porque el comando a los motores queda congelado durante ese ciclo.

\### Parte 3b para Codex (correcciones S1-S6 y versión)
- Leer solo: esta sección "Revisión" (S1-S6) y las líneas indicadas en cada hallazgo.
- Alcance: S1 a S6 exactamente como están escritos. No tocar firmware, puente ni documentación (los textos de `LEEME md/ENSAYOS.md` siguen siendo válidos; si alguno menciona el Enter para terminar, cambiarlo por `FIN`, y solo eso).
- Pruebas:
  - `powershell -ExecutionPolicy Bypass -File probar.ps1` (suite, comparador y lanzador);
  - verificar sintaxis de `ENSAYO.ps1` y `BANCO.ps1` como en la Parte 3;
  - `tests\bin\comparar_ensayo.exe tmp\vuelo_corto` debe dar `RESULTADO: OK`.
- Commit por nombre: `ENSAYO.cmd ENSAYO.ps1 BANCO.ps1 tests/picolog_checks.h tests/comparar_ensayo.h HANDOFF.md` (más `"LEEME md/ENSAYOS.md"` solo si se tocó). Mensaje: "Ensayos: ventana abierta al terminar, fin con FIN, copia tolerante y pruebas reforzadas".
- Versión: `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Correcciones de scripts de ensayo: pausa final, fin con FIN y copia tolerante"`. Verificar `Version Vx.y creada` y el `ACTUALIZACION_Vx_y.txt`.
- Completar "Última entrega": las líneas OK/FALLO, el `RESULTADO` del comparador y la versión creada.

- Revisor: Claude (2026-10-12), con Code Reviewer. **Parte 3b (`caab4f0`, V2.0): APROBADA.**
  - S1, S2, S3, S5 y S6 cumplen: sintaxis OK; `continue`/`break` correctos en PS 5.1; `-eq 'FIN'` no distingue mayúsculas; `$ErrorActionPreference` se restaura en `finally`.
  - Comparador sobre `tmpuelo_corto`: `RESULTADO: OK`. Tag V2.0 sobre `caab4f0` y `ACTUALIZACION_V2_0.txt` presentes.
  - **Pendiente menor** (para una tarea futura; no justifica otra versión):
    - S4: la prueba de "pausa inicial" sigue sin discriminar, porque `compare` empieza en `i=1` y `tel[1]` está armada. Mi indicación era insuficiente; corrección: tres filas (desarmada, en pausa desarmada, armada).
    - " FIN" con espacios no se recorta (`ENSAYO.ps1:48`).
  - **Usuario:**
    - cargar `build-pico2-potentiometers/MiYoke_PD.ino.uf2` (12/10 21:37, incluye el ajuste de `lastSkip` de la Parte 2);
    - banco armado con la fuente de motores apagada;
    - vuelo con `ENSAYO.cmd`.
  - Integración a `master` solo a pedido del usuario, después de esos ensayos.

\## Ensayos del usuario

- Vuelo completo con `ENSAYO.cmd` (usuario, 2026-10-12, V2.0, fuente de motores apagada). Carpeta: `ensayos/20261012-220623_vuelo1/`. Análisis de Claude con Test Results Analyzer.
  - **Resultado:** `RESULTADO: OK`.
    - Fidelidad 100 % en 14974 revisiones; B1-B9 OK.
    - B6: 13 desarmes (pausas, datos viejos, slew, simulador detenido) en 31-63 ms; las reactivaciones, en 15-16 ms.
    - Sin pérdidas ni líneas malas; us/lp/gap máximos 277/2377/2404 µs; ningún ciclo ≥ 10 ms.
  - **Por fase:**
    - Menú: 13,1 s con cmd=pwm=0; ganancias sin cambios (se salió sin guardar).
    - Suelo: aporte máximo 40,0 PWM en roll y 4,0 en pitch (techo respetado en todas las filas); 0 en las 9565 filas en el aire.
    - Centrado: rigidez 0,150 en tierra frente a 0,713 de media en vuelo; signo opuesto a la posición 95-100 %.
    - Turbulencia solo en F13 (RMS 0,13 roll y 0,14 pitch).
    - Buffet de tren: 0 con el tren arriba; satura desde 150 kt.
    - Buffet de pérdida desde aoa ≈ stall_alpha − 3,9°.
  - **Observaciones del ensayo:**
    - Las marcas se pusieron al terminar cada fase (no al empezarla); el resumen por fase se lee desplazado una fase.
    - Durante el slew, P3D deja de enviar muestras (`running=0`), así que `slew≠0` nunca llega. La Pico desarmó igual en 46 ms. El comparador lo clasifica como el tramo anterior: pendiente menor, clasificar `DETENIDO` con `running=0` o `armed=0`.
    - Crucero con el tren arriba muy breve (tramo VUELO de 3,5 s); se voló hasta 190 kt con el tren abajo.
  - **HALLAZGO IMPORTANTE (antes de encender motores):** el pote de pitch, **en la lectura de la Pico** (ADC), tiene 4 picos de una sola muestra hacia valores extremos (p. ej. 0,84 → −1,00 → 0,85 en host 95754046). El `yoke_pitch` del simulador (Arduino UNO, mismo cursor) no tiene ninguno, y roll tampoco. Esos picos cambian el comando de pitch hasta ~275 PWM durante ~47 ms; con motores sería un golpe. Hipótesis: contacto o divisor en el tramo cursor → Pico (ADC1/GPIO27, AGND) o ruido del ADC; pico.csv registra a ~20 Hz, así que a 500 Hz puede haber más. A investigar con `depuracion-sistematica` en una tarea propia (hardware y, si corresponde, filtro de picos en `PotentiometerPosition`).
  - **Aclaración del usuario (2026-10-13) sobre el hallazgo del pote de pitch:**
    - Los potes son solo un banco de pruebas: están soldados en una placa experimental, se mueven a mano y su salida va por una protoboard empalmada al UNO y a la Pico.
    - El proyecto final usa encoders, así que los saltos son del montaje de prueba y **no** se corrigen en el firmware. Por el mismo ruido, Damper y Fricción hacen variar el PWM con los potes quietos.
    - Al montar los encoders, repetir el banco y `ENSAYO.cmd` y revisar en `pico.csv` `pos`/`vel` en reposo y los saltos, antes de encender los motores.
  - **Otros pendientes para tareas futuras:**
    - ciclo de 39 ms visto una vez entre ensayos;
    - rigidez de centrado hasta 2,25 a 190 kt (con el pote a tope el comando se recortaría): revisar con TelemFFB antes de usar motores;
    - menores del comparador: S4, `FIN` con espacios, `SIN DATOS` en el signo de centrado.

- Banco armado (usuario, 2026-10-12, firmware V2.0 cargado, fuente de motores apagada confirmada con `APAGADA`). Carpeta: `ensayos/20261012-215853_banco/`.
  - **Resultado:** `RESULTADO: OK`.
    - A: 100 % en 4376 revisiones (peor 5e-5).
    - B1-B9 OK; B6: 3 transiciones, latencia máxima 47 ms.
    - Sin pérdidas (B7 0 %) ni líneas malas (B9: 4535 líneas, 0 malas).
  - **Tiempos:** us/lp/gap máximos = 277/2303/2333 µs; un solo ciclo de lazo ≥ 2000 µs. En este arranque no se repitió el ciclo de 39 ms.
  - **Fases:** coherentes con el vuelo grabado: buffet de pérdida 0,68 en PERDIDA; buffet de tren hasta 1,0 a 150 kt con tren extendido; suelo solo en RODAJE; comando 0 en las 3 pausas.
  - **Observaciones:**
    - `centrado signo R=0% P=0%` no indica un error. En el banco los potes estuvieron quietos (pos_roll entre -0,025 y 0,018; pos_pitch entre -0,170 y -0,132), así que ninguna fila cumple |pos| > 0,2 y el 0 % sale de 0/0. Pendiente menor: informar `SIN DATOS` cuando no haya filas.
    - Con los potes en reposo, pitch lee alrededor de -0,15. Revisar el centro calibrado del pote de pitch o su posición física antes del vuelo; si no, el centrado empuja siempre en pitch.

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
