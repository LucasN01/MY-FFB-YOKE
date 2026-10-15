## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).

## Limitaciones del grafo en este proyecto
- MiYoke_PD.ino se indexa vía copia en graphify-ino/ (ejecutar graphify-sync.cmd tras editarlo).
- Las llamadas desde setup()/loop() a objetos globales NO aparecen en el grafo.
  Para trazar el flujo completo, leer MiYoke_PD.ino directamente.
- SimConnectBridge/*Model.h son wrappers de un include; la lógica está en la raíz.

## AHORRO DE TOKENS: graphify, PDF y LaTeX (regla del usuario)
- Codigo: antes de buscar o abrir archivos, orientarse con graphify (query/explain/path). La lectura directa es solo para lineas puntuales de un archivo ya identificado. Cada "Plan para Codex" le indica lo mismo.
- PDF: no leer el PDF directamente. Convertirlo con `markitdown archivo.pdf -o tmp/<nombre>.md` (markitdown instalado; tmp/ esta en .gitignore) y leer solo la seccion necesaria con grep. Si una tabla o formula sale desordenada, verificar solo esa pagina en el PDF.
- LaTeX (.tex): ya es texto, markitdown no aplica. La tesis de efectos esta dividida en tesis/capitulos/NN_*.tex; abrir solo el capitulo afectado y, dentro de el, solo las lineas ubicadas por \label o titulo. Verificar con compilar-tesis.ps1 (salida de una linea), nunca con pdflatex directo.
- Pruebas y compilacion: usar `powershell -ExecutionPolicy Bypass -File probar.ps1` (suite C++ + lanzador; agregar -Pico, -Encoders, -Potentiometers, -Bridge o -Todo segun el plan). Imprime una linea OK/FALLO por paso con solo las lineas de error. No invocar MSBuild, arduino-cli ni Build-*.ps1 directamente, no volcar logs completos y no repetir una compilacion que ya dio OK salvo que el codigo haya cambiado. En HANDOFF copiar solo esas lineas de resultado.
- Versionado: ejecutar nueva-version.ps1 siempre desde PowerShell (si falla la copia, el script lo detecta, borra la etiqueta y se repite).


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
- Tampoco generan version (regla del usuario, 2026-10-15) las herramientas del flujo entre agentes (lanzar-codex.py, compilar-tesis.ps1, nueva-version.ps1, graphify-sync.cmd) ni nada dentro de tesis/ (scripts de redaccion o analisis para el PDF). Si solo cambiaron esos, se commitean por nombre y no se ejecuta nueva-version.ps1 (el script igual los ignora y no crea version). Lista elegida por el usuario. Si aparece un script nuevo que podria ser del flujo, preguntar al usuario si se excluye antes de sumarlo a la lista $flujo de nueva-version.ps1 (Codex lo anota en HANDOFF para que Claude pregunte). Los .ps1/.cmd/.py del proyecto (lanzador, Build-*, ensayos, probar.ps1, tests/) si generan version.
- Para volver atras: git checkout Vx.y, o restaurar desde MiYoke_PD_VERSIONES\Vx_y.
- El agente revisor no crea versiones (no edita codigo).

## DOCUMENTACION RELACIONADA (regla del usuario)
Cuando un cambio de codigo modifica algo que esta descripto en documentacion (.md .tex .txt), esa documentacion se actualiza en la misma rama. Ejemplos: calculo de efectos o fuerzas, ganancias, filtros, limites, modelos (suelo, rafagas, turbulencia, buffet), protocolo serie, pines, configuracion de compilacion, uso del lanzador.
- Solo cuando sea necesario: actualizar unicamente los documentos cuyo contenido queda incorrecto o incompleto por el cambio, y solo las partes afectadas. No reescribir, reformatear ni "mejorar" documentos por las dudas.
- Documentos a revisar segun el tema: tesis de efectos = tesis/capitulos/NN_*.tex (un archivo por seccion; tesis/main.tex solo tiene preambulo e \input, es el que se sube a Overleaf), otros .tex de tesis/, LEEME md/*.md, LEEME.md y LEEME-*.md. No tocar ACTUALIZACION_Vx_y.txt ni los documentos de diseno de otras tareas.
- Diseno (Claude, modo Flujo): el documento de diseno incluye la seccion "Documentacion afectada" con archivo, seccion/ecuacion y que debe decir; o indica explicitamente "No afecta documentacion".
- Tesis en modo Flujo: cuando el diseno toque algo descripto en la tesis (tesis/*.tex, tesis/capitulos/NN_*.tex), Claude PREGUNTA al usuario si quiere modificarla antes de incluirla en "Documentacion afectada", indicando que capitulo y por que. Sin un si del usuario, la tesis no se toca. Criterio para recomendar: solo si cambia como se calculan los efectos o fuerzas (modelos, ecuaciones, constantes, ganancias, filtros, limites); herramientas de prueba, registros, scripts y lanzador van solo en los LEEME. Si el diseno no toca nada de la tesis, no se pregunta.
- La tesis (.tex) la redacta y edita Claude directamente, no Codex (regla del usuario, 2026-10-15; motivo: calidad de redaccion tecnica y menos tokens, porque antes Claude ya escribia el texto completo en el diseno y Codex solo lo volvia a leer y copiar). Momento (regla del usuario, 2026-10-15): al final, recien cuando la revision de Claude da "aprobado"; nunca en el diseno ni mientras haya correcciones pendientes, para no gastar tokens en texto que se descarta si la mejora no sale. La version que crea Codex no incluye ese cambio de tesis (es solo documentacion, no genera version nueva); queda en la rama y llega a master al integrarla. Pasos: ubicar el lugar con grep por \label o titulo, leer solo esas lineas, editar con Edit, ejecutar `powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1` (una linea OK/FALLO) y hacer commit por nombre solo del capitulo tocado (si ese archivo tiene cambios sin commit del usuario, preguntar antes). Sin subagente Technical Writer para estas ediciones (ahorro de tokens). En "Documentacion afectada" del diseno queda solo: "tesis/capitulos/NN_*.tex, seccion X: la actualiza Claude al aprobar la revision; Codex no la toca" (sin redactar el texto en el diseno). La redaccion describe lo que quedo implementado y aprobado, no lo disenado. Al terminar todo (tesis editada y commiteada), Claude PREGUNTA al usuario si quiere compilar main.tex; con un si ejecuta `powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1 -CopiarPdf`, que deja (o sobreescribe) tesis/main.pdf junto a main.tex (ignorado por git). Sin el si, no se genera el PDF. Si falla al copiar, suele ser porque el PDF esta abierto en un visor: pedir al usuario que lo cierre.
- Implementacion (Codex): actualiza los demas documentos (LEEME y otros .md/.txt) y los incluye por nombre en el mismo commit que el codigo, antes de ejecutar nueva-version.ps1, para que la copia de la version quede coherente. Codex no edita archivos de tesis/; si ve que falta algo en la tesis, lo anota en HANDOFF para Claude.
- Revision (Claude): verifica que la documentacion coincida con el codigo nuevo (formulas, constantes, unidades, nombres) y que no haya cambios innecesarios. Documentacion faltante o desactualizada = "requiere cambios".

## ROL DE CLAUDE EN EL FLUJO (regla del usuario)
- Claude disena, planifica y revisa. No implementa cambios de codigo salvo pedido explicito del usuario.
- Toda propuesta de mejora tiene fundamento verificable (TelemFFB, documentacion de SimConnect/Prepar3D, bibliografia tecnica) y cita la fuente. Si no hay fuente, se indica como hipotesis a validar con ensayo.
- Cada diseno aprobado se entrega a Codex como especificacion cerrada: archivos a modificar, cambio exacto, constantes y unidades, criterios de aceptacion, prueba a realizar y seccion "Documentacion afectada".
- Instrucciones para Codex en archivos, no en el chat: cada vez que Claude termina una fase de Flujo que deja trabajo para Codex (diseno nuevo, o revision con "requiere cambios"), escribe las instrucciones detalladas en HANDOFF.md, seccion "Plan para Codex" (rama, documento de diseno o hallazgos, precondiciones, secciones a leer, alcance, pruebas y compilaciones, mensajes de commit, comando exacto de nueva-version.ps1 y que completar en HANDOFF), y las commitea.
- FLUJO AUTOMATICO (regla del usuario, 2026-10-13): Claude lanza a Codex el mismo; el usuario no copia mensajes entre agentes.
  - Lanzamiento, siempre en segundo plano (run_in_background) para recibir el aviso al terminar, desde la carpeta del repo:
    `python lanzar-codex.py "Claude termino. Lee HANDOFF.md y hace la parte N del Plan para Codex." --add-dir "<carpeta PROYECTO MODIFICADO>" --add-dir "$TEMP" < /dev/null` (o "...y segui las instrucciones." si no hay partes). Deja la ultima respuesta en tmp/codex_ultimo.txt.
  - Nombre del chat en la app = titulo de la tarea (pedido del usuario, 2026-10-15): el lanzador lo toma de la linea "Tarea:" de la seccion "Plan para Codex" de HANDOFF.md (sin parentesis ni negritas) y antepone "Parte N - " si el mensaje pide una parte. Por eso cada Plan para Codex empieza con una linea "Tarea: <titulo corto que diga que se hace>". Para relanzar con correcciones de una revision, pasar `--titulo "Correcciones - <tarea>"`.
  - Por que no `codex exec` (2026-10-15): sus sesiones quedan con origen "exec" y la app de Codex no las muestra. lanzar-codex.py usa `codex app-server` (el protocolo de la app), asi cada lanzamiento queda como chat visible en el proyecto MY FFB YOKE de la app, con el mismo mensaje que antes pegaba el usuario. Equivale a `--approve-for-me` (workspace-write + auto_review) con `windows.sandbox="unelevated"` (el "elevated" de la config falla al lanzarse desde Claude). Modelo y esfuerzo: los de ~/.codex/config.toml. Si lanzar-codex.py falla, usar como respaldo `codex exec -C "<repo>" --approve-for-me -c 'windows.sandbox="unelevated"' --add-dir ... -o "<repo>/tmp/codex_ultimo.txt" "<mensaje>" < /dev/null` (no aparece en la app).
  - Permisos elegidos por el usuario: proyecto + aprobacion automatica. No usar --dangerously-bypass-approvals-and-sandbox.
  - Un solo Codex a la vez; mientras corre, Claude no edita ni commitea en la carpeta del repo.
  - Al aviso de fin: leer tmp/codex_ultimo.txt y HANDOFF ("Ultima entrega"), revisar como siempre (Code Reviewer), y relanzar con las correcciones o seguir con la parte siguiente.
  - Si Codex anota un bloqueo: Claude lo resuelve en el diseno y relanza; si es una decision del usuario (tesis, desvio del diseno, seguridad), le pregunta al usuario.
  - Al usuario se le escribe solo cuando debe actuar (cargar UF2, ensayos con hardware, decisiones suyas) o al terminar la tarea.
  - Si `codex exec` falla (no instalado, error de permisos o de red), avisar al usuario y volver al modo manual: darle el mensaje corto para pegar en Codex.
- Division en partes (ahorro de tokens): Claude decide segun el tamano. Tarea simple (pocos archivos, sin estado nuevo, pocas pruebas) = un solo plan sin partes. Tarea larga (muchos archivos o archivos nuevos, logica con estado, muchas pruebas, o riesgo de agotar el contexto de Codex) = partes numeradas, una por sesion nueva de Codex, cada una cerrada con commit y HANDOFF. Orden habitual: (1) codigo sin estado + pruebas, (2) codigo con estado + pruebas + firmware, (3) documentacion + version. nueva-version.ps1 solo en la ultima parte. Cada parte limita lo que Codex lee (secciones del diseno; en archivos grandes, solo las partes ubicadas por busqueda). Al dividir, Claude explica en una linea por que. Modelo recomendado: el de codigo con esfuerzo medium; mini/low para cambios triviales.

## SUBAGENTES (Agency Agents) – uso obligatorio por tipo de tarea
Usar el subagente indicado para cada tarea. Si la tarea abarca varias, encadenarlos en este orden: diseno → plan → (Codex implementa) → revision → validacion.
- Diseno o revision de firmware (lazo de 2 ms, ISR, HID/PID, filtros, PWM, temporizacion) → Embedded Firmware Engineer.
- Critica del plan de una mejora antes de pasarlo a Codex → Master Plan Architect. Su salida es el documento de diseno del modo Flujo, con la seccion "Documentacion afectada".
- Revision del codigo implementado por Codex → Code Reviewer. Resultado: "aprobado" o "requiere cambios" con lista concreta. No edita codigo ni crea versiones.
- Desglose en tareas, hitos y cronograma del proyecto → Senior Project Manager; seguimiento del avance → Project Shepherd.
- Diseno y registro de ensayos (hipotesis, condiciones, variables medidas, resultado) → Experiment Tracker.
- Analisis de logs del Serial, telemetria SimConnect o mediciones → Test Results Analyzer.
- Decidir si una mejora cumple su objetivo con evidencia → Reality Checker. Sin evidencia medida no se declara "resuelto".
- Redaccion de documentacion tecnica larga → Technical Writer, en registro formal y tecnico (las ediciones puntuales de la tesis las hace Claude directamente, ver DOCUMENTACION RELACIONADA), respetando la regla DOCUMENTACION RELACIONADA (solo partes afectadas, sin reescribir por las dudas).

## PRIORIDAD DE REGLAS
Las reglas de este archivo prevalecen sobre las convenciones propias de cualquier subagente o skill instalado (estilo de commits, ramas, formato de documentos, alcance de cambios). Ante conflicto, se aplica este archivo.
