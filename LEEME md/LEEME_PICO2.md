> Antecedente de versiones anteriores. Los ajustes y seleccion de fuente vigentes estan en [AJUSTE_FUERZAS_Y_MOVIMIENTO.md](AJUSTE_FUERZAS_Y_MOVIMIENTO.md). Usar YOKE_POSITION_SOURCE (1=simulacion, 0=encoders, 2=potenciometros ADC en Pico, 3=AS5600; ver LEEME_AS5600.md); ver LEEME_POTENCIOMETROS.md para el modo 2 actual. Los valores de ganancias y banderas anteriores de este documento ya no aplican.

> Version actual: fuerzas solo por SimConnect; sin CONSTANT/SINE ni centrado. Ver [LEEME_SIMCONNECT.md](LEEME_SIMCONNECT.md).

# MiYoke en Raspberry Pi Pico 2 (RP2350)

El mismo firmware (`src/main.cpp`) selecciona automaticamente el hardware
segun la placa elegida. La configuracion predeterminada sigue siendo dos
potenciometros (`USE_POTENTIOMETERS = 1`). Tambien se adapto el modo encoders.
No se necesita MicroPython ni un sistema operativo en la Pico.

## Compilacion con Pico SDK

El firmware se compila con el Pico SDK y CMake, sin Arduino. Programa principal:
`src/main.cpp`; cabeceras en `include/` y `include/hal/`; implementacion del
hardware en `src/hal/rp2350/`; `CMakeLists.txt` y `pico_sdk_import.cmake` en la raiz.

Requisitos (rutas por defecto de `Build-Pico2.ps1`):

- Pico SDK 2.2.0 en `D:/pico/pico-sdk` (`PICO_SDK_PATH`).
- TinyUSB 0.20.1 (commit `e954302c1`) en `D:/pico/tinyusb` (`PICO_TINYUSB_PATH`).
- Compilador pqt-gcc 5.0.0 (GCC 16.1.0) en `D:/pico/toolchains/pqt-gcc-5.0.0` (`PICO_TOOLCHAIN_PATH`).
- picotool en `D:/pico/picotool/picotool` y pioasm en `D:/pico/sdk-tools/pioasm`.
- CMake 4.4 (`C:/Program Files/CMake`) y Ninja (instalado con winget).

Lo normal es compilar con `probar.ps1`, desde la raiz del repositorio:

```powershell
powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1 -Pico
```

Opciones de variante: `-Encoders`, `-Potentiometers`, `-As5600`, `-Topes`, `-PruebaFalla`;
`-Todo` incluye todo menos `-PruebaFalla`, y `-Bridge` el puente. `Build-Pico2.ps1`
separa `-Topes` en `-Topes` y `-TopesAs5600`. `probar.ps1` llama a
`herramientas/compilacion/Build-Pico2.ps1`, que admite los demas interruptores
de variante y estos parametros: `-SdkPath`, `-TinyUsbPath`, `-ToolchainBin`,
`-CMake`, `-Ninja`, `-PicotoolDir`, `-PioasmDir` y `-BuildRoot`.

- La compilacion de CMake se hace en `%TEMP%\myk` (o `MIYOKE_BUILD_ROOT`):
  ruta corta para no superar el limite MAX_PATH de Windows.
- Si cambian el SDK, TinyUSB o el compilador respecto de la cache, reconfigura solo.
- Antes de copiar comprueba que el `.bin` no alcance 0x3FE000 (sector de calibracion).
- Copia `MiYoke_PD.uf2`, `.elf`, `.bin` y `.map` a la carpeta de la variante.
- Tamano de la variante configurada (modo 1): flash 65460 bytes, RAM 8608 bytes.

Binarios generados:

- `build-pico2/MiYoke_PD.uf2`: configuracion de `mis_defines.h` (hoy YOKE_POSITION_SOURCE=1, posicion desde SimConnect).
- `build-pico2-encoders/MiYoke_PD.uf2`: fuerza encoders (modo 0).
- `build-pico2-potentiometers/MiYoke_PD.uf2`: fuerza potenciometros (modo 2).
- `build-pico2-as5600/MiYoke_PD.uf2`: fuerza AS5600 (modo 3).
- `build-pico2-topes/MiYoke_PD.uf2`: simulacion (modo 1) con calibracion de topes por finales de carrera.
- `build-pico2-topes-as5600/MiYoke_PD.uf2`: AS5600 (modo 3) con calibracion de topes.
- `build-pico2-prueba-falla/MiYoke_PD.uf2`: HardFault provocado a los 20 s, solo para validar el diagnostico; no usar para volar.

Los binarios reflejan la configuracion del momento de compilacion. Si se cambian
pines, ganancias o calibracion, hay que volver a compilar. Ningun comando carga
ni activa la placa.

Carga: conectar por USB manteniendo BOOTSEL, soltarlo y copiar el `.uf2` a la
unidad **RP2350**. La placa se reinicia y presenta el puerto serie USB CDC, sin
joystick HID. Con el firmware cargado, abrir el puerto a 1200 baudios reinicia la
Pico en modo BOOTSEL sin tocar el boton.

## Nuevo cableado

Los numeros en el codigo son **GPIO (GP)**. No confundirlos con el numero fisico
de la pata del conector. Este pinout corresponde a Raspberry Pi Pico 2.

| Funcion | GPIO | Pata fisica |
| --- | --- | --- |
| Potenciometro Roll, cursor | GP26 / ADC0 | 31 |
| Potenciometro Pitch, cursor | GP27 / ADC1 | 32 |
| Encoder Roll A (modo alternativo) | GP0 | 1 |
| Encoder Roll B (modo alternativo) | GP1 | 2 |
| Encoder Pitch A (modo alternativo) | GP2 | 4 |
| Encoder Pitch B (modo alternativo) | GP3 | 5 |
| Finales de carrera (`YOKE_ENDSTOP_CALIBRATION 1`): Roll NEG, Roll POS, Pitch NEG, Pitch POS | GP0, GP1, GP2, GP3 | 1, 2, 4, 5 |
| LCD SDA | GP4 | 6 |
| LCD SCL | GP5 | 7 |
| Roll PWM LEFT | GP6 | 9 |
| Roll PWM RIGHT | GP7 | 10 |
| Pitch PWM LEFT | GP8 | 11 |
| Pitch PWM RIGHT | GP9 | 12 |
| Roll enable | GP10 | 14 |
| Pitch enable | GP11 | 15 |
| Alimentacion de potenciometros | 3V3 OUT | 36 |
| Masa analogica de potenciometros | AGND | 33 |
| Masa comun de drivers y adaptadores | GND | Por ejemplo 38 |

**Adaptar la electronica a 3,3 V antes de conectar.** Alimentar los extremos de
los potenciometros entre 3V3 OUT y AGND; nunca aplicar 5 V al ADC. Los valores
0..1023 se conservan por software, pero el fondo de escala electrico ahora es
3,3 V: volver a medir centro y extremos e introducirlos en `mis_defines.h`.

Si los encoders entregan 5 V, adaptar sus salidas A/B a 3,3 V. Para colector
abierto, usar pull-ups a 3,3 V compatibles con su salida y longitud de cable.
Los pull-ups internos estan habilitados. Verificar el sentido y cambiar
`ROLL_ENCODER_DIRECTION` o `PITCH_ENCODER_DIRECTION` si es necesario.

Si el LCD/PCF8574 se alimenta a 5 V y tiene pull-ups a 5 V, usar un adaptador
**I2C bidireccional 3,3 V / 5 V** entre GP4/GP5 y el modulo. No llevar esas
resistencias directamente al bus de la Pico. Se conserva la direccion 0x27.

PWM y enable ahora salen a 3,3 V: comprobar que el modelo concreto del puente H
reconozca ese nivel como HIGH; si no, adaptar niveles. Mantener alimentacion de
motores separada y masa comun. Si no se usan enables, configurar sus pines en -1.

## Implementacion actual

- TinyUSB mantiene el puerto serie USB CDC sin registrar una interfaz joystick HID.
- El firmware recibe SimConnect por CDC. Otra placa envia los ejes a Prepar3D.
- La lectura de posicion y la velocidad filtrada (YokeMotion.h) siguen activas a 2 ms para diagnostico y futuras funciones locales.
- Se conservan ADC a 10 bits, encoders x4, LCD y sus reintentos, mapas de pines y PWM a 490 Hz con rango 0..255.
- Las fuerzas son exclusivamente las de SimConnect. No hay centrado ni amortiguacion local.
- La identificacion serie sigue siendo `YOKE17SC2`; Windows puede reasignar el COM al reenumerar USB.
- `Build-Pico2.ps1` compila y exporta el UF2 sin cargar la placa.

## Comprobacion en el equipo real

1. Cargar primero con la potencia de motores desconectada. Abrir el Monitor Serial
   y comprobar posicion y velocidad de ambos sensores. En modo encoder, encender centrado.
   Esta Pico ya no aparece como joystick en joy.cpl; los ejes del simulador vienen de la otra placa.
2. Ajustar calibraciones y sentidos en `mis_defines.h` y volver a cargar.
3. Verificar LCD y telemetria. Para SimConnect, usar el nuevo COM y cerrar el
   monitor serie antes de abrir el bridge (ambos compiten por el puerto).
4. Ajustar `SIMCONNECT_TOTAL_GAIN` y confirmar el signo de cada motor. El valor actual es 90.
5. Probar efectos SimConnect en vuelo y ausencia de fuerza en suelo, pausa y al cerrar el puente.

Para la validacion de esta entrega, ver VALIDACION_SIMCONNECT_V3.md.
No se ha cargado en una placa ni medido PWM, senales, tiempos reales o fuerzas.
La enumeracion USB CDC y la respuesta mecanica requieren esa prueba.

Antes de editar se guardaron los 20 archivos originales de firmware en
`migration_backup/`. Esa carpeta es un respaldo de fuentes, no el firmware nuevo.

## Referencias oficiales

- [Raspberry Pi Pico-series C/C++ SDK](https://www.raspberrypi.com/documentation/pico-sdk/).
- [Pinout de Pico 2](https://datasheets.raspberrypi.com/pico/Pico-2-Pinout.pdf).
- [Datasheet de Pico 2](https://datasheets.raspberrypi.com/pico/pico-2-datasheet.pdf).

