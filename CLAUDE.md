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

## Versionado del proyecto (reglas propias)
- Tras aprobar la revisión, validar el usuario en hardware e integrar a master, ejecutar:
  powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "<una línea: qué cambió>"
- No crear copias de carpetas a mano ni versionar sin pasar por el script.
- Para volver atrás: git checkout V1.x, o restaurar desde MiYoke_PD_VERSIONES\V1_x.
- Nunca usar git add . ni git add -A (la carpeta graphify-ino/ no se versiona).
