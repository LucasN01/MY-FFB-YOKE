# Ajuste empirico de fuerzas y fuente de movimiento — 01/10/2026

Esta guia reemplaza los ajustes de fuerza/fuente de versiones anteriores.
Los parametros de fabrica estan en `mis_defines.h`. Las ganancias ahora se pueden editar y guardar desde el [menu de calibracion](MENU_CALIBRACION.md), sin recompilar. Otros parametros requieren recompilar y cargar el UF2. Se conserva el puente/protocolo SC4: no hace falta reemplazar
el EXE SC4 que ya funciona.

## Una sola bandera: simulacion, encoders o potenciometros

```cpp
#define YOKE_USE_SIMULATION 1
```

- **1:** posicion y velocidad desde YOKE X/Y POSITION de SimConnect. No se
  inicializan encoders ni se leen ADC. Es el modo actual para tus pruebas.
- **0:** posicion y velocidad desde los encoders AB de la Pico. Calibrar
  `ROLL_ENCODER_MIN/MAX`, `PITCH_ENCODER_MIN/MAX` y signos. Encender centrado.
  Los datos del avion (velocidad, viento, tren, etc.) siguen llegando por SimConnect.

- **2:** posicion desde potenciometros en ADC0/GP26 (Roll) y ADC1/GP27
  (Pitch), velocidad local con el mismo estimador que los encoders.
  Ver [cableado y calibracion ADC](LEEME_POTENCIOMETROS.md).

La bandera gobierna centrado y toda la compensacion por movimiento.
`Build-Pico2.ps1` respeta la bandera y escribe en `build-pico2`.
`-Encoders` fuerza 0 y escribe en `build-pico2-encoders`.
`-Potentiometers` fuerza 2 y escribe en `build-pico2-potentiometers`.
Los tres generan `MiYoke_PD_V2.3.ino.uf2` para Pico 2.

## Rango y ganancias

Antes, la suma se limitaba a 0.250, aproximadamente 64 PWM con PWM_MAX=255.
Ahora `SIMCONNECT_EXTRA_LIMIT = 220.0f / 255.0f`, es decir **0.862745**:
la salida firmada queda entre **-220 y +220 PWM**, incluida la compensacion.
0.900 equivaldria a aproximadamente 230 PWM, no a 220. Se rechazan limites
normalizados superiores a 0.900 al compilar. No se cambia la escala electrica
`PWM_MAX=255`; si se baja, tambien se reduce el maximo efectivo.

Valores iniciales y picos aislados aproximados, con ganancia global 90%,
envolvente completa, sin compensacion por movimiento ni otros efectos:

| Efecto | Parametro en mis_defines.h | Valor | PWM pitch / roll |
| --- | --- | ---: | ---: |
| Centrado, maxima presion y recorrido | SIMCONNECT_CENTERING_PITCH_GAIN | 0.800 | 184 / 92 |
| Centrado detenido, recorrido completo | SIMCONNECT_CENTERING_MIN_STIFFNESS | 0.150 | 28 / 14 |
| Turbulencia, senal extrema | SIMCONNECT_GUST_PITCH / SIMCONNECT_GUST_ROLL | 0.700 / 0.350 | 161 / 80 |
| Rodaje (ganancia referida a roll) | SIMCONNECT_GROUND_VIBRATION_GAIN | 0.180 | 4 / 40 |
| Buffet de perdida | SIMCONNECT_STALL_BUFFET_GAIN | 0.240 | 55 / 28 |
| Buffet de tren | SIMCONNECT_GEAR_BUFFET_GAIN | 0.160 | 37 / 18 |
| Buffet de spoilers | SIMCONNECT_SPOILER_BUFFET_GAIN | 0.160 | 37 / 18 |

Las vibraciones alternan signo: estos son picos, no fuerza continua. Un efecto
suave o con envolvente parcial no debe llegar al maximo solo por existir.
`SIMCONNECT_TOTAL_GAIN=90` multiplica todos los efectos y la compensacion.
Las ganancias individuales aceptan 0..1, la global 0..100.
La relacion pitch/roll 2:1 vale con estados comparables antes de saturar, excepto rodaje: roll/pitch 10:1.
La fuerza fisica depende del motor y de la mecanica; el comando no mide newtons.

## Compensacion segun el movimiento del piloto

Se aplica UNA vez a la suma de efectos excepto rodaje, antes del limite final. Sigue el ejemplo
pedido; no es un damper independiente que frene siempre, ni friccion estatica:

- Motor hacia negativo y piloto hacia positivo: aumenta la fuerza negativa.
- Motor hacia negativo y piloto hacia negativo: reduce la fuerza negativa,
  como minimo hasta cero, sin invertirla.
- Los mismos criterios se aplican al sentido positivo.
- Sin fuerza base, no genera fuerza extra. Centrado puro en posicion cero
  produce cero aunque el yugo atraviese el centro con velocidad.

| Parametro | Valor | Que cambiar |
| --- | ---: | --- |
| YOKE_DAMPER_ENABLED | 1 | 0 desactiva la parte proporcional a velocidad |
| YOKE_DAMPER_GAIN | 0.120 | Subir aumenta la diferencia entre mover lento y rapido |
| YOKE_FRICTION_ENABLED | 1 | 0 desactiva la parte de friccion suave |
| YOKE_FRICTION_GAIN | 0.030 | Subir aumenta la correccion que se estabiliza al ganar velocidad |
| YOKE_FRICTION_SPEED | 0.200 | Velocidad adicional sobre la zona muerta para media correccion de friccion |
| YOKE_MOTION_CORRECTION_LIMIT | 0.400 | Tope de correccion antes de ganancia global y peso del eje |
| YOKE_MOTION_FORCE_BLEND | 0.100 | Atenua la correccion si la fuerza base es pequena, evitando saltos de signo |
| YOKE_MOTION_FILTER_TAU_S | 0.040 | Subir suaviza la velocidad y demora la respuesta |
| YOKE_MOTION_DEAD_SPEED | 0.020 | Movimiento menor se ignora en la correccion |
| YOKE_MOTION_MAX_SPEED | 10.0 | Limita velocidades extremas/ruido |

La unidad de velocidad es recorrido centro-extremo por segundo: pasar del
centro a un extremo en 0.5 s equivale a 2 unidades/s. Roll usa la mitad de la
correccion de pitch, igual que los efectos base. Con los ajustes actuales,
el aporte adicional maximo es 0.360 en pitch y 0.180 en roll (90% global).

```text
s = max(abs(velocidad_filtrada) - zona_muerta, 0)
C = damper_gain * s + friction_gain * s / (friction_speed + s)
C = min(C, limite_correccion) * ganancia_global * peso_eje
C *= min(abs(fuerza_base) / force_blend, 1)
F = fuerza_base - signo(velocidad) * C
F se recorta a cero si cruzaria el signo de fuerza_base
salida = limitar(F, -220/255, +220/255)
```

La velocidad simulada se calcula solo al recibir muestras nuevas, con el
intervalo real entre paquetes; mantener una muestra durante varios ciclos de
2 ms no crea velocidades artificiales. La fuente encoder usa su intervalo
local. Ambas usan el mismo filtro, limites y convencion de signos. El arranque,
una interrupcion larga o reconexion no producen un salto de derivada. Tras
40 ms de muestras quietas se elimina el residuo del filtro; al invertir el
movimiento se elimina la velocidad residual del sentido anterior.

## Procedimiento de ajuste

1. Para centrado aislado, avion detenido y ejecutar sin --gusts:
   `.\YokeSimConnectBridge.exe --port COM13 --enable --no-buffets`.
   Desactivar temporalmente YOKE_DAMPER_ENABLED y YOKE_FRICTION_ENABLED.
2. Ajustar primero SIMCONNECT_CENTERING_MIN_STIFFNESS para reposo; despues
   SIMCONNECT_CENTERING_PITCH_GAIN para vuelo. Probar misma posicion a distintas
   velocidades. Roll se calcula como mitad de pitch. El parametro del puente
   --reference-knots 120 sigue definiendo la presion de referencia; bajarlo
   endurece antes, pero tambien cambia el escalado de turbulencia.
3. Activar damper y mover contra la fuerza, luego acompanarla. Cambiar
   YOKE_DAMPER_GAIN en pasos de 0.02; comparar igual posicion y distintas
   velocidades de mano. Ajustar friccion despues, en pasos de 0.01.
4. Probar cada vibracion con las demas banderas de efectos en 0. Ajustar su
   ganancia de a 0.02..0.05. Para viento usar --gusts --gust-gain 1 inicialmente:
   --gust-gain 10 puede saturar la senal del puente y ocultar diferencias.
5. Restaurar los efectos deseados y comprobar la mezcla. Si pasa mucho tiempo
   en +/-220, bajar la ganancia global o los efectos dominantes; subir ganancias
   estando saturado no da mas detalle. El tope es un techo, no un objetivo para
   toda situacion. Mantener SIMCONNECT_EXTRA_LIMIT para conservar el margen.

El diagnostico USB muestra `x` (posicion de la fuente activa x10000), `v`
(velocidad filtrada x1000), `f` (fuerza final x10000) y `p` (PWM firmado).
El LCD muestrea a 100 ms: una vibracion de 10 Hz puede verse fija por muestreo,
aunque el motor oscile. Los indicadores CSV del puente no conocen las ganancias
ni las banderas compiladas en la Pico.

Pausa/desarme, perdida de DTR y watchdog siguen retirando la fuerza. Incluso en
modo encoder se requiere telemetria del avion valida y armada. No se cargaron
los firmwares ni se probaron motores durante esta modificacion.

## Rodaje con predominio en roll — actualizacion

El aporte de vibracion de suelo ahora tiene relacion roll:pitch de 10:1:
30 PWM equivalentes en roll corresponden a 3 en pitch. Se limita primero el
aporte de roll a +/-40 PWM y luego se divide por 10 para pitch (+/-4 PWM).
La conversion a PWM entero puede redondear los valores pequenos.

En mis_defines.h:
- SIMCONNECT_GROUND_VIBRATION_GAIN: intensidad de roll, actualmente 0.180.
- SIMCONNECT_GROUND_ROLL_PITCH_RATIO: relacion roll/pitch, actualmente 10.0.
- SIMCONNECT_GROUND_MAX_PWM: tope de roll, actualmente 40 (admite 0..40).

El rodaje se suma DESPUES de la compensacion por movimiento para que damper
ni friction alteren la relacion o amplifiquen esa vibracion. Los demas efectos
conservan su compensacion. El techo general de la mezcla sigue siendo 220 PWM:
centrado u otros efectos simultaneos pueden hacer que el PWM TOTAL supere 40,
pero el aporte de rodaje no. Con PWM_MAX inferior a 255 los picos tambien bajan.
El efecto conserva su deteccion de suelo/movimiento, frecuencia y suavizado.
No requiere cambiar el puente SC4; recompilar y cargar el firmware.

