# Menu de calibracion — Pico 2 (05/10/2026)

El firmware inicia en modo normal, con LED verde y telemetria de fuerzas/PWM.
Carga ganancias, efectos y calibracion de potenciometros guardados en flash.
Si un registro no es valido, utiliza los valores correspondientes de fabrica.

## Pulsadores y navegacion

Los pines son GPIO, no numeros fisicos. Pulsadores normalmente abiertos a GND,
con INPUT_PULLUP y antirrebote de 30 ms:

| Funcion | GPIO |
| --- | --- |
| CALIBRACION | GP20 |
| ATRAS | GP19 |
| OK | GP18 |
| - | GP17 |
| + | GP16 |
| LED verde normal | GP12 |
| LED amarillo calibracion | GP13 |

Cada LED lleva resistencia de 330 ohm en serie. El LCD usa GP4/GP5.

CALIBRACION recorre circularmente estas tres secciones:

1. **MODO CALIBRACION**: ganancias modificables. +/- recorre la lista, OK
   entra a editar, +/- cambia el valor, OK guarda y ATRAS descarta.
2. **EFECTOS**: +/- selecciona, OK entra, + activa y - desactiva, OK guarda.
   ATRAS descarta la edicion. Se conserva la ganancia de un efecto desactivado.
3. **CALIBRAR POTES**: OK inicia la secuencia descrita abajo.

Otra pulsacion de CALIBRACION vuelve a ganancias. Cambiar de seccion descarta
la edicion no confirmada. ATRAS desde cualquiera de las tres listas sale al
modo normal; desde el asistente de potes vuelve a CALIBRAR POTES.

Mantener +/- al editar ganancias repite luego de 1 segundo, cada 150 ms.
OK y CALIBRACION no repiten. ATRAS tiene prioridad sobre OK si coinciden.

## Calibracion guiada de potenciometros

Requiere `YOKE_USE_SIMULATION=2`. Los modos 0 y 1 muestran REQUIERE MODO 2
al intentar iniciar; no leen ADC ni permiten guardar una calibracion ficticia.
Roll usa ADC0/GP26 y Pitch ADC1/GP27.

Tras seleccionar CALIBRAR POTES y pulsar OK:

| Paso | Primera fila LCD | Accion y confirmacion |
| --- | --- | --- |
| 1 | CALIBRAR PITCH | Girar antihorario hasta el extremo y pulsar OK |
| 2 | CALIBRAR PITCH | Girar horario hasta el extremo y pulsar OK |
| 3 | CALIBRAR ROLL | Girar antihorario hasta el extremo y pulsar OK |
| 4 | CALIBRAR ROLL | Girar horario hasta el extremo y pulsar OK |

Para que las instrucciones quepan en 16x2, la segunda fila alterna cada
1,5 segundos GIRAR ANTIHOR. / GIRAR HORARIO, HASTA EL FINAL y PRESIONAR OK.
No es necesario esperar un mensaje determinado para confirmar.

Cada OK captura el ADC actual en memoria provisional. **Solo el ultimo OK
valido guarda ambos ejes juntos** y muestra POTES GUARDADOS. El centro de
cada eje es el promedio entero de sus dos extremos. No hay paso de centro.
Los nuevos valores se aplican y se recuperan al reiniciar, sin recompilar.

**ATRAS en cualquier paso descarta toda la secuencia**, incluso los extremos
ya confirmados, y vuelve a CALIBRAR POTES. CALIBRACION tambien descarta la
secuencia y vuelve a ganancias. No se modifica la calibracion anterior.

Se rechazan pares de extremos separados por menos de 64 cuentas ADC:
POCO RECORRIDO mantiene el paso actual para corregir el extremo y pulsar OK,
o cancelar con ATRAS y reiniciar. Se admiten ADC crecientes o decrecientes.
Un error de escritura muestra ERROR GUARDADO; OK reintenta las mismas
capturas, y ATRAS cancela conservando los valores activos anteriores.

## Sentido del giro y comprobacion

El primer extremo (antihorario) representa posicion -1; el segundo (horario),
+1. Esto no permite detectar automaticamente un giro equivocado: la relacion
entre sentido mecanico y aumento/disminucion del ADC depende del cableado.
Confirmar los dos extremos al reves invierte el eje y puede invertir el
centrado. La comprobacion de recorrido no detecta esa inversion.

Realizar la primera comprobacion con potencia de motores desconectada:
`x` debe ir aproximadamente de -10000 a +10000, y el centro debe dar 0.
Verificar que los sentidos fisicos de cada eje correspondan a la mecanica
prevista (roll derecha / pitch tirar positivos). En caso de error, repetir
la calibracion o corregir la correspondencia mecanica; no compensar a ciegas
cambiando el signo del motor. Ver [cableado ADC](LEEME_POTENCIOMETROS.md).

## Ganancias y efectos

Ganancias disponibles: total, centrado Pitch, centrado Roll, centro reposo,
turbulencia Pitch/Roll, vibracion de suelo, buffet de perdida/tren/spoiler,
damper y friccion. Los centrados de Pitch y Roll se ajustan independientemente.
La lista EFECTOS contiene las mismas entradas excepto la ganancia total.
Las banderas de compilacion de efectos siguen aplicando: el menu no puede
habilitar codigo que se haya excluido al compilar.

Los limites siguen siendo +/-220 PWM total, con aporte de rodaje limitado
a +/-40 Roll y +/-4 Pitch. Frecuencias, filtros y fuente no se editan aqui.

## Fuerzas, pausa y persistencia

Se permite entrar con simulador pausado o sin conexion/telemetria vigente.
Telemetria reciente de vuelo activo impide entrar: PAUSE SIMULADOR.
Si el vuelo se reanuda durante una edicion, se descarta lo pendiente y el
menu vuelve al modo normal. Se conserva el protocolo actual SC6 del puente.

Los motores quedan desactivados mientras el menu esta activo. Antes de
escribir flash, ambos PWM se ponen a cero; se reinician los estimadores de
movimiento para que cambiar la calibracion no produzca una velocidad ficticia.

Ganancias/efectos y potes tienen registros independientes con version y CRC.
Guardar potes conserva ganancias/efectos y viceversa. No se escribe flash por
navegar ni cancelar. Una escritura interrumpida por corte de alimentacion
puede causar que el proximo arranque use valores de fabrica.
`YOKE_SETTINGS_VERSION` reinicia ganancias/efectos, no la calibracion de potes.

## Compilacion

- `Build-Pico2.ps1`: usa la fuente de mis_defines.h (actualmente 1).
- `Build-Pico2.ps1 -Encoders`: fuerza modo 0.
- `Build-Pico2.ps1 -Potentiometers`: fuerza modo 2, genera el UF2 en
  `build-pico2-potentiometers`.

Son firmwares para **Pico 2**, no para Pico original. No se cargan automaticamente.
La verificacion fisica de LCD, pulsadores, sentido, ruido y persistencia al
apagar/encender queda pendiente de la prueba en banco.

## Validacion del asistente LCD (05/10/2026)

Compilacion Arduino-Pico 6.1.1, Pico 2 ARM con TinyUSB:

| Modo | Flash (bytes) | RAM global (bytes) |
| --- | ---: | ---: |
| 0, encoders | 91692 | 18096 |
| 1, SimConnect | 90700 | 17760 |
| 2, potenciometros | 91532 | 17860 |

Los tres firmwares compilaron. La suite C++ paso 84404 comprobaciones por
modo (0, 1 y 2), incluyendo las tres secciones, cancelacion en los cuatro
pasos, captura invertida de ADC, centro automatico, recorrido insuficiente,
fallo/reintento de guardado, conservacion de ganancias/efectos, recuperacion
por CRC, cambio de modo y reanudacion del simulador. Prueba fisica pendiente.
