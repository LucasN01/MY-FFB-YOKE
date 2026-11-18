\# HANDOFF entre agentes



\## Reglas

1\. Un solo agente edita a la vez. Cada tarea va en su propia rama: `git checkout -b tarea/<nombre>`.

2\. Al terminar, el agente completa la sección "Última entrega" y hace commit.

3\. El otro agente revisa el diff (`git diff master...HEAD`) sin editar, y anota hallazgos en "Revisión".

4\. Nada se integra a `master` sin que el usuario valide en hardware.

5\. Antes de afirmar algo del código, indicar la fuente: \[Grafo], \[INO] o \[Código], con archivo y línea. Las aristas INFERRED se tratan como hipótesis.

6\. Tras editar codigo, ejecutar `graphify update .` (o `herramientas/flujo/graphify-sync.cmd`, que hace lo mismo).

7\. No modificar constantes de seguridad (límites de PWM, `MOTOR\_PWM\_HARD\_LIMIT`, `SIMCONNECT\_EXTRA\_LIMIT`) sin aprobación explícita del usuario.

8\. Nunca usar `git add .` ni `git add -A`: agregar archivos por nombre.

9\. Un hallazgo de "posible bug" se verifica leyendo el código involucrado antes de proponer cambios. Informar la evidencia (archivo y línea), no solo la sospecha.

10\. Las reglas de versionado, documentacion, ahorro de tokens y pruebas estan en AGENTS.md (Codex) y CLAUDE.md (Claude); no se repiten aca.

11\. "Ultima entrega" y "Revision" contienen solo la tarea en curso: al empezar una tarea nueva se reemplazan (lo anterior queda en el historial de git).

\## Plan para Codex

Tarea: Revision del capitulo de tesis del modelo del motor DC 775

(Claude, 2026-11-18.) Rama `doc/modelo-motor-775`. Solo revision: no editar ni commitear nada.

- Que revisar: `tesis/capitulos/12_modelo_motor_775.tex` y las dos lineas nuevas de `tesis/main.tex` (`git diff mejora/limite-topes...HEAD`).
- Lista de control: `tmp/lista_control_modelo_motor.md`. Primero buscar defectos con libertad; despues marcar cada punto como cumple, parcial o no, con evidencia (hoja y celda, o archivo y linea).
- Datos: `modelizacion motor 775/` esta fuera de git en este equipo pero existe en disco. Leer los .xlsx con Python y openpyxl, sin guardarlos.
- No compilar la tesis salvo para el punto 19: `powershell -ExecutionPolicy Bypass -File herramientas/flujo/compilar-tesis.ps1` (una linea OK/FALLO; hoy da 7 errores ignorados que ya estaban antes del cambio).
- Resultado: `tmp/revision_codex.md` con "aprobado" o "requiere cambios" y la lista de hallazgos numerados (H1, H2...).
- Ronda 2 (Claude, 2026-11-18): verificar solo que H1-H4 de la ronda 1 quedaron corregidos en el commit nuevo (`git diff 4ef84c1 HEAD -- tesis/`) y que no se introdujo nada nuevo incorrecto. Reemplazar `tmp/revision_codex.md` con el resultado de la ronda 2.
- No crea version (solo documentacion).

\## Última entrega

- Capitulo nuevo de tesis `tesis/capitulos/12_modelo_motor_775.tex` (seccion `sec:motor775`): modelo del motor DC 775, ensayos 1 a 6, mediciones de R con fuente continua y de Ke en vacio, eleccion de R = 0.79 ohm, Kt = Ke = 0.0222, ajuste previsto en MATLAB con los scripts de Castano Giraldo (pendiente), J pendiente (falta t_mss e i_arr). Lo redacto Claude a pedido explicito del usuario.
- `tesis/main.tex`: `\input` del capitulo y una linea en el resumen.
- Compilacion: OK, 0 referencias indefinidas, 7 errores ignorados (los mismos sin el cambio).
- Regla del usuario: cuando cambien valores medidos (Kt, Ke, R, L, J, B...), actualizar tambien este capitulo.

\## Revisión

Ronda 1 de Codex (2026-11-18): requiere cambios, 4 hallazgos, todos verificados por Claude contra la planilla y aplicados. H1: contacto de escobillas absorbido en R como simplificacion local, sin afirmarlo resistivo; caida de la posicion 2 como "compatible". H2: asimetria de Kt con causa pendiente (montaje como sospecha). H3: Icont "estimado por carcasa", con Ta 24 C, R0 0.92 y calculo de Rth explicitos. H4: rango del Ensayo 3 2507-10271 rpm. Ademas: T0 = 22 C como supuesto, el +-2 % de Kt como dispersion entre metodos y la separacion de escalas como hipotesis hasta medir J. Compila OK.

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
