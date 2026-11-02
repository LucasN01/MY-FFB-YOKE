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

Tarea: Migrar el firmware de Arduino al Pico SDK con CMake

(Claude, 2026-11-02.) Rama: `firmware/pico-sdk` (ya creada, con el diseno commiteado). Diseno cerrado y revisado: `LEEME md/DISENO_migracion_pico_sdk.md`. Las decisiones del usuario estan en su seccion **10.1** y son vinculantes (rutas de compilador, SDK y TinyUSB incluidas). Aplicar el diseno tal cual; si algo no coincide con el codigo real o con el entorno, no improvisar: anotar `BLOQUEO:` en "Ultima entrega" y terminar.

Cinco partes, una por sesion (HAL nueva, ~20 archivos nuevos, logica con estado, movimiento de todas las cabeceras y cambio del marcador de raiz de los scripts del flujo: no entra en una sesion). Cada parte cierra con commit por nombre y HANDOFF, y deja `probar.ps1` en verde segun sus criterios. `nueva-version.ps1` solo en la parte 5.

Para todas las partes:
- Leer del diseno solo las secciones que indica la parte (seccion 6) mas 10.1. Orientarse con `graphify query`/`explain`; en archivos grandes leer solo las lineas ubicadas con grep. Para [ARD] (`%LOCALAPPDATA%/Arduino15/packages/rp2040/hardware/rp2040/6.1.1`) y [SDK] (`D:/pico/pico-sdk`), solo las lineas citadas.
- Entorno (verificado por Claude): SDK `D:/pico/pico-sdk` (2.2.0), TinyUSB `D:/pico/tinyusb` (commit `e954302c1` = 0.20.1), compilador `D:/pico/toolchains/pqt-gcc-5.0.0` (GCC 16.1), picotool `D:/pico/picotool/picotool`, pioasm `D:/pico/sdk-tools/pioasm`, CMake `C:/Program Files/CMake/bin/cmake.exe`, ninja `C:/Users/norie/AppData/Local/Microsoft/WinGet/Packages/Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe/ninja.exe` (no esta en PATH). El directorio de build de CMake debe tener ruta corta (MAX_PATH): usar el de 5.2.
- No tocar `tesis/` (la actualiza Claude al aprobar), `graphify-ino/` ni constantes de seguridad. **Unica excepcion (aclaracion de Claude, 2026-11-02, resuelve el BLOQUEO de `0ef0964`):** en la parte 4, punto 6 del diseno, agregar `graphify-ino/` a `.gitignore` y borrar la carpeta `graphify-ino/` (no esta versionada; es la copia del `.ino` para el grafo, que deja de existir con `src/main.cpp`). No cambiar el tamano de pila (2 KiB, igual que hoy): el hallazgo de pila baja de "Ensayos del usuario" es otra tarea.
- Commits por nombre, nunca `git add .`/`-A`. Pruebas solo con `powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1 ...`; copiar en HANDOFF solo las lineas OK/FALLO.
- Al terminar cada parte: completar "Ultima entrega" (parte, commit, lineas OK/FALLO, desvios) y commitear HANDOFF aparte.

\### Parte 1 - logica pura de la HAL y pruebas de host
Seguir la seccion 6 "Parte 1" del diseno (alcance, lecturas, 5 grupos de pruebas, criterios y mensaje de commit). Linea base antes de editar: `probar.ps1` y `probar.ps1 -Pico`.

\### Parte 2 - HAL de destino y CMake base
Seccion 6 "Parte 2". `Build-PicoSdk.ps1` con el parametro `-TinyUsbPath` de 10.1. Precondicion: parte 1 commiteada y verde.

\### Parte 3 - cabeceras a `include/` y modulos con hardware sobre la HAL
Seccion 6 "Parte 3", con U1 = eliminar ramas AVR. Anotar en HANDOFF que entre la parte 3 y la 4 no se puede generar UF2. Ademas (revision de la parte 1): mover los `extern` de `tests/hal_checks.cpp:14-33` a un `tests/hal_fake.h` comun; en `tests/hal_fake.cpp` registrar `endTransmission` de longitud 0 como `i2cProbe` (no `i2cWrite`); eliminar (no portar) las ramas `WIRE_HAS_TIMEOUT` de `LcdTelemetry.h:135-147` y `YokeI2C.h:17-20`, que son codigo muerto con el Wire de arduino-pico y no tienen equivalente en `I2cBus`. Y (revision de la parte 2): en `src/hal/rp2350/i2c.cpp` `I2cBus::begin()` poner `transmitting_ = false; length_ = offset_ = 0;` (como `Wire.cpp:133-134`); en `src/hal/rp2350/usb_cdc.cpp` agregar `#ifndef YOKE_USB_GUARD` + `#error` antes del primer `#if YOKE_USB_GUARD`; en `src/hal/rp2350/pwm_adc.cpp` `pwmWrite` agregar `if (pin >= 30) return;` (como `analogWrite`); quitar de `CMakeLists.txt:25` el include provisorio de la raiz (queda solo `include/`, como 5.1); anotar en el diseno 2.1 que `pwmSetup` reinicia los slices aunque frecuencia y rango no cambien (inocuo: solo se llama en el arranque). Agregar una prueba host por cada uno de los tres cambios de codigo que sea comprobable con `hal_fake` o, si no lo es, decirlo en la entrega.

\### Parte 4 - `src/main.cpp`, 5 variantes y scripts
Seccion 6 "Parte 4" (puntos 1 y 4 en el mismo commit). `Build-Pico2.ps1` con `-TinyUsbPath`. UF2 = `MiYoke_PD.uf2` (U2). Restaurar `pico_set_printf_implementation` en cada ejecutable (en la parte 2 se difirio porque el SDK no lo acepta en una biblioteca STATIC). Ademas:
- SDK de SimConnect (el bloqueo de la parte 3): Prepar3D v4 ya no esta instalado en esta PC; el mismo SDK (FSX-SP2-XPACK, Win32) esta en Prepar3D v3. En cada `.vcxproj` de `SimConnectBridge/` que define `P3DSdkDir`, reemplazar el valor por defecto por dos lineas con condicion `Exists`: si `P3DSdkDir` esta vacio y existe `C:\Program Files\Lockheed Martin\Prepar3D v4
edist\Interface\FSX-SP2-XPACK
etail\inc\SimConnect.h`, usar la carpeta de v4; si sigue vacio, usar `C:\Program Files (x86)\Lockheed Martin\Prepar3D v3
edist\Interface\FSX-SP2-XPACK
etail`. Claude verifico que con la ruta de v3 `probar.ps1 -Bridge` da OK. `-Todo` debe incluir el puente en verde. Actualizar la ruta del SDK donde la mencionen los LEEME (en la parte 5).
- Restos del reemplazo de texto de la parte 3 (revision): `include/LcdTelemetry.h:5-8` deja un solo `<stdio.h>` y un solo `<string.h>`; volver a los textos originales en `include/LcdTelemetry.h:273` ("clavados en LOW"), `include/CalibrationControls.h:69` ("EEPROM solapada") e `include/mis_defines.h:171` ("commit de EEPROM"); mover el `#error` de `YOKE_USB_GUARD` de `src/hal/rp2350/usb_cdc.cpp:34-36` a nivel de archivo, despues de `#include "mis_defines.h"`; completar U1 quitando la rama `#ifdef __AVR__` de `include/SimTelemetry.h:19` (conservando la rama que usa la Pico).
- **Reanudacion de la parte 4 (Claude, 2026-11-02):** el turno anterior de Codex termino con error durante `probar.ps1 -Todo` y dejo los cambios de la parte 4 en el arbol sin commit: continuar sobre ellos (no descartarlos). Claude corrio `probar.ps1 -Todo -PruebaFalla` sobre ese arbol: OK simconnect_checks, comparar_ensayo, hal_checks (694), launcher y puente SimConnect; FALLO las 5 variantes Pico con un unico error, `pico/platform.h:20: #error pico/platform.h should not be included directly; include pico.h instead`. Era un error del diseno (ya corregido en sus secciones 3 y 6): en `src/main.cpp:19` usar `#include <pico.h>`. Si Ninja se queda sin actividad dentro del sandbox (paso en las partes 2 y 3), repetir esa compilacion fuera del sandbox en lugar de esperar.
- Primera compilacion de las cabeceras de `include/` con arm-none-eabi: si falta algun `<stdint.h>`/`<stddef.h>`/`<string.h>` que antes llegaba por `Arduino.h`, agregarlo en la cabecera que lo usa. Despues del commit: `graphify update .`.

\### Parte 5 - documentacion, reglas de agentes y version
Seccion 6 "Parte 5" y seccion 9, sin `tesis/`. Incluye U4 (`CMakeLists.txt|\.cmake$` generan version en `nueva-version.ps1`, CLAUDE.md y AGENTS.md) y reemplazar en CLAUDE.md/AGENTS.md la regla "Arduino exige que esten juntos" y la estructura de carpetas, y en este HANDOFF las reglas 6 y 8 (ya no hay `.ino` ni `graphify-ino/`). Version: `powershell -ExecutionPolicy Bypass -File herramientas/flujo/nueva-version.ps1 -Mensaje "Firmware migrado de Arduino-Pico al Pico SDK con CMake, HAL propia y estructura src/include"`; verificar "Version Vx.y creada" y el ACTUALIZACION. Ademas, antes de la documentacion (revision de la parte 4; son codigo, entran en el commit de la version): (a) `herramientas/compilacion/Build-Pico2.ps1:162`: la guarda de tamano (`.bin` < 0x3FE000) debe correr ANTES del bucle de copia a `build-pico2*/`, comprobando `$BuildRoot/$target.bin`; si falla, no se copia nada (que no quede un UF2 que pise el sector de calibracion); ajustar la seccion 5.2 del diseno en el mismo sentido. (b) `Build-Pico2.ps1:143`: si ya existe `CMakeCache.txt` y `-SdkPath`, `-ToolchainBin` o `-TinyUsbPath` difieren de los guardados en la cache, reconfigurar (borrar la cache del directorio de build) en lugar de ignorarlos. (c) `include/mis_defines.h:485`: comentario "2=noInterrupts+for(;;)" -> "2=hal::irqSave+for(;;)". (d) `include/LcdTelemetry.h:291`: corregir la codificacion rota del comentario ("est�"). Despues `probar.ps1 -Todo -PruebaFalla` OK. En "Ultima entrega": la lista de ensayos con hardware H0-H13 que debe hacer el usuario, en orden (H0 respaldo del sector de calibracion y H13 osciloscopio antes de mover el yugo).

\## Última entrega

- Parte 5 completada por Claude a pedido explicito del usuario ("Hacela vos ahora"), 2026-11-02, rama `firmware/pico-sdk`. Commit: `d6d8557`. **Version V2.7** creada por `nueva-version.ps1` ("Version V2.7 creada"; etiqueta V2.7 sobre `d6d8557`; `../MiYoke_PD_VERSIONES/V2_7/ACTUALIZACION_V2_7.txt` verificado).
- Codigo (revision de la parte 4): `Build-Pico2.ps1` comprueba el `.bin` (< 0x3FE000) antes de copiar a `build-pico2*/` y reconfigura si la cache tiene otro SDK/TinyUSB/compilador (probado: mismas rutas no reconfigura; otra ruta de TinyUSB reconfigura); comentarios de `mis_defines.h` (`hal::irqSave`) y `LcdTelemetry.h` (codificacion). U4: `nueva-version.ps1` cuenta `CMakeLists.txt` y `*.cmake`.
- Documentacion (seccion 9, via subagente Technical Writer): `LEEME_PICO2.md`, `INVENTARIO_CODIGO.md`, `ENSAYOS.md`, `MENU_CALIBRACION.md`, `LEEME_POTENCIOMETROS.md`, `LEEME_AS5600.md`, `LEEME-LANZADOR.md`, `LEEME_ENCODERS.md`/`LEEME_ETAPAS.md` (linea historica), `AJUSTE_FUERZAS_Y_MOVIMIENTO.md`, `LEEME_SIMCONNECT.md`, `.claude/skills/depuracion-sistematica/SKILL.md`; diseno 5.2. CLAUDE.md/AGENTS.md: estructura de carpetas, limitaciones del grafo, versionado y "no invocar cmake/ninja". HANDOFF: reglas 6 y 8. Excepciones historicas (registros fechados) sin tocar; `MOBIFLIGHT_PICO_Y_UNO.md:35` nombra un UF2 V2.3 historico.
- Pruebas: `probar.ps1 -Todo -PruebaFalla`, codigo 0: OK simconnect_checks (143769), OK comparar_ensayo, OK hal_checks (694), OK launcher_port_checks (19), OK Pico 2 configurado 65460/8608, encoders 66756/8924, potenciometros 66316/8708, AS5600 67404/8772, prueba falla 66356/8708 (flash/RAM), OK puente SimConnect.
- Pendiente: tesis (Claude, al aprobar la revision) y ensayos con hardware del usuario.
- **Ensayos con hardware (usuario), en este orden** (detalle y criterios en `LEEME md/DISENO_migracion_pico_sdk.md` seccion 7):
  1. H0 con el firmware actual V2.6 (Arduino), antes de cargar nada: anotar valores del menu, PWM en GP6..GP9 para un valor conocido, COM y descriptores USB, `CICLOS.ps1` de referencia; placa en BOOTSEL y `D:/pico/picotool/picotool/picotool.exe save -r 0x103FE000 0x103FF000 D:/pico/calib_V2_6.bin`.
  2. H13 con `build-pico2/MiYoke_PD.uf2`, fuente del puente H conectada y motor desacoplado: osciloscopio en EN (GP10/GP11) y PWM (GP6..GP9) en encendido, RUN, BOOTSEL, toque a 1200 y watchdog. Ningun pulso PWM con EN alto.
  3. H8 calibracion conservada; H1 PWM 489,99 Hz; H2 sentido de motores; H7 USB (mismo COM, 30 min con el puente); H6 LCD; H11 toque a 1200; H10 tiempo de ciclo y pila; H9 watchdog/falla (variante prueba-falla, EN y PWM en 0 durante la falla); H3/H4/H5 segun sensor; H12 `ENSAYO.ps1` + `comparar_ensayo`.

\## Revisión

- Pendiente: Claude debe revisar la parte 2 (`8688f0a`). La parte 1 estaba aprobada al comenzar (registro anterior en `818c6ba`).
- Revisor: Claude con el subagente Code Reviewer (2026-11-02). **Parte 2 (`8688f0a`): APROBADA, 0 bloqueantes.**
  - EEPROM en flash (0x3FE000, erase+program con IRQ deshabilitadas), USB (init, IRQ de software, 1200 baudios, DTR, serie por `pico_get_unique_board_id`), I2C (sonda, recuperacion, codigos 0/2/4/5), PWM sin pulso al arrancar, watchdog y guarda D13 verificados contra arduino-pico 6.1.1. Desvio de printf aceptado (restaurar en parte 4).
  - Suite del revisor (`probar.ps1 -Sdk`): OK simconnect_checks, OK comparar_ensayo, OK hal_checks (394), OK launcher_port_checks, OK SDK myk_hal (RP2350).
  - Sugerencias trasladadas a las partes 3 y 4 del plan.
- Revisor: Claude con el subagente Code Reviewer (2026-11-02). **Parte 3 (`2c99677`): APROBADA, 0 bloqueantes.**
  - 17 modelos puros movidos sin cambio de contenido (R100). Modulos con hardware: cambio mecanico, mismo orden de llamadas y mismas constantes (MOTOR_PWM_HARD_LIMIT 220, pines, direcciones I2C, tiempos). Las 6 sugerencias previas incorporadas. Las dos aserciones corregidas por Codex siguen al codigo original.
  - BLOQUEO del puente: no es de la migracion; Prepar3D v4 fue desinstalado. Claude verifico `probar.ps1 -Bridge` OK con `P3DSdkDir` de Prepar3D v3; la parte 4 lo deja automatico.
  - Suite del revisor (`probar.ps1 -Sdk`): OK simconnect_checks, OK comparar_ensayo, OK hal_checks (694), OK launcher_port_checks, OK SDK myk_hal (RP2350).
  - Sugerencias trasladadas a la parte 4 del plan.
- Revisor: Claude con el subagente Code Reviewer (2026-11-02). **Parte 4 (`d80d804` R100 + `2f44614`): APROBADA, 0 bloqueantes.**
  - `src/main.cpp` frente al `.ino`: solo los cambios de la tabla de 3; sin cambios de constantes, tiempos, orden de etapas ni watchdog. Arranque del motor: PWM en 0 antes de EN, sin pulso. `newlib_locks.cpp` (agregado por Claude) equivale a `lock.cpp` de arduino-pico sin FreeRTOS; ninguna ISR usa printf/malloc. CMake con los defines de las 5 variantes, scripts y `.vcxproj` correctos.
  - Suite del revisor (`probar.ps1 -Todo -PruebaFalla`): OK simconnect_checks (143769), comparar_ensayo, hal_checks (694), launcher (19), las 5 variantes Pico y el puente.
  - Sugerencias pasadas a la parte 5 (guarda de tamano antes de copiar, reconfigurar cache, dos comentarios). Para H13: `hal::usb::init()` corre antes de poner el motor en 0, igual que el core Arduino (GPIO en estado de reset); confirmarlo con osciloscopio.

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
