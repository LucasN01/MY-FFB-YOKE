# SC8: efectos calculados en Raspberry Pi Pico 2

La PC conserva SimConnect, selección automática del COM, armado y registros. Envía las 22 entradas originales y configuración. La Pico calcula presión normalizada, turbulencia, envolventes de buffets, vibración de suelo, centrado, damper, fricción, mezcla y PWM. La física del avión sigue ejecutándose en el simulador.

## Puesta en uso

1. Cargar `build-pico2/MiYoke_PD_V2.4.ino.uf2` en la Pico 2 mediante BOOTSEL. Se compiló para ARM/TinyUSB y potenciómetros, como la configuración actual. No se cargó automáticamente.
2. Usar el lanzador habitual `INICIAR SIMULADOR.cmd` (o V3). Ahora selecciona `SimConnectBridge/bin/Win32/RealismoSC8/YokeSimConnectBridge.exe` y exige identificación `YOKE17SC8`.
3. La detección USB/COM permanece automática. El puente SC8 requiere el firmware SC8; no combinarlo con el UF2 antiguo. Los ajustes de ganancias persistidos conservan el mismo formato y no se reinician por esta modificación.
4. Para repetir el ensayo informado, comprobar en el menú suelo **0.450** y total **90 %**. Esos valores se usaron en la reproducción del CSV; no se modificaron los valores iniciales del firmware (.18 y .90).

## Vibración de suelo

- Hasta 0.5 kt: cero.
- Entre 0.5 y 40 kt: multiplicador `(velocidad - 0.5) / 39.5`.
- Desde 40 kt: multiplicador uno. No significa fuerza constante máxima.

La Pico filtra aceleraciones X/Y del simulador (pasaaltos 3 Hz, pasabajos 20 ms), mezcla y aplica la rampa. No sintetiza una triangular ni consulta un mapa de baches. El resultado depende de las sacudidas que realmente informa Prepar3D. Permanece el techo de suelo de 40 PWM en roll y 4 en pitch con PWM_MAX=255; otros efectos pueden sumarse.

El CSV anterior tenía 4518 intervalos de tiempo cero entre 5669 pares consecutivos. SC7 reiniciaba el filtro en esos casos. SC8 selecciona la última muestra del despacho antes de enviarla y procesa cada marca nueva una sola vez en la Pico. El modelo conserva su estado ante tiempo cero. La marca es de recepción en Windows, no un reloj de física del simulador. Seleccionar muestras puede omitir picos dentro de un lote.

## Registros

En `SimConnectBridge/bin/Win32/RealismoSC8`:

- `telemetry.csv`: 34 columnas de entradas crudas, tiempos, configuración y estados. `sent=1` significa escritura USB completada; las filas de recepción tienen `sent=0,armed=0` y no representan una orden de desarme. Las antiguas columnas de fuerzas desaparecen porque ya no se calculan en la PC.
- `pico.log`: diagnóstico recibido de la Pico. `SC8 t=... gr=... gp=... R=... P=... us=... lp=... gap=...` muestra marca, señales de suelo por 10000 antes de ganancias, PWM total aplicado, máximo tiempo observado de `Effects::update` (`us`), máxima duración del cuerpo de `loop()` (`lp`) y máximo intervalo entre ejecuciones del lazo (`gap`), en microsegundos desde el arranque. Se registra aproximadamente cada 100 ms y puede omitir picos. Una vez por arranque la Pico envía `BOOT reset=... wd=0x... pm=0x...`; tras un reinicio por watchdog agrega `etapa=n:NOMBRE` (última etapa del lazo) y `falla=1` si hubo HardFault.
- `bridge.log`: conexión, errores del enlace USB con hora local y código de Windows, reconexiones y las líneas `Pico: BOOT ...`.

Los registros se reemplazan al iniciar el ejecutable. Si el `pico.log` anterior no estaba vacío, la generación previa se conserva como `bridge.anterior.log`, `telemetry.anterior.csv` y `pico.anterior.log`. El CSV original usado como regresión quedó preservado en `backups/antes-sc8/` junto al firmware y fuentes anteriores.

## Comprobaciones realizadas

Puente Win32 y firmware Pico 2 compilados. Firmware: 98 588 bytes de programa y 18 400 bytes de RAM global. Suite C++: 94 761 comprobaciones con reproducción del CSV. Detección del COM: 19 comprobaciones sin abrir puertos físicos.

Reproducción de las 5670 filas por el codec SC8, receptor y efectos reales del firmware: 1152 grupos temporales, 1129 paquetes seleccionados y 375 muestras con PWM de suelo no nulo. Con suelo .450 y total .90, picos calculados de 40/4 PWM roll/pitch. El CSV anterior sólo tenía ocho filas con señal de suelo no nula. Las cantidades no son tasas directamente comparables porque cambia la selección de muestras.

Las pruebas cubren integridad CRC de todos los bits SC8, reloj repetido/retroceso/desbordamiento, datos inválidos, pausa, slew, watchdog, integración de ráfagas y buffets, retención entre paquetes y rampa de suelo. No se flasheó la placa ni se midió fuerza o tiempo real en hardware. `us` permite revisar el costo del cálculo durante el siguiente ensayo.

Para repetir la reproducción, compilar `tests/simconnect_checks.vcxproj` y ejecutar desde la raíz:

```powershell
.\tests\bin\simconnect_checks.exe .\backups\antes-sc8\SimConnectBridge__bin__Win32__RealismoSC7__telemetry.csv
```

Los modelos vigentes están en la raíz: `PicoForceModels.h`, `GroundModel.h`, `GustModel.h`, `BuffetModel.h`; transporte en `RawTelemetry.h` y `SimTelemetry.h`. La explicación extensa está actualizada en `tesis/main.tex` (capítulos en `tesis/capitulos/`).

## Actualización de textura sobre pista lisa

Ahora `SimEffects.h` suma una base suave de amplitud `sqrt(E)` (ISO 8608): `clip(0.12 * sqrt(E) * (0.65*N7 + 0.35*N11.3), -0.12*sqrt(E), 0.12*sqrt(E))`. N7/N11.3 son ruido de banda estrecha 7/11.3 Hz, RMS 0.7071, con `SIMCONNECT_GROUND_NARROWBAND_TEXTURE=1`; el flag en 0 conserva senoides. Se calcula al ritmo de motores; E sigue siendo la rampa 0.5–40 kt para los baches de P3D. La suma con los baches se limita a ±sqrt(E), antes de ganancias y techos existentes. No identifica superficies: agrega sensación de rodaje cuando la telemetría es demasiado lisa.

Con suelo .450 y total .90, la base aislada alcanza aproximadamente 12 PWM roll y 1 pitch como máximo, mientras los baches conservan el techo conjunto 40/4. Se anula parado, en vuelo o al deshabilitar efectos. `gr/gp` sigue mostrando sólo los baches; R/P incluye textura y demás efectos.

Prueba actual: 96 764 comprobaciones correctas; reproducción del CSV con 800 muestras de suelo no nulas (la cifra 375 anterior corresponde a la revisión sin textura). Se agregó una prueba de aceleración constante en pista, y cortes en reposo y vuelo. Para usar esta revisión, cargar el nuevo UF2 de `build-pico2`; el puente y protocolo SC8 no cambian. No se cargó automáticamente ni se ensayó físicamente.
