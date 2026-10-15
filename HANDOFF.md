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

Tarea: **posición desde SimConnect por defecto** (dejar de usar los potenciómetros de la Pico 2) y **compilar-tesis.ps1 con LaTeX instalado** (Claude, 2026-10-15).
Rama: `cambio/posicion-simconnect` (sale de `mejora/diagnostico-hardfault`, ya creada). Plan único, sin partes: son cambios de pocas líneas.

\### Precondiciones
- `git status`: ignorar `graphify-ino/`, `tesis/main.tex`, `tesis/capitulos/11_maquinas_estado.tex` y `tesis/caracterizacion_motores_775.tex` (cambios del usuario; no commitearlos).
- Orientarse con `graphify query`; leer solo las líneas indicadas. Si un texto actual no aparece exactamente, anotar `BLOQUEO:` o la diferencia en "Última entrega" y seguir con el resto (AGENTS.md, FLUJO AUTOMATICO).

\### Fundamento (verificado por Claude)
- [Código] `mis_defines.h:53-56`: `YOKE_USE_SIMULATION` 0 = encoders, 1 = SimConnect, 2 = potes. Con 1, `MiYoke_PD.ino` no incluye `AxisPotentiometers.h` ni lee ADC (`:2-6`, `:88-95`, `:257`), toma posición de `simTelemetry.yoke[axis]` y velocidad de `simEffects.simulationVelocity(axis)` (`:339-347`), y pasa `useSimPosition=true` (`:371`). La fricción vuelve a la ley de velocidad (`SimEffects.h:10`). El puente ya envía YOKE X/Y POSITION. No hace falta otro cambio de firmware.
- [Código] `compilar-tesis.ps1:21-41`: con MiKTeX instalado (latexmk en PATH) el script falla siempre. (a) Con `$ErrorActionPreference='Stop'`, el texto que latexmk escribe en stderr ("This is METAFONT...") se vuelve `NativeCommandError` en PowerShell 5.1 y corta el script. (b) El patrón `^(.*:\d+: |! )` toma como error las líneas `...: ignored error: Infinite glue shrinkage...` del log, aunque latexmk termine con código 0 y genere el PDF (Claude lo probó: código 0, PDF de 1,1 MB, 8 líneas "ignored error", 0 referencias indefinidas).

\### Alcance
1. `mis_defines.h:55`: `#define YOKE_USE_SIMULATION 2` → `#define YOKE_USE_SIMULATION 1`. Nada más en ese archivo.
2. `ARRANQUE.ps1:7` (`Select-Elf`): en el caso normal usar `'build-pico2'` en vez de `'build-pico2-potentiometers'` (el UF2 normal ahora es el de la configuración de `mis_defines.h`). El caso `etapa=15:PRUEBA` no cambia. Actualizar también `tmp/verificar-select-elf.ps1` si se reutiliza para la prueba.
3. `compilar-tesis.ps1`, solo la rama latexmk (`:21-41`):
   - alrededor de la llamada a `latexmk`, poner `$ErrorActionPreference = 'Continue'` y restaurar el valor anterior en el `finally` (mismo patrón que `ARRANQUE.ps1:18`); mantener `*> $null`;
   - separar las líneas que contienen `ignored error`: no cuentan como error; contarlas aparte;
   - salida OK: `OK compilacion latexmk ($refs referencias indefinidas, $ignorados errores ignorados por LaTeX). PDF en tmp\tesis-build`;
   - FALLO solo si el código de salida no es 0 o hay errores reales (sin `ignored error`). El chequeo estático no cambia.
4. Documentación (solo estas líneas; cambios mínimos):
   - `LEEME md/LEEME_PICO2.md:30`: `- \`build-pico2/MiYoke_PD_V2.2.ino.uf2\`: potenciometros, configuracion predeterminada.` → `- \`build-pico2/MiYoke_PD_V2.2.ino.uf2\`: posicion desde SimConnect (YOKE_USE_SIMULATION=1), configuracion predeterminada.`
   - `LEEME md/ENSAYOS.md:83`: `si no, build-pico2-potentiometers.` → `si no, build-pico2 (configuracion de mis_defines.h; para otro UF2 pasar -Elf).`
   - `LEEME md/ENSAYOS.md:88` (R1): `build-pico2-potentiometers\MiYoke_PD.ino.uf2` → `build-pico2\MiYoke_PD.ino.uf2`.
   - No cambiar `LEEME_POTENCIOMETROS.md`, `AJUSTE_FUERZAS_Y_MOVIMIENTO.md` ni `MENU_CALIBRACION.md`: ya dicen que el valor por defecto es 1.
5. Tesis (aprobado por el usuario el 2026-10-15; reemplazo literal, sin leer el capítulo entero):
   - `tesis/capitulos/03_centrado.tex`
     - TEXTO ACTUAL: `\textbf{Hallazgo importante:} \code{mis_defines.h} define \code{YOKE_USE_SIMULATION=2}. Por defecto, la fuerza depende de la presión de SimConnect y de la posición de los \textbf{potenciómetros locales}. No depende de la posición de SimConnect, aunque el puente siga solicitándola, validándola y transmitiéndola. Las opciones son:`
     - TEXTO NUEVO: `\textbf{Hallazgo importante:} \code{mis_defines.h} define \code{YOKE_USE_SIMULATION=1}. Por defecto, la fuerza depende de la presión de SimConnect y de la posición del mando virtual (\code{YOKE X POSITION} / \code{YOKE Y POSITION}) que transmite el puente. Los potenciómetros locales no se leen en este modo. Las opciones son:`
   - `tesis/capitulos/03_centrado.tex`
     - TEXTO ACTUAL: `1 & SimConnect & \code{YOKE X POSITION} / \code{YOKE Y POSITION}\\`
     - TEXTO NUEVO: `1 & SimConnect & \code{YOKE X POSITION} / \code{YOKE Y POSITION}; opción predeterminada\\`
   - `tesis/capitulos/03_centrado.tex`
     - TEXTO ACTUAL: `2 & Potenciómetros & ADC0/ADC1; opción predeterminada\\`
     - TEXTO NUEVO: `2 & Potenciómetros & ADC0/ADC1\\`
   - `tesis/capitulos/09_damper.tex`
     - TEXTO ACTUAL: `El valor predeterminado inspeccionado es \textbf{2}.`
     - TEXTO NUEVO: `El valor predeterminado inspeccionado es \textbf{1}.`
6. `graphify update .` (no se edita `MiYoke_PD.ino`, no hace falta `graphify-sync.cmd`).

\### Pruebas
- `powershell -ExecutionPolicy Bypass -File probar.ps1 -Pico -Potentiometers` (suite + firmware modo 1 configurado + variante potes, para confirmar que el modo 2 sigue compilando). Anotar programa/RAM del modo 1 si `probar.ps1` lo informa.
- Prueba de `Select-Elf` como en la Parte 2b: BOOT `etapa=15:PRUEBA` → `build-pico2-prueba-falla`; BOOT normal → `build-pico2`.
- `powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1`: esperado `OK compilacion latexmk (...)`. Puede tardar la primera vez (MiKTeX). Si da FALLO, copiar solo sus líneas.
- No leer logs de LaTeX completos ni ejecutar pdflatex/latexmk directamente.

\### Commit y versión
- Commit por nombre: `mis_defines.h ARRANQUE.ps1 compilar-tesis.ps1 "LEEME md/LEEME_PICO2.md" "LEEME md/ENSAYOS.md" tesis/capitulos/03_centrado.tex tesis/capitulos/09_damper.tex HANDOFF.md`. Mensaje: "Posicion desde SimConnect por defecto; compilar-tesis funciona con LaTeX instalado".
- Versión: `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Posicion desde SimConnect por defecto y compilar-tesis con LaTeX instalado"`. Verificar `Version Vx.y creada` y el `ACTUALIZACION_Vx_y.txt`. Si los cambios del usuario en `tesis/` lo impiden, usar el checkout temporal limpio, como en la Parte 2, y anotarlo.
- "Última entrega": archivos, líneas OK/FALLO de las pruebas, prueba de `Select-Elf`, línea de `compilar-tesis.ps1` y versión. Terminar con un resumen corto como último mensaje.

\## Última entrega

- (vacío: tarea nueva. La entrega anterior, Parte 2b del HardFault, está en el historial de git.)

\## Revisión

- (vacío: tarea nueva.)

\## Ensayos del usuario

- Protocolo V1 y R1-R3 (usuario, 2026-10-13, V2.3, motores apagados). Carpetas `ensayos/20261013-191138_arranque` a `20261013-192018_arranque`. Análisis de Claude.
  - **V1 (HardFault provocado): el instrumento funciona de punta a punta.**
    - `BOOT reset=WDT wd=0x1 etapa=15:PRUEBA falla=1 exc=0 pc=0x10007d06 lr=0x100060bd cfsr=0x00010000`.
    - `FALLA.ps1` lleva el PC a `loop` (`MiYoke_PD.ino:568` = `__builtin_trap()`) y el LR a `StageTimer::enter` (la última llamada, `markStage(TestHang)`).
    - CFSR 0x00010000 = UFSR.UNDEFINSTR, que corresponde a la instrucción UDF de `__builtin_trap`. `exc=0` (hilo). Todo coherente.
  - **R1 y R3 (carga de UF2 sin desenchufar):** `reset=SOFT wd=0x1`, sin falla. Es el reinicio del bootrom después de copiar el UF2.
  - **R2 y R3 (desenchufar y enchufar, 4 veces):** `reset=ENCENDIDO`, sin falla.
  - **El HardFault del 13/10 (`reset=WDT falla=1 etapa=0`) no se reprodujo en 6 arranques.** No es un efecto de la carga del UF2, que da `SOFT`: fue una falla real en ejecución.
  - **HALLAZGO: stack libre mínimo muy bajo.** De 2048 B quedaron libres 448, 544, 544, 560, 568 y 688 B, en sesiones de solo ~15 s de replay en monitor.
    - En cada sesión el valor **baja con el tiempo**, por ejemplo 824 → 800 → 776 → 560 B en 11 s.
    - Está por debajo del umbral de 512 B del diseño (§5): **es evidencia a favor de H-b, desborde de stack**. En un vuelo largo, con menú, LCD y todas las líneas de diagnóstico, el uso puede ser mayor.
    - Un desborde corrompe la RAM contigua sin aviso (MSPLIM=0) y explicaría una falla posterior fuera del lazo (`etapa=0`).
  - **Siguiente paso propuesto (tarea por Flujo, pendiente de aprobación del usuario):**
    1. medir qué consume el stack (locales grandes en `loop()`, `snprintf` de newlib, interrupciones anidadas);
    2. ver si el core permite agrandar el stack del núcleo 0 o activar MSPLIM, para que un desborde falle enseguida y con el PC exacto;
    3. bajar el consumo o agrandar el stack, con criterio medido: stack libre ≥ 1 KB en un vuelo completo con `ENSAYO.cmd`.

- (vacío: tarea nueva. El ensayo CICLOS y el hallazgo del HardFault están en `master`, commit `bccd1b7`.)
