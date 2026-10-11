> Actualizacion 11/10/2026: en SC8 la Pico usa τ_H = 29.5 m / V_e, limitada a [0.25, 1.5] s (1.5 s con V_e=0), y ponderacion √Q (0..2), en lugar de min(Q,1). Ver la seccion de actualizacion de realismo (`sec:realismo`) de tesis/capitulos/02_realismo.tex. El resto de este documento conserva la descripcion historica.

> Actualizacion 30/09/2026: se conserva este modelo de viento, pero su salida ahora forma parte de la fuerza exclusiva de SimConnect. Se eliminaron la suma con FFB nativo y la habilitacion HID de actuadores. Ganancia global: SIMCONNECT_TOTAL_GAIN. Puesta en marcha actual: LEEME_SIMCONNECT.md.

# Modelo de turbulencia por SimConnect: implementación y justificación

Fecha: 25/09/2026. Ampliación SC2: ver `EFECTOS_SIMCONNECT.md` para los tres
buffets nuevos y las banderas independientes. Este documento desarrolla solo
el modelo de viento, que conserva sus ecuaciones. Implementación: `SimConnectBridge/GustModel.h`, integración en
`SimConnectBridge/main.cpp` y zona muerta en `SimTelemetry.h`.

## 1. Objetivo y alcance

Generar una perturbación háptica reproducible a partir de variaciones del viento
simulado. Se suma a CONSTANT + SENO + amortiguación local. No se reemplazan los
algoritmos HID, ni se calcula una fuerza de bisagra real en newtons.

No es un generador Dryden/Von Kármán ni una reconstrucción certificada de mandos.
La fuente de las perturbaciones es Prepar3D: si las variables no contienen
variaciones, este modelo no inventa turbulencia mediante ruido o una senoide.

## 2. Fuentes y trazabilidad

| Fuente | Parte utilizada | Alcance de la justificación |
| --- | --- | --- |
| [Prepar3D v4 SDK, Simulation Variables](https://www.prepar3d.com/SDKv4/sdk/references/variables/simulation_variables.html), secciones Aircraft Environment y Flight Instrumentation | `AIRCRAFT WIND X/Y/Z`, `DYNAMIC PRESSURE` y estados del simulador | Identifica variables y unidades; no prescribe cómo convertirlas a FFB |
| [NASA Glenn, Dynamic Pressure](https://www1.grc.nasa.gov/beginners-guide-to-aeronautics/dynamic-pressure/) | q = rho V² / 2 | Fundamenta la presión dinámica; no determina las ganancias del yoke |
| `VPforce-TelemFFB-refactor/telemffb/utils/TurbulenceModulator.py`, `update()` | Filtrado temporal, combinación de ejes y saturación vectorial | Referencia de implementación háptica; sus coeficientes no son datos experimentales del avión |
| `VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbulenceMixIn.py`, `update_turbulence()` y `_apply_joystick_turbulence()` | Uso de componentes de viento para producir fuerza variable en dos ejes | TelemFFB envía su propia fuerza constante; aquí se conserva el transporte serial existente |
| `VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py`, definición `SimVarArray("RelWind", ...)` | Permite identificar qué variables usa la referencia | Usa `RELATIVE WIND VELOCITY BODY` en m/s; no es la misma entrada que nuestro modelo |
| Versión anterior del puente, descrita en `LEEME_SIMCONNECT.md` | Escala de 5 kt, tiempos de 1 s y 80 ms, presión de referencia a 120 kt | Valores de continuidad con el proyecto, elegidos para ajuste; no constantes universales |

Consultadas las páginas el 25/09/2026. Para identificar exactamente la copia local
analizada, SHA-256 de `TurbulenceModulator.py`:
`508F3B75F31509F1F43D8046C8FC14A38CDA2A2C075C985E38DE65EDDE0B49D8`.

Se escribió una implementación C++ para esta arquitectura usando las ideas
indicadas. La referencia local declara GPL v3 en su `AGENTS.md`; conservar esta
atribución y revisar las obligaciones correspondientes antes de distribuir
código derivado de esa referencia.

## 3. Variables elegidas

Se solicitan `AIRCRAFT WIND X`, `Y`, `Z` en knots: componentes lateral, vertical y
longitudinal del viento en ejes del avión. Se mantienen X/Y del puente anterior y
se agrega Z. No se sustituyen por `RELATIVE WIND VELOCITY BODY`, que representa
velocidad relativa al viento e incluye el movimiento del avión.

Esto reduce la incorporación directa de cambios de velocidad del avión a la
señal de ráfagas. No elimina toda ambigüedad: al cambiar la actitud del avión,
un viento constante en ejes terrestres puede variar en ejes del avión. Por eso
el detector puede responder a maniobras con viento y no identifica de manera
inequívoca la causa atmosférica. Separarla exigiría procesar viento en ejes
terrestres y transformaciones de actitud, fuera de esta adaptación.

## 4. Ecuaciones implementadas

### 4.1 Separación del viento lento y la perturbación

Para cada componente `w_i`, en knots, y tiempo real entre muestras `dt`, en segundos:

```text
a = exp(-dt / tauH)                 tauH = 1,0 s
m_i[n] = m_i[n-1] + (1-a) × (w_i[n] - m_i[n-1])
h_i[n] = w_i[n] - m_i[n]
```

`m` sigue el viento sostenido y `h` conserva el cambio transitorio. La expresión
se obtiene resolviendo `tauH × dm/dt + m = w` con entrada constante durante cada
intervalo. Es la discretización exponencial del filtro de primer orden.

Al sustituir `m[n-1] = w[n-1] - h[n-1]` se obtiene:

```text
h[n] = a × (h[n-1] + w[n] - w[n-1])
```

Esta es la estructura de pasaaltos usada en `TurbulenceModulator.update()`.
Su coeficiente `alpha_ref^(dt/dt_ref)` equivale a `exp(-dt/tau)` cuando
`tau = -dt_ref / ln(alpha_ref)`. No se copiaron los valores por defecto de
TelemFFB: se conservó `tauH=1 s` del proyecto. A mayor tau, menor frecuencia de
corte del pasaaltos (`fc ≈ 1/(2 pi tau)`); esta relación se deriva de la ecuación,
no del comentario de la referencia que describe el sentido de alpha al revés.

La dependencia de dt mantiene las constantes temporales entre tasas de muestras;
no garantiza señales idénticas entre FPS distintos debido al muestreo y al aliasing.
El reloj utilizado es `GetTickCount64`, tiempo real del puente, no tiempo de
simulación acelerado. La calibración propuesta supone simulación a velocidad 1x.

### 4.2 Combinación de ejes

```text
u_roll  = 0,70 × h_X / Wref
u_pitch = (h_Y + kZ × h_Z) / Wref
Wref = 5 kt; kZ = 0,25 por defecto
```

Se toma de TelemFFB la idea de predominio vertical en pitch, contribución
longitudinal menor y peso lateral 0,70. Una ráfaga vertical altera el flujo que
incide en el avión; una longitudinal modifica la velocidad relativa y puede
alterar las cargas aerodinámicas. El valor y signo del esfuerzo real dependen
del avión, trim, geometría y transmisión de mandos. Los pesos son una
aproximación perceptual, no una derivación del momento de bisagra.

TelemFFB usa `-0,70` para roll. Aquí se mantiene `+0,70` para conservar el signo
X -> roll del puente anterior. No se afirma que la polaridad mecánica esté
validada. Se debe comprobar en el equipo. El término longitudinal puede
ajustarse entre -1 y 1 con `--gust-longitudinal`; 0 lo deshabilita.

### 4.3 Escalado por presión dinámica

```text
qPa  = pressurePSF × 47,88025898
qRef = 0,5 × 1,225 × (referenceKnots × 0,514444444)²
Q    = limitar(qPa/qRef, 0, 1)
v    = gustGain × Q × (u_roll, u_pitch)
```

NASA fundamenta `q = rho V²/2`. Aquí se usa la presión medida por el simulador;
la densidad 1,225 kg/m³ solo fija una referencia convencional, no reemplaza la
densidad del vuelo. `referenceKnots=120` por defecto no representa Vne.

Aplicar Q a la perturbación es una decisión del modelo para ponderar la carga
con el flujo; no demuestra proporcionalidad exacta del esfuerzo de mando con
q. El término longitudinal y Q son dos aproximaciones distintas y podrían
sobreponderar cambios longitudinales; kZ=0 permite evaluar esa contribución.

### 4.4 Saturación conjunta y suavizado

```text
L = sqrt(v_roll² + v_pitch²)
target = v / max(1, L)
b = 1 - exp(-dt / tauL)             tauL = 0,08 s
out[n] = out[n-1] + b × (target - out[n-1])
```

La limitación radial proviene de la estrategia de TelemFFB: un mismo factor
reduce ambos ejes y conserva la dirección del vector instantáneo al saturar.
La ganancia se aplica antes del límite y del suavizado. No se recorta cada
componente del viento previamente, evitando deformar su proporción.

El suavizado es otro filtro de primer orden derivado de la misma ecuación
diferencial. Su corte nominal es aproximadamente 1,99 Hz. La combinación
pasaaltos/pasabajos favorece perturbaciones transitorias; son cortes graduales.
Al ser una combinación convexa de vectores de norma <=1, la salida también
queda dentro de ese límite, partiendo de cero.

La preservación de dirección se refiere al limitador del puente: el suavizado,
las ganancias diferentes por eje del Arduino, la zona muerta y la saturación
final con CONST/SENO pueden modificar la dirección del esfuerzo final.

### 4.5 Zona muerta continua del Arduino

Para la señal recibida `g` limitada a [-1,1]:

```text
D(g) = 0                                      si |g| <= d
D(g) = signo(g) × (|g|-d)/(1-d)                si |g| > d
d = 0,02
extra_eje = D(g_eje) × coeficiente_eje × ganancia_total_eje
salida = limitar(base_FFB_y_amortiguacion + extra_eje, -1, 1)
```

El 2% es un ajuste de rechazo de señales pequeñas, no una propiedad física.
La función es continua: el límite por ambos lados del umbral es cero. Conserva
la escala máxima y la simetría de signo. Sustituye el salto que existía en 0,10.
El PWM final sigue siendo discreto, por lo que no se promete continuidad mecánica
perfecta. Se mantienen los máximos existentes: roll 0,100 y pitch 0,200,
multiplicados por la ganancia total y sujetos al margen de la suma final.

## 5. Independencia del Arduino y estados

- El cálculo exponencial se ejecuta en Windows, no en el AVR.
- El paquete serial actual SC2 tiene 20 bytes con CRC y secuencia; añade tres
  amplitudes de buffet. El receptor también acepta v1 de 14 bytes. El firmware
  procesa como máximo 32 bytes disponibles por ciclo y nunca espera un paquete.
- A los 300 ms sin paquete válido y armado, el aporte adicional es cero;
  continúa el procesamiento HID de CONST/SENO y la amortiguación existente.
- Pausa, slew, suelo o falta de frescura retiran/resetearán el modelo. El puente
  considera fresco un intervalo menor a 250 ms y envía desarme al detectar
  pérdida de datos; si muere el puente, actúa el watchdog independiente del AVR.
- Primera muestra después de reset inicializa la media y devuelve cero. Evita
  interpretar el viento ya existente como una ráfaga al reconectar.
- Se rechazan entradas no finitas, parámetros fuera de rango y componentes de
  viento mayores a 1000 kt en módulo; este último es un límite de sanidad de
  datos, no una envolvente de vuelo.
- Se conservan habilitación/pausa HID, comprobación DTR y limitación final.

## 6. Uso y validación

Se requiere el EXE recompilado y cargar el firmware actualizado para obtener
también la nueva zona muerta. La identificación actual es `YOKE17SC2`; actualizar puente y firmware juntos.

```powershell
.\SimConnectBridge\bin\Win32\Release\YokeSimConnectBridge.exe --port COM7 --enable --gusts --gust-gain 1 --gust-longitudinal 0.25
```

Sustituir COM7 por el puerto real. Para observar el modelo sin abrir el COM:

```powershell
.\SimConnectBridge\bin\Win32\Release\YokeSimConnectBridge.exe --seconds 60 --gusts --gust-gain 1
```

El CSV contiene las tres componentes de viento en knots, las dos salidas,
presión, estado y parámetros. Se reemplaza en cada ejecución (la generación previa queda como `telemetry.anterior.csv`). En monitor puede
mostrar fuerzas calculadas con `armed=0`; no significa que se hayan aplicado.

Prueba comparativa propuesta: mismo avión, misma velocidad, viento sostenido,
ráfagas, maniobras con viento constante, y kZ=0 frente a kZ=0,25. Comprobar luego
pausa, suelo, cierre del puente y reconexión. Registrar CSV y configuración.
Esto permite distinguir continuidad de señal de realismo subjetivo y detectar
respuesta a maniobras. CONST/SENO pueden contener turbulencia propia: la suma
puede duplicar parte de esa sensación, porque se conservaron ambos efectos.

Resultado de verificación del 25/09/2026: puente compilado, 17.278 comprobaciones
correctas y descriptor HID verificado. Arduino Micro: potenciómetros 26.020 bytes
de programa / 1.498 bytes RAM; encoders 26.802 / 1.538. No se cargó firmware ni
se verificó la respuesta de motores o vuelo.

Pruebas automatizadas: `tests/simconnect_checks.cpp` cubre protocolo, timeout,
mezcla, continuidad de zona muerta, viento constante, ráfagas, límite vectorial,
decaimiento, reinicio, datos inválidos, presión cero y comparación a 30/60/120 Hz.
`tests/ffb_descriptor_checks.py` verifica que se conserven CONSTANT/SENO.

## 7. Texto sugerido para la justificación del proyecto

Se implementó un modelo háptico de perturbaciones de viento basado en telemetría
SimConnect. Las componentes de viento en ejes del avión se separan de su valor
lento mediante filtros de primer orden y se combinan en señales de alabeo y
cabeceo. La estructura de filtrado, mezcla y limitación vectorial toma como
referencia la implementación TelemFFB identificada en este documento. El aporte
se pondera mediante presión dinámica normalizada y se suaviza antes de enviarse
al microcontrolador. Los coeficientes de mezcla y tiempos son parámetros de
ajuste perceptual, no coeficientes aerodinámicos identificados experimentalmente.
La arquitectura mantiene los efectos FFB originales y retira únicamente la
perturbación adicional ante pérdida de comunicación, sin bloquear el control.
