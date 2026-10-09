NOTA 30/09/2026: documento historico. La version activa usa solo SimConnect, sin FFB nativo ni amortiguacion local. Ver LEEME_SIMCONNECT.md.

# Implementacion gradual: etapas 1 y 2

**Actualizacion:** ya se agrego la etapa 3 de amortiguacion local por
velocidad, documentada en [LEEME_AMORTIGUACION.md](LEEME_AMORTIGUACION.md).
Las instrucciones siguientes describen la base sin esa contribucion;
se puede volver a ella con LOCAL_DAMPING_ENABLED=0.

**Modo actual: potenciometros conectados directamente al Micro**
(`USE_POTENTIOMETERS=1`). Ver [LEEME_POTENCIOMETROS.md](LEEME_POTENCIOMETROS.md)
para cableado, calibracion, telemetria ADC y memoria de esta compilacion.
Las indicaciones de centro al encender, cuentas `c` y rango HID -350..350
de esta guia corresponden al modo encoder (`USE_POTENTIOMETERS=0`).

## Alcance actual

Se conserva FFB -> PWM, ahora con una referencia intermedia normalizada.
Se observa posicion y velocidad de ambos encoders sin agregar una fuerza
local dependiente de ellas. La libreria sigue recibiendo la posicion para
los efectos spring existentes, tal como antes.

No se incorporaron P/PD, lectura de corriente, PI de corriente,
sobrecorriente, stall, finales de carrera ni modelo termico.
PDController.h y TorqueMonitor.h se conservan como archivos anteriores,
pero no se incluyen ni se instancian en el sketch. Sus parametros no
intervienen en estas etapas.

## Flujo y archivos

1. `Joystick.getForce()` entrega Roll/Pitch en -10000..10000.
2. `normalizeFfb()` divide por 10000 y satura en -1..1.
3. `motorCommandRoll/Pitch` son, por ahora, exactamente el FFB normalizado.
4. `MotorDriver.setCommand()` aplica el limite PWM_MAX y convierte a PWM.
5. `AxisMotion.h` calcula posicion y velocidad como datos de observacion.
   Delega el filtrado a `TimeConstantLowPassFilter`, declarado en `filters.h`
   e implementado en `filters.cpp`, con una instancia por eje. La clase
   `LowPassFilter` de los efectos FFB conserva su implementacion anterior.

La conversion usa redondeo simetrico: con PWM_MAX=255, +/-5000 producen
comandos +/-0.5 y PWM +/-128; cero produce exactamente cero y +/-10000
producen +/-255. Frente al map() anterior puede diferir una unidad PWM
por redondeo. Valores FFB fuera del rango se saturan antes de convertir.
Ganancias FFB y pines conservan su configuracion anterior. El LCD muestra
ahora el comando normalizado en lugar de la fuerza cruda.

## Centro, unidades y signos

El centro sigue siendo la posicion al encender o reiniciar. Colocar el
yugo centrado antes de encender: no hay homing ni centro persistente.

En `mis_defines.h`:

- `ROLL_POS_MIN/MAX` y `PITCH_POS_MIN/MAX`: cuentas observadas desde el
  centro hasta cada extremo, con minimo negativo y maximo positivo.
  Los +/-1000 actuales siguen siendo valores de ejemplo a verificar.
- `ROLL_ENCODER_DIRECTION` / `PITCH_ENCODER_DIRECTION`: +1 o -1.
- `ROLL_MOTOR_DIRECTION` / `PITCH_MOTOR_DIRECTION`: +1 o -1, independientes
  del encoder. Todos empiezan en +1 para conservar los signos anteriores.
- Convencion propuesta: roll derecha positivo, pitch tirar positivo.
- Los encoders estan despues de la reduccion: no multiplicar sus cuentas
  por 3.2 ni por 19.2. `*_COUNTS_PER_REV` no interviene en este calculo.

La posicion normalizada es cero en el centro y +/-1 en los extremos
configurados. Si los recorridos son asimetricos se usa la escala de cada
lado. Se conserva internamente sin recortar para seguir midiendo velocidad
fuera del rango; `boundedPosition()` ofrece una copia acotada para mostrar.
Esto NO es un final de carrera ni detiene el motor al superar +/-1.

La velocidad es el cambio de posicion normalizada por segundo, usando el
tiempo real de `micros()`. No es grados/s. Con limites asimetricos, la misma
velocidad fisica puede dar valores diferentes en cada mitad del recorrido;
habra que considerar esto al elegir las unidades del futuro amortiguador.

El filtro usa tau=18 ms (`VELOCITY_FILTER_TAU_S`), equivalente a alpha=0.90
con muestras cada 2 ms. Se adapta al dt real. Tau=0 elimina el filtrado.
La primera muestra inicializa la posicion con velocidad cero. La resta
unsigned permite cruzar el desbordamiento normal de micros().

El rango HID enviado a Windows sigue en -350..350 cuentas, como antes;
es independiente de los limites de normalizacion. No se hizo calibracion
HID en esta etapa. Se podra ajustar cuando se mida el recorrido real.

## Primera prueba: encoders y telemetria

1. Mantener desconectada la alimentacion de potencia de motores para
   comprobar encoders; Arduino y encoders deben estar alimentados.
2. Centrar el yugo, encender/reiniciar Arduino y abrir el monitor Serial
   a 115200. No hace falta Prepar3D para verificar el movimiento.
3. Mover un eje por vez y observar su linea. Se emite una linea cada
   100 ms alternando R/P: cada eje aparece aproximadamente cada 200 ms.
4. Verificar que `c` aumenta en el sentido positivo elegido. Si hace
   falta, cambiar solo el signo de ese encoder, recompilar y reiniciar.
5. Anotar `c` en ambos extremos y configurar POS_MIN/MAX. No usar la
   posicion acotada `x` para medir los extremos: puede saturarse antes.
6. Al mover hacia positivo, `v` debe tender a positivo; al invertir el
   movimiento, debe cambiar de signo con el retardo del filtro. Al parar,
   debe tender a cero. Encoder quieto implica `c` estable.

Ejemplo de formato (valores ilustrativos):

```text
R c=250 x=250 v=120 u=500 p=128
P c=-100 x=-100 v=0 u=0 p=0
```

| Campo | Significado |
| --- | --- |
| R / P | Roll / Pitch |
| c | Cuentas relativas al centro, con signo configurado |
| x | Posicion acotada por 1000: 250 significa +0.250 |
| v | Velocidad filtrada por 1000: 120 significa +0.120 por segundo |
| u | Comando normalizado por 1000: 500 significa +0.500 |
| p | PWM firmado realmente enviado despues de aplicar el signo del motor |

El sketch no espera que se abra Serial. Si no hay espacio de transmision,
omite la muestra. `DEBUG_MOTION_TELEMETRY=0` permite quitar la telemetria.
El LCD muestra ahora comando normalizado y PWM de los dos ejes, por
ejemplo `RF+.7500 PW+191`. La letra F identifica el comando normalizado,
redondeado a cuatro decimales; no es la fuerza FFB cruda ni torque medido.

## Segunda prueba: conservar FFB -> PWM

Despues de comprobar encoders, probar con la potencia conectada y una
ganancia FFB reducida respecto de la actual (FFB_TOTAL_GAIN sigue en 90).
Verificar primero efectos pequenos conocidos, de un eje por vez.

- Cero comando debe dar PWM cero. Esto no significa motor libre:
  con BTS7960 habilitado, ambos PWM en cero pueden producir frenado.
- El signo de `u` debe corresponder al movimiento positivo/negativo elegido.
  `p` incluye MOTOR_DIRECTION: puede ser negativo con `u` positivo si ese
  eje necesita inversion de cableado logica.
- Para corregir solo el accionamiento, ajustar MOTOR_DIRECTION, no cambiar
  a la vez motor y encoder. El criterio es que un comando logico positivo
  intente aumentar las cuentas del encoder.
- Posicion y velocidad nuevas no deben agregar centrado ni amortiguacion.
- Comparar efecto constante y seno con el funcionamiento anterior.

La verificacion en placa y del sentido mecanico sigue pendiente: compilar
no permite verificar cableado ni ganancias. No se carga firmware automaticamente.

## Proximas etapas, despues de verificar estas dos

3. Agregar solo P pequeno y luego D con velocidad filtrada, con activacion
   explicita y unidades/ganancias nuevas. No reutilizar directamente las
   ganancias del antiguo PDController: fueron planteadas para otras unidades.
4. Medir corriente y observarla sin regularla.
5. Agregar PI de corriente. Supervisores y protecciones quedan para despues.

## Verificacion de compilacion

Compila para `arduino:avr:micro` con Arduino AVR core 1.8.8 y telemetria
habilitada y LCD con comando normalizado, en modo encoder. Resultado: 28080/28672
bytes de programa y 1982/2560 bytes de variables globales; quedan 578
bytes de RAM para pila/variables locales.
El compilador avisa de poca memoria. Antes de agregar la siguiente etapa
hay que revisar este margen; esta compilacion no demuestra estabilidad
en placa ni una frecuencia real sostenida de 500 Hz.

La consulta de conexion Serial usa `dtr()`: se comprobo que el operador
booleano de Serial del core instalado introduce 10 ms de espera y se
evito usarlo en el lazo.

## Volver a la version anterior

Los cinco archivos editados que ya existian se guardaron antes del cambio
en `.stage12-backup/`, con sufijo `.bak`. Restaurarlos quitando ese sufijo
recupera el sketch y la configuracion anterior. AxisMotion.h y esta guia
son nuevos y no son usados por el sketch anterior.
