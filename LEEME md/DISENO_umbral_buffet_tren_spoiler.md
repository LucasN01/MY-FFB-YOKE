# Diseño: umbral de inicio para el buffet de tren y de spoiler

- Rama: `mejora/realismo-efectos` (continúa V1.4; todavía sin integrar a `master`).
- Diseño: Claude (2026-10-11). Implementa: Codex. Revisa: Claude.
- Este documento no modifica código.

## 1. Problema (con evidencia del usuario)

En V1.4 el buffet de tren es `E_g = d·min[(V/150)², 1]` (`BuffetModel.h`), sin umbral. Validación en hardware del usuario (2026-10-11): todos los efectos funcionan bien, pero el buffet de tren **se siente a baja velocidad**. Por ejemplo, a 60 kt recién despegado, con el tren todavía abajo, resulta incómodo e irreal.

Con tren abajo y ganancias de fábrica: 60 kt → 16 % (6/3 PWM pico pitch/roll); 80 kt → 28 % (10/5).

El buffet de spoiler, `min[(V/140)², 1]`, tiene la misma ley sin umbral. El C182RG no tiene spoilers, por eso no se observó, pero el problema teórico es el mismo.

## 2. Fundamento y decisión

- **La forma ∝ q se mantiene.** La fuerza del flujo separado es `q·S·C_p'` (Mabey, AGARD R-750, 1988), así que por encima del umbral la intensidad crece con IAS² (IAS≈EAS).
- **El umbral es un umbral de percepción**, que la física de V1.4 no fijaba (`q·S·C_p'` describe cuánto crece, no desde cuándo se nota). En el avión, a baja velocidad las fluctuaciones son pequeñas frente al ruido de la estructura y la transmisión; en el yugo, en cambio, cualquier comando mayor que ~1 PWM se percibe como vibración de 10–15 Hz. **Hipótesis validada en hardware por el usuario:** sin umbral, el tren se siente a 60 kt y no corresponde.
- **Ley elegida:** cuadrática con umbral, continua, que vale 0 en el inicio y 1 en la intensidad plena:

  `R(V) = sat_[0,1]( (V² − V_on²) / (V_full² − V_on²) )`

  Por encima de `V_on` crece en proporción a la presión dinámica excedente (`q − q_on`).
- **Tren:** `V_on = 100 kt`, elegido por el usuario. Es la calibración que tenía antes de V1.4 y que le resultaba cómoda; ahora queda validada como umbral de percepción. `V_full = 150 kt` (sin cambio). Ninguno de los dos es Vle/Vlo del avión.
- **Spoiler:** `V_on = 80 kt`, `V_full = 140 kt`. Fuente verificable: TelemFFB usa exactamente esos umbrales para el buffet de spoiler (`VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:31-32`, `spoiler_spd_thresh_low = 80*kt2ms`, `spoiler_spd_thresh_hi = 140*kt2ms`). Era también la ley de MiYoke antes de V1.4. Se conserva la forma cuadrática por encima del inicio.

Valores (`d` o `s̄` = 1):

| IAS (kt) | Tren V1.4 | Tren nuevo | Spoiler V1.4 | Spoiler nuevo |
|---|---|---|---|---|
| 60 | 0.1600 | 0 | 0.1837 | 0 |
| 80 | 0.2844 | 0 | 0.3265 | 0 |
| 95 | 0.4011 | 0 | 0.4605 | 0.198864 |
| 100 | 0.4444 | 0 | 0.5102 | 0.272727 |
| 110 | 0.5378 | 0.168 | 0.6173 | 0.431818 |
| 125 | 0.6944 | 0.45 | 0.7972 | 0.698864 |
| 140 | 0.8711 | 0.768 | 1 | 1 |
| 150 | 1 | 1 | 1 | 1 |

PWM pico de tren con ganancias de fábrica (0.16·0.9·255 = 36.72 pitch; la mitad en roll): 110 kt → 6/3, 125 kt → 17/8, 140 kt → 28/14, 150 kt → 37/18.

## 3. Especificación

### 3.1 `BuffetModel.h` (único archivo de código)

Reemplazar la función `quadratic(double v, double full)` por:

```cpp
// Separated-flow/wake load ~ q*S*Cp' ~ IAS^2 (IAS ~ EAS at low altitude). Above a
// perceptual onset it grows with the excess dynamic pressure: 0 at `onset`, 1 at `full` kt.
inline double quadraticOnset(double v, double onset, double full) {
    const double x=(v*v-onset*onset)/(full*full-onset*onset);
    return v<=onset ? 0 : (x>=1 ? 1 : x);
}
```

y en `calculate`:

```cpp
    // Onset/full IAS: tren 100/150 kt (user-validated perceptual onset), spoiler 80/140 kt
    // (TelemFFB thresholds). Not aircraft Vle; physics fixes the q-proportional growth.
    if (t.retractable==1 && range(t.gear,0,1))
        result.gear=t.gear*quadraticOnset(t.ias,100,150);
    if (range(t.spoilerLeft,0,1) && range(t.spoilerRight,0,1))
        result.spoiler=(t.spoilerLeft+t.spoilerRight)*0.5*quadraticOnset(t.ias,80,140);
```

El comentario anterior de la línea "Full-intensity IAS 150/140 kt..." se reemplaza por el de arriba. No se toca nada más (pérdida, validaciones, `ramp`).

### 3.2 Pruebas

- `tests/simconnect_checks.cpp:506-509` (comentario y casos de tren):
  - Comentario: `// Gear load grows with excess IAS^2 above 100 kt, full at 150 kt.`
  - `100 → ==0`; agregar `110 → near(.168f)`; `125 → near(.45f)`; `150 → ==1`.
- Agregar casos de spoiler, después de la línea 509, empezando con `input.spoilerLeft=input.spoilerRight=1;` (en ese punto `spoilerRight` es NaN y `spoilerLeft` vale 0): `60 → ==0`, `80 → ==0`, `110 → near(float(YokeBuffet::calculate(input,true).spoiler),5700.0f/13200)`, `140 → ==1`.
- `tests/raw_checks.h:103` (IAS 125, tren 1, spoilers 0.5): `air.buffet[1]` → `near(.45f)`; `air.buffet[2]` → `near(.5f*9225.0f/13200)` (=0.349432).
- Los casos de `simconnect_checks.cpp:463-481` se evalúan a 150 kt (o con IAS 0): no cambian. Si algún otro caso falla, detenerse e informarlo en HANDOFF.
- Suite `tests/simconnect_checks.vcxproj` (Release Win32): 0 fallas.

### 3.3 Criterios de aceptación

1. `git diff` de código: solo `BuffetModel.h`. Ningún otro archivo de código.
2. Suite con 0 fallas; informar el total de comprobaciones (V1.4: 140 908).
3. `Build-Pico2.ps1` sin errores ni advertencias nuevas.
4. Versión creada con el mensaje de §5.

## 4. Documentación afectada

Solo las partes indicadas, en `tesis/explicacion_efectos_proyecto_ffb_yoke.tex`. Después copiar el archivo sobre `tesis/main.tex` y verificar con `diff`. No leer el `.tex` completo: ubicar cada parte con `grep -n` de los textos citados.

- **`sec:realismo`:**
  - Fila "Tren/spoiler" de la tabla "Cambios y fundamentos" (`(V/150)^2`): columna "Ahora" → `$d\,\sat[(V^2-100^2)/(150^2-100^2)]$; $\bar s\,\sat[(V^2-80^2)/(140^2-80^2)]$`; columna "Fundamento" → `Carga $qSC_p'$ sobre un umbral de percepción`; referencia → `Mabey; TelemFFB (spoiler)`.
  - Subsección "Tren y spoiler" (`IAS$^2$ si IAS`): agregar el umbral, la ley con `V_on`, y que el inicio es un umbral de percepción validado en hardware por el usuario (tren 100 kt) o tomado de TelemFFB (spoiler 80 kt).
  - Ítem de hipótesis "Tren abajo a 80 kt: 28.4\%": reemplazar por "Tren y spoiler sin efecto por debajo de 100/80 kt; umbral validado por el usuario en hardware (2026-10-11)".
- **Sección BUFFET POR TREN DE ATERRIZAJE:** la ecuación `\label{eq:gear-env}`, el párrafo siguiente ("No hay umbral de 100 kt..."), la cadena de cálculo (`(IAS/150)^2`), el párrafo de procedencia de umbrales, el ejemplo a 125 kt (`E_g=0.45`, recalcular `w_g`, `u_P`, `u_R` y PWM con la misma muestra `T=0.8`), la tabla de picos (`80&0.284444`; usar 100/110/125/140/150 kt de §2) y los párrafos de TelemFFB (`sólo se conserva 150 kt` y `escala IAS$^2$ sin umbral`).
- **Sección BUFFET POR SPOILER:** la ecuación `\label{eq:spoiler-env}`, el párrafo siguiente ("No hay umbral de 80 kt..."), la cadena (`(IAS/140)^2`), el ejemplo a 110 kt (`R_sp=0.431818`, `E_sp=0.259091`; recalcular el resto con la misma muestra), la tabla de picos y los párrafos de TelemFFB (`sin umbral de 80`). Ahora el inicio en 80 kt coincide con TelemFFB: decirlo.
- **Partes adicionales que también quedan incorrectas** (revisión del plan, 2026-10-11; números de línea orientativos, ubicar por texto):
  - Tren:
    - `~1501` ("Qué hace este efecto"): "sin umbral de 100 kt" → "nula hasta 100 kt; crece con el exceso de $q$ hasta 150 kt".
    - `~1549`: además de los valores, "media extensión a 125 kt: $E_g=0.225$" (antes 0.347222).
    - `~1557` (párrafo "El flujo separado aporta…"): agregar el umbral de 100 kt como umbral de percepción.
    - `~1712` (ajuste del tren): "ya no existe umbral de 100 kt" → "inicio 100 kt e intensidad plena 150 kt, ambos en `BuffetModel.h`; mantener 150>100".
  - Spoiler:
    - `~2049` ("Qué hace el efecto"): agregar "desde 80 kt".
    - `~2190`: además del ejemplo, los máximos pasan de "14/7" a **10/5** PWM pitch/roll (0.259091·36.72=9.51 → 10; 4.76 → 5).
    - `~2278` (ajuste del spoiler): "ya no existe umbral de 80 kt" → "inicio 80 kt e intensidad plena 140 kt en `BuffetModel.h`; mantener 140>80".
  - **Tabla de picos de tren** (`~1645-1652`): 80 kt → 0 (PWM 0/0); 110 kt → 0.168 (6/3); 120 kt → 0.352 (13/6); 125 kt → 0.45 (17/8); 140 kt → 0.768 (28/14); 150 y 180 kt → 1 (37/18, sin cambio). Formato PWM pitch/roll.
  - **Tabla de picos de spoiler:** 95 kt → 0.198864; 110 kt → 0.431818; 125 kt → 0.698864; la fila 110 kt con 0.8/0.4 → 0.259091. Recalcular el PWM de esas filas. Las filas de 140 kt (incluidas 1/0 y 0.05/0.05) y las superiores **no cambian**.
  - **Atribución:** donde se cite TelemFFB para el spoiler, aclarar que de TelemFFB se toman solo los umbrales 80/140 kt (TelemFFB interpola linealmente entre ellos, `utils.scale`). La forma cuadrática por encima del inicio es de MiYoke, fundada en la carga `qSC_p'` (Mabey).
- **Notas visibles** de cada subsección tocada (incluidas las de `~1506`, `~1560`, `~1715` y `~2054`): actualizar su texto con el formato vigente, `\textit{Actualización (realismo): <antes> $\rightarrow$ <ahora>; <por qué>; ver Sección~\ref{sec:realismo}.}`. Por ejemplo: `$d\min[(V/150)^2,1]$ $\rightarrow$ $d\,\sat[(V^2-100^2)/(150^2-100^2)]$; umbral de percepción validado en hardware (2026-10-11), crecimiento con $q$ (Mabey)`. Anteponer `% [V-realismo]` a cada párrafo, ecuación o tabla modificada.
- Compilar con pdflatex (dos pasadas) y verificar que no queden referencias indefinidas.

No afectados: `LEEME md/CALCULO_PICO_SC8.md` (no describe tren ni spoiler), `LEEME md/EFECTOS_SIMCONNECT.md` (histórico) y `LEEME md/DISENO_realismo_efectos.md` (diseño de otra tarea).

## 5. Versión

Commits por nombre:
1. `BuffetModel.h` y las pruebas: "Buffet tren y spoiler: umbral de inicio 100/80 kt con crecimiento por presion dinamica".
2. Los dos `.tex`: "Tesis: umbral de inicio de buffet de tren y spoiler".

Luego, con el árbol limpio (salvo `graphify-ino/`):

```
powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Buffet de tren y spoiler: umbral de inicio 100 y 80 kt con crecimiento por presion dinamica"
```
