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

Tarea: Migrar el firmware de Arduino al Pico SDK con CMake

(Claude, 2026-11-02.) Rama: `firmware/pico-sdk` (ya creada, con el diseno commiteado). Diseno cerrado y revisado: `LEEME md/DISENO_migracion_pico_sdk.md`. Las decisiones del usuario estan en su seccion **10.1** y son vinculantes (rutas de compilador, SDK y TinyUSB incluidas). Aplicar el diseno tal cual; si algo no coincide con el codigo real o con el entorno, no improvisar: anotar `BLOQUEO:` en "Ultima entrega" y terminar.

Cinco partes, una por sesion (HAL nueva, ~20 archivos nuevos, logica con estado, movimiento de todas las cabeceras y cambio del marcador de raiz de los scripts del flujo: no entra en una sesion). Cada parte cierra con commit por nombre y HANDOFF, y deja `probar.ps1` en verde segun sus criterios. `nueva-version.ps1` solo en la parte 5.

Para todas las partes:
- Leer del diseno solo las secciones que indica la parte (seccion 6) mas 10.1. Orientarse con `graphify query`/`explain`; en archivos grandes leer solo las lineas ubicadas con grep. Para [ARD] (`%LOCALAPPDATA%/Arduino15/packages/rp2040/hardware/rp2040/6.1.1`) y [SDK] (`D:/pico/pico-sdk`), solo las lineas citadas.
- Entorno (verificado por Claude): SDK `D:/pico/pico-sdk` (2.2.0), TinyUSB `D:/pico/tinyusb` (commit `e954302c1` = 0.20.1), compilador `D:/pico/toolchains/pqt-gcc-5.0.0` (GCC 16.1), picotool `D:/pico/picotool/picotool`, pioasm `D:/pico/sdk-tools/pioasm`, CMake `C:/Program Files/CMake/bin/cmake.exe`, ninja `C:/Users/norie/AppData/Local/Microsoft/WinGet/Packages/Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe/ninja.exe` (no esta en PATH). El directorio de build de CMake debe tener ruta corta (MAX_PATH): usar el de 5.2.
- No tocar `tesis/` (la actualiza Claude al aprobar), `graphify-ino/` ni constantes de seguridad. No cambiar el tamano de pila (2 KiB, igual que hoy): el hallazgo de pila baja de "Ensayos del usuario" es otra tarea.
- Commits por nombre, nunca `git add .`/`-A`. Pruebas solo con `powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1 ...`; copiar en HANDOFF solo las lineas OK/FALLO.
- Al terminar cada parte: completar "Ultima entrega" (parte, commit, lineas OK/FALLO, desvios) y commitear HANDOFF aparte.

\### Parte 1 - logica pura de la HAL y pruebas de host
Seguir la seccion 6 "Parte 1" del diseno (alcance, lecturas, 5 grupos de pruebas, criterios y mensaje de commit). Linea base antes de editar: `probar.ps1` y `probar.ps1 -Pico`.

\### Parte 2 - HAL de destino y CMake base
Seccion 6 "Parte 2". `Build-PicoSdk.ps1` con el parametro `-TinyUsbPath` de 10.1. Precondicion: parte 1 commiteada y verde.

\### Parte 3 - cabeceras a `include/` y modulos con hardware sobre la HAL
Seccion 6 "Parte 3", con U1 = eliminar ramas AVR. Anotar en HANDOFF que entre la parte 3 y la 4 no se puede generar UF2. Ademas (revision de la parte 1): mover los `extern` de `tests/hal_checks.cpp:14-33` a un `tests/hal_fake.h` comun; en `tests/hal_fake.cpp` registrar `endTransmission` de longitud 0 como `i2cProbe` (no `i2cWrite`); eliminar (no portar) las ramas `WIRE_HAS_TIMEOUT` de `LcdTelemetry.h:135-147` y `YokeI2C.h:17-20`, que son codigo muerto con el Wire de arduino-pico y no tienen equivalente en `I2cBus`. Y (revision de la parte 2): en `src/hal/rp2350/i2c.cpp` `I2cBus::begin()` poner `transmitting_ = false; length_ = offset_ = 0;` (como `Wire.cpp:133-134`); en `src/hal/rp2350/usb_cdc.cpp` agregar `#ifndef YOKE_USB_GUARD` + `#error` antes del primer `#if YOKE_USB_GUARD`; en `src/hal/rp2350/pwm_adc.cpp` `pwmWrite` agregar `if (pin >= 30) return;` (como `analogWrite`); quitar de `CMakeLists.txt:25` el include provisorio de la raiz (queda solo `include/`, como 5.1); anotar en el diseno 2.1 que `pwmSetup` reinicia los slices aunque frecuencia y rango no cambien (inocuo: solo se llama en el arranque). Agregar una prueba host por cada uno de los tres cambios de codigo que sea comprobable con `hal_fake` o, si no lo es, decirlo en la entrega.

\### Parte 4 - `src/main.cpp`, 5 variantes y scripts
Seccion 6 "Parte 4" (puntos 1 y 4 en el mismo commit). `Build-Pico2.ps1` con `-TinyUsbPath`. UF2 = `MiYoke_PD.uf2` (U2). Restaurar `pico_set_printf_implementation` en cada ejecutable (en la parte 2 se difirio porque el SDK no lo acepta en una biblioteca STATIC). Despues del commit: `graphify update .`.

\### Parte 5 - documentacion, reglas de agentes y version
Seccion 6 "Parte 5" y seccion 9, sin `tesis/`. Incluye U4 (`CMakeLists.txt|\.cmake$` generan version en `nueva-version.ps1`, CLAUDE.md y AGENTS.md) y reemplazar en CLAUDE.md/AGENTS.md la regla "Arduino exige que esten juntos" y la estructura de carpetas, y en este HANDOFF las reglas 6 y 8 (ya no hay `.ino` ni `graphify-ino/`). Version: `powershell -ExecutionPolicy Bypass -File herramientas/flujo/nueva-version.ps1 -Mensaje "Firmware migrado de Arduino-Pico al Pico SDK con CMake, HAL propia y estructura src/include"`; verificar "Version Vx.y creada" y el ACTUALIZACION. En "Ultima entrega": la lista de ensayos con hardware H0-H13 que debe hacer el usuario, en orden (H0 respaldo del sector de calibracion y H13 osciloscopio antes de mover el yugo).

\## Última entrega

- Parte 2 completada (Codex, 2026-11-02), rama `firmware/pico-sdk`. Commit de implementacion: `8688f0a` (`Pico SDK parte 2: HAL de destino y CMake base`). Esta entrega se commitea aparte.
- Archivos nuevos: `CMakeLists.txt`, `pico_sdk_import.cmake` (copia identica al SDK, SHA256 verificado), `herramientas/compilacion/Build-PicoSdk.ps1` y `src/hal/rp2350/{time_gpio,pwm_adc,i2c,watchdog,eeprom_flash,usb_cdc,usb_descriptors}.cpp`, `src/hal/rp2350/tusb_config.h`. Modificado: `herramientas/compilacion/probar.ps1` (switch provisorio `-Sdk`).
- Implementada la HAL de destino: tiempo, GPIO/IRQ y drenaje abierto, PWM/ADC, I2C con sonda y recuperacion, watchdog/reset, EEPROM y USB CDC/descriptores. Los siete `.cpp` tienen D13 como primera/ultima linea; asserts de mascaras del SDK, flash de 4 MiB y offset EEPROM `0x3FE000`. Se conservan las interfaces de la parte 1.
- CMake compila solo `myk_hal` STATIC para pico2/rp2350-arm-s con C++17, opciones D9 y definicion D14. El script usa las rutas U3/U6, acepta `-TinyUsbPath` y normaliza `-ToolchainBin` tanto desde `bin/` como desde la raiz del compilador. No genera UF2.
- Detalles de integracion: el include de la raiz es provisorio porque `mis_defines.h` se mueve en la parte 3. `pico_set_printf_implementation` se reserva para los ejecutables de la parte 4: el SDK rechaza aplicarlo a una biblioteca estatica. Se mantienen las opciones, definiciones y bibliotecas especificadas para la parte 2.
- Documentacion actualizada: uso de `-Sdk` en el comentario de `probar.ps1` y esta entrega. La documentacion del firmware migrado sigue en la parte 5; el firmware Arduino aun no cambio. No se abrio ni modifico la tesis.
- Verificacion: suites de host ejecutadas con `probar.ps1 -Sdk -Pico`; el primer paso SDK se interrumpio porque Ninja quedo sin actividad en el try_compile dentro del sandbox. Fuera del sandbox se detecto y retiro la llamada de printf sobre el objetivo STATIC; se descarto la cache incompleta de esa configuracion y `probar.ps1 -Sdk` completo con codigo 0. El paso Pico de la primera corrida termino OK (sin cambios de firmware posteriores). Resultados finales:
  - OK suite simconnect_checks - PASS: 143769 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
  - OK comparar_ensayo
  - OK suite hal_checks - PASS hal_checks (394 casos)
  - OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
  - OK SDK myk_hal (RP2350) - HAL RP2350 compilada: myk_hal. No se genero ni cargo firmware.
  - OK Pico 2 (configurado) - Sketch uses 105404 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22920 bytes (4%) of dynamic memory, leaving 501368 bytes for local variables. Maximum is 524288 bytes.
- `git diff --cached --check` sin errores; `graphify update .` ejecutado tras los cambios. `graphify-ino/` ya estaba sin seguimiento al comenzar y se dejo intacto. Sin carga de firmware ni ensayos con hardware.
- Version: no corresponde en esta parte; `nueva-version.ps1` se ejecuta solo en la parte 5, segun el plan. Sin bloqueos pendientes.
- Sigue: revision de Claude de la parte 2 y, al aprobar, parte 3 (cabeceras a `include/` y modulos sobre la HAL), incluidas las sugerencias de revision de parte 1 ya incorporadas al Plan para Codex.

\## Revisión

- Pendiente: Claude debe revisar la parte 2 (`8688f0a`). La parte 1 estaba aprobada al comenzar (registro anterior en `818c6ba`).
- Revisor: Claude con el subagente Code Reviewer (2026-11-02). **Parte 2 (`8688f0a`): APROBADA, 0 bloqueantes.**
  - EEPROM en flash (0x3FE000, erase+program con IRQ deshabilitadas), USB (init, IRQ de software, 1200 baudios, DTR, serie por `pico_get_unique_board_id`), I2C (sonda, recuperacion, codigos 0/2/4/5), PWM sin pulso al arrancar, watchdog y guarda D13 verificados contra arduino-pico 6.1.1. Desvio de printf aceptado (restaurar en parte 4).
  - Suite del revisor (`probar.ps1 -Sdk`): OK simconnect_checks, OK comparar_ensayo, OK hal_checks (394), OK launcher_port_checks, OK SDK myk_hal (RP2350).
  - Sugerencias trasladadas a las partes 3 y 4 del plan.

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
