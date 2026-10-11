---
name: recibir-revision
description: Usar al recibir hallazgos de una revision de codigo o de diseno ("requiere cambios" de Claude, comentarios de un subagente revisor, observaciones del usuario) antes de aplicarlos, sobre todo si algun punto es confuso o parece tecnicamente incorrecto. Exige verificar cada punto contra el codigo, no aceptar ni aplicar a ciegas.
---

# Recibir una revision

Adaptado de `receiving-code-review` de obra/superpowers (licencia MIT, Jesse Vincent / Prime Radiant) al flujo de MiYoke_PD.

Principio: verificar antes de aplicar, preguntar antes de suponer. La correccion tecnica importa mas que quedar bien.

## Quien recibe que

- **Codex** recibe los hallazgos de Claude en HANDOFF.md ("requiere cambios"). Los aplica dentro del alcance indicado, uno por uno, con cambio minimo.
- **Claude** recibe hallazgos de subagentes (Code Reviewer, Embedded Firmware Engineer, Master Plan Architect, Reality Checker) y observaciones del usuario sobre un diseno.
- El **usuario** decide ante cualquier conflicto de arquitectura o de criterio.

## Secuencia

```
1. LEER todos los hallazgos sin aplicar nada.
2. ENTENDER: reformular cada punto con palabras propias.
3. VERIFICAR contra el codigo real (graphify para ubicar, lectura puntual de lineas).
4. EVALUAR: es correcto para ESTE proyecto (Pico, lazo de 2 ms, SimConnect, protocolo serie)?
5. RESPONDER: confirmacion tecnica o objecion razonada.
6. APLICAR de a uno, probando cada uno.
```

## Puntos confusos

Si algun punto no se entiende, no aplicar ninguno todavia: los hallazgos suelen estar relacionados y entender a medias lleva a implementar mal.

- Codex: anota en HANDOFF que puntos entendio y cuales necesitan aclaracion, y se detiene en esos.
- Claude: pide aclaracion al subagente o al usuario antes de cambiar el diseno.

## Verificacion de cada hallazgo

Antes de aplicar, comprobar:
1. Es tecnicamente correcto para este codigo?
2. Rompe algo que ya funciona (otra prueba, otro eje, otro efecto, el protocolo)?
3. Hay una razon para la implementacion actual (decision de diseno previa, documentada en HANDOFF o en la tesis)?
4. Respeta las restricciones del firmware (nada bloqueante ni memoria dinamica en el lazo, saturar antes del PWM, tipos y unidades)?
5. El revisor conoce el contexto completo?

Si un hallazgo parece incorrecto: objetarlo con razones tecnicas (prueba, linea de codigo, fuente como TelemFFB o la documentacion de SimConnect). Si no se puede verificar, decirlo: "No puedo verificarlo sin X". Si contradice una decision previa del usuario, parar y consultar.

## YAGNI

Si la revision pide "hacerlo bien" agregando funcionalidad (opciones, configuraciones, registros extra), buscar primero si se usa. Si no se usa, proponer no agregarlo. Para Codex vale ademas la regla Minimal Change Engineer: lo que parezca mejorable y no este en los hallazgos se anota en HANDOFF como sugerencia, sin aplicarlo.

## Orden de aplicacion

1. Aclarar todo lo confuso.
2. Bloqueantes (no compila, fuerza sin saturar, riesgo para el hardware, falla de seguridad).
3. Correcciones simples.
4. Correcciones complejas (logica, estado).
5. Probar cada una por separado con `probar.ps1` y verificar que no haya regresiones (skill `verificacion-antes-de-terminar`).

## Como responder

- Correcto: "Corregido: [que cambio y donde]." Sin agradecimientos ni elogios ("tenes razon!", "excelente observacion"); el cambio muestra que se entendio.
- Objecion equivocada: "Lo verifique y era correcto: [motivo]. Lo aplico." Sin largas disculpas.
- En HANDOFF: por cada hallazgo, estado (aplicado / objetado con motivo / pendiente de aclaracion).
