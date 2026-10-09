# Potenciometros en Pico / Pico 2 (05/10/2026)

Seleccionar en `mis_defines.h` y recompilar:

```cpp
#define YOKE_USE_SIMULATION 2
```

0 = encoders; 1 = posicion SimConnect; 2 = potenciometros. El valor guardado
por defecto sigue siendo 1. `Build-Pico2.ps1 -Potentiometers` fuerza 2 sin
editar ese valor y genera `build-pico2-potentiometers/MiYoke_PD_V2.3.ino.uf2`.
El script de compilacion actual apunta a **Pico 2**; no cargar ese UF2 en una
Pico original (RP2040). Para ella seleccionar la placa correspondiente al compilar.

## Cableado compartido con MobiFlight

| Eje | Arduino MobiFlight | Pico despues del divisor |
| --- | --- | --- |
| Roll | Cursor a A0 | ADC0 = GPIO26 = pin fisico 31 |
| Pitch | Cursor a A1 | ADC1 = GPIO27 = pin fisico 32 |

Los extremos de cada pote conservan +5 V y GND del Arduino. La derivacion
hacia Pico sale del mismo cursor, pasa por su divisor, y el punto reducido
va al ADC. Compartir GND entre Arduino, divisores y Pico (AGND: pin 33).
No aplicar 5 V directamente al ADC. Verificar que cada salida del divisor
permanezca entre 0 y 3.3 V en todo el recorrido, con ambas placas conectadas.
No unir las salidas de alimentacion de las dos placas.

Referencia: [pinout oficial Pico 2](https://datasheets.raspberrypi.com/pico/Pico-2-Pinout.pdf).
Lecturas de 12 bits: [Arduino-Pico](https://arduino-pico.readthedocs.io/en/latest/analog.html).
El divisor carga el cursor y puede modificar su respuesta: calibrar con
el cableado definitivo conectado, tambien del lado MobiFlight si corresponde.

## Calibracion desde el LCD

Pulsar CALIBRACION tres veces: ganancias, efectos, CALIBRAR POTES.
OK inicia cuatro capturas: Pitch antihorario, Pitch horario, Roll antihorario,
Roll horario; llevar a cada extremo y confirmar con OK. El centro se calcula
como promedio de los dos extremos. El ultimo OK guarda ambos ejes en flash.
ATRAS en cualquier paso descarta todo y vuelve a CALIBRAR POTES.
Ver [instrucciones y protecciones](MENU_CALIBRACION.md).

La calibracion guardada tiene prioridad sobre ROLL/PITCH_POT_ADC_NEG/CENTER/POS
de mis_defines.h. Estos siguen siendo los valores de fabrica, utilizados si
no existe un registro valido. La calibracion del LCD no requiere recompilar.
El centro es absoluto; no se toma la posicion al encender.

En el diagnostico serie a 115200, `R a=...` es Roll y `P a=...` es Pitch.
`a` muestra el promedio ADC antes de histeresis. Con motores sin potencia,
comprobar `x` aproximadamente -10000, 0, +10000 en extremos y centro;
`v` debe volver a cero al detenerse. Puede quedar una diferencia de hasta
4 cuentas ADC por la histeresis.

Los sentidos horario/antihorario no se pueden verificar automaticamente
sin una referencia mecanica independiente. Confirmarlos al reves invierte
el eje. La validacion de recorrido minimo no detecta esa inversion.
Los signos y limites de los encoders no afectan a los potes; las polaridades
de motor siguen siendo independientes.

## Posicion, velocidad y fuerzas

El modo 2 usa un promedio de 4 conversiones por canal y descarta la primera
al cambiar canal. `YOKE_POT_HYSTERESIS_ADC=4` reduce ruido en reposo sin
perder movimientos lentos acumulados. Se puede ajustar segun el ruido real.
No implica deteccion de un cable desconectado.

La posicion se normaliza por separado a cada lado del centro, limitada a
-1..1. El mismo `YokeMotion` que usan los encoders calcula la velocidad con
el tiempo real, filtro de 40 ms, limite de velocidad y primera muestra a
velocidad cero. El centrado, damper y friccion existentes reciben estas
mediciones locales; no las posiciones que llegan por SimConnect.

Los datos del avion, habilitacion de fuerzas y proteccion por perdida de
telemetria siguen dependiendo de SimConnect. No se generan fuerzas autonomas
con el puente desconectado. MobiFlight sigue enviando los ejes al simulador.

La compilacion no comprueba el divisor, la calibracion ni el ruido reales;
esas verificaciones requieren el hardware.

## Validacion anterior al asistente LCD (05/10/2026)

Core Arduino-Pico 6.1.1, placa Pico 2 ARM con USB TinyUSB:

| Modo | Flash (bytes) | RAM global (bytes) |
| --- | ---: | ---: |
| 0, encoders | 90476 | 18048 |
| 1, SimConnect | 89516 | 17712 |
| 2, potenciometros | 90196 | 17808 |

Los tres modos compilaron correctamente. `tests/simconnect_checks.cpp` paso
84027 comprobaciones por modo, compilado con la bandera en 0, 1 y 2.
Incluye ADC normal/invertido, calibraciones invalidas, ruido en reposo,
movimiento lento, cambio de sentido, parada, saturacion y seleccion de
fuente, ademas de las pruebas existentes de fuerzas y watchdog.
No se cargo firmware ni se verifico hardware fisico.


La version con asistente LCD tambien compilo en los tres modos y paso
84404 comprobaciones por modo. Ver los resultados actuales y el flujo en
[MENU_CALIBRACION.md](MENU_CALIBRACION.md#validacion-del-asistente-lcd-05102026).
