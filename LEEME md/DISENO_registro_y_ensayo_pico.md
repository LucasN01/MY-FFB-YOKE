# Diseño: registro CSV de lo que calcula la Pico 2 y rutina de ensayo con el simulador

Rama: `mejora/registro-ensayo-pico` (desde `master`, V1.8). Autor: Claude, 2026-10-12.

## 1. Objetivo y decisión de quién usa el simulador

### 1.1 Problema
- Hasta SC7 la PC calculaba las fuerzas y `telemetry.csv` las registraba. Desde SC8 la Pico
  calcula todo y `telemetry.csv` solo tiene entradas crudas (`LEEME md/CALCULO_PICO_SC8.md:26`).
  [Código] `SimConnectBridge/main.cpp:59-67`.
- Lo único que vuelve de la Pico es `pico.log`, texto cada ~100 ms con suelo (`gr/gp`) y PWM
  (`R/P`) [INO] `MiYoke_PD.ino:325-338`, más la línea de movimiento `debugPrintMotion`
  (`MiYoke_PD.ino:79-125`). No incluye presión, ráfagas, buffets, centrado, comando por eje,
  estado de armado ni ganancias. No se puede verificar el cálculo de la Pico con esos datos.

### 1.2 Objetivo
1. Que el puente genere `pico.csv` (una fila por muestra procesada por la Pico, con todas las
   salidas de modelos, estado, comandos y PWM) y `ganancias.csv` (ganancias del menú).
2. Que exista una rutina de ensayo con marcas de fase (`marcas.csv`) y archivo automático de
   todos los registros en `ensayos\<fecha>_<nombre>\`, para que no se pisen.
3. Que una herramienta (`comparar_ensayo.exe`) recalcule en la PC, con el **mismo código** del
   firmware, lo que la Pico debió calcular con las mismas entradas, lo compare con `pico.csv`,
   verifique invariantes de seguridad y escriba `informe.txt` corto para que Claude lo revise.
4. Un **banco sin simulador**: el puente reproduce un `telemetry.csv` grabado hacia la Pico real
   (`--replay`). Lo puede ejecutar Codex sin intervención física.

### 1.3 ¿Quién usa el simulador? → **el usuario**, con la rutina de §6
Motivos (verificables en el proyecto):
- Pitch y roll entran al simulador por los potenciómetros del Arduino UNO (MobiFlight) y a la
  Pico por el mismo cursor (`LEEME md/LEEME_POTENCIOMETROS.md`, tabla "Cableado compartido").
  Moverlos es una acción física; Codex no puede hacerlo.
- El menú de calibración usa los 5 pulsadores físicos.
- Volar (rodaje, despegue, aproximación a pérdida) exige pilotaje en lazo cerrado con la imagen
  del simulador. Automatizarlo con SimConnect sería otro proyecto y no probaría los mandos reales.
- La sensación en las manos (cuando haya motores) solo la puede juzgar el usuario.

Reparto:
| Quién | Qué |
| --- | --- |
| Codex | Implementa (Partes 1, 2 y 3), compila y prueba en PC. Puede correr el banco en monitor; armado solo con la regla de §5.2. |
| Usuario | Carga el UF2 tras la Parte 1 y hace un vuelo corto de 2 min. Corre el banco armado (`BANCO.ps1 -Armar`). Vuela la rutina `ENSAYO.ps1` (§6). |
| Claude | Lee `informe.txt` y `resumen_fases.csv` de cada ensayo y decide si el proyecto calcula bien. |

### 1.4 Fuera de alcance
- No se cambia ningún modelo de fuerza, ganancia, filtro ni constante de seguridad (regla 7 de HANDOFF).
- No se toca el protocolo PC→Pico (trama SC8 de 113 bytes, `YOKE17SC8`).
- No se automatiza Prepar3D ni MobiFlight.

## 2. Firmware: línea `M` (modelos) y línea `G` (ganancias)

### 2.1 Nuevo `PicoLog.h` (raíz), puro, sin `Serial`
```cpp
namespace YokeLog {
struct Frame {
    uint16_t seq; uint32_t t, rev; uint8_t flags;
    float q, gust[2], buffet[3], ground[2], env, kc, pos[2], vel[2], cmd[2];
    int16_t pwm[2], peak[2]; uint16_t dropped;
};
// Escribe "\nM,<26 campos>\n". Devuelve longitud (sin el 0 final) o -1 si no cabe.
int formatModel(char* out, size_t size, const Frame& f);
// Escribe "\nG,<seq>,<enabledBits>,<12 valores raw 0..10000 en orden YokeGains::Id>\n".
int formatGains(char* out, size_t size, uint16_t seq, uint16_t enabledBits, const uint16_t* raw, uint8_t count);
enum Flag : uint8_t { Active=1, Armed=2, Paused=4, Menu=8, Dtr=16, Raw=32, NonFinite=64, Modeled=128 };
}
```
Campos de `M`, en este orden: `seq,t,rev,flags,q,gustR,gustP,bStall,bGear,bSpoiler,gndR,gndP,env,kc,posR,posP,velR,velP,cmdR,cmdP,pwmR,pwmP,pkR,pkP,drop,skip`
— 26 enteros después de la `M` (agregar `uint16_t skip` a `Frame`).
- Escala: `q, gust, buffet, gnd, env, kc, pos, cmd` → limitar a ±9,9999 **antes** de `lroundf(x*10000)`
  (±99999); `vel` → limitar a ±9999,999 antes de `lroundf(x*1000)` (±9 999 999); `pwm/peak` enteros; todo con
  `%ld`/`%u` (sin `%f`). `lroundf` fuera de rango no está definido: por eso se limita antes.
- Valores no finitos → se escriben como 0 y se activa el bit `NonFinite=64` de `flags`.
- `#define YOKE_LOG_M_MAX 192` en `PicoLog.h`; peor caso ≈ 190 B; buffer local `char line[YOKE_LOG_M_MAX]`. Debe ser < 256 = `CFG_TUD_CDC_TX_BUFSIZE`
  [Código] `Arduino15/.../6.1.1/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/tusb_config_rp2040.h:111`.

Significado:
| Campo | Origen en el firmware |
| --- | --- |
| `t`, `rev` | `simTelemetry.raw.timeMs`, `simEffects.modeledRevision()` (revisión con la que se calcularon los modelos; 0 si no hay modelo) |
| `flags` | `Active`=`simEffects.active()`, `Armed`=`simTelemetry.armed`, `Paused`=`simTelemetry.isPaused()` (accesor nuevo, §2.2b), `Menu`=`calibration.menu.active()` solo bajo `#if YOKE_CALIBRATION_ENABLED` (si no, 0), `Dtr`=`dtr`, `Raw`=`simTelemetry.rawAvailable`, `Modeled`=`modeledRevision()!=0` |
| `q, gust, buffet, gnd, env` | `simEffects.calculated()`: `pressure, gust[2], buffet[3], ground[2], groundEnvelope` |
| `kc` | `simEffects.centering()` (rigidez de centrado suavizada, `centerPressure`) |
| `pos, vel` | `yokePosition[2]`, `yokeVelocity[2]` |
| `cmd` | `motorCommands[2]` ya puestos a 0 si el menú está activo (`MiYoke_PD.ino:299`) |
| `pwm` | `rollMotor/pitchMotor.getLastAppliedPWM()` |
| `peak` | máximo `|pwm|` por eje desde la última línea `M` escrita; se reinicia al escribir |
| `drop` | acumulado de revisiones **modeladas** que no llegaron a tener línea `M` (USB lleno), satura en 65535 |
| `skip` | acumulado de revisiones recibidas que **nunca se modelaron** (llegaron 2 tramas en un mismo ciclo de 2 ms y se modeló solo la última), satura en 65535 |

### 2.2 Cambios en `SimEffects.h` (accesores y un contador, sin cambiar fuerzas)
```cpp
bool active() const { return enabled; }
float centering() const { return centerPressure; }
uint32_t modeledRevision() const { return modelReady ? modelRevision : 0; }
uint16_t skipped() const { return skipCount; }
```
- `skipCount` (`uint16_t`, saturado): en `update()`, justo antes de `models.update(rx.raw)` (`SimEffects.h:70`),
  si `modelReady && uint32_t(rx.revision-modelRevision)>1` sumar la diferencia − 1. No se reinicia en `reset()`.
- Motivo: en el primer ciclo armado y tras `dt>50` el `update` reinicia y retorna (`SimEffects.h:60-67`) con
  `enabled=true` y modelos en 0; sin `modeledRevision()` esa línea parecería un desvío de cálculo.
Junto a `calculated()` (`SimEffects.h:134`).

### 2.2b Cambio en `SimTelemetry.h` (accesor de solo lectura)
`Receiver::paused` es privado (`SimTelemetry.h:116`). Agregar en la parte pública de `Receiver`, junto a `fresh()`
(`SimTelemetry.h:44`):
```cpp
bool isPaused() const { return paused; }
```
No cambia visibilidad ni lógica de `paused`. Prueba: en `tests/picolog_checks.h`, tras una trama con flag de pausa
(bit1, sin armado) `rx.isPaused()==true`, y tras una trama armada `false`.

### 2.3 Cambios en `MiYoke_PD.ino` (bloque `ModelLog`, `MiYoke_PD.ino:322-338`)
Bajo `#if SIMCONNECT_ADDITIONS_ENABLED && YOKE_MODEL_LOG`:
1. Estado estático: `seq`, `lastLoggedRev`, `logSynced` (bool), `lastDtr`, `lastLineMs`, `peak[2]`, `drop`,
   `gainSeq`, `lastGainsMs`, `lastGains[YokeGains::Count]`, `lastBits`, `gainsSent`.
2. Cada ciclo: `peak[i]=max(peak[i],abs(pwm[i]))`.
3. Sincronización: en el flanco de subida de `dtr` poner `logSynced=false`. La primera línea con
   `logSynced==false` hace `lastLoggedRev=modeledRevision()` **sin** contar `drop` y pone `logSynced=true`.
4. Hay que escribir `M` si `dtr && simTelemetry.rawAvailable` y (`modeledRevision() != lastLoggedRev`
   **o** `now-lastLineMs >= 100` (latido: repite `rev`; el comparador lo usa para invariantes, no para A)).
   - Antes de formatear, comprobar `Serial.availableForWrite() >= YOKE_LOG_M_MAX` dentro de `UsbLock`; si no
     hay lugar, no formatear ni esperar: se reintenta el ciclo siguiente.
   - Cada ciclo, justo después de leer `r=modeledRevision()`: si `r==0` (no hay modelo: pausa, desarme, slew o
     reinicio por `dt>50`), poner `lastLoggedRev=0`. Así, al volver a modelar no se cuentan como pérdidas las
     revisiones recibidas sin modelo (corrección de la revisión de la Parte 1).
   - `drop`: con `r!=0 && lastLoggedRev!=0`, `d=uint32_t(r-lastLoggedRev)`; si `d>1`, sumar
     `max(0, (d-1) - (skipped()-lastSkip))` (saturado), donde `lastSkip` es `skipped()` al escribir la línea
     anterior: las revisiones que la Pico nunca modeló ya están en `skip` y no se cuentan dos veces. Nunca restar 1
     cuando `d==0`. El cálculo va en una función pura de `PicoLog.h`:
     `uint16_t nextDrop(uint16_t drop, uint32_t r, uint32_t lastLoggedRev, uint16_t skipNow, uint16_t lastSkip)`.
   - Al escribir: `seq++`, `lastLoggedRev=r` (si `r!=0`), `lastLineMs=now`, `peak=0`.
5. Una sola escritura `M` o `G` por ciclo (si se escribió `M`, `G` espera al siguiente).
   `G`: cuando `dtr && simTelemetry.rawAvailable` y (`!gainsSent` o algún `raw()`/`effects()` distinto de lo
   último enviado o `now-lastGainsMs >= 5000`). Los bits salen de `gainSettings.effects()` (ya existe,
   `GainSettings.h:43`).
6. **Se conserva** la línea `SC8 t=...` y `debugPrintMotion` sin cambios (documentadas en tesis y LEEME).
7. `YokeHealth::mark(YokeHealth::SerialTx)` antes de cada `Serial.write`, como el resto del archivo.

### 2.4 `mis_defines.h`
`#define YOKE_MODEL_LOG 1` con `#ifndef`, junto a `DEBUG_MOTION_TELEMETRY` (línea ~222). Con 0 el bloque
desaparece de la compilación.

### 2.5 Ancho de banda
Hasta ~50 líneas `M`/s (el puente envía cada ≥20 ms, `main.cpp:235`) × ~200 B = 10 kB/s, más `SC8` y
movimiento (~1 kB/s). El puente lee hasta 256 B por vuelta de ~5 ms (`main.cpp:243-249`) ≈ 50 kB/s: hay
margen ×4. **Cambiar el buffer de lectura del puente a 4096 B** para tener margen ante ráfagas.

## 3. Puente: `pico.csv`, `ganancias.csv` y modo `--replay`

### 3.1 Nuevo `SimConnectBridge/PicoCsv.h` (puro, `inline`, sin `Windows.h`)
- `struct LineSplitter { char line[256]; size_t n; bool overflow; template<class F> void feed(const char*, size_t, F onLine); }`
  igual que `BootLineScanner` (`SerialLink.h:51-67`) pero llama `onLine` con **toda** línea completa sin overflow.
- `bool modelRow(const char* line, char* out, size_t size)`: acepta solo `M,` + exactamente 26 enteros
  decimales con signo opcional; escribe la fila CSV **en unidades físicas** (÷10000, ÷1000 según §2.1, `%.4f`
  para ÷10000 y `%.3f` para vel; `seq,t,rev,flags,pwm,peak,drop,skip` enteros). Devuelve `false` si algo no
  cumple (línea corrupta → se descarta y se cuenta).
- `bool gainsRow(const char* line, char* out, size_t size)`: `G,` + 14 enteros (`seq, bits`, 12 valores);
  valores ÷10000 con `%.4f`.
- Cabeceras como constantes:
  - `pico.csv`: `host_time_ms,seq,sample_time_ms,revision,flags,pressure,gust_roll,gust_pitch,buffet_stall,buffet_gear,buffet_spoiler,ground_roll,ground_pitch,ground_envelope,center_stiffness,pos_roll,pos_pitch,vel_roll,vel_pitch,cmd_roll,cmd_pitch,pwm_roll,pwm_pitch,peak_pwm_roll,peak_pwm_pitch,dropped,skipped`
  - `ganancias.csv`: `host_time_ms,seq,enabled_bits,total,center_pitch,center_rest,gust_roll,gust_pitch,ground,stall,gear,spoiler,damper,friction,center_roll`
    (orden de `YokeGains::Id`, `GainSettings.h:7-8`).

### 3.2 Cambios en `SimConnectBridge/main.cpp`
1. Abrir `pico.csv` y `ganancias.csv` junto al EXE, con cabecera; agregarlos a la rotación
   (`main.cpp:209-210`: `pico.csv→pico.anterior.csv`, `ganancias.csv→ganancias.anterior.csv`).
2. Buffer de lectura de la Pico: 4096 B (`main.cpp:244`).
3. Un `YokeCsv::LineSplitter` alimentado con los mismos bytes que `bootScanner` (no tocar `bootScanner`):
   `M,` → `fprintf(picoCsv,"%llu,%s\n",GetTickCount64(),row)`; `G,` → ídem en `ganancias.csv`.
   `fflush` como máximo cada 200 ms (contador de tiempo), y siempre al cerrar.
   Al reconectar (`main.cpp:257`) reiniciar también el splitter.
   La lectura de puerto y reconexión (`main.cpp:243-259`) se extrae a `void PollPort(ULONGLONG now)` para usarla
   también en el modo replay sin duplicar código.
4. Contadores de líneas `M`/`G` recibidas y descartadas por formato; imprimirlos en la línea final `Fin: ...` como `lineas_pico=%u lineas_pico_malas=%u`.
5. **Modo `--replay <archivo.csv>`** (requiere `--port`; `--enable` opcional):
   - Nuevo `SimConnectBridge/Replay.h` puro: `struct ReplayRow { uint64_t host; uint32_t sample; float v[22]; bool paused,running,armed,sent,gusts,buffets; double ref,gain,longi; };`
     `bool parseHeader(const std::string&)` exige la cabecera exacta de `main.cpp:222`;
     `bool parseRow(const std::string&, ReplayRow&)` exige 34 campos.
     Debe aceptar los tokens no finitos que escribe MSVC con `%.9g` (`nan`, `-nan(ind)`, `inf`, `-inf`, sin
     distinguir mayúsculas) y convertirlos a NaN/±Inf: el C182 de P3D informa `spoiler_left/right` como
     `-nan(ind)` en casi todas las filas (vuelo corto del 12/10), y `YokeRaw::valid` lo admite a propósito
     (`RawTelemetry.h:48`). Cualquier otro texto no numérico rechaza la fila. El comparador (§4) usa el mismo
     lector.
   - En `main`: si hay `--replay`, cargar todas las filas con `sent=1`; si la cabecera no coincide o no hay
     filas, `Log` y `return 2`. No llamar `SimConnect_Open`.
   - Con `--replay` se rechazan (`return 2`) `--gusts`, `--gust-gain`, `--gust-longitudinal`, `--reference-knots`,
     `--no-buffets` y `--seconds`: esos valores vienen de cada fila.
   - `timeBeginPeriod(1)` al empezar y `timeEndPeriod(1)` al terminar (enlazar `winmm.lib`), para que `Sleep(1)`
     no dure 15 ms.
   - Bucle (sale también con `quit`, Ctrl+C): para cada fila, esperar hasta `inicio + (row.host - primera.host)`; fijar `opened=true`,
     `pauseKnown=true`, `paused`, `running`, `referenceKnots`, `gustGain`, `gustLongitudinal`, `gusts`,
     `buffets` desde la fila; `latest.value=v`, `latest.timeMs=row.sample` (marca **original**: la Pico usa
     diferencias de `timeMs`); `++samples`; `Send(enable && row.armed)`.
     Entre filas, llamar `PollPort(now)` y `Sleep(1)`.
   - Al terminar el archivo o con `quit`: `Send(false)` 3 veces separadas 50 ms, luego cierre normal; código de
     salida igual que `main.cpp:268`.
   - `Log` inicial: `REPLAY %s: %u filas enviadas, %.1f s | %s` (`HABILITADO`/`SOLO MONITOR`).
   - Uso actualizado en el mensaje de `main.cpp:199`.
   - El archivo de entrada **no puede** ser el `telemetry.csv` de la carpeta del EXE (se pisa al iniciar):
     si `GetFullPathName` coincide, `Log` y `return 2`.

## 4. Herramienta `comparar_ensayo.exe`

Proyecto nuevo `tests/comparar_ensayo.vcxproj` (copia de configuración de `tests/simconnect_checks.vcxproj`,
Release Win32, salida `tests/bin/`), fuente `tests/comparar_ensayo.cpp`. Uso:
`tests\bin\comparar_ensayo.exe <carpeta_ensayo>`. Lee `telemetry.csv`, `pico.csv`, `ganancias.csv`,
`pico.log` y, si existe, `marcas.csv`. Escribe `informe.txt` y `resumen_fases.csv` en la misma carpeta.
Reutiliza `Replay.h` y `PicoCsv.h` para leer, y `PicoForceModels.h`, `RawTelemetry.h` del firmware.

### 4.1 Fidelidad del cálculo (A)
- En el banco se compara contra el `telemetry.csv` **generado por el replay**, no contra `entrada.csv`.
- El comparador reproduce el receptor real: para cada fila de `telemetry.csv` con `sent=1`, en orden, arma la
  trama completa como `Send` (`main.cpp:71-81`: settings, `options`, payload vacío si `!YokeRaw::valid`, byte de
  modo 1 si `armed=1`, 2 si `paused=1 && armed=0`, si no 0; secuencia propia creciente; CRC) y la pasa por
  `YokeSC::Receiver::feed` con `now=uint32_t(host_time_ms)` (reutilizar el patrón de `tests/raw_checks.h:5-13`).
  - Si `rx.revision` aumentó, `rx.armed` y `rx.rawAvailable`: `models.update(rx.raw)` y guardar el estado esperado
    con clave `rx.raw.timeMs`.
  - `models.reset()` si `!rx.armed`, si Slew != 0, o si el hueco entre filas `sent=1` es >= 300 ms.
- Emparejar cada fila de `pico.csv` con `flags&Modeled` (deduplicada por `revision`: se usa la primera de cada
  revisión, los latidos repiten) con el estado esperado de igual `sample_time_ms`.
- Si `skipped` aumenta entre dos filas, la Pico salteó una muestra: el estado con memoria (ráfaga, suelo) puede
  diferir. Se excluyen de A las filas desde ese punto hasta la primera fila en que el comparador y la Pico tengan
  ráfaga y suelo en 0 a la vez, o 2 s, lo que ocurra primero; se informa la cantidad excluida.
- Tolerancias: `pressure, buffet_*, ground_envelope` 2e-4; `gust_*, ground_*` 2e-3. `center_stiffness` **no** se
  compara en A (depende de ganancias y del suavizado por ciclo): solo se informa en C.
- **Criterio A: >= 99 % de las filas emparejadas dentro de tolerancia en cada señal**, y >= 95 % de las
  revisiones esperadas emparejadas (las no emparejadas por `drop` cuentan aparte). Informar peor diferencia con
  su `sample_time_ms`.
- Si no hay ninguna fila armada: A = `SIN DATOS` (no `FALLO`), y `RESULTADO` = `SIN DATOS` salvo que B falle.
- Si falta `pico.csv` pero `pico.log` tiene líneas `M,`, el comparador las lee de `pico.log` con `modelRow`
  (sin `host_time_ms`; B6 queda `SIN DATOS`).

### 4.2 Invariantes (B) — cualquier falla = `FALLO`
| Id | Regla | Fuente |
| --- | --- | --- |
| B1 | `flags&Active==0` ⇒ `cmd_roll=cmd_pitch=0` | `SimEffects.h:137` |
| B2 | `flags&Menu` ⇒ `cmd=0` y `pwm=0` (no `peak`: arrastra el valor previo al menú) | `MiYoke_PD.ino:299` |
| B3 | `|pwm| ≤ PWM_MAX`, `|peak| ≤ PWM_MAX`, `|cmd| ≤ SIMCONNECT_EXTRA_LIMIT` | `MiYoke_PD.ino:306-307`, `SimEffects.h:179` |
| B4 | solo filas `Modeled`, emparejadas con la **primera** fila `sent=1` de igual `sample_time_ms`: `on_ground=0` ⇒ `ground_*=0` y `ground_envelope=0` | `PicoForceModels.h:47-51`, `GroundModel` |
| B5 | mismo emparejamiento que B4: `ground_speed_knots ≤ 0.5` ⇒ `ground_envelope=0` | `CALCULO_PICO_SC8.md` "Vibración de suelo" |
| B6 | tras la primera fila `sent=1` con `paused=1` (o `armed=0` después de `armed=1`), la primera fila `pico.csv` con `host_time_ms` ≥ +150 ms tiene `cmd=0` | `SimTelemetry.h:44` (watchdog 300 ms) |
| B7 | pérdida en la Pico: `dropped_final/(revisiones distintas en pico.csv + dropped_final) ≤ 2 %`; pérdida en el puente: `Σ max(0, seq[i]-seq[i-1]-1) / seq_final ≤ 2 %` | §2.3 |
| B8 | ningún `flags&64` (no finito) | §2.1 |
| B9 | `lineas_pico_malas / lineas_pico` (de la línea `Fin:` de `bridge.log`) ≤ 0,1 % | §3.2 |

### 4.3 Informativo (C) — no hace fallar
- Máximos de `us`, `lp`, `gap` leídos de `pico.log` (`SC8 ...`), y si `lp ≥ 2000`.
- Concordancia de signo del centrado: filas con `flags&Active`, `|pos|>0,2`, `|vel|<0,05`, buffets y suelo = 0:
  % con `signo(cmd) = -signo(pos)` por eje.
- Relación `pressure` vs `(ias/ref)²` (correlación).
- `skip` final y filas excluidas de A por ese motivo.
- Por fase (§4.5): duración, medias/máximos de `ias, ground_speed, on_ground, pressure, |gust|,
  buffets, |ground|, center_stiffness, |cmd|, |pwm|, peak` por eje y % `Active`. Va a `resumen_fases.csv`
  (una fila por fase) y como una línea por fase en `informe.txt`.

### 4.5 Fases detectadas automáticamente (no hace falta tocar el teclado en vuelo)
El comparador segmenta con la telemetría (filas `sent=1`), en este orden de prioridad:
`PAUSA` (`paused=1`), `SLEW` (`slew!=0`), `PARADO` (`on_ground=1 && gs<=0,5`), `RODAJE` (`on_ground=1 && gs>0,5`),
`PERDIDA` (en vuelo y `aoa_deg >= stall_alpha_deg - 3`), `TREN` (en vuelo y `gear_fraction>0,1`),
`VUELO` (resto en vuelo). Tramos de menos de 2 s se unen al anterior. Cada tramo es una fila de
`resumen_fases.csv` con número de orden y nombre (p. ej. `03 RODAJE`). Si existe `marcas.csv`, se agrega a cada
tramo el número de fase de la rutina de la última marca anterior.

### 4.4 Formato de `informe.txt` (corto, para leer sin gastar tokens)
```
ENSAYO <carpeta> | filas tel=<n> sent=<n> pico=<n> gan=<n>
A fidelidad: OK|FALLO emparejadas=<p>% q=<p>% gust=<p>% buffet=<p>% ground=<p>% env=<p>% peor=<señal>@<t>:<dif>
B1..B9: OK|FALLO <dato>   (una línea cada uno)
C us/lp/gap max=<a>/<b>/<c> | centrado signo R=<p>% P=<p>% | corr q-ias=<r>
F<n> <titulo>: <dur>s ias=<..> gs=<..> q=<..> gust=<..> buf=<..> gnd=<..> cmdR/P=<..>/<..> pwmR/P=<..>/<..> act=<p>%
RESULTADO: OK|FALLO
```
Código de salida 0 si A y B dan OK.

## 5. Scripts de ensayo

### 5.1 `ENSAYO.ps1` (lo usa el usuario; también `ENSAYO.cmd` de doble clic)
`powershell -ExecutionPolicy Bypass -File ENSAYO.ps1 [-Nombre vuelo1] [-Rutina completa|corta]`
1. Crea al inicio `ensayos\AAAAMMDD-HHMMSS_<Nombre>\` y escribe ahí `marcas.csv` y `rutina.txt` (la tabla de §6
   de la rutina elegida, para tenerla a la vista o impresa).
2. Si no corre el proceso `YokeSimConnectBridge` (nombre usado en `Iniciar-Simulador.ps1:135`), avisa: "Primero
   abrí el simulador con INICIAR SIMULADOR V3.cmd y esperá a estar en la pista" y espera Enter.
3. Hora con `GetTickCount64` por P/Invoke (`Add-Type`, kernel32): **mismo reloj** que `host_time_ms`.
4. Muestra la rutina y pide **un solo** Enter para marcar `inicio` (en tierra, antes de F1). Durante el vuelo
   **no** hay que volver a la consola: las fases se detectan solas (§4.5). Opcional: en pausa o en tierra, escribir
   el número de fase y Enter agrega una marca (`host_time_ms,fase,titulo`).
5. Al final: "Cerrá el simulador (o el puente) y presioná Enter". Espera a que termine el proceso del puente
   (máx. 120 s; si no termina, copia igual y lo avisa).
6. Copia `telemetry.csv, pico.csv, ganancias.csv, pico.log, bridge.log` a la carpeta del ensayo, ejecuta
   `tests\bin\comparar_ensayo.exe` sobre ella y muestra solo `informe.txt`.
7. Nunca borra nada fuera de su carpeta de ensayo.

### 5.2 `BANCO.ps1` (lo usa Codex; sin simulador)
`powershell -ExecutionPolicy Bypass -File BANCO.ps1 -Entrada <telemetry.csv> [-Puerto COMn] [-Armar] [-Confirmado]`

**Seguridad:** con armado la Pico genera PWM real. Codex no puede comprobar que la fuente de motores esté apagada
y el pull-down de EN sigue pendiente. Por eso `-Armar` sin `-Confirmado` pide escribir `APAGADA` en la consola (si
no, sale sin armar). `-Confirmado` solo lo usa Codex cuando HANDOFF contiene la confirmación del usuario "fuente
de motores apagada" con fecha del día. Sin `-Armar` corre en monitor: sirve para B; A queda `SIN DATOS`.
1. Falla con mensaje claro si corre Prepar3D o `YokeSimConnectBridge` (el COM es exclusivo).
2. Puerto: `-Puerto` o detección con `Iniciar-Simulador.ps1 -DetectarPlaca`, que imprime `Yoke detectado: COMn | ...`
   (`Iniciar-Simulador.ps1:141`); extraer con la regex `Yoke detectado: (COM\d+)`; si no aparece, salir pidiendo `-Puerto`.
3. Crea `ensayos\AAAAMMDD-HHMMSS_banco\`, copia la entrada como `entrada.csv`.
4. Ejecuta el puente: `--port COMn --replay <entrada.csv>` y agrega `--enable` solo si se cumplió la regla de seguridad.
5. Copia los registros como en §5.1 y ejecuta `comparar_ensayo.exe`; imprime solo `informe.txt`.

### 5.3 `.gitignore`
Agregar `ensayos/`.

## 6. Rutina de vuelo (la sigue el usuario con `ENSAYO.ps1`)

**Seguridad:** primera sesión con la **fuente de potencia de los motores apagada**: se valida el cálculo, no la
fuerza. Con motores, solo después de instalar el pull-down de EN (pendiente de V1.8).

Las fases se detectan solas: durante el vuelo no hace falta volver a la consola (evita alt-tab, que puede
pausar P3D). Seguir el orden de la tabla y dejar cada fase el tiempo indicado.

Preparación: abrir con `INICIAR SIMULADOR V3.cmd`, avión C182RG (o el habitual) en cabecera de pista, motor en
marcha, frenos puestos, sin turbulencia en el clima. Potes centrados.

| Fase | Duración | Qué hacer | Qué debería verse (lo verifica `comparar_ensayo`) |
| --- | --- | --- | --- |
| F1 Pausa | 15 s | Pausar el simulador (P). | `armed=0`, `cmd=0`, `pwm=0` (B6). |
| F2 Menú | 30 s | En pausa: entrar al menú con los pulsadores, recorrer 2-3 pantallas y **salir sin guardar** (guardar en flash produce un hueco del lazo y pérdidas legítimas). | `flags&Menu`, `cmd=0` (B2); `ganancias.csv` con los valores vigentes. |
| F3 Parado | 20 s | Quitar pausa, quieto con frenos. | `gs≤0,5` ⇒ `env=0`, suelo 0 (B5). |
| F4 Potes parado | 30 s | Roll a tope izq/der y pitch adelante/atrás, despacio, 2 veces. | Centrado con signo opuesto a la posición (C); `q≈0`, rigidez = reposo. |
| F5 Rodaje | 60 s | Soltar frenos, rodar 10→30 kt en recta. | `env` crece con la velocidad; `ground` ≠ 0; `peak` roll ≤ 40, pitch ≤ 4 por suelo. |
| F6 Despegue | ~60 s | Potencia máxima, rotar a ~60 kt, subir a 1000 ft. | Al despegar (`on_ground=0`) suelo 0 inmediato (B4). |
| F7 Crucero | 60 s | Nivelado, 100-120 kt, potes quietos. | `pressure≈(ias/ref)²`; buffets 0; `gust` 0 sin turbulencia. |
| F8 Potes en vuelo | 30 s | Mismo movimiento que F4. | Centrado mayor que en F4 (`center_stiffness` > F4). |
| F9 Tren y flaps | 60 s | Bajar tren y flaps por etapas dentro de los límites; luego subir. | `buffet_gear` > 0 con tren extendido y velocidad; 0 al retraer. |
| F10 Pérdida | 60 s | Potencia baja, levantar nariz despacio hasta la bocina/pérdida, recuperar. | `buffet_stall` crece al acercarse `aoa` a `stall_alpha`. |
| F11 Pausa en vuelo | 10 s | Pausar (P) y quitar. | `cmd=0` en ≤150 ms (B6), luego retoma. |
| F12 Slew | 10 s | Activar slew (Y) 5 s y desactivar. | Modelos reiniciados y `cmd=0` durante slew. |
| F13 Turbulencia (opcional) | 60 s | Clima con turbulencia moderada. | `gust` ≠ 0 (`--gusts` ya lo pasa el lanzador, `Iniciar-Simulador.ps1:255`). |
| F14 Aterrizaje | libre | Aterrizar y rodar hasta parar. | Suelo vuelve al tocar; 0 al detenerse. |

Rutina `corta`: F1, F3, F4, F5, F7, F8, F11.

## 7. Pruebas (escritorio)

Nuevo `tests/picolog_checks.h`, incluido desde `tests/simconnect_checks.cpp` (estilo `CHECK`).
Parte 1: casos 1-6 y 9. Parte 2: casos 7-8.
1. `formatModel` con todos los campos fuera de rango (q=1e9, vel=-1e9, pwm ±255, t=0xFFFFFFFF, seq/drop/skip=65535) → longitud < `YOKE_LOG_M_MAX` y valores limitados.
2. `formatModel` con NaN/Inf → 0 en el campo y bit 64.
3. Ida y vuelta: `formatModel` → `LineSplitter` (en 3 fragmentos arbitrarios) → `modelRow` → valores ±1e-4.
4. `modelRow` rechaza: 25 o 27 enteros, letra en un número, línea vacía, prefijo `X,`.
5. `formatGains` → `gainsRow` ida y vuelta con los 12 valores.
6. `LineSplitter` con línea > 255 → no llama y sigue con la siguiente.
7. `Replay.h`: cabecera exacta aceptada, una columna cambiada rechazada, fila de 33 campos rechazada, fila válida
   leída con `sample` y `armed` correctos.
8. Comparador: sobre un ensayo sintético generado en la prueba (telemetría + `pico.csv` producido con
   `PicoForceModels` + `formatModel` + `modelRow`) → A OK; alterando un `ground` en 0,01 → A informa el desvío.
   Además: un tramo con `skip` creciente queda excluido; sin filas armadas da `SIN DATOS`.
   (Separar la lógica del comparador en `tests/comparar_ensayo.h` para poder llamarla desde la suite.)
9. `SimEffects`: `modeledRevision()` es 0 tras el primer ciclo armado (reinicio por `!started`) y vale la revisión
   después del segundo; dos tramas antes de un `update` → `skipped()==1`; las fuerzas de `command()` no cambian
   (mismo resultado que un caso existente de la suite).

`tests/comparar_ensayo.vcxproj`: mismas `AdditionalIncludeDirectories` que `tests/simconnect_checks.vcxproj`
(raíz del repo y `SimConnectBridge/`), sin SimConnect SDK.

Compilación: `probar.ps1` debe compilar también `tests\comparar_ensayo.vcxproj` en el paso 1 y reportar
`OK comparar_ensayo` (cambio pequeño en `probar.ps1`).

## 8. Criterios de aceptación
1. Suite C++ OK con los casos de §7 (los de cada parte al cerrarla); firmware (3 variantes) y puente compilan sin advertencias nuevas.
2. Firmware: tamaño de programa crece < 8 kB; con `YOKE_MODEL_LOG 0` compila y no emite `M`/`G`.
3. Vuelo corto de comprobación del usuario tras la Parte 1 (2 min, rutina libre): `pico.csv` y `ganancias.csv` con
   filas, `lineas_pico_malas` 0 o casi; `us` y `lp` de las líneas `SC8` no crecen más de 100 µs respecto de V1.8.
4. Banco (`BANCO.ps1 -Armar`, con la regla de seguridad de §5.2, Pico conectada y UF2 nuevo): `RESULTADO: OK`,
   A >= 99 %, B7 <= 2 %.
5. Ensayo de vuelo del usuario: `RESULTADO: OK` y fases F1-F11 con los comportamientos de §6. Lo evalúa Claude.

## 9. Riesgos
| Riesgo | Mitigación |
| --- | --- |
| Más tráfico USB → bloqueo | Solo escrituras con `availableForWrite` bajo `UsbLock`, sin espera; pérdidas contadas (B7). |
| `snprintf` largo alarga el lazo | Enteros sin `%f`; `lp` informado en C; criterio de banco. |
| `--replay` con motores alimentados | `--enable` solo con `APAGADA` escrito o confirmación del usuario en HANDOFF (§5.2). |
| Guardado en flash durante el ensayo | Hueco del lazo y `drop` legítimos; la rutina pide salir del menú sin guardar. |
| Dos tramas en un ciclo | Contadas en `skip`; tramo excluido de A e informado. |
| Diferencias de `libm` PC vs Pico | Tolerancias de §4.1; se informa la peor diferencia. |
| Fases mal marcadas | Las marcas solo agrupan el resumen; A y B no dependen de ellas. |

## 10. Documentación afectada
- `LEEME md/CALCULO_PICO_SC8.md` — sección "Registros":
  - TEXTO ACTUAL: `` - `bridge.log`: conexión, errores del enlace USB con hora local y código de Windows, reconexiones y las líneas `Pico: BOOT ...`. ``
  - TEXTO NUEVO:
    ```
    - `bridge.log`: conexión, errores del enlace USB con hora local y código de Windows, reconexiones y las líneas `Pico: BOOT ...`. La línea final informa `lineas_pico_malas`.
    - `pico.csv`: una fila por muestra modelada por la Pico (y al menos cada 100 ms), con lo que calcula: presión normalizada, ráfagas, amplitudes de buffet, suelo, envolvente, rigidez de centrado, posición, velocidad, comando y PWM por eje, pico de PWM desde la fila anterior, estado (`flags`: 1 efectos activos, 2 armado, 4 pausa, 8 menú, 16 DTR, 32 telemetría cruda, 64 valor no finito, 128 modelo calculado), líneas perdidas (`dropped`) y muestras no modeladas (`skipped`). `sample_time_ms` coincide con el de `telemetry.csv`.
    - `ganancias.csv`: ganancias del menú (0..1) y bits habilitados, al conectar, al cambiar y cada 5 s.
    - `--replay <archivo.csv>` reenvía a la Pico las filas `sent=1` de un `telemetry.csv` grabado, sin simulador (banco). Con `--enable` arma: usar solo con la fuente de los motores apagada. `ENSAYO.ps1`, `BANCO.ps1` y `tests\bin\comparar_ensayo.exe` archivan y comparan los registros en `ensayos\` (ver `LEEME md/ENSAYOS.md`).
    ```
  - TEXTO ACTUAL: `la generación previa se conserva como `bridge.anterior.log`, `telemetry.anterior.csv` y `pico.anterior.log`.`
  - TEXTO NUEVO: `la generación previa se conserva como `bridge.anterior.log`, `telemetry.anterior.csv`, `pico.anterior.log`, `pico.anterior.csv` y `ganancias.anterior.csv`.`
- `LEEME md/LEEME_SIMCONNECT.md:54`:
  - TEXTO ACTUAL: `` `bridge.log`, `telemetry.csv` y `pico.log`, junto al EXE, se reemplazan en cada ejecucion; ``
  - TEXTO NUEVO: `` `bridge.log`, `telemetry.csv`, `pico.log`, `pico.csv` y `ganancias.csv`, junto al EXE, se reemplazan en cada ejecucion; ``
- La tesis (`tesis/`) **no** se modifica: esta tarea agrega herramientas de registro y ensayo y no cambia cómo se
  calculan los efectos.
- Nuevo `LEEME md/ENSAYOS.md`: cómo usar `ENSAYO.ps1`/`ENSAYO.cmd` y `BANCO.ps1`, tabla de fases de §6, columnas de
  `pico.csv` (§3.1) y significado de `informe.txt` (§4.4). Breve, sin repetir este diseño completo.
