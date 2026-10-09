> Antecedente de versiones anteriores. Los ajustes y seleccion de fuente vigentes estan en [AJUSTE_FUERZAS_Y_MOVIMIENTO.md](AJUSTE_FUERZAS_Y_MOVIMIENTO.md). Usar YOKE_USE_SIMULATION (1=simulacion, 0=encoders, 2=potenciometros ADC en Pico); ver LEEME_POTENCIOMETROS.md para el modo 2 actual. Los valores de ganancias y banderas anteriores de este documento ya no aplican.

# Centrado dinamico y vibraciones en tierra (SC4)

Ambos efectos estan activados por defecto en `mis_defines.h`:

```cpp
#define SIMCONNECT_DYNAMIC_CENTERING_ENABLED 1
#define SIMCONNECT_GROUND_VIBRATION_ENABLED 1
```

Cambiar a 0 para desactivar individualmente y recompilar/cargar firmware.
El interruptor general `SIMCONNECT_ADDITIONS_ENABLED` y la ganancia global
`SIMCONNECT_TOTAL_GAIN` siguen aplicandose a todos los efectos.

## Rodaje

El puente solicita `SIM ON GROUND` y `GROUND VELOCITY` en knots.
Solo genera amplitud si el avion esta en tierra, la simulacion corre sin
pausa/slew y hay muestras recientes. No usa IAS: viento con el avion parado
no debe generar vibraciones de rodaje.

`amplitud = limitar((velocidad_suelo - 0.5) / 39.5, 0, 1)`.
Estos umbrales de 0.5 y 40 kt son ajustes propios en `GroundModel.h`.
La Pico sintetiza una portadora triangular de 8 Hz, con entrada suavizada
a 100 ms y ganancia maxima pitch 0.040, roll 0.020. Al recibir amplitud cero
por detenerse o despegar, corta el efecto sin cola del filtro.
No representa impactos de aterrizaje ni distingue superficies de pista.
`SIMCONNECT_GROUND_VIBRATION_GAIN` ajusta la intensidad en el firmware.

Variable verificada en el [SDK de Prepar3D](https://www.prepar3d.com/SDKv2/LearningCenter/utilities/variables/simulation_variables.html).

## Centrado

Adaptacion del principio de resorte aerodinamico de la copia local de
VPforce TelemFFB (GPL v3), `telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py`,
metodos `_calculate_dynamic_pressures`, `_calculate_vne_and_gains` y
`_calculate_control_coefficients`: coeficiente proporcional a presion dinamica
normalizada y fuerza restauradora proporcional al desplazamiento.

```text
qRef = 0.5 * 1.225 * (referenceKnots * 0.514444444)^2
qNormalizada = limitar(DYNAMIC_PRESSURE_PSF * 47.88025898 / qRef, 0, 1)
F_roll  = -posicion_normalizada_roll  * qSuavizada * 0.100 * ganancia_global
F_pitch = -posicion_normalizada_pitch * qSuavizada * 0.200 * ganancia_global
```

El coeficiente es K = 0.15 + 0.85 * qNormalizada y se suaviza con constante de 100 ms; en las ecuaciones anteriores qSuavizada ahora representa K suavizado. A igual desplazamiento
normalizado, pitch tiene el doble de comando que roll. La posicion predeterminada viene de YOKE X/Y POSITION de SimConnect; cero anula el centrado inmediatamente. Se usa el
signo opuesto a la posicion y luego la polaridad de motor configurada.

A densidad constante, duplicar la velocidad cuadruplica el aporte aerodinamico hasta
su limite. La referencia sigue siendo `--reference-knots 120` por defecto;
es un ajuste del yugo, NO la Vne del avion. TelemFFB utiliza Vne y contempla
propwash, trim y otras correcciones; esta adaptacion no las implementa.
La misma referencia ya alimentaba turbulencia, por lo que cambiarla tambien
afecta ese modelo. El centrado actua en tierra y en vuelo con telemetria valida y
presion dinamica variable. A presion cero hay rigidez minima 15% del maximo, solo con telemetria valida y armada. Se ajusta con SIMCONNECT_CENTERING_MIN_STIFFNESS (0 restaura el comportamiento puramente aerodinamico).

`SIMCONNECT_CENTERING_PITCH_GAIN` ajusta el maximo; roll se deriva como mitad.
Con ganancia global 90%, los maximos aislados son 0.180 pitch y 0.090 roll.
La mezcla conserva `SIMCONNECT_EXTRA_LIMIT=0.250`; otros efectos simultaneos
pueden saturar esa suma. La relacion 2:1 describe el centrado aislado, no la
fuerza fisica en newtons ni la mezcla completa.

## Actualizacion y ejecucion

Usar juntos el puente recompilado y firmware SC4, identificado como
`YOKE17SC4`. El puente rechaza firmware SC2/SC3. El receptor acepta paquetes
historicos v1/v2/v3. En modo posicion simulada solo SC4 habilita centrado.
SC4 lleva 26 bytes: payload SC2 hasta offset 17, amplitud de rodaje uint16
0..10000 en offset 18, posiciones roll/pitch int16 (-10000..10000) en offsets 20/22 y CRC16 en offset 24. El campo de presion existente
alimenta el centrado. Secuencia, CRC, watchdog 300 ms y DTR siguen activos.
Pausa/slew/simulacion detenida desarman todo; tierra ya no desarma el puente.
Turbulencia y buffets aerodinamicos siguen calculandose solo en vuelo.

Compilar con `Build-Bridge.ps1` y `Build-Pico2.ps1` (o `-Encoders`).
Usar el UF2 **MiYoke_PD_V2.3.ino.uf2** de la carpeta del modo correspondiente.
La compilacion no carga la placa.

```powershell
.\SimConnectBridge\bin\Win32\Release\YokeSimConnectBridge.exe --port COM7 --enable --gusts
```

Reemplazar COM7 por el puerto real. Sin `--gusts`, rodaje y centrado siguen
disponibles. `--no-buffets` solo desactiva perdida/tren/spoilers.
Sin `--enable` el puente es monitor, sin fuerzas.

El CSV agrega `ground_speed_knots`, `ground_amp`, `Vibracion_Tierra` y
`Centrado_Disponible`, `yoke_roll` y `yoke_pitch`. Los indicadores describen disponibilidad en el puente,
no conocen las banderas del firmware ni confirman PWM. El indicador de centrado
puede estar en 1 con el yugo centrado y fuerza cero.

## Validacion

Pruebas C++: rodaje parado/movimiento/aire/pausa/datos invalidos, CRC SC3,
rango y compatibilidad del protocolo, fuerza restauradora, centro cero,
relacion pitch/roll 2:1, respuesta a presion, ganancias, mascaras de efectos,
watchdog, DTR y reinicio progresivo. Se mantienen pruebas anteriores.
No se cargo firmware ni se probaron motores o un vuelo real del simulador.
La comprobacion fisica pendiente es verificar centrado en ambos sentidos,
rodaje detenido/en movimiento, despegue, pausa y desconexion.

## Fuente de posicion SC4 y comprobacion

`SIMCONNECT_CENTERING_USE_SIM_POSITION=1` en mis_defines.h usa el simulador
(valor actual). Para encoders futuros: cambiar esta bandera a 0, compilar con
Build-Pico2.ps1 -Encoders y cargar ese UF2. La seleccion de sensores y la fuente
de centrado son independientes; compilar -Encoders por si solo conserva la
fuente SimConnect. En modo local el centro es el cero de encoders al encender
o el centro calibrado de potenciometros.

Las variables [YOKE X/Y POSITION del SDK](https://prepar3d.net/SDKv2/LearningCenter/utilities/variables/simulation_variables.html)
son posiciones de mandos, no el angulo de alabeo/cabeceo del avion. El modelo
del avion debe publicarlas correctamente. El puente muestra yokeR/yokeP en
consola y CSV para comprobar que vuelvan a 0. Una posicion no finita o fuera
de -1..1 desarma la salida; no se interpreta como extremo ni se usa un sensor
local como reemplazo. La respuesta depende de la frecuencia de SimConnect
y del envio serie (hasta 50 Hz), no de la lectura local de 500 Hz.

Para aislar centrado en tierra, detenerse y mover cada mando a ambos lados.
Al devolverlo al centro, el aporte de centrado debe ser cero. En vuelo, viento
y buffets pueden producir PWM aunque el centrado sea cero. Para aislarlo usar
--no-buffets sin --gusts; rodaje seguira disponible si el avion se mueve en suelo.
Ejemplo desde la carpeta del EXE:

```powershell
.\YokeSimConnectBridge.exe --port COM13 --enable --no-buffets
```

SC4 requiere volver a cargar firmware y reemplazar el EXE juntos.
Pruebas automaticas: 8963 comprobaciones, incluidas posicion simulada frente a
sensor local, retorno inmediato a cero, signos por eje, rigidez en reposo,
CRC y rangos SC4, desarmado y rechazo de centrado simulado con paquetes viejos.
No se ha probado esta revision con el avion del usuario ni con motores.
