# Diseno: limitador de fuerza cerca de los topes mecanicos

Rama `mejora/limite-topes` (desde `firmware/pico-sdk`). Clasificacion: **critica** (calculo de fuerzas y limite de seguridad): Claude disena e implementa; Codex revisa con lista de control.

## Objetivo

Con el yugo contra un tope, ningun efecto (vibracion, buffet, turbulencia, rodaje, friccion) debe empujar el motor hacia ese tope. La fuerza hacia el centro sigue pasando. Fuera de una zona configurable, todo funciona como hasta ahora. Decision del usuario (2026-11-05): se acepta que la vibracion cerca del tope se sienta mas debil y sesgada hacia el centro, a cambio de proteger los motores.

Fuente: TelemFFB no tiene un mecanismo equivalente (busqueda en `VPforce-TelemFFB-refactor`). Los valores de la zona son **hipotesis a validar con ensayo**.

## Decisiones

1. **Actua sobre la orden total del eje**, no por efecto: cubre todos los efectos actuales y futuros en un solo punto.
2. **Ubicacion:** al final de `YokeSC::Effects::commandWithPositionSource` (`include/SimEffects.h`), con la misma posicion que usa el calculo de fuerzas. Es la unica entrada que usa el firmware y se puede probar en PC, a diferencia de `main.cpp`. `command()` no cambia.
3. **Ley, con x = posicion normalizada y u = orden del eje:**
   - Si `u*x <= 0` (hacia el centro, o sin fuerza): sale `u`.
   - Si `u*x > 0` (hacia el tope): sale `u*f(|x|)`, donde `f` vale 1 hasta `ZONE_START`, baja en linea recta y vale 0 desde `ZONE_FULL`.
   - `u` o `x` no finitos: sale 0.
   - El damper que frena una marcha hacia el tope queda entero: se limita la suma sin damper y despues se le suma. Asi el limitador no quita amortiguamiento justo donde hace falta (critica de Embedded Firmware Engineer). El damper que empuja hacia el tope (yugo alejandose) se limita como el resto. En la zona se calcula `command()` dos veces, sin efecto sobre la friccion, que es idempotente para la misma posicion.
4. **Rampa y no corte:** evita un escalon de fuerza al cruzar el limite y no necesita histeresis ni estado. `f` llega a 0 antes de 1 porque el ruido y la histeresis del sensor pueden hacer leer 0,99 contra el tope.
5. **Constantes por eje en `mis_defines.h`**, porque el recorrido fisico es distinto (`ROLL_ENDSTOP_ZONE_START/FULL`, `PITCH_ENDSTOP_ZONE_START/FULL`). Valores iniciales 0,90 y 0,98 en ambos ejes: roll +-81 a 88 grados y pitch +-135 a 147 mm con los recorridos estimados. `static_assert`: `0 < START < FULL <= 1`. Sin interruptor de apagado ni ajuste por menu.
6. **Polaridad:** se aplica antes de `*_MOTOR_DIRECTION`. "Hacia el tope" es la misma convencion que ya usa el centrado (`u = -x*k` restaura).

## Invariantes

- I1. La fuerza hacia el tope nunca aumenta. `endstopLimited` cumple `|salida| <= |u|` sin cambiar el signo. Con damper frenando, la salida total puede cambiar de signo, siempre alejandose del tope.
- I2. Con `|x| >= ZONE_FULL`, la salida nunca empuja hacia ese tope (PWM 0 en ese sentido).
- I3. Con el damper sin frenar una marcha hacia el tope (en particular, yugo quieto), una orden hacia el centro es identica a la de antes, en cualquier posicion. Con damper frenando, dentro de la zona se quita la parte de los efectos que empuja hacia el tope y queda el frenado: la orden hacia el centro puede crecer, como maximo hasta el valor del damper (revision de Codex, H1). Es intencional: la vibracion no debe comerse el frenado justo cuando el yugo va hacia el tope. El aporte efectivo del damper es la diferencia entre la orden completa y la orden sin damper, ambas ya acotadas por `SIMCONNECT_EXTRA_LIMIT`. La saturacion puede reducirlo; si ambas ordenes quedan en el mismo limite, esa diferencia es cero y el limitador no puede recuperar el frenado perdido.
- I4. Con `|x| <= ZONE_START`, la salida es identica a la de antes, sea cual sea el modo de posicion.
- I5. La salida es continua en `x` y en `u`, incluido `x=0` y los bordes de la zona.
- I6. Sin estado ni operaciones lentas: costo despreciable en el lazo de 2 ms. No altera el estado de friccion ni el `f.cmd` deja de reflejar lo enviado al motor.

## Alcance y limites conocidos

No incluye calibracion nueva. El limitador es tan bueno como la posicion que recibe:

**Requisito del usuario (2026-11-05): el limitador funciona en los cuatro modos de posicion.** Los cuatro pasan por `commandWithPositionSource` (`src/main.cpp:374`), asi que el mismo codigo los cubre.

- **Potes (modo 2):** funciona con la calibracion del menu. Con los valores por defecto (0/2048/4095) puede no llegar a la zona: hay que calibrar. El menu acepta hasta 64 cuentas de recorrido total, pero con la histeresis de 4 cuentas el tope podria leerse 0,875; hacen falta al menos 200 cuentas por lado (mas si hay ruido).
- **Encoders (modo 0) y AS5600 con centro al encender (modo 3):** los extremos son fijos y relativos al encendido. Si se enciende fuera del centro, la zona queda corrida. Resolverlo requiere una referencia contra los topes: tarea aparte. El AS5600 con falla ya corta la fuerza en `main.cpp`.
- **Posicion del simulador (modo 1):** usa la posicion recibida; +-1 coincide con el tope solo si la placa que envia los ejes esta calibrada. **Sin posicion disponible (`useSimPosition && !rx.yokeAvailable`) la orden es 0:** sin posicion no se puede saber si se empuja contra un tope. Antes solo se quitaba el centrado. El puente actual siempre envia la posicion.
- **Motor frenado a mano contra el tope:** el centrado sigue circulando corriente. Solo lo resuelve el sensado de corriente, tarea aparte.

## Pruebas

Nuevo `tests/endstop_checks.h`, llamado desde `tests/simconnect_checks.cpp`:

- Tabla de la ley en ambos signos y ambos ejes: dentro, bordes, mitad de rampa, en el tope, `|x|>1`, no finitos.
- Barrido de `x` y `u` que comprueba I1 a I5.
- Integracion con `commandWithPositionSource`, con posicion local y con posicion del simulador: buffet o rodaje activos en `x=+-1` nunca dan fuerza hacia el tope; en `x=0` la orden coincide con `command()`; en modo simulador sin posicion la orden es 0.
- Las variantes de firmware de los cuatro modos compilan (`probar.ps1 -Todo`).

`probar.ps1 -Todo`. Ensayo con hardware (usuario, F-tope en `ENSAYOS.md`): yugo contra cada tope con buffet activo, motor sin acoplar o con limite bajo; el registro no debe mostrar PWM hacia ese tope, y al salir de la zona vuelve la vibracion completa. Ademas, en modo simulador: comprobar que el eje llega a 0,98 contra el tope, y sostener el yugo con mano suave cerca del tope para ver si aparece oscilacion. Una simulacion simple de un eje sugiere un ciclo limite con retardos de 20 a 50 ms (hipotesis, sin ensayo). Si oscila, la proteccion en ese modo se declara insuficiente.

## Documentacion afectada

- `LEEME md/AJUSTE_FUERZAS_Y_MOVIMIENTO.md`: las cuatro constantes y su efecto.
- `LEEME md/LEEME_SIMCONNECT.md`, junto a `SIMCONNECT_EXTRA_LIMIT`: el limitador se aplica despues de ese limite.
- `LEEME md/ENSAYOS.md`: ensayo F-tope.
- Tesis: se consulta al usuario. Si acepta, la actualiza Claude al aprobar la revision; Codex no la toca.

## Fuera de alcance

Calibracion de encoders y AS5600, referencia contra topes, sensado de corriente, ajuste por menu, cambios en constantes de seguridad existentes.
