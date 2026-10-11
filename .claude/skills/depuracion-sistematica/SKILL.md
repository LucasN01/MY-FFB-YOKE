---
name: depuracion-sistematica
description: Usar ante cualquier falla, prueba que no pasa, compilacion rota o comportamiento inesperado del yoke (firmware Pico, bridge SimConnect, lanzador, fuerzas raras, reinicios, latencia) ANTES de proponer una correccion. Exige encontrar la causa raiz con evidencia.
---

# Depuracion sistematica

Adaptado de `systematic-debugging` de obra/superpowers (licencia MIT, Jesse Vincent / Prime Radiant) al flujo de MiYoke_PD.

## Regla de hierro

```
NINGUNA CORRECCION SIN INVESTIGAR ANTES LA CAUSA RAIZ
```

Corregir el sintoma no es corregir la falla. Si no se completo la Fase 1, no se propone ninguna correccion.

## Encaje con el Flujo del proyecto

- **Claude**: hace las Fases 1 a 3 y entrega la correccion como especificacion cerrada en HANDOFF.md ("Plan para Codex"), con la causa raiz y la evidencia. No implementa salvo pedido explicito del usuario.
- **Codex**: si una prueba falla mientras implementa, aplica las Fases 1 a 3 solo dentro del alcance del plan. Si la causa esta fuera de lo especificado, se detiene y la anota en HANDOFF (sintoma, evidencia, hipotesis); no redisena.
- Analisis de logs del Serial, telemetria SimConnect o mediciones → subagente Test Results Analyzer. Falla de lazo de 2 ms, ISR, HID/PID o PWM → Embedded Firmware Engineer.

## Fase 1: Causa raiz

1. **Leer el error completo.** Salida de `probar.ps1` (linea FALLO y sus lineas de error), mensajes del Serial, codigo de diagnostico de reinicio del Pico. Anotar archivo, linea y codigo.
2. **Reproducir.** Pasos exactos y condicion (aeronave, fase de vuelo, eje, valor de fuerza). Si no se reproduce siempre, juntar mas datos; no adivinar.
3. **Revisar cambios recientes.** `git log`, `git diff Vx.y..HEAD`, HANDOFF ("Ultima entrega"). Si una version anterior funcionaba, comparar contra ella (`..\MiYoke_PD_VERSIONES\Vx_y`).
4. **Evidencia en cada frontera.** El sistema tiene varias capas: SimConnect → bridge (PC) → protocolo serie → Pico → calculo de fuerza → saturacion → PWM → motor/encoder. Antes de corregir, verificar que entra y que sale en cada frontera (log por capa, una sola corrida) para ubicar DONDE se rompe. En el firmware, el registro de diagnostico no debe bloquear el lazo de 2 ms (nada de Serial bloqueante ni String dentro del lazo).
5. **Rastrear el dato hacia atras.** De donde viene el valor malo, quien lo produjo, hasta encontrar el origen. Corregir en el origen. Para ubicar llamadas usar `graphify query/path/explain`; recordar que las llamadas desde setup()/loop() no estan en el grafo y hay que leer MiYoke_PD.ino.

## Fase 2: Patrones

- Buscar codigo similar que si funciona (otro eje, otro efecto, version anterior).
- Si se sigue una referencia (TelemFFB, documentacion de SimConnect/Prepar3D), leerla completa, no en diagonal.
- Listar todas las diferencias entre lo que anda y lo que no, aunque parezcan irrelevantes (unidades, signo, escala, tipo entero/flotante, periodo de muestreo).

## Fase 3: Hipotesis y prueba

1. **Una sola hipotesis, escrita:** "La causa es X porque Y (evidencia Z)".
2. **Prueba minima:** el cambio mas chico posible, una variable a la vez.
3. **Resultado:** si se confirma, Fase 4. Si no, hipotesis NUEVA; no apilar correcciones.
4. **Si no se entiende algo, decirlo.** Sin fuente ni medicion, se marca como hipotesis a validar con ensayo.

## Fase 4: Correccion

1. **Caso que falla primero.** Prueba en la suite C++ de `probar.ps1` cuando sea logica pura (filtros, modelos, saturacion, protocolo). Si depende del hardware, definir el ensayo (condicion, variable medida, resultado esperado) con Experiment Tracker.
2. **Una sola correccion**, en la causa raiz. Sin "ya que estoy" ni refactorizaciones de paso.
3. **Verificar** con la skill `verificacion-antes-de-terminar`.
4. **Si no funciona:** volver a la Fase 1 con la informacion nueva. **Con 3 correcciones fallidas, parar**: probablemente el problema es de diseno (estado compartido, temporizacion, arquitectura). Discutirlo con el usuario antes de un cuarto intento.

## Senales de alarma (volver a la Fase 1)

- "Lo arreglo rapido y despues investigo."
- "Pruebo cambiar X a ver si anda."
- Varios cambios juntos y despues compilar.
- "Seguro es X" sin evidencia.
- Proponer soluciones antes de rastrear el dato.
- "Un intento mas" despues de 2 fallidos.
- Cada correccion hace aparecer un problema nuevo en otro lugar.
- El usuario dice "no adivines", "eso no esta pasando?", "estamos trabados?".

## Cuando no hay causa raiz

Si la investigacion completa muestra que es del entorno, de temporizacion o externo (USB, simulador, driver): documentar lo investigado, aplicar el manejo adecuado (timeout, reintento, mensaje, watchdog) y dejar registro para el proximo ensayo. Casi siempre "no hay causa raiz" significa investigacion incompleta.
