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

\- Agente: Codex (2026-10-11) — parte 2 del Plan para Codex **detenida por una prueba incompatible con el diseño, fuera de los casos autorizados**. Parte 2 incompleta; no se creó el commit B.

\- Rama: `mejora/realismo-efectos`. Base de esta sesión: `9fa96e9`. Precondición cumplida: existe commit A `d8ca14b` (Realismo A); árbol inicialmente limpio salvo `graphify-ino/`. Parte 1 sigue completada: suite Win32 PASS, 89 730 comprobaciones, 0 fallas; HANDOFF de esa entrega en `9fa96e9`. No se modificó `master`.

\- Bloqueo que debe aclarar Claude:
  - [Código] `tests/raw_checks.h:25` exige `std::fabs(u)<=.450f*.9f*.12f+.000001f` para la textura en pista sin aceleraciones: conserva el límite de pico de las senoides anteriores.
  - [Diseño] `LEEME md/DISENO_realismo_efectos.md:266-274` (§2.8.5) prescribe portadoras de ruido con RMS 0.7071, sin recorte individual, y conserva únicamente el recorte posterior de la suma a `±roadLevel`.
  - [Código] `SimEffects.h:78-83` implementa esa fórmula; la señal aleatoria puede superar el pico anterior y la suite efectivamente falla en el caso citado.
  - [Diseño] `LEEME md/DISENO_realismo_efectos.md:328` (§2.9) manda detenerse si falla otro caso, y la prueba de `raw_checks.h:25` no está entre los casos autorizados a cambiar. No se eligió entre modificar el modelo o modificar la prueba; no se relajó el límite. Se requiere aclaración del autor del diseño.

\- Cambios de código pendientes, SIN commit, conservados para verificar y retomar en la próxima sesión: `NarrowbandNoise.h` y `FrictionModel.h` (nuevos), `mis_defines.h` y `SimEffects.h`; `tests/simconnect_checks.cpp` adapta únicamente el caso de fricción por posición autorizado (línea original 109). Implementación del firmware de parte 2 realizada según el diseño; pruebas nuevas y flags en 0 todavía pendientes debido al bloqueo. `tests/raw_checks.h` no se modificó.

\- Validación: `tests/simconnect_checks.vcxproj`, MSBuild de Visual Studio 18 Community, Release Win32, `/t:Rebuild`, toolset v141: compila sin errores ni advertencias. Ejecución de `tests/bin/simconnect_checks.exe`: **FAIL line 25: std::fabs(u)<=.450f*.9f*.12f+.000001f**, salida 1. La suite se detiene en el primer fallo; total completo de comprobaciones no disponible. No se modificó la comprobación fallida ni ningún otro caso fuera de los autorizados. `git diff --check`: sin errores.

\- Compilaciones Pico2: `powershell -ExecutionPolicy Bypass -File Build-Pico2.ps1` (modo configurado, potenciómetros `YOKE_USE_SIMULATION=2`) y `-Encoders` terminan con salida 0, sin advertencias en sus salidas. Potenciómetros: programa **101 636 bytes** y RAM **22 528 bytes**; frente a 99 868 / 18 408: **+1768 / +4120 bytes**. Encoders: programa **101 796 bytes** y RAM **22 764 bytes**; frente a esa misma referencia: **+1928 / +4356 bytes**. No se cargó firmware en la placa. Estos resultados no resuelven el bloqueo de la suite ni habilitan commit B.

\- Revisión firmware de solo lectura: conforme al diseño. [Código] `NarrowbandNoise.h:12-28` precalcula coeficientes solo en constructor; `next()` y `reset()` no usan funciones trascendentales. [INO] `MiYoke_PD.ino:34` instancia global antes de `setup()`. [Código] `SimEffects.h:78-81`, `:113-115` agregan a lo sumo cinco pasos AR(2) por update; reinicios y dt=0 conservan lo especificado. `SimEffects.h:10` excluye fricción por posición en modo 1; `:157-165` conserva techo conjunto `0.4·axisGain`; `:174-177` conserva techos de suelo/fuerza. Sin bloqueos ni memoria dinámica añadidos. Tiempo máximo del lazo de 2 ms y `us=` no medidos en hardware.

\- Grafo: `graphify update .` ejecutado (AST, sin API), 814 nodos y 1002 aristas. `MiYoke_PD.ino` no cambió; no corresponde `graphify-sync.cmd`. `graphify-ino/` permanece fuera de los commits.

\- Documentación funcional y `.tex`: sin cambios, diferidos expresamente a parte 3. Compilación `.tex` no ejecutada. Versión no creada: el plan por sesiones reserva `nueva-version.ps1` para parte 3.

\- Commits de esta sesión: únicamente el registro del bloqueo en `HANDOFF.md`, en un commit separado. Commit B pendiente hasta aclarar el diseño y completar pruebas; no se presenta la implementación parcial como entrega aceptada.

\- Sigue: Claude debe aclarar el límite de la textura o autorizar el ajuste de esa prueba. Después, retomar parte 2 verificando los cambios pendientes, completar sus casos nuevos y flags en 0, obtener suite con 0 fallas y crear commit B. Parte 3 (documentación, versión y cierre) todavía no habilitada. Sugerencias fuera de alcance: ninguna.

\## Revisión

\- Revisor: Claude (2026-10-10) — aclaración del diseño ante el bloqueo de Codex.

\- Resolución: el bloqueo era correcto. Se corrigió el diseño: se especifica el precálculo fuera del lazo y no se acepta costo en el lazo.
  - §2.2: `Narrowband` recibe `(hz, stepSeconds, zeta, rms, seed)` en el constructor y calcula ahí `a1`, `a2` y `gain`. `next()` no recibe parámetros y solo ejecuta el paso AR(2) y el xorshift. `reset()` conserva los coeficientes.
  - §2.8.4, §2.8.5 y §2.8.6: los miembros `noise[3]` y `texNoise[2]` se construyen con sus frecuencias (13 / `SIMCONNECT_GEAR_BUFFET_HZ` / 15 y 7 / 11.3 Hz). Como `simEffects` es global (`MiYoke_PD.ino:34`), eso ocurre en la inicialización estática, antes de `setup()`. Las llamadas en `update` son `next()`.
  - §2.9 y §3.5: pruebas y criterio de costo ajustados a lo anterior.

\- Estado: diseño desbloqueado; continuar la implementación desde el punto 5 del pedido original.

\- Aclaración 2: Claude (2026-10-11), ante el bloqueo de la parte 2 (`8da61f6`). El bloqueo era correcto: el ruido de la textura podía superar el pico de las senoides que exige `tests/raw_checks.h:25`. Esa prueba es correcta: el pico ±0.12·E es la textura "suave" requerida por el usuario. Resolución en el diseño (§2.8.5): recortar la textura a ±0.12·roadLevel antes de sumarla, conservando RMS 0.7071. Simulado con las semillas del diseño (1000 pasos, E=1): se recorta el 2.5 %, extremos ±12.4 PWM. Cumple también `low*255<-10 && high*255>10`. No se autoriza modificar esa prueba.
