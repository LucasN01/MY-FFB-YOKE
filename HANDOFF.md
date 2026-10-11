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

Tarea: realismo y fundamentación teórica de los efectos. Rama `mejora/realismo-efectos`. Diseño: `LEEME md/DISENO_realismo_efectos.md`. Modelo recomendado: el de código, con esfuerzo medium.

Hacé **solo la parte que te pidan**, cada una en una sesión nueva. Al empezar, revisá `git status`, `git log -5` y "Última entrega". Si quedaron cambios sin commit de una sesión anterior, verificalos contra el diseño y completalos, sin rehacerlos. Al terminar cada parte: commit por nombre y "Última entrega" actualizada con la parte completada, commits, archivos, total de comprobaciones de la suite, desvíos o dudas y qué parte sigue; después, commit de HANDOFF.md. Ante una ambigüedad o contradicción, detenete y registrala aquí.

\### Parte 1: commit A (envolventes y centrado, sin estado)
- Estado conocido (2026-10-10): una sesión anterior dejó cambios sin commit en BuffetModel.h, GustModel.h, PicoForceModels.h, SimEffects.h, mis_defines.h, tests/raw_checks.h y tests/simconnect_checks.cpp.
- Leé del diseño solo:
  - §0, §2.3, §2.4, §2.5 y §2.6.
  - De §2.1, solo `SIMCONNECT_CENTERING_AERO_STIFFNESS` y el comentario de `SIMCONNECT_CENTERING_MIN_STIFFNESS`.
  - De §2.8, solo el punto 3 (centrado).
  - De §2.9, solo los casos de estos cambios: buffets, raw_checks, turbulencia, PicoForceModels y centrado.
- Alcance: GustModel.h, PicoForceModels.h (factor √Q, tauHigh, textura con raíz), BuffetModel.h, el centrado en SimEffects.h y la constante en mis_defines.h, más sus pruebas. Todavía NO: NarrowbandNoise.h, FrictionModel.h, la escala del damper, los flags nuevos ni la documentación.
- Suite `tests/simconnect_checks.vcxproj` (Win32) con 0 fallas. Actualizá solo los casos que el diseño lista como "cambian por diseño".
- Commit: "Realismo A: envolventes de turbulencia, buffets, textura y centrado con fundamento teorico". Sin versión.

\### Parte 2: commit B (ruido, fricción y damper, con estado)
- **Aclaración 2026-10-11 (bloqueo `5cccbd5`), autorización general para pruebas de fricción:** toda prueba existente que llame a `command`/`commandWithPositionSource` con `Friction` en la máscara esperando fricción por velocidad, con posición constante, puede adaptarse con el mismo patrón ya autorizado para `simconnect_checks.cpp:109`. Primero una llamada en la posición inicial, que fija el ancla; luego la misma llamada con la posición desplazada ±0.05 en el sentido de la velocidad original (velocidad negativa → posición −0.05), conservando el signo y la intención esperados. Ejemplo: `tests/calibration_checks.h:375` pasa a `fx.command(rx,1,1,YokeSC::Friction,0,0);` y `CHECK(fx.command(rx,1,1,YokeSC::Friction,-.05f,0)>0);`. No cambiar signos esperados, cotas ni casos de otros efectos. Las pruebas de `motionAdjustedForce` no cambian (la función conserva la fricción por velocidad). Si alguna prueba de fricción no puede expresarse así, detenete. Listar en HANDOFF cada prueba adaptada.
- **Aclaración 2026-10-11 (bloqueo `8da61f6`):** la textura de pista se recorta a ±0.12·E (ver §2.8.5 del diseño). `tests/raw_checks.h:25` NO se modifica: debe pasar tal cual. Retomá los cambios sin commit (NarrowbandNoise.h, FrictionModel.h, mis_defines.h, SimEffects.h, tests/simconnect_checks.cpp), aplicá ese recorte, completá las pruebas nuevas y los flags en 0, y seguí con el resto de esta parte.
- Requisito: debe existir el commit "Realismo A". Si no existe, detenete.
- Leé del diseño solo:
  - §0, el resto de §2.1, §2.2 (coeficientes solo en el constructor), §2.7 y §2.8 puntos 1, 2 y 4 a 8.
  - De §2.9, los casos de Narrowband, FrictionModel, límite conjunto, escala del damper, flags en 0 y las líneas 109 y 238-244.
  - §3.
- Alcance: NarrowbandNoise.h y FrictionModel.h (nuevos), los flags y `static_assert` de mis_defines.h, y SimEffects.h. Fricción Hayward–Armstrong solo con `YOKE_USE_SIMULATION!=1`; techo conjunto de damper y fricción 0.4. Más sus pruebas.
- Suite con 0 fallas. Compilar `Build-Pico2.ps1` y también con `-Encoders`, sin advertencias nuevas; anotar programa y RAM frente a 99 868 / 18 408 bytes.
- Commit: "Realismo B: ruido de banda estrecha, friccion Hayward-Armstrong y damper escalado". Sin versión y sin tocar documentación.

\### Parte 3: documentación, versión y cierre
- Requisito: deben existir los commits "Realismo A" y "Realismo B". Si falta alguno, detenete.
- Leé del diseño solo §6, §7 y §4. Para los números de los ejemplos, usá las fórmulas ya implementadas en el código.
- `tesis/explicacion_efectos_proyecto_ffb_yoke.tex`: NO lo leas completo. Ubicá cada parte de §6.1 B con `grep -n` de su `\label` o título y leé solo esas líneas. Agregá la sección de §6.1 A (`\label{sec:realismo}`), los comentarios `% [V-realismo]` y las notas visibles. Después copiá el archivo editado sobre `tesis/main.tex` y verificalo con `diff`.
- Solo lo indicado en `LEEME md/CALCULO_PICO_SC8.md` (§6.2), `CAMBIOS_REALISMO.md` (§6.2b) y `MODELO_TURBULENCIA.md` (§6.3).
- pdflatex, dos pasadas; anotar si compila.
- Commit: "Realismo: documentacion y fundamentacion teorica en tesis y LEEME".
- Versión, con el árbol limpio (salvo graphify-ino/):
  `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Mejora basada en incorporar realismo a los efectos y fundamentacion teorica: turbulencia, buffets, centrado, damper, friccion y suelo"`
  Verificar "Version Vx.y creada" y que exista `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt`.
- "Última entrega" final con las tres partes: commits, tamaños, si el .tex compila, versión creada, desvíos y sugerencias.

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
