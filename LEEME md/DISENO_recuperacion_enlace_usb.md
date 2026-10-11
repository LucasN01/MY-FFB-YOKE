# Diseño: congelamiento de la Pico 2 y recuperación del enlace USB

Rama: `mejora/recuperacion-enlace-usb`. Diseño: Claude (2026-10-11), criticado por
Embedded Firmware Engineer y Master Plan Architect (hallazgos incorporados).
Estado: APROBADO PARA IMPLEMENTAR (Codex, dos partes; ver HANDOFF.md).

## 1. Problema

Durante la simulación el puente muestra
`ERROR serial write; watchdog del firmware retirara fuerzas.` y termina. El LCD queda
congelado y solo se recupera desenchufando la Pico 2 y reiniciando el lanzador. No hay
motores conectados: se descartan causas eléctricas de potencia.

### 1.1 Evidencia [logs/]

- 7 ocurrencias en `logs/inicio-*.log` (2026-10-04 a 2026-10-11), con firmware SC6
  (COM13) y SC8 (COM45): no depende de un modelo de efectos.
- En tierra (13,5 y 22,3 kt) y en vuelo (57 a 131 kt), entre 5135 y 227984 muestras:
  sin patrón de datos de entrada.
- Después, el lanzador falló varias veces con "El puerto 'COM45' no existe".
- `bridge.log`, `pico.log` y `telemetry.csv` se pisan al iniciar: no hay registro de la
  Pico de ninguna sesión con falla.

### 1.2 Causa raíz más probable (H5): interbloqueo de mutex en la pila USB de la Pico

Fuentes: core arduino-pico 6.1.1 y Adafruit TinyUSB incluidos en
`%LOCALAPPDATA%\Arduino15\packages\rp2040\hardware\rp2040\6.1.1\`.

1. `libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/Adafruit_TinyUSB_rp2040.cpp:81-88`:
   `tud_task()` se ejecuta en una IRQ de software (`usb_task_irq`), protegida solo
   con `mutex_try_enter(&__usb_mutex)`.
2. `.../src/osal/osal_pico.h:100`: "Within tinyusb, mutex is never used in ISR context".
   Pero `tud_task()` llega a `tu_edpt_claim` (`.../src/tusb.c:227-241`), que hace
   `osal_mutex_lock(mutex, OSAL_TIMEOUT_WAIT_FOREVER)` sobre el mutex interno de TinyUSB.
3. `cores/rp2040/delay.cpp:50-54`: `yield()` llama a `TinyUSB_Device_Task()` y luego a
   `TinyUSB_Device_FlushCDC()` **sin** `__usb_mutex`. `yield()` corre después de cada
   `loop()` (`cores/rp2040/main.cpp:74-77,183-184`), es decir, millones de veces por
   segundo porque `loop()` vuelve enseguida fuera del período de 2 ms.
4. `FlushCDC` → `tud_cdc_n_write_flush` → `tu_edpt_stream_write_xfer` →
   `stream_claim` → toma el mutex interno (`tusb.c:401-404`) cuando hay bytes de
   salida pendientes (tras cada `Serial.write`: cada ~100 ms en este firmware).
   `Serial.read` llega al mismo mutex por `tu_edpt_stream_read_xfer` (`tusb.c:444-452`).
5. Si la IRQ USB entra justo mientras el hilo principal tiene ese mutex, `tud_task()`
   (en la IRQ) espera para siempre a un dueño que nunca vuelve a ejecutarse (mismo
   núcleo). La IRQ no retorna: `loop()` y el LCD se congelan, nadie lee el FIFO CDC, la
   escritura del puente vence y Windows termina perdiendo el dispositivo.
6. El `SerialUSB` propio del core (pila no TinyUSB) protege cada llamada CDC con el
   mutex USB (`cores/rp2040/SerialUSB.cpp:86,97,110,120,130,144,188`,
   `CoreMutex m(&USB.mutex, false)`). La clase CDC de Adafruit no lo hace.

La ventana es de microsegundos y depende de la coincidencia con una interrupción USB,
lo que explica la rareza y la falta de patrón. Es la explicación más fuerte, pero
**sigue siendo una hipótesis** hasta validarla con el ensayo de estrés de §5.

### 1.3 Otras hipótesis (se discriminan con el diagnóstico de §3.4)

| Id | Hipótesis | Firma esperada |
|----|-----------|----------------|
| H5 | Interbloqueo USB (§1.2) | `BOOT reset=WDT etapa=INACTIVO` o `SERIE_RX/SERIE_TX`; desaparece con `YOKE_USB_GUARD 1` |
| H1 | `loop()` en bucle sin salida | `reset=WDT`, misma etapa repetible, `falla=0` |
| H2 | HardFault | `reset=WDT ... falla=1` |
| H3 | Pila USB trabada con `loop()` vivo | Sin BOOT, LCD actualizando, reintentos sin éxito |
| H4 | Lado Windows | Error de dispositivo (código ≠ 121) sin BOOT nuevo |

Descartada por revisión: Wire/LCD. Todo tiene límite: timeout de 2 ms,
`_clockStretch` de 500 µs (`Wire.cpp:301-304`) y recuperación del bus con 9 pulsos
(`Wire.cpp:388`). Nota: `WIRE_HAS_TIMEOUT` no existe en este core, así que
`LcdTelemetry.h:127-130,146-157` es código muerto. No se toca en esta tarea.

### 1.4 Defectos del puente [SimConnectBridge/main.cpp]

- `:61-63`: cualquier falla de `WriteFile` pone `failed=quit=true` y el puente termina.
- `:203`: un fallo de `ReadFile` también lo termina.
- `OpenPort` (`:123-147`) no cierra el handle cuando falla después de `CreateFileA`.
  Ocurre en `:128`, `:132`, `:134`, `:139`, `:140` y `:146`.
- `:184`: tampoco cierra `logFile` ni `picoLog` cuando falla la identificación.
- Firmware: el watchdog de 300 ms (`SimTelemetry.h:44`) es software y vive dentro de
  `loop()`. No hay watchdog por hardware.

### 1.5 Segundo síntoma: guardar en el menú cierra el puente (causa confirmada en código)

Reporte del usuario (2026-10-11): con el simulador en pausa, al entrar en calibración,
cambiar un efecto y guardar, a veces aparece `ERROR serial write` y el puente termina
(`Fin: muestras=555 error=1`). Cadena causal:

1. `CalibrationControls.h:61-78`: `save()` escribe el registro con `EEPROM.write` y llama a
   `EEPROM.commit()`.
2. `libraries/EEPROM/src/EEPROM.cpp:102-106`: `write` marca `_dirty` solo si el byte
   cambia. `commit()` (`:110-131`) vuelve enseguida si no hay cambios. Si los hay, ejecuta
   `noInterrupts()`, borra un sector de 4 KB y lo programa. El borrado tarda unos 45 ms
   típicos y hasta unos 400 ms como máximo, según la hoja de datos de la flash; ese máximo
   falta confirmarlo para la flash de la Pico 2.
3. Con las interrupciones deshabilitadas, el USB de la Pico no se atiende y el FIFO CDC no
   se vacía.
4. En pausa, el puente envía una trama cada 100 ms (`main.cpp:198`). Si una escritura
   coincide con el borrado, vence a los 10 ms (`main.cpp:133`), y la primera falla termina
   el puente (`main.cpp:61-63`).

Esto explica el "en algunos efectos": solo hay escritura en flash cuando el valor guardado
cambia, y además la escritura del puente tiene que coincidir con el borrado. La Pico no se
cuelga; el problema es que el puente no tolera una pausa breve del USB. Lo resuelven
`LinkMonitor` (§3.1: no da el enlace por perdido antes de 1000 ms y 3 vencimientos
seguidos) y la eliminación de `failed=quit=true` (§3.2). No hace falta tocar el firmware de
calibración. Prueba de aceptación: C (§5).

## 2. Objetivos y criterios de aceptación

1. **Corrección de causa (H5):** todas las llamadas CDC del hilo principal y el `yield()`
   se serializan con `__usb_mutex`.
   - Criterio: con el ensayo de estrés (§5, prueba E), el firmware con
     `YOKE_USB_GUARD 1` funciona 30 min sin `BOOT reset=WDT` ni `ERROR enlace`.
   - Si el firmware con `YOKE_USB_GUARD 0` se cuelga en ese mismo ensayo, queda
     confirmada H5.
2. **Watchdog por hardware:** si `loop()` no corre durante `YOKE_WDT_MS` (250 ms), la
   Pico se reinicia sola.
   - Criterio (prueba B): `Pico: BOOT reset=WDT` aparece en la consola y el LCD se
     reinicializa sin desenchufar.
3. **Reconexión del puente:** ninguna falla del enlace termina el puente.
   - Criterio (pruebas A y B): `bridge.log` muestra "Enlace USB recuperado" como mucho
     5 s después de reenchufar la Pico o de su reinicio, sin reiniciar el lanzador.
4. **Armado diferido:** después de reconectar, el puente envía tramas desarmadas
   durante 500 ms.
   - Criterio: en `telemetry.csv`, `armed=0` durante 500 ms o más a partir de la hora
     de recuperación.
5. **Sin reconexiones falsas:** guardar ganancias desde el menú con el puente
   conectado no provoca `ERROR enlace` (prueba C).
6. **Evidencia de cada falla**, con hora local:
   - código de Windows, bytes escritos y cola de salida;
   - motivo del reinicio y última etapa del lazo, con los registros crudos;
   - marca de HardFault.
   - Los registros de la sesión anterior se conservan.
7. **Salida del puente:** si se cierra con el enlace sin recuperar, el puente
   devuelve el código 4 (no 0).

## 3. Diseño

### 3.1 Parte 1 — `SimConnectBridge/SerialLink.h` (nuevo, puro, sin `Windows.h`)

Todo `inline` en el encabezado (el proyecto de pruebas compila 3 unidades de
traducción, `tests/simconnect_checks.vcxproj:12`).

```cpp
#pragma once
#include <cstddef>
#include <cstring>
#include <cstdint>
namespace YokeLink {
constexpr unsigned long kErrorSemTimeout = 121;   // ERROR_SEM_TIMEOUT (usbser)
constexpr unsigned kMinConsecutiveTimeouts = 3;
constexpr uint64_t kLinkTimeoutMs = 1000;         // > borrado de sector flash (max ~400 ms)
constexpr uint64_t kReconnectPeriodMs = 500;
constexpr unsigned kReconnectHandshakeMs = 600;   // por intento de reconexion
constexpr uint64_t kRearmHoldoffMs = 500;
constexpr uint64_t kRetryLogPeriodMs = 5000;

enum class WriteOutcome { Ok, Timeout, DeviceError };

// TRUE con menos bytes (STATUS_TIMEOUT) o FALSE+121 = vencimiento; otro error = dispositivo.
inline WriteOutcome classifyWrite(bool ok, unsigned long written,
                                  unsigned long size, unsigned long error) {
    if (ok && written == size) return WriteOutcome::Ok;
    if (ok || error == kErrorSemTimeout) return WriteOutcome::Timeout;
    return WriteOutcome::DeviceError;
}

// Vencimientos: reconectar solo si hay >=3 seguidos Y pasaron >=1000 ms desde el primero.
struct LinkMonitor {
    unsigned consecutiveTimeouts = 0;
    uint64_t firstTimeoutMs = 0;
    bool onWrite(WriteOutcome o, uint64_t nowMs) {   // true = enlace perdido
        if (o == WriteOutcome::Ok) { consecutiveTimeouts = 0; return false; }
        if (o == WriteOutcome::DeviceError) return true;
        if (consecutiveTimeouts++ == 0) firstTimeoutMs = nowMs;
        return consecutiveTimeouts >= kMinConsecutiveTimeouts &&
               nowMs - firstTimeoutMs >= kLinkTimeoutMs;
    }
    void reset() { consecutiveTimeouts = 0; firstTimeoutMs = 0; }
};

// Máquina de reconexión (testeable sin Windows).
struct Reconnector {
    bool lost = false;
    uint64_t lostAtMs = 0, nextAttemptMs = 0, rearmAtMs = 0, lastRetryLogMs = 0;
    unsigned reconnections = 0;
    void onLost(uint64_t now) { if (!lost) { lost = true; lostAtMs = now; nextAttemptMs = now; ++reconnections; } }
    bool shouldAttempt(uint64_t now) const { return lost && now >= nextAttemptMs; }
    void onAttemptFailed(uint64_t now) { nextAttemptMs = now + kReconnectPeriodMs; }
    void onReopened(uint64_t now) { lost = false; rearmAtMs = now + kRearmHoldoffMs; }
    bool armAllowed(uint64_t now) const { return !lost && now >= rearmAtMs; }
    bool shouldLogRetry(uint64_t now) { if (now - lastRetryLogMs < kRetryLogPeriodMs && lastRetryLogMs) return false; lastRetryLogMs = now; return true; }
};

// Extrae líneas completas que empiezan con "BOOT " (pueden llegar partidas).
struct BootLineScanner {
    char line[160] = {0};
    size_t n = 0;
    bool overflow = false;
    template <class F> void feed(const char* data, size_t size, F onBoot) {
        for (size_t i = 0; i < size; ++i) {
            const char c = data[i];
            if (c == '\n' || c == '\r') {
                line[n] = 0;
                if (!overflow && n >= 5 && std::strncmp(line, "BOOT ", 5) == 0) onBoot(line);
                n = 0; overflow = false;
            } else if (n + 1 < sizeof(line)) line[n++] = c;
            else overflow = true;
        }
    }
};
}
```

Notas de diseño:
- La ventana de 1000 ms evita reconexiones falsas al guardar en flash.
  `EEPROM.commit` deshabilita interrupciones mientras borra un sector de 4 KB
  (`libraries/EEPROM/src/EEPROM.cpp:121-129`) y en ese lapso el USB no se atiende.
- Las tramas parciales no necesitan `PurgeComm`. El analizador de la Pico descarta una
  trama incompleta tras 40 ms sin bytes (`SimTelemetry.h:54`), o falla el CRC y se
  resincroniza en la próxima cabecera `a5 5a`. Se pierden 1 o 2 tramas; es aceptable.

### 3.2 Parte 1 — cambios en `SimConnectBridge/main.cpp`

1. `#include "SerialLink.h"`. Globales: `YokeLink::LinkMonitor link;`,
   `YokeLink::Reconnector reconnect;`, `YokeLink::BootLineScanner bootScanner;`.
2. Función `Stamp(char out[16])` con `GetLocalTime` → `"hh:mm:ss"`. Se usa en las líneas
   de error, reconexión y `Pico:`.
3. `OpenPort(const std::string& name, unsigned handshakeMs)`:
   - Ante cualquier `return false` posterior a `CreateFileA` (líneas actuales 128, 132,
     134, 139, 140 y 146): `CloseHandle(port); port=INVALID_HANDLE_VALUE;`.
   - Condición del handshake: `while(!quit && GetTickCount64()-start<handshakeMs)`.
   - La llamada inicial usa 3000 ms (sin cambios). La reconexión usa
     `kReconnectHandshakeMs` (600 ms).
4. `ClosePort()`: `CloseHandle(port); port=INVALID_HANDLE_VALUE;` si es válido.
5. `LinkLost(const char* motivo, unsigned long codigo, unsigned long escritos)`:
   - Si `port` es válido, `ClearCommError(port,&errores,&estado)` (cola de salida =
     `estado.cbOutQue`).
   - Log: `"[%s] ERROR enlace USB con la Pico: %s (codigo Windows %lu, bytes %lu/%u, cola salida %lu, errores 0x%lX). Sin tramas validas el firmware desarma por watchdog. Reconectando %s...\n"`.
   - `ClosePort(); reconnect.onLost(GetTickCount64());`. **No** toca `failed` ni `quit`.
6. `Send(bool armed)`:
   - Antes de construir la trama: `if(port==INVALID_HANDLE_VALUE || !reconnect.armAllowed(GetTickCount64())) armed=false;`.
   - Reemplazar el bloque actual `main.cpp:59-66` por:
   ```cpp
   bool wrote=false;
   if(port!=INVALID_HANDLE_VALUE) {
       DWORD written=0;
       const BOOL ok=WriteFile(port,frame,sizeof(frame),&written,nullptr);
       const DWORD error=ok?0:GetLastError();
       const auto outcome=YokeLink::classifyWrite(ok!=FALSE,written,sizeof(frame),error);
       wrote=outcome==YokeLink::WriteOutcome::Ok;
       if(link.onWrite(outcome,GetTickCount64()))
           LinkLost(outcome==YokeLink::WriteOutcome::Timeout?"escritura vencida":"error de dispositivo",error,written);
   }
   lastSend=GetTickCount64();
   Record(wrote,wrote&&armed);
   ```
   - `Record(wrote, wrote&&armed)` mantiene la definición documentada de las columnas:
     `sent=1` = escritura USB completa; las filas con `sent=0` tienen `armed=0`.
7. Lazo principal (`main.cpp:188-207`):
   - `ReadFile` fallido → `LinkLost("error de lectura",GetLastError(),0)` en lugar de
     `failed=true; break;`.
   - Lo leído (si `n>0`) se escribe en `picoLog` como hoy y además pasa por
     `bootScanner.feed(b,n,[](const char* l){ char t[16]; Stamp(t); Log("[%s] Pico: %s\n",t,l); });`.
   - Bloque de reconexión, solo si `!com.empty()`, antes de `Sleep(5)`:
   ```cpp
   if(!com.empty() && port==INVALID_HANDLE_VALUE && reconnect.shouldAttempt(now)) {
       if(reconnect.shouldLogRetry(now)) Log("Reintentando %s...\n",com.c_str());
       if(OpenPort(com,YokeLink::kReconnectHandshakeMs)) {
           const ULONGLONG t=GetTickCount64(); char s[16]; Stamp(s);
           Log("[%s] Enlace USB recuperado en %s tras %.1f s; armado diferido %llu ms.\n",
               s,com.c_str(),(t-reconnect.lostAtMs)/1000.0,(unsigned long long)YokeLink::kRearmHoldoffMs);
           reconnect.onReopened(t); link.reset(); bootScanner=YokeLink::BootLineScanner{};
       } else reconnect.onAttemptFailed(GetTickCount64());
   }
   ```
   - El cálculo de `active` no cambia: el desarme se aplica dentro de `Send()` (punto 6).
8. Salida: `Log("Fin: muestras=%u error=%d reconexiones=%u\n",samples,failed,reconnect.reconnections)`.
   Código de salida: `reconnect.lost ? 4 : (failed||samples==0 ? 1 : 0)`.
9. `main.cpp:184`: antes de `return 3`, cerrar también `logFile` y `picoLog`.
10. Rotación de registros, antes de los `fopen_s(...,"w")` (`main.cpp:177-179`).
    - Si el `pico.log` existente mide más de 0 bytes, renombrar con
      `MoveFileExA(...,MOVEFILE_REPLACE_EXISTING)`:
      - `bridge.log` → `bridge.anterior.log`
      - `telemetry.csv` → `telemetry.anterior.csv`
      - `pico.log` → `pico.anterior.log`
    - Una ejecución de monitor sin puerto (pico.log vacío) no pisa la generación
      anterior. Si el renombrado falla, continuar.
11. Desaparece el texto "watchdog del firmware retirara fuerzas".

### 3.3 Parte 2 — corrección H5: `UsbGuard.h` (nuevo) y `yield()` propio

```cpp
#pragma once
#if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB) && YOKE_USB_GUARD
#include <pico/mutex.h>
extern mutex_t __usb_mutex;   // Adafruit_TinyUSB_rp2040.cpp:79 (enlace C++)
// Mismo criterio que cores/rp2040/SerialUSB.cpp (CoreMutex sobre el mutex USB).
// La IRQ usb_task_irq usa try_enter: si el hilo tiene el mutex, la IRQ saltea y
// tud_task() se ejecuta luego desde yield(). Mismo núcleo: nunca espera.
struct UsbLock {
    UsbLock() { mutex_enter_blocking(&__usb_mutex); }
    ~UsbLock() { mutex_exit(&__usb_mutex); }
    UsbLock(const UsbLock&) = delete; UsbLock& operator=(const UsbLock&) = delete;
};
#else
struct UsbLock { UsbLock() {} };
#endif
```

En `MiYoke_PD.ino`, con `YOKE_USB_GUARD 1`:

- Definir el `yield()` fuerte. El del core es débil: `cores/rp2040/delay.cpp:30,50`.
  ```cpp
  #if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB) && YOKE_USB_GUARD
  extern "C" void yield() {
      if (mutex_try_enter(&__usb_mutex, nullptr)) {
          tud_task();
          TinyUSB_Device_FlushCDC();
          mutex_exit(&__usb_mutex);
      }
  }
  #endif
  ```
  No se llama a `TinyUSB_Device_Task()`: su `try_enter` fallaría, porque el mutex no
  es recursivo.
- Envolver con `{ UsbLock usb; ... }` **cada** acceso a `Serial` dentro de `loop()`:
  - el bucle de lectura (`.ino:144-149`, incluida la respuesta `YOKE17SC8`);
  - las dos llamadas a `Serial.dtr()` (`.ino:154,167`): tomar `const bool dtr` una vez
    bajo el lock y reutilizarla;
  - `debugPrintMotion` (`.ino:61,91-92`);
  - la línea del modelo (`.ino:234-243`);
  - la línea BOOT (§3.4).
- Dentro del lock solo se llama a `Serial.write` si antes `availableForWrite()>=n`.
  Si no, el `yield()` interno de `write` (`Adafruit_USBD_CDC.cpp:247-252`) no lograría
  tomar el mutex y giraría sin fin. Hoy ya se cumple; mantenerlo.
- `mis_defines.h`: `#define YOKE_USB_GUARD 1` (0 solo para el ensayo comparativo E).

### 3.4 Parte 2 — supervisión: `LoopSupervisor.h` (nuevo), `mis_defines.h`, `MiYoke_PD.ino`

`LoopSupervisor.h`: parte pura `inline` (se compila en `tests/`) y parte de hardware
bajo `#if defined(ARDUINO_ARCH_RP2040)`.

```cpp
namespace YokeHealth {
enum Stage : uint8_t { Idle=0, SerialRx=1, Calibration=2, FlashSave=3, Effects=4,
    Motion=5, Force=6, Motor=7, ModelLog=8, DebugLog=9, Lcd=10, PotRead=11,
    SerialTx=12, TestHang=15 };
constexpr uint32_t kMagic = 0x59C80000u;
constexpr uint8_t kFaultBit = 0x80;
inline uint32_t encode(uint8_t stage) { return kMagic | stage; }
inline bool decode(uint32_t v, uint8_t& stage, bool& fault) {
    if ((v & 0xFFFFFF00u) != kMagic) return false;
    stage = uint8_t(v & 0x7F); fault = (v & kFaultBit) != 0; return true; }
inline const char* stageName(uint8_t s);  // INACTIVO,SERIE_RX,CALIB,FLASH,EFECTOS,MOVIM,FUERZA,MOTOR,LOGMOD,LOGDBG,LCD,POTES,SERIE_TX,?,?,PRUEBA
inline const char* resetName(int r);      // orden de RP2040::resetReason_t: DESCONOCIDO,ENCENDIDO,PIN_RUN,SOFT,WDT,DEBUG,GLITCH,BROWNOUT
// "BOOT reset=<R> wd=0x<hex> pm=0x<hex>[ etapa=<n>:<NOMBRE> falla=<0|1>]\n"; -1 si no entra.
inline int formatBoot(char* out, size_t size, int reason, uint32_t wdReason,
                      uint32_t powmanReason, bool stageValid, uint8_t stage, bool fault);
#if defined(ARDUINO_ARCH_RP2040)
inline void mark(uint8_t s) { watchdog_hw->scratch[0] = encode(s); } // <hardware/structs/watchdog.h>
inline uint32_t readScratch() { return watchdog_hw->scratch[0]; }
#else
inline void mark(uint8_t) {}
#endif
}
```

Datos de la revisión:
- `scratch[0]` está libre: pico-sdk usa `scratch[4]` (`watchdog.c:84,102-108`) y crt0
  usa 2-7 solo con `PICO_NO_FLASH`.
- Sobrevive al reset por watchdog (el bloque WATCHDOG no está en `PSM_WDSEL`).
- `getResetReason` en RP2350 figura como "untested" (`RP2040Support.h:614`). Por eso
  BOOT incluye también los registros crudos: `wd` = `watchdog_hw->reason` y
  `pm` = `powman_hw->chip_reset`. Codex verifica el nombre exacto de la estructura
  POWMAN en el SDK del core.

`mis_defines.h` (junto a `CONTROL_LOOP_MS`):
```cpp
#define YOKE_WDT_MS        250   // reinicio por hardware si loop() no corre en 250 ms
#define YOKE_WDT_FLASH_MS  3000  // durante EEPROM.commit (borrado de sector, max ~400 ms)
#define YOKE_USB_GUARD     1     // serializar CDC con __usb_mutex (correccion H5)
#define YOKE_TEST_HANG     0     // SOLO ENSAYO: 0=no, 1=for(;;), 2=noInterrupts+for(;;), 3=__builtin_trap
#define YOKE_TEST_HANG_MS  60000 // instante del ensayo de cuelgue
#define YOKE_USB_STRESS    0     // SOLO ENSAYO: 1 = escribir linea corta en cada ciclo de 2 ms
static_assert(YOKE_WDT_MS >= 50*CONTROL_LOOP_MS, "WDT demasiado corto para el lazo");
```

Cambios en `MiYoke_PD.ino`:

1. **Setup, motores primero.** Primeras líneas de `setup()`: `rollMotor.begin(); pitchMotor.begin();`.
   Se mueven desde `.ino:120-121`, para que EN quede en bajo antes de USB, LCD y
   entradas.
2. **Setup, motivo del reinicio.** Leer y guardar en globales:
   ```cpp
   bootReason=rp2040.getResetReason();
   bootWd=watchdog_hw->reason;
   bootPm=<POWMAN chip_reset>;
   bootStageValid=YokeHealth::decode(YokeHealth::readScratch(),bootStage,bootFault);
   YokeHealth::mark(YokeHealth::Idle);
   ```
3. **Setup, watchdog.** Última línea de `setup()`: `rp2040.wdt_begin(YOKE_WDT_MS);`.
4. **Alimentar el watchdog.** Primera línea de `loop()`, antes del retorno temprano de
   2 ms: `rp2040.wdt_reset();`.
5. **Marcas de etapa.** `YokeHealth::mark(...)` antes de cada bloque:
   - PotRead, SerialRx, Calibration, FlashSave, Effects, Motion, Force, Motor;
   - ModelLog, DebugLog, Lcd;
   - SerialTx dentro de cada envío;
   - Idle al final de `loop()`.
6. **Guardado en flash.** En los dos bloques de guardado:
   `rp2040.wdt_begin(YOKE_WDT_FLASH_MS); calibration.save(); rp2040.wdt_begin(YOKE_WDT_MS);`.
   Llamar de nuevo a `wdt_begin` es seguro: `watchdog.c:47-75` deshabilita, recarga y
   rehabilita.
7. **HardFault.** Manejador fuerte:
   `extern "C" void isr_hardfault(void) { watchdog_hw->scratch[0] |= YokeHealth::kFaultBit; for(;;){} }`.
   - El del SDK es débil y solo ejecuta `bkpt` (`crt0.S:200-205,349`).
   - No llamar a `watchdog_reboot`: escribe `scratch[4]` y el motivo se informaría
     como SOFT.
   - El watchdog reinicia la Pico a los 250 ms.
8. **Diagnóstico de tiempos.** Dos máximos desde el arranque, que se agregan a la línea
   del modelo como ` lp=%lu gap=%lu`:
   - `loopWorstUs`: duración del cuerpo de `loop()`;
   - `gapWorstUs`: `nowUs-lastControlLoopUs`, medido antes de actualizarlo.
9. **Línea BOOT.** Una vez por arranque, cuando `simTelemetry.rawAvailable && dtr &&
   availableForWrite()>=n`:
   - escribir `"\n"` + `formatBoot(...)` bajo `UsbLock`;
   - se envía después de la primera trama válida, así el handshake no la consume.
10. **Ensayos** (código compilado solo si `YOKE_TEST_HANG>0` o `YOKE_USB_STRESS>0`):
    - Cuelgue: si `millis()>=YOKE_TEST_HANG_MS`, todavía no ocurrió en este arranque y
      `bootReason!=WDT_RESET`, ejecutar `mark(TestHang)` y la variante elegida.
    - Estrés: en cada ciclo de 2 ms, bajo `UsbLock` (salvo con `YOKE_USB_GUARD 0`),
      escribir `"S\n"` si `availableForWrite()>=2`.

## 4. Riesgos y mitigaciones

- **Reinicio falso del watchdog durante el guardado en flash:** se usa
  `YOKE_WDT_FLASH_MS` alrededor de `save()`.
- **Reconexión falsa del puente durante el guardado en flash:** la ventana de
  1000 ms de `LinkMonitor` lo evita (prueba C).
- **Bloqueo por `UsbLock`:** solo se toma en el hilo principal, sin esperas internas.
  La IRQ usa `try_enter`, así que no hay espera circular.
- **`yield()` propio:** el enlazador debe aceptar la definición fuerte. Codex confirma
  con grep que ninguna librería enlazada defina otro `yield()` fuerte para rp2040.
- **Fuerza al reiniciar:** el watchdog reinicia IO_BANK0, PADS y PWM, y EN queda en
  bajo en la primera línea de `setup()`. Durante el reinicio los pines solo tienen el
  pull-down interno, y la errata RP2350-E9 desaconseja confiar en él.
  - **Requisito de hardware antes de conectar motores:** pull-down externo
    ≤ 8,2 kΩ en EN (GPIO 10 y 11).
  - Verificar EN con osciloscopio durante la prueba B (hipótesis a validar).
- **Reconexión que bloquea el lazo del puente:** cada intento dura 600 ms como máximo,
  respeta `quit` y se repite cada 500 ms.
- **H3 (USB trabado con `loop()` vivo):** la reconexión no la resuelve. Quedaría
  registrada como reintentos sin éxito y se trataría con evidencia en otra tarea.

## 5. Pruebas

### Escritorio (`probar.ps1`)

Parte 1, archivo nuevo `tests/link_checks.h`, incluido desde `tests/simconnect_checks.cpp`:

- **`classifyWrite`:**
  - `(true,113,113,0)` → Ok
  - `(true,40,113,0)` → Timeout
  - `(false,0,113,121)` → Timeout
  - `(false,40,113,121)` → Timeout
  - `(false,0,113,31)` → DeviceError
  - `(false,0,113,22)` → DeviceError
  - `(false,0,113,995)` → DeviceError
- **`LinkMonitor`:**
  - 3 Timeout en t=0, 20 y 40 → false (no pasó la ventana).
  - Timeout en t=1000 → true.
  - 50 Timeout en 0..400 ms (borrado de flash) → siempre false.
  - Ok reinicia el contador y la ventana.
  - DeviceError → true en la primera.
- **`Reconnector`:**
  - `onLost(100)` → `shouldAttempt(100)` es true; `onAttemptFailed(100)` →
    `shouldAttempt(599)` false y `shouldAttempt(600)` true.
  - `onReopened(1000)` → `armAllowed(1499)` false y `armAllowed(1500)` true.
  - `onLost` repetido sin reabrir no incrementa `reconnections`.
  - `shouldLogRetry` limita a una vez cada 5 s.
- **`BootLineScanner`:**
  - línea en un solo bloque;
  - línea partida en 3 bloques;
  - fin de línea `\r\n`;
  - una línea que no empieza con `BOOT ` se ignora;
  - una línea de más de 159 caracteres se descarta y la siguiente válida se reporta.

Parte 2, archivo nuevo `tests/health_checks.h` (mismo punto de inclusión):

- `decode(encode(s))` para todas las etapas.
- Bit de falla: `encode(4)|0x80` → etapa 4 y `fault=true`.
- Un valor sin la marca → false.
- `formatBoot(..., 4 /*WDT*/, 0x1, 0x0, true, 4, false)` →
  `"BOOT reset=WDT wd=0x1 pm=0x0 etapa=4:EFECTOS falla=0\n"`.
- Encendido sin etapa → `"BOOT reset=ENCENDIDO wd=0x0 pm=0x...\n"` (sin `etapa`).
- Buffer chico → -1.

Compilación:
- Parte 1: `probar.ps1 -Bridge`.
- Parte 2: `probar.ps1 -Pico -Potentiometers -Encoders`.

### Hardware (usuario, después de la parte 2)

- **A.** Desenchufar el USB de la Pico 5 s durante el vuelo y volver a enchufarlo.
  - Debe aparecer `ERROR enlace`, después `Reintentando`, y "Enlace USB recuperado"
    en 5 s o menos.
  - El LCD se reinicializa y la simulación sigue sin reiniciar el lanzador.
  - Esta prueba ya puede hacerse después de la parte 1, sin cambiar el firmware.
- **B.** Firmware con `YOKE_TEST_HANG 1`, después con `2` y después con `3`.
  - Esperado: `Pico: BOOT reset=WDT ... etapa=15:PRUEBA` (con `falla=1` en la
    variante 3) y recuperación automática.
  - Al terminar, volver a `YOKE_TEST_HANG 0`.
- **C.** (Síntoma de §1.5.) Con el simulador en pausa, entrar en calibración, cambiar
  un efecto (activar o desactivar, para que el valor guardado cambie) y guardar.
  Repetir 10 veces alternando efectos, también en la sección de potenciómetros.
  - El puente no debe cerrarse y no debe aparecer ninguna `ERROR enlace` ni ninguna
    `BOOT` nueva.
  - Puede haber vencimientos aislados sin mensaje.
  - Esta prueba ya puede hacerse después de la parte 1.
- **D.** Sesión normal de 30 min o más, en tierra y en vuelo, con el firmware final.
  No debe aparecer ninguna `ERROR enlace` ni ninguna `BOOT reset=WDT`. Si aparecen,
  conservar la consola y `*.anterior.*`.
- **E. Validación de H5:** `YOKE_USB_STRESS 1`, con el simulador en vuelo.
  - Primero `YOKE_USB_GUARD 0`, 30 min o hasta que aparezca una línea BOOT.
  - Después `YOKE_USB_GUARD 1`, 30 min.
  - H5 queda confirmada si con 0 aparece `BOOT reset=WDT` y con 1 no.
  - Si con 0 no aparece, H5 no queda confirmada: se conserva la corrección porque
    sigue el criterio del core, y se decide con la sesión D.
  - Al terminar, volver a `YOKE_USB_STRESS 0` y `YOKE_USB_GUARD 1`.

## 6. Documentación afectada

Reemplazos exactos (Codex busca el TEXTO ACTUAL y lo reemplaza por el TEXTO NUEVO).
Todos se aplican en la **parte 2**.

### 6.1 `LEEME md/LEEME-LANZADOR.md`

TEXTO ACTUAL (3 líneas):
```
La deteccion se realiza en cada inicio; no se agrego reconexion automatica durante
una sesion activa del puente. Si se desenchufa la Pico durante el uso, terminar
la sesion del puente y volver a iniciar con la placa conectada.
```
TEXTO NUEVO:
```
La deteccion se realiza en cada inicio. Durante una sesion activa, si se pierde el
enlace USB con la Pico (desconexion, reinicio por watchdog o escritura vencida), el
puente no termina: registra el error con hora y codigo de Windows, reintenta abrir
el mismo COM cada 500 ms y, al recuperarlo, envia tramas desarmadas durante 500 ms
antes de volver a armar. Si se cierra el puente con el enlace sin recuperar, termina
con codigo 4.
```

### 6.2 `LEEME md/LEEME_SIMCONNECT.md`

TEXTO ACTUAL:
```
`bridge.log` y `telemetry.csv`, junto al EXE, se reemplazan en cada ejecucion.
```
TEXTO NUEVO:
```
`bridge.log`, `telemetry.csv` y `pico.log`, junto al EXE, se reemplazan en cada ejecucion; si el `pico.log` anterior no estaba vacio, la generacion previa se conserva como `*.anterior.*`.
```

### 6.3 `LEEME md/EFECTOS_SIMCONNECT.md`

TEXTO ACTUAL:
```
`bridge.log` se reemplazan en cada ejecución.
```
TEXTO NUEVO:
```
`bridge.log` se reemplazan en cada ejecución; la generación previa se conserva como `*.anterior.*` si su `pico.log` no estaba vacío.
```

### 6.4 `LEEME md/MODELO_TURBULENCIA.md`

TEXTO ACTUAL:
```
Se reemplaza en cada ejecución.
```
TEXTO NUEVO:
```
Se reemplaza en cada ejecución (la generación previa queda como `telemetry.anterior.csv`).
```

### 6.5 `LEEME md/CALCULO_PICO_SC8.md`

TEXTO ACTUAL:
```
- `pico.log`: diagnóstico recibido de la Pico. `SC8 t=... gr=... gp=... R=... P=... us=...` muestra marca, señales de suelo por 10000 antes de ganancias, PWM total aplicado y máximo tiempo observado de `Effects::update` en microsegundos. No incluye el resto del bucle. Se registra aproximadamente cada 100 ms y puede omitir picos.
- `bridge.log`: conexión y errores.

Los registros se reemplazan al iniciar el ejecutable: guardar los de cada prueba antes de otra ejecución.
```
TEXTO NUEVO:
```
- `pico.log`: diagnóstico recibido de la Pico. `SC8 t=... gr=... gp=... R=... P=... us=... lp=... gap=...` muestra marca, señales de suelo por 10000 antes de ganancias, PWM total aplicado, máximo tiempo observado de `Effects::update` (`us`), máxima duración del cuerpo de `loop()` (`lp`) y máximo intervalo entre ejecuciones del lazo (`gap`), en microsegundos desde el arranque. Se registra aproximadamente cada 100 ms y puede omitir picos. Una vez por arranque la Pico envía `BOOT reset=... wd=0x... pm=0x...`; tras un reinicio por watchdog agrega `etapa=n:NOMBRE` (última etapa del lazo) y `falla=1` si hubo HardFault.
- `bridge.log`: conexión, errores del enlace USB con hora local y código de Windows, reconexiones y las líneas `Pico: BOOT ...`.

Los registros se reemplazan al iniciar el ejecutable. Si el `pico.log` anterior no estaba vacío, la generación previa se conserva como `bridge.anterior.log`, `telemetry.anterior.csv` y `pico.anterior.log`.
```

### 6.6 `tesis/capitulos/01_arquitectura_sc8.tex`

Solo se reemplazan los fragmentos indicados; no leer el capítulo completo.

(a) Párrafo nuevo, después del párrafo que empieza con "Esta selección evita procesar".

TEXTO ACTUAL:
```
no reconstruye exactamente la planificación de Windows ni mide los motores.
```
TEXTO NUEVO:
```
no reconstruye exactamente la planificación de Windows ni mide los motores.

El enlace USB tiene dos niveles de supervisión. En la Pico, un watchdog por hardware de 250~ms reinicia la placa si el lazo deja de ejecutarse; antes de cada etapa del lazo se guarda un identificador en un registro que sobrevive al reinicio, y la placa lo informa al arrancar junto con el motivo del reinicio. Las llamadas USB del lazo principal se serializan con el mutex USB del núcleo, de la misma forma que la pila USB propia de arduino-pico, para evitar un interbloqueo con la tarea USB que se ejecuta en interrupción. En la PC, una falla de escritura o lectura ya no termina el puente: tras tres vencimientos consecutivos durante al menos 1~s, o ante un error de dispositivo, el puente cierra el puerto, reintenta abrirlo cada 500~ms y, al recuperarlo, envía tramas desarmadas durante 500~ms antes de volver a armar.
```

(b) TEXTO ACTUAL:
```
La línea \code{SC8 t=... gr=... gp=... R=... P=... us=...} muestra marca de muestra, señales de suelo multiplicadas por 10000 antes de ganancias, PWM total aplicado a cada eje y máximo tiempo observado de \code{Effects::update} en microsegundos.
```
TEXTO NUEVO:
```
La línea \code{SC8 t=... gr=... gp=... R=... P=... us=... lp=... gap=...} muestra marca de muestra, señales de suelo multiplicadas por 10000 antes de ganancias, PWM total aplicado a cada eje, máximo tiempo observado de \code{Effects::update} (\code{us}), máxima duración del cuerpo del lazo (\code{lp}) y máximo intervalo entre ejecuciones del lazo (\code{gap}), en microsegundos desde el arranque.
```

(c) TEXTO ACTUAL:
```
El tiempo medido no incluye el resto del bucle ni demuestra por sí solo que se cumpla siempre el período de 2 ms.
```
TEXTO NUEVO:
```
Estos máximos no demuestran por sí solos que se cumpla siempre el período de 2 ms, porque la línea puede omitir picos y no hay medición externa.
```

No se modifican (siguen siendo correctas): `AJUSTE_FUERZAS_Y_MOVIMIENTO.md:133`,
`tesis/capitulos/04_turbulencias.tex:365`, ni la descripción de `sent`/`armed`
(`CALCULO_PICO_SC8.md:26`, `01_arquitectura_sc8.tex:33`), que se conserva gracias a
`Record(wrote, wrote&&armed)`. `MODELO_TURBULENCIA.md:181` ya estaba
desactualizada antes de este cambio y queda fuera de alcance.
