> Antecedente de versiones anteriores. Los ajustes y seleccion de fuente vigentes estan en [AJUSTE_FUERZAS_Y_MOVIMIENTO.md](AJUSTE_FUERZAS_Y_MOVIMIENTO.md). Usar YOKE_USE_SIMULATION (1=simulacion, 0=encoders, 2=potenciometros ADC en Pico); ver LEEME_POTENCIOMETROS.md para el modo 2 actual. Los valores de ganancias y banderas anteriores de este documento ya no aplican.

> Version actual: fuerzas solo por SimConnect; sin CONSTANT/SINE ni centrado. Ver [LEEME_SIMCONNECT.md](LEEME_SIMCONNECT.md).

# MiYoke en Raspberry Pi Pico 2 (RP2350)

El mismo sketch `MiYoke_PD_V2.2.ino` ahora selecciona automaticamente el hardware
segun la placa elegida. La configuracion predeterminada sigue siendo dos
potenciometros (`USE_POTENTIOMETERS = 1`). Tambien se adapto el modo encoders.
No se necesita MicroPython ni un sistema operativo en la Pico.

## Configuracion de Arduino IDE

1. Usar el paquete **Raspberry Pi Pico/RP2040/RP2350 de Earle F. Philhower**.
   La migracion se compilo con la version **6.1.1**, ya instalada en esta PC.
   URL del gestor, si hay que instalarlo en otra PC:
   `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`.
2. Abrir `MiYoke_PD_V2.2.ino` dentro de su carpeta actual, con todos los `.h` y `.cpp`.
3. Elegir **Raspberry Pi Pico 2**, arquitectura **ARM**.
4. En **Tools / Herramientas > USB Stack**, elegir **Adafruit TinyUSB**.
   La libreria viene incluida en ese paquete. No elegir Pico SDK, No USB ni USB Host.
5. Compilar. Para la primera carga, conectar por USB manteniendo BOOTSEL.
   Soltar BOOTSEL y copiar el `.uf2` a la unidad **RP2350** que aparece.
   La placa se reinicia y presenta el puerto serie USB CDC, sin joystick HID.
6. Para cargas posteriores se puede usar Subir desde Arduino IDE con el puerto
   de la Pico seleccionado; si no lo detecta, repetir BOOTSEL.

Binarios generados:

- `build-pico2/MiYoke_PD_V2.2.ino.uf2`: posicion desde SimConnect (YOKE_USE_SIMULATION=1), configuracion predeterminada.
- `build-pico2-encoders/MiYoke_PD_V2.2.ino.uf2`: encoders, con la bandera de sensores en 0.

Los binarios reflejan la configuracion del momento de compilacion. Si se cambian
pines, ganancias o calibracion, hay que volver a compilar.

Tambien se puede compilar desde PowerShell, situado en esta carpeta:

```powershell
.\Build-Pico2.ps1
.\Build-Pico2.ps1 -Encoders
```

El primer comando respeta `USE_POTENTIOMETERS` del archivo de configuracion;
el segundo fuerza encoders. Ninguno carga ni activa la placa.

probar.ps1 -PruebaFalla compila build-pico2-prueba-falla (HardFault provocado a los 20 s, solo para validar el diagnostico; no usar para volar).

FQBN utilizado: `rp2040:rp2040:rpipico2:usbstack=tinyusb,arch=arm`.

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
- El sketch recibe SimConnect por CDC. Otra placa envia los ejes a Prepar3D.
- La lectura de posicion, AxisMotion y los filtros siguen activos a 2 ms para diagnostico y futuras funciones locales.
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

- [Arduino-Pico: USB y seleccion de TinyUSB](https://arduino-pico.readthedocs.io/en/latest/usb.html).
- [Arduino-Pico: ADC y PWM](https://arduino-pico.readthedocs.io/en/latest/analog.html).
- [Pinout de Pico 2](https://datasheets.raspberrypi.com/pico/Pico-2-Pinout.pdf).
- [Datasheet de Pico 2](https://datasheets.raspberrypi.com/pico/pico-2-datasheet.pdf).

