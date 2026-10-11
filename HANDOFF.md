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

Tarea: umbral de inicio para el buffet de tren (100 kt) y de spoiler (80 kt). Rama `mejora/realismo-efectos`. Diseño: `LEEME md/DISENO_umbral_buffet_tren_spoiler.md`. Tarea simple, **sin partes**. Modelo recomendado: el de código, con esfuerzo medium.

1. Al empezar: `git status` y `git log -3`. El árbol debe estar limpio (salvo `graphify-ino/`).
2. Leé el diseño completo (es corto). Código: solo `BuffetModel.h` (§3.1). Pruebas: §3.2. Si otro caso falla, detenete y registralo aquí.
3. Suite `tests/simconnect_checks.vcxproj` (Release Win32) con 0 fallas; informá el total de comprobaciones. Compilá `Build-Pico2.ps1` sin advertencias nuevas.
4. Commit 1 por nombre (`BuffetModel.h` y pruebas): "Buffet tren y spoiler: umbral de inicio 100/80 kt con crecimiento por presion dinamica".
5. Documentación (§4): solo `tesis/explicacion_efectos_proyecto_ffb_yoke.tex`. No lo leas completo: ubicá cada parte con `grep -n`. Después copialo sobre `tesis/main.tex` y verificá con `diff`. pdflatex, dos pasadas. Commit 2: "Tesis: umbral de inicio de buffet de tren y spoiler".
6. Versión, con el árbol limpio:
   `powershell -ExecutionPolicy Bypass -File nueva-version.ps1 -Mensaje "Buffet de tren y spoiler: umbral de inicio 100 y 80 kt con crecimiento por presion dinamica"`
   Verificar "Version Vx.y creada" y que exista `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt`.
7. "Última entrega": commits, archivos, total de comprobaciones, tamaño de compilación, si el .tex compila, versión creada y desvíos. Commit de HANDOFF.md por nombre.

\## Última entrega

\- Agente: Claude (2026-10-11) — fase DISEÑO del Flujo: umbral de inicio del buffet de tren y de spoiler.

\- Origen: validación del usuario en hardware de V1.4. Todos los efectos funcionan bien, salvo el buffet de tren, que se siente desde baja velocidad (por ejemplo, 60 kt tras el despegue).

\- Diseño: `LEEME md/DISENO_umbral_buffet_tren_spoiler.md`.
  - Ley: `sat[(V²−V_on²)/(V_full²−V_on²)]`, con tren 100/150 kt (umbral elegido por el usuario) y spoiler 80/140 kt (umbrales de TelemFFB).
  - Revisado por el Master Plan Architect; sus hallazgos están incorporados (partes adicionales del `.tex`, tablas de picos y aclaración de atribución).

\- Historial de V1.4 (implementación, revisión y corrección de notas): commits hasta `915ade4`.

\## Revisión

\- (pendiente)
