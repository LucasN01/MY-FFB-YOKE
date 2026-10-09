# Auditoría de efectos del yoke para Cessna R182 / Skylane RG

> Auditoría de la versión anterior a las mejoras SC6. Los cambios implementados posteriormente y el procedimiento de prueba están en `CAMBIOS_REALISMO.md`, en la raíz del proyecto. Las fórmulas y evidencias de este informe se conservan como referencia histórica.

**Fecha:** 3 de octubre de 2026. **Proyecto:** MiYoke_PD_V2.3. **Objetivo confirmado:** versión retráctil; conservar todos los efectos. **Etapa actual confirmada:** cálculo normalizado y conversión a PWM mostrada en LCD; encoders y medición de corriente pendientes de implementación.

## Dictamen

El proyecto tiene una base válida como **generador de señales hápticas inspirado en TelemFFB**. Todavía no es un modelo de fuerzas del R182 validado en newtons y newton-metro. Algunas relaciones son físicas, otras reproducen algoritmos de TelemFFB y otras son decisiones empíricas del proyecto. Es importante presentar esas tres categorías por separado.

Las mejoras con mayor impacto serían: incorporar equilibrio por trim y flujo de hélice al centrado; separar amortiguación/fricción de las otras fuerzas; mejorar la excitación de rodaje; y calibrar el buffet con el comportamiento del avión simulado. Cambiar únicamente las ganancias no resuelve esas limitaciones.

No se modificaron fórmulas del firmware, ejecutables del puente ni archivos del avión durante esta auditoría. Se generaron este informe, evidencia reproducible y ejemplos numéricos. Las propuestas siguientes están explícitamente separadas del comportamiento actual.

## 1. Qué avión se analizó

El usuario indicó como original:

`C:\Users\norie\Downloads\fsxnative_cessna182rg\Cessna182RG_fsx`

La instalación activa de Prepar3D está en:

`C:\Users\norie\OneDrive\Documentos\Prepar3D v4 Add-ons\Cessna182RG_AF_Scrub\SimObjects\Airplanes\Cessna182RG_fsx`

Su paquete figura activo en `add-ons.cfg`; el escenario `CESSNA 182 APAGADO.fxml` referencia `Cessna Skylane FSX_RG`, igual al `title` de `aircraft.cfg`. `sim=Cessna182` selecciona `Cessna182.air`. Esto identifica el escenario configurado; no equivale a una consulta en vivo del título del avión durante la auditoría.

La copia instalada y la original tienen **el mismo `.air`**, SHA-256:

`642ed8b8f454e1b05c843504537edd3d46e2005c7d4bc8908475ed31cf54e1ec`

Al comparar las claves del `.cfg`, la instalada agrega únicamente `yaw_slip_control_gain=0` y `yaw_control_uses_trim=0`, en autopilot. Las incoherencias que se describen a continuación ya están presentes en el paquete original. El Read Me identifica a A.F. Scrub, mayo de 2017.

### Parámetros que realmente declara el modelo instalado

| Parámetro | Valor del archivo | Relevancia para los efectos |
|---|---:|---|
| Potencia / régimen nominal | 300 hp / 2850 rpm | No asumir empuje de un R182 estándar a partir del nombre visual. |
| `power_scalar` / `thrust_scalar` | 1,1 / 1,05 | Modificadores del simulador; no son mediciones de potencia o empuje. |
| Hélice | 2 palas; diámetro 6,666667 ft ≈ 2,032 m | Área de disco ≈ 3,243 m² para una futura aproximación de flujo. |
| Peso máximo | 3110 lb | Peso declarado; el peso instantáneo depende de carga y combustible. |
| Ala | 176 ft²; envergadura 36 ft | No confundir área del ala con área del elevador. |
| Elevador | 16,6 ft²; límites +28° / −21° | Geometría útil; no basta para conocer el momento de bisagra. |
| Alerones | 18,3 ft²; límites 20° arriba / 15° abajo | Deflexiones asimétricas; una relación fija 2:1 entre ejes no deriva de esto. |
| Flaps | 0°, 10°, 20°, 30° | El modelo no tiene el mismo recorrido que la referencia R182 de 1978. |
| VS1 / VS0 de referencia | 65 / 56 kt | Coinciden con la telemetría guardada, aproximadamente 65,01 / 56,01. |
| Crucero / máxima indicada | 175 / 200 kt | Son parámetros del addon, no límites certificados verificados. |
| Spoilers | límite 0°, sin spoilerons ni auto-spoiler | El perfil R182 debe conservar el algoritmo pero producir cero. |
| Alarma de pérdida | `type=1`, `stick_shaker=0` | No modelar el buffet natural como un stick shaker instalado. |
| Tren | puntos con tiempos 3,5 / 4,7 / 5,2 s | Conviene distinguir movimiento del tren de vibración aerodinámica. |

El archivo `cessna182_ref.txt` del propio addon contradice partes del `.cfg`: indica VNE 181 KIAS, VS 41 KIAS y VS0 39 KIAS; el bloque descriptivo `performance` también menciona otro peso máximo. Los textos de descripción no sustituyen los parámetros usados por el simulador.

### Contraste con una referencia real identificable

Se consultó el **POH Cessna R182 de 1978**, D1115-13, como referencia provisional; falta confirmar el año que se quiere representar. No se está mezclando con el Cessna **182R** de tren fijo.

| Dato del POH 1978 | Referencia verificable |
|---|---|
| 235 BHP a 2400 rpm | Sección 1, p. 1-3 |
| VNE 182 KIAS; VLE y VLO 140 KIAS | Figura 2-1, p. 2-4; hoja PDF 17 |
| Flaps hasta 40° | Figura 2-1 y figura 5-3 |
| Pérdida, 3100 lb, sin potencia, alas niveladas, CG delantero: 42 KIAS/55 KCAS limpios; 37 KIAS/52 KCAS a 40° | Figura 5-3, p. 5-11; hoja PDF 76 |

El manual advierte que esas KIAS son aproximadas. **No reemplazar directamente 65/56 por números del POH:** IAS y CAS difieren, los flaps no coinciden y modificar una referencia no corrige la aerodinámica del addon. [POH R182 1978](https://www.northcoastair.com/documents/Cessna_182RG_C182RG_1978_POH_scanned.pdf).

### Qué se pudo extraer del `.air`

Se recorrieron sus **125 bloques**, comprobando sus límites hasta completar los 9378 bytes. Se exportó el bloque `0x0404` de 17 pares de números de doble precisión. Su máximo tabulado es 1,34 en x=0,281; interpretando x en radianes corresponde a 16,10°. Esto es consistente con `STALL ALPHA=16,10°` del registro.

La identificación de ese bloque como curva de sustentación es una interpretación del formato heredado y su contenido; no se dispone aquí de una correspondencia oficial completa de todos los tokens de esta versión. Por eso el archivo exportado se llama `tabla_0404_sin_calibrar.csv`. No se extrajeron ni inventaron coeficientes de bisagra del R182 a partir de él. El SDK sí documenta bloques, tablas y su interpolación: [Flight Models, Prepar3D v4](https://www.prepar3d.com/SDKv4/sdk/simulation_objects/flight_models.html).

## 2. Evidencia de ejecución ya disponible

Se conservó una copia de `telemetry.csv` como `telemetry_snapshot.csv`, para que el siguiente vuelo no borre la evidencia.

| Medida | Resultado |
|---|---:|
| Filas | 35 189 |
| Intervalo registrado | 370,688 s |
| Filas en vuelo con `armed=1` | 19 213 |
| Ángulo de pérdida recibido | 16,10° constante |
| VS0 / VS1 recibidas | 56,01 / 65,01 kt constantes |
| Filas con presión normalizada saturada | 2248 |
| Spoiler izquierdo inválido | 35 189 de 35 189 filas |
| IAS máxima | 256,87 kt |
| Ángulo de ataque observado | −87,02° a +154,42° |

Este registro demuestra el comportamiento de las señales, **no valida fuerzas físicas ni un vuelo realista**: contiene estados extremos. El modelo de buffet desactiva su cálculo ante AoA fuera de ±90°. Los spoilers inválidos se convierten correctamente en efecto cero en `BuffetModel.h`; no deberían convertirse en vibración ficticia.

Hay 16 616 intervalos de timestamp cero y un intervalo máximo de 11 422 ms. El CSV se escribe por cada muestra recibida; la transmisión serie tiene su propia condición de 20 ms y el firmware sintetiza ondas a aproximadamente 500 Hz. No inferir la frecuencia del controlador contando filas del CSV.

Se recompiló y ejecutó `tests/simconnect_checks.vcxproj`: **77 503 comprobaciones pasaron**. Esto verifica invariantes de software y límites, no la fidelidad de un R182 ni el funcionamiento del hardware conectado.

## 3. Cadena actual de cálculo

`Prepar3D → SimConnect → main.cpp → modelos de envolvente/viento → serie SC5 → SimEffects.h → MotorDriver.h → PWM y LCD`

La Pico no recibe una fuerza en N. Recibe presión normalizada, ráfagas normalizadas, amplitudes de efectos y posiciones del mando del simulador. `MotorDriver` multiplica el comando firmado por `PWM_MAX` y redondea. Los valores del LCD etiquetados como `F` son comandos normalizados; **no son una lectura de fuerza**.

Los valores que siguen son los defaults del código. El menú puede guardar otras ganancias en flash; no se consultó la configuración persistida de la placa. Tampoco se verificó qué binario está cargado en ella.

Notación: `sat(x)` limita a [0,1]; `clip(x,a,b)` limita a [a,b]; `ramp(x,a,b)=sat((x−a)/(b−a))`; `T_f(t)` es la onda triangular de media cero y pico ±1 a frecuencia f.

## 4. Centrado aerodinámico

**Implementación:** `SimConnectBridge/main.cpp`, `SimEffects.h::update/command`, `mis_defines.h`.

El puente calcula:

\[
q=47{,}88025898\,q_{psf},\quad
q_{ref}=\tfrac12(1{,}225)(120\cdot0{,}514444444)^2\approx2334{,}24\;Pa
\]

\[
p=\operatorname{sat}(q/q_{ref}),\qquad k_n=0{,}15+0{,}85p.
\]

La rigidez se filtra con `b=dt/(100 ms+dt)`. Una vez estabilizada, antes de compensación por movimiento y saturación final:

\[
u_P=-0{,}9\cdot0{,}8\,k_n y_P,\qquad
u_R=-0{,}9\cdot0{,}4\,k_n y_R.
\]

`y` es posición normalizada, actualmente del simulador. Por encima de q_ref se pierde la dependencia con velocidad.

| EAS equivalente a esa presión | q [Pa] | k_n | Magnitud PWM pitch, medio recorrido |
|---:|---:|---:|---:|
| 0 kt | 0 | 0,150 | 13,77 |
| 60 kt | 583,56 | 0,3625 | 33,28 |
| 80 kt | 1037,44 | 0,5278 | 48,45 |
| 100 kt | 1621,00 | 0,7403 | 67,96 |
| 120 kt | 2334,24 | 1 | 91,80 |
| 160 kt | 4149,76 | 1 | 91,80 |
| 180 kt | 5252,04 | 1 | 91,80 |

Son ejemplos estacionarios con ganancias por defecto, sin otros efectos. EAS no es necesariamente IAS ni TAS. El fundamento de usar presión dinámica es físico: [NASA, Dynamic Pressure](https://www1.grc.nasa.gov/beginners-guide-to-aeronautics/dynamic-pressure/). **120 kt, 15 % residual y relación 2:1 no son constantes del R182.**

TelemFFB, en `MsfsXpFlightControlsMixIn.py`, también utiliza presión para la rigidez, pero añade flujo de hélice, offsets por AoA, trim y comportamientos de controles. Sus ganancias siguen siendo normalizadas, no mediciones certificadas de esfuerzo.

**Evaluación:** realismo parcial. Reproduce endurecimiento hasta un umbral; no el equilibrio completo de un mando convencional. Dos vuelos con distinta compensación pueden producir el mismo centrado para la misma posición y presión.

**Propuesta:** conservar el efecto, separar rigidez aerodinámica, carga mecánica y equilibrio de trim; calibrar K(q) en varios puntos y reservar la saturación para el límite del actuador. No basta con cambiar `--reference-knots` a VNE: eso desplaza la saturación pero no identifica una fuerza real.

El POH describe **downsprings en el sistema del elevador** y trim mediante tab, sección 7, p. 7-8. Por ello tampoco sería correcto eliminar toda carga a velocidad cero: hay que modelar la contribución mecánica separadamente; el 15 % simétrico actual no la identifica. [POH](https://www.northcoastair.com/documents/Cessna_182RG_C182RG_1978_POH_scanned.pdf).

### Modelo físico defendible para el centrado y trim

La forma lineal del coeficiente de bisagra, válida localmente antes de separaciones fuertes, es:

\[
H_e=q_tS_e\bar c_eC_{h,e},\qquad
C_{h,e}=b_0+b_1\alpha_t+b_2\delta_e+b_3\delta_t.
\]

`q_t`: presión efectiva en la cola [Pa]; `S_e` y `c_e`: referencias del área/cuerda detrás de la bisagra [m²,m]; `α_t`, `δ_e`, `δ_t`: ángulos de incidencia, elevador y tab [rad, con coeficientes por rad]. `H_e` es momento aerodinámico firmado [N·m]. La relación y la identificación experimental están documentadas en **NASA/TP-2000-209908, §5.3.4, p. 17**. Es una referencia de metodología para otro avión, **no una tabla de coeficientes Cessna**. [Informe NASA](https://ntrs.nasa.gov/api/citations/20000025766/downloads/20000025766.pdf).

Aplicando trabajo virtual, la carga equivalente en el yoke es:

\[
F_{P,aero}=H_e\frac{d\delta_e}{dx_P},\qquad
\tau_{R,aero}=\sum_{i=L,R}H_{a,i}\frac{d\delta_{a,i}}{d\theta_R}.
\]

Con H firmado como momento ejercido por el aire, el signo restaurador sale de los coeficientes y de las convenciones mecánicas. El esfuerzo estático del piloto para sostenerlo es el opuesto. Esto es una derivación propuesta por trabajo virtual, no código ya implementado. No convertir un momento de cabeceo del avión completo en fuerza del volante: son magnitudes distintas.

Faltan `b0..b3`, cuerdas de referencia y cinemática del mando real. `elevator_area` por sí sola no los suministra. Mientras no existan, usar una aproximación declarada:

\[
F_P=-K_P(q_t)[x_P-x_0(trim,\alpha,flaps,potencia)]+F_{mec}(x_P),
\]

con K y x0 identificados, no números supuestamente exactos. Con MobiFlight controlando los ejes hay que verificar qué hace el trim en Prepar3D para no sumar dos veces su acción. La ruta de trim de TelemFFB controla también ejes/offsets virtuales bajo ciertas opciones; copiar sólo el desplazamiento de resorte no copia esa interacción.

### Flujo de hélice

TelemFFB utiliza una aproximación de disco actuador:

\[
V_w=\sqrt{V_\infty^2+\frac{2T}{\rho A}},\quad
q_t=(1-\lambda)q+\lambda\tfrac12\rho V_w^2.
\]

Si q y V∞ son coherentes, `q_t=q+λT/A`. La relación ideal deriva de cantidad de movimiento; `λ` representa cobertura/transferencia a la cola y necesita ajuste. Usar T en N, densidad en kg/m³ y diámetro en m. **No copiar el factor aproximado ×10 de la conversión de empuje de TelemFFB sin verificar la unidad que devuelve P3D.** [NASA, Propeller Thrust](https://www.grc.nasa.gov/www/k-12/airplane/propth.html).

Así el elevador puede cambiar de carga con potencia aun con poca velocidad de avance. La fórmula no identifica por sí sola el flujo real de la hélice, pérdidas, downwash ni la carga del R182.

## 5. Turbulencia / ráfagas

**Implementación:** `GustModel.h`, `SimTelemetry.h::gustCommand`. **Referencia:** copia local `TurbulenceModulator.py`.

Con viento `w_i` en kt, cada componente se trata así:

\[
a=e^{-dt/1s},\quad \mu_i\leftarrow\mu_i+(1-a)(w_i-\mu_i),\quad h_i=w_i-\mu_i.
\]

\[
r^*=0{,}70\frac{h_X}{5}\,pG,\qquad
P^*=\frac{h_Y+0{,}25h_Z}{5}\,pG.
\]

El vector se limita a módulo uno y se filtra con `b=1−exp(−dt/0,08s)`. En firmware, una zona muerta continua elimina |s|≤0,02 y reescala el resto mediante `(abs(s)−0,02)/0,98`. Luego se aplican 0,35 para roll, 0,70 para pitch y ganancia total 0,9. Se apaga en tierra.

El pasaaltos equivale a la recurrencia `h[n]=a(h[n−1]+w[n]−w[n−1])`: es del mismo tipo que TelemFFB. Los cortes aproximados son 0,159 Hz y 1,99 Hz. La mezcla 0,70/1/0,25 procede de la referencia, pero no de identificación aerodinámica del R182. TelemFFB usa viento relativo en m/s, otra normalización, y signo lateral distinto. La entrada aquí es `AIRCRAFT WIND` en kt: no son intercambiables sin transformar. [Variables P3D v4](https://www.prepar3d.com/SDKv4/sdk/references/variables/simulation_variables.html).

**Evaluación:** aproximación háptica razonable, sin correspondencia cuantitativa demostrada con fuerzas de bisagra. El movimiento del avión ante una ráfaga no determina por sí solo el signo del esfuerzo que recibe la mano.

**Propuesta:** conservar filtros, límite vectorial y señales del simulador; identificar signos y ganancias por eje. Filtrar viento en ejes del mundo y transformar la perturbación a ejes del avión puede evitar interpretar una rotación del avión en viento constante como una nueva ráfaga. Validar esto con ensayos en el simulador.

Para un futuro modelo de bisagra, una perturbación vertical pequeña admite `Δα_t≈Δw_t/V_t` en radianes, siempre que Vt sea suficientemente grande. Entonces `ΔH_e≈q_t S_e c_e b1 Δα_t`; una ráfaga longitudinal modifica q aproximadamente como `Δq≈ρVΔV`. Son linealizaciones propuestas, no coeficientes nuevos del R182. No sumarlas otra vez si el cálculo completo ya usa el mismo viento y q instantáneos. Añadir un generador aleatorio de turbulencia independiente de P3D podría producir sensaciones sin movimiento correspondiente del avión.

## 6. Buffet de pérdida

**Implementación:** `BuffetModel.h` y portadora en `SimEffects.h`.

Con f igual al promedio de las fracciones de flap:

\[
\alpha_{inicio}=\alpha_{stall}(0{,}5+0{,}2f),\quad V_s(f)=V_{s1}+(V_{s0}-V_{s1})f
\]

\[
A_s=\operatorname{sat}\left[
\operatorname{ramp}(\alpha,\alpha_{inicio},\alpha_{stall})
\min\{(IAS/V_s(f))^2,1{,}5\}\right].
\]

Con la telemetría observada, el inicio va de **8,05° limpio a 11,27° con flap completo**. La amplitud se filtra a 100 ms; la portadora es triangular de **13 Hz**; las ganancias base son 0,24 en pitch y la mitad en roll, multiplicadas por la ganancia total.

La envolvente está claramente basada en `BuffetingEffectMixIn.py`. Sin embargo, el código local satura A_s a uno antes de aplicar la ganancia; TelemFFB multiplica por la intensidad antes de su saturación final. A cargas elevadas no son numéricamente idénticos. Tampoco se copió su dirección aleatoria ni el renderizador Dynamic completo.

**Evaluación:** defendible como aviso háptico asociado al AoA; no se demostró que 8,05°, 13 Hz o ese reparto entre ejes representen buffet del R182. El POH sitúa la alarma acústica 5–10 kt antes de la pérdida, p. 4-20; eso no establece ni la frecuencia ni el inicio del buffet táctil. [POH](https://www.northcoastair.com/documents/Cessna_182RG_C182RG_1978_POH_scanned.pdf).

**Propuesta:** mantener AoA como variable principal. Determinar `α_inicio` y `α_stall` por configuración/peso/CG del modelo, contrastándolos con el comportamiento esperado. Como forma suave propuesta puede emplearse `z=sat((α−α_inicio)/(α_stall−α_inicio))` y `s(z)=3z²−2z³`; este polinomio evita pendientes discontinuas, pero no es una ley experimental de Cessna. Usar q/q_ref de pérdida para escalar la excitación, con límite declarado.

Una señal de banda limitada e irregular se parece más a una vibración que un tono puro, pero su espectro necesita validación. El modo Dynamic de TelemFFB sirve como antecedente de implementación, no como prueba de frecuencia real. No inventar un efecto stick pusher ni un stick shaker.

## 7. Buffet del tren retráctil

\[
A_g=g\operatorname{ramp}(IAS,100,150),
\quad u_{P,g}=0{,}9\cdot0{,}16\,\bar A_gT_{10}(t),
\quad u_{R,g}=0{,}5u_{P,g}.
\]

g es la extensión del tren izquierdo; se exige avión en vuelo y `IS GEAR RETRACTABLE=1`. Se usa la misma suavización de amplitud de 100 ms. No es un modelo del motor hidráulico ni del golpe de bloqueo del tren.

**Evaluación:** dependencia cualitativa coherente con extensión y velocidad; umbrales y fuerza no validados. La cifra 150 kt no debe presentarse como límite del avión. El `#define` activo fija 10 Hz, aunque su comentario todavía menciona 2 Hz.

TelemFFB separa vibración aerodinámica, movimiento y clunk. Sus umbrales genéricos y los de este puente no deben equipararse sin revisar unidades: la telemetría de la referencia normalmente está en m/s y este puente usa kt.

**Propuesta:** conservar el buffet; distinguirlo del movimiento si se amplía el modelo. Medir una envolvente `A_g=k_g(q/q_g)g` o una curva por puntos; kg y qg son calibraciones. VLE puede servir como ancla documental del rango de ensayo, pero no prueba el inicio o el máximo del buffet. La frecuencia física no se deduce de VLE. Verificar las tres patas si se quiere reproducir la secuencia de extensión.

## 8. Buffet de spoilers

\[
A_{sp}=\tfrac12(s_L+s_R)\operatorname{ramp}(IAS,80,140).
\]

Portadora triangular de 15 Hz; ganancia 0,16 en pitch y la mitad en roll. TelemFFB usa múltiples portadoras en esa ruta. Las dos implementaciones son aproximaciones sensoriales.

**Evaluación para este addon:** no aplicable físicamente. Tiene `spoiler_limit=0`; el registro devuelve datos inválidos y el cálculo actual produce cero. **Se conserva el efecto en el código y en la documentación**, con una habilitación por capacidad del avión. Si se quiere mostrarlo en una exposición, usar un ensayo sintético identificado o un avión que sí tenga spoilers, no atribuírselo al R182.

## 9. Vibración de rodaje

\[
A_t=\operatorname{sat}\left(\frac{V_{suelo}-0{,}5}{39{,}5}\right),
\quad V_{suelo}>0{,}5\;kt,
\]

con cero fuera de tierra o en reposo. Onda triangular a **8 Hz**. Su salida en roll es `clip(0,9·0,18·Ā_t·T8,−40/255,40/255)`; pitch recibe un décimo. Se suma después de la compensación de movimiento y antes del límite total.

Los picos estacionarios roll son aproximadamente 4,71 PWM a 5 kt, 9,94 a 10 kt, 20,39 a 20 kt y un máximo de 40 PWM; pitch recibe un décimo antes del redondeo. El uso de velocidad sobre suelo es correcto: viento con el avión detenido no debe generar rodaje.

**Evaluación:** es el efecto con una excitación menos ligada a la física del modelo. La misma velocidad produce la misma onda aunque no haya irregularidad, cambie la superficie o se mueva una sola rueda. No se encontró fundamento R182 para 8 Hz fijos ni para 10:1.

TelemFFB utiliza variaciones de compresión de contactos, incluyendo diferencia entre ruedas laterales. Esa idea relaciona la excitación con el simulador. **No asumir compatibilidad directa:** `CONTACT POINT COMPRESSION` no aparece en la tabla de SimVars P3D v4 consultada; requiere una prueba de disponibilidad en esta instalación, especialmente con la interfaz FSX-SP2 del puente.

**Propuesta:** conservar rodaje. Primero probar compresión/aceleraciones efectivamente disponibles. Si no hay datos útiles de contactos, usar aceleración vertical filtrada y velocidades angulares, con eliminación del componente cuasiestático y gating de suelo. No multiplicar directamente gravedad por una ganancia para crear un empuje permanente.

Como alternativa empírica documentable, definir una rugosidad espacial h(s) de media cero, avanzar con `s=∫V_suelo dt` y filtrar su excitación con una transferencia estructural identificada. Una longitud característica λ implica `f=V_suelo/λ`, lo que fundamenta variar la frecuencia con velocidad. La rugosidad y la transferencia siguen siendo parámetros a calibrar; no se deben vender como mediciones del R182. La relación entre roll y pitch dependerá de cómo llega la vibración al yoke.

## 10. Amortiguación y fricción

**Implementación:** `SimEffects.h::motionAdjustedForce`; usa la velocidad de `YokeMotion`, filtrada con τ=40 ms, limitada a ±10 recorridos centro-extremo/s y con zona muerta 0,02.

Para una fuerza base u, define `v_m=max(|v|−0,02,0)` y aproximadamente:

\[
c=\min\left(0{,}12v_m+0{,}03\frac{v_m}{0{,}2+v_m},0{,}4\right)
\,G_{eje}\,\operatorname{sat}(|u|/0{,}1).
\]

Resta `sign(v)c` a u, pero trunca el resultado para que nunca invierta el signo de u. Si u=0, la salida es cero aunque haya movimiento.

**Evaluación:** no son un amortiguador ni una fricción independientes. Es una compensación de una fuerza preexistente. Puede alterar también la forma de los buffets porque se aplica a la suma previa. Este comportamiento explica por qué las etiquetas del menú no describen completamente la física.

Las formas mínimas propuestas son:

\[
F_d=-B\dot x,\qquad F_f=-F_c\tanh(\dot x/v_0).
\]

Para roll se emplean θ y velocidad angular. La fricción suavizada no representa adherencia estática exacta; modelarla requeriría más estado/identificación. Estos términos deben poder oponerse al movimiento aun cuando el resorte esté en cero, sin invertir arbitrariamente todas las demás fuerzas.

Como referencia de implementación, **OpenFFBoard** calcula spring con posición, damper con velocidad y friction como una condición separada. Es mejor antecedente para esa separación que el compensador actual, pero no aporta coeficientes de un R182. [EffectsCalculator.cpp](https://github.com/Ultrawipf/OpenFFBoard/blob/master/Firmware/FFBoard/Src/EffectsCalculator.cpp).

En la etapa LCD se pueden demostrar esas ecuaciones con la posición del simulador claramente etiquetada. Para fuerza física local, la velocidad debe venir del encoder del mando: los ejes del simulador pueden tener retardos, curvas, zonas muertas y otras intervenciones. No ajustar B como si la telemetría fuera una medición instantánea del eje mecánico.

## 11. Mezcla, límites y lo que muestra el LCD

La salida final es `clip(suma,−220/255,+220/255)` por eje y luego se convierte a PWM. Es un límite eléctrico/de comando, no una fuerza máxima conocida. Los efectos compiten por margen: al saturar, una vibración puede deformarse o desaparecer sobre un centrado fuerte. Conviene registrar las contribuciones antes y después de limitar, y el tiempo de saturación.

El LCD toma instantáneas nominalmente cada 100 ms, aproximadamente 10 Hz. Un efecto de tren a 10 Hz puede muestrearse siempre en una fase parecida y aparentar cero. La prueba existente imprime exactamente un caso con rango del efecto ±35,25 PWM equivalentes y muestras LCD 0/0. Esto no significa que el efecto no se genere.

**Propuesta para la demostración:** mantener el PWM instantáneo, pero añadir por efecto amplitud, pico y/o RMS en una ventana conocida, y distinguir «cálculo activo» de «actuador midiendo fuerza». No reducir la frecuencia física sólo para que el display la muestre. El flag `Turbulencia` del puente usa umbral 0,1, mientras el firmware empieza a responder desde 0,02: tampoco ese flag es prueba de PWM exactamente cero.

## 12. Cómo afectan tus motores y transmisiones

Datos confirmados por el usuario: motores DC 775, 24 V/20 A de fuente, BTS7960, reducción 3:1 en roll y 19:1 en pitch, roll ±90°, recorrido pitch aproximado 15–20 cm, encoders AB de 1000 PPR, sensores de corriente y cuatro finales de carrera previstos. No se aportó aún curva de fuerza/torque medida.

Para un motor DC de imanes permanentes:

\[
\tau_m=K_tI,\qquad V=RI+L\frac{dI}{dt}+K_e\omega.
\]

La tensión media PWM no determina por sí sola I cuando hay movimiento, fricción, temperatura y caída en el driver. [IIT Kharagpur, modelo de motor DC](https://vlabs.iitkgp.ac.in/ctrl/Exp3/theory.html). «775» identifica una familia/tamaño, no un Kt único. Los 20 A de la fuente tampoco son el límite identificado de cada eje.

Para una transmisión idealizada:

\[
\tau_{roll}\simeq\eta_R\,3K_tI_R,\qquad
F_{pitch}\simeq\frac{\eta_P\,19K_tI_P}{r_{ef}}.
\]

Estas son relaciones propuestas del montaje, no del mecanismo del avión. Falta `r_ef` o el paso del mecanismo lineal, Kt, eficiencias y pérdidas. A igual corriente la reducción 19:1 da 6,33 veces el torque de salida de 3:1 si los motores/eficiencias fueran iguales, pero pitch se compara en N y roll en N·m: **no se puede deducir de ahí una relación de ganancias 2:1 ni 10:1**.

También importan fricción, holgura e inercia reflejada, que crece como N² en el eje de salida bajo el modelo rígido ideal. Una reducción alta puede impedir sentir vibraciones rápidas aunque el PWM calculado sea correcto.

El código para encoders asume actualmente roll directo ±1000 cuentas y pitch ±2546 con una hipótesis de ±150 mm y polea de 75 mm 1:1. Eso no constituye calibración de tu montaje. Si el encoder de 1000 PPR x4 estuviera en el motor de roll antes de 3:1, ±90° de volante serían ±3000 cuentas, no ±1000. La ubicación del encoder y si los 15–20 cm son recorrido total todavía deben confirmarse.

En el código auditado no hay lectura de corriente ni lógica de los cuatro finales. Es coherente con la fase de demostración declarada; esas funciones serán necesarias para la posterior puesta en servicio mecánica. No se activaron motores para esta auditoría.

## 13. Arquitectura propuesta, manteniendo todos los efectos

| Capa | Responsabilidad | Referencia |
|---|---|---|
| Perfil R182 | Identidad, unidades, capacidades, geometría y procedencia de parámetros | Addon + POH del año elegido |
| Telemetría | Valores con validez individual, tiempo del simulador y conversión explícita a SI | SDK Prepar3D |
| Fuerza base | Presión efectiva, trim, AoA y carga mecánica del elevador | TelemFFB + modelo de bisagra |
| Perturbaciones | Ráfagas, pérdida, tren, spoilers y rodaje separados | TelemFFB; parámetros calibrados |
| Mecánica local | Posición, amortiguación, fricción y posteriormente límites | Encoders + referencia OpenFFBoard |
| Actuación | Fuerza/torque objetivo → corriente objetivo → regulador → PWM | Modelo DC e identificación del montaje |
| Evidencia | Contribuciones, unidades, RMS, saturación y valores medidos | Ensayos repetibles |

En la etapa actual se puede terminar en «comando normalizado → PWM de demostración». La arquitectura debe dejar claro dónde se incorporará el lazo de corriente, sin fingir que ya existe.

### Cambios por prioridad

1. **Documentación y observabilidad:** unidades explícitas, perfil R182, parámetros con fuente/estado, indicadores RMS y validación de SimVars. Mantener la reproducción actual como referencia A/B.
2. **Centrado y trim:** capturar trim, deflexión de elevador, presión, potencia/empuje disponible y condiciones de vuelo; construir una curva de equilibrio coherente con MobiFlight. Separar resorte mecánico de rigidez aerodinámica.
3. **Amortiguación/fricción:** ofrecer condiciones independientes para el modelo físico, conservando una opción de comparación con la compensación histórica. Preparar la fuente de encoder sin activarla antes de calibrar.
4. **Rodaje:** pasar de un tono fijo a excitación ligada al simulador o rugosidad explícita. Mantener ganancia y límite como ajustes del dispositivo.
5. **Buffets y ráfagas:** conservarlos, identificar umbrales y formas temporales; evitar importar coeficientes genéricos como constantes del R182.
6. **Fuerza física:** identificar transmisiones y Kt/corriente; medir fuerza/torque antes de afirmar correspondencia con el avión.

No se propone agregar vibración de motor ni otras funciones como requisito para conservar los efectos actuales. El flujo de hélice mencionado mejora el centrado; no implica introducir un nuevo efecto de vibración.

## 14. Validación que permitiría defender el realismo

| Ensayo | Variables / condición | Qué debe demostrar |
|---|---|---|
| Centrado | Presión y posiciones fijas; barrido de velocidad; otros efectos aislados | Curva K(q), simetrías/asimetrías y margen de saturación |
| Trim | Mismo régimen, peso, CG y posición; variar trim | La carga puede anularse en un equilibrio; signo correcto y sin doble aplicación |
| Potencia | Misma velocidad, distinta potencia | Efecto de flujo sobre pitch; no inventar el mismo cambio en roll |
| Ráfagas | Viento constante, cambios controlados y giros con viento uniforme | Distinguir cambio atmosférico de cambio de coordenadas; signos correctos |
| Pérdida | En simulador, por flap, peso/CG; registrar α, q, aviso y amplitud | Umbral y evolución coherentes; no una vibración permanente en vuelo normal |
| Tren | Extendido/retraído/parcial a igual IAS | Diferenciar extensión, buffet y futura secuencia mecánica |
| Spoilers | R182 y entrada sintética separada | Cero en el R182; algoritmo conservado y demostrable fuera de ese perfil |
| Rodaje | Detenido, rodando, despegado; superficies distintas si la telemetría las distingue | Cero en reposo/vuelo; excitación no idéntica para toda condición |
| Damper / fricción | Posición en cero con velocidad ±v | Resistencia opuesta al movimiento independiente de fuerza base |
| Presentación | Ondas a 8, 10, 13, 15 Hz | RMS/pico correcto aun si el LCD instantáneo muestra aliasing |
| Hardware futuro | Corriente, encoder y dinamómetro/celda de carga | N o N·m por corriente, pérdidas, histéresis y respuesta en frecuencia |

La validación del avión simulado y la del actuador son distintas. Un valor de telemetría correcto no prueba una fuerza correcta. Las tolerancias aceptables deben fijarse antes de medir, según objetivos del proyecto y disponibilidad de datos, no inventarse después de ver resultados.

## 15. Qué puede afirmarse en la exposición

> El prototipo transforma telemetría de Prepar3D en comandos hápticos mediante modelos inspirados en TelemFFB. La dependencia con presión dinámica y el modelo de cargas de bisagra tienen fundamento físico; las amplitudes, frecuencias y transferencias al dispositivo requieren identificación. En esta etapa se verifica el cálculo y la señal PWM. La validación de fuerza en el mando queda para la implementación de encoders, medición de corriente y ensayos de carga.

No afirmar «reproduce exactamente el Cessna 182» ni «PWM 100 equivale a determinada fuerza» sin medirlo. Sí se puede defender cada ecuación indicando: fundamento físico, antecedente de software, hipótesis, coeficientes empíricos y ensayo previsto.

## 16. Fuentes y trazabilidad

1. **Código auditado de MiYoke**: `SimConnectBridge/main.cpp`, `GustModel.h`, `BuffetModel.h`, `GroundModel.h`, `SimEffects.h`, `SimTelemetry.h`, `GainSettings.h`, `mis_defines.h`, `YokeMotion.h`, `MotorDriver.h`, `LcdTelemetry.h`. Hashes y rutas completas en `evidencia.json`.
2. **Copia local TelemFFB**: `MsfsXpFlightControlsMixIn.py` (presiones, coeficientes, trim), `TurbulenceModulator.py` (filtrado/mezcla), `BuffetingEffectMixIn.py` (pérdida), `MotionEffectsMixIn.py` (rodaje/tren/spoilers). Sin metadatos Git en la copia local: no se le atribuye un commit remoto. [Repositorio del autor](https://github.com/walmis/VPforce-TelemFFB), [documentación del autor](https://docs.vpforce.eu/telemffb/).
3. **Cessna Aircraft Company, POH R182 1978**, D1115-13. Páginas impresas 1-3, 2-4, 4-20, 5-11 y 7-8; copia local en `fuentes/R182_1978_POH.pdf`. [Copia consultada](https://www.northcoastair.com/documents/Cessna_182RG_C182RG_1978_POH_scanned.pdf). Referencia de una variante concreta, pendiente de seleccionar año objetivo.
4. **Lockheed Martin, Prepar3D v4 SDK**, [Simulation Variables](https://www.prepar3d.com/SDKv4/sdk/references/variables/simulation_variables.html) y [Flight Models](https://www.prepar3d.com/SDKv4/sdk/simulation_objects/flight_models.html). Unidades/variables y estructura del modelo, no una calibración de FFB.
5. **NASA Glenn**, [Dynamic Pressure](https://www1.grc.nasa.gov/beginners-guide-to-aeronautics/dynamic-pressure/) y [Propeller Thrust](https://www.grc.nasa.gov/www/k-12/airplane/propth.html). Presión dinámica y aproximación ideal del flujo de hélice.
6. **NASA/TP-2000-209908**, §5.3.4, p. 17, [informe](https://ntrs.nasa.gov/api/citations/20000025766/downloads/20000025766.pdf). Momento de bisagra y relación con fuerza medida; no importar sus coeficientes de otro avión.
7. **Ultrawipf/OpenFFBoard**, [EffectsCalculator.cpp](https://github.com/Ultrawipf/OpenFFBoard/blob/master/Firmware/FFBoard/Src/EffectsCalculator.cpp), casos SPRING/FRICTION/DAMPER. Referencia de arquitectura del actuador; no modelo R182 validado.
8. **IIT Kharagpur, Virtual Labs**, [Study of DC Motor](https://vlabs.iitkgp.ac.in/ctrl/Exp3/theory.html). Relación corriente/torque y ecuación eléctrica.

Las páginas online se consultaron el 03/10/2026. Los enlaces a ramas de software pueden cambiar; los hashes identifican la copia local realmente analizada.

## 17. Reproducir la parte numérica

Desde la raíz del proyecto:

```powershell
python tesis\auditoria_c182rg\auditar_modelo.py
```

Usa sólo la biblioteca estándar de Python. Lee el snapshot de telemetría existente; con `--refresh-telemetry` lo sustituye deliberadamente por el registro actual. No abre el puerto serie, el simulador ni los motores. Los CSV de ejemplos son cálculos estacionarios con defaults del código, no datos experimentales de fuerza.

Para completar la identificación física faltan el año/variante objetivo, modelo exacto/Kt de motor, radio o paso de transmisión de pitch, ubicación de encoders, recorrido total medido y curvas de carga. La disponibilidad de piloto o mando real para contraste tampoco quedó confirmada. Esos datos no impiden explicar el modelo actual; sí impiden afirmar o fijar una fuerza realista exacta en N/N·m.
