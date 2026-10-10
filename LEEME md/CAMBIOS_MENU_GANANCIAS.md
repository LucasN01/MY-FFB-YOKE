# Menú de ganancias: roll, pulsación mantenida y límites

- Se agregó **Centrado roll**, después de Centrado pitch. Ajusta sólo el resorte de roll; pitch mantiene su propio valor. Por defecto roll=0,400 y pitch=0,800. Centro reposo sigue siendo un factor compartido de rigidez residual.
- Los ajustes guardados con el menú anterior se conservan mediante migración con CRC. Para conservar el comportamiento previo, el nuevo valor de roll se inicializa como la mitad del pitch guardado. Al guardar cualquier cambio se escribe el formato nuevo.
- En la pantalla de edición, + o − produce un paso al pulsar. Mantenerlo aproximadamente un segundo comienza la repetición, un paso cada 150 ms. Soltar detiene la repetición. No hay repetición en navegación, OK, Volver ni Modo; ambos signos presionados se cancelan. Mantener una tecla durante el encendido no dispara ajustes.
- Se conserva el máximo de **1,000** para cada ganancia y **100 %** para la ganancia total, con mínimo cero. El menú satura en los límites; la carga de memoria rechaza valores fuera de rango. La repetición no permite sobrepasarlos.
- La suma de efectos ya estaba limitada a **220/255 PWM**. Ahora el propio `MotorDriver::setPWM` también aplica un techo absoluto de ±220, incluso si otra ruta llama directamente al driver. `MOTOR_PWM_HARD_LIMIT` permite reducirlo en `mis_defines.h`; no puede aumentarse desde el menú. Esto conserva el techo existente, no establece un nuevo valor medido como seguro.

**Un límite de ganancia/PWM no garantiza que el motor no se dañe.** Sin lectura de corriente, temperatura y límites mecánicos no se detectan sobrecorriente o bloqueo. El proyecto continúa en la etapa declarada de cálculo y LCD. La protección física necesita implementar esos sensores y fijar límites a partir del motor y transmisión reales.

## Actualizar

Cargar `build-pico2/MiYoke_PD_V2.3.ino.uf2` en la Pico 2. No hace falta recompilar ni cambiar el puente: sigue siendo SC6. La compilación no carga automáticamente la placa. Después iniciar normalmente, esperar a que el puente esté conectado y pausar Prepar3D antes de entrar en calibración.

En el menú, seleccionar Centrado roll con +/−, pulsar OK y ajustar. OK confirma/guarda; Volver descarta la edición. Antes de cargar esta versión, las nuevas opciones todavía no existen en el firmware que está ejecutando la placa.

## Verificación

82.271 comprobaciones de escritorio pasaron, incluyendo independencia roll/pitch, migración de ajustes anteriores, rechazo de ganancias inválidas, límites de edición, antirrebote, retardo/repetición/liberación y desbordamiento de millis. Los archivos anteriores se conservaron en `tesis/auditoria_c182rg/antes_menu_roll`. No se activaron motores ni se cargó la placa.

Orden del menú: Ganancia total, Centrado pitch, Centrado roll, Centro reposo, Turbul. pitch, Turbul. roll, Vib. suelo, Buffet perdida, Buffet tren, Buffet spoiler, Damper, Friccion. Los nombres se abrevian para el LCD de 16 columnas. El orden visual no cambia los identificadores guardados ni las ganancias.

## Activar y desactivar efectos sin cambiar ganancias

Con el simulador pausado y el puente conectado, o en modo sin enlace:

1. **Cal** desde la pantalla normal abre la lista de ganancias, en el orden indicado arriba.
2. **Cal** nuevamente cambia a la lista **EFECTOS**. Otro Cal vuelve a ganancias.
3. **+ / −** recorre los efectos. Están en el mismo orden, excluyendo Ganancia total, que no es un efecto.
4. **OK** muestra el nombre en la primera línea y **Activado** o **Desactivado** en la segunda.
5. **+** selecciona Activado; **−** selecciona Desactivado. No se aplica hasta confirmar.
6. **OK** guarda el estado en memoria y vuelve a la lista. El valor numérico de la ganancia se conserva.
7. **Atrás**, durante la edición, descarta el cambio y vuelve a la lista de efectos. Desde la lista sale a la pantalla normal, igual que en ganancias.

Cal durante una edición cambia a la otra lista y descarta la edición sin confirmar. Si se reanuda el simulador, el menú se cierra y se cancela lo no guardado. Si falla el guardado, se conserva el estado anterior y se puede reintentar con OK.

Los estados se conservan al reiniciar. Al cargar este firmware se aceptan los dos formatos anteriores de ganancias (con y sin Centrado roll), preservando sus valores y dejando inicialmente todos los efectos activados. Una ganancia guardada en cero sigue produciendo cero aunque el efecto esté activado: para recuperarlo también debe tener una ganancia mayor que cero.

Centrado pitch/roll y turbulencia pitch/roll se pueden desactivar por separado. Centro reposo desactivado equivale a utilizar cero como coeficiente de reposo en la fórmula de rigidez, conservando el valor guardado. Damper y fricción actúan sobre ambos ejes. Las opciones de compilación siguen siendo el límite superior de disponibilidad de cada efecto.

La repetición por pulsación mantenida se aplica sólo a la edición numérica de ganancias, no a Activado/Desactivado. Pasaron **83.793 comprobaciones** incluyendo navegación, cancelación, guardado fallido/exitoso, recuperación de estados, migración de memoria y anulación efectiva de las salidas sin modificar sus ganancias. No se cargó la placa automáticamente.

## Calibración con el simulador cerrado

Se permite abrir y guardar en ambos menús cuando el puente comunica pausa, cuando no hay enlace USB abierto, cuando todavía no se recibió telemetría válida o cuando transcurrieron 300 ms sin un paquete válido nuevo. Este último umbral es el mismo del watchdog que anula las fuerzas. El cierre normal del simulador hace que el puente termine y libere el enlace.

Con paquetes recientes del simulador sin pausa, la edición se bloquea incluso si el puente está en modo monitor o la salida está desarmada. Al reconectar en ejecución, el menú se cierra y descarta cambios no confirmados. Mientras el menú está abierto hay además una anulación explícita de ambos comandos de motor antes de enviarlos al driver. Antes de grabar flash también se detienen ambas salidas.

La placa no puede observar el proceso Prepar3D de Windows: **sin enlace también se permite calibrar si el simulador sigue abierto pero el puente fue desconectado**. En ese caso no hay fuerzas activas. El bloqueo durante un vuelo se restablece al recibir comunicación válida. No se presenta ausencia de telemetría como confirmación de que el proceso esté cerrado.

Esta revisión pasó **83.804 comprobaciones**, incluyendo arranque sin puente, pausa, timeout exacto de 300 ms, reconexión durante una edición pendiente y estado reciente sin pausa/desarmado. Requiere cargar el UF2 nuevo; el puente sigue usando SC6 sin cambios.
