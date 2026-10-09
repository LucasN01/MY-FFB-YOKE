> Actualizacion SC3: se agregaron centrado dinamico y rodaje. Consultar [CENTRADO_Y_RODAJE.md](CENTRADO_Y_RODAJE.md) y LEEME_SIMCONNECT.md para el comportamiento actual. Las secciones SC2 de abajo son antecedentes y no describen el nuevo armado en tierra.

# Efectos adicionales de SimConnect: uso, matemática y fuentes

Actualización del 30/09/2026: SimConnect es la única fuente de fuerzas. Se retiraron CONSTANT/SENO y la amortiguación local. Ver LEEME_SIMCONNECT.md para puesta en marcha con Pico 2 y Prepar3D v3.

## Qué se agregó y por qué

| Efecto | Qué representa | Cuándo funciona | Bandera en `mis_defines.h` |
| --- | --- | --- | --- |
| Turbulencia (existente, ahora con bandera propia) | Variaciones del viento en ejes del avión | En vuelo, con `--gusts` en el puente | `SIMCONNECT_TURBULENCE_ENABLED` |
| Buffet de pérdida | Vibración progresiva al aproximarse al ángulo de pérdida | En vuelo, con AoA/ángulo de pérdida/velocidades de referencia válidos | `SIMCONNECT_STALL_BUFFET_ENABLED` |
| Buffet de tren | Vibración aerodinámica con tren retráctil desplegado | En vuelo; aumenta entre 100 y 150 kt IAS | `SIMCONNECT_GEAR_BUFFET_ENABLED` |
| Buffet de spoilers | Vibración asociada a spoilers desplegados | En vuelo; aumenta entre 80 y 140 kt IAS | `SIMCONNECT_SPOILER_BUFFET_ENABLED` |

Los nuevos efectos transmiten información sobre la configuración y el estado
aerodinámico. No añaden resortes, centrado ni esfuerzos constantes de maniobra.
No se implementaron fuerzas por G o seguimiento de trim/autopiloto: quedan fuera del alcance de esta versión. Vibración de motor,
rodaje e impacto de aterrizaje requieren calibrar la transmisión de vibraciones
al yoke y discriminar las condiciones del avión; quedaron fuera de esta entrega.

Es un modelo háptico orientado a aviones de ala fija. Su utilidad y amplitud
perceptual dependen del modelo de avión y del mecanismo del mando. No es un
modelo certificado de fuerza de bisagra ni de vibración estructural.

## Activar/desactivar en Arduino

En `mis_defines.h`, 1 activa y 0 desactiva. Recompilar y volver a cargar:

```cpp
#define SIMCONNECT_ADDITIONS_ENABLED       1 // interruptor general
#define SIMCONNECT_TURBULENCE_ENABLED      1
#define SIMCONNECT_STALL_BUFFET_ENABLED    1
#define SIMCONNECT_GEAR_BUFFET_ENABLED     1
#define SIMCONNECT_SPOILER_BUFFET_ENABLED  1
```

Se dejan las cuatro banderas activadas. Una bandera individual en cero solo
retira ese aporte. El interruptor general en cero elimina toda la recepción y
generacion SimConnect del sketch: con la bandera general en cero, no hay fuerza.

Ganancias iniciales, también en `mis_defines.h`:

| Parámetro | Valor | Significado |
| --- | ---: | --- |
| `SIMCONNECT_GUST_ROLL` | 0,100 | Máximo nominal de turbulencia roll |
| `SIMCONNECT_GUST_PITCH` | 0,200 | Máximo nominal de turbulencia pitch |
| `SIMCONNECT_STALL_BUFFET_GAIN` | 0,040 | Pico nominal de buffet de pérdida en pitch |
| `SIMCONNECT_GEAR_BUFFET_GAIN` | 0,025 | Pico nominal de buffet de tren en pitch |
| `SIMCONNECT_SPOILER_BUFFET_GAIN` | 0,025 | Pico nominal de buffet de spoilers en pitch |
| `SIMCONNECT_EXTRA_LIMIT` | 0,250 | Límite de la suma de todos los aportes adicionales, por eje |

Los tres nuevos efectos usan en roll la mitad de su amplitud de pitch y la
misma fase. Todos se multiplican por la ganancia total del eje. Por ejemplo,
0,040 con ganancia 90% corresponde a un pico nominal de 0,036 de la escala de
salida en pitch, antes de sumar y saturar. No son newtons ni porcentajes de una
fuerza física calibrada. Se conservó la polaridad mecánica existente.

## Ejecución y actualización necesaria

Hay que actualizar **puente y firmware**: el puente exige la identificación
`YOKE17SC2`. Un Arduino con firmware SC1 se rechaza durante la identificación,
antes de transmitir las fuerzas del protocolo ampliado.

```powershell
.\SimConnectBridge\bin\Win32\Release\YokeSimConnectBridge.exe --port COM7 --enable --gusts --gust-gain 1
```

Reemplazar COM7 por el puerto real. Con este comando se calculan los cuatro
efectos; Arduino decide cuáles aplica según las banderas. `--enable` sin
`--gusts` ahora permite los tres buffets, pero no turbulencia. `--no-buffets`
apaga los tres buffets desde el puente y sirve para comparar solo turbulencia.
`--gust-gain` y `--gust-longitudinal` afectan únicamente a la turbulencia.

Monitor sin abrir COM ni enviar fuerzas:

```powershell
.\SimConnectBridge\bin\Win32\Release\YokeSimConnectBridge.exe --seconds 60 --gusts
```

`telemetry.csv` registra las amplitudes `stall_amp`, `gear_amp`, `spoiler_amp`
y los datos nuevos usados para calcularlas. Son amplitudes calculadas, no
mediciones del PWM ni confirmación de banderas activas en Arduino. El CSV y
`bridge.log` se reemplazan en cada ejecución.

La carpeta raíz se llama `MiYoke_PD_V2.0` pero el `.ino` se llama
`MiYoke_PD_V1.9.ino`. Arduino exige coincidencia entre carpeta y sketch. Las
compilaciones de verificación usan una copia de los archivos raíz dentro de
`.build-gust-source/MiYoke_PD_V1.9`; actualizar esa copia antes de reutilizarla.
No se cambió el nombre de tu sketch.

## Datos nuevos solicitados a Prepar3D

La definición SimConnect pasa de 7 a 17 valores FLOAT64. Se usan los nombres
del SDK Prepar3D v4 y se solicitan explícitamente las siguientes unidades:

| Variable | Unidad solicitada | Uso |
| --- | --- | --- |
| `INCIDENCE ALPHA` | degrees | Ángulo de ataque |
| `STALL ALPHA` | degrees | Ángulo de pérdida del modelo |
| `TRAILING EDGE FLAPS LEFT PERCENT` | percent over 100 | Posición real de flap izquierdo, 0..1 |
| `TRAILING EDGE FLAPS RIGHT PERCENT` | percent over 100 | Posición real de flap derecho, 0..1 |
| `DESIGN SPEED VS0` | knots | Referencia de pérdida con flaps |
| `DESIGN SPEED VS1` | knots | Referencia de pérdida limpia |
| `GEAR LEFT POSITION` | percent over 100 | Extensión del tren principal izquierdo, 0..1 |
| `IS GEAR RETRACTABLE` | Bool | Evitar vibración permanente en tren fijo |
| `SPOILERS LEFT POSITION` | percent over 100 | Despliegue real izquierdo, 0..1 |
| `SPOILERS RIGHT POSITION` | percent over 100 | Despliegue real derecho, 0..1 |

Se mantiene IAS en knots, presión dinámica, viento X/Y/Z, suelo y slew. El SDK
publica algunos ángulos en radianes y velocidades de diseño en ft/s; la petición
expresa las unidades requeridas para que SimConnect realice la conversión.
Se solicita la posición real de las superficies, no solo la palanca.

Datos opcionales no finitos o fuera de los rangos de sanidad anulan el efecto
que depende de ellos. No se inventa un `STALL ALPHA` si el avión devuelve cero.
Si SimConnect produce una excepción de definición/transporte, el puente se
cierra y desarma sus aportes; la placa conserva la lectura local de sensores. Un
add-on puede no publicar correctamente las variables estándar: hay que
comprobarlas en el CSV antes de interpretar ausencia de vibración como ausencia
de la condición aerodinámica.

## Matemática y procedencia

### 1. Buffet de aproximación a la pérdida

Referencia: `VPforce-TelemFFB-refactor/telemffb/sim/base/BuffetingEffectMixIn.py`,
función `ac_update_buffeting()`: banda de AoA, dependencia de flaps, mezcla de
velocidades VS0/VS1 y factor de flujo limitado. Adaptación implementada en
`SimConnectBridge/BuffetModel.h`, función `YokeBuffet::calculate()`.

Con `sat(x)=min(max(x,0),1)`:

```text
f = (flapLeft + flapRight) / 2
alpha_inicio = alpha_perdida × (0,5 + 0,2 f)
Vs_ref = Vs1 + (Vs0 - Vs1) f
r_AoA = sat((alpha - alpha_inicio)/(alpha_perdida - alpha_inicio))
flujo = min((IAS / Vs_ref)², 1,5)
A_perdida = sat(r_AoA × flujo)
```

La referencia TelemFFB utiliza TAS en su factor de velocidad. Aquí se utiliza
IAS como aproximación de carga relativa frente a las velocidades de diseño;
no se afirma que el SDK convierta las velocidades de diseño en velocidades de
pérdida reales para cualquier peso, altitud o configuración. La relación al
cuadrado está motivada por presión dinámica. Las fracciones 0,5/0,2 y el tope
1,5 provienen de TelemFFB y son decisiones del estimador, no leyes universales.
A mayor ángulo dentro de la banda, mayor amplitud. El tope final es 1.

El modelo se desactiva en suelo. Requiere ángulo de pérdida entre 1 y 45 grados,
AoA entre -90 y 90 grados, VS0/VS1 entre 10 y 400 kt y flaps entre 0 y 1. Son
límites de sanidad; fuera de ellos no se fuerza un resultado. No representa
pérdida con ángulo negativo ni caída asimétrica de un ala.

Se usa una portadora triangular de 13 Hz. TelemFFB configura 13 Hz para buffet
y contempla portadora triangular en su variante Dynamic; nuestra versión es
más simple: frecuencia fija, sin envolvente aleatoria ni dirección aleatoria.
No debe describirse como copia completa de su buffet Dynamic.

### 2. Buffet del tren desplegado

Referencia: `telemffb/sim/base/MotionEffectsMixIn.py`, función
`ac_update_landing_gear()`: amplitud dependiente de despliegue y velocidad,
con vibración en ambos ejes. La referencia tiene una frecuencia de 10 Hz y
admite umbrales relacionados con Vle para ciertos simuladores.

```text
A_tren = gear × sat((IAS - 100) / (150 - 100))
```

Se aplica solo en vuelo y con tren retráctil. Los umbrales **100 y 150 kt son
una calibración elegida para este proyecto**, no una transcripción de unidades
del código TelemFFB ni límites Vle de tu avión. Se usa portadora triangular a
`SIMCONNECT_GEAR_BUFFET_HZ` (ahora 2 Hz); TelemFFB usa seno. La version anterior
usaba 10 Hz: se cambio a 2 Hz para obtener una oscilacion lenta y evitar que
el muestreo del LCD a 100 ms la muestre casi inmovil. La ganancia maxima sigue
siendo 0,025. No simula el motor de extensión ni golpes de enclavado.
Para ajustar los umbrales, cambiar `BuffetModel.h` y recompilar el puente.

### 3. Buffet de spoilers

Referencia: `telemffb/sim/base/MotionEffectsMixIn.py`, función
`ac_update_spoilers()`: promedio de extensión izquierda/derecha, rampa con
velocidad y vibraciones alrededor de 14..18 Hz. Allí aparecen umbrales de
80 y 140 kt convertidos a m/s; aquí se trabaja directamente en knots.

```text
spoiler = (spoilerLeft + spoilerRight) / 2
A_spoiler = spoiler × sat((IAS - 80) / (140 - 80))
```

Se aplica solo en vuelo y se sintetiza una única portadora triangular a 15 Hz.
No se importan las cuatro senoides de TelemFFB ni la vibración de movimiento del
actuador. El promedio puede responder también a spoilers que asisten al alabeo:
se representa el despliegue real, no exclusivamente la orden de aerofreno.
Se omitió el corte de despliegue >0,1 de la referencia para mantener una entrada
continua desde cero. Los dos efectos de superficies son aproximaciones de
sensación: la vibración real transmitida al mando depende del avión.

### 4. Síntesis y mezcla en Arduino

`SimEffects.h` sintetiza las portadoras a partir del tiempo real `millis()`
en el lazo nominal de 2 ms. El puente transmite amplitudes, no muestras de la
onda; por eso los 10/13/15 Hz no quedan sujetos al muestreo de 50 Hz del serial.

```text
b = dt_ms / (100 + dt_ms)
a[n] = a[n-1] + b × (A_recibida - a[n-1])
fase = (fase + dt_ms × frecuencia_Hz) módulo 1000
triangulo(fase) = 0,004 fase - 1        si fase < 500
                 3 - 0,004 fase        en otro caso
vibracion = suma(a_efecto × triangulo × ganancia_efecto)
extra = turbulencia + vibracion × peso_eje × ganancia_total_eje
salida = limitar(extra, -0,25, 0,25)
```

El suavizado de amplitud tiene constante aproximada de 100 ms. La portadora
triangular tiene media nula y pico unitario, no necesita `sin()` en AVR. Sus
armónicos y RMS difieren de una senoide: no se asume equivalencia de sensación
a igual ganancia. Los pesos pitch=1 y roll=0,5 y ganancias moderadas son ajustes
propios. El límite conjunto evita que la activación simultánea incremente sin
control la fuerza total. No se suma ninguna fuerza nativa.

## Protocolo y pérdida de datos

SC2 usa 20 bytes: encabezado A5 5A, versión 2, flags, secuencia, presión y
ráfagas (mismos offsets 0..11), amplitudes de pérdida/tren/spoilers en offsets
12/14/16 (uint16 0..10000), CRC16 en 18. Todo es little-endian.

Arduino también reconoce paquetes v1 de 14 bytes y pone los tres buffets en
cero al recibirlos. Los puentes normales se identifican por SC1/SC2 para evitar
mezclar ejecutables y firmware sin intención. La Pico actual no anuncia joystick HID; solo usa USB CDC para recibir SimConnect y emitir identificacion/diagnostico.

Todos los aportes se retiran cuando no hay paquete válido/armado en 300 ms,
o DTR está desconectado. No se requiere habilitacion FFB por HID. No se espera por datos seriales:
se conserva el máximo de 32 bytes disponibles por ciclo. CRC incorrecto,
secuencia duplicada y valores fuera de rango no renuevan el watchdog.
El generador borra amplitudes al perder habilitación y arranca desde amplitud
cero al recuperar conexión; una pausa local del lazo >50 ms reinicia las
portadoras. En suelo/slew/pausa el puente desarma. Desarme y watchdog retiran inmediatamente toda la fuerza, sin esperar el decaimiento de las envolventes.

## Fuentes verificables

1. Copia local TelemFFB, `BuffetingEffectMixIn.py`, `ac_update_buffeting()`.
2. Copia local TelemFFB, `MotionEffectsMixIn.py`, `ac_update_landing_gear()` y
   `ac_update_spoilers()`; parámetros de clase al comienzo del archivo.
3. [Prepar3D v4 SDK: Simulation Variables](https://www.prepar3d.com/SDKv4/sdk/references/variables/simulation_variables.html): definición de las variables solicitadas.
4. [NASA Glenn: Dynamic Pressure](https://www1.grc.nasa.gov/beginners-guide-to-aeronautics/dynamic-pressure/): fundamento de q=rho V²/2, usado para motivar el escalado; no prescribe ganancias hápticas.
5. `MODELO_TURBULENCIA.md`: derivación y fuentes del efecto de viento existente.

Consulta de documentación: 25/09/2026. Las ecuaciones, unidades, ganancias,
restricciones y diferencias con TelemFFB se explicitan arriba para no atribuir
a una fuente decisiones tomadas para este yoke. TelemFFB declara GPL v3;
conservar la atribución y considerar sus condiciones al distribuir derivados.

## Validación y límites

Pruebas C++ en `tests/simconnect_checks.cpp`: protocolo v1/v2, todos los errores
de un bit en los paquetes, rangos, secuencia, timeout, recuperación parcial,
mezcla, datos inválidos, umbrales y banderas independientes, portadoras y
modelo de turbulencia. Las pruebas del descriptor FFB anterior se archivaron en native_ffb_legacy/.
La compilación no sustituye una prueba en Prepar3D ni una validación mecánica.

Para comparar, activar un efecto por vez. Registrar CSV al aumentar AoA,
extender/retraer tren y desplegar spoilers. Comprobar ausencia en suelo,
retirada al cerrar puente y recuperación sin golpe. No se procesan efectos SENO del simulador.


Resultados historicos del 25/09/2026, anteriores a esta migracion (validacion actual en VALIDACION_SIMCONNECT_V3.md): puente SC2 compilado, **17.278 comprobaciones correctas** y descriptor
HID CONSTANT/SENO verificado. Compilación Micro con todos los efectos: potes
26.020 bytes de programa / 1.498 de RAM; encoders 26.802 / 1.538. También
compilaron las cuatro banderas en cero (25.056 / 1.496) y la bandera general
en cero (23.518 / 1.391). No se cargó firmware ni se accionaron motores.

Identificación de las referencias locales (SHA-256):

- `BuffetingEffectMixIn.py`: `4A358058C86E3AEB9DA931581B287C2EE47B40A6E88A987EA9D8BCBB47A93DC7`.
- `MotionEffectsMixIn.py`: `86BED566EE1594E43A3F918A1E9F4A0E3A364B885445FF4DAA2A5A21F4AEA086`.


## Indicadores 0/1 al final de telemetry.csv

Se agregan cuatro columnas independientes a la derecha:

- `Turbulencia`: 1 si el puente está armado y `abs(gust_roll)>0.1` o
  `abs(gust_pitch)>0.1`. Exactamente +0.1 o -0.1 no activa el indicador.
- `Buffet_Perdida`: 1 si está armado y la amplitud de pérdida codificada en
  el paquete es mayor que cero.
- `Buffet_Tren`: mismo criterio para el tren.
- `Buffet_Spoilers`: mismo criterio para spoilers.

En los otros casos valen 0. Pueden aparecer varios 1 en la misma fila.
En modo monitor sin `--enable` todos quedan en 0; las amplitudes numéricas
siguen permitiendo observar el modelo. Los buffets usan la resolución del
paquete (1/10000), evitando marcar amplitudes que se cuantizan a cero.
La turbulencia se compara con el valor calculado antes del redondeo del CSV.

Estos indicadores describen la salida calculada y habilitada del puente en
esa muestra, no confirman envío de cada fila ni fuerza efectiva en motores.
El puente no recibe las banderas compiladas en Arduino, su estado HID ni el
PWM aplicado. Además, el umbral 0.1 de la columna Turbulencia es solo visual:
no cambia la zona muerta 0.02 ni los cálculos de fuerza del firmware.
Si no llegan muestras SimConnect no se generan nuevas filas; el último 1
registrado no significa que el efecto siga aplicado después del watchdog.

## Revisión del registro de vuelo del 25/09/2026

El CSV disponible contiene 4.927 muestras que abarcan 49,718 s, no los dos
vuelos completos de cinco minutos. IAS: 65,48..83,12 kt. Cuando la extensión
del tren era mayor que cero, IAS: 65,48..77,37 kt. Por ello el buffet del tren
quedaba anulado por su umbral de 100 kt. La vibración implementada representa
el flujo sobre el tren desplegado, no el ruido del motor al moverlo.

Flaps izquierdo/derecho = 1; STALL ALPHA = 13,98 grados. El inicio de buffet
calculado era 13,98 x 0,7 = 9,786 grados; AoA máximo registrado = 9,58 grados.
Spoilers izquierdo/derecho = 0 durante todo el registro. Esto explica las
columnas en cero sin atribuirlo a lluvia ni a tiempo de vuelo.

También se observó gear_fraction máximo = 0,010. Es compatible con un problema
de escala de GEAR TOTAL PCT EXTENDED, aunque el registro por sí solo no prueba
que se haya completado toda la extensión. Se reemplazó su lectura por
GEAR LEFT POSITION solicitado en percent over 100, documentado como fracción
normalizada por el SDK. gear_fraction ahora representa el tren izquierdo como
aproximación al despliegue general; no modela asimetrías ni averías del tren.
Hay que verificar en una nueva captura que vaya de 0 a aproximadamente 1 al
completar la extensión. Se conservó el umbral de 100 kt, pendiente de calibrar
para el avión utilizado. No requiere cambiar Arduino.
