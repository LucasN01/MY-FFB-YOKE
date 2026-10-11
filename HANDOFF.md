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

## VERSIONADO OBLIGATORIO Y AUTOMATICO (regla del usuario)
Cada vez que un agente (Claude o Codex) modifique codigo (.h .hpp .c .cpp .ino .ps1 .cmd .py .vcxproj), DEBE crear una version nueva sin que el usuario lo pida. Pasos, en este orden:
1. Hacer commit por nombre de los archivos modificados (nunca git add . ni git add -A).
2. Ejecutar: powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "<una linea breve, sin tildes, que diga que se modifico>"
3. Verificar que imprima "Version Vx.y creada" y que exista ..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt.
4. Informar al usuario el numero de version creada en la respuesta final.
Reglas:
- Esta tarea se hace ANTES de dar el trabajo por terminado. Si no se creo la version, la tarea esta incompleta.
- Numeracion: 1.0, 1.1 ... 1.9, 2.0, 2.1 ... (la asigna el script; no inventar numeros ni crear carpetas a mano).
- Si el script se detiene por cambios sin commit, resolver con commit por nombre y repetir; no saltarse el script.
- Cambios solo de documentacion (.md, .tex, .txt) no generan version.
- Para volver atras: git checkout Vx.y, o restaurar desde MiYoke_PD_VERSIONES\Vx_y.
- El agente revisor no crea versiones (no edita codigo).

## DOCUMENTACION RELACIONADA (regla del usuario)
Cuando un cambio de codigo modifica algo que esta descripto en documentacion (.md .tex .txt), esa documentacion se actualiza en la misma rama. Ejemplos: calculo de efectos o fuerzas, ganancias, filtros, limites, modelos (suelo, rafagas, turbulencia, buffet), protocolo serie, pines, configuracion de compilacion, uso del lanzador.
- Solo cuando sea necesario: actualizar unicamente los documentos cuyo contenido queda incorrecto o incompleto por el cambio, y solo las partes afectadas. No reescribir, reformatear ni "mejorar" documentos por las dudas.
- Documentos a revisar segun el tema: tesis/explicacion_efectos_proyecto_ffb_yoke.tex y otros .tex de tesis/, LEEME md/*.md, LEEME.md y LEEME-*.md. No tocar ACTUALIZACION_Vx_y.txt ni los documentos de diseno de otras tareas.
- Diseno (Claude, modo Flujo): el documento de diseno incluye la seccion "Documentacion afectada" con archivo, seccion/ecuacion y que debe decir; o indica explicitamente "No afecta documentacion".
- Implementacion (Codex): actualiza esos documentos y los incluye por nombre en el mismo commit que el codigo, antes de ejecutar nueva-version.ps1, para que la copia de la version quede coherente. Si edita un .tex, anotar en HANDOFF si compila.
- Revision (Claude): verifica que la documentacion coincida con el codigo nuevo (formulas, constantes, unidades, nombres) y que no haya cambios innecesarios. Documentacion faltante o desactualizada = "requiere cambios".

\## Plan para Codex

Sin tareas pendientes para Codex (2026-10-11). Sigue la validación del usuario en hardware.

\## Última entrega

- Agente: Codex (2026-10-11) — **Plan para Codex completado**, corrección de notas de documentación tras la revisión de V1.4, sin partes.

- Rama: `mejora/realismo-efectos`. Commit de los dos `.tex`: `5612d0a` — "Tesis: notas de actualizacion especificas (que cambio y por que)". Este cierre de HANDOFF.md se registra en un commit posterior por nombre. No se integró a master.

- Archivos modificados: `tesis/explicacion_efectos_proyecto_ffb_yoke.tex`, copia idéntica `tesis/main.tex` y este HANDOFF.md. Sin cambios de código ni pruebas.

- Notas, por cada `.tex`: **12 eliminadas** dentro de `sec:realismo` y **102 reescritas**, de 114 originales a 102 notas específicas. Cada nota externa indica antes → ahora, motivo, referencia corta tomada de la tabla "Cambios y fundamentos" y remisión a `sec:realismo`; los ejemplos señalan la ley con la que se recalcularon. No quedan notas genéricas ni autorreferencias dentro de la sección de realismo.

- Marcadores, por cada `.tex`: **34 eliminados**; 22 ya estaban sueltos (el plan estimaba unos 19) y 12 quedaron sin contenido al quitar las notas internas. Se conservaron los marcadores que preceden contenido modificado.

- Validación de alcance: comparación automática contra HEAD anterior, excluyendo sólo notas y marcadores, confirma que el resto de líneas no cambió: texto, fórmulas, tablas y números intactos. `git diff --check` sin errores. Copia a `tesis/main.tex` verificada con `git diff --no-index` (salida 0) y comparación byte a byte.

- LaTeX: **compila**, `pdflatex -interaction=nonstopmode -halt-on-error`, todas las pasadas con salida 0. Se hicieron las dos pasadas pedidas y una tercera para estabilizar el índice y la paginación; la última no tiene referencias indefinidas ni aviso de recompilar. PDF final de **102 páginas**. Persisten avisos de cajas y de babel/hyperref preexistentes; ninguna nota nueva tiene Overfull. Se inspeccionaron visualmente páginas 22, 35, 48, 66, 88 y 96 (centrado, turbulencia, textura, pérdida, damper y fricción), con notas legibles y referencias resueltas. PDF, logs, métricas y renders en `tmp/notas-realismo-tex/`, fuera de los commits.

- Versión: **no se creó una nueva**, conforme al plan y a la excepción de documentación; continúa V1.4. No corresponde `graphify-sync.cmd` ni actualización AST manual al no modificar código. El hook de commit lanza su reconstrucción habitual; `graphify-ino/` permanece sin seguimiento y fuera de los commits.

- Sin ambigüedades ni cambios fuera de alcance. Sigue revisión de Claude de esta corrección documental y validación del usuario en hardware antes de integrar a master.

\## Revisión

\- Revisor: Claude (2026-10-10) — aclaración del diseño ante el bloqueo de Codex.

\- Resolución: el bloqueo era correcto. Se corrigió el diseño: se especifica el precálculo fuera del lazo y no se acepta costo en el lazo.
  - §2.2: `Narrowband` recibe `(hz, stepSeconds, zeta, rms, seed)` en el constructor y calcula ahí `a1`, `a2` y `gain`. `next()` no recibe parámetros y solo ejecuta el paso AR(2) y el xorshift. `reset()` conserva los coeficientes.
  - §2.8.4, §2.8.5 y §2.8.6: los miembros `noise[3]` y `texNoise[2]` se construyen con sus frecuencias (13 / `SIMCONNECT_GEAR_BUFFET_HZ` / 15 y 7 / 11.3 Hz). Como `simEffects` es global (`MiYoke_PD.ino:34`), eso ocurre en la inicialización estática, antes de `setup()`. Las llamadas en `update` son `next()`.
  - §2.9 y §3.5: pruebas y criterio de costo ajustados a lo anterior.

\- Estado: diseño desbloqueado; continuar la implementación desde el punto 5 del pedido original.

\- Aclaración 2: Claude (2026-10-11), ante el bloqueo de la parte 2 (`8da61f6`). El bloqueo era correcto: el ruido de la textura podía superar el pico de las senoides que exige `tests/raw_checks.h:25`. Esa prueba es correcta: el pico ±0.12·E es la textura "suave" requerida por el usuario. Resolución en el diseño (§2.8.5): recortar la textura a ±0.12·roadLevel antes de sumarla, conservando RMS 0.7071. Simulado con las semillas del diseño (1000 pasos, E=1): se recorta el 2.5 %, extremos ±12.4 PWM. Cumple también `low*255<-10 && high*255>10`. No se autoriza modificar esa prueba.

\- Aclaración 3: Claude (2026-10-11), ante el bloqueo `5cccbd5`. El bloqueo era correcto: `tests/calibration_checks.h:375` comprueba que activar Friccion en el menú produce fuerza, usando velocidad con posición constante; con el modelo por posición esa llamada solo fija el ancla. Se mantiene la intención (el interruptor del menú habilita la fricción) y se autoriza el patrón ancla + desplazamiento para esa y cualquier otra prueba de fricción del mismo tipo (ver Plan, parte 2). Revisadas también `simconnect_checks.cpp:264-270`: por saturación del centrado y dominio del damper, deberían seguir pasando sin cambios.

\- Revisión final de V1.4: Claude (2026-10-11), con el Code Reviewer para código y pruebas.
  - **Código y pruebas: APROBADO.** Suite recompilada y ejecutada de forma independiente (Release Win32): PASS, 140 908 comprobaciones, 0 fallas, sin advertencias.
    - Conforme con §2.1–§2.8 del diseño: fórmulas, constantes, flags, condiciones de la escala del damper, fricción por posición solo con sensor local, techo conjunto 0.4, coeficientes AR(2) en el constructor, textura con recorte ±0.12·√E y `GroundModel.h` sin cambios.
    - Sin cambios en los archivos prohibidos de §0.
    - Pruebas modificadas solo en los casos autorizados; los casos nuevos de §2.9 están presentes. `legacy_effects_checks.cpp` verifica bit a bit los flags en 0 (§3.1).
  - **Versión: OK.** Tag `V1.4` → `9861ae0`. `ACTUALIZACION_V1_4.txt` con el mensaje pedido. La copia coincide con el tag (salvo los finales de línea CRLF). Los `.tex` son idénticos entre sí y el `.tex` compila.
  - **Documentación (`.md`): OK.** `CALCULO_PICO_SC8.md`, `CAMBIOS_REALISMO.md` y `MODELO_TURBULENCIA.md` coinciden con el código.
  - **Documentación (`.tex`): REQUIERE CAMBIOS (menor).**
    - Contenido técnico correcto. Se recalculó el ejemplo de pérdida: 10.584°, R 0.3207, E 0.4188, 14/7 PWM, pico 23/12; coincide.
    - Las 114 notas visibles usan solo 3 textos genéricos y no dicen qué cambió ni por qué, como pide §6 del diseño y el usuario.
    - Hay notas dentro de `sec:realismo` que remiten a sí misma, y ~19 marcadores `% [V-realismo]` sueltos.
    - Corrección: ver "Plan para Codex".
  - **Sugerencias de código diferidas** (cosméticas, sin efecto en el comportamiento; no se aplican ahora para no generar una versión solo por comentarios):
    1. `SimEffects.h:182-188`: indentar `noise[3]`, `texNoise[2]` y su comentario como el resto de `private:`.
    2. `SimEffects.h:71-73`: actualizar el comentario a "Narrow-band 7/11.3 Hz texture (or fixed sines with flag 0); amplitude sqrt(E) of the 0.5..40 kt ramp (ISO 8608)."
  - **Estado:** requiere cambios menores de documentación. Después: validación del usuario en hardware (sección 4 del diseño) y recién entonces integrar a master.

\- Revisión de la corrección de notas: Claude (2026-10-11). **APROBADO.**
  - Commit `5612d0a`: 102 notas específicas (antes → ahora; motivo; referencia), ninguna duplicada ni genérica, ninguna dentro de `sec:realismo`. 0 marcadores sueltos.
  - El diff, excluyendo notas y marcadores, no tiene otras líneas cambiadas. `main.tex` es idéntico. Compila (informado por Codex: 102 páginas).
  - Observación: la copia `..\MiYoke_PD_VERSIONES\V1_4` conserva el `.tex` anterior a esta corrección, porque los cambios solo de documentación no generan versión. El `.tex` vigente es el de la rama.
- **Estado final de la tarea: aprobado.** Pendiente: validación del usuario en hardware (sección 4 del diseño). Después, integración a master por pedido del usuario.
