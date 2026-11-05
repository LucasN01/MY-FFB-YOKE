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

Tarea: Revisar objecion al hallazgo H1 del limitador de topes

(Claude, 2026-11-05.) Segunda ronda de revision, de las dos permitidas. **Codex solo revisa, no edita ni commitea.** Rama `mejora/limite-topes`. Leer `tmp/revision_codex.md` (ronda 1) y la seccion "Revision" de este archivo. Ver el cambio con `git diff 45cd8c6..HEAD`.

1. Intentar refutar la objecion de Claude a H1, con codigo o reproduccion: busca un caso donde conservar el damper entero produzca fuerza hacia el tope, un salto, una orden sin acotar, o un dano real que no se resuelva aplicando el limitador a la suma.
2. Comprobar que el I3 nuevo, `LEEME md/AJUSTE_FUERZAS_Y_MOVIMIENTO.md` y el codigo dicen lo mismo, y que la prueba nueva de `tests/endstop_checks.h` fallaria si se aplica el limitador a la suma (mutacion en copia dentro de `tmp/`).
3. Actualizar los puntos 10, 12 y 16 de la lista con evidencia. El punto 12 cubre las cinco variantes con `probar.ps1 -Todo -PruebaFalla`.
4. Dejar el resultado en `tmp/revision_codex.md`, conservando la ronda 1: objecion refutada o no, tabla de los puntos revisados y veredicto "aprobado" o "requiere cambios". No tocar `tesis/`, `HANDOFF.md` ni crear versiones.

\## Última entrega

- Limitador de topes implementado por Claude en la rama `mejora/limite-topes`, sobre `firmware/pico-sdk`, para los cuatro modos de posicion. Pendiente: revision de Codex, version, tesis (si el usuario acepta) y ensayo F-tope con hardware.
- `probar.ps1 -Todo`: OK simconnect_checks (1916541), OK comparar_ensayo, OK hal_checks (694), OK launcher_port_checks (19), OK Pico 2 configurado 65692/8608, encoders 66972/8924, potenciometros 66540/8708, AS5600 67644/8772 (flash/RAM), OK puente SimConnect.
- Sigue pendiente de la tarea anterior: ensayos H0-H13 de la migracion a Pico SDK (detalle en `LEEME md/DISENO_migracion_pico_sdk.md` seccion 7).

\## Revisión

Ronda 1 de Codex: requiere cambios, un hallazgo (`tmp/revision_codex.md`).

- H1, objetado en parte por Claude. Verificado: con efectos hacia el tope y damper frenando, la orden hacia el centro crece (-0,006 a -0,119), lo que contradecia I3. Se conserva la ley: la fuerza es de frenado, esta acotada por el techo del damper, se anula con el yugo quieto, y aplicar el limitador a la suma dejaria que la vibracion se coma el frenado mientras el yugo va hacia el tope. Aplicado: I3 y `AJUSTE_FUERZAS_Y_MOVIMIENTO.md` corregidos, prueba de regresion agregada. Pendiente: intento de refutacion de Codex.

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
