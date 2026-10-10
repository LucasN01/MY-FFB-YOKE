# Mejoras implementadas y procedimiento para continuar

Esta versión conserva todos los efectos. Implementa una primera mejora del cálculo de centrado, amortiguación y fricción. No equivale todavía a una calibración de fuerza del R182. Los comandos siguen siendo adimensionales y el PWM no representa newtons.

## 1. Cambios implementados

### Centrado: conservar la variación con presión

Antes: `p=min(q/qRef,1)`. A partir de 120 kt de velocidad equivalente, con el ajuste por defecto, el centrado ya no aumentaba.

Ahora: `p=min(q/qRef,4)`, `k=rest+(1-rest)*p`, `u=-G*K*k*x`.

Se conserva el filtrado de 100 ms, las ganancias actuales y la suma final limitada a ±220/255. El factor 4 es un límite de representación del protocolo, no una propiedad del avión. Con referencia 120 kt cubre hasta 240 kt EAS, por encima del rango operativo de la referencia R182. Cambiar la referencia cambia ese rango. Puede alcanzarse antes el límite de salida por posición, ganancias u otros efectos: eso es saturación del dispositivo, no saturación aerodinámica.

Ejemplo estacionario, pitch al 20 % de recorrido, ganancias por defecto y sin otros efectos:

| EAS | PWM equivalente anterior | PWM equivalente nuevo |
|---:|---:|---:|
| 120 kt | 36,72 | 36,72 |
| 160 kt | 36,72 | 61,00 |
| 180 kt | 36,72 | 75,74 |

Se utiliza DYNAMIC PRESSURE de Prepar3D, no IAS al cuadrado. La EAS de la tabla sólo permite expresar la presión como una velocidad equivalente. Fundamento de q=ρV²/2: [NASA Glenn](https://www1.grc.nasa.gov/beginners-guide-to-aeronautics/dynamic-pressure/). El resorte proporcional a presión/posición sigue el enfoque de TelemFFB; las ganancias y el 15 % residual siguen siendo ajustes sin identificación R182.

El protocolo SC6 mantiene el tamaño de trama, amplía el campo de presión a 0..40000 y valida su rango. El firmware sigue leyendo SC1..SC5 con sus rangos originales. El puente nuevo exige identificación SC6 para impedir usar este campo ampliado con firmware viejo. Las ráfagas conservan su presión limitada a uno: no se amplificaron indirectamente al ampliar el centrado.

### Amortiguación y fricción: fuerzas independientes

Antes se escalaban por la fuerza base y se impedía invertir su signo. Por eso no existían en el centro aunque el mando estuviera moviéndose.

Ahora se suman términos independientes:

```
v = velocidad normalizada limitada a ±10
m = max(abs(v) - 0.02, 0)
u_d = -sign(v) * B * m
u_f = -sign(v) * Fc * m/(0.2 + m)
u_mov = limitar(u_d + u_f, -0.4, 0.4) * ganancia_del_eje
u_total = limitar(u_centrado + u_rafagas + u_buffets + u_mov + u_rodaje)
```

B=0.12 y Fc=0.03 son los defaults existentes; el menú/flash puede contener otros valores. La regularización racional de fricción ya existía en el código. Ahora actúa de forma independiente y continua, incluyendo el centro. No representa adherencia estática exacta. La zona muerta limita respuesta al ruido; fuera de ella el amortiguador aproxima resistencia viscosa. La potencia normalizada de estos términos cumple `u_mov*v <= 0`: no empujan en el sentido del movimiento.

[OpenFFBoard, EffectsCalculator.cpp](https://github.com/Ultrawipf/OpenFFBoard/blob/master/Firmware/FFBoard/Src/EffectsCalculator.cpp) fundamenta la separación entre spring, damper y friction. No se copiaron sus coeficientes ni se atribuye esta regularización concreta al Cessna. Se añadieron máscaras independientes Damper y Friction para poder probarlos por separado. Los límites de seguridad de comunicación y salida siguen aplicándose a todos los efectos.

## 2. Cómo probar esta versión

1. El firmware nuevo se genera en `build-pico2/MiYoke_PD_V2.3.ino.uf2`. Cargarlo en la Pico 2 mediante el procedimiento BOOTSEL habitual. Esta tarea no lo cargó automáticamente ni activó motores. La primera comprobación corresponde a la etapa de demostración LCD declarada por el usuario.
2. Después de cargarlo, usar **PROBAR REALISMO.cmd**. Inicia la misma secuencia MobiFlight → Prepar3D → avión en tierra → puente, pero selecciona `SimConnectBridge/bin/Win32/RealismoSC6/YokeSimConnectBridge.exe` en COM13.
3. Tanto `INICIAR SIMULADOR.cmd` (incluido su acceso directo) como `PROBAR REALISMO.cmd` seleccionan ahora SC6 por defecto, coincidiendo con la placa comprobada. El ejecutable SC5 anterior se conserva. Para volver a él, cargar el UF2 guardado en `tesis/auditoria_c182rg/antes_mejoras/firmware_anterior.uf2` y ejecutar `Iniciar-Simulador.ps1 -Realismo:$false` desde PowerShell. Las fuentes anteriores modificadas también se guardaron allí como `.txt`.
4. Para ensayar centrado, poner temporalmente en cero desde el menú las ganancias de ráfagas, buffets, rodaje, amortiguación y fricción. Anotar los valores antes de cambiarlos y restaurarlos después. Comparar igual posición a distintas presiones. No esperar que el PWM siga creciendo cuando alcance el límite total.
5. Para amortiguación/fricción, poner centrado y perturbaciones en cero; activar primero sólo amortiguación y después sólo fricción. Mover el eje de MobiFlight en ambos sentidos y atravesar el centro. Debe cambiar el signo para oponerse al movimiento, incluso cerca del centro; al detenerse debe desaparecer después de actualizarse el estimador de velocidad.
6. Restaurar todas las ganancias anotadas. El LCD instantáneo no permite validar frecuencias de 8–15 Hz porque actualiza a 10 Hz. Usar las pruebas de escritorio para esos casos; queda pendiente añadir RMS/pico al display.

## 3. Procedimiento concreto para los efectos restantes

### Si el menú pide pausa aunque Prepar3D esté pausado

La calibración necesita que el puente esté conectado y transmita la pausa, no sólo que el simulador esté pausado. Se comprobó por USB que la placa responde `YOKE17SC6`; el último registro del puente anterior mostraba un rechazo por exigir SC5. Se corrigió el lanzador predeterminado para elegir SC6. No requiere volver a cargar firmware.

Al iniciar, dejar que el avión esté en tierra sin pausa hasta que el lanzador indique que inició el puente. Luego pausar Prepar3D, esperar aproximadamente un segundo y pulsar calibración. Si se pausa antes de que arranque el puente, el lanzador continúa esperando un vuelo listo y la placa todavía no recibe el estado. Mantener abierta la ventana del puente. La conexión USB SC6 fue verificada; la comprobación de pausa en vivo quedó pendiente porque Prepar3D no estaba abierto durante la corrección.

No hace falta que el usuario deduzca nuevas ecuaciones. El siguiente trabajo consiste en implementar y contrastar estas etapas, conservando el estado anterior para comparación:

| Etapa | Cambio de modelo | Datos/ensayo necesarios | Fuente y criterio |
|---|---|---|---|
| Trim y potencia | Añadir presión efectiva en cola y posición de equilibrio al resorte: `u=-K(qt)*(x-x0)+u_mecanica` | Leer trim, elevador, empuje/RPM; igual IAS y posición, variar trim y potencia. Comprobar cómo MobiFlight y P3D aplican trim para no duplicarlo. | TelemFFB; modelo de momento de bisagra NASA. Coeficientes y posición de equilibrio deben identificarse. |
| Pérdida | Ajustar inicio de envolvente por configuración y reemplazar tono puro por señal irregular de banda limitada | Desaceleraciones en el simulador, mismo peso/CG, flap 0/10/20/30. Registrar AoA, IAS, q y aviso de pérdida. | TelemFFB para arquitectura; manual para contrastar velocidades. La alarma acústica no demuestra el comienzo del buffet táctil. |
| Rodaje | Excitación basada en aceleración/contactos; si no hay telemetría útil, rugosidad espacial explícita con `f=V/longitud` | Comprobar qué SimVars soporta P3D v4; comparar reposo y varias velocidades sobre distintas superficies. | TelemFFB utiliza contactos; no asumir que sus variables de MSFS existen en P3D. La longitud y transmisión al mando serán parámetros empíricos. |
| Tren | Envolvente ligada a presión/extensión y diferenciación entre buffet y mecanismo | Ensayo a misma velocidad con tren retraído/extendido; no usar VLE como umbral de aparición del buffet | TelemFFB separa esos fenómenos. La intensidad requiere contraste; no hay aquí medición del R182. |
| Ráfagas | Verificar signo por eje y distinguir cambios atmosféricos de rotaciones del avión | Viento constante con giros, luego escalones de viento controlados | TelemFFB + unidades/ejes SDK. Mantener la señal actual hasta comprobar transformaciones. |
| Spoilers | Conservar módulo, salida cero en perfil sin spoilers | Entrada sintética separada para demostrar el algoritmo | Configuración del addon; no simular una superficie inexistente. |

La referencia provisional es el R182 de 1978. Su POH y el addon no coinciden en potencia/flaps; para decidir entre reproducir el addon o corregir su modelo de vuelo hace falta elegir explícitamente el avión objetivo. No se modificó aircraft.cfg.

Fuentes para continuar: [TelemFFB](https://github.com/walmis/VPforce-TelemFFB), [NASA/TP-2000-209908 §5.3.4](https://ntrs.nasa.gov/api/citations/20000025766/downloads/20000025766.pdf), [SDK Prepar3D v4](https://www.prepar3d.com/SDKv4/sdk/references/variables/simulation_variables.html), [POH R182 1978](https://www.northcoastair.com/documents/Cessna_182RG_C182RG_1978_POH_scanned.pdf).

## 4. Validación realizada

81.753 comprobaciones de escritorio pasaron: protocolo antiguo y SC6, presión ampliada y rechazo de valores inválidos, incremento del centrado entre qRef y 2.25*qRef, resistencia al movimiento en el centro, simetría, pasividad de las cargas de velocidad, watchdog y límite PWM. Es validación del cálculo y la comunicación, no medición de fuerza ni prueba de vuelo.

Compilaron correctamente el puente Win32 y el firmware Pico 2 (88.828 bytes de programa, 17.668 bytes de variables globales). El lanzador experimental pasó `-Realismo -Comprobar`. No se realizó una prueba en vivo con Prepar3D/placa ni se cargó el firmware. El compilador del puente mostró advertencias de conversión signed/unsigned en el encabezado del SDK SimConnect.

El informe `tesis/auditoria_c182rg/ANALISIS_Y_MODELO_PROPUESTO.md` describe la versión anterior a estos cambios. Sus registros conservan la evidencia histórica. No volver a generar esa evidencia para atribuirla retroactivamente a SC6.
