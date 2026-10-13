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

Tarea: diagnóstico del HardFault — **Parte 2b**: correcciones de la revisión de la Parte 2 (Claude, 2026-10-13).
Rama: `mejora/diagnostico-hardfault`. Diseño: `LEEME md/DISENO_diagnostico_hardfault.md` (solo como referencia: §4.2 y §8). Plan único, sin partes.

\### Precondiciones
- `git status`: ignorar `graphify-ino/`, `tesis/main.tex`, `tesis/capitulos/11_maquinas_estado.tex` y `tesis/caracterizacion_motores_775.tex` (cambios del usuario).
- Orientarse con `graphify query`; leer solo lo indicado. Si algo no coincide, anotar `BLOQUEO:` en "Última entrega" y terminar (AGENTS.md, FLUJO AUTOMATICO).

\### Alcance
1. `ARRANQUE.ps1`:
   - **R2b-1, `.elf` según la línea BOOT** (`:65-77`):
     - sin `-Elf`, elegir el `.elf` automáticamente: si la línea `BOOT` contiene `etapa=15:PRUEBA`, usar `build-pico2-prueba-falla\MiYoke_PD.ino.elf`; si no, `build-pico2-potentiometers\MiYoke_PD.ino.elf`;
     - si el archivo existe, llamar a `FALLA.ps1` con ese `-Elf`, guardar `falla.txt` e imprimir la línea `Elf usado: <ruta> (elegido por la linea BOOT; si cargaste otro UF2, repetir FALLA.ps1 con su .elf)`;
     - si no existe, imprimir el comando de `FALLA.ps1` con ese `.elf` y el aviso `No existe <ruta>`;
     - con `-Elf` explícito, sigue como hoy.
   - **R2b-2, carpeta del ensayo** (`:28-34`): crear la carpeta `ensayos\..._arranque` **después** de comprobar que existe el puente (`$bridge`) y de detectar el COM, para que no queden carpetas vacías si falla un chequeo.
2. `LEEME md/LEEME_PICO2.md:43`: mover la línea `probar.ps1 -PruebaFalla compila build-pico2-prueba-falla (...)` para que quede **después** del párrafo `El primer comando respeta ... Ninguno carga ni activa la placa.` y no entre los comandos y ese párrafo. Sin otros cambios.
3. `LEEME md/ENSAYOS.md:81`: el título pasa a ser `## Arranque y fallas (ARRANQUE.cmd, motores apagados)`. En la sección, agregar una línea: `Sin -Elf, ARRANQUE elige el .elf por la linea BOOT: etapa=15:PRUEBA usa build-pico2-prueba-falla; si no, build-pico2-potentiometers.`
4. `graphify update .`.

\### Pruebas
- `powershell -ExecutionPolicy Bypass -File probar.ps1` (suite, comparador y lanzador) y verificación de sintaxis de `ARRANQUE.ps1` con el parser de PowerShell 5.1, como en la Parte 2.
- Probar la selección del `.elf` con una línea BOOT de ejemplo (sin hardware): extraer la elección a una función dentro de `ARRANQUE.ps1` (`function Select-Elf([string]$boot)`) y llamarla desde un comando de PowerShell con los dos casos. Anotar el resultado.

\### Commit y versión
- Commit por nombre: `ARRANQUE.ps1 "LEEME md/LEEME_PICO2.md" "LEEME md/ENSAYOS.md" HANDOFF.md`. Mensaje: "ARRANQUE: elegir .elf por la linea BOOT y crear carpeta tras los chequeos".
- Versión: `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "ARRANQUE elige el .elf segun la linea BOOT"`. Verificar `Version Vx.y creada` y el `ACTUALIZACION_Vx_y.txt`. Si los cambios del usuario en `tesis/` lo impiden, usar el checkout temporal limpio, como en la Parte 2, y anotarlo.
- "Última entrega": reemplazar el contenido por lo de la Parte 2b (la Parte 2 queda en el historial de git), con las líneas OK/FALLO, la prueba de `Select-Elf` y la versión. Terminar con un resumen corto como último mensaje.

\## Última entrega

- Parte 2b completada (Codex, 2026-10-13), rama `mejora/diagnostico-hardfault`. Commit de implementacion: `bc16be5` (codigo, documentacion y HANDOFF); esta anotacion final se guarda en un commit posterior de HANDOFF.
- Archivos modificados: `ARRANQUE.ps1`, `LEEME md/LEEME_PICO2.md`, `LEEME md/ENSAYOS.md` y `HANDOFF.md`.
- R2b-1 aplicado: [Codigo] `ARRANQUE.ps1:6` agrega `Select-Elf`; sin `-Elf`, BOOT con `etapa=15:PRUEBA` elige `build-pico2-prueba-falla\MiYoke_PD.ino.elf`; los otros casos eligen `build-pico2-potentiometers\MiYoke_PD.ino.elf`. Si existe, ejecuta FALLA, guarda `falla.txt` e informa el ELF elegido; si falta, informa `No existe` y el comando con ese ELF. Con `-Elf` explicito conserva el comportamiento anterior.
- R2b-2 aplicado: [Codigo] `ARRANQUE.ps1:32-37` comprueba el puente antes de crear la carpeta del ensayo; la deteccion del COM ya ocurre antes. R2b-3 aplicado: se movio la linea de PruebaFalla despues del parrafo de los dos comandos en LEEME_PICO2 y se agregaron el titulo con motores apagados y la seleccion automatica del ELF en ENSAYOS. Sin otros cambios documentales; no afecta tesis.
- `graphify update .`: completado, AST sin API. Cambios del usuario en tesis/ y graphify-ino/ conservados, fuera del commit.
- Pruebas: `powershell -ExecutionPolicy Bypass -File probar.ps1`, codigo 0:

```text
OK suite simconnect_checks - PASS: 143619 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
OK comparar_ensayo
OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
OK sintaxis ARRANQUE.ps1 (PowerShell 5.1)
OK Select-Elf: etapa=15:PRUEBA -> build-pico2-prueba-falla; BOOT normal -> build-pico2-potentiometers
```

- Select-Elf probado desde PowerShell 5.1 extrayendo solo la funcion mediante AST a `tmp/verificar-select-elf.ps1`, sin ejecutar el cuerpo de ARRANQUE ni abrir puertos. El primer comando aislado uso Invoke-Expression y perdio PSScriptRoot (cadena vacia): fallo del contexto de prueba; se corrigio el harness mediante archivo temporal y ambos casos pasaron sin cambios adicionales al codigo.
- `git diff --check`: sin errores. No se recompilo firmware ni puente porque no se modificaron; no se editaron ni compilaron .tex. Sin ensayos de hardware, carga de firmware ni integracion a master.
- Version: **V2.3**, creada por `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "ARRANQUE elige el .elf segun la linea BOOT"`. Salida: `Version V2.3 creada`. Verificado `../MiYoke_PD_VERSIONES/V2_3/ACTUALIZACION_V2_3.txt`: `VERSION: V2.3`, `COMMIT: bc16be5`; etiqueta sobre `bc16be5`. Se uso el checkout temporal limpio `../MiYoke_PD-versionado-hardfault-codex`, en la misma rama, para conservar los cambios del usuario en tesis/. Fue retirado tras verificar la version.
- Sigue: revision de Claude de Parte 2b; ensayos V1 y R1-R4 por el usuario con motores apagados.

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

- Revisor: Claude (2026-10-13), con Code Reviewer. **Parte 2 (`97d26e8`, V2.2): APROBADA**, con correcciones menores que van a la **Parte 2b** (ver Plan para Codex).
  - **Cumple:**
    - `stackLine` estricto y probado;
    - el puente registra el stack libre y su mínimo en `Fin:`;
    - se reinicia `lastStackBytes` con el flanco de `dtr`;
    - `FALLA.ps1` cumple §4.1, y su prueba con el `.elf` de prueba lleva el PC 0x100067dc a `loop` (`MiYoke_PD.ino:240`);
    - `ARRANQUE.ps1`/`.cmd` cumplen §4.2 (sin `--enable`, copia los logs aunque el puente falle, códigos 0/1/2);
    - documentación de §8 literal; tag V2.2 sobre `97d26e8`.
  - **R2b-1 (medio):** sin `-Elf`, `ARRANQUE.ps1:75-76` imprime siempre el `.elf` de la prueba de falla. En R1-R4 se carga el UF2 normal, así que la traducción daría una función equivocada.
  - **R2b-2 (detalle):** la carpeta del ensayo se crea antes de los chequeos.
  - **R2b-3 (documentación):** la línea de `LEEME_PICO2.md` corta el párrafo siguiente, y a `ENSAYOS.md` le falta "motores apagados".
- Flujo automático (2026-10-13): prueba de lanzamiento de `codex exec` OK con `--approve-for-me -c windows.sandbox="unelevated"`. El modo "elevated" falla desde Claude. Regla actualizada en `CLAUDE.md`. La Parte 2b es la primera parte lanzada por Claude.

\## Ensayos del usuario

- (vacío: tarea nueva. El ensayo CICLOS y el hallazgo del HardFault están en `master`, commit `bccd1b7`.)
