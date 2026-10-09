NOTA 30/09/2026: documento historico. La version activa usa solo SimConnect, sin FFB nativo ni amortiguacion local. Ver LEEME_SIMCONNECT.md.

# Etapa 3: mantener centrado de Prepar3D y agregar resistencia por velocidad

El usuario confirmo que CONSTANT ya centra el yugo en ambos sentidos.
Por eso no se agrega un resorte local por posicion ni se convierte FFB
en una posicion objetivo. El cambio es una amortiguacion adicional:

```text
ffb = salida original de Joystick.getForce() / 10000
d   = limitar(-Kd * velocidadFiltrada, -maximo, +maximo)
d   = d * gananciaTotalFFB * gananciaConstantFFB
u   = limitar(ffb + d, -1, +1)
```

La posicion medida se usa para calcular velocidad. CONSTANT conserva su
centrado y su variacion con el estado del avion. La amortiguacion no estima
velocidad del avion: depende de la velocidad REAL del mando. No se intenta
deducir rigidez a partir del valor instantaneo de CONSTANT.

Al alejarse del centro, la amortiguacion se suma a la resistencia al mando.
Al regresar al centro se opone al retorno, suavizandolo. Por eso no siempre
aumenta el valor absoluto de PWM: puede reducir la fuerza de centrado cuando
el yugo ya vuelve rapidamente. Quieto, la contribucion tiende a cero con
el filtro y queda la fuerza original de P3D.

## Activacion

En mis_defines.h:

```cpp
#define LOCAL_DAMPING_ENABLED 1
#define LOCAL_DAMPING_KD_ROLL    0.020f
#define LOCAL_DAMPING_KD_PITCH   0.010f
#define LOCAL_DAMPING_MAX_ROLL   0.080f
#define LOCAL_DAMPING_MAX_PITCH  0.040f
```

Son valores iniciales pequenos para ajustar en el montaje, no ganancias
validadas fisicamente. Pitch empieza menor por su mayor reduccion.
Para recuperar el comportamiento anterior, poner LOCAL_DAMPING_ENABLED=0.
Tambien se puede anular un eje poniendo su Kd en cero. PD_KP/KD antiguos
no se utilizan en esta etapa.

Kd convierte velocidad normalizada (recorridos centro-extremo/s) a comando
normalizado. Con Roll Kd=0.020, ganancias 90% y 100%, velocidad +1 produce
d=-0.018; velocidad +2 produce d=-0.036. La contribucion maxima es +/-0.072
con esas ganancias (0.080 * 0.90). MAX limita esta contribucion, NO es un
limite de corriente ni una proteccion termica.

La suma final se limita a +/-1. Si satura, se recorta la onda resultante,
incluido el seno. Mantener margen al ajustar ganancias.

## Relacion con los efectos USB

Joystick registra un bit por eje cuando evalua un CONSTANT activo dentro
de su ventana temporal que aporta fuerza distinta de cero sobre ese eje,
despues de la ganancia individual, la ganancia CONSTANT y la proyeccion.
La ganancia total se aplica despues: al ponerla en cero, d queda en cero.
Un CONSTANT de magnitud cero ya no habilita amortiguacion, aunque el host
lo mantenga en PLAYING. Esto tambien desactiva d al cruzar exactamente por
fuerza CONSTANT cero en el centro. Un SINE solo no la activa.

La mascara se reinicia en cada calculo: stop, pausa, reset o fin de ventana
del CONSTANT eliminan el aporte local. Disable Actuators tambien inhibe
la activacion local. No se agrega vigilancia de desconexion ni timeout de
comunicacion: se siguen los estados USB existentes. La ganancia individual
del efecto habilita/deshabilita el bit, pero no escala proporcionalmente d;
las ganancias global y CONSTANT del eje si escalan d.

La generacion y temporizacion de CONSTANT/SINE no cambian. No hace falta
recibir USB SPRING o USB DAMPER para que esta amortiguacion funcione.

## Archivos

- LocalDamping.h: ley -Kd*v y limite de contribucion.
- filters.h/.cpp: mantienen todo el filtrado de velocidad existente.
- Joystick.h/.cpp: reportan actividad de CONSTANT por eje, sin modificar
  sus magnitudes.
- MiYoke_PD_V1.7.ino: suma d a la referencia FFB antes del PWM.

LCD muestra en F el comando FINAL (FFB + amortiguacion).
Serial separa f (FFB calculado desde los efectos USB) y d (amortiguacion
local), ambos multiplicados por 10000. p muestra el PWM final con polaridad.
Por ejemplo, f=2500 y d=-720 corresponden a +0.25 y -0.072.
Se sustituyeron x/u por f/d para mantener cada fragmento dentro del buffer
USB de 64 bytes. a conserva el ADC, y c las cuentas en modo encoder.
v conserva la velocidad filtrada multiplicada por 1000. Por ejemplo,
v=1000 significa velocidad +1 recorrido centro-extremo por segundo.

Con ganancias 90%/100%, los limites locales son aproximadamente +/-18 PWM
en Roll y +/-9 en Pitch. Ver esos valores no demuestra un efecto SINE:
puede ser amortiguacion saturada por movimiento o por lecturas erroneas.
Comprobar a/c y v con el mando quieto si ocurre. La LCD no genera efectos.
Sin efectos activos, o con un unico CONSTANT de magnitud cero, deben salir
f=0, d=0 y p=0 incluso al mover el sensor. Con CONSTANT no nulo, el movimiento
debe seguir modificando d y, al detenerse, d debe tender a cero.

Se conserva el cambio del usuario `//Joystick.sendState();`: este sketch
no envia nuevas posiciones HID mientras esa linea siga comentada. Eso no
impide leer los potenciometros para amortiguacion local; la posicion del
simulador debe seguir llegando desde el dispositivo que ya este utilizando.
Tambien se conserva PITCH_POT_ADC_CENTER=511 y las opciones temporales FFB.

## Prueba incremental en el yugo

1. Verificar posicion y signo de v con la potencia desconectada. Motor
   logico positivo debe intentar aumentar la posicion medida. Signo
   incorrecto puede convertir la amortiguacion en asistencia al movimiento.
2. Conectar con ganancias moderadas, probar un eje por vez. Comparar
   LOCAL_DAMPING_ENABLED=0 y 1 con el mismo efecto y estado del avion.
3. Moverse desde el centro hacia el mismo lado, primero despacio y luego
   mas rapido. Debe aumentar la resistencia al movimiento hasta el limite.
4. Dejar de moverse: tras asentarse el filtro debe quedar solo el FFB original.
5. Volver al centro: la amortiguacion frena el retorno, no agrega fuerza
   hacia el centro. Verificar tambien el lado contrario.
6. Ajustar solo Kd de ese eje en pasos pequenos. Un Kd excesivo o ruido del
   sensor puede producir tirones. MAX se ajusta por separado si se alcanza
   demasiado pronto; no es necesario aumentarlo para comenzar.

No se cargaron firmware ni se hicieron pruebas de motores desde Codex.
No hay control de corriente/torque, stall, finales de carrera ni proteccion
termica agregados en esta etapa, segun el alcance acordado.

## Validacion de software anterior a la limpieza

Las cifras de memoria siguientes corresponden a la version anterior. Ver LEEME.md para el estado actual.

Las 11 comprobaciones en tests/local_damping_checks.cpp pasan con avr-g++:
reposo, ambos sentidos, aumento con velocidad, saturacion simetrica,
ganancias y desactivacion. Se evalua la funcion real con static_assert.
No sustituyen una prueba dinamica con motor, filtros y transmision.

Modo potenciometros: compila para arduino:avr:micro, AVR core 1.8.8.
27896/28672 bytes de programa; 1952/2560 globales y 608 bytes restantes
para pila/locales. Sigue la advertencia de poca memoria.

Tambien compila el modo encoder: 28666/28672 bytes de programa (solo 6
bytes libres), 1992 bytes globales y 568 bytes de RAM restantes. Antes
de ampliar funcionalidades con encoders sera necesario liberar programa.

Los archivos anteriores a esta etapa estan en .stage3-backup con sufijo .bak.
