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

- Parte 1: implementacion parcial, detenida por contradiccion entre el diseno y el codigo (2026-10-12). No lista para cargar el UF2.
- Bloqueo: el diseno §2.1 exige `Paused=simTelemetry.paused`, usado en `MiYoke_PD.ino:351`, pero `Receiver::paused` es privado (`SimTelemetry.h:116`) y no hay accesor publico. El alcance y la lista de archivos de Parte 1 no incluyen `SimTelemetry.h`. Claude debe especificar como exponer/leer ese estado y autorizar el archivo afectado; Codex no cambio su visibilidad ni sustituyo el dato.
- Archivos implementados sin commit: `PicoLog.h`, `SimEffects.h`, `mis_defines.h`, `MiYoke_PD.ino`, `SimConnectBridge/PicoCsv.h`, `SimConnectBridge/main.cpp`, `tests/picolog_checks.h`, `tests/simconnect_checks.cpp`. Registros M/G, accesores/skip, recepcion/rotacion CSV y PollPort; casos 1-6 y 9.
- Documentacion: solo este HANDOFF; los reemplazos de documentacion siguen previstos para Parte 3. Cambios previos del usuario en tesis y graphify-ino preservados.
- Verificacion:
  - OK suite simconnect_checks - PASS: 141133 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
  - OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
  - FALLO Pico 2 (configurado) (codigo 1): `MiYoke_PD.ino:351:75: error: 'bool YokeSC::Receiver::paused' is private within this context`.
- La corrida tambien reporto `FALLO Pico 2 (encoders) (codigo 1)` por el mismo miembro privado. Se interrumpio la corrida restante al confirmar el bloqueo; potenciometros y puente no verificados.
- La primera prueba del contador skip fallo porque el ensayo enviaba dos tramas con el mismo timeMs; Receiver descarta la repetida (`SimTelemetry.h:73`). Se corrigio el caso para enviar timeMs distintos con la misma llegada, siguiendo `tests/raw_checks.h`; la corrida posterior dio los OK anteriores.
- Ejecutados graphify-sync.cmd y graphify update .; grafo actualizado. git diff --check sin errores de whitespace.
- Pendientes: resolver el bloqueo con especificacion de Claude, terminar compilaciones de las tres variantes y puente, medir crecimiento frente a 103148 B, compilar con YOKE_MODEL_LOG 0 y volver a 1, commit por nombre. No hay commit de entrega ni version (Parte 1 sin version segun el plan).
- Luego de una entrega con todo OK: cargar UF2 de build-pico2-potentiometers (o variante usada), vuelo corto de 2 min, pegar en Ensayos del usuario la linea Fin: de bridge.log y las 3 primeras filas de pico.csv. No hacer ese ensayo con esta entrega parcial.


\## Revisión

- Claude (2026-10-12), sobre el bloqueo de la Parte 1: correcto detenerse. Hueco del diseño: `Receiver::paused` es privado (`SimTelemetry.h:116`). Resuelto en §2.2b: agregar el accesor público `bool isPaused() const { return paused; }` en `SimTelemetry.h` y usar `simTelemetry.isPaused()` en `MiYoke_PD.ino`. Se autoriza `SimTelemetry.h` solo para ese accesor (agregado al alcance y al commit de la Parte 1) y su prueba. Continuar la Parte 1 desde los cambios sin commit: arreglar el acceso, completar las compilaciones pendientes (encoders, potenciómetros, puente, `YOKE_MODEL_LOG 0`) y commitear.
- Además: la tesis queda fuera de esta tarea (pedido del usuario); se quitó de §10 y de la Parte 3.
\## Ensayos del usuario

- (el usuario o Codex anotan acá: resultado del vuelo corto, confirmación "fuente de motores apagada" con fecha, y la carpeta de cada ensayo en `ensayos\`)
