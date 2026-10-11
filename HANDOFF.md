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

\- Agente: Codex (2026-10-11) — retomada la parte 2 con la aclaración del 2026-10-11, **detenida por otro caso incompatible con la fricción por posición y fuera de los cambios autorizados**. Parte 2 incompleta; commit B pendiente.

\- Rama: `mejora/realismo-efectos`. Base de esta sesión: `8a7d885`. Precondición cumplida: commit A `d8ca14b` presente; Parte 1 completada con 89 730 comprobaciones y 0 fallas, HANDOFF `9fa96e9`. Se verificaron y conservaron los cambios sin commit de la sesión anterior, sin rehacerlos. No se modificó `master`.

\- Aclaración aplicada: [Diseño] `LEEME md/DISENO_realismo_efectos.md:272-276` (§2.8.5); [Código] `SimEffects.h:83-85` ahora recorta la textura a `±0.12·roadLevel` antes de sumarla. `tests/raw_checks.h:25` permanece intacta y ese caso ya no bloquea la suite.

\- Nuevo bloqueo que debe aclarar Claude:
  - [Código] `tests/calibration_checks.h:371-375` activa las ganancias de damper/fricción, y en la línea 375 exige `fx.command(rx,1,1,YokeSC::Friction,0,-2)>0`: fuerza de fricción positiva por velocidad negativa con posición cero.
  - [Diseño] §2.7 y §2.8.7 prescribe fricción Hayward-Armstrong por posición cuando `YOKE_USE_SIMULATION!=1`. [Código] `FrictionModel.h:14-18` fija el ancla en la primera posición y devuelve cero; sin desplazamiento la fuerza sigue siendo cero. Esa prueba depende de la ley anterior por velocidad.
  - [Diseño] §2.9 solo autoriza adaptar los casos de fricción originales `simconnect_checks.cpp:109` y `:238-244`, y exige detenerse si falla otro caso. No autoriza modificar `tests/calibration_checks.h:375`, tampoco la aclaración del 2026-10-11.
  - Suite ejecutada y fallo confirmado. No se modificó la prueba ni se alteró la implementación para satisfacerla por cuenta propia; se requiere especificar su adaptación o aclarar el comportamiento esperado.

\- Archivos de código pendientes SIN commit: `NarrowbandNoise.h` y `FrictionModel.h` (nuevos), `mis_defines.h`, `SimEffects.h` y `tests/simconnect_checks.cpp`. Esta sesión añade únicamente el recorte de textura sobre la implementación heredada. El caso autorizado original de fricción de `simconnect_checks.cpp:109` ya estaba adaptado. `tests/raw_checks.h` y `tests/calibration_checks.h` no se modificaron. Pruebas nuevas y flags en 0 todavía pendientes, pues el plan exige detenerse ante el fallo ajeno.

\- Suite: `tests/simconnect_checks.vcxproj`, MSBuild de Visual Studio 18 Community, Release Win32, `/t:Rebuild`, toolset v141: compila sin errores ni advertencias. `tests/bin/simconnect_checks.exe`: **FAIL line 375: fx.command(rx,1,1,YokeSC::Friction,0,-2)>0**, salida 1, caso de `tests/calibration_checks.h`. Total completo de comprobaciones no disponible por detención en el primer fallo; no se afirma suite con 0 fallas. `git diff --check`: sin errores.

\- Compilaciones Pico2: `powershell -ExecutionPolicy Bypass -File Build-Pico2.ps1` (potenciómetros, modo configurado `YOKE_USE_SIMULATION=2`) y `-Encoders` terminan con salida 0, sin advertencias en sus salidas. Potenciómetros: programa **101 660 bytes**, RAM **22 528 bytes**; frente a referencia 99 868 / 18 408: **+1792 / +4120 bytes**. Encoders: programa **101 820 bytes**, RAM **22 764 bytes**; frente a la misma referencia: **+1952 / +4356 bytes**. No se cargó firmware en la placa. No resuelve el fallo de la suite ni habilita commit B.

\- Revisión firmware de solo lectura: conforme al diseño aclarado, sin hallazgos. [Código] `SimEffects.h:83-85` mantiene el pico suave y `:95` conserva el recorte global de suelo. Precálculo de coeficientes solo en constructor global antes de `setup()`; máximo cinco pasos AR(2) por update y dos actualizaciones de fricción por ciclo; reinicios y dt=0 conservados. Exclusión de fricción por posición en modo 1, techo conjunto `0.4·axisGain`, límites de suelo/fuerza/PWM intactos. Sin bloqueos ni memoria dinámica añadidos. Tiempo del lazo de 2 ms y `us=` pendientes de medida en hardware.

\- Grafo: `graphify update .` ejecutado, AST sin API, 814 nodos y 1002 aristas. INO sin cambios, no corresponde `graphify-sync.cmd`. `graphify-ino/` fuera de commits.

\- Documentación funcional y `.tex` sin cambios, diferidos a parte 3; no se compiló `.tex`. Versión no creada: regla por sesiones reserva `nueva-version.ps1` para parte 3.

\- Commit de esta sesión: únicamente `HANDOFF.md` para registrar el nuevo bloqueo. Código parcial conservado sin commit; no se creó commit B porque falta la suite aprobada y las pruebas nuevas.

\- Sigue: Claude debe aclarar el caso `tests/calibration_checks.h:375`. Después, retomar parte 2 verificando los cambios pendientes, completar los casos nuevos y flags en 0, obtener suite con 0 fallas y crear commit B. Parte 3 todavía no habilitada. Sin sugerencias fuera de alcance.

\## Revisión

\- Revisor: Claude (2026-10-10) — aclaración del diseño ante el bloqueo de Codex.

\- Resolución: el bloqueo era correcto. Se corrigió el diseño: se especifica el precálculo fuera del lazo y no se acepta costo en el lazo.
  - §2.2: `Narrowband` recibe `(hz, stepSeconds, zeta, rms, seed)` en el constructor y calcula ahí `a1`, `a2` y `gain`. `next()` no recibe parámetros y solo ejecuta el paso AR(2) y el xorshift. `reset()` conserva los coeficientes.
  - §2.8.4, §2.8.5 y §2.8.6: los miembros `noise[3]` y `texNoise[2]` se construyen con sus frecuencias (13 / `SIMCONNECT_GEAR_BUFFET_HZ` / 15 y 7 / 11.3 Hz). Como `simEffects` es global (`MiYoke_PD.ino:34`), eso ocurre en la inicialización estática, antes de `setup()`. Las llamadas en `update` son `next()`.
  - §2.9 y §3.5: pruebas y criterio de costo ajustados a lo anterior.

\- Estado: diseño desbloqueado; continuar la implementación desde el punto 5 del pedido original.

\- Aclaración 2: Claude (2026-10-11), ante el bloqueo de la parte 2 (`8da61f6`). El bloqueo era correcto: el ruido de la textura podía superar el pico de las senoides que exige `tests/raw_checks.h:25`. Esa prueba es correcta: el pico ±0.12·E es la textura "suave" requerida por el usuario. Resolución en el diseño (§2.8.5): recortar la textura a ±0.12·roadLevel antes de sumarla, conservando RMS 0.7071. Simulado con las semillas del diseño (1000 pasos, E=1): se recorta el 2.5 %, extremos ±12.4 PWM. Cumple también `low*255<-10 && high*255>10`. No se autoriza modificar esa prueba.

\- Aclaración 3: Claude (2026-10-11), ante el bloqueo `5cccbd5`. El bloqueo era correcto: `tests/calibration_checks.h:375` comprueba que activar Friccion en el menú produce fuerza, usando velocidad con posición constante; con el modelo por posición esa llamada solo fija el ancla. Se mantiene la intención (el interruptor del menú habilita la fricción) y se autoriza el patrón ancla + desplazamiento para esa y cualquier otra prueba de fricción del mismo tipo (ver Plan, parte 2). Revisadas también `simconnect_checks.cpp:264-270`: por saturación del centrado y dominio del damper, deberían seguir pasando sin cambios.
