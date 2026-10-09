# Diseño: fijar el identificador USB de la Pico 2 en el firmware

- Rama: `mejora/usb-id-pico2` (creada desde `master` @ `42ae7e3`)
- Diseño: Claude (2026-10-09). Implementa: Codex. Revisa: Claude.
- Este documento no modifica código.

## Problema (con evidencia)

`INICIAR SIMULADOR.cmd` falla con "No se encontro una Pico 2 con USB VID_2E8A/PID_000F", aunque la placa está conectada y el firmware responde bien.

- [Código] `Iniciar-Simulador.ps1:55` solo acepta puertos COM con `USB\VID_2E8A&PID_000F`.
- Windows (2026-10-09): la Pico 2 con el firmware del yoke aparece como **`USB\VID_239A&PID_CAFE&MI_00` (COM13)**. Consultada con `?` a 115200, responde `YOKE17SC8`, que es el firmware esperado por `Iniciar-Simulador.ps1:20`.
- El historial de dispositivos de Windows registra `2E8A:000F` en esta PC solo como "RP2350 Boot" / almacenamiento USB (modo BOOTSEL, 29/09/2026 21:08). Desde las 21:47 de ese día, el firmware del yoke siempre se presentó como `239A:CAFE`.
- Causa, en el paquete Arduino-Pico 6.1.1 (también 6.2.0):
  - `boards.txt:641-642` define `USBD_VID=0x2e8a` / `USBD_PID=0x000f`. Ese nombre lo usa solo el stack Pico SDK (`cores/rp2040/USB.cpp:51`).
  - Con **Adafruit TinyUSB** (el stack requerido por `Build-Pico2.ps1:13` y `LEEME md/LEEME_PICO2.md:20`), la librería busca otros nombres, `USB_VID`/`USB_PID` (`libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_Device.cpp:46-58`). Como no existen, usa su valor por defecto: `0x239A`/`0xCAFE`.
- Conclusión: el filtro del lanzador nunca coincidió con el firmware compilado con TinyUSB. No es una regresión de una versión del paquete.

## Objetivo

Que el firmware, compilado con Adafruit TinyUSB, se presente siempre como **VID 0x2E8A / PID 0x000F**, sin depender de la versión del paquete Arduino-Pico. Así lo encuentra `Iniciar-Simulador.ps1` sin cambiar el lanzador.

Descartado: aceptar `239A:CAFE` en el lanzador. Es el identificador por defecto de cualquier placa con TinyUSB y quitaría sentido al filtro ("no sondear otras placas").

## API usada (verificada en Arduino-Pico 6.1.1, Adafruit TinyUSB 3.7.7)

- `TinyUSBDevice.setID(vid, pid)` (`Adafruit_USBD_Device.cpp:112-115`) escribe `idVendor`/`idProduct` en el descriptor de dispositivo. Ese descriptor se entrega al host en cada enumeración (`tud_descriptor_device_cb`, `Adafruit_USBD_Device.cpp:340-342`).
- El núcleo inicializa TinyUSB **antes** de `setup()` (`cores/rp2040/main.cpp:147-148`, `TinyUSB_Device_Init(0)`). Por eso:
  - En condiciones normales, `setup()` corre microsegundos después y el host todavía no leyó el descriptor.
  - Si ya estuviera montado (`TinyUSBDevice.mounted()`), hay que desconectar y reconectar (`detach()`/`attach()`, `Adafruit_USBD_Device.h:139-143`) para que Windows lea el ID nuevo. Es el patrón documentado por Adafruit.
- `Adafruit_TinyUSB.h` ya está incluido bajo la misma condición en [INO] `MiYoke_PD.ino:14-17`.

## Ubicación exacta

Archivo `MiYoke_PD.ino` en la rama `mejora/usb-id-pico2` (basada en master, **sin** el comentario de la prueba de flujo). Terminadores CRLF.

Insertar el bloque **entre la línea 102 (`#endif` del bloque de `analogWrite`) y la línea 103 (`#if DEBUG_MOTION_TELEMETRY || SIMCONNECT_ADDITIONS_ENABLED`)**. Queda dentro de `setup()`, antes de `Serial.begin`.

Contexto actual [INO] `MiYoke_PD.ino:98-104`:

```cpp
void setup() {
#if defined(ARDUINO_ARCH_RP2040)
    analogWriteRange(255);
    analogWriteFreq(MOTOR_PWM_HZ);
#endif
#if DEBUG_MOTION_TELEMETRY || SIMCONNECT_ADDITIONS_ENABLED
    Serial.begin(115200); // USB CDC: no esperar la conexion del monitor
```

Después del cambio:

```cpp
void setup() {
#if defined(ARDUINO_ARCH_RP2040)
    analogWriteRange(255);
    analogWriteFreq(MOTOR_PWM_HZ);
#endif
#if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB)
    // TinyUSB usa 239A:CAFE por defecto; el lanzador busca la Pico 2 como 2E8A:000F.
    TinyUSBDevice.setID(0x2E8A, 0x000F);
    if (TinyUSBDevice.mounted()) { // ya enumerada con el ID anterior: re-enumerar
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }
#endif
#if DEBUG_MOTION_TELEMETRY || SIMCONNECT_ADDITIONS_ENABLED
    Serial.begin(115200); // USB CDC: no esperar la conexion del monitor
```

¿Por qué ahí?

- Es antes de `Serial.begin` y de cualquier uso del USB, lo más temprano posible sin mezclarlo con el bloque PWM.
- Usa la misma condición que el `#include` de la línea 14, así que con Pico SDK o en AVR no se compila y no cambia nada.
- Queda a varias líneas del comentario de `mejora/prueba-flujo` (arriba de `void setup()`), así que las dos ramas se integran sin conflicto.

## Texto exacto

Las 9 líneas del bloque "Después del cambio", desde `#if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB)` hasta su `#endif`, exactamente así:

- Sangría de 4 espacios dentro del bloque y de 8 dentro del `if`. Las directivas van en columna 0.
- Literales en mayúscula (`0x2E8A`, `0x000F`). Usar números explícitos, no `USBD_VID`/`USBD_PID`, para que coincidan siempre con el lanzador.
- Comentarios sin tildes, como el resto del archivo.
- CRLF.

## Criterios de aceptación

1. `git diff master...mejora/usb-id-pico2 -- MiYoke_PD.ino` muestra **solo las 9 líneas agregadas**, en la ubicación indicada, y 0 líneas eliminadas o modificadas. No hay reescritura de fines de línea.
2. Ningún otro archivo de código (`.h .hpp .c .cpp .ino .ps1 .cmd .py .vcxproj`) aparece en el diff. Opcional: una línea en `LEEME md/LEEME_PICO2.md`, sección "Implementacion actual", que diga que el firmware fija el USB en 2E8A:000F con `TinyUSBDevice.setID`.
3. Compila sin errores ni advertencias nuevas con `powershell -ExecutionPolicy Bypass -File Build-Pico2.ps1` (FQBN `rp2040:rp2040:rpipico2:usbstack=tinyusb,arch=arm`). Anotar en HANDOFF los bytes de programa y RAM, y la diferencia con la compilación anterior si está disponible.
4. `graphify-sync.cmd` ejecutado. `graphify-ino/` no se versiona.
5. Commit por nombre (`git add MiYoke_PD.ino` y, si se tocó, `git add "LEEME md/LEEME_PICO2.md"`).
6. `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Fijar identificador USB 2E8A 000F con TinyUSB para el lanzador"` imprime `Version Vx.y creada`, y existe `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt`.
7. "Última entrega" de HANDOFF.md completada con la versión, la compilación y lo que quedó sin verificar.
8. **Validación en hardware (usuario):**
   - Después de subir, el Administrador de dispositivos muestra el puerto COM de la Pico con `USB\VID_2E8A&PID_000F&MI_00`.
   - `INICIAR SIMULADOR.cmd` encuentra la placa y la identifica como `YOKE17SC8`.
   - Las fuerzas de SimConnect y el LCD funcionan igual que antes.

## Riesgos conocidos

- `2E8A:000F` es el mismo identificador que muestra la Pico 2 en modo BOOTSEL. No afecta al lanzador: en BOOTSEL no hay puerto COM, y el filtro exige clase `Ports` con COM (`Iniciar-Simulador.ps1:49-58`). Es también el identificador que usa Arduino-Pico con el stack Pico SDK.
- Windows puede instalar de nuevo el driver serie la primera vez y asignar otro número de COM. Es normal: el lanzador busca por identificador y firmware, no por número.
- Si alguna vez el host enumerara justo entre `TinyUSB_Device_Init` y `setID`, el `detach/attach` lo corrige. Sin ese bloque, quedaría `239A:CAFE` hasta reconectar.

## Qué no se toca

- `Iniciar-Simulador.ps1` y `tests/launcher_port_checks.ps1`: el filtro `2E8A:000F` queda igual.
- Ninguna otra línea de `MiYoke_PD.ino`: el include de la línea 14-17, `Serial.begin`, la identificación `YOKE17SC8`, `loop()`.
- `mis_defines.h`, constantes de seguridad (`MOTOR_PWM_HARD_LIMIT`, `SIMCONNECT_EXTRA_LIMIT`, límites PWM), `SimConnectBridge/`, scripts de build.
- No se integra a `master` hasta la validación en hardware (regla 4). `mejora/prueba-flujo` (V1.2) sigue pendiente de integrar por separado.
