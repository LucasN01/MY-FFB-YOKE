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
- Leer del diseño: §1.4, §3.1, §3.2 y §5 "Escritorio / Parte 1".
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
  6. Ejecutar `graphify-sync.cmd` (se editó el `.ino`) y `graphify update .`.
- Pruebas:
  - `probar.ps1 -Pico -Potentiometers -Encoders` (incluye la suite C++). Tiene que dar todo OK.
  - `powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1`: anotar su única línea.
- Commit por nombre: `UsbGuard.h LoopSupervisor.h mis_defines.h MiYoke_PD.ino tests/health_checks.h tests/simconnect_checks.cpp "LEEME md/LEEME-LANZADOR.md" "LEEME md/LEEME_SIMCONNECT.md" "LEEME md/EFECTOS_SIMCONNECT.md" "LEEME md/MODELO_TURBULENCIA.md" "LEEME md/CALCULO_PICO_SC8.md" tesis/capitulos/01_arquitectura_sc8.tex HANDOFF.md`.
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

\- Pendiente (Parte 1 de la tarea de recuperación del enlace USB).

\## Revisión

\- Pendiente.
