# Cómo calcula las fuerzas la copia local de VPforce-TelemFFB

Análisis estático de `VPforce-TelemFFB-refactor`, realizado el 29/09/2026. Describe esta copia local, no una versión genérica de Internet. No se modificó ni ejecutó el firmware ni se activaron motores. Los valores finales dependen del perfil de aeronave, las opciones activadas, el simulador y las capacidades del dispositivo.

## 1. La idea más importante

TelemFFB **no calcula una única fuerza en newtons que después convierte a PWM**. Calcula varios efectos independientes:

- **Resortes:** coeficientes de resistencia y posiciones de equilibrio para cada eje.
- **Fuerzas constantes actualizadas:** cargas por aceleración, ráfagas, caída del elevador, etc. “Constante” es el tipo de efecto HID; su valor puede cambiar en cada muestra.
- **Fuerzas periódicas:** frecuencia, amplitud, forma de onda, dirección y, a veces, fase y duración.
- **Condiciones:** amortiguación, fricción e inercia, que el dispositivo debe evaluar con su movimiento local.

El backend envía estos efectos al dispositivo. La generación temporal de ondas, evaluación de condiciones, mezcla final y accionamiento del motor pertenecen al dispositivo/backend. Esta aplicación no permite deducir por sí sola cuántos N o N·m produce un ajuste de 0,3.

Para interpretar un resorte ideal podés pensar en `F = -K·(posición-centro)`, pero esa expresión es una **interpretación para implementar tu controlador**, no la ecuación completa del firmware Rhino contenida aquí. Coeficientes positivos/negativos, saturaciones, deadband y ganancias del dispositivo también intervienen.

## 2. Recorrido de los datos

`SimConnectManager → telemetría → clase de aeronave y mixins → efectos → backend HID/DirectInput → dispositivo`

| Archivo | Qué hace |
|---|---|
| [SimConnectManager.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:362>) | Solicita variables del simulador y convierte algunas unidades. |
| [aircraft_base.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircraft_base.py:54>) | Compone los efectos comunes mediante herencia; administra efectos y estados. |
| [msfs_xp/Aircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:242>) | Prepara datos de MSFS/X-Plane y ejecuta la cadena correspondiente. |
| [MsfsXpFlightControlsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:1056>) | Modelo principal de resistencia de controles convencionales. |
| [ffb_rhino.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:781>) | Convierte magnitud/dirección/condiciones a informes del dispositivo. |
| [ffb_dinput.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1>) | Backend DirectInput genérico mediante un puente externo. |

La cadena usa llamadas cooperativas a `super().on_telemetry(...)`. Que dos efectos estén en archivos distintos no significa que sean excluyentes: pueden estar activos simultáneamente. Tampoco todas las funciones del repositorio se usan en MSFS o en un yugo.

### Variables relevantes y unidades de esta copia

Ver [definiciones SimConnect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:362>).

| Nombre interno | Variable/origen | Uso |
|---|---|---|
| `IAS`, `TAS` | AIRSPEED INDICATED / TRUE, m/s | Resistencia, umbrales y vibraciones. |
| `DynPressure` | DYNAMIC PRESSURE, Pa | Resorte aerodinámico. |
| `AirDensity` | AMBIENT DENSITY, kg/m³ | Flujo de hélice y normalización. |
| `G` | G FORCE | Efectos de carga G y caída del elevador. |
| `AccBody` | ACCELERATION BODY X/Y/Z, ft/s², factor 0,031081 | Aceleración expresada aproximadamente en g. |
| `AoA`, `StallAoA` | INCIDENCE ALPHA / STALL ALPHA, grados | Buffet, carga AoA y fuerza crítica. |
| `RelWind` | RELATIVE WIND VELOCITY BODY X/Y/Z, m/s | Turbulencia. |
| `VelWorld`, `AmbWind` | Velocidad y viento en ejes del mundo, m/s | Construcción del vector de incidencia. |
| `ElevDefl`, `ElevDeflPct` | Deflexión angular y fracción del elevador | Desplazamiento del centro por AoA. |
| `ElevTrimPct`, `AileronTrimPct` | Fracciones de trim | Centros y corrección del eje enviado. |
| `PropThrust` | PROP THRUST solicitado en kilograms, multiplicado por 10 | Aproximación usada como empuje en el cálculo de flujo. |
| `WeightOnWheels` | CONTACT POINT COMPRESSION de tres contactos | Detecta apoyo y genera irregularidades; **no es peso en N**. |
| `DesignSpeed` | VC, VS0, VS1, m/s | Velocidades de referencia. |
| `Flaps`, `Gear`, `Spoilers` | Posiciones/fracciones | Movimiento de mecanismos y buffet. |

Hay conversiones y convenciones específicas de cada simulador. No conviene copiar números de umbral sin copiar también su conversión de unidades.

## 3. Resistencia principal del yugo: el archivo que conviene estudiar primero

Archivo: [MsfsXpFlightControlsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:319>).

En las ecuaciones: `clip(x,a,b)` limita un valor; `mix(a,b,r)=(1-r)a+rb`; `ramp` es interpolación lineal limitada. Las fuerzas y coeficientes indicados son normalizados.

### 3.1 Velocidad e incidencia

- [`_calculate_airspeeds`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:319>): utiliza **IAS como velocidad de trabajo y la asigna a TAS** en esta ruta. No asumir que todo identificador TAS conserva aquí la velocidad verdadera. Convierte también `AccBody` a m/s² para datos auxiliares.
- [`_calculate_angles`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:348>): obtiene `beta=atan2(Incidence.x,Incidence.z)` en radianes y `AoA=-atan2(Incidence.y,Incidence.z)` convertido a grados.
- El vector `Incidence` parte de `VelWorld-AmbWind`, rotado por rumbo, cabeceo y alabeo en [Aircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:242>).

La sobrescritura de TAS/AoA se produce dentro de esta ruta. Otros mixins pueden haber procesado la muestra antes: no hay que atribuir automáticamente esta sustitución a todos los efectos del programa.

### 3.2 Flujo de hélice

[`_calculate_prop_airflow`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:388>):

```text
A = π·(diámetro_hélice/2)²
T = máximo empuje entre las hélices y cero
Vprop = sqrt(V² + 2·T/(ρ·A))
```

Es una aproximación para aumentar la resistencia de las superficies bañadas por la hélice. **Usa el máximo de los motores, no la suma.** El diámetro y las proporciones de flujo son ajustes. También calcula `_elevator_aoa`, pero ese dato no gobierna el resorte principal descrito a continuación.

### 3.3 Presiones por superficie

[`_calculate_dynamic_pressures`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:437>):

```text
s  = 0,005
qa = s·DynPressure
qe = s·mix(DynPressure, 0,5·ρ·Vprop², elevator_prop_flow_ratio)
qr = s·mix(DynPressure, 0,5·ρ·Vprop², rudder_prop_flow_ratio)
```

`qa`, `qe`, `qr` representan alerón, elevador y timón. Alerón no recibe ese refuerzo de hélice.

### 3.4 Normalización por velocidad de referencia

[`_calculate_vne_and_gains`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:482>): usa `vne_override` si está configurado; en X-Plane puede tomar Vne. Para MSFS estima una referencia desde VC:

```text
Tvne = 1,4·VC
qv = 0,5·ρ0·Tvne²
Vne = vsound·sqrt(5·((qv/P0 + 1)^(2/7) - 1))
Qvne = 0,5·ρ0·Vne²
Qgain = 1/(0,005·Qvne)
```

Aquí “Vne” puede ser **una estimación interna**, no necesariamente la Vne publicada de la aeronave. Revisar [conversions.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/conversions.py:1>): `std_air_pressure` es un nombre heredado para un valor de densidad y `vsound` tiene un TODO en el propio código. Conviene definir una referencia explícita para tu proyecto.

### 3.5 Coeficientes de resorte

[`_calculate_control_coefficients`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:563>):

```text
slip = 1 - slip_gain·abs(beta)
Ca = qa·Qgain·slip
Ce = qe·Qgain·slip
Cr = qr·Qgain·slip
```

Sin hélice y sin deslizamiento, el coeficiente es aproximadamente `q/Qvne`: crece con el cuadrado de la velocidad. Después aplica curvas exponenciales o curvas configurables. El factor `slip` no tiene una limitación local a cero en esa expresión.

[`_calculate_joystick_spring_coefficients`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:857>) transforma esos coeficientes a la resistencia configurada para X/Y. En BASIC el piso es cero; CENTER agrega un piso. Con piso cero y curva lineal:

```text
Kx ≈ max_aileron_coeff·clip(Ca,0,1)
Ky ≈ max_elevator_coeff·clip(Ce,0,1)
```

El modo avanzado consulta curvas. En esta ruta del yugo la entrada puede ser el coeficiente escalado, no directamente IAS; no confundirlo con las curvas de otros mixins que sí reciben velocidad.

Ejemplo ilustrativo, sin hélice, sin deslizamiento, sin expo: con `ρ=ρ0`, V=40 m/s y referencia=80 m/s, `C=0,25`. Si la ganancia máxima es 0,5, `K=0,125`. Para posición 0,4 y centro 0,1, un resorte ideal local daría `F=-0,0375`. A 60 m/s sería `K=0,28125` y `F=-0,084375`. **Son valores normalizados, no N.**

### 3.6 Trim: centro físico y corrección virtual

[`_calculate_joystick_trim_offsets`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:714>) calcula centros físicos y offsets virtuales. La ruta requiere trim following y control de ejes por TelemFFB.

- Roll: fracción de trim × ganancia física, limitada; offset virtual proporcional a `1-ganancia_virtual`.
- Pitch: procesa el trim con `Dampener`, aplica ganancia física y utiliza helpers de calibración cuando están habilitados.
- AP: puede reemplazar centros por posiciones de las superficies o valores calibrados.

[`MsfsXpSimConnectMixIn.py`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:167>) contiene los helpers de pitch. [`trim_follow_blend`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:488>) interpola curvas de trim calibradas a distintas IAS, alineando sus puntos de referencia. Fuera del rango de velocidades usa la curva extrema; limita el resultado a ±1.

[`_send_joystick_axis_commands`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:781>) resta offsets virtuales al eje físico y envía eventos de alerones/elevador. Por eso **mover el centro mecánico y corregir el mando enviado al simulador son dos partes del mismo problema**.

### 3.7 Desplazamiento del centro por AoA

[`_calculate_aoa_offset`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:827>), si está habilitado, en vuelo y con fracción de deflexión no nula:

```text
recorrido_estimado = ElevDefl/ElevDeflPct
centroY = clip(AoA/recorrido_estimado + force_trim_y_offset + trim_físico_Y, -1,1)
          ·clip(IAS/Vne,0,1)·aoa_effect_gain
```

De lo contrario usa el centro de trim. Es una heurística de desplazamiento del equilibrio; no calcula un momento de bisagra del elevador.

### 3.8 Fuerzas adicionales `control_weight`

[`elevator_droop_term_for`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:556>), [`_calculate_g_term`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:635>) y [`_apply_joystick_constant_forces`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:901>):

```text
droop = elevator_droop_moment·G/(1+qe)
gterm = g_force_gain·AccBody[1]·abs(dy)
pitch = clip(-droop-gterm,-1,1)
roll  = -AccBody[0]·lateral_force_gain       # si se habilita fuerza lateral
```

`g_force_gain` tiene valor base 0,1. `dy` es posición relativa al centro del resorte, **no fuerza medida en la mano**. Normaliza el vector si su módulo supera 1 y emite una fuerza constante con ataque de 1000 ms. Con poco aire, la caída del elevador pesa más; con mayor `qe`, disminuye.

Preservar y comprobar convenciones de dirección al portar: este método usa `atan2(roll,pitch)`, mientras la turbulencia usa `atan2(-roll,pitch)`.

### 3.9 Pedales, bloqueo y selección del modelo

- [`_calculate_rudder_force`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:670>): fuerza a partir de `(beta-rudderAngle)·qa·slip·Qgain`, limitada a ±1; procesa con `Dampener`, multiplica por `clip(IAS/Vne)` y envía en dirección 270°. Ese multiplicador agrega otra dependencia de velocidad.
- [`_apply_joystick_controls_lock`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:175>): resorte rígido centrado y detents cuando el dispositivo los admite. Es una restricción, no carga aerodinámica.
- [`msfs_update_flight_controls`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:1056>): separa controles convencionales, FBW, helicópteros, colectivo y calibración. El seguimiento de AP también puede seleccionar el modelo FBW.

## 4. Todos los efectos comunes: archivo, cálculo y salida

### `BuffetingEffectMixIn.py`: buffet de pérdida

Ver [`ac_update_buffeting`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/BuffetingEffectMixIn.py:93>) y [`_dynamic_buffet_render`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/BuffetingEffectMixIn.py:224>).

```text
AoA_inicio = StallAoA·(0,5+0,2·flaps)
Vref = VS1+(VS0-VS1)·flaps
aire = clip((TAS/Vref)²,0,1,5)
profundidad = ramp(AoA,AoA_inicio,StallAoA,0,1)
amplitud = clip(aire·profundidad·buffeting_intensity,0,1)
```

Requiere vuelo sin apoyo; existen alternativas cuando faltan velocidades/ángulos de referencia. Modo clásico: seno, frecuencia base configurable (13 Hz en el código), dirección aleatoria. Dinámico: onda triangular, frecuencia decreciente con la profundidad, envolvente aleatoria de 0,5 a 1,5 renovada cada 0,3–0,9 s y suavizada; multiplica amplitud por envolvente y 1,2, y limita. `ac_update_drag_buffet` es un stub, no un efecto implementado.

### `AoAEffectsMixIn.py`: dos fuerzas distintas

Ver [archivo](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AoAEffectsMixIn.py:38>).

- **AoA normal**, `ac_update_aoa_effect`: `A=ramp(abs(AoA),0,stall_aoa,0,max_aoa_cf_force)·ramp(TAS,Vmin,Vmax,0,1)`. Dirección 0° o 180° según signo. Usa umbral configurado y excluye joystick FBW.
- **AoA crítico**, `ac_update_aoa_reduction_force_effect`: promedio de ocho muestras; superado el umbral, `A=clip(max_force·((AoA_prom-inicio)/(fin-inicio))^1,5,0,1)`. Fuerza 180° en vuelo y con velocidad suficiente.

El efecto normal es independiente del desplazamiento del centro por AoA del modelo principal. Revisar ambos al configurar.

### `ElevatorDroopEffectMixIn.py`: caída extra a baja velocidad

Ver [archivo](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/ElevatorDroopEffectMixIn.py:13>). Fuerza constante 180°, ataque 1000 ms, decrece linealmente desde el máximo a velocidad cero hasta cero a 20 kt. **Es un efecto adicional al término `droop` del modelo principal.**

### `GForceEffectMixIn.py`: fuerzas G configurables

Ver [archivo](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:181>).

- **LEGACY:** normaliza G entre límites y aplica potencia, por defecto 2,2; resta una contribución de derivada filtrada y limita a 0–1; fuerza 180°.
- **NEW:** distingue G positivas y menores que 1/negativas, aplica rampas, derivada filtrada y posición relativa del mando. Condiciona la fuerza al lado hacia el cual se está desplazando el mando; usa 0°/180°. No equivale simplemente a `masa·G`.
- **ADVANCED:** evalúa curvas configurables de G positivas/negativas. Puede emitir fuerza constante o desplazar el centro de resorte mediante adjuster.

Tiene exclusiones por apoyo/estado/dispositivo y filtros contra saltos de telemetría. No atribuir automáticamente efecto a cada parámetro declarado: por ejemplo, el bloque LEGACY leído no multiplica su salida por `gforce_effect_max_intensity`. Es independiente del `gterm` del modelo principal.

### `DecelerationEffectMixIn.py`: desaceleración

Ver [`ac_update_decel_effect`](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DecelerationEffectMixIn.py:31>). En MSFS usa `AccBody[2]`, promedio de ocho muestras; por defecto se desactiva en vuelo. Cuando la media es menor que −0,03 g:

```text
A = clip(min(abs(media), deceleration_max_force)·decel_scale_factor,0,1)
```

Fuerza 180° o invertida. Rechaza cambios grandes y requiere TAS no nula. La condición escrita `avg_y_gs < -0.03 < 500` **no impone un límite superior de 500 a la aceleración**; no copiarla como si validara eso. En DCS existe otra rama que deriva TAS y escala por aerofreno.

### `TurbulenceMixIn.py` + `TurbulenceModulator.py`: ráfagas

Ver [adaptación al dispositivo](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbulenceMixIn.py:27>) y [algoritmo](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/TurbulenceModulator.py:75>).

Entrada: componentes de `RelWind`. Elimina la parte lenta con un pasaaltos y suaviza la fuerza resultante:

```text
a = hpf_alpha^(dt/(1/60))
b = 1-(1-smoothing_alpha)^(dt/(1/60))
h = a·(h + viento_actual - viento_anterior)
pitch_raw = h_vertical + 0,25·h_longitudinal
roll_raw  = -0,7·h_lateral
yaw_raw   = -0,5·h_lateral
D = 10-9·sensitivity
```

Para pitch/roll calcula el módulo, lo normaliza por D y limita a `intensity`; yaw tiene su propia limitación. Suaviza con `F=b·objetivo+(1-b)·F_anterior`. El yugo recibe magnitud y dirección `atan2(-roll,pitch)`.

No inventa ruido aleatorio: responde a cambios del viento relativo, que también pueden contener cambios por maniobras y rotación del avión. Esta implementación **no multiplica explícitamente la fuerza por presión dinámica**.

### `WindEffectMixIn.py`: otro efecto de viento

Ver [archivo](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WindEffectMixIn.py:26>). Toma módulo de `Wind`, pasaaltos de 3 y pasabajos de 15 según las clases del proyecto, limita/escala amplitud y emite fuerza con dirección aleatoria. No confundir `Wind` con `RelWind`: si el adaptador no aporta ese campo, este efecto no tiene la misma entrada que la turbulencia MSFS.

### `EngineRumbleMixIn.py`: motor y postcombustión

Ver [pistón](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:38>), [jet](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:139>) y [postcombustión](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:183>).

| Efecto | Amplitud | Señal |
|---|---|---|
| Pistón | Interpolación por RPM entre intensidades configuradas de bajas/altas RPM, con limitación. Reparte la amplitud entre portadora y batido según régimen. | `f=RPM/60`; cuatro senos alrededor de f y f+2 Hz, dos direcciones y modulaciones lentas. |
| Jet | Ganancia × RPM porcentual/100. | Dos senos: frecuencia base + `10·RPM/100`, segundo desplazado 10 Hz, con modulación. |
| Afterburner | Ganancia × nivel de postcombustión, limitado. | Ondas triangulares alrededor de 20 Hz con modulación lenta, direcciones 0°/45°. |

No derivan vibración estructural desde masa, rigidez y amortiguamiento de la aeronave: son síntesis de sensación dependiente del régimen.

### `MotionEffectsMixIn.py`: pista, tren, flaps y mecanismos

Ver [archivo](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:82>). Los números son constantes del código o valores base; el perfil puede cambiar parámetros configurables.

| Función / efecto | Cómo obtiene la fuerza |
|---|---|
| `ac_update_runway_rumble` | Pasaaltos de compresión del contacto delantero y de diferencia izquierda−derecha; multiplica por intensidad y limita a ±0,5. Dos fuerzas constantes variables con dirección aleatoria mientras hay apoyo. |
| `_bms_taxi_bumps` | Rama BMS: pulsos cuadrados de 15 Hz a partir del evento/intensidad de bache. |
| `ac_update_touchdown_effect` | En MSFS usa aceleración vertical `AccBody[1]`; rampa hasta fuerza máxima, dirección 180°. **El bloque está condicionado por estar apoyado; no es un detector de un único instante de impacto.** |
| `ac_update_flaps` | Mientras cambia la posición: cuadrada 180 Hz, intensidad configurada. |
| `ac_update_canopy` | Movimiento: cuadrada 120 Hz. Cierre: pulso 10 Hz, 40 ms, amplitud aumentada. |
| `ac_update_landing_gear` | Movimiento: cuadradas 150 Hz en dos direcciones. Final de recorrido: golpe 10 Hz/40 ms. Buffet: `I·extensión·ramp(velocidad,Vmin,Vmax)`, seno con frecuencia configurada en dos ejes. |
| `ac_update_speed_brakes` | Movimiento cuadrado 180 Hz; buffet proporcional a despliegue y velocidad, con umbral de activación y dirección aleatoria. Usa helper `_create_buffeting_effect`. |
| `ac_update_spoilers` | Movimiento: senos 118 Hz. Buffet: `I·despliegue·ramp(V,Vmin,Vmax)`, cuatro senos de 15/16 Hz en un eje y 14/18 Hz en el otro. |
| `ac_update_tailhook_effect` | Movimiento: diente de sierra ascendente 160 Hz; golpe al extremo 10 Hz/40 ms. |
| Reabastecimiento / boom | Movimiento: sierra descendente 150 Hz; golpe al extremo 10 Hz/40 ms. |
| `ac_update_wingfold_effect` | En suelo, movimiento: sierra descendente 100 Hz en dos direcciones; golpe al extremo 10 Hz/100 ms. |

Las comprobaciones de cambio usan ventanas temporales para sostener el efecto durante movimiento. No todos los buffets de mecanismos exigen vuelo en esta copia; tu adaptación agrega algunas exclusiones. Hay parámetros declarados de buffet de flaps/canopy sin una implementación dedicada equivalente a la de gear/spoilers en este archivo.

### `FFBForcesMixIn.py` y `HydraulicLossMixIn.py`: resistencia local

Ver [condiciones base](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FFBForcesMixIn.py:29>) y [pérdida hidráulica](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:1>).

El primero envía coeficientes limitados de **damper**, **friction** e **inertia**. La aplicación no calcula aquí `-b·velocidad_del_yugo`: encarga la condición al dispositivo.

Pérdida hidráulica obtiene un nivel de salud, suaviza su transición durante aproximadamente 2,5 s y, bajo el umbral, interpola damper/friction entre valores de pérdida y valores normales. Reemplaza esas condiciones para evitar duplicarlas. En esta implementación no hay una tercera condición especial de inercia por fallo hidráulico.

### `AdvancedSpringMixIn.py`, `PedalSpringOverrideMixIn.py`, `DeadzoneMixIn.py`

- [AdvancedSpringMixIn](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:1>): curva de ganancia frente a IAS; trim por botones con paso `rate·dt/4096`; suma el desplazamiento del efecto G si está en modo offset. Aplica adjuster o ruta alternativa según capacidades/tap.
- [PedalSpringOverrideMixIn](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/PedalSpringOverrideMixIn.py:40>): resorte estático, dinámico por TAS entre velocidades de referencia o curva avanzada; force trim reduce coeficiente y sigue la posición durante liberación. Parte de estas rutas excluye MSFS/X-Plane porque tienen control propio.
- [DeadzoneMixIn](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DeadzoneMixIn.py:25>): configura zona muerta; no es un generador de fuerza adicional.
- [DynamicSpringMixin](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DynamicSpringMixin.py:1>): conserva condiciones/handle de resorte; **el nombre no implica que aquí esté el cálculo de presión dinámica**.
- [AircraftParamsMixIn](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftParamsMixIn.py:1>): parámetros comunes y valores de referencia, no un modelo separado.

### `WeaponsEffectMixIn.py` y `FfbTapMixIn.py`

- [WeaponsEffectMixIn](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WeaponsEffectMixIn.py:26>): cambios de contadores/eventos producen pulsos: carga liberada 10 Hz/80 ms, disparo sierra 10 Hz/80 ms, contramedidas 50 Hz/80 ms. No corresponden a variables estándar de la ruta MSFS examinada.
- [FfbTapMixIn](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:90>): retransmite/modifica efectos DirectInput capturados del juego. Copia condiciones, escalas, centros, ondas, fases y envolventes; permite modificar el resorte. **No reconstruye sus fuerzas desde SimConnect.**

## 5. Variantes MSFS/X-Plane y controles especiales

| Archivo | Dónde y cómo interviene |
|---|---|
| [MsfsXpFBWFlightControlsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFBWFlightControlsMixIn.py:76>) | Resorte de ganancia fija configurable en lugar del modelo q/Vne. Seguimiento de AP puede llevar K a 1; centra con superficies, trim o calibración. Pedales tienen rama análoga. |
| [Aircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:118>) | Stick shaker: `StallWarning` activa cuadrada 14 Hz. Maneja también estados y resortes de colectivo/pausa según configuración. |
| [MsfsXpNosewheelShimmyMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpNosewheelShimmyMixIn.py:18>) | Solo pedales MSFS, suelo, no taildragger: `A=I·((freno-freno_min)/(1-freno_min))^2,5`. Frecuencia escala de 8 a 16 Hz entre velocidad mínima y tres veces ella; la escala de frecuencia no está limitada en esa expresión. |
| [MsfsXpSteeringFrictionMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSteeringFrictionMixIn.py:1>) | Dirección en suelo: camino spring combina vectores del resorte de timón y del tren, con fuerza de dirección `ajuste/40`; agua multiplica por extensión del timón acuático. Camino friction calcula fricción extra decreciente con GroundSpeed usando expo. Revisar llamadas reales: `on_telemetry` llama al método de fricción además de existir la selección de método para el resorte. |
| [MsfsXpTrimwheelMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpTrimwheelMixIn.py:49>) | Rueda de trim: resorte al valor de trim, inicialización rígida y después ganancia configurada; mueve eventos del simulador o sigue AP. No usa q como carga de rueda. |
| [TrimCalibrator.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2735>) | `_hold_stick_centered` usa K=1 y centros objetivo durante calibración. Ajusta curvas de relación trim/eje a distintas velocidades; no calcula momentos aerodinámicos físicos. |
| [GliderAircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/GliderAircraft.py:34>) | Force trim de planeador: captura posición y altera el resorte; liberación/reenganche/reset. |
| [PropellerAircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/PropellerAircraft.py:1>), [JetAircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/JetAircraft.py:1>), [TurbopropAircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbopropAircraft.py:1>) | Seleccionan clase y/o valores de configuración heredados. No contienen otro modelo independiente de fuerza de yugo. |

### Helicópteros: fuera del yugo de ala fija, pero con efectos propios

[HelicopterEffectsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HelicopterEffectsMixIn.py:77>):

| Efecto | Cálculo |
|---|---|
| ETL | Envolvente gaussiana con TAS entre umbrales, intensidad configurable. Frecuencia `RotorRPM/75·n_palas`, segunda frecuencia +4 Hz. |
| Overspeed | Por encima de umbral de velocidad, rampa no lineal con exponente 0,7; vibraciones relacionadas con la frecuencia ETL. |
| VRS | En descenso y debajo de velocidad límite: intensidad aumenta entre velocidades verticales umbral; ondas 10/12 Hz. El factor de velocidad interno usa límites proporcionales a la misma velocidad y vale 1 para velocidad positiva: no interpretarlo como una atenuación progresiva efectiva. |
| Rotor rumble | Frecuencia `RotorRPM/45·n_palas` y +2 Hz, cada una con media intensidad. |
| Blade slap | Usa señal nativa cuando corresponde o estima una envolvente por IAS, ángulo de descenso y G; `f=RotorRPM/60·n_palas`, amplitud × `clip(2/n_palas,0,6,1)`, sierras descendentes en dos ejes. |
| Force trim de colectivo | Cambia centro y coeficiente durante liberación/reenganche. Un parámetro denominado damper en esta ruta puede ser un resorte débil que sigue la posición, no amortiguación derivativa. |

La dependencia de frecuencia de ETL/rotor rumble no coincide con la frecuencia física de paso de pala usada en blade slap: son fórmulas distintas en el código.

| Archivo | Particularidad |
|---|---|
| [MsfsXpHeliControlsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:46>) | Resortes de cíclico y force trim: al liberar, centro sigue mando con coeficiente bajo/cero; al reenganchar conserva centro. Ajusta offsets físicos y virtuales. |
| [Helicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:196>) | Pedales/colectivo con modos estático, libre y trim, inicialización y bloqueos. |
| [HPGHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:217>) | Seguimiento AFCS/SAS específico, promedios móviles y pequeños desplazamientos del centro; ramas para manos/pies sobre el mando, pedales y colectivo. VRS específico en línea 881: estado discreto escala onda de 10 Hz. |
| [SASHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/SASHelicopter.py:61>) | Suaviza señales unos 100 ms y desplaza centros con pasos normalizados; no usa el modelo q del ala fija. |
| [XAW109Helicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/XAW109Helicopter.py:71>) | Usa tasas de trim del addon y `afcs_motion_rate/4096` para mover centros por muestra; ramas específicas de pedales/colectivo. |
| [FFBApiHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:729>) | Recibe centros/estados de trim de la API del addon y aplica ganancias de resorte. Durante liberación usa posición física y coeficiente bajo; sin API vuelve a rutas base. |
| [FlyInsideHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FlyInsideHelicopter.py:58>) | Parte del vector `FI_Vib` del addon: módulo, rampa RotorRPM 0–150, curva expo y ganancia; dirección del vector. Sustituye ETL/VRS genéricos. |
| [CowanSimHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/CowanSimHelicopter.py:1>), [TaogH500Helicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TaogH500Helicopter.py:1>) | Adaptación a aeronave y condiciones de envío/bloqueo. No agregan otro modelo aerodinámico general de yugo. |

## 6. Otros simuladores: dónde están sus fuerzas

Estos archivos ayudan a comparar técnicas, pero no deben confundirse con ecuaciones alimentadas por SimConnect.

- [aircrafts_dcs.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:269>): daño mediante pulsos aleatorios; posiciones/trim mediante resortes; shaker por AoA; resortes de copiloto y force trim. Las fuerzas de vuelo pueden provenir del juego y modificarse, en vez de reconstruirse con el modelo MSFS.
- [aircrafts_il2.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:176>): vibraciones de motor desde frecuencia/amplitud nativas; disparos/cargas y daño por eventos; buffet desde señal nativa; resorte/constantes/damper reconstruidos desde registros FFB para determinadas rutas de dispositivo. Las constantes se limitan por módulo y usan `atan2(-fx,fy)`.
- [aircrafts_msfs_xp.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_msfs_xp.py:1>): archivo de organización/importación de las clases separadas, no otro cálculo completo duplicado.
- [BaseTelemetryData.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1>): estructura y acceso a datos; no genera fuerzas.
- [AircraftEffectUtilsBase.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:1>): helpers comunes de dispositivo, cambios y estado. El índice complementario permite localizar métodos auxiliares.

No encontré un adaptador específico de Prepar3D equivalente al MSFS en esta copia. Las ideas se pueden portar al puente SimConnect existente, verificando disponibilidad, unidades y semántica de cada variable.

## 7. Matemática auxiliar y salida al hardware

### `utils/_math.py`

Ver [helpers matemáticos](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:240>).

- `scale` interpola y puede extrapolar; `scale_clamp` limita. Confundirlas cambia el comportamiento fuera del rango.
- `non_linear_scaling`: potencia de la entrada normalizada; curvatura positiva usa ese exponente, negativa su inverso absoluto.
- `expocurve`: mezcla lineal/exponencial con constante 5,5; no equivale a elevar simplemente a un exponente.
- `get_gain_from_speed` / `get_gain_from_gs`: interpolación de curvas configuradas, con ganancias porcentuales convertidas a fracción.
- `Dampener`: resta a la entrada una derivada filtrada multiplicada por una ganancia. **No es sinónimo de suavizar la entrada ni del efecto HID damper.**
- `Smoother`: medias por número de muestras o ventana temporal; una ventana por muestras cambia de duración cuando cambia la frecuencia de telemetría.
- Los filtros de esta biblioteca usan sus propias fórmulas. Si se portan, copiar la ecuación y el tratamiento de `dt`, no solo el número llamado cutoff.

### `hw/ffb_rhino.py`

Ver [condiciones](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:301>), [constantes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:781>) y [periódicas](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:864>).

```text
constante: magnitud limitada a [-1,1] → magnitud·4096
periódica: amplitud limitada a [0,1] → amplitud·4096
frecuencia f → período en ms aproximadamente round(1000/f)
dirección grados → representación angular 0..255
condiciones float → escala 4096 en setters
```

El tipo importa: algunos setters distinguen enteros ya expresados en unidades del dispositivo de floats normalizados. No multiplicar dos veces por 4096. Los helpers `HapticEffect` crean/actualizan/inician efectos; actualizar un efecto que ya corre no implica reiniciar su envolvente cada muestra.

`CP_scaled_axisXY` mide desplazamiento respecto del centro y normaliza cada lado por el recorrido restante; no es sensor de fuerza. El firmware del dispositivo resuelve el efecto condicional y la actuación del motor.

### `hw/ffb_dinput.py`, `ffb_backend.py`, `ffb_sdl.py`

Esta copia incluye soporte DirectInput genérico mediante `TelemFFB-DInput-Bridge`/`directlink.dlk`, con conversión entre la escala interna 4096 y DirectInput 10000. No es correcto describir **esta copia** como exclusivamente compatible con Rhino. Las capacidades especiales como adjuster y detents requieren rutas alternativas o no están disponibles de la misma forma. `ffb_backend.py` define contratos/capacidades; el backend SDL aparece como ruta obsoleta, no como modelo aerodinámico adicional.

La existencia de este backend no hace que cualquier firmware HID implemente automáticamente todos los efectos que TelemFFB solicita.

## 8. Qué significa para tu MiYoke_PD_V2.0

### Lo que ya está implementado

| Tu archivo | Relación con TelemFFB |
|---|---|
| [BuffetModel.h](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/SimConnectBridge/BuffetModel.h:1>) | Ya adapta inicio de buffet por flaps/AoA, referencia VS0/VS1 y escalado cuadrático, además de gear/spoilers. Usa IAS y agrega condiciones propias. |
| [GustModel.h](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/SimConnectBridge/GustModel.h:1>) | Ya adapta la idea pasaaltos → mezcla de componentes → límite vectorial → suavizado. Usa viento del avión en nudos, constantes de tiempo explícitas y un factor `q·gain`; **no es una copia numérica exacta** del modulador TelemFFB. |
| [SimEffects.h](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/SimEffects.h:1>) | Genera ondas locales para efectos recibidos y mezcla ráfagas, con límites y control de frescura de telemetría. |
| [LocalDamping.h](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/LocalDamping.h:1>) | Amortiguación local dependiente del movimiento: evaluar su convivencia con cualquier damper adicional. |
| [MiYoke_PD_V2.0.ino](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/MiYoke_PD_V2.0.ino:149>) | Recoge fuerza HID, normaliza, suma amortiguación local, mezcla efectos SimConnect y manda los motores. |
| [Joystick.cpp](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/Joystick.cpp:468>) | El evaluador de efectos que revisé implementa **CONSTANT y SINE**. No vi allí ramas que calculen SPRING/DAMPER/FRICTION/INERTIA. Un descriptor que los enumere no reemplaza su implementación. |

En tu proyección de dirección se usa `Fx=-magnitud·sin(θ)` y `Fy=magnitud·cos(θ)`. Esto exige revisar signos al portar vectores de otras rutas, además de polaridades mecánicas.

### Qué incorporaría primero

1. **Resorte dependiente de presión dinámica**, si querés que tu propio modelo gobierne la resistencia principal. Portar la cadena `q → coeficiente → Kx/Ky`, con límites y piso elegidos para tu mecánica.
2. **Centro de trim**, junto con una decisión explícita sobre el eje enviado al simulador. Primero centro estático verificable; luego compensación virtual y AP.
3. **Caída del elevador** como término independiente y graduable. Elegir entre la versión por velocidad y la del modelo principal antes de activar ambas.
4. **G/AoA adicionales** solo después de comprobar qué carga ya reproduce el resorte y qué entrega el FFB original del simulador.
5. **Vibraciones de motor/mecanismos** según utilidad. Buffet y ráfagas ya tienen una base en tu proyecto.

### Dos arquitecturas posibles

**Conservar el FFB principal del simulador y añadir efectos:** continuar la mezcla actual, evitando que un resorte nuevo duplique una resistencia que ya venga por HID. Es la evolución más cercana al diseño actual.

**Gobernar la resistencia principal desde tu puente:** enviar al firmware `Kx`, `Ky`, `centroX`, `centroY` y cargas adicionales. Evaluar el resorte con la posición local del yugo, a la frecuencia del control de motor. Esto evita que la frecuencia de SimConnect limite el lazo de resistencia. Alternativamente, implementar correctamente los efectos condicionales HID que el backend solicita.

Como propuesta para tu firmware —**no como código existente de TelemFFB**—:

```text
F_resorte_x = -Kx·(x-centroX)
F_resorte_y = -Ky·(y-centroY)
F_amortiguación = -B·velocidad_local
F_solicitada = resorte + amortiguación + cargas + vibraciones
F_mando = limitar_y_convertir_a_motores(F_solicitada)
```

El modelo TelemFFB sirve para estudiar sensaciones y dependencias. Para afirmar una fuerza física en N hace falta además una relación calibrada entre mando de motor, transmisión y fuerza en el yugo; para un modelo aerodinámico físico harían falta geometría y coeficientes de momento de bisagra que este esquema genérico no utiliza.

## 9. Límites y observaciones de la lectura

- Se analizaron los generadores de fuerza, helpers principales, backends y las partes relevantes de tu integración. La interfaz, traducciones y empaquetado no constituyen otros modelos de fuerza.
- La presencia de un efecto en código no garantiza que tu perfil lo habilite. `defaults.xml`, perfiles de aeronave y ajustes del usuario determinan ganancias, umbrales y modos efectivos.
- Hay posibles solapamientos: droop principal/extra, G principal/efecto G, AoA centro/constante, amortiguación local/condición de dispositivo y FFB nativo/resorte nuevo.
- Algunas fórmulas son deliberadamente empíricas; otras contienen peculiaridades que conviene revisar antes de copiar (IAS/TAS, empuje máximo, constantes de referencia, condiciones de salida y diferencias entre comentarios y ejecución).
- No se midieron fuerzas reales ni se verificó el comportamiento dinámico con hardware. Los enlaces y el índice corresponden al código disponible durante esta revisión.

El [índice complementario](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/INDICE_TELEMFFB_FUERZAS.md:1>) enumera los archivos de simulación y sus funciones para navegar al código exacto, incluyendo auxiliares y ramas que no intervienen en un yugo de ala fija.
