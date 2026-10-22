# DISEÑO: fuente de posición 3 = encoders magnéticos AS5600 (I2C) y renombre a YOKE_POSITION_SOURCE

Rama: `mejora/encoder-as5600` (desde `master`). Autor del diseño: Claude, 2026-10-22.
Decisiones del usuario (2026-10-22): bus "Roll en I2C1 propio, Pitch en I2C0 compartido con el LCD"; tesis: actualizar nombre de la bandera y describir el modo 3 (la edita Claude al aprobar la revisión).

## 1. Objetivo

1. Renombrar la bandera `YOKE_USE_SIMULATION` a `YOKE_POSITION_SOURCE` (mismos valores 0, 1, 2) en todo el código, scripts y documentación vigente.
2. Agregar el valor 3: posición de roll y pitch leída de dos sensores magnéticos absolutos AS5600 (módulo I2C) montados después de la caja reductora.
3. Comportamiento seguro: si un sensor falla (I2C, imán ausente) la fuerza de ambos motores es cero hasta que se recupere; si se pierde la cuenta de vueltas, la fuerza queda en cero hasta reiniciar.
4. Los modos 0, 1 y 2 deben comportarse exactamente igual que hoy.

## 2. Cómo funciona el AS5600 (fundamento)

Fuentes: hoja de datos ams AS5600 (tabla de filtros SF y nota de aplicación de entrehierro, vía [alldatasheet](https://www.alldatasheet.net/html-pdf/1452014/OSRAM/AS5600/3593/20/AS5600.html)); mapa de registros y bits de CONF/STATUS en los drivers de [ESPHome](https://api-docs.esphome.io/as5600_8cpp_source) y [TinyGo](https://pkg.go.dev/tinygo.org/x/drivers@v0.29.0/as560x), coincidentes entre sí.

- Sensor Hall que mide el ángulo del campo de un imán diametral que gira sobre el chip. Salida absoluta de 12 bits: 4096 cuentas por vuelta (0,088°/cuenta), **dentro de una sola vuelta**. No cuenta vueltas: más de 360° hay que acumularlos en el firmware.
- Dirección I2C fija `0x36` (no configurable): un sensor por bus. Admite I2C hasta 1 MHz.
- Registros usados: `STATUS 0x0B` (bit5 MD imán detectado, bit4 ML imán débil, bit3 MH imán fuerte), `RAW ANGLE 0x0C/0x0D` (12 bits, nibble alto en 0x0C), `AGC 0x1A` (0..128 a 3,3 V; óptimo cerca del centro del rango), `CONF 0x07/0x08`.
- Como 0x0B, 0x0C y 0x0D son consecutivos, **una sola lectura de 3 bytes desde 0x0B** trae estado + ángulo crudo en cada ciclo.
- Se usa `RAW ANGLE` (sin escalado ZPOS/MPOS ni histéresis interna) para no depender de la programación OTP del módulo. Nunca se usa el comando BURN.
- `CONF` (16 bits, volátil): WD bit13, FTH bits12:10, SF bits9:8, PWMF 7:6, OUTS 5:4, HYST 3:2, PM 1:0. Filtro lento SF (retardo / ruido RMS): 16x 2,2 ms / 0,015°; **8x 1,1 ms / 0,021°**; 4x 0,55 ms / 0,030°; 2x 0,286 ms / 0,043°. FTH≠0 activa el filtro rápido para saltos grandes.
- Elección: `CONF = 0x0500` → SF=8x (1,1 ms, menor que el ciclo de 2 ms), FTH=001 (6 LSB: respuesta rápida en movimientos bruscos, poco ruido en reposo), WD=0 (sin pasar a bajo consumo en reposo, que agregaría retardo), PM=NOM, HYST=0, OUTS/PWMF por defecto. Se escribe en cada arranque (se pierde al apagar) y se verifica leyendo de vuelta.
- Entrehierro típico 0,5–3 mm según el imán; la hoja de datos indica ajustar la distancia hasta que AGC quede en el centro de su rango. Por eso la telemetría informa AGC y los bits ML/MH.

## 3. Hardware y asignación de pines (Pico 2, números GPIO)

| Señal | Roll | Pitch |
|---|---|---|
| Bus | I2C1 (`Wire1`), propio | I2C0 (`Wire`), compartido con el LCD (0x27/0x3F, sin conflicto con 0x36) |
| SDA | GP14 (pin físico 19) | GP4 (ya cableado al LCD) |
| SCL | GP15 (pin físico 20) | GP5 (ya cableado al LCD) |
| Reloj | `YOKE_I2C1_HZ` = 400 kHz | `YOKE_I2C0_HZ` = 100 kHz (límite del PCF8574 del LCD) |
| Costo por ciclo (lectura 3 bytes) | ~0,15 ms | ~0,6 ms |

GP14/GP15 están libres (encoders GP0–3, LCD GP4/5, PWM/EN GP6–11, LED GP12/13, botones GP16–20, potes GP26/27). Módulos alimentados a 3,3 V (pin 3V3 de la Pico); pin DIR a GND; OUT y PGO/GPO sin conectar. **El bus I2C0 debe quedar a 3,3 V**: el AS5600 a 3,3 V no tolera 5 V en SDA/SCL. Si el LCD/PCF8574 va a 5 V con pull-ups a 5 V, el AS5600 de pitch se conecta del lado de 3,3 V del adaptador de nivel que ya pide `LEEME_PICO2.md` (línea 85), junto a la Pico.

## 4. Cambios exactos

### 4.1 `mis_defines.h`

Reemplazar el bloque "Fuente UNICA de posicion…" (líneas de `YOKE_USE_SIMULATION`) por:

```cpp
// Fuente UNICA de posicion y velocidad para todos los efectos.
// 0 = encoders incrementales AB; 1 = SimConnect; 2 = potenciometros ADC0/ADC1;
// 3 = encoders magneticos absolutos AS5600 por I2C (solo Pico).
#ifdef YOKE_USE_SIMULATION
#error "YOKE_USE_SIMULATION se renombro a YOKE_POSITION_SOURCE (0..2 iguales; 3 = AS5600)"
#endif
#ifndef YOKE_POSITION_SOURCE
#define YOKE_POSITION_SOURCE 1
#endif
static_assert(YOKE_POSITION_SOURCE >= 0 && YOKE_POSITION_SOURCE <= 3,
              "YOKE_POSITION_SOURCE debe ser 0, 1, 2 o 3");
#if (YOKE_POSITION_SOURCE == 2 || YOKE_POSITION_SOURCE == 3) && defined(ARDUINO) && !defined(ARDUINO_ARCH_RP2040)
#error "Los modos 2 (potenciometros) y 3 (AS5600) requieren Raspberry Pi Pico/Pico 2 (Arduino-Pico)"
#endif
```

Agregar después del bloque de potenciómetros (antes de "PINES - ENCODERS"):

```cpp
// ============================================================
// ENCODERS MAGNETICOS AS5600 (YOKE_POSITION_SOURCE 3). Ver LEEME_AS5600.md.
// ============================================================
// Direccion I2C fija 0x36: un sensor por bus. Roll: I2C1 propio.
// Pitch: I2C0 compartido con el LCD (LCD_SDA_PIN/LCD_SCL_PIN).
#define AS5600_I2C_ADDRESS 0x36
#define ROLL_AS5600_SDA_PIN 14 // I2C1 SDA, pin fisico 19
#define ROLL_AS5600_SCL_PIN 15 // I2C1 SCL, pin fisico 20
#define YOKE_I2C1_HZ 400000UL  // bus propio del AS5600 de roll
#define YOKE_I2C0_HZ 100000UL  // LCD + AS5600 de pitch; PCF8574: 100 kHz maximo por hoja de datos
// Centro: -1 = la posicion al encender es el centro (como encoders incrementales).
// 0..4095 = valor a= medido con el yugo centrado (centro absoluto, sin centrar al
// encender); solo si el eje del sensor gira menos de media vuelta a cada lado.
#define ROLL_AS5600_CENTER_RAW  -1
#define PITCH_AS5600_CENTER_RAW -1
// Cuentas desde el centro a cada extremo (4096 = 1 vuelta del eje del sensor).
// Roll +-90 grados: 1024. Pitch +-150 mm con polea efectiva de 75 mm en el eje del
// sensor: 150/(pi*75)*4096 = 2608. ESTIMACIONES: reemplazar por cuentas medidas.
#define ROLL_AS5600_COUNTS_NEG  1024
#define ROLL_AS5600_COUNTS_POS  1024
#define PITCH_AS5600_COUNTS_NEG 2608
#define PITCH_AS5600_COUNTS_POS 2608
#define ROLL_AS5600_DIRECTION   1   // +1 o -1 (tambien se invierte con el pin DIR)
#define PITCH_AS5600_DIRECTION  1
#define YOKE_AS5600_HYSTERESIS_COUNTS 1 // banda movil contra el ruido de +-1 cuenta
#define YOKE_AS5600_FAULT_CYCLES 3      // lecturas malas seguidas para cortar fuerza (6 ms)
#define YOKE_AS5600_RECOVER_CYCLES 50   // lecturas buenas seguidas para volver (100 ms)
#define YOKE_AS5600_MAX_GAP_MS 40       // con centro al encender: hueco mas largo = bloqueo hasta reiniciar
#define YOKE_AS5600_START_WINDOW_MS 500 // con centro al encender: primera lectura despues de esto = bloqueo
#define YOKE_AS5600_OVERTRAVEL 1.25f    // |cuentas| > 1,25 * extremo = vuelta perdida (bloqueo)
#define YOKE_AS5600_REQUIRE_MD 1        // 1 = exigir bit MD (iman detectado); 0 si un modulo no lo informa
#define YOKE_AS5600_CONF 0x0500         // SF=8x (1,1 ms), FTH=6 LSB, WD=0, PM=NOM. Volatil, nunca BURN
```

`static_assert` a agregar junto a ese bloque (mensajes en castellano, sin tildes):
- `*_AS5600_CENTER_RAW` igual a -1 o en 0..4095.
- `*_AS5600_COUNTS_NEG/POS` en 1..32767; y si `CENTER_RAW >= 0`, ambos ≤ 1800 (el centro absoluto solo vale dentro de media vuelta, con margen para sobrerrecorrido mecánico).
- `*_AS5600_DIRECTION` ±1; `YOKE_AS5600_HYSTERESIS_COUNTS` 0..8; `FAULT_CYCLES` 1..50; `RECOVER_CYCLES` 1..500; `MAX_GAP_MS` 10..100; `START_WINDOW_MS` 100..2000; `OVERTRAVEL` 1,05..2; `REQUIRE_MD` 0 o 1.
- `YOKE_I2C0_HZ` en 100000..400000; `YOKE_I2C1_HZ` en 100000..1000000.
- Pines I2C1 válidos en RP2350: `ROLL_AS5600_SDA_PIN % 4 == 2 && <= 26` y `ROLL_AS5600_SCL_PIN % 4 == 3 && <= 27`.
- Con `YOKE_POSITION_SOURCE == 3` (dentro de `#if`): los pines del AS5600 de roll distintos de `ROLL_PWM_*`, `PITCH_PWM_*`, `*_EN_PIN`, `LCD_SDA_PIN`, `LCD_SCL_PIN`.

No se tocan `ROLL_ENCODER_MIN/MAX`, `*_POS_MIN/MAX` (siguen siendo del modo 0) ni constantes de seguridad.

### 4.2 Nuevo `As5600Position.h` (lógica pura, sin Arduino; se prueba en `tests/`)

```cpp
#pragma once
#include <stdint.h>
#include "mis_defines.h"
namespace YokeAs5600 {
constexpr uint8_t StatusMH=0x08, StatusML=0x10, StatusMD=0x20;
struct Frame { uint16_t raw; uint8_t status; };
// Lectura de 3 bytes desde 0x0B: STATUS, RAW ANGLE alto, RAW ANGLE bajo.
inline Frame decode(uint8_t status,uint8_t high,uint8_t low);   // raw=((high&0x0F)<<8)|low; status&0x38
inline int wrapDelta(int delta);                               // lleva a -2048..2047
class Axis {
public:
    Axis(int centerRaw,int countsNeg,int countsPos,int direction);
    void start(uint32_t nowMs);          // instante de begin(): abre la ventana de arranque
    void update(bool ok,uint16_t raw,uint32_t nowMs);
    void excuseGap();                    // el proximo hueco no bloquea (pausa por guardado en flash)
    bool healthy() const;   // inicializado, sin falla activa y sin bloqueo
    bool locked() const;    // vueltas perdidas (solo centro al encender): hasta reiniciar
    float position() const; // -1..1, ultima valida (0 antes de la primera)
    long counts() const;    // cuentas firmadas desde el centro, con signo e histeresis, sin recortar
    uint16_t raw() const;   // ultimo crudo valido
};
}
```

Comportamiento exacto de `Axis::update(ok, raw, nowMs)`:
1. `ok == false`: `failures` (saturado en 255) +1; `goods = 0`; si `failures >= YOKE_AS5600_FAULT_CYCLES` → `faulted = true`. Posición y cuentas sin cambio. Fin.
2. `ok == true` y no inicializado: si `centerRaw < 0` y `uint32_t(nowMs - startMs) > YOKE_AS5600_START_WINDOW_MS` → `locked = true` (el centro se tomaría con el yugo en cualquier posición) y fin. Si no: `center = centerRaw < 0 ? raw : centerRaw`; `accum = wrapDelta(raw - center)`; `held = accum`; `lastRaw = raw`; `lastGoodMs = nowMs`; `failures = 0`; `faulted = false`; inicializado. Calcular posición (paso 5). Fin.
3. `ok == true`, inicializado:
   - Centro absoluto (`centerRaw >= 0`): `accum = wrapDelta(raw - center)` siempre (se autocorrige; no acumula error).
   - Centro al encender (`centerRaw < 0`): si `uint32_t(nowMs - lastGoodMs) > YOKE_AS5600_MAX_GAP_MS` y no hay `excuseGap` pendiente → `locked = true` (haya o no lecturas malas: también cubre un lazo detenido). Si no, `accum += wrapDelta(raw - lastRaw)`; luego, si `|accum| > OVERTRAVEL * (accum<0 ? countsNeg : countsPos)` → `locked = true` (más allá de los topes mecánicos solo se llega con una vuelta mal contada). El pedido de `excuseGap` se consume en esta lectura.
   - Justificación de 40 ms: `wrapDelta` es correcto mientras el eje gire menos de 2048 cuentas entre lecturas buenas. 2048 cuentas en 40 ms = 51 cuentas/ms ≈ 4,5 vueltas/s del eje del sensor: pitch haría su recorrido completo estimado (5216 cuentas) en 0,1 s, más rápido que cualquier movimiento manual. Hueco normal = 2 ms.
   - `excuseGap` lo usa solo el guardado en flash del menú (motores en cero, el piloto está pulsando botones; hueco de hasta ~400 ms). Riesgo residual aceptado: si en ese lapso el eje gira más de media vuelta, la vuelta perdida solo la detecta el control de sobrerrecorrido. En centro absoluto no aplica.
4. Luego: `failures = 0`; `lastRaw = raw`; `lastGoodMs = nowMs`; si `faulted`: `goods` +1 y si `goods >= YOKE_AS5600_RECOVER_CYCLES` → `faulted = false`, `goods = 0`.
5. Histéresis (misma regla de banda móvil que `PotentiometerPosition`): si `accum > held + H` → `held = accum - H`; si `accum < held - H` → `held = accum + H`. `signed = direction * held`; `position = signed < 0 ? signed / countsNeg : signed / countsPos`, recortada a ±1. Si `locked`, posición y cuentas quedan congeladas.

`healthy() = inicializado && !faulted && !locked`. En centro absoluto nunca hay bloqueo (ventana de arranque, hueco y sobrerrecorrido aplican solo a centro al encender): la posición siempre se recalcula desde el valor absoluto.

### 4.3 Nuevo `YokeI2C.h` (bus I2C0 compartido; solo `ARDUINO_ARCH_RP2040`/Arduino)

Mover sin cambios de comportamiento el cuerpo de `LcdTelemetry::configureBus()` a `YokeI2C::configureBus0()`, reemplazando el literal `100000UL` por `YOKE_I2C0_HZ`; `LcdTelemetry::configureBus()` pasa a llamarla. En `finishTransmission()` solo reemplazar el literal por `YOKE_I2C0_HZ` (no mover nada más). Con los valores por defecto los modos 0–2 quedan idénticos. No agregar banderas de "ya iniciado": en arduino-pico 6.1.1 `setSDA/setSCL` con el mismo pin y `begin()` con el bus corriendo no hacen nada (Wire.cpp:57-80, 113-120), y hoy el LCD ya llama `configureBus()` en cada `State::Probe`.

Hecho verificado (arduino-pico 6.1.1, `libraries/Wire/src/Wire.cpp`): no existen `WIRE_HAS_TIMEOUT` ni `setWireTimeout` (esa rama del LCD es código muerto; no se toca). Con `setTimeout(ms, true)`, ante un tiempo límite la biblioteca reinicia el bus sola (`_handleTimeout`, Wire.cpp:364-404) y `endTransmission` devuelve 5 (Wire.cpp:429-430). Por eso el driver decide falla solo por los valores de retorno y no reinicia el bus por su cuenta.

### 4.4 Nuevo `AxisAs5600.h` (driver; solo se incluye con modo 3)

Interfaz análoga a `AxisPotentiometers`:

```cpp
class AxisAs5600 {
public:
    void begin();                 // configura buses, escribe y verifica CONF en ambos sensores
    void update(uint32_t nowMs);  // una lectura de 3 bytes por eje y Axis::update
    bool healthy() const;         // ambos ejes sanos
    uint16_t raw(uint8_t axis) const;
    float position(uint8_t axis) const;
    const char* statusText(uint8_t axis) const; // "BLQ","I2C","SIN","DEB","FUE","OK" en ese orden de prioridad
    uint8_t agc(uint8_t axis);    // lee 0x1A a demanda (1 byte); 0 si falla o el eje no esta sano
};
```

- `begin()`: `Wire1.setSDA(ROLL_AS5600_SDA_PIN); Wire1.setSCL(ROLL_AS5600_SCL_PIN); Wire1.begin(); Wire1.setClock(YOKE_I2C1_HZ);` `Wire1.setTimeout(1, true)`. Pitch: `YokeI2C::configureBus0()` (tiempo límite 2 ms, el del LCD). Llamar `start(millis())` en ambos ejes. Escribir CONF: `beginTransmission(0x36); write(0x07); write(YOKE_AS5600_CONF >> 8); write(YOKE_AS5600_CONF & 0xFF); endTransmission() == 0`, y leer 2 bytes desde 0x07 para verificar (`confOk[axis]`). Comparar con máscara `0x3FFF` (CONF tiene 14 bits).
- Lectura por eje: `beginTransmission(0x36); write(0x0B); endTransmission(false) == 0`, `requestFrom(0x36, 3) == 3`, leer 3 bytes, `decode`. `ok = transacción correcta && (!YOKE_AS5600_REQUIRE_MD || (status & StatusMD))`. Sin reinicio manual del bus (lo hace la biblioteca).
- Cada 1000 ms, alternando ejes (un eje por vez, solo si su última lectura fue correcta): leer CONF; si no coincide con `YOKE_AS5600_CONF & 0x3FFF` (sensor reiniciado por baja tensión o reconexión, o escritura inicial fallida), volver a escribirla. `confOk[axis]` refleja el último resultado.
- `agc(axis)` solo lo llama la telemetría (cada 100 ms, alternando ejes): una transacción de 1 byte como máximo cada 100 ms en el bus compartido.
- `excuseGap()`: reenvía a ambos ejes.
- Guardar el último estado del bus (`lastI2cOk[axis]`) y el último STATUS para `statusText`.

### 4.5 `MiYoke_PD.ino`

Renombrar todas las apariciones de la bandera. Además, solo con `YOKE_POSITION_SOURCE == 3`:
- Include `AxisAs5600.h` (rama `#elif == 3`) y objeto global `AxisAs5600 positionInput;`.
- En `loop()`, en el mismo lugar que la lectura de potes (antes de SerialRx): `markStage(YokeHealth::SensorRead); positionInput.update(now);`.
- En los dos bloques de guardado en flash del menú, después de `calibration.save()`: `positionInput.excuseGap();` (solo modo 3).
- Bloque de movimiento: `positions[]` desde `positionInput.position(axis)`. Si `!positionInput.healthy()`: `localMotion[axis].reset()`, `yokePosition[axis] = positionInput.position(axis)`, `yokeVelocity[axis] = 0` (sin pico de velocidad al recuperarse).
- Junto a la línea que anula comandos con el menú activo: `if(!positionInput.healthy()) motorCommands[0]=motorCommands[1]=0;`.
- Telemetría de movimiento (hoy `#if == 2` imprime `a=`): rama para modo 3 con formato `"%c a=%u s=%s g=%u x=%ld v=%ld f=%d p=%d%s"` (`s` = `statusText`, `g` = `agc(axis)`); cabe en el búfer de 64.
- Comentario de `positionInput.begin()`: agregar "AS5600: centro configurado o al encender".

### 4.6 Otros archivos

- `LoopSupervisor.h`: nueva etapa `SensorRead=16`, nombre `"SENSOR_I2C"` en `stageName` (el 13 no se reutiliza; las etapas se guardan en 7 bits).
- `CalibrationControls.h`: en `available()` agregar `(YOKE_POSITION_SOURCE!=3 || (p!=ROLL_AS5600_SDA_PIN && p!=ROLL_AS5600_SCL_PIN))` y renombrar la bandera.
- `SimEffects.h` (2 lugares) y tests (`simconnect_checks.cpp:223`, `sim_position_friction_checks.cpp:4`): solo renombre.
- `herramientas/compilacion/Build-Pico2.ps1`: renombrar `-DYOKE_USE_SIMULATION` por `-DYOKE_POSITION_SOURCE`; nuevo switch `-As5600` → `-DYOKE_POSITION_SOURCE=3`, salida `build-pico2-as5600`, modo `as5600`; excluyente con `-Encoders`, `-Potentiometers` y `-PruebaFalla`.
- `herramientas/compilacion/probar.ps1`: switch `-As5600` (línea "Pico 2 (AS5600)"), incluido en `-Todo`, y comentario de uso.
- `tests/health_checks.h`: agregar `SensorRead` a la lista de etapas verificadas.
- `.gitignore`: ya cubre `build-pico2*/` (verificar, no editar si cubre).

## 5. Pruebas

Nuevo `tests/as5600_checks.h` con `as5600Checks()`, incluido en `simconnect_checks.cpp` y llamado desde `main`. Usar las macros (no números fijos) para umbrales. Casos:
1. `decode(0x27,0xFF,0xFF)` → raw 4095, status 0x20; nibble alto enmascarado.
2. `wrapDelta`: 0→0, 2047→2047, 2048→-2048, -2049→2047, 4095→-1, -4095→1.
3. Centro al encender (H = `YOKE_AS5600_HYSTERESIS_COUNTS`): primera 4090 → 0; luego 5 → `counts() == 11 - H`; luego 4000 → `counts() == -90 + H` (cruce de 0 en ambos sentidos; la banda móvil descuenta H en cada cambio de sentido de avance, igual que `PotentiometerPosition`).
4. Varias vueltas: pasos de +1000 → `counts() == accum - H` con accum > 4096; posición recortada a 1.
5. Centro absoluto, cada caso con un `Axis` nuevo (la primera lectura inicializa `held = accum` sin descontar H): center 3000, ±1000 → primera 3500 da 0,5; primera 2600 da -0,4; center 100, primera 4000 → -0,196. Luego, en el primer eje, 2600 → `(-400 + H) / 1000`.
6. `direction = -1` invierte el signo.
7. Histéresis: ±1 cuenta alrededor del valor no cambia `counts()` con H=1; avance de 2 mueve 1.
8. Fallas: `FAULT_CYCLES-1` malas → sano; `FAULT_CYCLES` → no sano y posición retenida; `RECOVER_CYCLES-1` buenas → no sano; una más → sano.
9. Centro al encender, falla corta (< MAX_GAP) con cruce 4000→50 durante la falla → cuentas continúan bien.
10. Centro al encender, falla > MAX_GAP → `locked()`, no sano aunque sigan 1000 lecturas buenas. Igual sin lecturas malas: dos lecturas buenas separadas por MAX_GAP+1 ms → bloqueo.
11. Centro absoluto, falla larga → sin bloqueo, resincroniza.
12. Antes de la primera lectura buena: no sano, posición 0.
13. Desborde de `millis()`: lastGoodMs = 0xFFFFFFF0, nowMs = 0x10 → hueco de 32 ms (no bloquea).
14. `excuseGap()` y luego lectura buena 400 ms después → sin bloqueo; el siguiente hueco largo sí bloquea.
15. Centro al encender: primera lectura buena después de START_WINDOW_MS desde `start` → bloqueo; antes → normal.
16. Sobrerrecorrido: cuentas que superan OVERTRAVEL × extremo → bloqueo (centro al encender); en centro absoluto no bloquea.

Compilaciones: `probar.ps1 -Todo -As5600` (suite C++ + lanzador, Pico configurado, encoders, potenciómetros, AS5600, puente) todo OK, sin advertencias nuevas. Informar tamaño de programa/RAM del build AS5600 frente al configurado.

Comprobación de renombre: `git grep -n YOKE_USE_SIMULATION -- . ':!LEEME md/DISENO_*' ':!tesis'` → solo las dos líneas `#ifdef`/`#error` de `mis_defines.h`.

Ensayo con hardware (lo hace el usuario cuando tenga los sensores; queda en LEEME_AS5600.md): s=OK y g cerca de 64 en ambos ejes; a= varía suave en todo el recorrido y coincide en sentido y magnitud con el ángulo girado (verifica la lectura de 3 bytes desde 0x0B; si no coincidiera, alternativa: leer STATUS y RAW ANGLE en dos transacciones); desconectar SDA de un sensor → fuerza cero y s=I2C; reconectar → vuelve en ~100 ms (con centro al encender queda BLQ hasta reiniciar, es lo esperado). **Criterio de aceptación en hardware: `loopWorstUs` y los ciclos lentos del registro sin aumento respecto del modo 2** (presupuesto estimado: pitch ~0,6 ms + roll ~0,15 ms + envío al LCD ~0,3–0,5 ms en el mismo bus). Si no se cumple: evaluar `writeReadAsync` (Wire.cpp:511) para pitch o no servir el LCD en el mismo ciclo que la lectura; no subir I2C0 a 400 kHz (el PCF8574 es de 100 kHz).

## 6. Criterios de aceptación

- Seguridad: cualquier falla, bloqueo o falta de primera lectura deja `motorCommands` en cero en ambos ejes.
- Modos 0, 1 y 2 compilan y la suite pasa sin cambios de comportamiento (mismos tests verdes).
- Modo 3 compila para Pico 2; con `YOKE_USE_SIMULATION` definido falla con el mensaje de renombre.
- Lógica de `As5600Position.h` cubierta por los 16 casos.
- Ninguna constante de seguridad modificada (`MOTOR_PWM_HARD_LIMIT`, `SIMCONNECT_EXTRA_LIMIT`, límites PWM).

Revisión del diseño: subagente Embedded Firmware Engineer (2026-10-22), hallazgos verificados contra arduino-pico 6.1.1 e incorporados (timeouts, bloqueo por hueco, ventana de arranque, sobrerrecorrido, tensión del bus, relectura de CONF).

## 7. Qué NO se toca

Menú de calibración (sigue "REQUIERE MODO 2" para potes; calibrar AS5600 desde el menú queda como mejora futura), LCD (no muestra fallas del sensor; solo la telemetría serie), puente SimConnect, efectos y ganancias, constantes de seguridad, documentos de diseño de otras tareas, `tesis/`.

## 8. Documentación afectada

- Nuevo `LEEME md/LEEME_AS5600.md`: qué es, cableado (tabla de la sección 3, 3,3 V, DIR a GND, OUT y PGO libres, cables cortos y trenzados lejos de los cables del motor), montaje del imán (sección 9), configuración en `mis_defines.h` (centro al encender vs. centro absoluto, cómo medir cuentas a cada extremo con `a=`), lectura de la telemetría (`a`, `s`, `g`), comportamiento ante fallas y el ensayo con hardware de la sección 5.
- `LEEME md/AJUSTE_FUERZAS_Y_MOVIMIENTO.md`, `LEEME_POTENCIOMETROS.md`, `LEEME_SIMCONNECT.md` (y el uso de `-As5600`), `MENU_CALIBRACION.md` (línea 44: "Los modos 0, 1 y 3…"), `MOBIFLIGHT_PICO_Y_UNO.md`, `LEEME_PICO2.md`, `LEEME_ENCODERS.md`, `CENTRADO_Y_RODAJE.md`: renombre de la bandera y, donde se listan los valores, agregar "3=AS5600".
- `LEEME md/INVENTARIO_CODIGO.md`: renombre y filas nuevas para `As5600Position.h`, `AxisAs5600.h`, `YokeI2C.h`.
- `AGENTS.md`, `CLAUDE.md`, `.claude/skills/verificacion-antes-de-terminar/SKILL.md`: agregar `-As5600` donde se listan los switches de `probar.ps1`.
- `tesis/capitulos/03_centrado.tex`, `09_damper.tex`, `10_friccion.tex` y `tesis/fundamento_efectos/` (main.tex y generar_documento.py): los actualiza Claude al aprobar la revisión; Codex no los toca.

## 9. Montaje del imán (para LEEME_AS5600.md)

- Imán diametral (polos N y S a los lados, no en las caras), típicamente 6 mm × 2,5–3 mm de neodimio (el módulo lo trae), pegado coaxial en la punta del eje de salida de la caja reductora.
- Entrehierro imán–chip de 0,5 a 3 mm (típico 1–2 mm); ajustarlo hasta que `g=` quede cerca de 64 (centro de 0..128 a 3,3 V) con `s=OK` en todo el recorrido. `s=DEB` = muy lejos; `s=FUE` = muy cerca.
- Centro del imán alineado con el centro del chip (ejes coincidentes, desalineación lateral de unas décimas de milímetro como máximo) y caras paralelas. La distancia debe mantenerse constante al girar: sin juego axial ni excentricidad.
- Soporte y eje cerca del imán de material no magnético (aluminio, plástico, bronce); evitar acero a pocos milímetros del imán. Alejar el sensor del motor 775 y de los cables de potencia.
- Ideal: el eje del sensor gira menos de media vuelta a cada lado del centro en todo el recorrido; así se puede usar centro absoluto (`*_AS5600_CENTER_RAW`) y no hace falta centrar el yugo al encender. Si gira más (pitch estimado: ±229°), usar centro al encender (-1).
