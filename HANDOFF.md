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

Tarea: opción REGISTRO DATOS en el menú y diagnóstico de ciclos lentos del lazo (Claude, 2026-10-13).
Rama: `mejora/registro-menu-y-ciclos-lentos` (ya creada; contiene el diseño). Diseño: `LEEME md/DISENO_registro_menu_y_ciclos_lentos.md`.
Se divide en **3 partes**, porque hay estado nuevo en firmware y en puente, un registro persistente, muchas pruebas y documentación con `.tex`. Cada parte se hace en una sesión de Codex distinta.

\### Precondiciones (todas las partes)
- `git checkout mejora/registro-menu-y-ciclos-lentos` y `git status`. Ignorar `graphify-ino/`, `tesis/main.tex`, `tesis/capitulos/11_maquinas_estado.tex` y `tesis/caracterizacion_motores_775.tex`: son cambios del usuario; no tocarlos ni commitearlos.
- Orientarse con `graphify query/explain` antes de abrir código; leer solo las líneas indicadas. Las líneas del diseño son de `master` V2.0; si no coinciden, ubicar por contenido.
- No tocar constantes de seguridad (regla 7). No usar `git add .` ni `git add -A`.
- Pruebas solo con `powershell -ExecutionPolicy Bypass -File probar.ps1 [...]`; en HANDOFF copiar solo las líneas OK/FALLO.
- Si el diseño contradice el código, detenerse y anotarlo en "Última entrega"; no improvisar.

\### Parte 1 — Encabezados puros + pruebas (sin versión)
- Leer del diseño: §2, §3.1, §4.1, `formatRecord` de §4.3, §5.1 y §8.
- Código a leer: `CalibrationMenu.h` (completo), `LoopSupervisor.h` (completo), `PicoLog.h` (completo), `SimConnectBridge/PicoCsv.h` (completo), `GainSettings.h:55-85`, `PotCalibration.h:20-30`, `tests/calibration_checks.h` (estilo y primeras 40 líneas), `tests/picolog_checks.h` (estilo), `tests/simconnect_checks.cpp:50-66`.
- Alcance:
  1. Crear `RecordSetting.h` (§2).
  2. `CalibrationMenu.h` según §3.1.
  3. `LoopSupervisor.h`: `Between`, `StageTimer`, `SlowLog` y `formatSlow` (§4.1).
  4. `PicoLog.h`: `formatRecord`.
  5. `SimConnectBridge/PicoCsv.h`: `CsvGate`, `recordLine` y `slowLine` (§5.1).
  6. Pruebas de §8 en `tests/calibration_checks.h` y `tests/picolog_checks.h`. La de `comparar_ensayo` queda para la Parte 2.
  7. `graphify update .`.
- Pruebas: `probar.ps1`. Tienen que dar OK la suite, el comparador y el lanzador. El firmware todavía no se compila, porque `CalibrationControls.h` y el `.ino` cambian en la Parte 2.
- Commit por nombre: `RecordSetting.h CalibrationMenu.h LoopSupervisor.h PicoLog.h SimConnectBridge/PicoCsv.h tests/calibration_checks.h tests/picolog_checks.h HANDOFF.md`.
  Mensaje: "Menu REGISTRO DATOS, registro persistente y medicion de ciclos lentos: partes puras y pruebas".
- **No** ejecutar `nueva-version.ps1`. Completar "Última entrega".

\### Parte 2 — Integración: firmware, puente, comparador y scripts (sin versión)
- Precondición: la Parte 1 está commiteada y anotada.
- Leer del diseño: §3.2, §3.3, §4.2, §4.3, §5.2, §6 y §7 (para `CICLOS.ps1`).
- Código a leer:
  - `CalibrationControls.h` (completo);
  - `MiYoke_PD.ino` (completo);
  - `mis_defines.h:160-230`;
  - `SimConnectBridge/main.cpp` (completo);
  - `tests/comparar_ensayo.h:250-300`;
  - `ENSAYO.ps1` y `ENSAYO.cmd`;
  - `Iniciar-Simulador.ps1:18-19` (ruta del puente).
- Alcance:
  1. `CalibrationControls.h` (§3.2), `mis_defines.h` (`YOKE_RECORD_OPTION`, `YOKE_SLOW_LOOP_US`) y `MiYoke_PD.ino` (§3.3, §4.2 y §4.3).
  2. `SimConnectBridge/main.cpp` (§5.2).
  3. `tests/comparar_ensayo.h` (§6.1) y su prueba en `tests/picolog_checks.h`.
  4. `ENSAYO.ps1` (§6.2).
  5. `CICLOS.ps1` y `CICLOS.cmd` (§6.3).
  5b. Pruebas menores de la revisión de la Parte 1, en `tests/calibration_checks.h`:
     - ciclo de CALIBRACIÓN partiendo de Edit, EditEffect y CalibratePots, con 4 listas y con 3;
     - comprobar los textos de `rows()` completos (`strncmp` con el texto esperado más el relleno de espacios), no solo `strlen<=16`;
     - en la prueba de `decode` inválido, iniciar `loaded=false` y verificar que siga en `false`.
     En el puente, descartar `v` cuando `slowLine` devuelve `false`.
  6. `graphify-sync.cmd` (se editó el `.ino`) y `graphify update .`.
- Pruebas:
  - `probar.ps1 -Pico -Potentiometers -Encoders -Bridge`: todo OK. Anotar el tamaño frente a 104 500 B.
  - Compilar una vez con `YOKE_MODEL_LOG 0` (`probar.ps1 -Pico`) y volver a 1.
  - Verificar la sintaxis de `CICLOS.ps1` y `ENSAYO.ps1` como en la tarea anterior.
  - `tests\bin\comparar_ensayo.exe tmp\vuelo_corto` tiene que seguir dando `RESULTADO: OK`.
- Commit por nombre: `CalibrationControls.h mis_defines.h MiYoke_PD.ino SimConnectBridge/main.cpp tests/comparar_ensayo.h tests/picolog_checks.h ENSAYO.ps1 CICLOS.ps1 CICLOS.cmd HANDOFF.md`.
  Mensaje: "Pico y puente: registro CSV activable desde el menu y eventos de ciclos lentos".
- **No** ejecutar `nueva-version.ps1`.

\### Parte 3 — Documentación + versión
- Leer del diseño: §11.
- Alcance:
  1. Aplicar los reemplazos exactos de §11 en `LEEME md/MENU_CALIBRACION.md`, `LEEME md/CALCULO_PICO_SC8.md`, `LEEME md/ENSAYOS.md` y `tesis/capitulos/03_centrado.tex`. Buscar cada TEXTO ACTUAL literal; si alguno no aparece, anotarlo y seguir con el resto. No leer capítulos completos.
  2. En `LEEME md/ENSAYOS.md`, agregar al final una sección breve `## Ciclos lentos (CICLOS.cmd)` con la tabla de pasos de §7 del diseño, copiada tal cual.
  3. Ajustes menores pendientes de la tarea anterior, en `tests/comparar_ensayo.h` y `tests/picolog_checks.h`:
     - `centrado signo` con 0 filas → `SIN DATOS`;
     - la prueba de "pausa inicial" con 3 filas (desarmada, pausada y desarmada, armada), esperando `transiciones=0 evaluadas=0`;
     - en `ENSAYO.ps1` y `CICLOS.ps1`, recortar espacios antes de comparar con `FIN`.
  4. `graphify update .`.
- Pruebas:
  - `probar.ps1 -Pico -Potentiometers -Encoders -Bridge`: todo OK;
  - `powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1`: anotar su línea;
  - `tests\bin\comparar_ensayo.exe tmp\vuelo_corto`: `RESULTADO: OK`.
- Commit por nombre: `"LEEME md/MENU_CALIBRACION.md" "LEEME md/CALCULO_PICO_SC8.md" "LEEME md/ENSAYOS.md" tesis/capitulos/03_centrado.tex tests/comparar_ensayo.h tests/picolog_checks.h ENSAYO.ps1 CICLOS.ps1 HANDOFF.md`.
  Mensaje: "Documentacion del menu REGISTRO DATOS y ciclos lentos; ajustes menores del comparador".
- Versión: `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Menu REGISTRO DATOS para activar los CSV y diagnostico de ciclos lentos del lazo"`. Verificar `Version Vx.y creada` y `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt`. Si los cambios del usuario en `tesis/` impiden ejecutar el script, usar el mismo procedimiento de la tarea anterior (checkout temporal limpio) y anotarlo.
- "Última entrega": lo hecho, las líneas OK/FALLO, la línea de la tesis, la versión y los desvíos.
- Luego: revisión de Claude; el usuario carga el UF2 y hace el ensayo `CICLOS.cmd` (§7). No integrar a `master`.

\## Última entrega

- Parte 1 completada (2026-10-13): encabezados puros y pruebas, sin integrar firmware ni puente.
- Archivos modificados: `RecordSetting.h`, `CalibrationMenu.h`, `LoopSupervisor.h`, `PicoLog.h`, `SimConnectBridge/PicoCsv.h`, `tests/calibration_checks.h`, `tests/picolog_checks.h` y `HANDOFF.md`.
- Implementado: registro YREC de 8 B con CRC y validacion sin modificar la salida ante error; cuarta lista REGISTRO DATOS con guardado/reintento; StageTimer, anillo SlowLog de 4 eventos y formato W; formato L; CsvGate y parsers L/W estrictos.
- Pruebas ejecutadas con `powershell -ExecutionPolicy Bypass -File probar.ps1`:
  - OK suite simconnect_checks - PASS: 143412 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
  - OK comparar_ensayo
  - OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
- `git diff --check`: sin errores. `graphify update .` ejecutado: 1145 nodos, 1499 aristas. No se edito el .ino.
- Documentacion: HANDOFF actualizado; los reemplazos de documentos funcionales y tesis corresponden a la Parte 3 segun el plan. No se edito .tex ni se compilo la tesis.
- Firmware y puente: no compilados en esta parte, segun el plan; integracion pendiente de Parte 2.
- Commit: el de esta entrega, mensaje "Menu REGISTRO DATOS, registro persistente y medicion de ciclos lentos: partes puras y pruebas", en `mejora/registro-menu-y-ciclos-lentos`.
- Version: no creada, tal como exige la Parte 1; `nueva-version.ps1` queda para la Parte 3.
- Cambios del usuario en `tesis/main.tex`, `tesis/capitulos/11_maquinas_estado.tex`, `tesis/caracterizacion_motores_775.tex` y `graphify-ino/` preservados fuera del commit.
- Desvios: ninguno. Sigue Parte 2 (integracion firmware, puente, comparador y scripts).

\## Revisión

- Revisor: Claude (2026-10-13), con Code Reviewer. **Parte 1 (`629e5a4`): APROBADA.**
  - **Diseño:** `RecordSetting.h`, `CalibrationMenu.h`, `LoopSupervisor.h`, `PicoLog.h` y `PicoCsv.h` cumplen §2, §3.1, §4.1, §4.3 y §5.1.
    - Menú: ciclo con 4 y 3 listas; los bloques del registro van antes del de Browse; la rama de guardado del registro va primero; RecordSaved se evalúa antes que las pantallas; todos los textos ≤ 16. Las otras listas no cambian de comportamiento.
    - Anillo y temporizador: correctos, con resta sin signo y saturación.
    - Parsers: estrictos.
  - **Firmware:** el firmware no incluye STL; `PicoCsv.h` no llega al `.ino`.
  - **Suite:** 143412 comprobaciones OK según Codex; no se recompiló.
  - **Observaciones menores** (pasan a la Parte 2, punto 5b):
    - falta probar el ciclo desde las pantallas de edición;
    - la comprobación de `strlen` no puede fallar porque `padded` trunca;
    - la prueba de `decode` inválido no detecta que se escriba `true`;
    - descartar `v` si `slowLine` falla.

\## Ensayos del usuario

- (vacío: tarea nueva. Los resultados de la tarea anterior, registro/ensayo de la Pico V1.9-V2.0, están en `master`, commit `66a4a51`.)
