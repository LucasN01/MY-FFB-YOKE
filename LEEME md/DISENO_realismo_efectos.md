# Diseño: realismo y fundamentación teórica de los efectos

- Rama: `mejora/realismo-efectos` (creada desde `master` @ `650741b`).
- Diseño: Claude (2026-10-10). Implementa: Codex. Revisa: Claude.
- Este documento no modifica código.
- Origen: análisis de fundamento teórico de los efectos (sesión del 2026-10-10). Resumen del diagnóstico en la sección 1.

## 0. Alcance y decisiones del usuario

Incluido: turbulencia, buffet de pérdida, buffet de tren, buffet de spoiler, forma de onda de los buffets, centrado, damper, fricción y vibración en suelo.

**No se toca (decisión del usuario):**
- La conversión de comando ±1 a PWM queda con el mapeo directo actual: `MotorDriver.h`, `PWM_MAX`, `MOTOR_PWM_HARD_LIMIT`, `SIMCONNECT_EXTRA_LIMIT`, polaridades y techo de suelo 40 PWM. Sin control de corriente ni compensación de fuerza contraelectromotriz.
- Vibración en suelo: se mantiene **roll dominante** (pitch /10) porque así lo requiere el usuario del simulador, y se mantiene la **textura constante en pista** (para que vibre aunque Prepar3D entregue aceleraciones casi constantes). Se documentan como requisitos de usuario, no como física.

**Precondición (antes de que Codex empiece):** el árbol de trabajo tiene cambios del usuario sin commit: `AGENTS.md`, `CLAUDE.md` y el traslado de `CAMBIOS_MENU_GANANCIAS.md`, `CAMBIOS_REALISMO.md` y `LEEME-LANZADOR.md` a `LEEME md/`. `nueva-version.ps1` exige árbol limpio. El usuario los commitea (o descarta) antes de la implementación. Si siguen pendientes, Codex no los incluye en sus commits, no ejecuta el script y lo informa en HANDOFF.

**Organización de la entrega:** una sola versión, en dos commits de código con la suite de pruebas pasando después de cada uno:
- **A, sin estado:** §2.3–2.6, centrado (§2.8.3) y comentarios.
- **B, con estado:** §2.2, §2.7 y el resto de §2.8.

Así, A y B se pueden revertir por separado con git. Las partes de B además tienen flags de reversión.

Tampoco se toca: protocolo SC8, puente de Windows (`SimConnectBridge/main.cpp`), lanzador, menú (etiquetas, pasos, ids, formato persistido, `YOKE_SETTINGS_VERSION`), pines, filtro de 100 ms de envolventes, zona muerta de turbulencia, límite vectorial, mezcla 0.70/1/0.25 de viento, estimador de velocidad `YokeMotion.h`.

## 1. Diagnóstico que motiva cada cambio

| Efecto | Problema actual | Fundamento del cambio |
|---|---|---|
| Turbulencia | Pondera por `Q=q/q_ref` (∝V²). Pasaaltos fijo de 1 s. | Una ráfaga vertical `w` cambia el ángulo de ataque `Δα=w/V`; el momento de bisagra es `∝ q·Δα = ½ρVw`, lineal en V (Pratt & Walker 1954; Hoblit 1988). El `Δα` de una ráfaga sostenida decae porque el avión acelera en "plunge" con constante `τ = 2m/(ρVSa)`. |
| Buffet pérdida | Inicio al `0.5+0.2f` de `STALL ALPHA` (TelemFFB): equivale a ~1.3–1.4·Vs, vibra en aproximación. | 14 CFR 23.207 (texto anterior a la enmienda 23-64; hoy rige §23.2150, sin márgenes en nudos): el aviso de pérdida, que puede ser buffet aerodinámico inherente, comienza a no menos de 5 kt sobre Vs y a no más de max(10 kt, 15 % Vs). Con `C_L` lineal en α, `α_inicio/α_s = (Vs/(Vs+Δ))²`. **Hipótesis de diseño:** el buffet empieza en el margen máximo de esa ventana (en el C182 el aviso reglamentario es la bocina; el buffet es la sensación aerodinámica previa). |
| Buffet tren / spoiler | Rampa lineal en IAS con umbral (100–150 kt / 80–140 kt). | La fuerza de un flujo separado o estela es `q·S·C_p'` → proporcional a `q ∝ IAS²` (IAS≈EAS a baja altura), sin umbral. |
| Forma de onda de buffets | Triangular periódica fija. | El buffet es una respuesta aleatoria de banda estrecha (Mabey, AGARD R-750, 1988); el modelo estándar es ruido blanco filtrado por un resonador de 2.º orden (Newland 1993). |
| Centrado | `k=r+(1−r)Q`: subir "Centro reposo" ablanda por encima de Q=1. | La rigidez es suma de una parte mecánica constante y una aerodinámica ∝ q (momento de bisagra, Etkin & Reid 1996; Nelson 1998): `k = k_mec + k_aero·Q`. |
| Damper | `B` constante: al endurecerse el resorte con Q, el yugo queda subamortiguado. | Razón de amortiguamiento `ζ = b/(2√(kJ))` (Ogata). Mantener ζ exige `b ∝ √k`. El amortiguamiento aerodinámico de la superficie también escala con ρV ∝ √q. |
| Fricción | Coulomb regularizada por velocidad: sin adherencia, depende de una derivada ruidosa. | Modelo de Hayward & Armstrong (2000), basado en el modelo de Dahl: fricción con presliding y adherencia calculada solo con posición. Solo con sensor local (potenciómetros/encoders). |
| Suelo, textura | Amplitud con rampa lineal 0.5–40 kt; dos senoides fijas. | ISO 8608 (`G_d ∝ n⁻²`): la excitación en velocidad del perfil es ruido blanco con nivel ∝ V, así que la respuesta RMS de un sistema resonante crece ∝ √V. La textura pasa a ruido de banda estrecha (rugosidad aleatoria filtrada por la resonancia tren/estructura). **Solo la textura sintética**: la componente de aceleraciones de P3D conserva la rampa lineal, porque esas aceleraciones ya traen su propia dependencia de la velocidad y aplicarles √V contaría dos veces. |

## 2. Especificación por archivo

Notación: los literales nuevos van con los nombres indicados. Mantener CRLF/LF del archivo existente, estilo y densidad de comentarios. Nada bloqueante ni memoria dinámica en el lazo de 2 ms.

### 2.1 `mis_defines.h` — constantes nuevas

Agregar, junto a las constantes existentes de cada tema (no reordenar lo demás):

```cpp
// Centrado: rigidez = reposo (mecanica) + este coeficiente * Q (aerodinamica).
// 0.85 = 1 - 0.15 conserva identica la curva de fabrica (k=0.15+0.85Q).
#define SIMCONNECT_CENTERING_AERO_STIFFNESS 0.85f

// Damper: B escala con sqrt(rigidez/rigidez a Q=1) para mantener la razon de
// amortiguamiento zeta=b/(2*sqrt(kJ)). 0 = B constante (comportamiento anterior).
#ifndef YOKE_DAMPER_STIFFNESS_SCALING
#define YOKE_DAMPER_STIFFNESS_SCALING 1
#endif
#define YOKE_DAMPER_SCALE_MIN 0.50f
#define YOKE_DAMPER_SCALE_MAX 2.00f

// Friccion: 0 = Coulomb regularizada por velocidad (anterior);
// 1 = Hayward-Armstrong (adherencia + presliding, solo posicion).
#ifndef YOKE_FRICTION_MODEL
#define YOKE_FRICTION_MODEL 1
#endif
#define YOKE_FRICTION_PRESLIDE 0.020f // desplazamiento normalizado hasta deslizar

// Buffets: 1 = ruido de banda estrecha; 0 = triangular periodica (anterior).
#ifndef SIMCONNECT_BUFFET_NARROWBAND
#define SIMCONNECT_BUFFET_NARROWBAND 1
#endif
#define SIMCONNECT_BUFFET_DAMPING_RATIO 0.15f // ancho de banda ~2*zeta*f
#define SIMCONNECT_BUFFET_RMS 0.45f           // RMS de la portadora antes de recortar a +-1
// Textura de pista: 1 = ruido de banda estrecha 7/11.3 Hz; 0 = senoides (anterior).
#ifndef SIMCONNECT_GROUND_NARROWBAND_TEXTURE
#define SIMCONNECT_GROUND_NARROWBAND_TEXTURE 1
#endif
```

Agregar `static_assert` en el estilo del archivo: flags ∈ {0,1}; `0 < SIMCONNECT_CENTERING_AERO_STIFFNESS <= 4`; `0 < YOKE_DAMPER_SCALE_MIN <= 1 <= YOKE_DAMPER_SCALE_MAX <= 4`; `0 < YOKE_FRICTION_PRESLIDE <= 0.2`; `0 < SIMCONNECT_BUFFET_DAMPING_RATIO < 1`; `0 < SIMCONNECT_BUFFET_RMS <= 1`.

Corregir solo el comentario de `SIMCONNECT_CENTERING_MIN_STIFFNESS` (línea 228): `// Rigidez mecanica de reposo, sumada a la aerodinamica (0 = solo aerodinamico).`

### 2.2 `NarrowbandNoise.h` — archivo nuevo (raíz)

Generador de ruido de banda estrecha: ruido blanco uniforme filtrado por un resonador AR(2), normalizado a un RMS objetivo.

```cpp
#ifndef YOKE_NARROWBAND_NOISE_H
#define YOKE_NARROWBAND_NOISE_H
#include <stdint.h>
#include <math.h>
// Random narrow-band vibration: white noise through a 2nd-order resonator
// (Newland, Random Vibrations). Stationary RMS fixed by the AR(2) variance.
namespace YokeNoise {
class Narrowband {
public:
    // Coefficients are computed here, once, outside the control loop (static
    // construction before setup()). next() never calls expf/cosf/sqrtf.
    Narrowband(float hz, float stepSeconds, float zeta, float rms, uint32_t seed)
        :initialSeed(seed?seed:1u) {
        if(hz>0 && stepSeconds>0 && zeta>0 && zeta<1 && rms>0) {
            const float w=6.2831853f*hz;
            const float r=expf(-zeta*w*stepSeconds), theta=w*sqrtf(1-zeta*zeta)*stepSeconds;
            a1=2*r*cosf(theta); a2=-r*r;
            const float unit=(1-a2)/((1+a2)*((1-a2)*(1-a2)-a1*a1)); // var for unit-variance input
            gain=rms/sqrtf(unit/3.0f); // uniform [-1,1] has variance 1/3
        } // invalid parameters: a1=a2=gain=0 -> next() always returns 0
        reset();
    }
    void reset() { state=initialSeed; y1=y2=0; } // keeps coefficients
    // One step of the fixed period given to the constructor. Unclipped output.
    float next() {
        state^=state<<13; state^=state>>17; state^=state<<5; // xorshift32
        const float x=state*(2.0f/4294967296.0f)-1.0f;
        const float y=a1*y1+a2*y2+gain*x;
        y2=y1; y1=y; return y;
    }
private:
    uint32_t initialSeed, state=1;
    float a1=0, a2=0, gain=0, y1=0, y2=0;
};
}
#endif
```

Uso:
- Los coeficientes se calculan **solo en el constructor**. Los objetos `noise[3]` y `texNoise[2]` son miembros de `Effects`, y `simEffects` es un objeto global (`MiYoke_PD.ino:34`): se construyen en la inicialización estática, antes de `setup()` y fuera del lazo de 2 ms. `reset()` conserva los coeficientes. En el lazo, `next()` solo hace el paso AR(2): 3 multiplicaciones y el xorshift.
- El llamador avanza **un paso por ciclo de `Effects::update` con `dt>0`**. Con `dt==0` no llama y conserva la última portadora.
- El paso fijo es `CONTROL_LOOP_MS*0.001f`, pasado al constructor. Si el período real difiere de 2 ms, hay un error de frecuencia igual a esa diferencia (por ejemplo, 2.1 ms da un 5 %), aceptable para una vibración aleatoria.

Notas de diseño (para la revisión): la varianza estacionaria de `y=a1·y[-1]+a2·y[-2]+e` es `σe²(1−a2)/((1+a2)((1−a2)²−a1²))` (Box, Jenkins & Reinsel). Verificado por simulación con paso 2 ms y ζ 0.15: RMS 0.449–0.455 para objetivo 0.45 a 10/13/15 Hz, |media|<0.002, cruces por cero/2 ≈ f0, recorte |y|>1 entre 2.5 y 2.8 %.

**Por qué RMS 0.45 y no 0.577 (RMS de la triangular, 1/√3):** con 0.45 el pico de 1 equivale a ~2.2σ y solo se recorta ~2.6 % de las muestras. Con 0.577 se recortaría ~8 % y la señal perdería su carácter aleatorio. La energía de la portadora baja un 22 % (−2.2 dB) respecto de la triangular. Esto se compensa, si hace falta, subiendo la ganancia del buffet en el menú. Debe quedar dicho en la tesis.

### 2.3 `GustModel.h` — turbulencia

1. Firma: agregar un último parámetro `double tauHigh=1.0` a `Model::update` (después de `bool active=true`). El valor por defecto conserva el comportamiento anterior para pruebas que no lo pasan.
2. Validación: cambiar `q>1` por `q>2`; agregar `!std::isfinite(tauHigh) || tauHigh<=0` a la condición de reinicio.
3. Línea 29: `std::exp(-dt/1.0)` → `std::exp(-dt/tauHigh)`.
4. Actualizar el comentario de la cabecera: `q` es ahora el factor de velocidad `V_e/V_ref` (0..2), no la presión normalizada.

### 2.4 `PicoForceModels.h` — parámetros físicos de turbulencia

En `update`, reemplazar el cálculo del argumento `q` y agregar `tauHigh`:

```cpp
// Gust hinge load ~ q*dalpha = q*w/V ~ rho*V*w: scale with V_e/V_ref = sqrt(Q) (Pratt-Walker).
const double gustSpeed=pressure>0 ? std::sqrt(double(pressure)) : 0.0; // pressure <=4 -> 0..2
// Plunge time constant tau=2(W/S)/(g*rho0*a*V_e): sustained-gust dalpha decays as the aircraft
// accelerates vertically. C182RG: W/S=853 Pa, a=4.82/rad (Helmbold, AR 7.45) -> 29.5 m.
const double ve=std::sqrt(2.0*(qPa>0 ? qPa : 0)/1.225);
const double tauHigh=ve<=gustPlungeLength/1.5 ? 1.5 :
                     (ve>=gustPlungeLength/.25 ? .25 : gustPlungeLength/ve);
```

con `static constexpr double gustPlungeLength=29.5;` como miembro privado de `PicoForceModels` (comentario: "metros; 2(W/S)/(g·rho0·a), perfil C182RG"). Llamada: `gustModel.update(WindX,WindY,WindZ,dt,gustSpeed,s.gustGain,s.gustLongitudinal,air && (s.options&Gusts),tauHigh)`. Se elimina el `pressure>1 ? 1 : pressure` de esa llamada. Incluir `<cmath>` si hace falta.

Valores de referencia, con V_e≈IAS y `--reference-knots 120` (el valor por defecto):

| IAS | τ | Factor |
|---|---|---|
| 60 kt | 0.95 s | 0.50 |
| 80 kt | 0.72 s | 0.67 |
| 120 kt | 0.48 s | 1.00 |
| 140 kt | 0.41 s | 1.17 |

Antes: τ 1 s y factor `min(Q,1)` (60 kt → 0.25).

- El factor √Q es relativo a `referenceKnots`. τ, en cambio, depende de V_e absoluta: `--reference-knots` cambia el factor, pero no τ.
- **Aproximaciones declaradas:**
  1. Con TAS, `ρV = V_e·√(ρρ0)`. Usar `ρ0·V_e` subestima τ en `√(ρ0/ρ)`: −8 % a 5000 ft, −16 % a 10 000 ft. Se acepta como aproximación al nivel del mar.
  2. Hipótesis: el pasaaltos del viento en ejes del cuerpo representa el decaimiento de Δα por la respuesta en plunge, cuyo término es Z_w en las ecuaciones longitudinales (Etkin & Reid). P3D también simula esa respuesta del avión. El modelo solo usa su constante de tiempo para separar la parte transitoria de la ráfaga.

### 2.5 `BuffetModel.h` — envolventes

1. Pérdida, reemplazar la línea `const double onset=t.stallAlpha*(0.5+0.2*flap);` y mover el cálculo de `reference` antes:

```cpp
const double reference=t.vs1+(t.vs0-t.vs1)*flap;
// 14 CFR 23.207 (pre-amendment 23-64): stall warning (may be inherent buffet) starts
// 5..max(10 kt,15% Vs) above Vs; buffet onset at the upper margin is a design hypothesis.
// With C_L linear in alpha: onset/stall = (Vs/(Vs+margin))^2.
const double margin=reference*0.15>10 ? reference*0.15 : 10;
const double onset=t.stallAlpha*(reference/(reference+margin))*(reference/(reference+margin));
```

   El factor de flujo `(ias/reference)²` con tope 1.5 y el resto quedan igual.
2. Tren: `result.gear=t.gear*ramp(t.ias,100,150);` → `result.gear=t.gear*quadratic(t.ias,150);`
3. Spoiler: `...*ramp(t.ias,80,140);` → `...*quadratic(t.ias,140);`
4. Agregar junto a `ramp`:

```cpp
// Separated-flow/wake load ~ q*S*Cp' ~ IAS^2 (IAS ~ EAS at low altitude); full at `full` kt.
inline double quadratic(double v, double full) {
    const double x=v/full;
    return v<=0 ? 0 : (x>=1 ? 1 : x*x);
}
```
5. Actualizar el comentario de la línea 35: los 150/140 kt son una **calibración del yugo** para la IAS de intensidad plena. No son Vle ni salen de la física: la física solo fija la forma cuadrática.
6. Línea 6: cambiar la remisión `EFECTOS_SIMCONNECT.md` por `tesis/explicacion_efectos_proyecto_ffb_yoke.tex (seccion de actualizacion)`.

Valores de control: α_s 15°, VS0 45, VS1 60, flaps 0 → inicio 11.02°; flaps 0.5 → 10.58°; flaps 1 → 10.04°. Tren a 125 kt: 0.6944 (antes 0.5); a 80 kt: 0.2844 (antes 0). Spoiler 110 kt: 0.6173 (antes 0.5).

Cambio de comportamiento con flaps, que debe quedar dicho en la tesis: antes, con más flaps, el inicio en ángulo de ataque se **retrasaba** (0.5→0.7·α_s). Ahora se **adelanta**, porque baja Vs de referencia: 11.02° → 10.04°.

Ataque de sustentación nula: la fórmula supone `α_0L≈0` respecto del ángulo de cuerpo `INCIDENCE ALPHA`. La forma general, que va en la tesis, es `α_on = α_0L + (α_s − α_0L)·(Vs/(Vs+Δ))²`. Con `α_0L≈−3.5°` (típico de un ala con curvatura e incidencia de GA; hipótesis) el inicio sería 10.1° en lugar de 11.0°. No se implementa porque P3D no publica α_0L.

### 2.6 `PicoForceModels.h` — progresión con velocidad de la textura de suelo

`GroundModel.h` **no cambia**: la componente de aceleraciones de P3D conserva la rampa lineal.

En `PicoForceModels.h:39`, el nivel que usa la textura pasa a ser la raíz de la rampa:

```cpp
// Runway texture amplitude: ISO 8608 (G_d ~ n^-2) gives white profile velocity with
// level ~ V, so a resonant response grows ~ sqrt(V). Bumps from P3D keep the linear ramp.
const double ramp=YokeGround::envelope(s.value[OnGround],s.value[GroundSpeed],true);
groundEnvelope=float(std::sqrt(ramp));
```

`groundEnvelope` solo se usa en `SimEffects.h:68` (`roadLevel`): amplitud de la textura y recorte `±roadLevel` de la suma. El recorte se vuelve un poco más permisivo, porque √E≥E, y sigue acotado a 1. El techo de suelo de 40/4 PWM, el roll dominante y el divisor de pitch se mantienen (requisito de usuario).

Valores de la amplitud de textura: 10 kt → 0.4904 (antes 0.2405); 10.375 kt → 0.5; 20.25 kt → 0.7071 (antes 0.5); 40 kt → 1.

### 2.7 `FrictionModel.h` — archivo nuevo (raíz)

```cpp
#ifndef YOKE_FRICTION_MODEL_H
#define YOKE_FRICTION_MODEL_H
#include <math.h>
// Hayward & Armstrong (2000) haptic friction (Dahl-type presliding): an anchor w follows position x;
// z=x-w is the presliding (bristle) displacement, bounded by zmax. Static
// friction holds up to Fc without velocity estimation; sliding gives -Fc*sgn.
namespace YokeFriction {
class Model {
public:
    void reset() { ready=false; anchor=0; }
    // Returns normalized resistance with sign opposite to the displacement.
    float update(float x, float fc, float zmax) {
        if(!isfinite(x) || !isfinite(fc) || fc<=0 || !(zmax>0)) { reset(); return 0; }
        if(!ready) { anchor=x; ready=true; return 0; } // no startup kick
        float z=x-anchor;
        if(z>zmax) { anchor=x-zmax; z=zmax; }
        else if(z<-zmax) { anchor=x+zmax; z=-zmax; }
        return -fc*z/zmax;
    }
private:
    bool ready=false;
    float anchor=0;
};
}
#endif
```

### 2.8 `SimEffects.h`

1. Includes: `#include "NarrowbandNoise.h"` y `#include "FrictionModel.h"`.
2. `motionAdjustedForce`: agregar último parámetro `float damperScale=1.0f`; multiplicar solo el término de damper: `settings.effective(YokeGains::Damper)*damperScale*moving`. Rechazar `!isfinite(damperScale) || damperScale<0` devolviendo `base` sin corrección. Nada más cambia en esa función (las pruebas existentes deben seguir pasando).
3. Centrado (línea 84):
   `const float stiffness=rest+(1-rest)*(...)` → `const float stiffness=rest+SIMCONNECT_CENTERING_AERO_STIFFNESS*(...)`.
   Actualizar el comentario de la línea 82 si menciona solo envolventes; el filtro de 100 ms se conserva.
4. Buffets (bucle de las líneas 87–97): cuando `SIMCONNECT_BUFFET_NARROWBAND` y `i<3`, la portadora es
   `if(dt) carrier[i]=clip(noise[i].next(),-1,1);`
   `wave[i]=amplitude[i]*carrier[i];`
   en lugar de `triangle(phase[i])`. `carrier[3]` es un nuevo miembro `float carrier[3]` que se pone en 0 en cada reinicio. Con `dt==0` se conserva la última portadora. La fase se sigue actualizando (diagnóstico y compatibilidad). `i==3` (suelo SC3–SC6) conserva la triangular. Con el flag en 0, el comportamiento es idéntico al actual.
5. Textura de pista (líneas 71–72): con `SIMCONNECT_GROUND_NARROWBAND_TEXTURE`, mantener un miembro `float texCarrier[2]`:
   ```cpp
   if(roadLevel>0 && dt) {
       texCarrier[0]=texNoise[0].next();
       texCarrier[1]=texNoise[1].next();
   }
   // Same peak as the former sines (+-0.12*E): the runway texture stays soft (user requirement).
   const float texture=roadLevel>0 ? clip(roadLevel*.12f*(.65f*texCarrier[0]+.35f*texCarrier[1]),
                                          -.12f*roadLevel,.12f*roadLevel) : 0;
   ```
   RMS 0.7071 es el RMS de una senoide unitaria: misma energía que antes. **Recorte a ±0.12·E (aclaración del 2026-10-11, bloqueo de Codex en `8da61f6`):** el ruido puede superar en picos el máximo de las senoides, que la prueba existente `tests/raw_checks.h:25` exige con razón (textura "suave", requisito del usuario). Se conserva ese pico y la prueba no cambia. Simulación con las semillas del diseño, en 1000 pasos a E=1: se recorta el 2.5 % de las muestras, con un extremo de ±12.4 PWM (ganancia de suelo 0.45, total 0.9). Así también se cumple `low*255<-10 && high*255>10` de esa misma prueba. Cuando `roadLevel==0`, además de `roadTime=0`, reiniciar `texNoise[0..1]` y poner `texCarrier[0..1]=0`. El recorte posterior `clip(...,-roadLevel,roadLevel)` queda igual. Con el flag en 0 quedan las senoides actuales.
   Requisito de usuario: la textura **sigue presente siempre** que el avión rueda a más de 0.5 kt. Con ruido de banda estrecha la amplitud instantánea fluctúa (envolvente aleatoria) alrededor de un nivel medio constante, en lugar de ser periódica. Si el usuario prefiere la sensación periódica, flag en 0.
6. Miembros privados nuevos:
   ```cpp
   // Coefficients precomputed at construction (global simEffects: before setup()).
   YokeNoise::Narrowband noise[3]={
       YokeNoise::Narrowband(13,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,SIMCONNECT_BUFFET_RMS,0x9E3779B9u),
       YokeNoise::Narrowband(SIMCONNECT_GEAR_BUFFET_HZ,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,SIMCONNECT_BUFFET_RMS,0x7F4A7C15u),
       YokeNoise::Narrowband(15,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,SIMCONNECT_BUFFET_RMS,0x94D049BBu)};
   YokeNoise::Narrowband texNoise[2]={
       YokeNoise::Narrowband(7,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,.7071f,0x2545F491u),
       YokeNoise::Narrowband(11.3f,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,.7071f,0x6C8E9CF5u)};
   ```
   Las frecuencias 13/`SIMCONNECT_GEAR_BUFFET_HZ`/15 deben coincidir con el arreglo `frequencies` de `update` (se conserva para la fase y el modo triangular). Reiniciar con `reset()` no recalcula coeficientes.
   ```cpp
   float carrier[3]={0,0,0}, texCarrier[2]={0,0};
   mutable YokeFriction::Model friction[2];
   ```
   Reiniciar `noise[i]`, `texNoise[i]`, `carrier`, `texCarrier` y `friction[i]` en `reset()` y en la rama de deshabilitación de `update` (donde hoy se reinician fases y `roadTime`).
7. `command` (líneas 122–126). Sea `axisGain = gain*(axis==0 ? 0.5f : 1.0f)` (el valor que hoy se pasa a `motionAdjustedForce`).
   a) Escala del damper:
   ```cpp
   float damperScale=1;
   #if YOKE_DAMPER_STIFFNESS_SCALING
   // zeta=b/(2*sqrt(kJ)) constant -> b ~ sqrt(k). Only when the aerodynamic spring exists.
   if((mask & Centering) && rx.centeringAvailable) {
       const float reference=settings.effective(YokeGains::CenterRest)+SIMCONNECT_CENTERING_AERO_STIFFNESS; // k at Q=1
       damperScale=clip(sqrtf(fmaxf(centerPressure,0)/reference),YOKE_DAMPER_SCALE_MIN,YOKE_DAMPER_SCALE_MAX);
   }
   #endif
   ```
   (`reference ≥ 0.85 > 0` siempre.) Sin centrado disponible, la escala vale 1, igual que V1.3. Durante los ~100 ms de subida del filtro de rigidez tras armar, la escala parte de 0.5. Es aceptable y queda documentado.
   b) Fricción Hayward–Armstrong solo con sensor local. Definir en `SimEffects.h`:
   `#define YOKE_FRICTION_POSITION_MODEL (YOKE_FRICTION_MODEL==1 && YOKE_USE_SIMULATION!=1)`.
   En modo 1 (posición de SimConnect, que llega en escalones de ~20 ms y con latencia) un resorte de presliding sin amortiguamiento puede producir un ciclo límite. Por eso ahí se usa la ley por velocidad anterior.
   c) Con `YOKE_FRICTION_POSITION_MODEL`:
   ```cpp
   // Linear in base (tested): correction = motionAdjustedForce(0,...).
   const float damping=motionAdjustedForce(0,velocity,axisGain,(mask & Damper)!=0,false,settings,damperScale);
   float stick=0;
   const float fc=settings.effective(YokeGains::Friction);
   if((mask & Friction) && fc>0)
       stick=friction[axis].update(clip(position,-1,1),fc,YOKE_FRICTION_PRESLIDE)*clip(axisGain,0,1);
   else friction[axis].reset();
   // Same joint ceiling as V1.3: damper+friction never exceed the correction limit.
   const float ceiling=YOKE_MOTION_CORRECTION_LIMIT*clip(axisGain,0,1);
   extra+=clip(damping+stick,-ceiling,ceiling);
   ```
   d) Sin `YOKE_FRICTION_POSITION_MODEL`: la llamada actual a `motionAdjustedForce` con `damperScale` como último argumento.
   e) `FrictionModel::update` es idempotente para un mismo `x`: llamar `command` varias veces por ciclo no altera el estado. No hace falta una regla de llamada única; poner un comentario que lo diga.
8. Actualizar los comentarios de las líneas 25–27: describir los dos modelos de fricción y el escalado del damper, y cambiar la remisión `CAMBIOS_REALISMO.md` por la sección de actualización del `.tex`.

### 2.9 Pruebas (`tests/`)

Actualizar solo los casos que cambian por diseño (listados aquí) y agregar casos nuevos. No relajar ningún otro caso. Si otro caso falla, detenerse e informarlo en HANDOFF.

Casos que cambian:
- `simconnect_checks.cpp:109`: `command(extended,1,1,Friction,0,-1)>0` dependía de la fricción por velocidad. Con el modelo de posición (build de pruebas sin `YOKE_USE_SIMULATION==1`) reemplazar por dos llamadas: `command(...,Friction,0,0)` (fija el ancla) y luego `command(...,Friction,-0.05f,0)`, esperando `>0`.
- `tests/calibration_checks.h:375`, y cualquier otra prueba que pida fricción por velocidad con posición constante a través de `command` (aclaración del 2026-10-11): usar el mismo patrón. Primero una llamada en la posición inicial; después la posición desplazada ±0.05 en el sentido de la velocidad original, con el mismo signo esperado. Las pruebas de `motionAdjustedForce` no cambian.
- `simconnect_checks.cpp:238-244`: si fallan, es porque dependen del estado acumulado de `friction[1]`. Antes de cada caso, llamar `effects.reset()` o fijar el ancla con una llamada en la posición inicial. No cambiar los valores esperados.
- Buffets (`simconnect_checks.cpp:402-447`), entrada existente (α_s 15, VS0 45, VS1 60):
  - `aoa=7.5→0` pasa a `aoa=10.9→0` (flaps 0, inicio 11.02°).
  - `aoa=10→(0,1)` pasa a `aoa=13→(0,1)`.
  - El caso de flaps 1 con `aoa=10` (línea 410) sigue dando 0 (inicio 10.04°): se conserva.
  - Agregar `aoa=10.5`: flaps 0 → 0 y flaps 1 → `>0`, que documenta que ahora los flaps adelantan el inicio.
  - Tren: 100 kt → `near(.4444)`, 125 kt → `near(.6944)`, 150 kt → 1. Las comprobaciones `.5` de las líneas 415/419 se evalúan a 150 kt y no cambian.
- `raw_checks.h:91-104`:
  - La referencia `referenceWind.update(...,.02,1,1,.25,true)` pasa a `(...,.02,2.0,1,.25,true,0.25)`: 2000 psf → Q=4 → factor 2; V_e≈395 m/s → τ recortado a 0.25 s.
  - `air.buffet[1]` → `near(.6944f)`.
  - `air.buffet[2]` → `near(.5f*.7972f)` (=0.3986).
  - Si algún caso compara `groundEnvelope`, ahora es la raíz de la rampa.
- Las envolventes de `GroundModel` (`simconnect_checks.cpp:195-203`, `ground_checks.h`, `raw_checks.h:75`) **no cambian**.

Casos nuevos:
- Turbulencia:
  - `q=2` aceptado y `q=2.01` reinicia.
  - `tauHigh=0` o NaN reinicia.
  - Con un escalón de viento de 5 kt, `tauHigh=0.5` deja una `h` menor que `1.0` al cabo de 1 s.
  - `PicoForceModels` con presión equivalente a 120 kt y `referenceKnots` 120: factor 1.0 y τ 0.478 ± 0.005 s.
- `Narrowband`:
  - 50 000 pasos de 0.002 s a 13 Hz, ζ 0.15, RMS 0.45 → RMS en [0.405, 0.495], |media|<0.02, cruces por cero/2/T en [10.4, 15.6] Hz.
  - Autocorrelación normalizada con desfase de 1/f0 (≈38 pasos) **< 0.8**; la triangular da **> 0.99**.
  - `reset()` reproduce la misma secuencia.
  - Construido con `hz=0`, `zeta=0` o `zeta≥1` → `next()` devuelve siempre 0.
  - `reset()` no cambia los coeficientes: misma secuencia antes y después de reiniciar.
- `FrictionModel` (Fc 0.03, zmax 0.02):
  - El primer llamado da 0.
  - x 0→0.01 da −0.015; x 0.05 da −0.03 (el ancla queda en 0.03).
  - Volver a x 0.04 da −0.015 (z=+0.01). Volver a x 0.02 da +0.015 (z=−0.01).
  - Simetría con signos opuestos.
  - Fc 0 o NaN → 0.
  - El mismo x dos veces da el mismo resultado (idempotente).
- Límite conjunto: damper y fricción saturados nunca superan `0.4·axisGain`.
- Escala del damper:
  - Con `centerPressure` estabilizado en 0.15 (r 0.15, Q 0) → 0.5; Q 1 → 1; Q 4 (k 3.55) → 1.884.
  - Sin máscara `Centering` o con `!rx.centeringAvailable` → 1.
- Centrado: con r 0.15, `k*` idéntico al anterior para Q 0, 0.25, 1 y 4. Con r 0.30 y Q 2, `k*=2.0` (antes 1.70).
- Flags en 0: con amplitudes de envolvente fijadas en la prueba, la portadora, el damper, la fricción y la textura son iguales bit a bit a la fórmula de V1.3. Probar cada flag con un caso.
- Compilar y ejecutar `tests/simconnect_checks.vcxproj` (Win32) con 0 fallas.

## 3. Criterios de aceptación

1. Con amplitudes de envolvente fijadas y los flags `YOKE_DAMPER_STIFFNESS_SCALING`, `YOKE_FRICTION_MODEL`, `SIMCONNECT_BUFFET_NARROWBAND` y `SIMCONNECT_GROUND_NARROWBAND_TEXTURE` en 0, la portadora, el damper, la fricción y la textura son iguales bit a bit a V1.3. Las envolventes nuevas (turbulencia, pérdida, tren, spoiler y amplitud de textura) **sí** cambian: no tienen flag y se revierten con el commit A.
2. `Build-Pico2.ps1` (potenciómetros, por defecto) compila sin errores ni advertencias nuevas. Informar programa y RAM frente a 99 868 / 18 408 bytes. Compilar además con `-Encoders` para verificar la ruta con `YOKE_USE_SIMULATION!=1`.
3. Suite `simconnect_checks` en 0 fallas, informando el número total de comprobaciones.
4. Ningún cambio en los archivos de la sección 0 "No se toca" (`git diff master...HEAD --stat`).
5. Costo: `expf/cosf/sqrtf` solo en el constructor de `Narrowband` (inicialización estática, fuera del lazo); `next()` y `reset()` no los usan. Por ciclo se agregan a lo sumo 5 pasos de AR(2) y 2 llamadas a `FrictionModel::update`. Informar `us=` de `pico.log` si el usuario prueba en placa.

## 4. Validación del usuario en hardware (después de la revisión)

Hipótesis a validar con ensayo (sin fuente cuantitativa para este yugo):
- **Adherencia de la fricción:** con Q≈0 (tierra), pitch, r 0.15 y centrado pitch 0.8, el resorte vale 0.12·x. La fricción retiene hasta Fc=0.03, así que **el yugo no vuelve solo al centro si |x|<≈0.25** (25 % del recorrido; en roll, igual). Es físicamente realista: en tierra, sin viento, los mandos de un avión quedan donde se los deja. Aun así es un cambio de sensación. Criterio:
  - Si molesta, bajar "Friccion" en el menú (con 0.01 la zona baja a ~8 %) o volver a `YOKE_FRICTION_MODEL 0`.
  - En vuelo, la zona se achica con Q: a Q=1, |x|<0.03/0.72≈4 %.
- **Prueba de oscilación en reposo:** con Damper en 0 en el menú, soltar el yugo en distintas posiciones. No debe aparecer vibración ni ciclo límite. Si aparece, subir `YOKE_FRICTION_PRESLIDE` a 0.03–0.04. Si al soltar se ve un rebote, bajarlo a 0.015.
- `SIMCONNECT_BUFFET_DAMPING_RATIO=0.15` y `SIMCONNECT_BUFFET_RMS=0.45`: el buffet debe sentirse irregular, no como un zumbido periódico. Si se siente débil, subir la ganancia del buffet en el menú (la portadora tiene −2.2 dB respecto de la triangular).
- **Tren sin umbral:** con tren abajo en aproximación (~80 kt) habrá buffet de tren al 28 % de su máximo. Antes era 0 por debajo de 100 kt. Evaluar si se siente bien. Los 150/140 kt de intensidad plena son calibración.
- **Textura de pista:** comparar rodaje a 5, 10 y 20 kt. Ahora la textura crece como √V: a 10 kt es el doble que antes. Los baches de P3D conservan la rampa lineal.
- Frecuencias 13/10/15 Hz y 7/11.3 Hz: se mantienen como resonancias de estructura (hipótesis).
- **Escala del damper:** comparar el retorno del yugo en tierra y a ~120 kt. No debe rebotar a alta velocidad ni sentirse "suelto" en tierra (mínimo 0.5).
- **Centro reposo distinto del de fábrica:** con la ley nueva, subir "Centro reposo" endurece en todo Q. Con r=1 y Q=4, k=4.4 (antes 1). Si hay ajustes guardados con r>0.15, el resorte satura antes; revisar el PWM a alta velocidad.
- **Perfil C182RG (W/S, a):** si se usa otro avión, recalcular `gustPlungeLength = 2(W/S)/(9.81·1.225·a)`.
- **Rollback rápido sin revertir la versión:** poner en 0 `YOKE_FRICTION_MODEL`, `YOKE_DAMPER_STIFFNESS_SCALING`, `SIMCONNECT_BUFFET_NARROWBAND` o `SIMCONNECT_GROUND_NARROWBAND_TEXTURE` y recompilar.

## 5. Versión

Commit por nombre de todos los archivos (código, pruebas y documentación). Luego:

```
powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Mejora basada en incorporar realismo a los efectos y fundamentacion teorica: turbulencia, buffets, centrado, damper, friccion y suelo"
```

El texto aparece como `CAMBIO:` en `ACTUALIZACION_Vx_y.txt`. No editar ese archivo a mano.

Ver la precondición de la sección 0 (árbol limpio).

## 6. Documentación afectada

Regla: solo las partes afectadas. Cada cambio en los `.tex` se marca para poder fundamentarlo:
- Comentario LaTeX `% [V-realismo]` en la línea anterior a cada párrafo, ecuación o tabla modificada.
- Una nota visible al final de cada subsección modificada: `\textit{Actualización (realismo): <qué cambió y por qué>; ver Sección~\ref{sec:realismo}.}`.

La sección nueva lleva `\label{sec:realismo}`. Los números de línea "~" son orientativos (archivo de 2513 líneas): ubicar cada parte por su `\label` o por el título de la subsección indicado entre comillas.

### 6.1 `tesis/explicacion_efectos_proyecto_ffb_yoke.tex` y `tesis/main.tex`

Hoy los dos archivos son idénticos (`diff -q` sin diferencias): aplicar exactamente los mismos cambios a ambos.

**A. Sección nueva**, ubicada antes de `\section{CENTRADO DINÁMICO (RESORTE)}`: `\section{Actualización: realismo y fundamentación teórica de los efectos}\label{sec:realismo}`. Contenido:
1. Un párrafo: motivo (análisis de fundamento teórico del 2026-10-10), qué no cambia (mapeo ±1→PWM, protocolo SC8, roll dominante y textura de pista en suelo por requisito del usuario).
2. Tabla "Cambio | Antes | Ahora | Fundamento | Referencia" con una fila por cada ítem de la sección 1 de este diseño, con las fórmulas de ambos lados.
3. Subsección "Derivaciones" con las cuatro derivaciones cortas:
   - ráfaga: `Δα=w/V`, `ΔH ∝ qΔα = ½ρVw` → factor `V_e/V_ref=√Q` (relativo a `--reference-knots`); constante de plunge `τ=2m/(ρVSa)=2(W/S)/(gρ0 a V_e)` (aproximación a nivel del mar: con TAS `ρV=V_e√(ρρ0)`, error −16 % a 10 000 ft; término Z_w de Etkin & Reid; hipótesis de que el pasaaltos representa el decaimiento por plunge), con datos C182RG (W=3100 lbf, S=174 ft², b=36 ft, AR 7.45, a=2πA/(2+√(A²+4))=4.82/rad) → 29.5 m/V_e; tabla de τ y factor para 60/80/120/140 kt.
   - pérdida: `C_L ∝ α` y `L=W` → `α_inicio/α_s=(Vs/(Vs+Δ))²`, Δ=max(10 kt, 0.15Vs) de 23.207 (anterior a la enmienda 23-64; hoy §23.2150); declarar como hipótesis que el buffet empieza en el margen máximo; dar la forma general `α_on=α_0L+(α_s−α_0L)(Vs/(Vs+Δ))²` y la diferencia numérica con `α_0L=−3.5°` (10.1° vs 11.0°); decir que con flaps el inicio ahora se adelanta (antes se retrasaba).
   - damper: `ζ=b/(2√(kJ))` → `b∝√k`; límites 0.5–2.
   - suelo: ISO 8608 → `G_v(f)=4π²G_d(n0)n0²V` (blanco, ∝V) → RMS ∝ √V. Se aplica **solo a la textura sintética**; los baches de P3D conservan la rampa lineal (evita contar dos veces la dependencia con V).
   - tren/spoiler: carga de flujo separado `q·S·C_p'` → `∝ IAS²` (IAS≈EAS); 150/140 kt son calibración de intensidad plena.
4. Subsección "Generador de banda estrecha": ecuaciones AR(2), `r=e^{-ζω0Δt}`, `θ=ω0√(1−ζ²)Δt`, varianza estacionaria y ganancia; valores verificados (RMS, cruces por cero, % recortado); justificación de RMS 0.45 frente a 0.577 de la triangular (−2.2 dB, compromiso contra el recorte); en la textura de pista, RMS de senoide y recorte al pico anterior ±0.12·E (2.5 % recortado) para conservar la textura suave pedida por el usuario.
5. Subsección "Fricción de Hayward–Armstrong": ecuaciones del ancla, rigidez de presliding `Fc/zmax=1.5`, adherencia sin velocidad; solo con sensor local (en modo SimConnect se conserva la ley por velocidad); zona de adherencia frente al resorte en tierra (|x|<0.25 con valores de fábrica) y en vuelo; techo conjunto damper+fricción 0.4.
6. Subsección "Hipótesis a validar con ensayo": la lista de la sección 4 de este diseño.
7. Subsección "Referencias teóricas incorporadas" con la lista completa de la sección 7 de este diseño.

**B. Partes existentes que quedan incorrectas** (actualizar solo esas):
- Centrado:
  - Ec. `\label{eq:rigidez}` → `k^*=r+k_aQ_{\rm rx}`, con `k_a=0.85`; la tabla de la línea ~221 no cambia, pero aclarar que coincide porque `k_a=1−0.15`.
  - Reemplazar el párrafo de `∂k*/∂r=1−Q` (líneas ~230–234) por `∂k*/∂r=1`: "subir Centro reposo endurece igual en todo Q".
  - Fila de la línea ~373, fila del menú (~388) y la cota de ganancias (~408): reemplazar `r+(1−r)Q` por `r+k_aQ`.
  - Subsección "Separar rigidez de reposo..." (~446): indicar que quedó implementada.
- Turbulencia:
  - Tabla de símbolos: `τ_H` variable y `Q_T` → factor `S_V=sat_{[0,2]}(√Q)`.
  - Ecs. `eq:turb-hp`, `eq:turb-q`, `eq:turb-raw`.
  - Condición de saturación (~654).
  - Párrafo de cortes (~677).
  - Recalcular los ejemplos de las líneas ~741–783 y la tabla de viento constante con el nuevo τ y factor, indicando la velocidad supuesta.
  - Tabla de procedencia (~856) y párrafo de `--reference-knots` (~885).
- Suelo:
  - `eq:ground-envelope` **no cambia** (baches). Solo la subsección "Actualización: textura suave de rodaje sobre pista" (~986–996): la amplitud de la textura pasa a `\sqrt{E}`, ruido de banda estrecha 7/11.3 Hz con RMS de senoide, recorte `\pm\sqrt{E}`, techo teórico de textura recalculado; marcar roll dominante y textura siempre presente como **requisito del usuario**.
  - Párrafo de "Paso 5" que menciona `\sat_{[-E,E]}` (~1114): ahora `\sat_{[-\sqrt E,\sqrt E]}`.
- Tren:
  - Ec. `eq:gear-env`, párrafos de umbral (~1285–1288), tabla de picos (~1361), ejemplo a 125 kt.
  - Sección de onda triangular (~1307–1316) → portadora de banda estrecha, conservando la explicación de la triangular como modo con el flag en 0.
  - Las tablas de arranque por fase (~1366–1371) pasan a "solo con flag 0".
- Pérdida:
  - Ec. `eq:stall-onset` y párrafo (~1486–1488), ejemplos y tabla (~1581–1611), saturación `α_sat` (~1532).
  - Triangular 13 Hz (~1551–1559) y "Diferencias con TelemFFB" (agregar: el inicio ya no es el de TelemFFB).
- Spoiler: ec. `eq:spoiler-env`, párrafos de umbral (~1736–1743), ejemplo y tabla (~1791–1809), portadora (~1756–1764).
- Damper: agregar el factor `s_B` a la ec. `eq:damper_aislado` y a `eq:movimiento_conjunto`, con un párrafo y la tabla de `s_B` vs Q; en "Presión y amortiguación" (~2203–2206), indicar que ahora el damper escala con √k.
- Fricción:
  - Las subsecciones de la fórmula (~2305–2349) pasan a describir el modelo de Hayward–Armstrong como predeterminado (`YOKE_FRICTION_MODEL=1`), conservando la ley racional como modo 0.
  - Ejemplos (~2351–2389) recalculados para el modelo 1 (en función de desplazamiento, no de velocidad).
  - "Lo que sucede cuando el mando queda quieto" (~2288): ahora sí hay adherencia hasta `Fc`.
  - Tabla de parámetros (~2460): agregar `YOKE_FRICTION_PRESLIDE`.
- Abstract (~22): agregar una oración que remita a la sección de actualización.

Compilar el `.tex` (pdflatex dos pasadas). Anotar en HANDOFF si compila.

### 6.2 `LEEME md/CALCULO_PICO_SC8.md`
- Línea 15 (rampa de baches): no cambia.
- Línea 50: la textura usa amplitud `sqrt(E)` (ISO 8608) y ruido de banda estrecha 7/11.3 Hz (flag `SIMCONNECT_GROUND_NARROWBAND_TEXTURE`); la suma se limita a `±sqrt(E)`.

### 6.2b `LEEME md/CAMBIOS_REALISMO.md`
Línea 11 (`k=rest+(1-rest)*p`): agregar a continuación una línea fechada `Actualizacion 2026-10: k=rest+0.85*p (SIMCONNECT_CENTERING_AERO_STIFFNESS); ver tesis, seccion de realismo.` No tocar el resto. Si el archivo sigue sin seguimiento en git por el traslado pendiente (precondición de la sección 0), informarlo en HANDOFF en lugar de editarlo.

### 6.3 `LEEME md/MODELO_TURBULENCIA.md`
Agregar una nota fechada al principio (mismo estilo que la de 30/09/2026): τ_H variable `29.5 m / V_e` en [0.25, 1.5] s y ponderación `√Q` (0..2) en lugar de `min(Q,1)`, con remisión a la sección de actualización del `.tex`. No reescribir el resto.

### 6.4 No afectados
`AJUSTE_FUERZAS_Y_MOVIMIENTO.md`, `EFECTOS_SIMCONNECT.md`, `CENTRADO_Y_RODAJE.md`, `LEEME_AMORTIGUACION.md` y `LEEME_ETAPAS.md` ya están marcados como históricos o describen versiones previas: no se tocan. `explicacion_efectos_telemffb.tex` describe TelemFFB: no se toca.

## 7. Referencias teóricas

1. Etkin, B.; Reid, L. D. *Dynamics of Flight: Stability and Control*, 3.ª ed., Wiley, 1996. Momento de bisagra, mando libre, fuerza en el mando ∝ q.
2. Nelson, R. C. *Flight Stability and Automatic Control*, 2.ª ed., McGraw-Hill, 1998. Coeficientes de bisagra y fuerzas en el mando.
3. Pratt, K. G.; Walker, W. G. *A Revised Gust-Load Formula and a Re-evaluation of V-G Data Taken on Civil Transport Airplanes from 1933 to 1950*, NACA Report 1206, 1954. Carga de ráfaga lineal en U·V; factor de alivio por parámetro de masa.
4. Hoblit, F. M. *Gust Loads on Aircraft: Concepts and Applications*, AIAA Education Series, 1988. Respuesta en plunge y cambio de α por ráfaga.
5. 14 CFR Part 23: §23.207 (aviso de pérdida; texto anterior a la enmienda 23-64 de 2017, márgenes ≥5 kt y ≤max(10 kt, 15 % Vs) según la redacción de 1993); §23.2150 (vigente: aviso claro con margen suficiente, sin cifras); §23.143(c) (fuerzas máximas de mando, edición 2013).
6. Mabey, D. G. *Some Aspects of Aircraft Dynamic Loads due to Flow Separation*, AGARD Report R-750, 1988 (ver también AGARD CP-483, *Aircraft Dynamic Loads due to Flow Separation*, 1990). El buffet como excitación aleatoria por flujo separado.
7. Newland, D. E. *An Introduction to Random Vibrations, Spectral and Wavelet Analysis*, 3.ª ed., Longman, 1993. Respuesta de banda estrecha de un resonador a ruido blanco.
8. Box, G. E. P.; Jenkins, G. M.; Reinsel, G. C. *Time Series Analysis: Forecasting and Control*. Varianza estacionaria de un proceso AR(2).
9. ISO 8608:2016, *Mechanical vibration — Road surface profiles — Reporting of measured data*. PSD de perfil `G_d(n)=G_d(n0)(n/n0)^-2`.
10. Ogata, K. *Modern Control Engineering*, Pearson. Razón de amortiguamiento de un sistema masa-resorte-amortiguador.
11. McCormick, B. W. *Aerodynamics, Aeronautics, and Flight Mechanics*, 2.ª ed., Wiley, 1995. Pendiente de sustentación del ala finita (Helmbold).
12. Hayward, V.; Armstrong, B. "A new computational model of friction applied to haptic rendering", *Experimental Robotics VI*, LNCIS 250, Springer, 2000, pp. 403–412.
13. Armstrong-Hélouvry, B.; Dupont, P.; Canudas de Wit, C. "A survey of models, analysis tools and compensation methods for the control of machines with friction", *Automatica* 30(7), 1994, pp. 1083–1138.
14. Karnopp, D. "Computer simulation of stick-slip friction in mechanical dynamic systems", *J. Dyn. Sys., Meas., Control* 107(1), 1985, pp. 100–103.
15. Datos C182RG (W 3100 lbf, S 174 ft², envergadura 36 ft): POH del Cessna 182RG del usuario; confirmar contra el POH local.
