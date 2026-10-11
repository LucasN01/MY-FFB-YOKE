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

Tarea: congelamiento de la Pico 2 y recuperación del enlace USB (Claude, 2026-10-11).
Rama: `mejora/recuperacion-enlace-usb` (ya creada; contiene el diseño). Diseño:
`LEEME md/DISENO_recuperacion_enlace_usb.md`. Se divide en **2 partes** (una sesión de
Codex por parte), porque toca puente, firmware con estado y documentación (con .tex).

\### Precondiciones (ambas partes)
- `git checkout mejora/recuperacion-enlace-usb` y `git status` limpio, salvo `graphify-ino/` sin versionar, que se ignora.
- Orientarse con `graphify query/explain` antes de abrir código. Leer solo las líneas indicadas.
- No tocar constantes de seguridad (regla 7). No usar `git add .` ni `git add -A`.
- Pruebas solo con `powershell -ExecutionPolicy Bypass -File probar.ps1 [...]`. En esta sección copiar únicamente las líneas OK/FALLO.

\### Parte 1 — Puente + pruebas (sin versión)
- Leer del diseño: §1.4, §1.5, §3.1, §3.2 y §5 "Escritorio / Parte 1".
- Código a leer: `SimConnectBridge/main.cpp` (completo, 215 líneas), `tests/simconnect_checks.cpp` (solo los `#include` y `main`) y `tests/simconnect_checks.vcxproj:12`.
- Alcance:
  1. Crear `SimConnectBridge/SerialLink.h` tal como §3.1: todo `inline`, sin `Windows.h`.
  2. Aplicar §3.2, puntos 1 a 11, en `SimConnectBridge/main.cpp`.
  3. Crear `tests/link_checks.h` con los casos de §5 Parte 1 e incluirlo desde `tests/simconnect_checks.cpp`, con el mismo estilo `CHECK` que los demás.
- Pruebas: `probar.ps1 -Bridge`. Tiene que dar OK la suite C++ y la compilación del puente.
- Commit por nombre: `SimConnectBridge/SerialLink.h SimConnectBridge/main.cpp tests/link_checks.h tests/simconnect_checks.cpp HANDOFF.md`.
  Mensaje: "Puente: reconexion USB automatica, diagnostico de enlace y rotacion de registros".
- Ejecutar `graphify update .`.
- **No** ejecutar `nueva-version.ps1` en esta parte.
- Completar "Última entrega" con lo hecho en la Parte 1, las líneas OK/FALLO y los desvíos.

\### Parte 2 — Firmware + documentación + versión
- Precondición: la Parte 1 está commiteada y anotada en "Última entrega".
- Leer del diseño: §1.2, §3.3, §3.4, §4, §5 "Parte 2" y §6.
- Código a leer: `MiYoke_PD.ino` (completo, 255 líneas), `mis_defines.h` alrededor de `CONTROL_LOOP_MS` (línea ~164) y `LoopSupervisor.h`/`UsbGuard.h` (nuevos).
- Verificaciones previas, con grep sobre `%LOCALAPPDATA%\Arduino15\packages\rp2040\hardware\rp2040\6.1.1`:
  - que ninguna librería defina un `yield()` fuerte para rp2040;
  - que el core no use `watchdog_hw->scratch[0]`;
  - el nombre exacto del registro de reinicio de POWMAN en RP2350 (`powman_hw->chip_reset` o equivalente).
  - Anotar el resultado en "Última entrega". Si algo contradice el diseño, detenerse y anotarlo; no improvisar.
- Alcance:
  1. Crear `UsbGuard.h` (§3.3) y `LoopSupervisor.h` (§3.4): parte pura `inline` y hardware bajo `ARDUINO_ARCH_RP2040`.
  2. Agregar a `mis_defines.h` las constantes de §3.4 con esos valores por defecto: `YOKE_USB_GUARD 1`, `YOKE_TEST_HANG 0`, `YOKE_USB_STRESS 0`.
  3. En `MiYoke_PD.ino`, aplicar §3.3 (`yield()` propio y `UsbLock` en cada acceso a `Serial`) y §3.4, puntos 1 a 10.
  4. Crear `tests/health_checks.h` (§5 Parte 2) e incluirlo desde `tests/simconnect_checks.cpp`.
  5. Documentación: aplicar los reemplazos exactos de §6.1 a §6.6.
     - Buscar el TEXTO ACTUAL literal y reemplazarlo. No leer capítulos completos ni redactar.
     - Si un texto actual no aparece exactamente, anotarlo aquí y seguir con el resto.
  6. Ajustes menores de la revisión de la Parte 1, en `SimConnectBridge/main.cpp`:
     - (a) En `LinkLost`, terminar el mensaje con "Reconectando %s..." solo si `!quit`; si `quit` está activo, terminarlo con "Fin del puente." (no reconectar mientras el programa termina).
     - (b) En `Stamp`, pasar `unsigned(t.wHour)`, `unsigned(t.wMinute)` y `unsigned(t.wSecond)`.
     - Agregar `SimConnectBridge/main.cpp` al commit y compilar también con `-Bridge`.
  7. Ejecutar `graphify-sync.cmd` (se editó el `.ino`) y `graphify update .`.
- Pruebas:
  - `probar.ps1 -Pico -Potentiometers -Encoders -Bridge` (incluye la suite C++). Tiene que dar todo OK.
  - `powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1`: anotar su única línea.
- Commit por nombre: `UsbGuard.h LoopSupervisor.h mis_defines.h MiYoke_PD.ino SimConnectBridge/main.cpp tests/health_checks.h tests/simconnect_checks.cpp "LEEME md/LEEME-LANZADOR.md" "LEEME md/LEEME_SIMCONNECT.md" "LEEME md/EFECTOS_SIMCONNECT.md" "LEEME md/MODELO_TURBULENCIA.md" "LEEME md/CALCULO_PICO_SC8.md" tesis/capitulos/01_arquitectura_sc8.tex HANDOFF.md`.
  Mensaje: "Pico: mutex USB contra interbloqueo, watchdog por hardware y diagnostico de reinicio".
- Versión:
  - Comando: `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Recuperacion del enlace USB: mutex USB en Pico, watchdog por hardware y reconexion del puente"`.
  - Verificar que imprima "Version Vx.y creada" y que exista `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt`.
- Completar "Última entrega" con:
  - lo hecho en la Parte 2;
  - las líneas OK/FALLO;
  - la línea de `compilar-tesis.ps1`;
  - las verificaciones previas;
  - la versión creada;
  - los desvíos.
- Luego: revisión de Claude y pruebas A a E del usuario (§5 Hardware). No integrar a `master`.

\## Última entrega

- Parte 2 completada (2026-10-11) en `mejora/recuperacion-enlace-usb`; Parte 1 aprobada por Claude en `e5a7c65` (implementacion `da4bd21`).
- Archivos de codigo: `UsbGuard.h`, `LoopSupervisor.h`, `mis_defines.h`, `MiYoke_PD.ino`, `SimConnectBridge/main.cpp`, `tests/health_checks.h`, `tests/simconnect_checks.cpp`.
- Implementado: mutex USB y yield propio; motores al principio de setup; watchdog de 250 ms (3000 ms al guardar flash) y alimentacion antes del retorno de 2 ms; etapas persistentes, HardFault, BOOT y maximos lp/gap; ensayos de cuelgue y estres desactivados por defecto. Ajustes del puente de la revision: formatos unsigned y mensaje de fin con quit activo.
- Documentacion actualizada mediante los ocho reemplazos exactos de §6: `LEEME md/LEEME-LANZADOR.md`, `LEEME md/LEEME_SIMCONNECT.md`, `LEEME md/EFECTOS_SIMCONNECT.md`, `LEEME md/MODELO_TURBULENCIA.md`, `LEEME md/CALCULO_PICO_SC8.md` y `tesis/capitulos/01_arquitectura_sc8.tex` (tres fragmentos). Ningun texto faltante.
- Verificaciones previas sobre Arduino-Pico 6.1.1 [Codigo]: `cores/rp2040/delay.cpp:30` declara yield debil; no hay otro yield fuerte en las bibliotecas enlazadas para RP2040. Las definiciones STM32/ESP32/CH32, ejemplos y tests de host no se enlazan; el yield de `cores/rp2040/freertos/freertos-main.cpp:111` esta bajo `__FREERTOS` (:21), desactivado en la configuracion de `Build-Pico2.ps1:13`. No hay uso de `watchdog_hw->scratch[0]` en core/SDK; `hardware_watchdog/watchdog.c:84,102-108` usa scratch[4..7]. `pico-sdk/src/rp2350/hardware_structs/include/hardware/structs/powman.h:113` confirma `powman_hw->chip_reset`. `RP2040Support.h:596,582,589` confirma el orden de resetReason_t y wdt_begin/wdt_reset.
- Pruebas: `powershell -ExecutionPolicy Bypass -File probar.ps1 -Pico -Potentiometers -Encoders -Bridge`:

```text
OK suite simconnect_checks - PASS: 141059 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
OK Pico 2 (configurado) - Sketch uses 103148 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22556 bytes (4%) of dynamic memory, leaving 501732 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (encoders) - Sketch uses 103308 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22792 bytes (4%) of dynamic memory, leaving 501496 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (potenciometros) - Sketch uses 103148 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22556 bytes (4%) of dynamic memory, leaving 501732 bytes for local variables. Maximum is 524288 bytes.
OK puente SimConnect
```
- Tesis:

```text
OK chequeo estatico (11 archivos, 44 labels). No hay LaTeX instalado: la compilacion real se hace en Overleaf.
```

- `graphify-sync.cmd` y `graphify update .`: ejecutados; grafo actualizado (918 nodos, 1160 aristas). `git diff --check`: sin errores.
- Commit de codigo y documentacion: `69a4097`, por los archivos nombrados en el plan: `Pico: mutex USB contra interbloqueo, watchdog por hardware y diagnostico de reinicio`.
- Version: V1.8 creada por nueva-version.ps1 sobre el commit `69a4097`. Verificado `..\MiYoke_PD_VERSIONES\V1_8\ACTUALIZACION_V1_8.txt` existente. El commit posterior de HANDOFF solo registra esta version y los hashes de entrega.
- Desvios: ninguno de alcance; la tesis solo tiene chequeo estatico porque no hay compilador LaTeX instalado, segun compilar-tesis.ps1.
- Sigue: revision de Claude y pruebas A a E del usuario (§5 Hardware), incluida la verificacion de EN y los pull-down externos requeridos por el diseno. No integrar a master.
\## Revisión

\- Revisor: Claude (2026-10-11), con Code Reviewer. **Parte 1 (`da4bd21`): APROBADA.**
  - **Fidelidad al diseño:** `SerialLink.h` es literal a §3.1, y `main.cpp` cumple los puntos 1 a 11 de §3.2: cierre del handle en todas las salidas de `OpenPort`, `quit` en el handshake, desarme con el puerto cerrado o durante el diferido, `Record(wrote,wrote&&armed)`, reconexión solo con puerto, salida 4, rotación condicionada y texto viejo eliminado.
  - **Errores:** no hay reentrada (un solo hilo; `Send` nunca se llama durante `OpenPort`), no hay uso del handle después de cerrarlo y los formatos son correctos.
  - **Pruebas:** `link_checks.h` cubre todos los casos de §5 Parte 1 con los valores esperados correctos. No se recompiló: Codex informó OK y el código no cambió desde entonces.
  - **Observaciones menores**, que se incorporan a la Parte 2, punto 6:
    - con `quit` activo, `LinkLost` imprime "Reconectando..." aunque el programa está terminando;
    - en `Stamp`, convertir los campos `WORD` a `unsigned`.
  - **Pruebas que el usuario ya puede hacer:** A (desenchufar y volver a enchufar en vuelo) y C (guardar efectos en pausa), con la Parte 1 compilada.

\- Revisor: Claude (2026-10-11), con Code Reviewer. **Parte 2 (`69a4097`, versión V1.8): APROBADA.**
  - **Corrección H5:** todos los accesos a `Serial` de `loop()` están bajo `UsbLock`, sin locks anidados y con `availableForWrite()` antes de cada `write`. El `yield()` propio usa `try_enter`. Ningún otro encabezado usa `Serial` (grep).
  - **Watchdog:**
    - 250 ms, alimentado antes del retorno de 2 ms;
    - 3000 ms solo alrededor de los dos `calibration.save()`;
    - motores al principio de `setup()`;
    - `isr_hardfault` fuerte, con el nombre del vector del SDK;
    - el motivo y la etapa se leen antes de `mark(Idle)`;
    - la línea BOOT se envía una sola vez, después de la primera trama válida;
    - los ensayos quedan fuera de la compilación por defecto y no se repiten tras un reinicio WDT.
  - **Pruebas y compilación:** `health_checks.h` cubre §5 Parte 2. La suite (141059 comprobaciones), las 3 variantes de firmware y el puente dieron OK según Codex; no se recompiló porque el código no cambió.
  - **Documentación:** los 8 reemplazos de §6 son literales y no hay otros cambios. Tag V1.8 en `69a4097` y `ACTUALIZACION_V1_8.txt` presente.
  - **Observaciones (no bloquean), resueltas en el diseño, sin código:**
    - `lp` y `gap` incluyen la duración de un guardado en flash, así que en la prueba D no hay que guardar desde el menú.
    - La redacción sobre EN se corrigió: `begin()` deja el PWM en 0 y EN en alto.
  - **Pendiente:**
    - pruebas de hardware A a E del usuario;
    - H5 sigue siendo hipótesis hasta el ensayo E;
    - pull-down externo en EN antes de conectar los motores.
    - Después, integrar a master solo a pedido del usuario.
