# LCD I2C 16x2 para Pro Micro

Abrir MiYoke_PD_V1.7.ino desde esta carpeta completa. No reemplazar solo el
.ino: tambien cambiaron mis_defines.h y se agrego LcdTelemetry.h.

## Conexiones necesarias

El encoder de pitch usaba D3 y D2, que son los pines I2C del ATmega32U4.
Mover los cables antes de usar esta version:

| Conexion | Antes | Ahora |
|---|---|---|
| Encoder pitch, canal A | D3 | D7 |
| Encoder pitch, canal B | D2 | D4 |
| Habilitacion del driver roll | D7 | A2 |
| LCD SDA | - | D2 |
| LCD SCL | - | D3 |
| LCD GND | - | GND comun |

Alimentar VCC de la LCD segun el modulo y la tension de la placa. Si la Pro
Micro es de 3,3 V y el adaptador LCD tiene pull-ups a 5 V, usar adaptacion
de niveles I2C; no conectar esas lineas directamente.

Los demas pines conservan la asignacion original. El canal A de pitch sigue
en un pin de interrupcion. La libreria Encoder admite el canal B en D4;
verificar el conteo a la velocidad real del encoder tras recablear.

## Pantalla y puerto serie

Ejemplo exacto, con datos variables:

```text
RF+.7500 PW+191
PF-.5000 PW-128
```

R = roll; P = pitch; F = comando normalizado -1..1 antes del signo del
motor; PW = PWM firmado enviado al driver. No es una medicion de torque,
voltaje ni corriente. El cambio de pantalla no modifica el control.

El formato habitual omite el cero antes del punto:

```text
RF+.0000 PW+  0
PF-.9000 PW-230
```

El comando se redondea a cuatro decimales y el PWM ocupa 3 digitos,
alineados a la derecha con espacios. Se usan enteros en unidades de 0.0001 para
formatear sin habilitar el soporte de printf para flotantes.
PW comienza en la columna 10 en el formato habitual. Si el comando alguna
vez redondea a +/-1, se muestra `RF+1.0000 PW+255` para no confundirlo con
cero. Se omite el espacio entre el eje y F y se completa la fila hasta
16 caracteres con espacios finales.
Los valores cortos se completan con espacios para borrar digitos anteriores.
El cero se muestra con signo +. Las dos filas usan una misma captura de datos.

DEBUG_MOTION_TELEMETRY esta en 1: monitor serie a 115200 baudios para
observar posicion, velocidad, comando y PWM (ver LEEME_ETAPAS.md).
Se puede poner en 0 para usar solo la LCD. No se espera la apertura del
monitor y se omiten muestras si no hay espacio de envio.

## Adaptador LCD

Se admite el adaptador habitual PCF8574 para LCD HD44780: P0=RS, P1=RW,
P2=E, P3=backlight activo alto, P4..P7=D4..D7. Otros adaptadores pueden
necesitar cambiar el controlador. No hace falta instalar LiquidCrystal_I2C:
se utiliza Wire, incluida con Arduino, y el archivo LcdTelemetry.h adjunto.

LCD_I2C_ADDRESS en mis_defines.h vale 0x27; si no responde, prueba 0x3F.
Si el modulo tiene otra direccion, editar esa constante. Ajustar el potenciometro
de contraste del adaptador si se enciende la luz pero no se ven caracteres.

Se captura una lectura cada 100 ms y se envia un comando o caracter por ciclo,
a 100 kHz. No se borra toda la pantalla en cada actualizacion. I2C es sincrono:
agrega tiempo al ciclo, aunque reparte la escritura para evitar una pausa de
una pantalla completa. En el core AVR usado se configura timeout de 1500 us.
Si falta la LCD o falla una transmision, se reintenta automaticamente cada 500 ms.
Para las direcciones habituales se alterna entre 0x27 y 0x3F; una direccion
personalizada se reintenta sin sustituirla. No hace falta reiniciar el Arduino.

## Recuperacion despues de cortar la alimentacion

El inicio y la recuperacion se realizan por estados, sin delay(): se espera el
asentamiento de la alimentacion, se fuerzan E/RW/RS a cero y se envia la secuencia
de sincronizacion de 4 bits (3, 3, 3, 2), respetando las esperas del HD44780.
Se reconfiguran las dos filas, el display y el modo de entrada; Home restaura
el desplazamiento y luego se reescriben los 32 caracteres con datos actuales.
No se borra la pantalla ni se apaga la luz durante la resincronizacion normal.

Un ACK I2C solo confirma que responde el PCF8574, no que el HD44780 siga
inicializado. Por eso se resincroniza tambien cada 5 segundos al terminar una
escritura de pantalla. Esto permite recuperar resets de la LCD o cortes breves
que no llegan a producir un error I2C. Durante ese proceso se pausa brevemente
la actualizacion visual. Las esperas se reparten entre ciclos y se realiza
como maximo una transaccion I2C por llamada; I2C sigue siendo sincrono y conserva
el timeout, por lo que no se promete tiempo de ejecucion nulo.

Parametros en mis_defines.h: LCD_POWER_ON_WAIT_MS=100, LCD_RETRY_MS=500 y
LCD_RESYNC_MS=5000. Tras restablecer alimentacion estable, la recuperacion por
falta de ACK deberia tomar del orden de 1--2 segundos; si el adaptador nunca
dejo de responder, esperar aproximadamente hasta 6 segundos. Son tiempos
previstos con el lazo actual, pendientes de medicion en hardware.

La recuperacion de software no corrige una alimentacion insuficiente, un bus
mantenido fisicamente en nivel bajo ni alimentacion parasita a traves de SDA/SCL.
Con fuente independiente, conservar GND comun y verificar tensiones/pull-ups.
Si el problema persiste, medir VCC del LCD durante el encendido y los cortes.

### Prueba en el montaje

1. Encender primero Arduino y despues LCD, y repetir en el orden inverso.
2. Reiniciar/desconectar y reconectar Arduino con LCD encendido.
3. Cortar la fuente del LCD unos segundos, restaurarla y comprobar ambas filas.
4. Repetir con un corte breve y esperar hasta 6 segundos al recuperar alimentacion.
5. Verificar que Serial siga mostrando movimiento y que el LCD recupere valores
   actuales. Observar tambien si hay artefactos durante la resincronizacion.
6. Repetir varios ciclos; registrar cualquier bloqueo persistente para revisar
   alimentacion, cableado y recuperacion del bus en el hardware real.

## Verificacion

Compilado con Arduino AVR 1.8.8, perfil arduino:avr:micro (ATmega32U4, 16 MHz).
Tras esta correccion, modo potenciometros: 23318 bytes de programa y 1397 bytes
de RAM global. La recuperacion debe validarse con los ciclos de alimentacion
anteriores: la compilacion no reproduce cortes electricos.
No se verifico el consumo maximo de pila ni se probo en hardware.

Para cargarlo, elegir el perfil y la tension/frecuencia de la Pro Micro real;
esta compilacion no valida la variante de 8 MHz. Se mantiene la dependencia
Encoder de Paul Stoffregen que ya usaba el proyecto.

Referencias: https://docs.arduino.cc/hardware/micro y
https://www.nxp.com/docs/en/data-sheet/PCF8574_PCF8574A.pdf
