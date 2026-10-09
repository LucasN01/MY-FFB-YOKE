# MiYoke Pico 2: fuerzas por SimConnect

Version 30/09/2026, preparada para usar el puente con Prepar3D v3 sin FFB nativo.

Los motores reciben exclusivamente turbulencia y buffet de perdida, tren y spoilers calculados con telemetria SimConnect. Se eliminaron del firmware activo los efectos CONSTANT y SINE, el centrado automatico y la amortiguacion local. No hay vibraciones de rodaje ni de contacto con el suelo.

La Pico 2 recibe la telemetria por USB CDC y controla los motores. Otra placa envia los ejes a Prepar3D; esta Pico no publica joystick HID ni envia posiciones al simulador. Se conservan la lectura local de potenciometros/encoders, AxisMotion, filtros, diagnostico serie y pantalla para uso actual y futuras implementaciones.

- [Puesta en marcha y traslado a la PC con Prepar3D v3](LEEME_SIMCONNECT.md)
- [Cableado y carga de Pico 2](LEEME_PICO2.md)
- [Potenciometros](LEEME_POTENCIOMETROS.md)
- [Encoders](LEEME_ENCODERS.md)
- [Efectos y ecuaciones](EFECTOS_SIMCONNECT.md)
- [Pantalla](LEEME_LCD.md)

Ajustar pines, sensores y ganancias en `mis_defines.h`. La ganancia global se llama ahora `SIMCONNECT_TOTAL_GAIN` (90%). Se conservaron las ganancias individuales y el limite conjunto de 0,250.

Se eliminaron del proyecto activo Joystick.cpp/.h, DynamicHID.cpp/.h y PicoHID.cpp/.h. La referencia VPforce-TelemFFB-refactor no forma parte del firmware. Los documentos de etapas anteriores se conservan como historial.
