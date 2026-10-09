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

\## Última entrega

\- Agente: Claude (2026-10-09) — fase DISEÑO del Flujo

\- Rama: `mejora/usb-id-pico2` (desde `master` @ 42ae7e3)

\- Tarea: fijar el ID USB 2E8A:000F en el firmware (TinyUSB usa 239A:CAFE y el lanzador no encuentra la placa). Diseño: `LEEME md/DISENO_usb_id_pico2.md`. Para Codex: implementar exactamente el bloque de 9 líneas en `MiYoke_PD.ino` entre las líneas 102 y 103, compilar con Build-Pico2.ps1, graphify-sync y crear versión.

\- Archivos modificados: `LEEME md/DISENO_usb_id_pico2.md` (nuevo), `HANDOFF.md`. Sin cambios de código; sin versión.

\- Pendiente / riesgos: implementación (Codex), revisión (Claude), validación en hardware (usuario: COM con VID_2E8A&PID_000F y lanzador OK). `mejora/prueba-flujo` (V1.2) sigue sin integrar a master.


\## Revisión

\- Revisor:

\- Hallazgos:

\- Estado: (aprobado / requiere cambios)

