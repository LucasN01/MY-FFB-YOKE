# Encoders magneticos AS5600: fuente de posicion 3

Dos AS5600 absolutos de 12 bits miden roll y pitch despues de las cajas reductoras. La velocidad se obtiene con el estimador local comun a las fuentes fisicas. No se envian ejes al simulador. El menu de calibracion de potenciometros sigue requiriendo modo 2; los AS5600 se configuran en `mis_defines.h`. Las fallas del sensor se informan por serie, no en el LCD.

## Cableado

Ambos sensores tienen direccion fija 0x36; por eso usan buses distintos.

| Senal | Roll | Pitch |
|---|---|---|
| Bus | I2C1 (`Wire1`), propio | I2C0 (`Wire`), compartido con LCD 0x27/0x3F |
| SDA | GP14, pin fisico 19 | GP4, pin fisico 6 |
| SCL | GP15, pin fisico 20 | GP5, pin fisico 7 |
| Reloj | `YOKE_I2C1_HZ`: 400 kHz | `YOKE_I2C0_HZ`: 100 kHz |
| Alimentacion | 3V3 de la Pico y GND comun | 3V3 de la Pico y GND comun |

DIR a GND; OUT y PGO/GPO sin conectar. Usar cables cortos y trenzados, lejos de los cables del motor. GP14/15 quedan reservados para roll en modo 3.

El bus I2C0 debe estar a **3,3 V**. Si el LCD/PCF8574 se alimenta a 5 V con pull-ups a 5 V, conectar el AS5600 de pitch del lado de 3,3 V del adaptador de nivel, junto a la Pico (ver [LEEME_PICO2.md](LEEME_PICO2.md)). No subir I2C0 a 400 kHz: el PCF8574 admite 100 kHz.

## Montaje del iman

Usar iman diametral (polos N y S a los lados, no en las caras), tipicamente neodimio de 6 mm por 2,5-3 mm, pegado coaxial en la punta del eje de salida de la reductora. Entrehierro iman-chip de 0,5 a 3 mm, tipico 1-2 mm; ajustar hasta obtener `g=` cerca de 64 y `s=OK` en todo el recorrido. `DEB` indica iman muy lejos; `FUE`, muy cerca.

Alinear el centro del iman con el del chip, con caras paralelas y desalineacion lateral de unas decimas de milimetro como maximo. Mantener la distancia al girar, sin juego axial ni excentricidad. Usar soporte y eje de aluminio, plastico o bronce cerca del iman; evitar acero a pocos milimetros. Alejar el sensor del motor 775 y de sus cables de potencia.

Idealmente el sensor gira menos de media vuelta por lado: permite centro absoluto. Si gira mas (pitch estimado: +/-229 grados), usar centro al encender.

## Configuracion y compilacion

Seleccionar `#define YOKE_POSITION_SOURCE 3`. Los valores 0, 1 y 2 conservan su significado: encoders AB, SimConnect y potenciometros. La bandera anterior produce un error de renombre.

- `ROLL_AS5600_CENTER_RAW` y `PITCH_AS5600_CENTER_RAW`: -1 toma la primera lectura como centro. Encender con el yugo centrado. Acumula cruces de 0 para recorridos mayores de media vuelta.
- Centro absoluto: guardar la lectura `a=` del centro (0..4095). No hace falta centrar al encender; cada extremo debe estar a 1800 cuentas o menos del centro.
- `*_AS5600_COUNTS_NEG/POS`: medir desde el centro hasta cada tope con `a=`. Cada cuenta equivale a 360/4096 grados. En centro absoluto usar la diferencia circular mas corta; en centro al encender sumar los incrementos entre lecturas consecutivas, corrigiendo cruces de 0 (+/-4096), para contar recorridos mayores de media vuelta. Cargar las magnitudes positivas de ambos extremos, despues de aplicar `*_AS5600_DIRECTION` (+1 o -1). Valores iniciales: roll 1024 por lado, pitch 2608 por lado.
- Histeresis movil: `YOKE_AS5600_HYSTERESIS_COUNTS=1`; normalizacion limitada a [-1,1].
- `YOKE_AS5600_CONF=0x0500`: SF=8x (1,1 ms), FTH=6 LSB, WD=0, modo nominal. Se escribe y verifica al iniciar y se comprueba cada segundo alternando ejes; si se perdio, se reescribe. Es volatil: nunca se usa BURN.

Compilar y verificar con `powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1 -Todo -As5600`. `-Todo` incluye AS5600. La variante usa `Build-Pico2.ps1 -As5600`, fuerza modo 3 y deja el UF2 en `build-pico2-as5600/`; es excluyente con encoders, potenciometros y prueba de falla. No se carga automaticamente.

## Telemetria y fallas

La linea de movimiento alterna Pitch/Roll cada 100 ms: `a` es RAW ANGLE (0..4095), `s` el estado y `g` AGC (0 si falla su lectura o el eje no esta sano). A 3,3 V, AGC va de 0 a 128; buscar cerca de 64. `x`, `v`, `f` y `p` son posicion, velocidad, comando y PWM.

Estados en orden de prioridad: `BLQ` bloqueo hasta reiniciar; `I2C` transaccion fallida; `SIN` sin iman detectado; `DEB` campo debil; `FUE` campo fuerte; `OK` lectura correcta. Por defecto se exige MD (iman detectado); `YOKE_AS5600_REQUIRE_MD=0` permite modulos que no lo informan.

Antes de la primera lectura valida, ante falla activa o bloqueo de cualquier eje, se anula la fuerza en ambos motores y se reinicia el estimador de velocidad para evitar picos al recuperar. Tres lecturas malas consecutivas activan la falla (6 ms nominales); hacen falta 50 buenas para recuperar (100 ms). La posicion se conserva durante lecturas malas.

Con centro al encender, un hueco mayor de 40 ms, incluso por detencion del lazo, bloquea hasta reiniciar: se pudo perder una vuelta. Tambien bloquea si la primera lectura tarda mas de 500 ms o si supera 1,25 veces las cuentas de un extremo. Solo los huecos conocidos por guardado de flash del menu se excusan. En centro absoluto no aplican esos bloqueos y la posicion se recalcula desde el angulo absoluto.

## Ensayo pendiente con sensores (usuario)

1. Comprobar `s=OK` y `g` cerca de 64 en ambos ejes durante todo el recorrido.
2. Verificar que `a=` varia suavemente y coincide en sentido y magnitud con el angulo girado. Esto verifica la lectura de tres bytes desde 0x0B; si no coincide, informar para evaluar STATUS y RAW ANGLE en transacciones separadas.
3. Desconectar SDA de un sensor: fuerza cero y `s=I2C`. Reconectar: recupera en unos 100 ms con centro absoluto; con centro al encender queda `BLQ` hasta reiniciar si el hueco supero 40 ms.
4. Comparar `loopWorstUs` y ciclos lentos del registro contra modo 2: no deben aumentar. Presupuesto estimado: pitch 0,6 ms + roll 0,15 ms + LCD 0,3-0,5 ms. Si no cumple, informar para evaluar `writeReadAsync` o servir LCD en otro ciclo. Mantener I2C0 a 100 kHz.

El firmware no fue validado con sensores ni motores en esta entrega; el ensayo lo hace el usuario cuando disponga de los sensores.
