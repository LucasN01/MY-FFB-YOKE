# Diseño: diagnóstico del HardFault y motores seguros durante una falla

Rama: `mejora/diagnostico-hardfault` (desde `master`, V2.1). Autor: Claude, 2026-10-13.
Revisado antes de entregar por Embedded Firmware Engineer y Master Plan Architect. Las rutas del core son relativas a
`%LOCALAPPDATA%\Arduino15\packages\rp2040\hardware\rp2040\6.1.1`.

## 1. Evidencia y objetivo
- Ensayo CICLOS del 13/10: `BOOT reset=WDT wd=0x1 pm=0x10000 etapa=0:INACTIVO falla=1`
  (`ensayos/20261013-171626_ciclos/bridge.anterior.log`). Las cargas de UF2 del 12/10 dieron `reset=ENCENDIDO`.
- `isr_hardfault` (`MiYoke_PD.ino:27-30`) marca `kFaultBit` y queda en un bucle; el watchdog (250 ms) reinicia.
- El reinicio fue por el temporizador del watchdog, así que la falla ocurrió **después** de `rp2040.wdt_begin` (`MiYoke_PD.ino:200`).
- `INACTIVO` no localiza la falla:
  - las interrupciones (USB, encoders `PicoEncoder.h:14-15`) y `__loop()` → `yield()` → `tud_task()` corren casi
    siempre con la etapa en Idle, porque el cuerpo del lazo es corto frente a 2 ms;
  - falta saber **qué instrucción** y **en qué contexto** (hilo o interrupción).
- Durante la falla, EN queda en alto y el PWM en su último valor durante hasta 250 ms.

### 1.1 Hipótesis
- **H-a, `yield()` con el mutex USB sin inicializar: DESCARTADA** [Código]:
  - `__usb_mutex` se inicializa en `TinyUSB_Port_InitDevice` (`libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/Adafruit_TinyUSB_rp2040.cpp:79,99`),
    llamado desde `cores/rp2040/main.cpp:148`, antes de `setup()`;
  - `delay()` es `sleep_ms` y no llama a `yield()` (`cores/rp2040/delay.cpp:35-41`).
- **H-b, desborde de stack:**
  - el stack del núcleo 0 mide 4 KB, en SCRATCH_Y (`memmap_default.ld`), y MSPLIM=0 (`crt0.S`), así que no hay protección;
  - un desborde puede corromper buffers USB o el heap y fallar después dentro de `tud_task` (con la etapa en Idle).
- **H-c, falla dentro de una interrupción o de la pila USB** (TinyUSB, IRQ).

### 1.2 Objetivos
1. Guardar en la falla PC, LR, CFSR y el número de excepción activa, e informarlos en `BOOT`.
2. Medir el mínimo de stack libre (H-b) e informarlo al puente.
3. Traducir PC/LR a función y línea (`FALLA.ps1`).
4. **Seguridad:** en el manejador, apagar EN y llevar los 4 PWM a 0 V como GPIO, antes de cualquier otra cosa.
5. Leer `BOOT` sin simulador (`ARRANQUE.ps1`) y validar todo de punta a punta con un HardFault provocado.
- **No se corrige la causa:** se busca evidencia.

## 2. Verificaciones previas de Codex (evidencia archivo:línea en HANDOFF; no detienen la tarea)
1. Que `scratch[1..3]` no los use el core ni el SDK. Se espera solo `scratch[4..7]` en `hardware_watchdog/watchdog.c:84,102-108`.
2. Que `isr_hardfault` sea un símbolo débil del SDK (`crt0.S`) y que `Build-Pico2.ps1` compile para ARM.
3. Los símbolos del linker del stack del núcleo 0 (`__StackLimit`, `__StackBottom` o `__StackTop`, en `memmap_default.ld` del
   RP2350) con sus nombres exactos, para §3.3.
4. Que `io_bank0_hw->io[p].ctrl`, `sio_hw->gpio_clr` y `sio_hw->gpio_oe_set` existan con esos nombres para RP2350.

## 3. Firmware

### 3.1 Manejador (`MiYoke_PD.ino`, reemplaza `:26-30`; bajo `ARDUINO_ARCH_RP2040 && __ARM_ARCH_8M_MAIN__`; si no, queda el actual)
```cpp
extern "C" __attribute__((used, noinline, noreturn)) void __not_in_flash_func(hardfaultCapture)(uint32_t* frame) {
    // 1) Seguridad primero: EN y luego los 4 PWM como GPIO SIO en 0.
    const uint32_t en  = (1u<<ROLL_EN_PIN) | (1u<<PITCH_EN_PIN);
    const uint32_t pwm = (1u<<ROLL_PWM_LEFT) | (1u<<ROLL_PWM_RIGHT) | (1u<<PITCH_PWM_LEFT) | (1u<<PITCH_PWM_RIGHT);
    sio_hw->gpio_clr = en;   sio_hw->gpio_oe_set = en;
    sio_hw->gpio_clr = pwm;  sio_hw->gpio_oe_set = pwm;
    io_bank0_hw->io[ROLL_PWM_LEFT].ctrl = GPIO_FUNC_SIO;  io_bank0_hw->io[ROLL_PWM_RIGHT].ctrl = GPIO_FUNC_SIO;
    io_bank0_hw->io[PITCH_PWM_LEFT].ctrl = GPIO_FUNC_SIO; io_bank0_hw->io[PITCH_PWM_RIGHT].ctrl = GPIO_FUNC_SIO;
    // 2) Diagnóstico. Marco estándar: r0,r1,r2,r3,r12,lr,pc,xpsr (con FPU, S0-S15 quedan por encima: PC sigue en [6]).
    watchdog_hw->scratch[1] = frame[6];                              // PC
    watchdog_hw->scratch[2] = frame[5];                              // LR
    watchdog_hw->scratch[3] = *(volatile uint32_t*)0xE000ED28u;      // CFSR
    const uint32_t exc = frame[7] & 0xFFu;                           // excepción activa al fallar: 0=hilo, >=16 IRQ
    watchdog_hw->scratch[0] = (watchdog_hw->scratch[0] & 0xFFFF00FFu) | (exc<<8) | YokeHealth::kFaultBit;
    for(;;){}
}
extern "C" __attribute__((naked)) void __not_in_flash_func(isr_hardfault)(void) {
    __asm volatile("tst lr, #4\n ite eq\n mrseq r0, msp\n mrsne r0, psp\n b hardfaultCapture\n");
}
```
- Los pines vienen de `mis_defines.h:132-137` (variante Pico). El orden es intencional: primero EN, después PWM.
- El cambio no modifica ninguna constante de seguridad (regla 7): solo apaga.
- Limitación: con el stack destruido, el propio manejador puede fallar (lockup). El watchdog reinicia igual, y cubren el
  pull-down de EN pendiente y el `begin()` del arranque.

### 3.2 Arranque y línea `BOOT` (`LoopSupervisor.h`, `MiYoke_PD.ino` `setup()`)
1. `decode()` (`LoopSupervisor.h`) pasa a validar `(v & 0xFFFF0000u) == kMagic`. La etapa queda en los bits 0-6, la falla en
   el bit 7 y la excepción en los bits 8-15. `encode(stage)` no cambia: escribe 0 en los bits 8-15. Agregar
   `inline uint8_t decodeException(uint32_t v){ return uint8_t((v>>8)&0xFF); }`.
2. En `setup()`, junto a la lectura de `scratch[0]` (`:174`):
   - si `bootFault`, leer `bootPc=scratch[1]`, `bootLr=scratch[2]`, `bootCfsr=scratch[3]` y
     `bootExc=decodeException(scratch[0])`;
   - después escribir 0 en `scratch[1..3]`.
3. `formatBoot(...)` agrega los parámetros `uint8_t exc, uint32_t pc, uint32_t lr, uint32_t cfsr`.
   - Solo si `stageValid && fault`, **antes** del `\n` final se agrega ` exc=%u pc=0x%08lx lr=0x%08lx cfsr=0x%08lx`.
   - Si el PC no está en flash (0x10000000-0x10400000) ni en SRAM (0x20000000-0x20082000), agregar además ` pc_invalido=1`.
   - Sin falla, la línea queda **idéntica** a la de hoy.
4. `char bootLine[160]` (`MiYoke_PD.ino:353`). `BootLineScanner` admite 159 caracteres (`SerialLink.h:51-67`).

### 3.3 Stack libre (H-b)
- En `LoopSupervisor.h`, funciones puras sobre un arreglo, para poder probarlas en la PC:
  - `void paintStack(uint32_t* bottom, uint32_t* current, uint32_t pattern)` pinta de `bottom` hasta 64 palabras antes de `current`;
  - `uint32_t freeStackBytes(const uint32_t* bottom, const uint32_t* top, uint32_t pattern)` cuenta palabras con el patrón
    desde `bottom` y devuelve bytes.
- En `setup()`, al principio, después de `rollMotor.begin(); pitchMotor.begin();`: pintar desde el símbolo inferior del
  stack (§2.3) hasta el SP actual menos 256 B, con el patrón `0xA5A5A5A5`.
- En el bloque `ModelLog`, una vez por segundo: `freeStackBytes(...)`. Si el valor es menor que el último informado o
  pasaron 60 s, enviar `"\nS,<bytes>\n"`, con la misma prioridad y condiciones que `L` (después de `W`).
- Puente: `PicoCsv.h` agrega `bool stackLine(const char*, uint32_t& bytes)` (exactamente `S,` más un entero sin signo).
  `main.cpp` registra cada valor nuevo como `[hh:mm:ss] Pico: stack libre minimo <n> B`. La línea `Fin:` agrega
  `stack_libre_min=<n>`, o `-` si no llegó ningún valor. El filtro del splitter acepta `S,`.

### 3.4 HardFault de prueba (para validar todo)
- `mis_defines.h:168-169`: poner `#ifndef` a `YOKE_TEST_HANG` y `YOKE_TEST_HANG_MS`, con los mismos valores por defecto
  (0 y 60000).
- `Build-Pico2.ps1`: nuevo parámetro `[switch]$PruebaFalla`. Agrega `-DYOKE_TEST_HANG=3 -DYOKE_TEST_HANG_MS=20000` al mismo
  `build.extra_flags` y compila en `build-pico2-prueba-falla\`, con su propio `build-path` y la variante de potenciómetros
  (como `-Potentiometers`).
- `probar.ps1`: nuevo parámetro `-PruebaFalla`, que compila esa variante y la informa como `Pico 2 (prueba falla)`. No
  entra en `-Todo`.
- Comportamiento esperado: a los 20 s, `__builtin_trap` (`MiYoke_PD.ino:510`) produce el HardFault. El watchdog reinicia y,
  como `bootReason==WDT`, la prueba no se repite (`:503`). El `BOOT` siguiente dice `etapa=15:PRUEBA falla=1 exc=0 pc=...`.

## 4. Herramientas de PC

### 4.1 `FALLA.ps1`
`powershell -ExecutionPolicy Bypass -File FALLA.ps1 -Pc 0x... [-Lr 0x...] -Elf <ruta .elf>` (`-Elf` obligatorio).
- Busca `addr2line` con el patrón `%LOCALAPPDATA%\Arduino15\packages\rp2040\tools\pqt-gcc\*\bin\arm-none-eabi-addr2line.exe`
  y usa la carpeta más nueva. Si no lo encuentra, muestra un mensaje y sale con 1.
- Convierte cada dirección con `[Convert]::ToUInt64($x.Replace('0x',''),16)`; al LR le aplica `-band 0xFFFFFFFE`.
- Si una dirección está fuera de flash (0x10000000-0x10400000) y de SRAM (0x20000000-0x20082000), imprime
  `<PC|LR> 0x... no es direccion de codigo`.
- Ejecuta `addr2line -f -C -e <elf> <dir>` e imprime `PC 0x... -> <funcion> (<archivo>:<linea>)`. Para el LR agrega
  `(LR = linea siguiente a la llamada)`.
- Muestra la fecha del `.elf` y la advertencia `El .elf debe ser de la misma compilacion que el UF2 cargado`.
- Sale con 0 si traduce y con 1 si hay error.

### 4.2 `ARRANQUE.ps1` + `ARRANQUE.cmd` (`pause` siempre)
`powershell -ExecutionPolicy Bypass -File ARRANQUE.ps1 -Entrada <telemetry.csv> [-Puerto COMn] [-Elf <ruta .elf>]`
1. `-Entrada` obligatorio; si no existe, mensaje y salida 1. `ARRANQUE.cmd` pasa `tmp\vuelo_corto\telemetry.csv` y
   avisa si no existe.
2. Igual que `BANCO.ps1:8-27`: falla si corren Prepar3D o el puente, y detecta el COM.
3. Crea `ensayos\AAAAMMDD-HHMMSS_arranque\entrada.csv` con la cabecera literal y hasta 300 filas con `sent=1`. Si hay
   menos, usa todas; si hay 0, mensaje y salida 1.
4. Ejecuta el puente `--port COMn --replay entrada.csv`, **sin** `--enable`.
   - **No** corta con `throw` si el código del puente no es 0: copia igual `bridge.log` y `pico.log` e informa el
     código (2 = CSV o argumentos, 3 = firmware no identificado, 4 = enlace perdido).
5. Busca con la regex `Pico: (BOOT .*)` solo en el `bridge.log` actual.
   - Sin coincidencia: `Sin linea BOOT: la Pico no se reinicio desde la ultima conexion (desenchufar y enchufar, o cargar el UF2, y repetir)`.
   - Con coincidencia, la imprime. Si tiene `falla=1` y `pc=`:
     - con `-Elf`, llama a `FALLA.ps1` y guarda su salida en `falla.txt`;
     - sin `-Elf`, imprime el comando de `FALLA.ps1` listo para copiar.
   - Imprime también la última línea `stack libre minimo` si existe.
6. Sale con 0 si hay `BOOT` sin falla, con 2 si hay falla y con 1 si no hubo `BOOT` o hubo un error.

## 5. Protocolo del usuario (motores apagados; anotar cómo se cargó cada UF2)
| Paso | Acción | Qué se espera |
| --- | --- | --- |
| V1 | Cargar `build-pico2-prueba-falla\MiYoke_PD.ino.uf2` (BOOTSEL). **Esperar 30 s.** Ejecutar `ARRANQUE.cmd` | `BOOT reset=WDT ... etapa=15:PRUEBA falla=1 exc=0 pc=...`; `FALLA.ps1` con `-Elf build-pico2-prueba-falla\MiYoke_PD.ino.elf` lleva el PC a `loop` |
| R1 | Cargar el UF2 normal `build-pico2-potentiometers\MiYoke_PD.ino.uf2`. Esperar 30 s. `ARRANQUE.cmd` | `ENCENDIDO` (o `WDT falla=1`: se captura) |
| R2 | Desenchufar y enchufar la Pico, esperar 30 s, `ARRANQUE.cmd`; 3 veces | `ENCENDIDO` |
| R3 | Volver a cargar el **mismo** UF2 normal, esperar 30 s, `ARRANQUE.cmd`; 2 veces | `ENCENDIDO`; si aparece `falla=1`, está ligada al primer arranque tras la carga |
| R4 | Uso normal en los días siguientes | Cada vuelo deja su `BOOT` y su `stack libre minimo` en `bridge.log` |

- **Decisión:**
  - con `falla=1` en R1-R4, `exc` (hilo o IRQ), PC y LR con `FALLA.ps1` indican dónde se produjo, y la corrección se
    diseña en otra tarea;
  - si el stack libre mínimo cae por debajo de 512 B, se toma como evidencia de H-b;
  - sin fallas, "no reproducido", y el diagnóstico queda activo.
- No recompilar entre la carga del UF2 y el uso de `FALLA.ps1`.
- Si `ARRANQUE.cmd` se ejecuta antes de los 20 s en V1, la falla ocurre con el puente conectado (código 4): esperar y
  repetir `ARRANQUE.cmd`.

## 6. Pruebas de escritorio
- `tests/health_checks.h`:
  - `formatBoot` sin falla: idéntico a hoy (`:22-29`, ajustando la llamada);
  - con falla: ` exc=0 pc=0x10001234 lr=0x10001235 cfsr=0x00008200` antes del `\n`;
  - PC fuera de rango → `pc_invalido=1`;
  - longitud máxima (`wd=0xffffffff pm=0xffffffff`, etapa más larga, todos los campos) < 159;
  - buffer chico → -1;
  - `decode` con bits 8-15 distintos de 0 y `decodeException`;
  - `encode` deja 0 en los bits 8-15.
- `tests/picolog_checks.h`:
  - `paintStack`/`freeStackBytes` sobre un arreglo (pintado parcial, todo usado, nada usado);
  - `stackLine` acepta `S,123`; rechaza `S,`, `S,-1`, `S,1,2`.
- El manejador (ensamblador ARM) no se prueba en PC: se valida con V1 en la Pico.

## 7. Criterios de aceptación
1. Verificaciones de §2 con evidencia.
2. `probar.ps1 -Pico -Potentiometers -Encoders -Bridge -PruebaFalla` todo OK; suite con §6.
3. V1: `BOOT` con `etapa=15:PRUEBA falla=1 exc=0` y PC dentro de `loop` según `FALLA.ps1`. Opcional, con multímetro: EN en
   0 V durante los 250 ms del bucle de falla.
4. R1-R3 hechos y anotados en HANDOFF, junto con el stack libre mínimo informado.

## 8. Documentación afectada
- `LEEME md/CALCULO_PICO_SC8.md`, "Registros". Reemplazo exacto:
  - TEXTO ACTUAL:
    ```
    tras un reinicio por watchdog agrega `etapa=n:NOMBRE` (última etapa del lazo) y `falla=1` si hubo HardFault.
    ```
  - TEXTO NUEVO:
    ```
    tras un reinicio por watchdog agrega `etapa=n:NOMBRE` (última etapa del lazo) y `falla=1` si hubo HardFault; en ese caso agrega `exc=` (0 = código normal, 16 o más = interrupción), `pc=0x...` (instrucción que falló), `lr=0x...` y `cfsr=0x...` (causa). `FALLA.ps1 -Pc ... -Lr ... -Elf ...` las traduce a función y línea con el `.elf` de la misma compilación, y `ARRANQUE.cmd` lee la línea `BOOT` sin simulador. Durante un HardFault el firmware pone EN y los cuatro PWM en 0 V antes de esperar el reinicio del watchdog. La Pico informa además el mínimo de stack libre (`Pico: stack libre minimo ... B` en `bridge.log`, `stack_libre_min=` en la línea final).
    ```
- `LEEME md/ENSAYOS.md`: agregar al final la sección `## Arranque y fallas (ARRANQUE.cmd)` con la tabla de §5 y su
  párrafo "Decisión", copiados tal cual.
- `LEEME md/LEEME_PICO2.md`: en el bloque que empieza con `Tambien se puede compilar desde PowerShell` (línea 36), después del bloque de comandos, agregar una línea:
  `probar.ps1 -PruebaFalla compila build-pico2-prueba-falla (HardFault provocado a los 20 s, solo para validar el diagnostico; no usar para volar).`
- `probar.ps1`: agregar `-PruebaFalla` al comentario de ayuda (líneas 1-11).
- Tesis: **no afecta** (no describe `BOOT` ni el manejador de fallas; verificado con grep).
