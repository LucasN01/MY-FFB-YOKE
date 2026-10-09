# Ensayos aislados de centrado, amortiguación y fricción

Ganancia total confirmada por el usuario: **60 %**. Centrado: roll 0,400, pitch 0,800, reposo 0,150. En los otros ensayos se declaran los restantes efectos en cero y, respectivamente, damper 0,120 o fricción 0,030. Los CSV no almacenan estas ganancias: proceden de la declaración del usuario.

## Resultado principal

Los registros contienen movimientos de ambos ejes suficientes para estudiar las entradas del modelo. No contienen la respuesta de la Pico, por lo que no permiten afirmar que los efectos hayan salido correctamente por PWM ni medir el error entre predicción y salida. No hay motivo demostrado por estos archivos para cambiar las ganancias todavía.

| Registro | Filas | Tiempo entre primera y última muestra, incluidas interrupciones | Máxima IAS con armed=1 | Máxima q/qRef con armed=1 |
|---|---:|---:|---:|---:|
| Centrado | 23.984 | 351,500 s | 84,01 kt | 0,55734 |
| Damper | 7.184 | 143,453 s | 75,22 kt | 0,39954 |
| Fricción | 11.096 | 154,359 s | 87,26 kt | 0,53493 |

## Qué debería producir cada ensayo

Los valores siguientes son **cálculos**, no mediciones de PWM. Para centrado se calcula la respuesta estática en cada fila armada, omitiendo el transitorio del filtro de rigidez. Para damper/fricción se reconstruye una secuencia aproximada con intervalos de al menos 20 ms, posición cuantizada a 0,0001 y el estimador de velocidad de YokeMotion. No se conocen los instantes reales de envío/recepción USB ni los mensajes entre muestras, por lo que la reconstrucción no es una reproducción exacta de la placa.

| Efecto | Pico absoluto calculado roll, PWM equivalente | Pico absoluto calculado pitch, PWM equivalente |
|---|---:|---:|
| Centrado, estático | 30,97 | 73,50 |
| Damper, reconstrucción aproximada | 30,60 | 61,20 |
| Fricción, reconstrucción aproximada | 2,22 | 4,39 |

### Centrado

`k = 0,150 + 0,850*(q/qRef)`

`PWM_R_equiv = -255*0,60*0,400*k*xR`

`PWM_P_equiv = -255*0,60*0,800*k*xP`

Con q=0 y desplazamiento completo se esperan 9,18 PWM equivalentes en roll y 18,36 en pitch. Con igual posición, aumentar q aumenta la carga restauradora. El efecto no necesita movimiento: sostener el mando desplazado mantiene el comando.

En este ensayo q máxima=1300,96 Pa, aproximadamente 89,59 kt EAS. IAS máxima=84,01 kt: IAS y EAS no son la misma magnitud. **No hay muestras por encima de qRef**, por lo que el ensayo no verifica la eliminación de la meseta anterior de 120 kt EAS. No se debe concluir que falló: ese rango simplemente no se ensayó.

### Damper

La velocidad v está expresada en recorridos centro-extremo por segundo y pasa por el filtro de 40 ms. Sea `m=max(abs(v)-0,02,0)`.

`PWM_P_equiv = -sign(v)*255*0,60*min(0,120*m,0,400)`

Roll aplica la mitad de esa ganancia. Esta relación entre ejes continúa siendo propia del damper; la nueva ganancia independiente Centrado roll sólo modifica el centrado.

El techo del término de movimiento aparece a `abs(v) >= 3,3533` recorridos/s. En la reconstrucción se alcanza en ambos ejes: los máximos son aproximadamente 31 PWM roll y 61 PWM pitch después del redondeo. Por eso un movimiento muy rápido no produce un crecimiento ilimitado. Esto es consecuencia del límite implementado, no evidencia de fuerza física correcta.

La verificación pendiente en la salida real es que el signo se oponga al movimiento, también al atravesar el centro, y que la salida desaparezca al detener el mando tras actualizarse el estimador.

### Fricción

`PWM_P_equiv = -sign(v)*255*0,60*0,030*m/(0,200+m)`

Roll recibe la mitad. El máximo asintótico es 4,59 PWM equivalentes en pitch y 2,295 en roll; con velocidad limitada a 10, los máximos son ligeramente menores. En estos datos la reconstrucción da 4,39 y 2,22. La salida entera debe ser pequeña, del orden de 4–5 y 2 PWM respectivamente, si se cumplen los ajustes declarados.

No aumentar Fc sólo porque el número en el display parezca pequeño. Primero confirmar el comando real. La fórmula implementa fricción suavizada que se opone al movimiento; no mantiene fuerza en reposo ni reproduce adherencia estática.

## Calidad del registro y límites de la conclusión

- Hay 10.160 / 3.329 / 5.049 timestamps repetidos en centrado/damper/fricción. El CSV se escribe por muestra del simulador, no por paquete USB. No dividir por dt=0 para estimar velocidad.
- Los huecos máximos son 105,219 / 71,047 / 39,703 segundos. No se interpoló movimiento entre esos intervalos. Son compatibles con interrupciones de telemetría, incluida una pausa, pero los archivos no permiten atribuir su causa.
- Ninguna fila contiene paused=1. Esto no demuestra que no se haya pausado: el flujo SIM_FRAME puede dejar de producir filas, mientras el puente transmite la pausa por otra ruta.
- armed=1 sólo indica que el puente habilitó el envío. No confirma ganancias diferentes de cero, recepción en la Pico ni salida eléctrica.
- Las columnas Turbulencia/Buffet_* describen señales del puente anteriores a las ganancias de la Pico. Pueden estar activas aunque el usuario haya anulado el efecto en el menú. No son prueba de contaminación del ensayo.
- No se registran corriente, fuerza, PWM aplicado, versión de firmware ni ganancias efectivas. Estos archivos no permiten calibrar N o N·m ni demostrar fidelidad al esfuerzo de un R182 real.

## Siguiente ensayo útil

Antes de repetir vuelos, registrar el retorno de la Pico: tiempo, eje, posición, velocidad filtrada, comando y PWM aplicado, junto con todas las ganancias y versión. La placa ya emite una línea de diagnóstico de movimiento, pero el puente actual lee y descarta esos bytes durante el funcionamiento. Guardarlos permitiría comparar predicción y salida; para sincronización precisa conviene añadir número de secuencia/tiempo y registrar las ganancias.

Después, en el simulador y con efectos aislados:

1. Centrado: posiciones ±0,25 y ±0,50 sostenidas al menos un segundo en varios niveles de q. Incluir q/qRef >1 para probar la mejora SC6. Usar presión como referencia, respetando los límites del modelo de avión elegido.
2. Damper: el mismo recorrido lento, medio y rápido, cruzando el centro y terminando con una detención. Comparar salida frente a velocidad, no sólo frente a posición.
3. Fricción: repetir velocidades distintas y detenciones. Verificar aproximación a una meseta y comando cero en reposo.

## Reproducibilidad

`analizar.py` lee los tres CSV originales sin modificarlos y escribe `resultados.json`, con hashes de origen, rangos y reconstrucciones para cuatro fases iniciales de muestreo. La coincidencia de esos cuatro cálculos no prueba coincidencia con los paquetes reales. Los cálculos utilizan ganancia total 0,60 y los coeficientes declarados. No se modificaron firmware, puente ni ganancias durante este análisis.
