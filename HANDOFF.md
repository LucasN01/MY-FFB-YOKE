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

\## Última entrega

\- Agente: Codex (2026-10-10) — fase IMPLEMENTACIÓN del Flujo: realismo y fundamentación teórica de los efectos. **Detenida antes de modificar código por contradicción en la especificación cerrada.**

\- Rama: `mejora/realismo-efectos`, base de esta entrega `24b8865`. Precondición (§0) cumplida: árbol limpio salvo `graphify-ino/`, permitido por el pedido. No se modificó `master`.

\- Bloqueo que debe aclarar el autor del diseño:
  - [Diseño] `LEEME md/DISENO_realismo_efectos.md:103-118` (§2.2): los valores de caché comienzan en cero y `next()` calcula `expf`, `cosf` y `sqrtf` en la primera llamada. Sin embargo, la línea 124 afirma que el peor ciclo no paga ese costo.
  - [Diseño] `LEEME md/DISENO_realismo_efectos.md:258` y `:264` (§2.8): las llamadas a `next()` están dentro de `Effects::update()`; [INO] `MiYoke_PD.ino:130` y `:167` sitúan la actualización en el lazo de control de 2 ms. No se especifica precálculo fuera de ese lazo.
  - Se requiere aclarar si se acepta el costo de la primera llamada en el lazo o especificar el precálculo fuera del lazo. No se eligió ni implementó una solución, conforme a AGENTS.md y al punto 3 del pedido.

\- Commits: A (sin estado), B (con estado) y documentación funcional pendientes. Esta entrega contiene únicamente el commit de `HANDOFF.md` que registra el bloqueo.

\- Archivos modificados: solo `HANDOFF.md`, sección "Última entrega". Ningún archivo de código modificado. Documentación funcional y diseño sin cambios.

\- Validación: suite `tests/simconnect_checks.vcxproj` (Win32) no ejecutada; número total de comprobaciones no medido. Compilaciones Pico2 con potenciómetros y con `-Encoders` no ejecutadas; tamaños de programa y RAM no medidos frente a la referencia de 99 868 / 18 408 bytes. `.tex` sin cambios y compilación con `pdflatex` no ejecutada.

\- Versión: no creada; esta entrega modifica solo documentación y no activa el versionado de código.

\- Desvíos respecto del diseño: ninguno implementado. Sugerencias no aplicadas: ninguna adicional. Pendiente: aclaración del bloqueo por el autor del diseño, implementación, pruebas, documentación, versionado, revisión y validación en hardware.

\## Revisión

\- (pendiente)
