\# HANDOFF entre agentes



\## Reglas

1\. Un solo agente edita a la vez. Cada tarea va en su propia rama: `git checkout -b tarea/<nombre>`.

2\. Al terminar, el agente completa la sección "Última entrega" y hace commit.

3\. El otro agente revisa el diff (`git diff master...HEAD`) sin editar, y anota hallazgos en "Revisión".

4\. Nada se integra a `master` sin que el usuario valide en hardware.

5\. Antes de afirmar algo del código, indicar la fuente: \[Grafo], \[INO] o \[Código], con archivo y línea. Las aristas INFERRED se tratan como hipótesis.

6\. Tras editar `MiYoke\_PD.ino`, ejecutar `graphify-sync.cmd`.

7\. No modificar constantes de seguridad (límites de PWM, `MOTOR\_PWM\_HARD\_LIMIT`, `SIMCONNECT\_EXTRA\_LIMIT`) sin aprobación explícita del usuario.

8\. Nunca usar `git add .` ni `git add -A`: la carpeta graphify-ino/ no debe versionarse. Agregar archivos por nombre.

9\. Un hallazgo de "posible bug" se verifica leyendo el código involucrado antes de proponer cambios. Informar la evidencia (archivo y línea), no solo la sospecha.

10\. Las reglas de versionado, documentacion, ahorro de tokens y pruebas estan en AGENTS.md (Codex) y CLAUDE.md (Claude); no se repiten aca.

11\. "Ultima entrega" y "Revision" contienen solo la tarea en curso: al empezar una tarea nueva se reemplazan (lo anterior queda en el historial de git).

\## Plan para Codex

Tarea: diagnóstico del HardFault, stack libre y motores seguros durante una falla (Claude, 2026-10-13).
Rama: `mejora/diagnostico-hardfault` (ya creada; contiene el diseño). Diseño: `LEEME md/DISENO_diagnostico_hardfault.md`.
Se divide en **2 partes**: firmware con manejador en ensamblador + compilación de prueba, y luego puente + scripts + documentación + versión. Cada parte se hace en una sesión distinta.

\### Precondiciones (ambas partes)
- `git checkout mejora/diagnostico-hardfault` y `git status`. Ignorar `graphify-ino/`, `tesis/main.tex`, `tesis/capitulos/11_maquinas_estado.tex` y `tesis/caracterizacion_motores_775.tex` (cambios del usuario).
- Orientarse con `graphify query/explain`; leer solo las líneas indicadas.
- No tocar constantes de seguridad (regla 7). No usar `git add .` ni `git add -A`.
- Pruebas solo con `probar.ps1`; en HANDOFF copiar solo las líneas OK/FALLO.
- Si el diseño contradice el código, detenerse y anotarlo en "Última entrega".

\### Parte 1 — Firmware, prueba de falla y pruebas (sin versión)
- Leer del diseño: §1, §2, §3 (completo) y §6.
- Código a leer: `MiYoke_PD.ino` (`:1-60`, `setup()`, el bloque `ModelLog` y el bloque `YOKE_TEST_HANG`), `LoopSupervisor.h` (completo), `mis_defines.h:125-175`, `Build-Pico2.ps1` (completo), `probar.ps1` (completo), `tests/health_checks.h` (completo), `tests/picolog_checks.h` (estilo), `SimConnectBridge/SerialLink.h:50-67`.
- Alcance:
  1. Verificaciones de §2, con evidencia en "Última entrega".
  2. `MiYoke_PD.ino`: §3.1, §3.2 y §3.3 (la parte del firmware: pintado y envío de la línea `S`).
  3. `LoopSupervisor.h`: `decode` con máscara 0xFFFF0000, `decodeException`, `formatBoot` con los campos nuevos, `paintStack` y `freeStackBytes`.
  4. `mis_defines.h`: `#ifndef` en `YOKE_TEST_HANG` y `YOKE_TEST_HANG_MS` (§3.4).
  5. `Build-Pico2.ps1` (`-PruebaFalla`) y `probar.ps1` (`-PruebaFalla` y su ayuda), según §3.4.
  6. Pruebas de §6 en `tests/health_checks.h`, y las de `paintStack`/`freeStackBytes` en `tests/picolog_checks.h`.
  7. `graphify-sync.cmd` y `graphify update .`.
- Pruebas:
  - `probar.ps1 -Pico -Potentiometers -Encoders -PruebaFalla`: todo OK; anotar los tamaños frente a 105 820 B;
  - verificar en `build-pico2-prueba-falla\MiYoke_PD.ino.map` (o con `arm-none-eabi-nm`) que `hardfaultCapture` e `isr_hardfault` están en RAM (sección `.data`/`.time_critical`) y que `isr_hardfault` es el símbolo fuerte. Anotar las direcciones.
- Commit por nombre: `MiYoke_PD.ino LoopSupervisor.h mis_defines.h Build-Pico2.ps1 probar.ps1 tests/health_checks.h tests/picolog_checks.h HANDOFF.md`.
  Mensaje: "Pico: HardFault con PC/LR/CFSR y excepcion, motores a 0 V en la falla, stack libre y compilacion de prueba".
- **No** ejecutar `nueva-version.ps1`. Completar "Última entrega".

\### Parte 2 — Puente, scripts, documentación y versión
- Precondición: la Parte 1 está commiteada y revisada por Claude.
- Leer del diseño: §3.3 (la parte del puente), §4, §5 y §8.
- Código a leer: `SimConnectBridge/PicoCsv.h` (completo), `SimConnectBridge/main.cpp` (`PollPort` y la salida final), `BANCO.ps1` (completo), `ENSAYO.cmd`, `tests/picolog_checks.h` (las pruebas de `slowLine`, como modelo).
- Alcance:
  1. `PicoCsv.h`: `stackLine`. `main.cpp`: filtro del splitter con `S,`, el registro en `bridge.log` y `stack_libre_min=` en la línea `Fin:`.
  2. Prueba de `stackLine` en `tests/picolog_checks.h`.
  3. Crear `FALLA.ps1`, `ARRANQUE.ps1` y `ARRANQUE.cmd` (§4).
  4. Documentación de §8: el reemplazo exacto en `LEEME md/CALCULO_PICO_SC8.md`, la sección nueva en `LEEME md/ENSAYOS.md` y la línea en `LEEME md/LEEME_PICO2.md`. Si un TEXTO ACTUAL no aparece, anotarlo y seguir.
  5. Ajustes de la revisión de la Parte 1:
     - en `MiYoke_PD.ino`, en la línea del flanco de subida de `dtr` (`if(dtr && !lastDtr) {...}`), agregar `lastStackBytes=0xFFFFFFFFu;`, para que una reconexión reciba la línea `S` enseguida (si hace falta, mover la declaración `static` de `lastStackBytes` antes de esa línea);
     - agregar `build-pico2-prueba-falla/` a `.gitignore`;
     - ejecutar `graphify-sync.cmd`.
  6. `graphify update .`.
- Pruebas:
  - `probar.ps1 -Pico -Potentiometers -Encoders -Bridge -PruebaFalla`: todo OK;
  - sintaxis de `FALLA.ps1` y `ARRANQUE.ps1`;
  - `FALLA.ps1 -Pc 0x<direccion de loop tomada del .map de build-pico2-prueba-falla> -Elf build-pico2-prueba-falla\MiYoke_PD.ino.elf` debe imprimir `loop`; anotar la salida;
  - `tests\bin\comparar_ensayo.exe tmp\vuelo_corto`: `RESULTADO: OK`.
- Commit por nombre: `MiYoke_PD.ino .gitignore SimConnectBridge/PicoCsv.h SimConnectBridge/main.cpp tests/picolog_checks.h FALLA.ps1 ARRANQUE.ps1 ARRANQUE.cmd "LEEME md/CALCULO_PICO_SC8.md" "LEEME md/ENSAYOS.md" "LEEME md/LEEME_PICO2.md" HANDOFF.md`.
  Mensaje: "Puente y scripts: stack libre, FALLA.ps1 y ARRANQUE para diagnosticar HardFault".
- Versión: `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Diagnostico de HardFault con PC y excepcion, motores a 0 V en la falla y stack libre"`. Verificar `Version Vx.y creada` y el `ACTUALIZACION_Vx_y.txt`. Si los cambios del usuario en `tesis/` lo impiden, usar el checkout temporal limpio, como en las tareas anteriores, y anotarlo.
- "Última entrega": las líneas OK/FALLO, la salida de `FALLA.ps1`, la versión y los desvíos.
- Luego: revisión de Claude y protocolo V1, R1-R3 del usuario (§5). No integrar a `master`.

\## Última entrega

- Parte 1 completada (Codex, 2026-10-13), rama `mejora/diagnostico-hardfault`. Commit de implementacion: `b6db398` (codigo y HANDOFF); esta anotacion del hash se guarda en un commit posterior de HANDOFF.
- Archivos modificados: `MiYoke_PD.ino`, `LoopSupervisor.h`, `mis_defines.h`, `Build-Pico2.ps1`, `probar.ps1`, `tests/health_checks.h`, `tests/picolog_checks.h` y `HANDOFF.md`.
- Implementado: apagado EN/PWM y captura PC/LR/CFSR/excepcion en HardFault ARM; lectura y limpieza de scratch al arrancar; BOOT ampliado solo con falla; pintado y medicion del stack, linea S despues de W; variante `-PruebaFalla` con potes y trap a los 20 s. Constantes de seguridad sin cambios.
- Documentacion: esta entrega actualizada. Los cambios de manuales quedan en Parte 2, como indica el plan; no se editaron .tex. Cambios del usuario en tesis/ y graphify-ino/ conservados y fuera del commit.
- Verificaciones de §2 [Codigo], rutas del core relativas a `%LOCALAPPDATA%/Arduino15/packages/rp2040/hardware/rp2040/6.1.1`:
  - `pico-sdk/src/rp2_common/hardware_watchdog/watchdog.c:84,102-108,126`: usa watchdog scratch[4..7], sin uso de [1..3] en cores/rp2040 ni en ese modulo. Salvedad: `pico-sdk/src/rp2_common/pico_crt0/crt0.S:434-452` usa scratch[2..3] para arranque RAM bajo `PICO_NO_FLASH`; esta variante usa flash (`build-pico2-prueba-falla/MiYoke_PD.ino.map:5828`, carga .data desde 0x1001cfb0), por lo que ese camino no aplica.
  - `pico-sdk/src/rp2_common/pico_crt0/crt0.S:200-201,349`: `decl_isr_bkpt isr_hardfault` declara el simbolo debil. `Build-Pico2.ps1:15` compila `arch=arm`; build.options.json de prueba confirma ese FQBN y `-DYOKE_USE_SIMULATION=2 -DYOKE_TEST_HANG=3 -DYOKE_TEST_HANG_MS=20000`.
  - `lib/rp2350/memmap_default.ld:305-310`: `__StackLimit` es el limite historico del heap; para pintar se usan `__StackBottom` y `__StackTop`. ELF/map de prueba: 0x20081800 y 0x20082000 (2048 B). Diferencia frente a la hipotesis de 4 KB del diseño: el core reserva 2 KB (`pico-sdk/src/rp2350/pico_platform/include/pico/platform.h:49-51`, PICO_STACK_SIZE=0x800). No se cambia el tamaño; se utilizan los limites reales, conforme a la verificacion no bloqueante de §2.
  - `pico-sdk/src/rp2350/hardware_structs/include/hardware/structs/io_bank0.h:36,62,180`: GPIO_FUNC_SIO=5 y `io[p].ctrl`; `.../sio.h:84,129`: `gpio_clr` y `gpio_oe_set`.
- [INO] `MiYoke_PD.ino:31-47`: manejadores; `:191-210`: pintado y captura de arranque; `:407-445`: medicion una vez por segundo y envio S con las condiciones de L, despues de W.
- [Codigo] `LoopSupervisor.h:45-59,92-115`: decode/excepcion, funciones puras de stack y BOOT. `SimConnectBridge/SerialLink.h:52-67` conserva su buffer de 160 B (hasta 159 caracteres).
- `graphify-sync.cmd` ejecutado y `graphify update .` actualizado (AST, sin API). La primera invocacion sin `./` no encontro el comando en PowerShell; se corrigio a `./graphify-sync.cmd` y se completo la sincronizacion.
- Pruebas: `powershell -ExecutionPolicy Bypass -File probar.ps1 -Pico -Potentiometers -Encoders -PruebaFalla` termino con codigo 0:

```text
OK suite simconnect_checks - PASS: 143603 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
OK comparar_ensayo
OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
OK Pico 2 (configurado) - Sketch uses 106364 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23016 bytes (4%) of dynamic memory, leaving 501272 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (encoders) - Sketch uses 106524 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23256 bytes (4%) of dynamic memory, leaving 501032 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (potenciometros) - Sketch uses 106364 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23016 bytes (4%) of dynamic memory, leaving 501272 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (prueba falla) - Sketch uses 106412 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23016 bytes (4%) of dynamic memory, leaving 501272 bytes for local variables. Maximum is 524288 bytes.
```

- Tamaños frente a 105820 B: configurado y potes +544 B; encoders +704 B; prueba falla +592 B.
- RAM verificada en `build-pico2-prueba-falla/MiYoke_PD.ino.map:5828-5837`: `.time_critical.hardfaultCapture` y `.time_critical.isr_hardfault` dentro de `.data`. `arm-none-eabi-nm -n`: `20000110 T hardfaultCapture`, `2000015c T isr_hardfault`; T mayuscula confirma simbolos fuertes. Para Parte 2: `100067dc T loop` en este ELF.
- `git diff --check`: sin errores de espacios. No se cargo firmware ni se valido el manejador en hardware; queda el protocolo V1 y R1-R3 del diseño.
- Version: no creada, conforme a Parte 1 (sin version); `nueva-version.ps1` queda para la Parte 2 final.
- Sigue: revision de Claude de Parte 1; despues, Parte 2 (puente, scripts, documentacion y version). No se integro a master.

\## Revisión

- Revisor: Claude (2026-10-13), con Code Reviewer. **Parte 1 (`b6db398`): APROBADA.**
  - **Manejador (revisado por Claude):**
    - primero EN, después los 4 PWM a GPIO SIO en 0;
    - PC/LR/CFSR y excepción en `scratch[1..3]` y en los bits 8-15 de `scratch[0]`;
    - en RAM (`.time_critical`) y como símbolos fuertes (`nm`: `hardfaultCapture` 0x20000110, `isr_hardfault` 0x2000015c).
  - **`BOOT`:** sin falla, la línea es idéntica; con falla, los campos van antes del `
` y el peor caso queda por debajo de 159.
  - **Stack:** el pintado deja 256 B bajo el SP; la medición es una vez por segundo y la línea `S` va después de `W`, una sola por ciclo.
  - **Compilación de prueba:** `-PruebaFalla` con un único `extra_flags` y fuera de `-Todo`.
  - **Pruebas:** discriminantes. Todo OK según Codex (+544 B).
  - **Dato nuevo [Código]:** el stack del núcleo 0 mide **2 KB** (`PICO_STACK_SIZE=0x800`, `__StackBottom`=0x20081800, `__StackTop`=0x20082000), no 4 KB como suponía el diseño. La hipótesis H-b gana peso; el valor de `stack libre minimo` en los ensayos lo dirá.
  - **Pendientes que van a la Parte 2, punto 5:**
    - reiniciar `lastStackBytes` en el flanco de `dtr`;
    - agregar `build-pico2-prueba-falla/` a `.gitignore`.
  - **Detalle aceptado:** con el manejador antiguo (no ARM) saldría `pc=0x00000000 pc_invalido=1`; esta placa compila para ARM.

\## Ensayos del usuario

- (vacío: tarea nueva. El ensayo CICLOS y el hallazgo del HardFault están en `master`, commit `bccd1b7`.)
