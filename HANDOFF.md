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

- Parte 2 completada (Codex, 2026-10-13), rama `mejora/diagnostico-hardfault`. Parte 1 aprobada por Claude en `9ea9978` (implementacion `b6db398`). Commit de Parte 2: `97d26e8` (codigo, documentacion y HANDOFF); esta anotacion final se guarda en un commit posterior de HANDOFF.
- Archivos modificados: `MiYoke_PD.ino`, `.gitignore`, `SimConnectBridge/PicoCsv.h`, `SimConnectBridge/main.cpp`, `tests/picolog_checks.h`, `FALLA.ps1`, `ARRANQUE.ps1`, `ARRANQUE.cmd`, `LEEME md/CALCULO_PICO_SC8.md`, `LEEME md/ENSAYOS.md`, `LEEME md/LEEME_PICO2.md` y `HANDOFF.md`.
- [Codigo] `SimConnectBridge/PicoCsv.h:48`: stackLine acepta S mas un entero uint32 sin signo, rechaza vacios, signo, campos extra y desbordes. `SimConnectBridge/main.cpp:227-238,406-410`: acepta S, registra cada valor distinto en bridge.log y conserva el minimo de toda la sesion para Fin (o - si no recibio valores).
- [INO] `MiYoke_PD.ino:420-425`: reinicia lastStackBytes en el flanco DTR para reenviar S tras reconexion. `.gitignore` excluye build-pico2-prueba-falla/. Constantes de seguridad sin cambios.
- Scripts: FALLA busca addr2line de la carpeta mas nueva, traduce PC/LR, limpia Thumb en LR, verifica rango y muestra fecha/advertencia del ELF. ARRANQUE conserva cabecera literal y hasta 300 filas sent=1 (UTF-8 sin BOM), ejecuta replay sin --enable, copia logs actuales aun con salida no cero, informa BOOT/stack y guarda falla.txt si se indica ELF. El wrapper avisa si falta el CSV y siempre hace pause.
- Documentacion actualizada en el mismo commit: reemplazo exacto de CALCULO_PICO_SC8.md; tabla y Decision de §5 copiadas a ENSAYOS.md; linea literal de compilacion de prueba en LEEME_PICO2.md. Todos los textos actuales aparecieron. La ayuda de probar.ps1 ya estaba actualizada por Parte 1. No afecta tesis; no se editaron ni compilaron .tex.
- `graphify-sync.cmd` y `graphify update .` completados (AST, sin API). Cambios del usuario en tesis/ y graphify-ino/ conservados y fuera del commit.
- Pruebas: `powershell -ExecutionPolicy Bypass -File probar.ps1 -Pico -Potentiometers -Encoders -Bridge -PruebaFalla`, codigo 0:

```text
OK suite simconnect_checks - PASS: 143619 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
OK comparar_ensayo
OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
OK Pico 2 (configurado) - Sketch uses 106380 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23016 bytes (4%) of dynamic memory, leaving 501272 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (encoders) - Sketch uses 106540 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23256 bytes (4%) of dynamic memory, leaving 501032 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (potenciometros) - Sketch uses 106380 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23016 bytes (4%) of dynamic memory, leaving 501272 bytes for local variables. Maximum is 524288 bytes.
OK puente SimConnect
OK Pico 2 (prueba falla) - Sketch uses 106420 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23016 bytes (4%) of dynamic memory, leaving 501272 bytes for local variables. Maximum is 524288 bytes.
OK sintaxis FALLA.ps1
OK sintaxis ARRANQUE.ps1
OK FALLA rechaza PC fuera de codigo (salida 1)
RESULTADO: OK
```

- Sintaxis con el parser de Windows PowerShell 5.1. RESULTADO: OK corresponde a `tests/bin/comparar_ensayo.exe tmp/vuelo_corto`, codigo 0.
- Tamaños respecto de 105820 B: configurado/potes +560 B, encoders +720 B, prueba falla +600 B.
- FALLA con PC de loop tomado del map final (`-Pc 0x100067dc -Elf build-pico2-prueba-falla/MiYoke_PD.ino.elf`), codigo 0:

```text
El .elf debe ser de la misma compilacion que el UF2 cargado
PC 0x100067dc -> loop (D:\LUCAS\UTN\PROYECTO FINAL\PROYECTO ALEMAN\PROYECTO MODIFICADO\MiYoke_PD/MiYoke_PD.ino:240)
```

- Adicional: FALLA con PC 0x20000110 y LR 0x2000015d del ELF configurado devuelve hardfaultCapture e isr_hardfault (LR limpio 0x2000015c), codigo 0. ELF de prueba final confirma `20000110 T hardfaultCapture` y `2000015c T isr_hardfault`: RAM y simbolos fuertes. Stack verificado en Parte 1: 2048 B, no 4 KB.
- `git diff --check`: sin errores. Sin desvios del alcance. ARRANQUE no se ejecuto en hardware; no se cargo firmware ni se integro a master.
- Version: **V2.2**, creada por `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Diagnostico de HardFault con PC y excepcion, motores a 0 V en la falla y stack libre"`. Salida: `Version V2.2 creada`. Verificada la existencia de `../MiYoke_PD_VERSIONES/V2_2/ACTUALIZACION_V2_2.txt` y su primera linea `VERSION: V2.2`; etiqueta sobre `97d26e8`. Se ejecuto desde el checkout temporal limpio `../MiYoke_PD-versionado-hardfault-codex`, en la misma rama, para conservar los cambios del usuario en tesis/. El checkout temporal fue retirado despues de verificar la version.
- Sigue: revision de Claude de Parte 2 y protocolo del usuario V1, R1-R3 (§5), con motores apagados.

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
