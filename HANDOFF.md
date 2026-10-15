\# HANDOFF entre agentes



\## Reglas

1\. Un solo agente edita a la vez. Cada tarea va en su propia rama: `git checkout -b tarea/<nombre>`.

2\. Al terminar, el agente completa la sección "Última entrega" y hace commit.

3\. El otro agente revisa el diff (`git diff master...HEAD`) sin editar, y anota hallazgos en "Revisión".

4\. Nada se integra a `master` sin que el usuario valide en hardware.

5\. Antes de afirmar algo del código, indicar la fuente: \[Grafo], \[INO] o \[Código], con archivo y línea. Las aristas INFERRED se tratan como hipótesis.

6\. Tras editar `MiYoke\_PD.ino`, ejecutar `herramientas/flujo/graphify-sync.cmd`.

7\. No modificar constantes de seguridad (límites de PWM, `MOTOR\_PWM\_HARD\_LIMIT`, `SIMCONNECT\_EXTRA\_LIMIT`) sin aprobación explícita del usuario.

8\. Nunca usar `git add .` ni `git add -A`: la carpeta graphify-ino/ no debe versionarse. Agregar archivos por nombre.

9\. Un hallazgo de "posible bug" se verifica leyendo el código involucrado antes de proponer cambios. Informar la evidencia (archivo y línea), no solo la sospecha.

10\. Las reglas de versionado, documentacion, ahorro de tokens y pruebas estan en AGENTS.md (Codex) y CLAUDE.md (Claude); no se repiten aca.

11\. "Ultima entrega" y "Revision" contienen solo la tarea en curso: al empezar una tarea nueva se reemplazan (lo anterior queda en el historial de git).

\## Plan para Codex

Tarea: **Reorganizar scripts de soporte en la carpeta herramientas** (Claude, 2026-10-15).
Rama: `cambio/reorganizar-carpetas` (ya creada desde `master` = V2.4, con el diseño commiteado). Plan único, sin partes (un solo commit atómico; ver diseño 7.3).

Diseño aprobado por el usuario: `LEEME md/DISENO_reorganizacion_carpetas.md`. Es una especificación cerrada: aplicarla tal cual, sin redactar ni decidir.

Leer solo: secciones 3 a 9 del diseño (estructura, tabla origen → destino, cambios exactos por script, `.gitignore`, pasos, pruebas, documentación afectada). Para ubicar líneas, usar grep y leer solo esas líneas.

Puntos a respetar:
- Precondición P1 del diseño: no tocar ni commitear `tesis/main.tex`, `tesis/capitulos/11_maquinas_estado.tex`, `tesis/caracterizacion_motores_775.tex` ni `graphify-ino/`.
- Movimientos con `git mv` y borrado del PDF duplicado con `git rm` (aprobado por el usuario).
- Conservar el BOM UTF-8 de los scripts que lo tienen (diseño 5 y 8.4).
- No editar nada de `tesis/`.
- **No ejecutar `nueva-version.ps1`** (paso 9 omitido): la versión V2.5 la crea Claude después de la revisión.
- Pruebas: línea base antes (P4) y diseño 8.1 a 8.6 después. Copiar en HANDOFF solo las líneas OK/FALLO.
- Commit único del paso 8 (por nombre) y luego `herramientaslujo\graphify-sync.cmd`.
- Si algo del diseño no coincide con el archivo real, no improvisar: anotar `BLOQUEO:` en "Última entrega" y terminar.

Al terminar, completar "Última entrega": commit, líneas OK/FALLO (base y final), "versión: pendiente, la crea Claude" y la lista 8.7 para el usuario.

\## Última entrega

- Reorganizacion implementada segun secciones 3 a 9 del diseno, rama `cambio/reorganizar-carpetas`: 16 scripts movidos con `git mv` a `herramientas/compilacion`, `herramientas/ensayos` y `herramientas/flujo`; rutas de raiz explicitas; PDF duplicado de la raiz eliminado con `git rm`; `.gitignore` anclado a `/ensayos/`.
- Documentacion actualizada: `AGENTS.md`, `CLAUDE.md`, regla 6 de `HANDOFF.md`, `.claude/skills/verificacion-antes-de-terminar/SKILL.md`, `LEEME md/ENSAYOS.md`, `LEEME md/LEEME_PICO2.md`, `LEEME md/LEEME_SIMCONNECT.md`, `LEEME md/INVENTARIO_CODIGO.md`. Tesis, firmware, lanzador y diseno sin editar.
- Commit unico: el que contiene esta entrega, con titulo `Reorganiza scripts de soporte en herramientas/ (compilacion, ensayos, flujo) y elimina PDF duplicado de la raiz` (consultar `git log -1 --oneline` al revisar).
- Version: pendiente, la crea Claude despues de la revision (no se ejecuto `nueva-version.ps1`, segun P3).
- BLOQUEO: el criterio literal de 8.5 exige que cambie LastWriteTime de `graphify-ino/MiYoke_PD_ino.cpp`, pero `copy /y` conserva la fecha del origen. La copia y el INO tienen 23770 bytes, el mismo SHA-256 B4CD68D4C47D347547A816A386441F157B93E452E28CC314B9B1A37250475E9C y fecha UTC 2026-10-13 21:39:03. Una copia de control en tmp confirma la conservacion de fecha. `graphify-sync.cmd` se implemento literalmente, sin redisenarlo. Claude debe resolver ese criterio antes de aprobar; no se cambio la fecha artificialmente.

Linea base (P4):

```text
FALLO compilacion latexmk (codigo -1):
OK suite simconnect_checks - PASS: 143619 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
OK comparar_ensayo
OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
OK Pico 2 (configurado) - Sketch uses 105388 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22920 bytes (4%) of dynamic memory, leaving 501368 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (encoders) - Sketch uses 106540 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23256 bytes (4%) of dynamic memory, leaving 501032 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (potenciometros) - Sketch uses 106380 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23016 bytes (4%) of dynamic memory, leaving 501272 bytes for local variables. Maximum is 524288 bytes.
OK puente SimConnect
OK compilacion latexmk (0 referencias indefinidas, 8 errores ignorados por LaTeX). PDF en tmp\tesis-build
```

La primera compilacion LaTeX fallo dentro del sandbox. Repetida con acceso a MiKTeX dio el OK registrado arriba; no se editaron archivos de tesis.

Final (8.1):

```text
OK suite simconnect_checks - PASS: 143619 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
OK comparar_ensayo
OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
OK Pico 2 (configurado) - Sketch uses 105388 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 22920 bytes (4%) of dynamic memory, leaving 501368 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (encoders) - Sketch uses 106540 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23256 bytes (4%) of dynamic memory, leaving 501032 bytes for local variables. Maximum is 524288 bytes.
OK Pico 2 (potenciometros) - Sketch uses 106380 bytes (2%) of program storage space. Maximum is 4186112 bytes. | Global variables use 23016 bytes (4%) of dynamic memory, leaving 501272 bytes for local variables. Maximum is 524288 bytes.
OK puente SimConnect
OK compilacion latexmk (0 referencias indefinidas, 8 errores ignorados por LaTeX). PDF en tmp\tesis-build
```

Comprobaciones 8.2 a 8.6 y 8.8:

```text
OK suite simconnect_checks - PASS: 143619 checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).
OK comparar_ensayo
OK launcher_port_checks - PASS: 19 launcher discovery checks. No hardware ports opened.
OK sintaxis PowerShell: 11 scripts, 0 errores; BOM solo en BANCO, Build-Bridge, CICLOS y ENSAYO.
OK PSScriptRoot: 10 calculos de raiz y 2 referencias hermanas a FALLA.ps1.
OK lanzar-codex: REPO apunta a MiYoke_PD; titulo Parte 1 - Reorganizar scripts de soporte en la carpeta herramientas.
OK ARRANQUE sin hardware: Error: No existe la entrada: noexiste.csv; codigo 1.
OK estructura: 35 archivos versionados en la raiz; UF2 en build-pico2, build-pico2-encoders y build-pico2-potentiometers; sin build-pico2 en herramientas/compilacion.
OK gitignore: herramientas/ensayos/ENSAYO.ps1 no ignorado; ensayos/x ignorado por /ensayos/.
OK graphify update desde TEMP: 1244 nodos, 1626 aristas; rutas nuevas indexadas.
FALLO criterio 8.5 graphify-sync: fecha sin actualizar (copy conserva la del origen; ver BLOQUEO).
```

- La primera copia graphify en sandbox fue denegada; se repitio con acceso de escritura. La igualdad de hashes confirma la copia correcta. No se altero manualmente graphify-ino ni se incluyo en el commit.
- Los cambios previos del usuario en tesis/main.tex, tesis/capitulos/11_maquinas_estado.tex, tesis/caracterizacion_motores_775.tex y graphify-ino/ se conservan fuera del commit. Sin carga de firmware ni ensayos de hardware.
- Siguiente: Claude revisa el commit, resuelve el criterio 8.5 y crea V2.5 segun P3.

Verificacion manual del usuario (8.7, pendiente):
1. Doble clic en el acceso directo del escritorio `INICIAR SIMULADOR V3`: debe iniciar igual que antes.
2. Doble clic en `herramientas\ensayos\ENSAYO.cmd`: debe crear `MiYoke_PD\ensayos\<fecha>_vuelo1\rutina.txt` en la raiz. Si es solo una prueba, cerrar la ventana y borrar esa carpeta.
3. Primer `ARRANQUE.cmd`/`BANCO.ps1` real: comprobar que los registros aparecen en `ensayos\` de la raiz.

\## Revisión

- Pendiente de Claude: reorganizacion de scripts. Resolver BLOQUEO del criterio 8.5 antes de aprobar y crear la version.

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
