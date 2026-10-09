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

\- Agente: Codex (2026-10-09) — implementación del diseño USB Pico 2.

\- Rama: `mejora/usb-id-pico2`.

\- Tarea: implementado exactamente `LEEME md/DISENO_usb_id_pico2.md`. [INO] `MiYoke_PD.ino:103`: bloque de 9 líneas que fija USB 2E8A:000F mediante TinyUSBDevice.setID y reconecta si mounted(), antes de Serial.begin.

\- Archivos modificados: `MiYoke_PD.ino` y `HANDOFF.md`. Diff contra master: únicamente 9 líneas agregadas en el .ino, cero eliminadas, CRLF conservado; ningún otro archivo de código cambiado. Sin discrepancias con el diseño.

\- Compilación: `powershell -ExecutionPolicy Bypass -File Build-Pico2.ps1`, FQBN `rp2040:rp2040:rpipico2:usbstack=tinyusb,arch=arm`, completada sin errores ni advertencias. Programa: 99.868 bytes (2%; +120 respecto de la compilación inicial de 99.748). RAM: 18.408 bytes (3%; sin cambio), 505.880 libres. Firmware generado en `build-pico2`; no cargado en la placa.

\- Versionado: commit por nombre `4f555c1`; `graphify-sync.cmd` ejecutado correctamente (incluye `graphify update .`). Versión **V1.3** creada con el mensaje exacto del diseño; salida `Version V1.3 creada` y existencia de `..\MiYoke_PD_VERSIONES\V1_3\ACTUALIZACION_V1_3.txt` verificadas. `graphify-ino/` no se versionó. Esta entrega se registra en un commit posterior de documentación.

\- Pendiente / riesgos: revisión de Claude y validación del usuario en hardware: COM con `USB\VID_2E8A&PID_000F&MI_00`, lanzador identifica `YOKE17SC8`, fuerzas SimConnect y LCD sin cambios. Windows podría asignar otro COM. No integrado a master. `mejora/prueba-flujo` (V1.2) sigue pendiente por separado.

\## Revisión

\- Revisor: Claude (2026-10-09) — fase REVISIÓN del Flujo

\- Hallazgos:
  1. OK. `git diff master...HEAD --stat`: solo `MiYoke_PD.ino` (+9, -0), `LEEME md/DISENO_usb_id_pico2.md` (diseño) y `HANDOFF.md`. Commits: fadebad (diseño), 4f555c1 (código), a85eae0 (entrega). Opcional de LEEME_PICO2.md no aplicado (aceptable).
  2. OK. [INO] `MiYoke_PD.ino:103-111`: bloque idéntico al diseño (condición, literales 0x2E8A/0x000F, sangrías, comentarios), entre el `#endif` de analogWrite (102) y `#if DEBUG_MOTION_TELEMETRY...` (112). CRLF conservado.
  3. OK. Tag anotado `V1.3` -> 4f555c1. `..\MiYoke_PD_VERSIONES\V1_3\ACTUALIZACION_V1_3.txt` existe; CAMBIO y RAMA correctos. Copia `V1_3\MiYoke_PD.ino` idéntica al working tree (sin CR). `graphify-ino/` sincronizado y sin versionar.
  4. OK. Compilación: `build-pico2/MiYoke_PD.ino.bin` de 17:41 (antes del tag de 17:42), 118.732 bytes vs 118.612 de V2.4 = +120, coincide con lo informado por Codex. No recompilado por el revisor.
  5. OBSERVACIÓN (no bloquea, no es error de Codex): ACTUALIZACION_V1_3 dice VERSION ANTERIOR V1.2 y lista `D LEEME md/DISENO_prueba_flujo.md` y un cambio de 10 líneas en el .ino. Es porque `nueva-version.ps1:11,33` toma la etiqueta de número más alto (V1.2, que está en otra rama) y no la anterior de esta rama. El cambio real respecto de master es solo +9 líneas (`git diff master V1.3 --stat`). Propuesta para una tarea aparte: usar `git describe --tags --abbrev=0 HEAD^` como versión anterior.
  6. Integración: al combinar con `mejora/prueba-flujo` no hay solape (comentario arriba de `void setup()` vs bloque en 103-111).

\- Estado: aprobado (pendiente validación del usuario en hardware: subir `build-pico2/MiYoke_PD.ino.uf2` o compilar con TinyUSB; COM con `USB\VID_2E8A&PID_000F&MI_00`; `INICIAR SIMULADOR.cmd` identifica `YOKE17SC8`; fuerzas y LCD sin cambios).

