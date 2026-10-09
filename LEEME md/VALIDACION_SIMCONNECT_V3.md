# Validacion de la migracion a SimConnect exclusivo

Fecha: 30/09/2026.

- Firmware actual sin HID, Pico 2 ARM/TinyUSB con potenciometros: compilacion correcta, 85.672 bytes de programa y 16.600 bytes de RAM global.
- Firmware actual sin HID, Pico 2 ARM/TinyUSB con encoders: compilacion correcta, 86.048 bytes de programa y 16.888 bytes de RAM global.
- Puente Win32: compilacion correcta con v141 y SimConnect FSX-SP2-XPACK. Advertencias signed/unsigned del encabezado del SDK, sin errores.
- `tests/bin/simconnect_checks.exe`: 8.039 comprobaciones correctas. Cubre CRC, rangos, secuencias, watchdog de 300 ms, desarmado, DTR, generacion de fuerza sin habilitacion HID/FFB, ganancias, turbulencia y buffets.
- El mapa de enlace del firmware con potenciometros no contiene PIDReportHandler, ConstantForceCalculator, SinForceCalculator ni localDampingCommand.
- Ganancia global conservada en 90%; ganancias individuales y limite combinado de SimConnect conservados.
- Se eliminaron Joystick.cpp/.h, DynamicHID.cpp/.h y PicoHID.cpp/.h. Ambos mapas de enlace carecen de esos objetos/clases. Se conservaron sin cambios AxisPositionInput, AxisEncoders, PicoEncoder, AxisMotion y filters; el sketch sigue midiendo sensores en cada ciclo.
- Se conserva USB CDC, la identificacion YOKE17SC2 y el diagnostico serie, sin envio de posiciones a Prepar3D. TinyUSB sigue siendo necesario para el transporte serie configurado.

No se cargo firmware, no se accionaron motores y no se realizo una prueba de vuelo ni enumeracion USB fisica con Prepar3D v3. La compatibilidad del cliente heredado esta documentada por Prepar3D; la instalacion y conexion en la PC de destino siguen pendientes de prueba. Ver LEEME_SIMCONNECT.md.

El buffet del tren permanece en 10 Hz, tal como estaba configurado. Las pruebas detectan variacion del comando motor, pero su muestreo por LCD cada 100 ms puede aparentar una señal inmovil; no se cambio esa frecuencia en esta migracion.
