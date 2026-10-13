# Diseño: opción "REGISTRO DATOS" en el menú y diagnóstico de ciclos lentos del lazo

Rama: `mejora/registro-menu-y-ciclos-lentos` (desde `master`, V2.0). Autor: Claude, 2026-10-13.
Revisado antes de entregar por Embedded Firmware Engineer, Master Plan Architect y Experiment Tracker.
Decisiones del usuario (2026-10-13):
- el estado del registro **se guarda en memoria**;
- al desactivarlo dejan de llenarse **los tres CSV** (`telemetry.csv`, `pico.csv`, `ganancias.csv`);
- la tesis lleva **una mención**.

Las líneas citadas de `MiYoke_PD.ino` son de `master` (V2.0). Cada punto se ubica además por su contenido, que
es lo que manda si una línea se corre.

## 1. Objetivos

### 1.1 Opción de menú "REGISTRO DATOS"
- CALIBRACIÓN recorre hoy 3 listas (`CalibrationMenu.h:81-84`): MODO CALIBRACION → EFECTOS → CALIBRAR POTES.
  Se agrega una cuarta: **REGISTRO DATOS**, con el estado abajo (`* Activado` / `* Desactivado`).
- OK entra a editar; + = Activado; − = Desactivado; OK guarda en flash; ATRÁS descarta.
- El texto "REGISTRO DE DATOS" (17 caracteres) no entra en el LCD de 16 columnas: se muestra `REGISTRO DATOS`.
- Desactivado: la Pico no envía `M` ni `G`, y el puente no escribe filas en los tres CSV, que quedan solo con su
  cabecera. `bridge.log` y `pico.log` siguen siempre. No cambia ninguna fuerza.

### 1.2 Diagnóstico del ciclo lento de ~39 ms
- Evidencia [HANDOFF, "Ensayos del usuario"]: `lp` = 38 948 µs (V1.8, 11/10) y 39 252 µs (12/10, entre el vuelo corto
  y el banco, sin ensayo activo); en ensayos activos ≤ 2 412 µs. `lp`/`gap` son máximos acumulados sin hora ni
  etapa (`MiYoke_PD.ino:172-173`).
- Objetivo: **medir, no corregir**. Para cada ciclo lento se registra qué etapa tardó más, cuánto, cuándo y en qué
  estado. La corrección será otra tarea.
- Hipótesis (sin verificar):
  - H1: abrir o cerrar el puerto USB (etapa SERIE o `ENTRE_CICLOS`, cerca de un cambio de `dtr`);
  - H2: LCD I2C (etapa `LCD`);
  - H3: menú habilitado sin puente (flag menú=1 con `dtr`=0);
  - H4: un guardado en flash oculto (etapa `FLASH` fuera de un guardado del usuario).

### 1.3 Fuera de alcance
Fuerzas, ganancias, filtros, constantes de seguridad (regla 7), la trama SC8, la línea `SC8 t=...` y la línea `BOOT`.

## 2. Registro persistente (nuevo `RecordSetting.h`, raíz, puro)
```cpp
namespace YokeRecord {
struct Settings { bool enabled=true; };
constexpr uint16_t recordOffset=96;   // ganancias 0-35 (GainSettings.h:61), potes 64-83 (PotCalibration.h:25-26)
constexpr uint8_t recordSize=8;       // "YREC"(4) + version=1 (1) + enabled 0/1 (1) + CRC16(2)
void encode(const Settings&, uint8_t* out);
bool decode(const uint8_t* in, Settings& out); // magic, version!=1, enabled∉{0,1} o CRC malo → false, out sin tocar
}
```
- CRC con `YokeSC::crc`, como `GainSettings.h:67`.
- `static_assert(YokePots::recordOffset+YokePots::recordSize<=recordOffset && recordOffset+recordSize<=256)`.
- Flash borrada o registro inválido ⇒ **Activado** (falla segura).
- `EEPROM.commit()` reescribe la imagen completa de 256 B: guardar este registro no altera los otros. La
  restauración ante fallo de `CalibrationControls.h:76` cubre los 8 B.

## 3. Menú y controles

### 3.1 `CalibrationMenu.h` (puro)
1. `Screen` agrega `BrowseRecord, EditRecord`; `Notice` agrega `RecordSaved` (`:196`).
2. Constructor: `Menu(YokeGains::Settings&, YokePots::Settings* pots=nullptr, YokeRecord::Settings* record=nullptr)`.
   La lista existe solo si `record!=nullptr`.
3. Ciclo de CALIBRACIÓN (`:81-84`):
   - Browse/Edit → BrowseEffects;
   - BrowseEffects/EditEffect → BrowsePots;
   - BrowsePots/CalibratePots → BrowseRecord, o Browse si `record==nullptr`;
   - BrowseRecord/EditRecord → Browse.
   Como hoy, `index=0` y `error=false`.
4. ATRÁS (`:86-90`): EditRecord → BrowseRecord; BrowseRecord → Normal.
5. Los casos BrowseRecord y EditRecord se evalúan **antes** del bloque `if(screen==Screen::Browse || ...)` (`:117`),
   para que no caigan en el `else` de edición, que usa `spec(selected())`.
   - BrowseRecord: +/− no hacen nada; OK → EditRecord con `draft=record->enabled`, `error=false`.
   - EditRecord: + → `draft=1, error=false`; − → `draft=0, error=false`; OK: si `error`, `pending=true` (reintento);
     si `draft==record->enabled`, volver a BrowseRecord; si no, `pending=true`.
6. `bool savingRecord() const { return pending && screen==Screen::EditRecord; }`.
   `YokeRecord::Settings recordCandidate() const { return {draft!=0}; }`.
   `candidate()` (`:142-148`) no aplica `draft` a las ganancias si `savingRecord()`.
7. `finishSave(ok)` (`:150`): la rama `savingRecord()` va **primero**. Con `ok`: `record->enabled=draft!=0`,
   `screen=BrowseRecord`, `show(Notice::RecordSaved)`. En cualquier caso: `error=!ok` y `pending=false`.
8. `rows()`:
   - `Notice::RecordSaved` se evalúa junto a los otros avisos, antes de las pantallas: `GUARDADO` / `REGISTRO DATOS`.
   - BrowseRecord: `REGISTRO DATOS` / `* Activado` o `* Desactivado`.
   - EditRecord: `REGISTRO DATOS` (o `ERROR GUARDADO`) / `Activado` o `Desactivado`, más `OK: REINTENTAR` si `error`.
   - Todos los textos tienen 16 caracteres o menos.
9. Sin repetición de teclas: `CalibrationControls.h:51` solo repite en `Screen::Edit`.

### 3.2 `CalibrationControls.h`
- Constructor: `Controls(YokeGains::Settings&, YokePots::Settings&, YokeRecord::Settings* record=nullptr)`; el puntero
  se pasa al `Menu`.
- `begin()`: si `record`, leer `recordSize` bytes desde `recordOffset` y aplicar `decode`.
- `save()` (`:61-78`): la rama `menu.savingRecord()` va **antes** que la de potes y ganancias:
  `offset=YokeRecord::recordOffset`, `size=YokeRecord::recordSize`, `encode(menu.recordCandidate(),record)`. El resto,
  igual que hoy.

### 3.3 Uso en `MiYoke_PD.ino`
- Global `YokeRecord::Settings recordSettings;`, declarado siempre que `YOKE_MODEL_LOG && SIMCONNECT_ADDITIONS_ENABLED`
  (si la calibración está apagada, queda `enabled=true`).
- `calibration(gainSettings,potSettings,YOKE_RECORD_OPTION ? &recordSettings : nullptr)`, con
  `#define YOKE_RECORD_OPTION (YOKE_MODEL_LOG && SIMCONNECT_ADDITIONS_ENABLED)` en `mis_defines.h`.
- El guardado usa el camino existente (motores a 0 y watchdog de 3000 ms).

## 4. Firmware: línea `L`, ciclos lentos y línea `W`

### 4.1 Medición por etapa (`LoopSupervisor.h`, puro)
```cpp
enum Stage { ..., Between=14 };             // stageName(14) = "ENTRE_CICLOS"
struct StageTimer {
    uint8_t current=Idle, worst=Idle; uint32_t since=0, worstUs=0;
    void begin(uint32_t us) { current=Idle; since=us; worst=Idle; worstUs=0; }
    void enter(uint8_t stage,uint32_t us) { const uint32_t d=us-since; if(d>worstUs){worstUs=d;worst=current;} current=stage; since=us; }
};
struct SlowEvent { uint32_t atMs, lpUs, idleUs, stageUs; uint8_t stage, flags; };
struct SlowLog {                            // anillo de 4; cada evento se envía una vez
    SlowEvent e[4]; uint8_t head=0, count=0; uint16_t total=0, lost=0;
    void push(const SlowEvent&);            // total++ (saturado); lleno: pisa el más viejo y lost++ (saturado)
    bool front(SlowEvent&) const; void pop();
};
int formatSlow(char* out,size_t size,uint16_t seq,uint32_t ageMs,const SlowEvent&,uint16_t total,uint16_t lost);
// "\nW,<seq>,<hace_ms>,<lp_us>,<idle_us>,<etapa>,<etapa_us>,<flags>,<total>,<perdidos>\n"
```
- `flags` del evento: 1 = `dtr`, 2 = `rawAvailable`, 4 = menú activo, 8 = armado.
- La resta sin signo resuelve el desborde de `micros()`. Una interrupción (p. ej. USB) que ocurra durante una etapa
  suma su tiempo a esa etapa; está bien, porque se busca dónde se perdió el tiempo.
- `mis_defines.h`: `#ifndef YOKE_SLOW_LOOP_US` → `3000`; `static_assert(YOKE_SLOW_LOOP_US > CONTROL_LOOP_MS*1000)`.

### 4.2 Integración en `MiYoke_PD.ino` (bajo `SIMCONNECT_ADDITIONS_ENABLED && YOKE_MODEL_LOG`)
1. Globales: `YokeHealth::StageTimer stageTimer; YokeHealth::SlowLog slowLog; uint32_t prevElapsedUs=0;`
   e `inline void markStage(uint8_t s){ YokeHealth::mark(s); stageTimer.enter(s,micros()); }`. Sin esos flags,
   `markStage` solo llama a `mark`.
2. En `loop()`, reemplazar cada `YokeHealth::mark(...)` por `markStage(...)`. Las de `setup()` y la de
   `isr_hardfault` quedan como están.
3. Después de cada `Serial.write` del bloque `ModelLog`, volver a `markStage(YokeHealth::ModelLog)`, para que el
   formateo siguiente no se atribuya a `SERIE_TX`.
4. Comienzo del cuerpo, justo después de `lastControlLoopUs = nowUs;` (`:174`):
   - `idleUs = gapUs - prevElapsedUs` (tiempo **fuera** del cuerpo, incluidos `yield()`/`tud_task`; `UsbGuard.h`);
   - si `idleUs >= YOKE_SLOW_LOOP_US - CONTROL_LOOP_MS*1000`, `push` con `stage=Between`, `stageUs=idleUs`, `lpUs=0`;
   - luego `stageTimer.begin(nowUs)`.
5. Nueva función `finishLoop(uint32_t nowUs)`. Reemplaza el par `elapsedUs`/`loopWorstUs` + `YokeHealth::mark(Idle)`
   en las **tres** salidas del cuerpo: los dos `return` tras `calibration.save()` (`:225-228` y `:254-257`, el que
   termina en `YokeHealth::mark(YokeHealth::Idle); return;`) y el final de `loop()`. Hace:
   - `elapsedUs=micros()-nowUs`; actualiza `loopWorstUs`; `stageTimer.enter(Idle,micros())`;
   - si `elapsedUs >= YOKE_SLOW_LOOP_US`, `push` con `lpUs=elapsedUs`, `idleUs=0`, `stage=stageTimer.worst`,
     `stageUs=stageTimer.worstUs`, `atMs=millis()` y los flags;
   - `prevElapsedUs=elapsedUs`; `YokeHealth::mark(Idle)`.
6. Cada guardado en flash genera un `W` con etapa `FLASH`. Es esperado y sirve de control positivo.

### 4.3 Líneas de diagnóstico `W` / `L` / `M` / `G` (bloque `ModelLog`)
- **Una sola** de `W`/`L`/`M`/`G` por ciclo, con este orden de prioridad (como `modelWritten`, `:340`). `BOOT`
  (`:312-321`) y `SC8` siguen como hoy, aparte.
- Todas exigen `dtr && simTelemetry.rawAvailable && uint32_t(now-simTelemetry.last)<300` (que haya llegado una trama
  del puente hace menos de 300 ms) y `availableForWrite() >= YOKE_LOG_M_MAX` antes de formatear. Así, un programa que
  solo abre el puerto (la detección del lanzador en P7, `?` sin tramas) no recibe los eventos `W`, que quedan en el
  anillo hasta que se conecte el puente.
- **W:**
  - si `slowLog.front(ev)` y pasaron ≥ 20 ms desde el último `W` (para no dejar a `M` sin salida), escribir
    `formatSlow(..., millis()-ev.atMs, ...)` y `pop()`;
  - es independiente de `recordSettings.enabled`, porque es diagnóstico.
- **L:**
  - `formatRecord(out,size,bool)` en `PicoLog.h` produce `"\nL,1\n"` o `"\nL,0\n"`;
  - se envía si `!recordSent`, si `recordSettings.enabled != lastRecordSent` o cada 5000 ms;
  - `recordSent=false` en el mismo flanco de subida de `dtr` que hoy reinicia `logSynced` (`:335`), antes de `lastDtr=dtr`.
- **M y G:**
  - solo si `recordSettings.enabled && recordSent`;
  - al pasar de Desactivado a Activado: `gainsSent=false`, `logSynced=false` y `peak[0]=peak[1]=0`;
  - `gainsSent=false` también en el flanco de subida de `dtr`.

## 5. Puente

### 5.1 `SimConnectBridge/PicoCsv.h` (puro)
```cpp
struct CsvGate {
    enum class State : uint8_t { Pending, On, Off };
    State state=State::Pending; uint64_t pendingSince=0; std::vector<std::string> held;
    void start(bool pico);   // sin Pico (--port vacío): On; con Pico: Pending, sin filas retenidas
    template<class W> void row(const std::string& r,uint64_t now,W write);
      // On: write(r). Off: descarta. Pending: la primera retenida fija pendingSince=now; retiene.
    template<class W> void recordState(bool on,W write);  // On: write() de las retenidas en orden; Off: las descarta
    template<class W> void tick(uint64_t now,W write);    // Pending con filas y (now-pendingSince>=2000 o >=2000 filas) → On + volcado
    template<class W> void finish(W write);              // al cerrar: Pending → volcar retenidas (falla segura)
    void reopen();           // al reconectar: Pending y held vacío
    bool writesPico() const { return state!=State::Off; }
};
bool recordLine(const char* line,bool& on);       // exactamente "L,0" o "L,1"
bool slowLine(const char* line,uint32_t v[9]);    // "W," + 9 enteros sin signo, sin texto extra
```
Las filas retenidas son la cadena completa con `\n`. Su hora es la que se tomó en `Record()`, y el orden se conserva.

### 5.2 `SimConnectBridge/main.cpp`
1. `Record()` (`:59-67`) arma la misma fila en `char row[1024]` con `snprintf`. Si el resultado es ≤ 0 o ≥ 1024, la
   fila se descarta y suma a `filas_csv_malas`. La fila va por `gate.row(row,GetTickCount64(),writer)`, donde `writer`
   hace `fputs` en `csv`. El `fflush(csv)` de las filas `sent` se mantiene, y se agrega otro `fflush(csv)` después de
   cada volcado (`recordState` On, `tick`, `finish`). **El formato de la fila no cambia**: `Replay.h` y el comparador
   exigen los mismos 34 campos.
2. `gate.start(!com.empty())` antes del bucle principal y también en `--replay`; `gate.tick(now,writer)` en `PollPort`;
   `gate.reopen()` junto a `reconnect.onReopened(...)`; `gate.finish(writer)` antes de `fclose(csv)` en todas las salidas.
3. Filtro del splitter (`:194`): aceptar los prefijos `M,`, `G,`, `L,` y `W,`. Las cuatro cuentan en `lineas_pico`;
   las mal formadas, en `lineas_pico_malas`.
   - `L,`: `recordState`. Si cambia el estado, `Log("[%s] Pico: registro CSV %s\n", hora, on ? "ACTIVADO" : "DESACTIVADO")`.
   - `M,`/`G,`: se escriben si `gate.writesPico()`, sin retención (la Pico ya no las envía antes de `L`).
   - `W,`: si `etapa<=14`, `Log("[%s] Pico: ciclo lento lp=%u us fuera=%u us etapa=%u:%s (%u us) hace %u ms flags=0x%X total=%u perdidos=%u\n",...)`
     con `YokeHealth::stageName` (incluir `LoopSupervisor.h`); si no, cuenta como mala.
4. La línea `Fin:` agrega `registro=ACTIVADO|DESACTIVADO|PENDIENTE ciclos_lentos=<n> filas_csv_malas=<n>`.
5. Cambiar el registro en medio de un ensayo produce un tramo incompleto en los CSV. Los ensayos que se evalúan
   (`ENSAYO`, `BANCO`) se hacen sin cambiarlo.

## 6. Comparador y scripts
1. `tests/comparar_ensayo.h::load()`:
   - inmediatamente después de leer `telemetry.csv` (antes de exigir `pico.csv`/`M`, hoy `:284`), si no hay filas de
     telemetría, devolver un resultado `SIN DATOS`;
   - el informe tiene la línea `REGISTRO: sin filas en telemetry.csv (REGISTRO DATOS desactivado en el menu?)` y
     `RESULTADO: SIN DATOS`;
   - código de salida 1.
2. `ENSAYO.ps1`: en el mensaje del `Read-Host` de "En tierra, antes de F1" (`:44`), anteponer la línea
   `Verifica en el menu: REGISTRO DATOS = Activado`.
3. Nuevo `CICLOS.ps1` + `CICLOS.cmd` (mismo estilo que `ENSAYO.cmd`, con `pause` siempre), para el ensayo de §7:
   - crea `ensayos\AAAAMMDD-HHMMSS_ciclos\`;
   - muestra la tabla de §7;
   - por cada Enter del usuario, escribe en `marcas.csv` la línea `host_time_ms,hora_local,paso`: `GetTickCount64`
     como en `ENSAYO.ps1`, y `hora_local` en `HH:mm:ss`, el mismo formato que `bridge.log`. El paso se toma en orden,
     de P0 a P9;
   - con `FIN`, copia de `SimConnectBridge\bin\Win32\RealismoSC8\` los archivos `bridge.log`,
     `bridge.anterior.log`, `pico.log` y `pico.anterior.log` que existan (si falta alguno, lo avisa), y extrae a
     `ciclos.txt` las líneas `ciclo lento` y `registro CSV` de los dos `bridge`;
   - no ejecuta el comparador ni borra nada.

## 7. Protocolo del ensayo de ciclos lentos (lo hace el usuario; motores apagados; ~60 min)
Reglas: no desenchufar la Pico (reinicia y borra los eventos pendientes); marcar cada paso con Enter en `CICLOS.cmd`.

| Paso | Acción | Duración | Para qué |
| --- | --- | --- | --- |
| P0 | UF2 nuevo; lanzador; avión en pista; pausa | — | base |
| P1 | No tocar nada (en pausa) | 10 min | control negativo: 0 `W` |
| P2 | REGISTRO DATOS → Desactivado → OK; luego → Activado → OK | 2 min | control positivo: 2 `W` FLASH; prueba la opción |
| P3 | Menú: recorrer las 4 listas y salir sin guardar, 3 veces, con 1 min entre cada una | 5 min | H2 con puente |
| P4 | Quitar pausa y volar | 5 min | operación |
| P5 | Cerrar el simulador. La Pico queda sola, conectada | 15 min | caso del 12/10 |
| P6 | Sin simulador: menú, cambiar de lista 3 veces, sin guardar | 5 min | H2 + H3 |
| P7 | Sin simulador: `powershell -ExecutionPolicy Bypass -File Iniciar-Simulador.ps1 -DetectarPlaca`, 3 veces, con 1 min entre cada una | 5 min | H1 aislada |
| P8 | Sin simulador: REGISTRO DATOS → Desactivado → OK; esperar 2 min | 3 min | control de `hace` (~120 s) |
| P9 | Abrir el simulador; pausa 2 min; REGISTRO DATOS → Activado → OK; cerrar | 4 min | recibe los eventos de P5-P8 |

**Criterios de decisión** (los aplica Claude con `ciclos.txt` y `marcas.csv`):
- **Validez del instrumento:** P1 da 0 `W`; P2 da 2 `W` FLASH; P8 da 1 `W` FLASH con `hace` dentro de 120 s ± 5 s
  contados desde la marca de P8 hasta la llegada en P9; `perdidos=0`. Si algo de esto falla, no se concluye nada y
  se corrige el instrumento.
- **Confirmar una hipótesis:** `W` ≥ 3 ms en al menos 2 de las 3 repeticiones de su condición, con etapa y flags
  coherentes y con el momento del evento (hora de llegada − `hace`) dentro de la ventana del paso.
- **Descartar:** 0 eventos en las 3 repeticiones descarta un disparo determinista, no un evento raro.
- **Sin eventos:** se concluye "no reproducido", no "resuelto". `W` queda activo en todos los ensayos y los
  eventos se acumulan.
- **Evento fuera de toda ventana:** comparar con FLASH (H4) y con `ENTRE_CICLOS`.

## 8. Pruebas de escritorio
- `tests/calibration_checks.h`:
  - ciclo de CALIBRACIÓN con 4 listas y con 3 (`record=nullptr`);
  - BrowseRecord con el estado;
  - ATRÁS desde EditRecord y desde BrowseRecord;
  - OK sin cambio vuelve sin `pending`;
  - con cambio: `savingRecord()` y `candidate()` igual a las ganancias;
  - `finishSave(true)` aplica y muestra `GUARDADO`/`REGISTRO DATOS`;
  - `finishSave(false)` muestra `ERROR GUARDADO` y OK reintenta;
  - sin pausa no se entra;
  - +/− en BrowseRecord no cambian nada;
  - todas las filas ≤ 16;
  - `YokeRecord`: ida y vuelta; 0xFF, CRC malo, versión 2 y `enabled=2` dejan `true`.
- `tests/picolog_checks.h`:
  - `formatRecord`;
  - `StageTimer`: la etapa más larga, con desborde de `micros`;
  - `SlowLog`: orden, `lost` y saturación;
  - `formatSlow` ↔ `slowLine`, y `slowLine` rechaza 8 o 10 campos;
  - `recordLine` rechaza `L,2`, `L,`, `L,1,0`;
  - `CsvGate`:
    - retiene y vuelca en orden con `L,1`;
    - descarta con `L,0`;
    - `pendingSince` desde la primera retenida (no desde `start`);
    - pasa a On por 2 s y por 2000 filas;
    - `finish` vuelca;
    - `reopen` vuelve a Pending;
    - sin Pico, On directo;
    - Off→On reanuda.
- `tests/comparar_ensayo.h`: telemetría vacía da `SIN DATOS` con la línea `REGISTRO`, aunque falte `pico.csv`.

## 9. Criterios de aceptación
1. `probar.ps1 -Pico -Potentiometers -Encoders -Bridge` todo OK; programa < +6 kB respecto de V2.0 (104 500 B).
2. Con `YOKE_MODEL_LOG 0` compila y el menú muestra 3 listas.
3. Ensayo del usuario (§7):
   - el instrumento es válido según §7;
   - con el registro Desactivado, los tres CSV no reciben filas, y vuelven a recibirlas al activarlo;
   - `bridge.log` muestra los cambios de registro.
4. `lp` en operación (P4) no crece más de 100 µs respecto del vuelo del 12/10 (2 377 µs). Es orientativo, porque son máximos.

## 10. Riesgos
| Riesgo | Mitigación |
| --- | --- |
| Se olvida el registro apagado y se pierde un ensayo | Arranca Activado con la flash vacía; aviso en `ENSAYO.ps1`; `SIN DATOS` con texto claro. |
| `W` deja sin salida a `M` si todos los ciclos son lentos | Máximo un `W` cada 20 ms; el resto se cuenta en `perdidos`. |
| Corte de energía durante `commit` | Los tres registros comparten la imagen de flash: un registro dañado cae a su valor por defecto (CRC). Mismo riesgo que hoy con las ganancias. |
| Costo de `micros()` por etapa | ~26 lecturas por ciclo; se controla con `us`/`lp` (§9.4). |
| Filas retenidas al inicio | Hasta 2 s o 2000 filas, y siempre se vuelcan al cerrar si sigue Pending. |

## 11. Documentación afectada
- `LEEME md/MENU_CALIBRACION.md`, "Pulsadores y navegacion":
  - TEXTO ACTUAL: `CALIBRACION recorre circularmente estas tres secciones:`
  - TEXTO NUEVO: `CALIBRACION recorre circularmente estas cuatro secciones:`
  - TEXTO ACTUAL: `3. **CALIBRAR POTES**: OK inicia la secuencia descrita abajo.`
  - TEXTO NUEVO:
    ```
    3. **CALIBRAR POTES**: OK inicia la secuencia descrita abajo.
    4. **REGISTRO DATOS**: abajo muestra el estado (Activado/Desactivado). OK entra, + activa y - desactiva,
       OK guarda en memoria y ATRAS descarta. Desactivado, no se llenan `telemetry.csv`, `pico.csv` ni
       `ganancias.csv`; `bridge.log` y `pico.log` siguen. Con la memoria vacia arranca Activado.
    ```
  - TEXTO ACTUAL: `ATRAS desde cualquiera de las tres listas sale al`
  - TEXTO NUEVO: `ATRAS desde cualquiera de las cuatro listas sale al`
- `LEEME md/CALCULO_PICO_SC8.md`, "Registros":
  - TEXTO ACTUAL: `La línea final informa `lineas_pico_malas`.`
  - TEXTO NUEVO: `La línea final informa `lineas_pico_malas`, `registro=`, `ciclos_lentos=` y `filas_csv_malas=`.`
  - TEXTO ACTUAL: `` - `ganancias.csv`: ganancias del menú (0..1) y bits habilitados, al conectar, al cambiar y cada 5 s. ``
  - TEXTO NUEVO:
    ```
    - `ganancias.csv`: ganancias del menú (0..1) y bits habilitados, al conectar, al cambiar y cada 5 s.
    - Menú REGISTRO DATOS: la Pico informa su estado con `L,1`/`L,0` al conectar, al cambiar y cada 5 s. Desactivado, la Pico no envía `M`/`G` y el puente no escribe filas en `telemetry.csv`, `pico.csv` ni `ganancias.csv` (al iniciar retiene hasta 2 s de filas hasta conocer el estado). `bridge.log` informa `Pico: registro CSV ACTIVADO/DESACTIVADO`.
    - Ciclos lentos: cada vuelta del lazo de 3 ms o más (`YOKE_SLOW_LOOP_US`), o un tiempo fuera del lazo equivalente (`ENTRE_CICLOS`), se informa en `bridge.log` como `Pico: ciclo lento lp=... fuera=... etapa=n:NOMBRE (... us) hace ... ms flags=...`, con la etapa que más tardó. Si ocurre sin puente, se informa en la conexión siguiente (anillo de 4; `perdidos` cuenta los pisados). Cada guardado en flash produce uno con etapa FLASH. `CICLOS.cmd` guía el ensayo de diagnóstico.
    ```
- `LEEME md/ENSAYOS.md`:
  - TEXTO ACTUAL: `Primera sesión: fuente de potencia de motores apagada. Con motores, solo después de instalar el pull-down de EN pendiente de V1.8.`
  - TEXTO NUEVO: el mismo texto, una línea en blanco y luego `Antes de cada ensayo, comprobar en el menú que REGISTRO DATOS esté en Activado; si no, los CSV quedan vacíos y el informe da SIN DATOS. No cambiarlo durante un ensayo.`
- `tesis/capitulos/03_centrado.tex` (autorizado por el usuario):
  - TEXTO ACTUAL: `Hay que registrar los valores efectivamente cargados al comparar ensayos.`
  - TEXTO NUEVO: `Hay que registrar los valores efectivamente cargados al comparar ensayos. El menú tiene además una cuarta lista, \code{REGISTRO DATOS}, que activa o desactiva el envío de los registros de ensayo (\code{telemetry.csv}, \code{pico.csv} y \code{ganancias.csv}); su estado se guarda en un registro persistente propio y no modifica ninguna ganancia ni el cálculo de fuerzas.`
- Sin cambios (verificado): `LEEME md/LEEME_POTENCIOMETROS.md:36`, `LEEME md/LEEME_LCD.md`.
