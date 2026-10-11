## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

When the user types `/graphify`, use the installed graphify skill or instructions before doing anything else.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- Dirty graphify-out/ files are expected after hooks or incremental updates; dirty graph files are not a reason to skip graphify. Only skip graphify if the task is about stale or incorrect graph output, or the user explicitly says not to use it.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).

## AHORRO DE TOKENS: graphify, PDF y LaTeX (regla del usuario)
- Codigo: antes de buscar o abrir archivos, orientarse con `graphify query`/`explain`/`path`. Leer directamente solo las lineas puntuales de un archivo ya identificado.
- PDF: no leer el PDF directamente. Convertirlo con `markitdown archivo.pdf -o tmp/<nombre>.md` (instalado; tmp/ esta en .gitignore) y leer solo la seccion necesaria con grep. Si una tabla o formula sale desordenada, verificar solo esa pagina.
- LaTeX (.tex): la tesis de efectos esta dividida en tesis/capitulos/NN_*.tex; abrir solo el capitulo afectado y, dentro de el, solo las lineas ubicadas con grep por \label o titulo. Aplicar los cambios como reemplazo exacto del texto que da el diseno. Verificar con compilar-tesis.ps1 (salida de una linea), nunca con pdflatex directo.
- Pruebas y compilacion: usar `powershell -ExecutionPolicy Bypass -File probar.ps1` (suite C++ + lanzador; agregar -Pico, -Encoders, -Potentiometers, -Bridge o -Todo segun el plan). Imprime una linea OK/FALLO por paso con solo las lineas de error. No invocar MSBuild, arduino-cli ni Build-*.ps1 directamente, no volcar logs completos y no repetir una compilacion que ya dio OK salvo que el codigo haya cambiado. En HANDOFF copiar solo esas lineas de resultado.
- Versionado: ejecutar nueva-version.ps1 siempre desde PowerShell (si falla la copia, el script lo detecta, borra la etiqueta y se repite).

## Limitaciones del grafo en este proyecto
- MiYoke_PD.ino se indexa vía copia en graphify-ino/ (ejecutar graphify-sync.cmd tras editarlo).
- Las llamadas desde setup()/loop() a objetos globales NO aparecen en el grafo.
  Para trazar el flujo completo, leer MiYoke_PD.ino directamente.
- SimConnectBridge/*Model.h son wrappers de un include; la lógica está en la raíz.

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

## ROL DE CODEX EN EL FLUJO (regla del usuario)
- Codex implementa especificaciones disenadas por Claude. No re-analiza ni redisena.
- Si la especificacion es ambigua, incompleta o contradice el codigo existente, se detiene y lo informa en HANDOFF en lugar de suponer.
- Al terminar, el HANDOFF indica: archivos modificados, documentacion actualizada, si compila (y si el .tex compila, cuando corresponda) y la version creada.
- Instrucciones: las escribe Claude en HANDOFF.md, seccion "Plan para Codex"; el mensaje del usuario solo indica leerlo (y que parte hacer, si esta dividido).
- Trabajo por sesiones: si el plan esta dividido en partes, cada sesion hace solo la parte pedida y termina con commit por nombre y HANDOFF ("Ultima entrega": parte completada, commits, pruebas, que sigue). Al empezar, revisar git status/log y HANDOFF; si hay cambios sin commit de una sesion anterior, verificarlos contra el diseno y completarlos, sin rehacerlos. nueva-version.ps1 solo en la parte final.
- Ahorro de contexto: leer solo las secciones del diseno indicadas en el mensaje. En archivos grandes (por ejemplo los capitulos de tesis/capitulos/) ubicar las partes con busqueda por \label o titulo y leer solo esas lineas, nunca el archivo completo. Si el contexto se esta agotando, cerrar la parte en curso con commit y HANDOFF antes de seguir.

## AGENTES (Agency Agents) – aplicacion obligatoria
- Toda implementacion → Minimal Change Engineer: cambiar solo lo especificado. Sin refactorizaciones, renombres, reformateos ni "mejoras" adicionales. Lo que parezca mejorable se anota en HANDOFF como sugerencia, sin aplicarlo.
- Sin subagentes de revision: Codex no lanza revisores (Code Reviewer, Embedded Firmware Engineer u otros) ni repite revisiones; la revision la hace Claude despues. Los perfiles de esta lista se aplican como criterios propios al escribir el codigo.
- Codigo de firmware → Embedded Firmware Engineer: respetar el lazo de control de 2 ms, nada bloqueante dentro del lazo (delay, Serial bloqueante, String), sin memoria dinamica en el lazo, saturar la fuerza antes de convertirla a PWM, mantener tipos y unidades de la especificacion.
- Commits y versionado → Git Workflow Master, limitado a la regla VERSIONADO OBLIGATORIO Y AUTOMATICO: commit por nombre de archivo y nueva-version.ps1. Prohibido: git add . / git add -A, rebase, reescritura de historial, force push, crear o borrar tags a mano y cambiar la estrategia de ramas sin pedido del usuario.

## PRIORIDAD DE REGLAS
Las reglas de este archivo prevalecen sobre las convenciones propias de cualquier agente o skill instalado (estilo de commits, ramas, alcance de cambios, formato de documentos). Ante conflicto, se aplica este archivo.
