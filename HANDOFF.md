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

Tarea: corrección de documentación tras la revisión de V1.4 (realismo). Rama `mejora/realismo-efectos`. Tarea simple, **sin partes**. Modelo recomendado: el de código, con esfuerzo medium. **Solo documentación:** no tocar código ni pruebas, y no crear versión (los cambios de `.tex` no generan versión).

1. Archivos: `tesis/explicacion_efectos_proyecto_ffb_yoke.tex` y, al final, la misma copia en `tesis/main.tex` (copiar el archivo y verificar con `diff`). No leas el `.tex` completo: ubicá cada nota con `grep -n "textit{Actualización (realismo)"` y leé solo unas líneas alrededor de cada una.
2. Notas visibles: hoy hay 114 notas con 3 textos genéricos. El diseño (§6, `LEEME md/DISENO_realismo_efectos.md`) pide `<qué cambió y por qué>`.
   a) **Dentro de la sección `\label{sec:realismo}`** (desde `\section{Actualización: realismo...}` hasta antes de `\section{CENTRADO DINÁMICO (RESORTE)}`): eliminar todas las notas `\textit{Actualización (realismo)...}`, porque esa sección ya es la explicación.
   b) **En el resto del documento:** reemplazar cada nota genérica por una sola oración específica para esa subsección, con el formato `\textit{Actualización (realismo): <antes> $\rightarrow$ <ahora>; <por qué, con la referencia corta>; ver Sección~\ref{sec:realismo}.}`. Tomar el "antes", el "ahora" y la referencia de la tabla "Cambios y fundamentos" de `sec:realismo`. Ejemplos:
      - Turbulencia, presión: `$Q_T=\min(Q,1)$ $\rightarrow$ $S_V=\sqrt Q\le2$; la carga de ráfaga es $\propto\rho Vw$ (Pratt--Walker)`.
      - Turbulencia, pasaaltos: `$\tau_H=1$ s $\rightarrow$ $\tau_H=29.5/V_e$ en [0.25, 1.5] s; decaimiento por plunge (Hoblit)`.
      - Pérdida, inicio: `$\alpha_s(0.5+0.2f)$ $\rightarrow$ $\alpha_s[V_s/(V_s+\Delta)]^2$; margen de aviso de la antigua §23.207 (hipótesis)`.
      - Fricción: `ley racional por velocidad $\rightarrow$ modelo Hayward--Armstrong por posición (sensor local); adherencia y presliding`.
      - Si en una subsección solo se recalcularon números de ejemplo: `ejemplos recalculados con <ley nueva>`.
      - Si varias notas seguidas de la misma subsección quedarían iguales, dejar una sola, al final de la subsección.
3. Marcadores: eliminar los `% [V-realismo]` que quedaron solos (seguidos de una línea vacía o de otro marcador, sin contenido modificado debajo). Hay unos 19. Conservar los que preceden a contenido modificado.
4. No cambiar ningún otro texto, fórmula, tabla ni número.
5. Compilar con pdflatex, dos pasadas, sin referencias indefinidas. Copiar a `tesis/main.tex` y verificar que son idénticos.
6. Commit por nombre (ambos `.tex`): "Tesis: notas de actualizacion especificas (que cambio y por que)".
7. HANDOFF, "Última entrega": cantidad de notas eliminadas y reescritas, marcadores eliminados, si compila y el commit. Commit de HANDOFF.md por nombre.

\## Última entrega

\- Agente: Codex (2026-10-11) — **las tres partes del Plan para Codex completadas**, incluidas las aclaraciones del 2026-10-11. Esta sesión hizo únicamente la parte 3: documentación, versión y cierre.

\- Rama: `mejora/realismo-efectos`. Commits: A `d8ca14b` — "Realismo A: envolventes de turbulencia, buffets, textura y centrado con fundamento teorico"; B `3b407d5` — "Realismo B: ruido de banda estrecha, friccion Hayward-Armstrong y damper escalado"; documentación `9861ae0` — "Realismo: documentacion y fundamentacion teorica en tesis y LEEME". Entregas anteriores: `9fa96e9` (parte 1) y `63f2264` (parte 2). Este registro se entrega en un commit posterior de HANDOFF.md. No se modificó ni integró a master.

\- Archivos de código y pruebas de A: `BuffetModel.h`, `GustModel.h`, `PicoForceModels.h`, `SimEffects.h`, `mis_defines.h`, `tests/raw_checks.h`, `tests/simconnect_checks.cpp`. De B: `NarrowbandNoise.h`, `FrictionModel.h`, `mis_defines.h`, `SimEffects.h`, `tests/calibration_checks.h`, `tests/simconnect_checks.cpp`, `tests/simconnect_checks.vcxproj`, `tests/realism_checks.h`, `tests/legacy_effects_checks.cpp`, `tests/sim_position_friction_checks.cpp`. Parte 3 no modificó código.

\- Documentación actualizada: `tesis/explicacion_efectos_proyecto_ffb_yoke.tex`, copia idéntica `tesis/main.tex`, `LEEME md/CALCULO_PICO_SC8.md`, `LEEME md/CAMBIOS_REALISMO.md`, `LEEME md/MODELO_TURBULENCIA.md`. Se agregó `sec:realismo` con tabla de cambios, derivaciones, hipótesis de hardware, mediciones reproducibles de ruido y referencias del diseño; se corrigieron únicamente las fórmulas, ejemplos y explicaciones afectadas, con comentarios `% [V-realismo]` y notas visibles. Los tres MD tienen sólo los cambios indicados en §6.2, §6.2b y §6.3. Datos geométricos/peso del R182 verificados en el POH local (figura 1-1 y páginas impresas ii/1-5).

\- Revisión de documentación contra el código: conforme. Distingue fricción por posición local y ley por velocidad en modo SimConnect; documenta el recorte individual del damper antes del límite conjunto, textura ±0.12·sqrt(E), suelo combinado ±sqrt(E), y portadoras triangulares sólo con flags en 0. Límites de PWM y ganancias de suelo conservados.

\- LaTeX: **compila**, dos pasadas finales de `pdflatex -interaction=nonstopmode -halt-on-error`, ambas salida 0; PDF de **102 páginas**, sin referencias indefinidas ni aviso de recompilar en la segunda pasada. `tesis/main.tex` y la fuente principal son idénticos (`git diff --no-index`, salida 0). Persisten avisos Overfull/Underfull de cajas, principalmente rutas/código preexistentes; tablas y secciones afectadas verificadas visualmente. Artefactos y logs en `tmp/parte3-tex/`, fuera del commit. `git diff --check` sin errores.

\- Suite parte 1: **89 730 comprobaciones, 0 fallas**. Suite final parte 2: `tests/simconnect_checks.vcxproj`, MSBuild Visual Studio 18 Community, Release Win32, `/t:Rebuild`, toolset v141; sin errores ni advertencias. Ejecución **PASS, 140 908 comprobaciones, 0 fallas**, salida 0; incluye pruebas de flags en 0 y modo SimConnect con fricción por velocidad. Parte 3 sólo cambió documentación y conserva esos resultados; no se repitieron compilaciones de código.

\- Pruebas existentes adaptadas en B según autorización: `tests/simconnect_checks.cpp`, caso original línea 109, fija ancla en 0 y desplaza −0.05 conservando fuerza positiva; rama anterior conserva prueba por velocidad. El caso de reposo posterior usa máscara Damper para limpiar el ancla antes de exigir cero. `tests/calibration_checks.h`, caso original línea 375, fija ancla y desplaza −0.05 al activar Friction desde menú, manteniendo signo e intención. `motionAdjustedForce`, cotas y casos originales 238-244 intactos; `tests/raw_checks.h:25` intacto en B y pasa. Pruebas nuevas de AR(2), reset, dt=0, fricción, techo conjunto y escala del damper; unidades de variantes aisladas para respetar ODR.

\- Compilaciones Pico2 del firmware final, salida 0 y sin advertencias: `Build-Pico2.ps1`, potenciómetros (`YOKE_USE_SIMULATION=2`), programa **101 660 bytes**, RAM **22 528 bytes**; referencia 99 868 / 18 408: **+1792 / +4120 bytes**. `-Encoders`: programa **101 820 bytes**, RAM **22 764 bytes**; misma referencia: **+1952 / +4356 bytes**. No se cargó firmware en la placa. Revisión firmware conforme: coeficientes AR(2) precalculados antes de setup, máximo cinco pasos de ruido y dos actualizaciones de fricción por ciclo; sin bloqueo ni memoria dinámica añadidos al lazo. Tiempo máximo de 2 ms y `us=` pendientes de medición en hardware.

\- Versión **V1.4 creada** por `nueva-version.ps1` con el mensaje exacto del plan, después del commit de documentación y con árbol limpio salvo `graphify-ino/`. Se verificó la salida "Version V1.4 creada", el tag en `9861ae0` y `..\MiYoke_PD_VERSIONES\V1_4\ACTUALIZACION_V1_4.txt`. La copia de versión incluye código y documentación coherentes; este cierre de HANDOFF es posterior al tag, según el plan.

\- Grafo: `graphify update .` ejecutado al finalizar cambios de código en parte 2 (AST, sin API); parte 3 sólo documentación. INO sin cambios, no corresponde `graphify-sync.cmd`. `graphify-ino/` permanece sin seguimiento y fuera de los commits.

\- Desvío mínimo necesario para compilar la tesis: se corrigió el error preexistente `\!left` a `\!\left` en una expresión afectada; no hubo rediseño ni cambios de código en parte 3. Sin dudas pendientes ni sugerencias adicionales fuera de alcance. Sigue revisión de Claude y validación del usuario en hardware antes de integrar a master.

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
