> Antecedente de versiones anteriores. Los ajustes y seleccion de fuente vigentes estan en [AJUSTE_FUERZAS_Y_MOVIMIENTO.md](AJUSTE_FUERZAS_Y_MOVIMIENTO.md). Usar YOKE_USE_SIMULATION (1=simulacion, 0=encoders, 2=potenciometros ADC en Pico); ver LEEME_POTENCIOMETROS.md para el modo 2 actual. Los valores de ganancias y banderas anteriores de este documento ya no aplican.

> Estado actual en Pico 2: sensores conservados para lectura local, diagnostico y futuras funciones; no se envian ejes al simulador y no hay amortiguacion local. Para cableado Pico 2 y niveles de 3,3 V, ver LEEME_PICO2.md; las secciones Micro siguientes describen el hardware anterior.

# Seleccionar potenciometros o encoders AB

En mis_defines.h cambiar una sola bandera y compilar/cargar de nuevo:

```cpp
#define USE_POTENTIOMETERS 1 // 1 = potenciometros; 0 = encoders AB
```

La configuracion guardada sigue en 1. La seleccion es al compilar, no en caliente.
Ambos modos alimentan la misma lectura de posicion/velocidad y amortiguacion local.
En modo pot no se incluye ni se inicializa Encoder.h; en modo encoder no se lee ADC.

## Cableado de los encoders al Arduino Micro de 5 V

| Senal | Pin del Micro |
| --- | --- |
| Roll A | D0 (RX) |
| Roll B | D1 (TX) |
| Pitch A | D7 |
| Pitch B | D4 |
| GND de ambos encoders | GND comun |

Son cuatro pines de **senal** en total; cada encoder tambien necesita alimentacion
y GND. Alimentarlo segun la ficha de su modelo. Las entradas deben recibir niveles
compatibles con 5 V, nunca 12/24 V. Para NPN de colector abierto, los pull-ups van
a 5 V; Encoder.h activa los internos. Segun cableado/ruido pueden necesitarse
pull-ups externos, dimensionados para el encoder. No hace falta canal Z.

Roll tiene interrupcion en ambos canales; Pitch tiene interrupcion en A y lee B.
Comprobar que no se pierdan cuentas con movimientos rapidos en el montaje real.
D2/D3 siguen reservados para el LCD. D0/D1 no pueden usarse tambien para Serial1;
el monitor Serial USB sigue disponible. Los motores conservan sus pines.

## Calibracion de encoders

1. Seleccionar 0 y cargar el programa. Para medir, mantener desconectada la potencia
   de los motores. Centrar fisicamente ambos ejes antes de reiniciar: A/B es
   incremental y el firmware pone las dos cuentas en cero al arrancar.
2. Abrir el monitor Serial a 115200. `R c=...` es Roll y `P c=...` es Pitch.
   Mover a la derecha (Roll) o tirar (Pitch) debe aumentar c. Si disminuye,
   cambiar ROLL_ENCODER_DIRECTION o PITCH_ENCODER_DIRECTION de 1 a -1,
   cargar de nuevo y reiniciar centrado. No cambiar el signo del motor para esto.
3. Anotar c en cada extremo y guardar ROLL_ENCODER_MIN/MAX y
   PITCH_ENCODER_MIN/MAX en mis_defines.h. Compilar/cargar nuevamente.
4. Verificar x=-1000/0/+1000 en extremo negativo/centro/extremo positivo.
   Estas x son milesimas normalizadas, no cuentas del encoder.

Valores iniciales para encoders de 1000 PPR, contados x4:

- Roll directo +/-90 grados: -1000 a +1000 cuentas.
- Pitch +/-150 mm con polea efectiva de 75 mm, encoder solidario a ella:
  -2546 a +2546 cuentas, aproximadamente.
- Pitch con polea de 50 mm seria aproximadamente +/-3820; con 100 mm, +/-1910.

El valor de Pitch es una estimacion, no una calibracion del montaje. Si cambia el
PPR, la polea o la relacion de transmision hay que actualizar los limites medidos.
Volver a USE_POTENTIOMETERS=1 recupera los limites equivalentes y la calibracion
ADC anteriores, sin tener que deshacer la calibracion de encoders.

El rango de posicion HID ahora coincide con los limites seleccionados en ambos
modos. `//Joystick.sendState();` sigue comentado como en la configuracion previa:
los sensores se usan localmente para FFB/amortiguacion, pero no se transmiten
nuevas posiciones al PC. Para usar este Micro tambien como entrada de posicion
del simulador, descomentar esa linea y asignar/calibrar sus ejes en el simulador.

Se comprueba compilacion para Arduino Micro; falta validar lecturas, sentido y
ausencia de perdida de cuentas con los encoders fisicos. No se carga firmware.

