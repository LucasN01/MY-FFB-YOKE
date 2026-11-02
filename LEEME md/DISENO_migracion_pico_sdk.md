# Diseno: migracion del firmware de Arduino-Pico al Pico SDK oficial (C++, CMake + Ninja)

- Rama: `firmware/pico-sdk` (creada desde `master` @ `b4a53fa`).
- Diseno: Claude / Embedded Firmware Engineer (2026-11-02). Implementa: Codex (5 partes, ver 6). Revisa: Claude.
- Revisado por Master Plan Architect (2026-11-02): ver seccion 12 "Revision del plan".
- Este documento no modifica codigo.
- Etiquetas de fuente: [ARD] = `%LOCALAPPDATA%\Arduino15\packages\rp2040\hardware\rp2040\6.1.1\` (core
  Arduino-Pico 6.1.1, Pico SDK interno 2.3.0 cuyo `lib/tinyusb` es 0.18.0 identico al de 2.2.0, pero el
  USB del firmware lo compila la biblioteca Adafruit TinyUSB, que trae TinyUSB 0.20.1; compilador
  `pqt-gcc 5.0.0-9576866` = GCC 16.1.0);
  [SDK] = `D:\pico\pico-sdk` (Pico SDK 2.2.0, tag `2.2.0` commit `a1438df`, TinyUSB 0.18.0 commit
  `86ad6e5`); compilador instalado: Arm GNU Toolchain 12.2.MPACBTI-Rel1; picotool v2.2.0-a4;
  [REPO] = este repositorio.
  Todas las referencias archivo:linea se verificaron el 2026-11-02 sobre esas copias.

---

## 1. Objetivo

Dejar de compilar con el core Arduino-Pico (FQBN `rp2040:rp2040:rpipico2:usbstack=tinyusb,arch=arm`,
`arduino-cli` desde `herramientas/compilacion/Build-Pico2.ps1`) y compilar con el Pico SDK oficial en
C++ con CMake + Ninja, con:

1. `MiYoke_PD.ino` convertido en `src/main.cpp` (sin preprocesado `.ino`; con `int main()` propio).
2. Estructura `src/` + `include/` (decision del usuario) en lugar de los `.h` en la raiz.
3. **Mismo comportamiento observable**: misma logica de fuerzas (fuente sin cambios), mismo PWM
   (pines, frecuencia, resolucion efectiva), mismo I2C (velocidades, tiempos de espera, recuperacion),
   mismo USB (VID/PID, cadenas, numero de serie, clase, tamanos de buffer, deteccion por DTR),
   misma persistencia (calibracion guardada se conserva), mismo watchdog y diagnostico de arranque,
   mismo lazo de 2 ms.
4. Scripts (`probar.ps1`, lanzador, ensayos) con la misma interfaz y la misma salida de una linea.

No es objetivo mejorar nada en esta tarea (PIO para encoders, DMA, multinucleo, etc.). Cada mejora
posterior va en un diseno propio.

---

## 2. Decisiones

| # | Decision | Fundamento |
|---|----------|-----------|
| D1 | USB con **TinyUSB directo** (`tinyusb_device` del SDK), sin `pico_stdio_usb`. Descriptores propios que reproducen byte a byte los de Adafruit TinyUSB. `tud_task()` en IRQ de software disparada por `USBCTRL_IRQ` + llamada desde el lazo principal, ambas con `mutex_try_enter`, igual que hoy. | `pico_stdio_usb` cambia el descriptor (agrega interfaz de reset, otras cadenas) y usa su propio mutex/alarma [SDK `src/rp2_common/pico_stdio_usb/stdio_usb.c:60-138`]. El patron actual esta en [ARD `libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/Adafruit_TinyUSB_rp2040.cpp:81-117`] y en el `yield()` propio de [REPO `MiYoke_PD.ino:58-65`] (correccion H5, `DISENO_recuperacion_enlace_usb.md` 1.2). |
| D2 | Capa `include/hal/` con **API de estilo Arduino en espacio de nombres `hal`** (mismos nombres de metodo para Wire/Serial) para que la migracion de modulos sea mecanica y revisable linea a linea. | Minimiza el diff de modulos que ya funcionan; permite compilar los modulos en el host con una HAL falsa. |
| D3 | Cada funcion HAL **reproduce la semantica de Arduino-Pico 6.1.1** (no la reimplementa "mejor"). Se reescribe a partir del comportamiento documentado aqui; no se copia codigo LGPL literal. | Objetivo 3. Las diferencias intencionales se listan en 2.1. |
| D4 | Persistencia: **mismo sector y mismo formato** que la emulacion EEPROM: offset de flash **`0x3FE000`** (direccion XIP **`0x103FE000`**, **penultimo** sector de 4 KiB de la flash de 4 MiB; el ultimo, `0x3FF000`, Arduino no lo usa), imagen de 256 bytes en RAM, `commit` = borrar 4096 + programar 256 con interrupciones deshabilitadas. **No se pierde la calibracion.** | [ARD `boards.txt:663`] `eeprom_start=272621568` = **`0x103FE000`** y `flash_length=4186112` = `0x3FE000` (l. 662, opcion de menu por defecto `4194304_0`, la primera, porque el FQBN no fija `flash`); [REPO `build-pico2/MiYoke_PD.ino.map:3340`] `PROVIDE (_EEPROM_start = 0x103fe000)`; [ARD `lib/rp2350/memmap_default.ld:36`]; [ARD `libraries/EEPROM/src/EEPROM.cpp:38-60,110-131`]; `CalibrationControls.h` llama `EEPROM.begin(256)`. (La version anterior de este diseno decia `0x103FF000`: error de conversion que habria leido un sector borrado y perdido la calibracion.) |
| D5 | Encoders: se conserva **cuadratura x4 por IRQ GPIO** (`PicoEncoder.h`, misma tabla de signo). No se usa PIO. | El firmware RP2040/RP2350 ya no usa la biblioteca PJRC `Encoder` (`AxisEncoders.h:16-18` elige `PicoEncoder`). PIO cambiaria el muestreo; queda como mejora futura. |
| D6 | Reloj del sistema 150 MHz por defecto del SDK (no se llama `set_sys_clock_khz`). | [ARD `boards.txt:729-730`] F_CPU 150 MHz; [ARD `cores/rp2040/main.cpp:104-114`] solo cambia el reloj si F_CPU != 150 MHz. El SDK para RP2350 arranca a 150 MHz. |
| D7 | Pila del nucleo 0: **2 KiB** (`PICO_STACK_SIZE` por defecto 0x800), en SCRATCH_Y, igual que hoy. Se conservan los simbolos `__StackBottom`/`__StackTop`. | [REPO `build-pico2/MiYoke_PD.ino.map:6458-6480`]: `.stack` 0x800 en 0x20081000, `__StackBottom=0x20081800`; [SDK `src/rp2_common/pico_crt0/crt0.S:604`], [SDK `src/rp2_common/pico_crt0/rp2350/memmap_default.ld`]. |
| D8 | Compilacion: un solo `CMakeLists.txt` en la raiz con **5 ejecutables** (uno por variante) en **un unico directorio de build corto** (`%TEMP%\myk` por defecto, configurable con `MIYOKE_BUILD_ROOT`). `Build-Pico2.ps1` compila solo el objetivo pedido y copia `.uf2/.elf/.bin/.map` a `build-pico2*/` de la raiz. | MAX_PATH de Windows (el usuario ya vio fallar un build con ruta larga); el flujo actual ya compila en `%TEMP%` (`Build-Pico2.ps1:19`). |
| D9 | Opciones de compilador alineadas con el build actual: `-Os -g`, C++ `gnu++17`, `-fno-exceptions -fno-rtti`, `-Werror=return-type -Wno-psabi`, printf de newlib (`pico_set_printf_implementation(... compiler)`), sin stdio. | [ARD `platform.txt:46-68,100-108`], [ARD `boards.txt:719`] (cortex-m33, softfp, cmse: lo mismo que pone el SDK para `rp2350-arm-s`). `-g` hace falta para `FALLA.ps1` (addr2line). `gnu++17` = mismo estandar que las pruebas de host (`tests/simconnect_checks.vcxproj:11`). |
| D10 | Macro de plataforma nueva `YOKE_TARGET_RP2350=1` (la define CMake solo en el firmware). Reemplaza a `ARDUINO_ARCH_RP2040`, `USE_TINYUSB`, `ARDUINO`. Las ramas AVR (Pro Micro) se eliminan (ver decision de usuario U1). | Sin Arduino esos macros no existen y `mis_defines.h:127,171` elegiria los pines AVR. |
| D11 | Nombre de los artefactos: `MiYoke_PD.uf2`, `MiYoke_PD.elf`, `MiYoke_PD.bin`, `MiYoke_PD.map` en las mismas carpetas `build-pico2`, `build-pico2-encoders`, `build-pico2-potentiometers`, `build-pico2-as5600`, `build-pico2-prueba-falla` (ver U2). | Ya no hay `.ino`; `ARRANQUE.ps1:10` y `ENSAYOS.md` se actualizan. |
| D12 | Marcador de raiz de los scripts: `src/main.cpp` en lugar de `MiYoke_PD.ino`. | 11 scripts y `graphify-sync.cmd` usan hoy `MiYoke_PD.ino` (lista en 6, parte 4, punto 4). |
| D13 | Mientras exista el `.ino` (partes 1-3), **todo `.cpp` dentro de `src/` va envuelto en `#if defined(YOKE_TARGET_RP2350)` ... `#endif` como primera y ultima linea** (antes de cualquier `#include`). | `arduino-cli` compila recursivamente la subcarpeta `src/` del sketch: sin la guarda, `usb_descriptors.cpp`/`usb_cdc.cpp` definirian `tud_descriptor_*_cb`/`tud_cdc_line_state_cb` duplicados con Adafruit TinyUSB y `probar.ps1 -Pico` fallaria en la parte 2. La guarda se conserva despues (inocua). |
| D14 | Paridad de arranque con el core: definicion de compilacion `PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64` en los objetivos de firmware. | [ARD `lib/platform_def.txt`] la define en 64; el SDK usa 6 por defecto ([SDK `src/rp2_common/hardware_xosc/include/hardware/xosc.h:16-18`]). Las bibliotecas del SDK son INTERFACE y se compilan con las definiciones del objetivo. |

### 2.1 Diferencias de comportamiento intencionales (todas inocuas, se documentan)

1. **Sin re-enumeracion USB al arrancar.** Hoy el dispositivo arranca con `239A:CAFE` y `setup()` lo
   cambia a `2E8A:000F` con `detach()`/`delay(10)`/`attach()` si ya estaba montado
   ([REPO `MiYoke_PD.ino:228-236`], `DISENO_usb_id_pico2.md`). Con descriptores propios el dispositivo
   enumera desde el primer momento como `2E8A:000F`. Se elimina el `delay(10)`.
2. **PWM a 490 Hz desde la primera escritura.** Hoy `rollMotor.begin()` escribe 0 con la configuracion
   por defecto (1000 Hz) y recien despues `setup()` fija 255/490 Hz ([REPO `MiYoke_PD.ino:205-224`]).
   La HAL configura 490 Hz antes de `begin()`. En ambos casos la salida es nivel 0 (siempre baja).
3. **`write` USB sin bucle de reintento, pero con la misma condicion de DTR.** `Adafruit_USBD_CDC::write`
   itera `while (remain && tud_cdc_n_connected(...))` con `yield()` mientras falte espacio
   ([ARD `.../Adafruit_USBD_CDC.cpp:240-258`]): **si DTR esta en 0 no escribe nada y devuelve 0**. Todas
   las escrituras del firmware se hacen despues de comprobar `availableForWrite() >= n` (verificado en
   `MiYoke_PD.ino`), asi que el bucle nunca itera. La HAL hace: `if (!tud_cdc_connected()) return 0;`
   y luego una sola `tud_cdc_write` y devuelve lo escrito. La comprobacion de DTR es obligatoria: la
   respuesta `YOKE17SC8` a `?` (`MiYoke_PD.ino:292`) y `YOKE_USB_STRESS` escriben sin mirar DTR, y
   TinyUSB 0.18 deja la FIFO TX "sobrescribible" con DTR=0 ([SDK `lib/tinyusb/src/class/cdc/cdc_device.c:303,418`]):
   sin la guarda se encolarian bytes viejos que saldrian al abrir el puerto. (Con `UsbLock` tomado, el
   bucle original nunca podria progresar: es un riesgo latente que desaparece.)
4. **Rama `WIRE_HAS_TIMEOUT` eliminada** en `LcdTelemetry.h`/`YokeI2C.h`: el core RP2040 no define ese
   macro (verificado: no aparece en [ARD `libraries/Wire`] ni en `cores/rp2040`), es codigo muerto.
5. **Compilador**: hoy GCC 16.1.0 (`pqt-gcc 5.0.0-9576866`, verificado con `--version`); despues el
   que fije U3 (por defecto Arm GNU Toolchain 12.2.MPACBTI-Rel1). La fuente de los modelos no cambia,
   pero la generacion de codigo de punto flotante (p. ej. fusion multiplicacion-suma, `-ffp-contract`)
   puede variar en el ultimo bit. Ver riesgo R3 y decision U3.
6. **TinyUSB 0.20.1 -> 0.18.0** (version incluida en SDK 2.2.0 **y tambien en la copia de SDK 2.3.0 del
   core**: `diff -r --strip-trailing-cr` de ambos `lib/tinyusb/src` no da diferencias; actualizar el
   SDK a 2.3.0 no cambia TinyUSB). Misma API usada (`tud_cdc_*`, `tusb_init(rhport, &init)` existe en
   0.18: [SDK `lib/tinyusb/src/tusb.h:134-149`]). Ver R2 y U6.
7. **Sin interrupcion SysTick.** `rp2040.begin(0)` instala un manejador de SysTick con recarga
   `0x00FFFFFF` (una excepcion cada 2^24 ciclos = 111,8 ms a 150 MHz) para `getCycleCount()`
   ([ARD `cores/rp2040/RP2040Support.h:294-300`]). El proyecto no usa `getCycleCount`; el SDK no la
   instala. Inocuo (una interrupcion periodica menos).
8. **`available()` sin `yield()`.** `Adafruit_USBD_CDC::available()` llama `yield()` si no hay datos.
   En el proyecto la unica llamada (`MiYoke_PD.ino:287`) esta dentro de `UsbLock`, donde el `yield()`
   propio no hace nada (su `mutex_try_enter` falla porque el mutex ya esta tomado). La HAL no lo llama.
9. **`setSDA/setSCL` con pin invalido o con bus en marcha a otro pin**: Arduino entra en `panic`; la HAL
   devuelve `false` sin cambiar nada. Con el bus en marcha y el **mismo** pin devuelve `true` (lo usa
   `YokeI2C::configureBus0()` en cada sonda del LCD).

10. **`pwmSetup` reinicia los slices aunque frecuencia y rango no cambien.** Inocuo: solo se llama en el arranque.

---

## 3. Inventario: API Arduino -> Pico SDK, por archivo

Notacion: "HAL x" = funcion o clase de `include/hal/` (seccion 4).

| Archivo actual | Uso Arduino | Reemplazo |
|---|---|---|
| `MiYoke_PD.ino` -> `src/main.cpp` | `#include <Adafruit_TinyUSB.h>`, `TinyUSBDevice.setID/mounted/detach/attach`, `delay(10)` | Se elimina; descriptores en `src/hal/rp2350/usb_descriptors.cpp` |
| | `Serial.begin/available/read/write/availableForWrite/dtr` | HAL `hal::usbSerial` (mismos metodos); `begin` se elimina |
| | `extern "C" void yield()` (tud_task + flush bajo mutex) | `hal::usb::background()` llamado tras cada `loop()` desde `main()` |
| | `rp2040.getResetReason()`, `RP2040::WDT_RESET` | HAL `hal::resetReason()`, `hal::ResetReason::Wdt` (mismos valores 0..7) |
| | `rp2040.wdt_begin/wdt_reset` | HAL `hal::watchdogBegin(ms)` / `hal::watchdogFeed()` |
| | `analogWriteRange(255); analogWriteFreq(MOTOR_PWM_HZ)` | HAL `hal::pwmSetup(MOTOR_PWM_HZ, 255)` (antes de `rollMotor.begin()`) |
| | `micros()/millis()` | HAL `hal::micros()/hal::millis()` |
| | `snprintf_P`, `PSTR`, `<pgmspace.h>`, `<avr/pgmspace.h>` | `snprintf` con literal normal (en RP2350 PSTR ya era identidad) |
| | `noInterrupts()` (solo `YOKE_TEST_HANG==2`) | `hal::irqSave()` |
| | `hardware/structs/{powman,io_bank0,sio,watchdog}.h`, `__StackBottom/__StackTop`, `isr_hardfault`, `__not_in_flash_func` | Se conservan tal cual (son del SDK, no de Arduino). `main.cpp` es solo de destino y puede incluir SDK. Hoy `__not_in_flash_func` llega por `Arduino.h`: `main.cpp` agrega `#include <pico.h>` explicito (correccion de Claude, 2026-11-02: SDK 2.2.0 da `#error` si se incluye `pico/platform.h` directo [SDK `src/rp2350/pico_platform/include/pico/platform.h:20`]; `pico.h:64` lo incluye y trae `pico/platform/sections.h`) (y conserva `hardware/structs/io_bank0.h`, que define `GPIO_FUNC_SIO`). |
| | `TinyUSB_Device_FlushCDC()`, `__usb_mutex` (en el `yield()` propio) | Desaparecen con el `yield()`; los reemplaza `hal::usb::background()` (4.7) |
| | `#include <pgmspace.h>` / `<avr/pgmspace.h>` (l. 67-71) | Se eliminan |
| | `#if defined(ARDUINO_ARCH_RP2040)`, `USE_TINYUSB`, `ARDUINO_ARCH_AVR` | `YOKE_TARGET_RP2350` o se elimina la condicion (ver 6.3) |
| `mis_defines.h` | `#include <Arduino.h>` (l. 12-14); `YOKE_CALIBRATION_ENABLED` por defecto segun `ARDUINO_ARCH_RP2040 \|\| !ARDUINO` (l. 24-30); ramas `ARDUINO_ARCH_RP2040` / AVR de pines (l. 127-139, 171-195); `#error` de modos 2/3 en AVR (l. 66-68); comentarios con `EEPROM.commit`/`analogWrite`/`noInterrupts` | Sin include; `YOKE_CALIBRATION_ENABLED` por defecto **1** sin condicion de plataforma (se conserva el `#ifndef` para poder forzarlo); pines Pico sin condicion; se borran ramas AVR y el `#error` (U1). Nota: hoy las pruebas de host (`comparar_ensayo.h:3`) incluyen `mis_defines.h` y ven los pines AVR; despues ven los de Pico (sin efecto: ninguna prueba usa pines). Comentarios: solo si quedan falsos |
| `LcdTelemetry.h` | `Wire.*` (beginTransmission/write/endTransmission), `millis`, `digitalRead(LCD_SDA/SCL)`, `pinMode` (solo AVR), `snprintf_P/PSTR`, `constrain`, `<pgmspace.h>` | `hal::wire0`, `hal::millis`, `hal::digitalRead`, `snprintf`, `hal::constrain`; rama AVR y `WIRE_HAS_TIMEOUT` se eliminan |
| `CalibrationControls.h` | `pinMode/digitalRead/digitalWrite/millis`, `EEPROM.begin/read/write/commit`, `LOW/HIGH` | `hal::pinMode/...`, `hal::eeprom` (misma semantica), `false/true` |
| `MotorDriver.h` | `pinMode`, `digitalWrite`, `analogWrite`, `constrain` | `hal::pinMode`, `hal::digitalWrite`, `hal::pwmWrite`, `hal::constrain` |
| `YokeI2C.h` | `Wire.setSDA/setSCL/begin/setTimeout(2,true)/setClock`, rama `WIRE_HAS_TIMEOUT` | `hal::wire0` mismos metodos; rama muerta eliminada |
| `PicoEncoder.h` | `pinMode(INPUT_PULLUP)`, `digitalRead`, `attachInterruptParam(..., CHANGE, this)`, `<hardware/sync.h>` | `hal::pinMode`, `hal::digitalRead`, `hal::attachChangeInterrupt`, `hal::irqSave/irqRestore` |
| `AxisEncoders.h` | `<Arduino.h>`, rama `<Encoder.h>` (PJRC, AVR) | Solo `PicoEncoder`; rama AVR eliminada |
| `AxisPotentiometers.h` | `analogReadResolution(12)`, `pinMode(INPUT)`, `analogRead` | `hal::pinMode(Input)`, `hal::analogRead12` |
| `AxisAs5600.h` | `Wire`, `Wire1`, `TwoWire&`, `millis` | `hal::wire0`, `hal::wire1`, `hal::I2cBus&`, `hal::millis` |
| `UsbGuard.h` | `pico/mutex.h`, `extern mutex_t __usb_mutex` (simbolo de Adafruit) | `hal::usb::lock()/unlock()` sobre un mutex propio de la HAL |
| `LoopSupervisor.h` | `#if defined(ARDUINO_ARCH_RP2040)` para `<hardware/structs/watchdog.h>` y `watchdog_hw->scratch` (l. 5-7, 112-117) | `#if defined(YOKE_TARGET_RP2350)` (el puente y las pruebas siguen sin definirlo). **Unica excepcion permitida** a la regla de 4.1 (cabecera del SDK en `include/`), porque esta guardada y no se compila en host |
| `UsbGuard.h` (detalle) | Hoy `UsbLock` real solo con `ARDUINO_ARCH_RP2040 && USE_TINYUSB && YOKE_USB_GUARD` | `#if YOKE_USB_GUARD` -> `UsbLock` llama `hal::usb::lock()/unlock()` (en host, `hal_fake` cuenta llamadas); si no, struct vacia. `UsbGuard.h` incluye `mis_defines.h` o recibe `YOKE_USB_GUARD` ya definido (hoy lo recibe por orden de include en el `.ino`: conservar ese orden) |
| `CalibrationMenu.h`, `GainSettings.h`, `RecordSetting.h` | Solo comentarios ("EEPROM") | Sin cambio de codigo |
| Sin Arduino (solo cambian de carpeta) | `SimEffects.h`, `GroundModel.h`, `GustModel.h`, `BuffetModel.h`, `RawTelemetry.h`, `SimTelemetry.h`, `NarrowbandNoise.h`, `FrictionModel.h`, `PicoLog.h`, `YokeMotion.h`, `PicoForceModels.h`, `As5600Position.h`, `PotentiometerPosition.h`, `PotCalibration.h` | `git mv` a `include/`; contenido identico |

Biblioteca PJRC `Encoder`: no se usa en RP2350 (solo en la rama AVR que se elimina).

---

## 4. Diseno de la capa HAL

### 4.1 Estructura de carpetas (final)

```
CMakeLists.txt                     (nuevo, raiz)
pico_sdk_import.cmake              (copia de [SDK]/external/pico_sdk_import.cmake)
include/                           (todos los .h que hoy estan en la raiz)
include/hal/hal.h                  (incluye todos los hal_*.h)
include/hal/hal_time.h  hal_gpio.h  hal_pwm.h  hal_adc.h  hal_i2c.h  hal_usb.h
include/hal/hal_watchdog.h  hal_eeprom.h  hal_util.h
include/hal/pwm_plan.h  reset_reason.h  eeprom_image.h  usb_descriptor_data.h   (logica pura, sin SDK)
src/main.cpp                       (ex MiYoke_PD.ino)
src/hal/rp2350/time_gpio.cpp  pwm_adc.cpp  i2c.cpp  watchdog.cpp  eeprom_flash.cpp
src/hal/rp2350/usb_cdc.cpp  usb_descriptors.cpp  tusb_config.h
tests/hal_fake.cpp  tests/hal_checks.cpp  tests/hal_checks.vcxproj   (nuevos)
```

Regla: los modulos de `include/` (salvo `main.cpp`) solo incluyen `hal/*.h`, nunca cabeceras del SDK.
Asi compilan en el host con `tests/hal_fake.cpp`. Los `.h` "puros" (`pwm_plan.h`, `reset_reason.h`,
`eeprom_image.h`, `usb_descriptor_data.h`, `hal_util.h`) no dependen de nada y los usan tanto el
destino como las pruebas.

### 4.2 Tiempo - `hal_time.h`

```cpp
namespace hal { uint32_t micros(); uint32_t millis(); }
```
Destino: `micros()` = `time_us_32()`; `millis()` = `to_ms_since_boot(get_absolute_time())`
(es decir `uint32_t(time_us_64()/1000)`). Identico a [ARD `cores/rp2040/delay.cpp:58-64`].

### 4.3 GPIO e interrupciones - `hal_gpio.h`

```cpp
namespace hal {
enum class PinMode : uint8_t { Input, InputPullup, Output };
void pinMode(uint8_t pin, PinMode mode);
bool digitalRead(uint8_t pin);
void digitalWrite(uint8_t pin, bool high);
using IrqHandler = void (*)(void* ctx);
void attachChangeInterrupt(uint8_t pin, IrqHandler handler, void* ctx); // flancos subida y bajada
uint32_t irqSave();             // save_and_disable_interrupts()
void irqRestore(uint32_t state);// restore_interrupts()
}
```
Semantica de destino (igual a [ARD `cores/rp2040/wiring_digital.cpp`, `__pinMode`]):
- `Input`: `gpio_set_dir(pin,false)`, `gpio_set_function(pin,GPIO_FUNC_SIO)`, `gpio_disable_pulls(pin)`.
- `InputPullup`: `gpio_set_dir(false)`, funcion SIO, `gpio_pull_up`, `gpio_put(pin,0)`.
- `Output`: funcion SIO, `gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_4MA)`, `gpio_set_dir(true)`.
- Para pines 26..29, `pinMode` borra la marca "ADC inicializado" (ver 4.5).
- La HAL guarda el modo de cada pin (`modes[30]`), necesario para el modo "drenaje abierto" de 4.6.
- `digitalWrite`: **siempre primero** `gpio_set_function(pin, GPIO_FUNC_SIO)`; luego, si el modo es
  `InputPullup`, `true` = `gpio_set_dir(false)` (soltar) y `false` = `gpio_set_dir(true)` (salida en 0,
  porque `pinMode` dejo `gpio_put(pin,0)`); si no, `gpio_put(pin, high)`. (Asi lo hace Arduino-Pico,
  [ARD `cores/rp2040/wiring_digital.cpp`, `__digitalWrite`]; lo usa la sonda/recuperacion I2C.)
- `digitalRead`: `gpio_get(pin)` (funciona aunque el pin este en funcion I2C: `LcdTelemetry::busAvailable`).
- `attachChangeInterrupt`: tabla estatica `{handler, ctx}[30]`; la primera vez registra un despachador
  con `gpio_set_irq_callback(dispatch)` + `irq_set_enabled(IO_IRQ_BANK0, true)`; luego
  `gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true)`. El SDK reconoce el
  evento antes de llamar al callback. Prioridad: la por defecto (igual que Arduino-Pico, que usa
  `gpio_set_irq_enabled_with_callback` con su despachador, [ARD `cores/rp2040/wiring_private.cpp:146,171`]).

### 4.4 PWM del motor - `hal_pwm.h` + `pwm_plan.h`

Valores reales actuales: `analogWriteRange(255)`, `analogWriteFreq(MOTOR_PWM_HZ=490)`
([REPO `MiYoke_PD.ino:223-224`, `mis_defines.h:181`]), pines GP6/GP7 (slice 3 A/B, roll) y
GP8/GP9 (slice 4 A/B, pitch), EN en GP10/GP11.

Algoritmo de Arduino-Pico ([ARD `cores/rp2040/wiring_analog.cpp:79-123`]) aplicado a 150 MHz:
1. Escala inicial 255. Mientras `clk/(escala*f) > 255` y `escala < 32678`: `escala *= 2`, `PS++` (PS
   empieza en 1). 150e6/(255*490)=1200,5 -> 510 -> 1020 -> 2040 (150,06 <= 255): **escala 2040, PS=4**.
2. Mientras `clk/(escala*f) < 1` y `escala >= 6`: no aplica. **SS=1**.
3. `pwm_config_set_clkdiv(clk/(2040*490.0f))` = 150,060 con redondeo al 1/16 mas cercano
   (`PICO_PWM_CLKDIV_ROUND_NEAREST`, que por defecto vale `PICO_CLKDIV_ROUND_NEAREST=1`:
   [SDK `src/rp2_common/hardware_pwm/include/hardware/pwm.h:106-108,161-168`],
   [SDK `src/rp2350/pico_platform/include/pico/platform.h:63-66`]) -> registro DIV = 2401 =
   **div = 150 + 1/16**.
4. `wrap = escala - 1 = 2039`; `pwm_init(slice, &c, true)` una vez por slice.
5. Nivel: `v = (valor << PS) >> SS` = `valor * 8` (en la HAL escribir `valor * (1 << PS) / (1 << SS)`
   con aritmetica con signo o recortar antes: desplazar a la izquierda un negativo es UB en C++17; el
   resultado debe ser el mismo); recorte a `[0, 2040]`; `gpio_set_function(pin, PWM)`;
   `pwm_set_gpio_level(pin, v)`.
6. Cada llamada a `pwmWrite` repite `gpio_set_function(pin, GPIO_FUNC_PWM)` (igual que `analogWrite`),
   porque `MotorDriver::begin()` antes pone esos pines como salida SIO con `pinMode(OUTPUT)`.

Resultado esperado (verificable con osciloscopio): **f = 150e6 / (150,0625 * 2040) = 489,99 Hz**,
ciclo de trabajo = valor/255 exacto (nivel 2040 = 100 %), modo no centrado, 4 mA.

API:
```cpp
// pwm_plan.h (puro)
struct PwmPlan { uint32_t scale;   // escala efectiva (2040); el registro TOP/wrap es scale-1 (2039)
                 uint8_t shiftUp, shiftDown; uint8_t divInt, divFrac; };
PwmPlan planPwm(uint32_t clkHz, uint32_t freqHz, uint32_t range);   // pasos 1-3 con las mismas
                                                                      // operaciones float
uint32_t pwmLevel(const PwmPlan& p, int value);                      // paso 5 (recorte incluido)
// hal_pwm.h
namespace hal { void pwmSetup(uint32_t freqHz, uint32_t range); void pwmWrite(uint8_t pin, int value); }
```
`pwmSetup` calcula el plan con `clock_get_hz(clk_sys)` y marca todos los slices como no inicializados;
`pwmWrite` inicializa el slice en la primera escritura (como `pwmInitted`). Las validaciones de rango
de `analogWriteFreq` (100 Hz..10 MHz) y `analogWriteRange` (3..65535) se conservan.

### 4.5 ADC - `hal_adc.h`

`uint16_t hal::analogRead12(uint8_t pin)`: pin fuera de 26..29 -> 0; `adc_init()` la primera vez;
`adc_gpio_init(pin)` si el pin no esta marcado (la marca la borra `pinMode`); `adc_select_input(pin-26)`
solo si cambia el canal; devuelve `adc_read()` (12 bits, equivale a `analogReadResolution(12)`).
Igual a [ARD `cores/rp2040/wiring_analog.cpp:134-160`]. `AxisPotentiometers` conserva su descarte de la
primera conversion y el promedio de `YOKE_POT_SAMPLES`.

### 4.6 I2C - `hal_i2c.h`

Clase con la API de `TwoWire` que usa el proyecto (nada mas):
```cpp
namespace hal {
class I2cBus {
public:
    I2cBus(uint8_t index, uint8_t defaultSda, uint8_t defaultScl);
    bool setSDA(uint8_t pin); bool setSCL(uint8_t pin);   // solo cambian si no esta corriendo
    void begin(); void end();
    void setClock(uint32_t hz);                            // i2c_set_baudrate si corre
    void setTimeout(uint32_t ms, bool resetOnTimeout);     // borra la bandera de timeout
    void beginTransmission(uint8_t addr);
    size_t write(uint8_t b);
    uint8_t endTransmission(bool stop = true);             // 0 ok, 2 NACK sonda, 4 error, 5 timeout
    size_t requestFrom(uint8_t addr, size_t qty, bool stop = true);
    int read(); int available();
};
extern I2cBus wire0;   // i2c0, SDA 4 / SCL 5 por defecto (pins_arduino.h de rpipico2)
extern I2cBus wire1;   // i2c1, SDA 26 / SCL 27 por defecto
}
```
Semantica a reproducir ([ARD `libraries/Wire/src/Wire.cpp`]):
- Valores iniciales: reloj 100 kHz (`TWI_CLOCK`), timeout 1000 ms (Stream), sin reset.
- `begin()` (si no corre): `i2c_init(i2c, clk)`, `i2c_set_slave_mode(false,0)`, SDA y SCL a funcion I2C
  con `gpio_pull_up` (lineas 118-135). Si ya corre, no hace nada.
- `end()`: `i2c_deinit`, `pinMode(sda/scl, Input)` (sin pull-ups) (lineas 250-268).
- Buffer: 32 bytes estaticos (el proyecto escribe como maximo 7 y lee 3; Arduino usa 256 con `malloc`).
  `write` devuelve 0 si no hay transmision iniciada o el buffer esta lleno.
- `endTransmission(stop)` (417-435): sin `begin` o sin `beginTransmission` -> 4. **Longitud 0 -> sonda
  por bit-bang** `probe()` -> 0 si hay ACK, 2 si no. Si no: `i2c_write_blocking_until(i2c, addr, buf,
  len, !stop, make_timeout_time_ms(timeout))`; `PICO_ERROR_TIMEOUT` -> `handleTimeout(reset)` y 5;
  `ret == len` -> 0; otro -> 4.
- `requestFrom` (281-298): sin correr, con transmision abierta, `qty==0` o `qty>buffer` -> 0.
  `i2c_read_blocking_until(..., !stop, make_timeout_time_ms(timeout))`; error generico o timeout -> 0
  (timeout ademas -> `handleTimeout`); offset de lectura a 0; devuelve la cantidad.
- `probe(addr)` (307-362): medio periodo `d = (1e6/clk)/2` us (5 us a 100 kHz; con 400 kHz, 1 us);
  pines a `InputPullup` y funcion SIO; secuencia START, 8 bits de direccion (`addr<<1`, escritura) con
  `sleep_us(d)`, `sleep_us(5)` **solo** tras el flanco bajo de SCL de cada uno de los 8 bits de
  direccion, espera de estiramiento de reloj de hasta 500 us en cada subida de SCL (`time_us_64`; si
  vence, salta directo al STOP con NACK), lectura de ACK, STOP y vuelta de ambos pines a funcion I2C.
  Seguir el orden exacto de [ARD `Wire.cpp:307-362`]. La sonda no usa el timeout de `setTimeout`. Todas
  las escrituras de pin con la semantica de drenaje abierto de 4.3.
- `endTransmission` con error distinto de timeout (NACK de datos, `PICO_ERROR_GENERIC`): pone la
  longitud del buffer en 0 y devuelve 4 (l. 432-433).
- `handleTimeout(reset)` (364-410): bandera de timeout; si `reset`: `end()`; pines `InputPullup` y SIO;
  si SDA esta en bajo, hasta 9 pulsos de SCL de 5 us/5 us mientras SDA siga baja; si SDA se libero,
  STOP (SDA baja 5 us y sube); `setClock(prev)`; `begin()`.
- `setSDA/setSCL` validos: i2c0 SDA {0,4,8,...,28}, SCL {1,5,...,29}; i2c1 SDA {2,6,...,26},
  SCL {3,7,...,27}. Orden de [ARD `Wire.cpp:57-80`]: (1) sin correr y pin valido -> se guarda, `true`;
  (2) pin igual al actual (corriendo o no) -> `true`; (3) si no, Arduino entra en `panic` y la HAL
  devuelve `false` sin cambiar nada (2.1-9). Los pines del proyecto (GP4/5 en i2c0, GP14/15 en i2c1)
  son validos.

Uso actual que no cambia: bus 0 (LCD + AS5600 de pitch) a `YOKE_I2C0_HZ` = 100 kHz con
`setTimeout(2, true)`; bus 1 (AS5600 de roll) a `YOKE_I2C1_HZ` = 400 kHz con `setTimeout(1, true)`.
`YokeI2C::configureBus0()` repite `setSDA/setSCL/begin/setTimeout/setClock` en cada sonda del LCD
(`setClock` con el bus en marcha reprograma la velocidad, como hoy).

### 4.7 USB CDC - `hal_usb.h`, `usb_cdc.cpp`, `usb_descriptors.cpp`, `tusb_config.h`

```cpp
namespace hal {
namespace usb { void init(); void background(); void lock(); void unlock(); }
class CdcSerial {
public:
    int available(); int read(); int availableForWrite(); bool dtr();
    size_t write(const uint8_t* data, size_t n); size_t write(const char* data, size_t n);
};
extern CdcSerial usbSerial;
}
```

Arranque y tarea de fondo (replica de [ARD `.../ports/rp2040/Adafruit_TinyUSB_rp2040.cpp:81-117`]):
- `init()`: `mutex_init(&usbMutex)`; `tusb_init(0, &{TUSB_ROLE_DEVICE, TUSB_SPEED_FULL})`;
  `irq = user_irq_claim_unused(true)`; `irq_set_exclusive_handler(irq, usbTaskIrq)`;
  `irq_set_enabled(irq, true)`; `irq_add_shared_handler(USBCTRL_IRQ, triggerIrq,
  PICO_SHARED_IRQ_HANDLER_LOWEST_ORDER_PRIORITY)` (el controlador de TinyUSB se registra con prioridad
  de orden mas alta: [SDK `lib/tinyusb/src/portable/raspberrypi/rp2040/dcd_rp2040.c:367`]).
- `usbTaskIrq()`: `if (mutex_try_enter(&usbMutex, nullptr)) { tud_task(); mutex_exit(&usbMutex); }`.
- `triggerIrq()`: `irq_set_pending(irq)`.
- `background()` (reemplaza al `yield()` propio, [REPO `MiYoke_PD.ino:58-65`]): con `YOKE_USB_GUARD=1`:
  `if (mutex_try_enter) { tud_task(); tud_cdc_write_flush(); mutex_exit; }`. Con `YOKE_USB_GUARD=0`
  reproduce el `yield()` por defecto del core ([ARD `cores/rp2040/delay.cpp:50-55`]): `tud_task` bajo
  `try_enter` y `tud_cdc_write_flush()` sin mutex.
- `lock()/unlock()`: `mutex_enter_blocking` / `mutex_exit` (los usa `UsbLock`).
- `CdcSerial`: `available()` = `tud_cdc_available()` (sin `yield`, 2.1-8); `read()` =
  `tud_cdc_read_char()` (-1 si no hay); `availableForWrite()` = `tud_cdc_write_available()`; `dtr()` =
  `tud_cdc_connected()`; `write()` = `if (!tud_cdc_connected()) return 0;` + una sola `tud_cdc_write()`
  (ver 2.1-3). Igual que [ARD `.../Adafruit_USBD_CDC.cpp:165-171,240-265`]. Ninguna funcion de
  `CdcSerial` toma el mutex: el llamador ya tiene `UsbLock` (como hoy).
- `tud_cdc_line_state_cb(itf, dtr, rts)`: si `itf == 0`, `!dtr` y la velocidad de linea es 1200
  baudios -> `reset_usb_boot(0, 0)` y `while (1) {}` (modo BOOTSEL; se conserva el "toque a 1200" de
  [ARD `.../Adafruit_USBD_CDC.cpp:271-290`] y [ARD `.../Adafruit_TinyUSB_rp2040.cpp:120-124`]).
  Sin agregados (la HAL no conoce los pines del motor; `reset_usb_boot` reinicia el chip y los GPIO
  vuelven a reset). Verificar con H13 que no hay pulso en EN/PWM durante el toque a 1200.

`tusb_config.h` (en `src/hal/rp2350/`): `CFG_TUSB_RHPORT0_MODE = OPT_MODE_DEVICE`, `CFG_TUSB_OS =
OPT_OS_PICO`, `CFG_TUSB_DEBUG 0`, `CFG_TUD_ENDPOINT0_SIZE 64`, `CFG_TUD_CDC 1`, `CFG_TUD_MSC/HID/MIDI/
VENDOR 0`, `CFG_TUD_CDC_RX_BUFSIZE 256`, `CFG_TUD_CDC_TX_BUFSIZE 256`, `CFG_TUD_CDC_EP_BUFSIZE 64`.
(Buffers iguales a [ARD `.../ports/rp2040/tusb_config_rp2040.h`]: RX/TX 256, EP0 64.)

**Descriptores (deben quedar identicos; se generan desde `usb_descriptor_data.h`, puro, probado en host):**

Dispositivo (18 bytes; [ARD `.../Adafruit_USBD_Device.cpp:164-181,232-241`], VID/PID de `MiYoke_PD.ino:230`):
`bLength 18, bDescriptorType 1, bcdUSB 0x0200, bDeviceClass 0xEF, bDeviceSubClass 0x02,
bDeviceProtocol 0x01, bMaxPacketSize0 64, idVendor 0x2E8A, idProduct 0x000F, bcdDevice 0x0100,
iManufacturer 1, iProduct 2, iSerialNumber 3, bNumConfigurations 1`.

Configuracion (9 + 66 = 75 bytes): `TUD_CONFIG_DESCRIPTOR(1, 2, 0, 75, 0xA0 /*bit7|remote wakeup*/,
100 /*mA*/)` ([ARD `.../Adafruit_USBD_Device.cpp:183-190`], `USB_CONFIG_POWER` = 100 porque el core no
define ese macro; `USBD_MAX_POWER_MA=250` de boards.txt no lo usa Adafruit) seguido de
`TUD_CDC_DESCRIPTOR(itf 0, str 4, notif 0x81, 8, out 0x01, in 0x82, 64)` ([ARD
`.../Adafruit_USBD_CDC.cpp:49-76`]; asignacion de endpoints de `Adafruit_USBD_Device.h:108-109`:
IN empieza en 1, OUT empieza en 1).

Cadenas: 0 = idioma 0x0409; 1 = "Raspberry Pi"; 2 = "Pico 2" ([ARD `boards.txt:656-657`] via
`-DUSB_MANUFACTURER/-DUSB_PRODUCT`, [ARD `platform.txt:54`]); 3 = numero de serie: 16 caracteres
hexadecimales en MAYUSCULA de `pico_get_unique_board_id()` (8 bytes, byte 0 primero, nibble alto
primero) ([ARD `.../Adafruit_USBD_Device.cpp:280-296`], [ARD `.../Adafruit_TinyUSB_rp2040.cpp:125-128`]);
4 = "TinyUSB Serial" (`Adafruit_USBD_CDC::begin`, [ARD `.../Adafruit_USBD_CDC.cpp:105`]). Indices
mayores -> NULL.

Consecuencia: Windows ve el mismo dispositivo compuesto `USB\VID_2E8A&PID_000F&MI_00` con el mismo
numero de serie, asi que conserva el mismo COM. El lanzador acepta `^USB\\VID_2E8A&PID_000F(?:&|\\)`
([REPO `Iniciar-Simulador.ps1:55`]); no se toca.

### 4.8 Watchdog y motivo de reset - `hal_watchdog.h` + `reset_reason.h`

```cpp
namespace hal {
enum class ResetReason : int { Unknown=0, PowerOn=1, RunPin=2, Soft=3, Wdt=4, Debug=5, Glitch=6, Brownout=7 };
void watchdogBegin(uint32_t ms);   // watchdog_enable(ms, 1)   ([ARD RP2040Support.h:582-584])
void watchdogFeed();               // watchdog_update()       ([ARD RP2040Support.h:589-591])
ResetReason resetReason();
}
// reset_reason.h (puro)
ResetReason decodeResetReason(bool wdCaused, bool wdEnableCaused, uint32_t wdReason, uint32_t chipReset);
```
Orden de decision identico a [ARD `cores/rp2040/RP2040Support.h:596-654`] (rama RP2350):
1. `watchdog_caused_reboot() && watchdog_enable_caused_reboot()` -> Wdt.
2. `wdReason & 0x1` (WATCHDOG_REASON_TIMER) -> Soft.
3. `chipReset & 0x00010000` (HAD_POR) -> PowerOn; `& 0x00040000` (HAD_RUN_LOW) -> RunPin;
   `& (0x00080000 | 0x00200000 | 0x08000000)` (DP_RESET_REQ, RESCUE, HZD_SYS_RESET_REQ) -> Debug;
   `& 0x04000000` (GLITCH_DETECT) -> Glitch; `& 0x00020000` (BOR) -> Brownout.
4. Si no -> Unknown.
Mascaras: [SDK `src/rp2350/hardware_regs/include/hardware/regs/powman.h:528-711`],
[SDK `.../regs/watchdog.h:99-106`]; el `.cpp` de destino agrega `static_assert` contra los macros del SDK.
Los valores numericos coinciden con `YokeHealth::resetName` ([REPO `LoopSupervisor.h:84-92`]).
`watchdog_caused_reboot()` en RP2350 exige tipo de arranque normal ([SDK
`src/rp2_common/hardware_watchdog/watchdog.c:116-127`]); es la misma funcion que usa el core.

### 4.9 EEPROM en flash - `hal_eeprom.h` + `eeprom_image.h` + `eeprom_flash.cpp`

```cpp
// hal_eeprom.h: el backend se DECLARA aqui y se DEFINE en eeprom_flash.cpp (destino) o en
// tests/hal_fake.cpp (host: arreglo de 4096 bytes en RAM iniciado en 0xFF + contadores de borrado y
// programacion + bandera para forzar fallo). Asi CalibrationControls.h compila igual en ambos lados.
struct FlashBackend {
    static void read(uint8_t* dst, size_t n);                 // copia desde el sector
    static bool eraseProgram(const uint8_t* src, size_t n);   // borra 4096 y programa n; true si ok
};
// eeprom_image.h (puro): misma semantica que EEPROMClass ([ARD libraries/EEPROM/src/EEPROM.cpp])
template <class Backend> class EepromImage {
public:
    void begin(size_t size);   // size<=0 o >4096 -> 4096; redondeo a multiplo de 256; copia desde flash
    uint8_t read(int addr);    // fuera de rango -> 0
    void write(int addr, uint8_t v); // fuera de rango: nada; marca sucio solo si cambia
    bool commit();             // sin begin -> false; limpio -> true; si no: Backend::eraseProgram -> true
private: uint8_t data[4096]; size_t size; bool dirty;
};
namespace hal { /* objeto */ extern EepromImage<FlashBackend> eeprom; }
```
Buffer estatico de 4096 bytes (hoy `new uint8_t[256]`): se usa estatico para no tener `new`; el
tamano usado sigue siendo 256 (`EEPROM.begin(256)`). Si la RAM lo exige, `YOKE_EEPROM_MAX=256`.

Backend de destino:
- `kOffset = PICO_FLASH_SIZE_BYTES - 2 * FLASH_SECTOR_SIZE` (penultimo sector, igual que Arduino) con
  `static_assert(kOffset == 0x3FE000)` y `static_assert(PICO_FLASH_SIZE_BYTES == 4*1024*1024)` (placa
  `pico2`, [SDK `src/boards/include/boards/pico2.h:78-80`]). Prueba de host: una constante pura
  `kEepromFlashOffset = 0x3FE000` en `eeprom_image.h` que usan el backend y `hal_checks`.
- Lectura: `memcpy(data, (const void*)(XIP_BASE + kOffset), size)` (igual que `memcpy(_data, _sector,...)`).
- `eraseProgram(data, size)`: `uint32_t s = save_and_disable_interrupts(); flash_range_erase(kOffset,
  4096); flash_range_program(kOffset, data, size); restore_interrupts(s);`. Un solo nucleo: no hace
  falta bloquear el nucleo 1 (en Arduino `idleOtherCore()` es no-op con un nucleo,
  [ARD `RP2040Support.h:115-126`]). Las funciones `flash_range_*` del SDK corren desde RAM y vacian la
  cache XIP.
- Guarda: `Build-Pico2.ps1` falla si el tamano del `.bin` (imagen contigua desde `0x10000000`) es
  `>= 0x3FE000` bytes (el binario pisaria el sector). Se usa el `.bin` y no `__flash_binary_end`
  porque ese simbolo es `PROVIDE` y puede no existir en el `.elf` (en el `.map` actual figura como
  `[!provide]`, l. 6466). Hoy ocupa ~126 KiB (UF2 actual: 502 bloques de 256 bytes desde `0x10000000`).

`CalibrationControls::save()` conserva su logica de restaurar el valor previo si `commit()` falla.
`main.cpp` conserva `rollMotor.stop(); pitchMotor.stop();` + `watchdogBegin(YOKE_WDT_FLASH_MS)` antes de
`save()` y `watchdogBegin(YOKE_WDT_MS)` despues.

**Conservacion de la calibracion al pasar de Arduino a SDK:** el UF2 nuevo, como el actual, solo
escribe los bloques de la imagen desde `0x10000000` mas el bloque absoluto de la errata RP2350-E10
(`--abs-block`, `0x10FFFF00`, familia `0xE48BFF57`), que el SDK agrega igual que el core
([SDK `tools/CMakeLists.txt:594-605`]; verificado en el UF2 actual: 503 bloques, 502 de familia
`0xE48BFF59` desde `0x10000000` y uno de familia `0xE48BFF57` en `0x10ffff00`; el UF2 de prueba
`D:\pico\hello\build\hello.uf2`, compilado con el SDK 2.2.0, tiene el mismo bloque). Por lo tanto el
sector `0x3FE000` no se toca y la calibracion guardada por el firmware Arduino se lee con el mismo
formato (`YokeGains::decode`, `YokePots::decode`, `YokeRecord::decode`). **No hay perdida prevista**,
y H0 hace ademas una copia de respaldo del sector con `picotool` antes de cargar el UF2 nuevo. Aunque
el bloque `--abs-block` se reflejara en el ultimo sector (`0x3FF000`) de una flash de 4 MiB, no toca
el sector de calibracion (`0x3FE000`).

### 4.10 Utilidades - `hal_util.h`

`template<class T,class L,class H> constexpr auto constrain(T amt, L low, H high)
{ return amt < low ? low : (amt > high ? high : amt); }` - misma expresion que el macro de
[ARD `ArduinoCore-API/api/Common.h:67`] (mismo orden de comparaciones: NaN pasa sin cambio, como hoy).

### 4.11 Lo que hace el core antes de `setup()` y como se replica

Secuencia actual ([ARD `cores/rp2040/main.cpp`]):
1. crt0 + `runtime_init` del SDK (relojes 150 MHz, constructores C++). Igual con el SDK.
2. F_CPU = 150 MHz: no llama `set_sys_clock_khz` (lineas 104-114). Igual.
3. `rp2040.begin(0)`: instala la excepcion SysTick (2.1-7); `fifo.begin(1)`/`registerCore()` e
   `initVariant()`: sin segundo nucleo no hacen nada util. Se omiten.
4. `TinyUSB_Device_Init(0)` (linea 148): arma el descriptor e inicializa TinyUSB e IRQ. -> `hal::usb::init()`.
5. `setup()` y luego `while (true) { loop(); __loop(); }` (181-185), con `__loop()` -> `yield()`
   (74-77) -> en este proyecto el `yield()` propio (tud_task + flush bajo mutex).

Nuevo `main()` al final de `src/main.cpp`:
```cpp
int main() {
    hal::usb::init();
    setup();
    for (;;) { loop(); hal::usb::background(); }
}
```
El lazo de 2 ms no depende de nada mas del core: `loop()` alimenta el watchdog en cada llamada, mide
con `micros()` y vuelve enseguida si no paso `CONTROL_LOOP_MS`. Entre ciclos de control la CPU gira en
`loop()`+`background()`, igual que hoy con `loop()`+`yield()`. No hay `serialEvent` en el proyecto.

Prototipos: en `MiYoke_PD.ino` todas las funciones (`markStage`, `slowFlags`, `finishLoop`,
`debugPrintMotion`, `setup`, `loop`) estan definidas antes de usarse; con `main()` al final no hacen
falta prototipos. Si Codex reordena, agrega prototipos explicitos arriba.

### 4.12 Cerrojos de newlib - `src/hal/rp2350/newlib_locks.cpp` (agregado por Claude en la parte 4)

La newlib de `pqt-gcc` (U3) esta compilada con `_RETARGETABLE_LOCKING`
[`D:/pico/toolchains/pqt-gcc-5.0.0/arm-none-eabi/include/sys/lock.h:26-66`]: el enlace falla con
`undefined reference to __retarget_lock_*` y `__lock___*_mutex` si el firmware no los define. El core
los daba en [ARD `cores/rp2040/lock.cpp`]; el SDK 2.2.0 no los trae. `newlib_locks.cpp` replica la
rama sin FreeRTOS de ese archivo: los nueve objetos `__lock___*` son `mutex_t`/`recursive_mutex_t`
en la seccion `.mutex_array` (los inicializa `runtime_init_mutex` antes de `main()`, marca
`spin_lock = 1` para los recursivos, igual que `auto_init_recursive_mutex` [SDK
`src/common/pico_sync/include/pico/mutex.h:306`, `mutex.c:24-35`]) y las funciones
`__retarget_lock_*` llaman a `mutex_*`/`recursive_mutex_*`. `static_assert`: ambos tipos entran en
`struct __lock` (8 bytes). Con guarda D13 y agregado a `MIYOKE_HAL_SOURCES`.

---

## 5. CMake y variantes

### 5.1 `CMakeLists.txt` (raiz)

```cmake
cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico2 CACHE STRING "")
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "")
include(pico_sdk_import.cmake)
project(miyoke_pd C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "" FORCE)
endif()
set(CMAKE_C_FLAGS_MINSIZEREL   "-Os -g -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -g -DNDEBUG")
pico_sdk_init()

set(MIYOKE_HAL_SOURCES src/hal/rp2350/time_gpio.cpp src/hal/rp2350/pwm_adc.cpp src/hal/rp2350/i2c.cpp
    src/hal/rp2350/watchdog.cpp src/hal/rp2350/eeprom_flash.cpp src/hal/rp2350/usb_cdc.cpp
    src/hal/rp2350/usb_descriptors.cpp)
set(MIYOKE_LIBS pico_stdlib hardware_pwm hardware_adc hardware_i2c hardware_flash hardware_watchdog
    hardware_sync hardware_irq pico_unique_id pico_bootrom tinyusb_device)

function(miyoke_firmware name)          # ARGN = definiciones de la variante
  add_executable(${name} src/main.cpp ${MIYOKE_HAL_SOURCES})
  target_include_directories(${name} PRIVATE include src/hal/rp2350)
  target_compile_definitions(${name} PRIVATE YOKE_TARGET_RP2350=1
      PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64 ${ARGN})     # D14
  target_compile_options(${name} PRIVATE -Werror=return-type -Wno-psabi
      $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>)
  target_link_libraries(${name} PRIVATE ${MIYOKE_LIBS})
  pico_enable_stdio_usb(${name} 0)
  pico_enable_stdio_uart(${name} 0)
  pico_set_printf_implementation(${name} compiler)
  pico_add_extra_outputs(${name})       # .uf2 .bin .hex .dis .map
endfunction()

miyoke_firmware(myk_cfg)
miyoke_firmware(myk_enc   YOKE_POSITION_SOURCE=0)
miyoke_firmware(myk_pot   YOKE_POSITION_SOURCE=2)
miyoke_firmware(myk_as    YOKE_POSITION_SOURCE=3)
miyoke_firmware(myk_falla YOKE_POSITION_SOURCE=2 YOKE_TEST_HANG=3 YOKE_TEST_HANG_MS=20000)
```
Nombres cortos por MAX_PATH. Las definiciones de variante son las de [REPO `Build-Pico2.ps1:22-25`]
(verificadas: configurado sin definiciones = `YOKE_POSITION_SOURCE 1` por defecto de `mis_defines.h:61-63`).
En la parte 2 solo existe `add_library(myk_hal STATIC ${MIYOKE_HAL_SOURCES})` con las mismas
opciones, definiciones (`YOKE_TARGET_RP2350=1`, D14) y bibliotecas (verifica que la HAL compila para
el destino sin `main`); se elimina en la parte 4.
Si la configuracion de U3/U6 cambia el SDK a 2.3.x, `picotool` y `pioasm` deben ser de la misma
version mayor.menor que el SDK (hoy picotool v2.2.0-a4 con SDK 2.2.0).

Si `-std=gnu++17` no alcanzara para algun encabezado (hoy el firmware se compila con gnu++23), usar
`CMAKE_CXX_STANDARD 20` + `-Wno-volatile` y anotarlo en HANDOFF; no cambiar codigo de modelos para
eso.

### 5.2 `herramientas/compilacion/Build-Pico2.ps1` (reescrito en la parte 4)

Interfaz igual a la actual: `-Encoders`, `-Potentiometers`, `-As5600`, `-PruebaFalla` con las mismas
combinaciones prohibidas (lineas 11-13). Parametros nuevos con valores por defecto y variable de
entorno que los reemplaza:

| Parametro | Entorno | Defecto |
|---|---|---|
| `-SdkPath` | `PICO_SDK_PATH` | `D:/pico/pico-sdk` |
| `-TinyUsbPath` | `PICO_TINYUSB_PATH` | `D:/pico/tinyusb` (U6: commit `e954302c1` = 0.20.1) |
| `-ToolchainBin` | `PICO_TOOLCHAIN_PATH` | `D:/pico/toolchains/pqt-gcc-5.0.0/bin` (U3 opcion B, GCC 16.1; acepta tambien la raiz sin `bin`) |
| `-CMake` | - | `C:/Program Files/CMake/bin/cmake.exe`, si no `Get-Command cmake` |
| `-Ninja` | - | `Get-Command ninja`, si no `%LOCALAPPDATA%/Microsoft/WinGet/Packages/Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe/ninja.exe` |
| `-PicotoolDir` | - | `D:/pico/picotool/picotool` |
| `-PioasmDir` | - | `D:/pico/sdk-tools/pioasm` |
| `-BuildRoot` | `MIYOKE_BUILD_ROOT` | `$env:TEMP\myk` |

Pasos: (0) si existe `<BuildRoot>/CMakeCache.txt` y su `PICO_SDK_PATH`, `PICO_TINYUSB_PATH` o `PICO_TOOLCHAIN_PATH` difiere del pedido, borrar la cache y `CMakeFiles/` (revision de la parte 4); (1) si no existe `<BuildRoot>/CMakeCache.txt`, configurar: `cmake -G Ninja -S <Raiz> -B
<BuildRoot> -DCMAKE_MAKE_PROGRAM=<ninja> -DPICO_SDK_PATH=... -DPICO_TOOLCHAIN_PATH=... -Dpicotool_DIR=...
-Dpioasm_DIR=... -DCMAKE_BUILD_TYPE=MinSizeRel`; (2) `cmake --build <BuildRoot> --target <myk_*>`;
(2b) guarda del sector EEPROM con el tamano de `<BuildRoot>/<myk_*>.bin` (error si >= `0x3FE000` bytes, 4.9) **antes de copiar**, para que no quede en `build-pico2*/` un UF2 que pise la calibracion (revision de la parte 4); (3) copiar `<myk_*>.uf2/.elf/.bin/.elf.map` a `<Raiz>/<carpeta>/MiYoke_PD.uf2/.elf/.bin/.map` y borrar
de esa carpeta los `MiYoke_PD.ino.*` viejos (solo esos cuatro nombres exactos; los `MiYoke_PD_V2.x.ino.*`
historicos de `build-pico2-encoders/` y `build-pico2-potentiometers/` no se tocan); (5) imprimir una linea
`Flash usada: <text+data> bytes; RAM usada: <data+bss> bytes` (de `arm-none-eabi-size`) y
`Firmware generado en <carpeta>. No se cargo en la placa.`. Codigo de salida != 0 ante cualquier falla.
Carpetas de salida y mapeo: configurado -> `build-pico2`/`myk_cfg`; encoders -> `build-pico2-encoders`/
`myk_enc`; potenciometros -> `build-pico2-potentiometers`/`myk_pot`; AS5600 -> `build-pico2-as5600`/
`myk_as`; prueba de falla -> `build-pico2-prueba-falla`/`myk_falla`.

Verificacion de MAX_PATH: la parte 4 compila las 5 variantes desde cero (borrando `<BuildRoot>`) y debe
dar OK. Si fallara por ruta larga, acortar `BuildRoot` (p. ej. `D:\myk`), no los nombres de archivos.

### 5.3 `probar.ps1`

Misma interfaz y misma salida de una linea por paso. Cambios: marcador de raiz `src/main.cpp`;
`$patronTam = '(?i)(flash usada|ram usada)'`; paso nuevo `suite hal_checks` (compila
`tests/hal_checks.vcxproj` y ejecuta `tests/bin/hal_checks.exe`, patron OK `^PASS`) en el paso 1; en la
parte 2 se agrega `-Sdk` (compila `myk_hal` con un script provisorio) y en la parte 4 se quita.

### 5.4 Pruebas de host

- `tests/simconnect_checks.vcxproj`, `tests/comparar_ensayo.vcxproj`: los `#include "../X.h"` pasan a
  `#include "../include/X.h"` (lineas actuales: `simconnect_checks.cpp:1,5,6`, `legacy_effects_checks.cpp:8`, `sim_position_friction_checks.cpp:6`,
  `as5600_checks.h:2`, `calibration_checks.h:3`, `comparar_ensayo.h:3,6,7,8`, `health_checks.h:2`,
  `picolog_checks.h:1,2`). Ninguna prueba usa hoy stubs de Arduino.
- `SimConnectBridge/main.cpp:13-18` incluye `SimTelemetry.h`, `RawTelemetry.h` (resueltos por
  `AdditionalIncludeDirectories=$(MSBuildProjectDirectory)\..`) y `../LoopSupervisor.h`:
  cambiar el directorio a `$(MSBuildProjectDirectory)\..\include` en `YokeSimConnectBridge.vcxproj` y
  `WaitForFlight.vcxproj`, y `../LoopSupervisor.h` -> `../include/LoopSupervisor.h`;
  `SimConnectBridge/{BuffetModel,GroundModel,GustModel}.h` y `Replay.h:11` -> `../include/X.h`.
- Nuevo `tests/hal_checks.vcxproj` (mismas propiedades que `simconnect_checks.vcxproj`: v141, C++17,
  Release Win32, salida `tests/bin/`), con `hal_checks.cpp` + `hal_fake.cpp`. `hal_fake.cpp` define
  todas las funciones `hal::` con estado observable: reloj manual, arreglo de niveles GPIO y modos,
  niveles PWM por pin, lecturas ADC programadas, `I2cBus` con respuestas programadas y registro de
  transacciones, `EepromImage` con backend en RAM (cuenta de borrados/programaciones), `CdcSerial`
  con colas. Imprime `PASS hal_checks (<n> casos)` o `FAIL ...`. Ademas `hal_fake` lleva un **registro
  ordenado de eventos** (`pinMode`, `digitalWrite`, `pwmWrite`, `pwmSetup`, transacciones I2C) que las
  pruebas consultan para verificar orden (p. ej. PWM en 0 antes de EN alto).
- `hal_checks.vcxproj` no incluye `simconnect_checks`: son dos ejecutables separados. Ningun encabezado
  de modelos incluye `hal/` (verificado: el grafo de `#include` de `SimEffects.h`/`PicoForceModels.h`/
  `CalibrationMenu.h` no llega a modulos con hardware), asi que `simconnect_checks` no necesita `hal_fake`.

---

## 6. Plan por partes para Codex

Division en **5 partes**: la propuesta de 3 obligaba a mover cabeceras, migrar `main` y cambiar el
marcador de raiz de los scripts del flujo (incluido `lanzar-codex.py`) en una sola sesion; la de 4
dejaba en la parte 1 toda la HAL (9 cabeceras, 7 `.cpp` de destino, CMake, script provisorio y
pruebas), demasiado para una sesion. Ahora la logica pura con sus pruebas (parte 1) va separada de la
HAL de destino con CMake (parte 2). Cada parte cierra con commit por nombre y HANDOFF, compila y pasa
`probar.ps1`; `nueva-version.ps1` solo en la parte 5.

**Precondiciones comunes (las cumple Claude antes de lanzar la parte 1):** este documento commiteado
en la rama; U1-U7 resueltas y anotadas en HANDOFF (en particular la ruta del compilador y del SDK,
U3/U6); arbol limpio salvo `graphify-ino/`. `tesis/PDFs Extras/` esta sin seguimiento y hace fallar
`nueva-version.ps1` ("Hay cambios sin commit"): el usuario decide antes de la parte 5 si se commitea,
se agrega a `.gitignore` o se mueve (Codex no lo toca; si lo encuentra, lo anota en HANDOFF).

### Parte 1 - logica pura de la HAL y pruebas de host (sin SDK, sin tocar el `.ino` ni la raiz)

Alcance (archivos nuevos): `include/hal/hal.h`, `hal_time.h`, `hal_gpio.h`, `hal_pwm.h`, `hal_adc.h`,
`hal_i2c.h`, `hal_usb.h`, `hal_watchdog.h`, `hal_eeprom.h` (solo declaraciones, 4.2-4.9),
`hal_util.h`, `pwm_plan.h`, `reset_reason.h`, `eeprom_image.h`, `usb_descriptor_data.h` (4.4, 4.7-4.10);
`tests/hal_checks.vcxproj`, `tests/hal_checks.cpp`, `tests/hal_fake.cpp`; modificado: `probar.ps1`
(paso `suite hal_checks`).
Leer: este documento secciones 2, 4, 5.3, 5.4; [ARD] solo las lineas citadas si hace falta.
Pruebas host obligatorias (`hal_checks`):
1. `planPwm(150000000, 490, 255)` -> scale 2040, shiftUp 4, shiftDown 1, divInt 150, divFrac 1.
   `pwmLevel`: 0->0, 1->8, 128->1024, 220->1760, 255->2040, -5->0, 300->2040.
   `planPwm(150000000, 1000, 255)` -> scale 1020, shiftUp 3, shiftDown 1.
2. `decodeResetReason`: los 8 resultados de 4.8 con una mascara por caso, y prioridad (Wdt antes que
   Soft, PowerOn antes que RunPin).
3. `EepromImage` con backend en RAM: `begin(256)` copia 256; `begin(0)`/`begin(5000)` -> 4096;
   `begin(300)` -> 512; `read(-1)`/`read(size)` -> 0; `write` igual no ensucia; `commit` limpio no borra;
   `commit` sucio borra 4096 y programa `size`; datos persistidos tras un nuevo `begin`;
   `kEepromFlashOffset == 0x3FE000`.
4. Descriptores: bytes exactos del descriptor de dispositivo (18) y de configuracion (75) de 4.7;
   cadena de serie de `{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF}` -> "0123456789ABCDEF".
5. `hal::constrain`: (0.5f,-1,1)->0.5; (2,-1,1)->1; NaN -> NaN; (int16_t 300, 0, 255) -> 255 tipo int.
Criterios de aceptacion: `probar.ps1` OK (suite, comparar_ensayo, hal_checks, lanzador); `probar.ps1
-Pico` sigue OK (Arduino intacto; esta parte no agrega `.cpp` en `src/`).
Commit: `Pico SDK parte 1: logica pura de la HAL y pruebas de host`.

### Parte 2 - HAL de destino y CMake base (sin tocar el `.ino`)

Alcance (archivos nuevos): `src/hal/rp2350/{time_gpio,pwm_adc,i2c,watchdog,eeprom_flash,usb_cdc,
usb_descriptors}.cpp` y `tusb_config.h` (4.2-4.9, **con la guarda D13**), `CMakeLists.txt` (solo
`myk_hal` STATIC, 5.1), `pico_sdk_import.cmake` (copia de `[SDK]/external/pico_sdk_import.cmake`),
`herramientas/compilacion/Build-PicoSdk.ps1` (provisorio: configura y compila un objetivo con los
parametros de 5.2 y la ruta de compilador de HANDOFF); modificado: `probar.ps1` (switch `-Sdk`).
Leer: secciones 2, 4, 5.1, 5.2 (parametros), 5.3; [ARD]/[SDK] solo las lineas citadas.
Comprobaciones en el `.cpp` de destino: `static_assert` de mascaras (4.8), de `kOffset == 0x3FE000` y
de `PICO_FLASH_SIZE_BYTES` (4.9).
Criterios de aceptacion: `probar.ps1` OK; `probar.ps1 -Sdk` OK (`myk_hal` compila para RP2350);
`probar.ps1 -Pico` sigue OK (prueba que la guarda D13 funciona).
Commit: `Pico SDK parte 2: HAL de destino y CMake base`.

### Parte 3 - cabeceras a `include/` y modulos con hardware sobre la HAL

Alcance: `git mv` de todos los `.h` de la raiz a `include/` (el `.ino` queda en la raiz y deja de
compilar con Arduino: es esperado); modificar `mis_defines.h` (quitar `Arduino.h`, ramas AVR segun U1,
pines Pico sin condicion), `MotorDriver.h`, `LcdTelemetry.h`, `YokeI2C.h`, `PicoEncoder.h`,
`AxisEncoders.h`, `AxisPotentiometers.h`, `AxisAs5600.h`, `CalibrationControls.h`, `UsbGuard.h`,
`LoopSupervisor.h` segun la tabla de 3 (cambios mecanicos: mismos nombres de metodo, mismas
constantes, mismo orden de llamadas; agregar `<stdint.h>/<stddef.h>/<string.h>` donde antes llegaban
por `Arduino.h`); rutas de `tests/` y `SimConnectBridge/` (5.4).
Pruebas host nuevas en `hal_checks` (compilan los modulos reales contra `hal_fake.cpp`):
1. `MotorDriver` (tras `hal::pwmSetup(490,255)`): en el registro de eventos de `hal_fake`, `begin()`
   escribe `pwmWrite(pin,0)` en los dos pines PWM antes de `digitalWrite(EN,true)`;
   `setCommand(0.5f,255)` -> PWM +128 -> nivel 1024 en pin derecho y 0 en izquierdo;
   `setCommand(-1,255)` -> -220 (limite duro) -> nivel 1760 en izquierdo. **Sin caso NaN**: la
   conversion `(int16_t)NaN` es comportamiento indefinido y da distinto en x86 (host) y ARM (destino);
   no se prueba ni se cambia en esta tarea.
2. `PicoEncoder`: con pares **(A,B)** = (0,0),(0,1),(1,1),(1,0),(0,0) (es decir, B adelantada a A) el
   contador suma +4; la secuencia inversa (0,0),(1,0),(1,1),(0,1),(0,0) resta 4 (tabla `delta[16]` sin
   cambio, indice `previo | actual<<2` con A en el bit 0). La prueba llama al manejador registrado por
   `attachChangeInterrupt` despues de cambiar el nivel simulado del pin.
3. `CalibrationControls`: `begin()` con imagen borrada (0xFF) conserva valores por defecto; guardar
   ganancias y releer con un `Controls` nuevo da los mismos valores; offsets 0, `YokePots::recordOffset`,
   `YokeRecord::recordOffset` sin solaparse.
4. `LcdTelemetry`: sin respuesta en 0x27 alterna a 0x3F en la siguiente sonda; con ACK recorre los
   estados hasta `Ready` con las esperas 100,5,5,1,1,1,1,1,1,3 ms (de `Probe` a `Finish`); la fila
   formateada para (0.5, 128) es `"RF+.5000 PW+128 "` (16 caracteres, `snprintf` de 17 trunca un espacio).
5. `AxisAs5600`: lectura de 3 bytes en 0x0B por `wire1` (roll) y `wire0` (pitch); fallo I2C -> `I2C` en
   `statusText`.
Criterios: `probar.ps1` OK, `probar.ps1 -Bridge` OK y `probar.ps1 -Sdk` OK (la HAL de destino sigue
compilando). No se exige `-Pico` (el `.ino` ya no compila). Registrar en HANDOFF que entre la parte 3 y
la 4 no se puede generar UF2.
Commit: `Pico SDK parte 3: cabeceras en include y modulos sobre la HAL`.

### Parte 4 - `src/main.cpp`, 5 variantes con CMake y scripts

Alcance:
1. `git mv MiYoke_PD.ino src/main.cpp` y adaptar segun la tabla de 3 y 4.11 (`main()` al final,
   `hal::pwmSetup` antes de `rollMotor.begin()`, sin `setID/detach/attach`, sin `yield()`, sin
   `Serial.begin`, `Serial` -> `hal::usbSerial`, `snprintf_P/PSTR` -> `snprintf`, sin
   `<pgmspace.h>`, con `#include <pico.h>` para `__not_in_flash_func` (no `pico/platform.h`: ver tabla de 3),
   `rp2040.getResetReason()`/`RP2040::WDT_RESET` -> `hal::resetReason()`/`hal::ResetReason::Wdt`
   convertido a `int` donde hoy se guarda en `bootReason`). Ninguna otra modificacion de logica.
   `main.cpp` conserva la guarda D13 solo si sigue en `src/` junto al `.ino`; como el `.ino` deja de
   existir en esta parte, `main.cpp` no la necesita.
2. `CMakeLists.txt`: 5 ejecutables de 5.1; quitar `myk_hal`.
3. `Build-Pico2.ps1` reescrito (5.2); borrar `Build-PicoSdk.ps1`; `probar.ps1` (5.3, sin `-Sdk`).
4. Marcador de raiz `src/main.cpp` en: `Build-Bridge.ps1:3`, `Build-Pico2.ps1`, `Build-WaitForFlight.ps1:3`,
   `probar.ps1:28`, `herramientas/ensayos/{ARRANQUE.ps1:7, BANCO.ps1:9, CICLOS.ps1:3, ENSAYO.ps1:7}`,
   `herramientas/flujo/{compilar-tesis.ps1:18, lanzar-codex.py:24-25, nueva-version.ps1:4}` y
   `graphify-sync.cmd` (quitar la linea `copy ... MiYoke_PD.ino ...`, queda solo `graphify update .`).
5. `ARRANQUE.ps1:10`: `MiYoke_PD.ino.elf` -> `MiYoke_PD.elf`. `FALLA.ps1:12-15`: buscar
   `arm-none-eabi-addr2line.exe` en `PICO_TOOLCHAIN_PATH` o en la ruta por defecto de 5.2 (ya no en
   Arduino15); mensaje de error acorde.
6. `.gitignore`: agregar `graphify-ino/`; borrar la carpeta `graphify-ino/` (no versionada).
7. Los puntos 1 y 4 van en el **mismo commit**: `lanzar-codex.py`, `nueva-version.ps1` y
   `compilar-tesis.ps1` dejan de funcionar si el `.ino` desaparece sin cambiar su marcador.
Pruebas: `probar.ps1 -Todo` OK (suite, hal_checks, lanzador, configurado, encoders, potenciometros,
AS5600, puente) y `probar.ps1 -PruebaFalla` OK, compilando desde un `BuildRoot` vacio. Ademas Codex
verifica, y copia a HANDOFF una linea por punto:
- en `build-pico2/MiYoke_PD.map`, `isr_hardfault` definido desde el objeto de `main.cpp` (no desde
  `crt0.S`); `__StackBottom = 0x20081800`, `__StackTop = 0x20082000`; `.stack_dummy` de tamano 0x800;
- `arm-none-eabi-nm build-pico2/MiYoke_PD.elf` lista `hardfaultCapture` y `isr_hardfault` con
  direccion en RAM (`0x2000xxxx`, por `__not_in_flash_func`), igual que en el `.map` Arduino;
- `picotool info -a build-pico2/MiYoke_PD.uf2` informa familia `rp2350-arm-s`; el UF2 contiene el
  bloque absoluto en `0x10ffff00` y ningun bloque en `0x103FE000-0x103FEFFF`;
- `MiYoke_PD.bin` < `0x3FE000` bytes (guarda de 4.9).
Commit: `Pico SDK parte 4: main.cpp, variantes CMake y scripts sin Arduino`.

### Parte 5 - documentacion, reglas de agentes y version

Alcance: seccion 9 (Documentacion afectada) salvo la tesis; `graphify update .`; commit por nombre de
los documentos; `powershell -ExecutionPolicy Bypass -File herramientas/flujo/nueva-version.ps1 -Mensaje
"Firmware migrado de Arduino-Pico al Pico SDK con CMake, HAL propia y estructura src/include"`;
verificar `Version Vx.y creada` y `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt`.
Criterios: `probar.ps1 -Todo` OK sobre el commit final; ningun archivo fuera de `tesis/`, `DISENO_*.md`,
`VPforce-TelemFFB-refactor/` y `Prepar3D-v3/` menciona `MiYoke_PD.ino`, `arduino-cli` ni `Arduino15`
como camino vigente (comprobar con grep y listar excepciones historicas en HANDOFF).

---

## 7. Ensayos con hardware (usuario)

**H0 (antes de cargar el firmware nuevo, con el UF2 Arduino actual V2.6):** anotar la linea base:
frecuencia y ciclo de trabajo PWM en GP6..GP9 para un PWM conocido; valores del menu (ganancias,
potes, registro); captura de descriptores USB (USB Device Tree Viewer o `Get-PnpDevice -PresentOnly |
? InstanceId -match 'VID_2E8A'` + COM asignado; con USB Device Tree Viewer guardar los bytes crudos
del descriptor de dispositivo y de configuracion y compararlos con los vectores de prueba de la parte 1:
si difieren, **los vectores se corrigen con la captura antes de la parte 4**); un `CICLOS.ps1`/`ENSAYO.ps1`
de referencia (lineas `SC8 ... us= lp= gap=`, `W,...`, `S,...`).
**Respaldo de la calibracion (obligatorio antes de cargar el UF2 nuevo):** placa en BOOTSEL y
`D:\pico\picotool\picotool\picotool.exe save -r 0x103FE000 0x103FF000 calib_V2_6.bin` (guardar fuera del
repo). Si H8 falla, se restaura con `picotool load calib_V2_6.bin -o 0x103FE000` (verificar la sintaxis
con `picotool help load` en ese momento) o recargando V2.6 y reingresando los valores anotados.

| # | Ensayo | Criterio |
|---|---|---|
| H1 | PWM medido (osciloscopio/analizador) en GP6..GP9 | 489,99 Hz +-0,5 %; ciclo = PWM/255; motor quieto con fuerza 0 (nivel 0 en ambos pines) |
| H2 | Sentido de motor roll y pitch (fuerza de centrado con desplazamiento) | Mismo sentido que H0 con `ROLL/PITCH_MOTOR_DIRECTION` sin cambio |
| H3 | Encoders (variante encoders): girar una vuelta del eje de roll | 4000 cuentas, mismo signo que antes; `x=` en la linea de depuracion |
| H4 | Potenciometros (variante potenciometros) | `a=` en los extremos y centro igual a H0 (+-4 cuentas) |
| H5 | AS5600 (variante as5600): normal, iman retirado, cable I2C suelto y reconectado | `s=OK`, `g=` AGC como H0; `SIN`/`I2C`/`BLQ` y fuerza cortada; recuperacion igual a hoy |
| H6 | LCD: encendido, desconexion y reconexion de alimentacion del LCD | Muestra filas; se recupera en <= 5 s; direccion 0x27/0x3F alternada; el FFB no se interrumpe |
| H7 | USB: Administrador de dispositivos, `Iniciar-Simulador.ps1 -DetectarPlaca`, `?` -> `YOKE17SC8`, sesion de 30 min con el puente, desconectar/reconectar cable, cerrar/abrir el puente (DTR) | `USB\VID_2E8A&PID_000F&MI_00`, mismo COM y mismos descriptores que H0; sin perdida de enlace; registros `pico.log` completos |
| H8 | Calibracion persistida | Tras cargar el UF2 nuevo, los valores del menu son los de H0; guardar un cambio, apagar y encender: se conserva; guardar durante una sesion: sin reset por watchdog (motores a 0 durante el guardado) |
| H9 | Watchdog, falla y motivo de reset | Variante prueba-falla: `BOOT reset=WDT ... etapa=15:PRUEBA falla=1 exc=...` y `FALLA.ps1` decodifica `pc` con el `.elf` nuevo; **durante la falla (antes del reinicio) EN de ambos puentes y los 4 PWM en 0, medido con osciloscopio o LED en EN**; desenchufar/enchufar: `reset=ENCENDIDO`; boton RUN: `reset=PIN_RUN` (nombres de `YokeHealth::resetName`, `LoopSupervisor.h:81-92`) |
| H10 | Tiempo de ciclo y pila | `CICLOS.ps1`: `lp`, `gap` y lineas `W` comparables a H0 (sin ciclos lentos nuevos); `S,` (pila libre) >= H0 - 64 bytes |
| H11 | Toque a 1200 baudios (`mode COMx BAUD=1200`, abrir y cerrar) | La placa entra en BOOTSEL |
| H13 | Arranque y transiciones seguras, **con la fuente de potencia del puente H conectada pero el motor desacoplado o con el yugo libre**: osciloscopio en EN (GP10/GP11) y PWM (GP6..GP9) durante: encendido en frio, boton RUN, carga del UF2 por BOOTSEL, toque a 1200 y reinicio por watchdog | Ningun pulso en PWM con EN alto; EN sube solo despues de que ambos PWM esten en nivel 0 (orden de `MotorDriver::begin`); en reset/BOOTSEL todo queda en bajo. Hacer H13 **antes** que H2 |
| H12 | Coherencia de modelos: `ENSAYO.ps1` con repeticion + `comparar_ensayo` | Mismo resultado OK que H0 (tolerancias actuales) |

---

## 8. Riesgos

| # | Riesgo | Mitigacion |
|---|---|---|
| R1 | Descriptor USB distinto -> Windows asigna otro COM o el lanzador no encuentra la placa | Bytes fijados en 4.7 y probados en host; H0/H7 comparan con captura previa |
| R2 | TinyUSB 0.18.0 se comporta distinto que 0.20.1 (flush automatico al llenar un paquete, FIFO TX sobrescribible con DTR=0, manejo de DTR) | Misma API; `write` exige DTR (2.1-3); el lazo vacia con `tud_cdc_write_flush` en cada vuelta; H7 de 30 min. **Actualizar el SDK a 2.3.0 NO resuelve esto**: el `lib/tinyusb` del SDK 2.3.0 del core es 0.18.0, identico byte a byte (sin CR) al de 2.2.0. Para usar 0.20.1 hay que pasar `PICO_TINYUSB_PATH` a un clon completo de TinyUSB en la etiqueta `0.20.1` (el SDK exige `hw/bsp/rp2040`, [SDK `src/rp2_common/tinyusb/CMakeLists.txt:1-18`]; la copia de Adafruit no sirve porque no trae `hw/`). Ver U6 |
| R3 | Compilador distinto del actual (GCC 16.1.0 de `pqt-gcc 5.0.0-9576866`): diferencias de ultimo bit en flotantes (contraccion FMA, orden de evaluacion) y de uso de pila | Fuente de modelos sin cambio; `comparar_ensayo` (H12) con sus tolerancias; H10 para pila. Se elimina usando el mismo GCC 16.1.0 (U3 opcion B) |
| R4 | Reimplementacion de I2C (sonda bit-bang, recuperacion de bus) con otro tiempo | Semantica detallada en 4.6; pruebas host de LCD/AS5600; H5/H6 |
| R5 | Borrado de flash con interrupciones deshabilitadas: USB sin servicio ~decenas a cientos de ms | Igual que hoy (`noInterrupts()` en `EEPROM::commit`); watchdog a 3000 ms durante el guardado; H8 |
| R6 | MAX_PATH en el directorio de build | `BuildRoot` corto en `%TEMP%`; parte 4 compila las 5 variantes desde cero |
| R7 | `isr_hardfault`, `__StackBottom`/`__StackTop` o tamano de pila distintos -> diagnostico de falla roto | Mismos simbolos en el linker del SDK; verificacion con `nm`/`.map` en la parte 4; H9/H10 |
| R8 | El bloque `--abs-block` (errata RP2350-E10) toca el ultimo sector | La calibracion esta en el penultimo (`0x3FE000`), no en el ultimo; el build Arduino ya emite el mismo bloque. Respaldo `picotool save` en H0; H8 lo confirma |
| R13 | Sector de calibracion mal ubicado (error ya cometido en la version previa de este diseno: `0x3FF000`) | `static_assert(kOffset == 0x3FE000)`, prueba de host de `kEepromFlashOffset`, verificacion del UF2 en la parte 4, respaldo en H0, H8 |
| R14 | `arduino-cli` compila `src/` del sketch mientras exista el `.ino` (partes 1-3) | Guarda D13; criterio `-Pico` OK en las partes 1 y 2 |
| R15 | `tud_cdc_line_state_cb` / descriptores definidos dos veces (HAL + otra fuente) o `pico_stdio_usb` enlazado por error | `pico_enable_stdio_usb(... 0)`; no enlazar `pico_stdio_usb`; el enlace falla si hay duplicados (detectado en la parte 2/4) |
| R9 | `printf` de newlib vs `pico_printf` (formato de lineas de registro) | Se fija `compiler` (newlib, como hoy); los formatos son solo enteros/cadenas; `picolog_checks` y H12 |
| R10 | SDK 2.2.0 vs 2.3.0 (core actual) en `runtime_init`, watchdog o I2C | Comparado (`diff -r --strip-trailing-cr`): `hardware_watchdog`, `hardware_i2c`, `hardware_pwm` solo cambian comentarios y macros equivalentes; `hardware_flash` agrega soporte de CS1 (sin efecto en CS0); `hardware_gpio`, `hardware_irq`, `hardware_clocks`, `pico_crt0`, `pico_runtime_init`, `pico_standard_link` tienen cambios no revisados linea a linea. Mitigacion: H1, H5, H9, H13; se elimina con U6 opcion 2.3.0 |
| R11 | Pines EN habilitados con PWM sin configurar | `pwmSetup` antes de `MotorDriver::begin`, que pone PWM 0 antes de EN alto (sin cambio); antes de `setup()` los GPIO estan en reset (sin salida) |
| R12 | Licencia: Arduino-Pico y Adafruit TinyUSB son LGPL-2.1/MIT | Se reescribe a partir de la semantica documentada; no copiar codigo literal LGPL |

---

## 9. Documentacion afectada

Solo las partes indicadas (regla DOCUMENTACION RELACIONADA). No se tocan los `DISENO_*.md` de otras
tareas ni los `ACTUALIZACION_Vx_y.txt`.

| Archivo | Parte a cambiar | Que debe decir |
|---|---|---|
| `LEEME md/LEEME_PICO2.md` | "Configuracion de Arduino IDE" (l. 12-27) y lista de UF2 (l. 30-39) | Reemplazar por "Compilacion con Pico SDK": requisitos (SDK 2.2.0, Arm GNU 12.2, CMake, Ninja, picotool), `Build-Pico2.ps1` y sus parametros/variables de entorno, carpeta de build corta, nombres `build-pico2*/MiYoke_PD.uf2`, carga por BOOTSEL y toque a 1200 baudios. La referencia al sketch `.ino` pasa a `src/main.cpp` |
| `LEEME md/INVENTARIO_CODIGO.md` | Metodo (l. 4), seccion 1 (l. 14-33), filas `Build-Pico2.ps1` (l. 93), `graphify-sync.cmd` (l. 104), `graphify-ino/` (l. 122) | Firmware en `src/main.cpp` + `include/` + `include/hal/` + `src/hal/rp2350/`; `CMakeLists.txt`; `Build-Pico2.ps1` usa CMake/Ninja; `graphify-sync.cmd` solo actualiza el grafo; quitar `graphify-ino/`; agregar `tests/hal_checks.*` |
| `LEEME md/ENSAYOS.md` | l. 85-90 | `MiYoke_PD.uf2` / `MiYoke_PD.elf` en lugar de `MiYoke_PD.ino.*`; `FALLA.ps1` usa addr2line de la Arm GNU Toolchain |
| `LEEME md/MENU_CALIBRACION.md` | l. 122-133 | Compilacion con `Build-Pico2.ps1` (CMake); persistencia: "penultimo sector de 4 KiB de la flash (0x103FE000), mismo sector y formato que la EEPROM emulada anterior; la calibracion se conserva al migrar" |
| `LEEME md/LEEME_POTENCIOMETROS.md` | l. 10-11 y l. 30, 82 | UF2 `build-pico2-potentiometers/MiYoke_PD.uf2`; lectura ADC de 12 bits con el Pico SDK (`adc_read`); compilacion Pico SDK en lugar de "Core Arduino-Pico 6.1.1" |
| `LEEME md/LEEME_AS5600.md` | Menciones a `Wire`/`Wire1` y compilacion (ubicar con grep `Wire\|Build-Pico2\|Arduino`) | Buses `hal::wire0`/`hal::wire1` con la misma velocidad y timeouts; compilacion Pico SDK |
| `LEEME md/LEEME-LANZADOR.md` | l. 21 | UF2 `build-pico2/MiYoke_PD.uf2` |
| `LEEME md/LEEME.md` (y `LEEME*.md` de la raiz si existen) | Menciones a Arduino IDE / `.ino` como forma vigente de compilar (grep) | Pico SDK + `Build-Pico2.ps1` |
| `.claude/skills/depuracion-sistematica/SKILL.md` | Menciones a `MiYoke_PD.ino` / `build-pico2/*.ino.*` | `src/main.cpp`, `MiYoke_PD.elf/.uf2` |
| `CLAUDE.md` y `AGENTS.md` | ESTRUCTURA DE CARPETAS (primer punto y "Generadas e ignoradas"); "Limitaciones del grafo" | Raiz: `CMakeLists.txt`, `pico_sdk_import.cmake`, lanzador y reglas; firmware en `src/` (main.cpp, hal/rp2350) e `include/` (cabeceras y hal/); el puente y `tests/` incluyen `../include/X.h`; se borra "Arduino exige que esten juntos"; build de CMake en `%TEMP%\myk` (o `MIYOKE_BUILD_ROOT`) y artefactos en `build-pico2*/`; se quita la linea "MiYoke_PD.ino se indexa via copia en graphify-ino/" y la referencia a `graphify-ino/`; "leer src/main.cpp directamente" en lugar de `.ino`. En AHORRO DE TOKENS: no invocar `cmake`/`ninja` directamente (en lugar de `arduino-cli`). En VERSIONADO la lista de extensiones queda igual salvo decision U4 |
| Tesis | - | `tesis/capitulos/NN_*.tex (menciones a Arduino como plataforma/framework/compilacion, sobre todo 01_arquitectura_sc8.tex): la actualiza Claude al aprobar la revision; Codex no la toca` |

Documentos historicos que mencionan Arduino AVR/Micro (`LEEME_LCD.md`, `LEEME_ENCODERS.md`,
`LEEME_ETAPAS.md`, `LEEME_AMORTIGUACION.md`, `MODELO_TURBULENCIA.md`, `EFECTOS_SIMCONNECT.md`,
`MOBIFLIGHT_PICO_Y_UNO.md`): describen etapas pasadas o el banco con Arduino Uno (hardware externo que
sigue existiendo); **no se modifican**, salvo que U1 se resuelva eliminando el soporte AVR, en cuyo caso
se agrega una sola linea al principio de `LEEME_ENCODERS.md` y `LEEME_ETAPAS.md`: "Historico: el
firmware actual solo compila para Pico 2 con el Pico SDK (ver LEEME_PICO2.md)".

---

## 10. Decisiones que requieren al usuario

- **U1 - Soporte AVR (Pro Micro):** sin Arduino no se puede compilar para AVR. Recomendado: eliminar
  las ramas AVR de `mis_defines.h`, `AxisEncoders.h`, `LcdTelemetry.h`, `MiYoke_PD.ino`. Alternativa:
  conservarlas como codigo muerto (no recomendado).
- **U2 - Nombre del UF2:** `MiYoke_PD.uf2` (recomendado) en lugar de `MiYoke_PD.ino.uf2`. El usuario
  carga el archivo a mano: cambia el nombre que busca.
- **U3 - Compilador** (antes de la parte 2). Versiones encontradas: hoy el firmware se compila con
  `pqt-gcc 5.0.0-9576866` = **GCC 16.1.0** (`%LOCALAPPDATA%\Arduino15\packages\rp2040\tools\pqt-gcc\
  5.0.0-9576866`, trae `addr2line`, `nm`, `size`, `objcopy`); instalado aparte: Arm GNU Toolchain
  **12.2.MPACBTI-Rel1** (con el que ya compila `D:\pico\hello` contra SDK 2.2.0). No hay Arm GNU 14.x/15.x
  instalada, y ninguna de esas versiones iguala a 16.1, asi que instalar una no aporta paridad.
  - Opcion A: Arm GNU 12.2 (probada con el SDK; diferencias de ultimo bit validadas con H12).
  - **Opcion B (recomendada por paridad):** copiar `pqt-gcc 5.0.0-9576866` completo a
    `D:\pico\toolchains\pqt-gcc-5.0.0` (fuera de Arduino15, para sobrevivir a U5) y usarlo como
    `PICO_TOOLCHAIN_PATH`: mismo compilador y misma newlib que hoy, elimina R3. Riesgo: SDK 2.2.0 no
    esta probado con GCC 16 (2.3.0 si, porque el core lo compila); puede dar advertencias nuevas. Si
    la parte 2 falla por el compilador, Codex lo anota y Claude decide.
- **U6 - Version del SDK y de TinyUSB** (antes de la parte 2):
  - SDK 2.2.0 instalado (recomendado si se quiere empezar ya) o **SDK 2.3.0** = el mismo que compila el
    firmware hoy (elimina R10; requiere clonar la etiqueta `2.3.0` con submodulos y picotool/pioasm
    2.3.x). Cualquiera de los dos trae TinyUSB 0.18.0.
  - TinyUSB 0.18.0 del SDK (recomendado, H7 lo valida) o 0.20.1 via `PICO_TINYUSB_PATH` (paridad con
    hoy, pero version de TinyUSB no probada por Raspberry Pi con ese SDK).
- **U7 - `tesis/PDFs Extras/` sin seguimiento:** bloquea `nueva-version.ps1` (parte 5). Commitear,
  ignorar en `.gitignore` o mover fuera del repo.
- **U4 - Versionado:** agregar `CMakeLists.txt`, `*.cmake` y `tusb_config.h` (ya cubierto por `.h`)
  a la lista de tipos que generan version en `nueva-version.ps1` y en CLAUDE.md/AGENTS.md. Recomendado:
  si (`CMakeLists.txt|\.cmake$`), porque cambiar opciones de compilacion cambia el firmware.
- **U5 - Desinstalar Arduino15:** recien despues de H1-H13 aprobados (hoy `FALLA.ps1` lo usa; la
  parte 4 lo independiza). Con U3 opcion B, copiar antes `pqt-gcc` fuera de Arduino15.
- Calibracion: no requiere decision; se conserva (4.9), confirmar con H8.
- USB: no requiere decision; el unico cambio es que ya no hay re-enumeracion inicial (2.1-1).

### 10.1 Resoluciones del usuario (2026-11-02) - vinculantes para Codex

- **U1: eliminar** las ramas AVR (Pro Micro) en la parte 3 (y en el `.ino` en la parte 4).
- **U2: `MiYoke_PD.uf2`**, en las mismas carpetas `build-pico2*/` de siempre.
- **U3: opcion B.** `pqt-gcc 5.0.0-9576866` (GCC 16.1.0) ya copiado por Claude a
  `D:/pico/toolchains/pqt-gcc-5.0.0` (con `bin/arm-none-eabi-addr2line.exe`). `PICO_TOOLCHAIN_PATH =
  D:/pico/toolchains/pqt-gcc-5.0.0`. Verificado por Claude: un programa con `pico_stdlib` + stdio USB
  compila para `pico2` con SDK 2.2.0 y GCC 16.1 sin advertencias.
- **U6: SDK 2.2.0 + TinyUSB 0.20.1.** No existe etiqueta `0.20.1` en el repositorio de TinyUSB: la
  "0.20.1" de Adafruit_TinyUSB_Arduino corresponde al commit upstream **`e954302c1`** (2026-05-04,
  `tusb_option.h` = 0.20.1), identico (0 lineas distintas, `diff -w`) a la copia del core en
  `src/class/cdc/`, `src/device/usbd.c` y el `dcd` de RP2040. Clon completo (con `hw/bsp/rp2040`) en
  `D:/pico/tinyusb`, en ese commit. Se pasa `-DPICO_TINYUSB_PATH=D:/pico/tinyusb`; verificado por
  Claude que el SDK lo detecta ("TinyUSB available at D:/pico/tinyusb/hw/bsp/rp2040") y compila.
  `Build-Pico2.ps1`/`Build-PicoSdk.ps1` agregan el parametro `-TinyUsbPath` (defecto
  `D:/pico/tinyusb`) junto a los de 5.2. Las mitigaciones de R2 (DTR en `write`, flush por vuelta)
  se mantienen: replican el comportamiento de Adafruit, no dependen de la version.
- **U4: si.** `CMakeLists.txt` y `*.cmake` generan version (parte 5: `nueva-version.ps1`, CLAUDE.md,
  AGENTS.md).
- **U5: no se desinstala Arduino15** en esta tarea; el firmware nuevo no debe depender de el.
- **U7: `tesis/PDFs Extras/` va a `.gitignore`** (lo hizo Claude antes de la parte 1).

---

## 11. Fuentes

Codigo local verificado (2026-11-02):
- [ARD] `cores/rp2040/main.cpp:37,74-77,104-114,148,181-185`; `cores/rp2040/delay.cpp:50-64`;
  `cores/rp2040/wiring_analog.cpp:79-160`; `cores/rp2040/wiring_digital.cpp` (`__pinMode`,
  `__digitalWrite`, `__digitalRead`); `cores/rp2040/wiring_private.cpp:121-171`;
  `cores/rp2040/RP2040Support.h:115-138,582-654`; `ArduinoCore-API/api/Common.h:67`;
  `libraries/EEPROM/src/EEPROM.cpp:43-131`; `libraries/Wire/src/Wire.cpp:43-135,250-298,307-436,
  442-500,742-754`; `libraries/Wire/src/Wire.h:38-42,141`; `variants/rpipico2/pins_arduino.h:30-31`;
  `libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/Adafruit_TinyUSB_rp2040.cpp:72-140`;
  `.../ports/rp2040/tusb_config_rp2040.h`; `.../arduino/Adafruit_USBD_Device.cpp:46-90,112-260,280-296`;
  `.../arduino/Adafruit_USBD_Device.h:108-109`; `.../arduino/Adafruit_USBD_CDC.cpp:49-107,165-290`;
  `.../arduino/Adafruit_TinyUSB_API.cpp:40-60`; `boards.txt:641-663,715-730,757-787,822-826`;
  `platform.txt:46-68,100-108,149`; `lib/rp2350/memmap_default.ld:29-36,276-311`;
  `pico-sdk/pico_sdk_version.cmake:4-10` (2.3.0).
- [SDK] `pico_sdk_version.cmake:4-10` (2.2.0); `src/rp2_common/pico_stdio_usb/stdio_usb.c:60-138`;
  `lib/tinyusb/src/portable/raspberrypi/rp2040/dcd_rp2040.c:367`; `lib/tinyusb/src/tusb_option.h:33-35`
  (0.18.0); `lib/tinyusb/src/tusb.h:134-149`; `tools/CMakeLists.txt:594-605`;
  `src/rp2_common/hardware_pwm/include/hardware/pwm.h:161-168`;
  `src/rp2350/pico_platform/include/pico/platform.h:63-66`;
  `src/rp2_common/hardware_watchdog/watchdog.c:116-127`;
  `src/rp2350/hardware_regs/include/hardware/regs/powman.h:528-711`;
  `src/rp2350/hardware_regs/include/hardware/regs/watchdog.h:99-106`;
  `src/rp2_common/pico_crt0/crt0.S:604`; `src/rp2_common/pico_crt0/rp2350/memmap_default.ld`.
- [REPO] `MiYoke_PD.ino` (completo); `mis_defines.h:12-30,66-68,127-200,204-214`;
  `herramientas/compilacion/Build-Pico2.ps1`, `probar.ps1`; `herramientas/ensayos/ARRANQUE.ps1:7-11`,
  `FALLA.ps1:8-15`; `Iniciar-Simulador.ps1:49-102`; `build-pico2/MiYoke_PD.ino.map:6454-6480`;
  `build-pico2/MiYoke_PD.ino.uf2` (bloques 0 y 1..502); `LEEME md/DISENO_usb_id_pico2.md`;
  `LEEME md/DISENO_recuperacion_enlace_usb.md` 1.2 y 3.3.

Documentacion:
- Raspberry Pi, "Raspberry Pi Pico-series C/C++ SDK" (APIs `hardware_pwm`, `hardware_i2c`,
  `hardware_adc`, `hardware_gpio`, `hardware_flash`, `hardware_watchdog`, `hardware_irq`,
  `pico_unique_id`, `pico_bootrom`, `pico_stdio_usb`, `pico_add_extra_outputs`,
  `pico_set_printf_implementation`): https://www.raspberrypi.com/documentation/pico-sdk/
- Raspberry Pi, "RP2350 Datasheet": capitulos PWM, I2C, ADC, Watchdog, POWMAN (CHIP_RESET), QMI/XIP
  (escritura de flash), Bootrom/UF2 y erratas (RP2350-E10).
  https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
- picotool, opcion `uf2 convert --abs-block`: https://github.com/raspberrypi/picotool
- TinyUSB, API de dispositivo CDC (`tud_cdc_*`, `tud_cdc_line_state_cb`) y configuracion
  (`tusb_config.h`): https://docs.tinyusb.org/
- Arduino-Pico (documentacion de usuario de `analogWrite`, `EEPROM`, `Wire`):
  https://arduino-pico.readthedocs.io/

---

## 12. Revision del plan

Critica roja (Master Plan Architect, 2026-11-02), verificada contra [REPO], [ARD] y [SDK]. Cambios
aplicados en este documento:

**Graves (corregidos):**
1. **Sector de calibracion equivocado.** `272621568` es `0x103FE000`, no `0x103FF000`; el `.map` actual
   lo confirma (`_EEPROM_start = 0x103fe000`). Con el valor anterior el firmware nuevo habria leido un
   sector borrado (perdida de calibracion) y escrito en otro. Corregidos D4, 4.9 (`kOffset =
   PICO_FLASH_SIZE_BYTES - 2*FLASH_SECTOR_SIZE`), guarda de tamano, R8, fila de MENU_CALIBRACION.md;
   agregados R13, prueba `kEepromFlashOffset`, verificacion del UF2 en la parte 4 y respaldo con
   `picotool save` en H0.
2. **`probar.ps1 -Pico` habria fallado al agregar `src/hal/rp2350/*.cpp`**: `arduino-cli` compila la
   subcarpeta `src/` del sketch (callbacks TinyUSB duplicados con Adafruit). Agregados D13 (guarda
   `#if defined(YOKE_TARGET_RP2350)`) y R14.
3. **`write` USB sin condicion de DTR**: Adafruit no escribe con DTR=0 y TinyUSB 0.18 deja la FIFO TX
   sobrescribible con DTR=0; la respuesta `YOKE17SC8` no mira DTR. 2.1-3 y 4.7 exigen
   `if (!tud_cdc_connected()) return 0;`.
4. **R2 mal mitigado**: subir a SDK 2.3.0 no trae TinyUSB 0.20.1 (el `lib/tinyusb` del 2.3.0 del core
   es 0.18.0, identico al del 2.2.0). Reescritos 2.1-6, R2 y nueva decision U6.

**Correcciones de exactitud:** macro `PICO_PWM_CLKDIV_ROUND_NEAREST` (no `PICO_CLKDIV_...`);
`digitalWrite` siempre llama `gpio_set_function(SIO)` primero; la sonda I2C solo espera 5 us tras los
8 bits de direccion y no usa el timeout; `setSDA/SCL` con mismo pin en marcha devuelve `true`;
`rp2040.begin(0)` instala SysTick (2.1-7); `available()` sin `yield` (2.1-8); nombres de reset en H9
(`ENCENDIDO`, no `POR/PWRON`); `PwmPlan.top` renombrado `scale` (no es el registro TOP); referencia
"ver 7.3" inexistente corregida; `__flash_binary_end` reemplazado por el tamano del `.bin` (el simbolo
es `PROVIDE`); el borrado de `MiYoke_PD.ino.*` no toca los `MiYoke_PD_V2.x.ino.*` historicos.

**Huecos cubiertos:** `mis_defines.h:24-30` (`YOKE_CALIBRATION_ENABLED` por defecto 1), `main.cpp`
necesita `<pico/platform.h>`, `TinyUSB_Device_FlushCDC`/`pgmspace.h` en la tabla, `UsbGuard.h` y la
excepcion de `LoopSupervisor.h` a la regla de 4.1, declaracion de `FlashBackend` para host y destino,
registro ordenado de eventos en `hal_fake`, `PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64` (D14),
`picotool`/`pioasm` deben acompanar la version del SDK, commit unico de `.ino`->`main.cpp` con los
marcadores de `lanzar-codex.py`/`nueva-version.ps1`, `tesis/PDFs Extras/` bloquea `nueva-version.ps1`
(U7).

**Partes:** de 4 a 5 (la parte 1 anterior excedia una sesion): 1 logica pura + host, 2 HAL de destino +
CMake (`-Sdk`, `-Pico` sigue OK), 3 cabeceras, 4 `main.cpp` + scripts, 5 documentacion + version.
Criterios de la parte 4 hechos verificables (simbolos y direcciones exactas en `.map`/`nm`, bloques del
UF2, tamano del `.bin`).

**Pruebas:** quitado el caso `setCommand(NaN)` (UB, distinto en x86 y ARM); encoder con notacion
explicita de pares (A,B) (la anterior era ambigua y con una lectura daba -4); vectores de descriptores
USB a confirmar con la captura cruda de H0.

**Seguridad del motor y ensayos:** H13 nuevo (osciloscopio en EN/PWM en encendido, RUN, BOOTSEL, toque
a 1200 y watchdog, antes de H2); H9 exige medir EN y PWM en 0 durante la falla; H0 agrega respaldo del
sector de calibracion y captura cruda de descriptores.

**Verificado sin cambios:** algoritmo PWM (escala 2040, PS=4, SS=1, div 150+1/16, 489,99 Hz);
mascaras y orden de `getResetReason`; descriptores Adafruit (clase 0xEF/02/01, potencia 100 mA, EP
0x81/0x01/0x82, cadena 4 "TinyUSB Serial", serie en mayusculas); semantica EEPROM (`begin`, `commit`);
I2C `begin/end/endTransmission/requestFrom/handleTimeout`; pila 0x800 en SCRATCH_Y; `isr_hardfault`
debil en crt0; reloj 150 MHz (primera opcion del menu `freq`); bloque `--abs-block` tambien en un UF2
del SDK 2.2.0; esperas del LCD 100,5,5,1,1,1,1,1,1,3 ms; sin formatos `%f` (newlib sin
`_printf_float` alcanza).

**Pendiente de decision del usuario:** U1, U2, U3 (A: Arm GNU 12.2 / B: pqt-gcc GCC 16.1.0 copiado,
recomendada por paridad), U4, U5, U6 (SDK 2.2.0 o 2.3.0; TinyUSB 0.18.0 o 0.20.1), U7.
