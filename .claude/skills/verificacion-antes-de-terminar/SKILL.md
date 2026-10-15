---
name: verificacion-antes-de-terminar
description: Usar justo antes de afirmar que algo esta terminado, corregido, aprobado o que compila/pasa las pruebas, y antes de commitear, crear version, cerrar HANDOFF o aprobar una revision. Exige evidencia fresca antes de cualquier afirmacion.
---

# Verificacion antes de terminar

Adaptado de `verification-before-completion` de obra/superpowers (licencia MIT, Jesse Vincent / Prime Radiant) al flujo de MiYoke_PD. Es la misma idea que la regla del Reality Checker: sin evidencia medida no se declara "resuelto".

## Regla de hierro

```
NINGUNA AFIRMACION DE EXITO SIN EVIDENCIA FRESCA
```

Si el comando de verificacion no se ejecuto en esta sesion, despues del ultimo cambio, no se puede decir que pasa.

## Compuerta

```
ANTES de afirmar cualquier estado:
1. IDENTIFICAR que comando o medicion prueba la afirmacion.
2. EJECUTARLO completo y despues del ultimo cambio.
3. LEER el resultado: linea OK/FALLO, codigo de salida, cantidad de fallas.
4. CONFIRMAR que el resultado respalda la afirmacion.
   - No: informar el estado real con la evidencia.
   - Si: afirmar CON la evidencia (copiar la linea de resultado).
5. Recien ahi, afirmar.
```

## Que evidencia corresponde en este proyecto

| Afirmacion | Evidencia necesaria | No alcanza |
|---|---|---|
| Pasan las pruebas | `powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1` (con -Pico, -Encoders, -Potentiometers, -Bridge o -Todo segun el plan): todas las lineas OK | Una corrida anterior al ultimo cambio, "deberia pasar" |
| Compila el firmware / bridge | Linea OK del paso correspondiente de `probar.ps1` | Que el editor no marque errores |
| Compila la tesis | `powershell -ExecutionPolicy Bypass -File herramientas/flujo/compilar-tesis.ps1`: linea OK | "El LaTeX se ve bien" |
| Version creada | Salida "Version Vx.y creada" y existe `..\MiYoke_PD_VERSIONES\Vx_y\ACTUALIZACION_Vx_y.txt` | Que el commit exista |
| Falla corregida | El sintoma original ya no ocurre (prueba o ensayo que antes fallaba y ahora pasa) | Se cambio el codigo, se supone corregido |
| Mejora de sensacion/fuerza cumple su objetivo | Medicion o log del ensayo (Test Results Analyzer) y decision del Reality Checker | Que compile y pase la suite |
| Codex termino la parte | `git log`/`git diff` muestran los cambios esperados y HANDOFF tiene las lineas de resultado | Que HANDOFF diga "listo" |
| Diseno/plan cumplido | Releer criterios de aceptacion del diseno y tildar uno por uno | "Las pruebas pasan" |
| Documentacion coherente | Formulas, constantes, unidades y nombres del .tex/.md coinciden con el codigo nuevo | Que se haya editado el documento |

Reglas de ahorro: no repetir una compilacion que ya dio OK si el codigo no cambio desde entonces; en HANDOFF copiar solo las lineas de resultado, no logs completos.

## Senales de alarma

- Usar "deberia", "probablemente", "parece que".
- Decir "listo", "perfecto", "aprobado" antes de verificar.
- Commitear, crear version o cerrar HANDOFF sin haber corrido las pruebas del plan.
- Confiar en el informe de otro agente (Codex o subagente) sin mirar el diff y los resultados.
- Verificacion parcial (un paso de `probar.ps1` cuando el plan pedia -Todo).
- "Por esta vez no hace falta."

## Para Claude en la revision

Antes de escribir "aprobado": revisar el diff real, confirmar las lineas OK de las pruebas pedidas (o correrlas si el codigo cambio despues), verificar la documentacion afectada y la version creada. Si falta cualquiera de esas evidencias, el resultado es "requiere cambios".
