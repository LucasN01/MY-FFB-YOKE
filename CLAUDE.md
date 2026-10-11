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
- Documentos a revisar segun el tema: tesis de efectos = tesis/capitulos/NN_*.tex (un archivo por seccion; tesis/main.tex solo tiene preambulo e \input, es el que se sube a Overleaf), otros .tex de tesis/, LEEME md/*.md, LEEME.md y LEEME-*.md. No tocar ACTUALIZACION_Vx_y.txt ni los documentos de diseno de otras tareas.
- Diseno (Claude, modo Flujo): el documento de diseno incluye la seccion "Documentacion afectada" con archivo, seccion/ecuacion y que debe decir; o indica explicitamente "No afecta documentacion".
- LaTeX como reemplazo exacto: para cada cambio en un .tex, Claude escribe en "Documentacion afectada" el archivo (tesis/capitulos/NN_*.tex), el TEXTO ACTUAL literal (fragmento corto y unico, copiado del archivo) y el TEXTO NUEVO completo, listo para pegar. Codex lo aplica tal cual con una busqueda del texto actual: no lee el capitulo entero, no redacta ni reformula. Si el texto actual no aparece exactamente, Codex no improvisa: lo anota en HANDOFF y sigue con el resto.
- Implementacion (Codex): actualiza esos documentos y los incluye por nombre en el mismo commit que el codigo, antes de ejecutar nueva-version.ps1, para que la copia de la version quede coherente. Si edita un .tex, ejecutar `powershell -ExecutionPolicy Bypass -File compilar-tesis.ps1` y anotar en HANDOFF su unica linea de resultado (OK/FALLO). No ejecutar pdflatex/latexmk directamente ni leer logs de LaTeX.
- Revision (Claude): verifica que la documentacion coincida con el codigo nuevo (formulas, constantes, unidades, nombres) y que no haya cambios innecesarios. Documentacion faltante o desactualizada = "requiere cambios".

## ROL DE CLAUDE EN EL FLUJO (regla del usuario)
- Claude disena, planifica y revisa. No implementa cambios de codigo salvo pedido explicito del usuario.
- Toda propuesta de mejora tiene fundamento verificable (TelemFFB, documentacion de SimConnect/Prepar3D, bibliografia tecnica) y cita la fuente. Si no hay fuente, se indica como hipotesis a validar con ensayo.
- Cada diseno aprobado se entrega a Codex como especificacion cerrada: archivos a modificar, cambio exacto, constantes y unidades, criterios de aceptacion, prueba a realizar y seccion "Documentacion afectada".
- Instrucciones para Codex en archivos, no en el chat: cada vez que Claude termina una fase de Flujo que deja trabajo para Codex (diseno nuevo, o revision con "requiere cambios"), escribe las instrucciones detalladas en HANDOFF.md, seccion "Plan para Codex" (rama, documento de diseno o hallazgos, precondiciones, secciones a leer, alcance, pruebas y compilaciones, mensajes de commit, comando exacto de nueva-version.ps1 y que completar en HANDOFF), y las commitea. Al usuario le da solo un mensaje corto para copiar y pegar en Codex: "Claude termino. Lee HANDOFF.md y segui las instrucciones." (tarea simple) o "Claude termino. Lee HANDOFF.md y hace la parte N del Plan para Codex." (tarea dividida), mas el modelo recomendado.
- Division en partes (ahorro de tokens): Claude decide segun el tamano. Tarea simple (pocos archivos, sin estado nuevo, pocas pruebas, sin ediciones grandes de .tex) = un solo plan sin partes. Tarea larga (muchos archivos o archivos nuevos, logica con estado, muchas pruebas, ediciones grandes de .tex, o riesgo de agotar el contexto de Codex) = partes numeradas, una por sesion nueva de Codex, cada una cerrada con commit y HANDOFF. Orden habitual: (1) codigo sin estado + pruebas, (2) codigo con estado + pruebas + firmware, (3) documentacion + version. nueva-version.ps1 solo en la ultima parte. Cada parte limita lo que Codex lee (secciones del diseno; en archivos grandes como los .tex, solo las partes por \label o titulo). Al dividir, Claude explica en una linea por que. Modelo recomendado: el de codigo con esfuerzo medium; mini/low para cambios triviales.

## SUBAGENTES (Agency Agents) – uso obligatorio por tipo de tarea
Usar el subagente indicado para cada tarea. Si la tarea abarca varias, encadenarlos en este orden: diseno → plan → (Codex implementa) → revision → validacion.
- Diseno o revision de firmware (lazo de 2 ms, ISR, HID/PID, filtros, PWM, temporizacion) → Embedded Firmware Engineer.
- Critica del plan de una mejora antes de pasarlo a Codex → Master Plan Architect. Su salida es el documento de diseno del modo Flujo, con la seccion "Documentacion afectada".
- Revision del codigo implementado por Codex → Code Reviewer. Resultado: "aprobado" o "requiere cambios" con lista concreta. No edita codigo ni crea versiones.
- Desglose en tareas, hitos y cronograma del proyecto → Senior Project Manager; seguimiento del avance → Project Shepherd.
- Diseno y registro de ensayos (hipotesis, condiciones, variables medidas, resultado) → Experiment Tracker.
- Analisis de logs del Serial, telemetria SimConnect o mediciones → Test Results Analyzer.
- Decidir si una mejora cumple su objetivo con evidencia → Reality Checker. Sin evidencia medida no se declara "resuelto".
- Redaccion de documentacion tecnica y de la tesis → Technical Writer, en registro formal y tecnico, respetando la regla DOCUMENTACION RELACIONADA (solo partes afectadas, sin reescribir por las dudas).

## PRIORIDAD DE REGLAS
Las reglas de este archivo prevalecen sobre las convenciones propias de cualquier subagente o skill instalado (estilo de commits, ramas, formato de documentos, alcance de cambios). Ante conflicto, se aplica este archivo.
