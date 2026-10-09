# Diagnóstico del rodaje SC7: reinicios por tiempo de recepción repetido

Fuente: `SimConnectBridge/bin/Win32/RealismoSC7/telemetry.csv`, registro de
5.670 muestras y 57,047 segundos analizado el 8 de octubre de 2026.
No se modificaron el CSV, el puente ni el firmware durante este análisis.

## Resultado

Las aceleraciones del simulador sí contienen sacudidas. El modelo de suelo
implementado pierde casi toda su respuesta porque reinicia los filtros ante
intervalos de recepción de cero milisegundos. La siguiente muestra con intervalo
positivo sólo inicializa el filtro; los mensajes posteriores del mismo lote
vuelven a reiniciarlo. El error está en cómo se interpreta el tiempo de llegada.

## Evidencia del registro

| Medida | Resultado |
|---|---:|
| Muestras | 5.670 |
| Duración entre primera y última muestra | 57,047 s |
| Muestras en suelo | 5.670 |
| Muestras armadas | 5.669 |
| Muestras en pausa | 0 |
| Muestras rodando a más de 0,5 kt y armadas | 4.339 |
| Velocidad máxima | 83,262 kt |
| Muestras con envolvente de velocidad igual a 1 | 2.759 |
| Aceleración vertical mínima durante rodaje | -16,70368 ft/s² |
| Aceleración vertical máxima durante rodaje | 19,45085 ft/s² |
| Intervalos entre filas con dt = 0 | 4.518 / 5.669 (79,70 %) |
| Marcas de tiempo distintas | 1.152 |
| Grupos de exactamente cinco filas con la misma marca | 1.094 |
| Grupos con aceleraciones distintas dentro de la misma marca | 949 |
| Muestras rodando con fuerzas de suelo distintas de cero a la precisión CSV | 8 / 4.339 |
| Muestras con Vibracion_Tierra = 1 | 6 |

No deben eliminarse las filas repetidas por tiempo suponiendo que toda la
telemetría es idéntica: dentro de muchos lotes las aceleraciones sí cambian.
La marca registrada es el reloj del PC durante el callback, no el instante
original de cada paso de física del simulador.

Ejemplo: en la línea 3.678 del CSV, a 37,062 s del inicio, se registra
velocidad 54,234 kt, envolvente 1, armado 1 y aceleración vertical
-10,99921 ft/s². Ambas fuerzas de suelo están en cero. Las filas alrededor
comparten la misma marca 527769843 ms a pesar de tener aceleraciones distintas.

## Cadena causal en el código

1. `SimConnectBridge/main.cpp` toma `GetTickCount64()` en cada callback y calcula
   `dt = (now-lastData)*0.001`; después actualiza `lastData`.
2. Varias muestras procesadas dentro del mismo tick producen `dt=0`.
3. `GroundModel.h::Model::update` trata `dt<=0` como invalidez: llama a `reset()`
   y devuelve fuerzas nulas.
4. La siguiente muestra con tiempo positivo encuentra `ready=false`, establece
   la referencia y vuelve a devolver cero.
5. El patrón se repite durante prácticamente todo el registro.

La envolvente 0,5–40 kt funciona y llega a uno. El estado de armado permanece
activo. Ningún intervalo observado llega a 250 ms. Los reinicios por tiempo
cero bastan para explicar la desaparición de la respuesta calculada.

## Reproducción independiente

Se reprodujeron en Python las ecuaciones actuales con los tiempos, velocidades
y aceleraciones guardados. La diferencia máxima respecto de ground_roll/pitch
del CSV fue 0,00000505, compatible con el redondeo de sus columnas de entrada
y salida. Esto confirma el diagnóstico del algoritmo sobre esta captura.

Como experimento, se conservó la última muestra de cada marca de tiempo y se
procesó una sola vez por grupo con su intervalo positivo real. No se cambiaron
la mezcla, los filtros, la escala 0,25 g, las ganancias ni la curva de velocidad.
El experimento produjo máximos absolutos de señal de aproximadamente 0,798 en
roll y 1 en pitch. Con las ganancias confirmadas por el usuario (suelo 0,450 y
total 0,90), ambos alcanzan sus techos de 40 PWM roll y 4 PWM pitch. Con los
valores predeterminados 0,18 y 0,90, los picos serían 33 y 4 PWM.
Es una simulación de una política posible, no PWM medido ni validación de una
corrección final. Agrupar muestras pierde detalle y debe evaluarse al diseñar
el arreglo; no recupera los tiempos originales de simulación.

## Por qué el LCD pudo mostrar cero

En el cálculo actual, aun suponiendo que todas las filas se enviaran, los picos
aislados de suelo serían aproximadamente 11 PWM roll y 2 pitch con las ganancias
confirmadas por el usuario: suelo 0,450 y total 0,90. Sólo cinco filas superarían
media cuenta de roll, y cuatro de pitch.
La condición de envío de al menos 20 ms puede descartar varias de esas filas.
El CSV no registra cada envío ni su confirmación, por lo que no permite
determinar exactamente qué órdenes llegaron a la placa.

Además, el LCD toma una lectura cada 100 ms y puede omitir pulsos breves.
No es la causa principal: la mayor parte de las fuerzas ya es cero en Windows.
El CSV tampoco informa el PWM total físico ni confirma si el interruptor del efecto
estaba habilitado; las estimaciones anteriores corresponden al aporte aislado usando
las ganancias declaradas por el usuario, suelo 0,450 y total 0,90.

## Corrección recomendada

- Separar tiempo de recepción de tiempo de actualización del modelo. No borrar
  el filtro por mensajes del mismo tick. Procesar los lotes de forma coherente,
  usando tiempo de simulación válido o agrupación con un reloj de modelo propio.
- No reemplazar arbitrariamente dt=0 por 1 ms: alteraría la respuesta temporal
  sin conocer el intervalo real entre muestras.
- Coordinar cálculo y envío para no descartar sistemáticamente los transitorios.
- Añadir una regresión con lotes de cinco muestras de tiempo repetido, además
  de las pruebas anteriores de pausa, parada, datos inválidos y pérdida de datos.
- Mantener la rampa 0,5–40 kt y límites actuales. Ajustar sensibilidad sólo
  después de corregir el tiempo y realizar una nueva prueba de rodaje.
- Registrar dt de modelo, reinicios y fuerzas efectivamente enviadas para
  distinguir cálculo, transporte y visualización en el próximo ensayo.

Las columnas spoiler_left/right contienen NaN en esta captura. Es una observación
separada: no causa los reinicios de suelo identificados aquí; el buffet de spoilers
permanece cero durante este recorrido en tierra.
