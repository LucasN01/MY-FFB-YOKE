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

Sin tareas pendientes para Codex (2026-10-11).

\## Última entrega

\- Agente: Codex (2026-10-11) — implementación completada del umbral de inicio del buffet de tren y de spoiler. Rama `mejora/realismo-efectos`.

\- Commit de código, pruebas y documentación: `4b9f5f6` — "Buffet tren y spoiler: umbral de inicio 100/80 kt con crecimiento por presion dinamica".
  - Código: `BuffetModel.h`.
  - Pruebas: `tests/simconnect_checks.cpp` y `tests/raw_checks.h`.
  - Documentación actualizada: `tesis/explicacion_efectos_proyecto_ffb_yoke.tex` y su copia `tesis/main.tex`; identidad verificada por hash.

\- Validación: suite Release Win32, **140913 comprobaciones, 0 fallas**. `Build-Pico2.ps1` terminó con código 0, sin advertencias; flash 101708/4186112 bytes (2 %), RAM 22528/524288 bytes (4 %), RAM libre 501760 bytes. `git diff --check` correcto. Revisión de Embedded Firmware Engineer sin hallazgos; no se midió en hardware el lazo de 2 ms.

\- Grafo: `graphify update .` completado con código 0, 824 nodos y 1014 aristas.

\- LaTeX: `pdflatex` no disponible en PATH ni ubicaciones usuales; no se realizaron las dos pasadas solicitadas con esa herramienta. El compilador integrado `mcp__codex_app__compile_latex_document` devolvió `success` en dos llamadas sobre el texto final, sin diagnósticos retornados. No se dispone de log detallado para confirmar ausencia de referencias indefinidas.

\- Versión creada: **V1.5**, mediante `nueva-version.ps1`; salida "Version V1.5 creada" y existencia verificada de `..\MiYoke_PD_VERSIONES\V1_5\ACTUALIZACION_V1_5.txt`.

\- Desvíos: un único commit de código, pruebas y documentación por la regla superior DOCUMENTACION RELACIONADA, en lugar de los dos commits del plan. Compilación integrada como alternativa a `pdflatex`.

\- Sigue: revisión de Claude y validación del usuario en hardware. No integrado a `master`.

\## Revisión

\- Revisor: Claude (2026-10-11), con el Code Reviewer para código y pruebas. **APROBADO.**
  - **Código:** solo `BuffetModel.h`. `quadraticOnset` y las llamadas 100/150 (tren) y 80/140 (spoiler) son idénticas al diseño (§3.1). No queda ninguna referencia a `quadratic`.
  - **Pruebas:** solo los casos de §3.2, con valores esperados correctos. Suite recompilada por el revisor en un directorio temporal: PASS, 140 913 comprobaciones (140 908 + 5), 0 fallas.
  - **Tesis:** ley, tablas de picos (tren y spoiler) y ejemplos verificados numéricamente. Las notas describen el cambio. `main.tex` es idéntico a la fuente.
    - Observación: no hay `pdflatex` en esta PC. Codex compiló con su herramienta integrada ("success"); no fue verificable por el revisor.
  - **Versión V1.5:** tag en `4b9f5f6`, `ACTUALIZACION_V1_5.txt` con el mensaje pedido.
  - **Desvío aceptado:** un único commit de código y documentación en lugar de dos. Es coherente con la regla DOCUMENTACION RELACIONADA.
  - **Pendiente:** validación del usuario en hardware (buffet de tren nulo hasta 100 kt). Después, integración a master por pedido del usuario.

\- Integración (2026-10-11): el usuario validó V1.4 y V1.5 en hardware. `mejora/realismo-efectos` se integró a `master` por avance rápido (fast-forward). No se subió a GitHub.
