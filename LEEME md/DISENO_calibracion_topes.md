# Diseno: calibracion de topes con finales de carrera en cada encendido

Rama `mejora/limite-topes`. Clasificacion: **critica** (logica de firmware con estado y bloqueo de fuerza): Claude disena e implementa; Codex revisa con lista de control. Continua `DISENO_limite_topes.md`.

## Objetivo

Pedido del usuario (2026-11-05): con una bandera de `mis_defines.h` activa, en cada encendido el usuario lleva el yugo a los 4 topes, cada uno con su final de carrera. Se registran las posiciones de los topes y el centro, y el limitador de topes los usa. Con la bandera apagada, todo sigue como en V2.9. El modo de posicion queda en simulacion (`YOKE_POSITION_SOURCE 1`, sin cambios).

Fuente de los pines: `DISENO_placa_pico2.md` tabla 4 (`FC_ROLL_NEG` GP0, `FC_ROLL_POS` GP1, `FC_PITCH_NEG` GP2, `FC_PITCH_POS` GP3; opticos de 3 pines con traductor BSS138). Los demas valores son hipotesis a validar con ensayo.

## Decisiones

1. **Bandera `YOKE_ENDSTOP_CALIBRATION`, valor 0 por defecto**, porque los finales de carrera todavia no estan instalados. Con 0, el codigo nuevo no se compila y el firmware se comporta igual que V2.9.
2. **Logica pura en `include/EndstopCalibration.h`**, sin HAL, para probarla en PC. `src/main.cpp` lee los pines y le pasa las muestras.
3. **Solo en RAM.** Se repite en cada encendido, incluso tras un reinicio por watchdog. No toca flash ni el formato de ajustes.
4. **Motores sin fuerza hasta terminar:** orden 0 en ambos ejes. El registro marca esos ciclos con `YokeLog::Menu`, que ya significa "orden anulada", para que `comparar_ensayo` no los cuente como violacion.
5. **Secuencia, en cualquier orden:**
   - Un final de carrera accionado (con antirrebote de `YOKE_ENDSTOP_SWITCH_DEBOUNCE_CYCLES`) registra la posicion mientras dure accionado, solo con muestra valida, y conserva el valor mas extremo de ese lado. Asi se absorbe el retardo de la posicion simulada.
   - Termina cuando los 4 estan registrados y ninguno sigue accionado, ni siquiera dentro del antirrebote: no se devuelve la fuerza con el yugo contra un tope.
6. **Centro = punto medio** de los dos topes de cada eje (mismo criterio que la calibracion de potes del menu). Coordenada para el limitador: `x_t = (p - c) / ((pos - neg) / 2)`; vale +-1 en los topes y puede superar 1.
7. **Rechazo:** se descarta todo, el LCD pide repetir y se empieza de nuevo, con los motores sin fuerza, en cualquiera de estos casos (critica de Embedded Firmware Engineer):
   - Los dos sensores de un eje accionados a la vez (sensor invertido o polaridad equivocada).
   - `pos - neg < YOKE_ENDSTOP_MIN_SPAN` (0,5 en unidades de posicion; cables NEG y POS cruzados o sensor mal ubicado).
   - Un tope registrado a mas de `YOKE_ENDSTOP_CAPTURE_TOLERANCE` (0,10) del extremo que alcanzo el yugo en esa calibracion (sensores de ejes distintos cruzados, si los ejes se mueven de a uno).
   - El valor registrado a mas de esa tolerancia de la ultima muestra con el sensor accionado (muestra suelta hacia afuera).
   No se agrega recorrido maximo: los recorridos de encoder son estimaciones y podria rechazar calibraciones validas.
8. **Muestra valida segun el modo:** simulacion, `yokeAvailable && fresh(now)` (vuelo corriendo y sin pausa); potes y encoders, siempre; AS5600, `healthy()`.
9. **Los datos solo alimentan el limitador**, como pidio el usuario. Centrado, damper y friccion siguen usando la posicion del modo. Por eso se agrega `Effects::commandWithStopPosition`, que separa la posicion de fuerzas de la posicion de topes; `commandWithPositionSource` la llama con la misma posicion.
10. **Entrada sin recortar en modos locales:** se calibra con `positions[axis]`, antes de `YokeMotion`, porque un encoder encendido fuera del centro puede pasar de +-1. Los potes ya recortan a +-1 y requieren su calibracion del menu.
11. **Pines y polaridad configurables.** `ENDSTOP_SWITCH_ACTIVE_LEVEL`, 0 por defecto: **a confirmar con los sensores reales**, porque los opticos pueden activar en alto. Entrada con pull-up interno. Un `static_assert` rechaza pines que choquen con otras funciones del modo compilado: con encoders (modo 0) y los pines por defecto no compila, a proposito.
12. **LCD:** `CALIBRAR TOPES` y en la segunda fila los topes pendientes (`R- R+ P- P+`), `SIN POSICION`, `SOLTAR TOPE` o `REPETIR TOPES`. El menu tiene prioridad al mostrarse.
13. **Fuera de la calibracion, los finales de carrera no se usan.**

## Invariantes

- C1. Con bandera 0, firmware y fuerzas iguales a V2.9.
- C2. Con bandera 1, mientras `!ready()`, orden y PWM son 0 en ambos ejes.
- C3. `ready()` solo se activa con los 4 topes registrados con muestras validas y finitas, los 4 sensores sueltos (filtrados y en la lectura de ese ciclo) y sin ninguna condicion de rechazo de la decision 7.
- C4. Una vez listo, el resultado no cambia hasta reiniciar, sea cual sea el estado de sensores y posicion.
- C5. Un rebote mas corto que el antirrebote no registra nada.
- C6. La calibracion no modifica la posicion de centrado, damper ni friccion, y orientacion de `x_t` igual a la del sensor.
- C7. Sin esperas, memoria dinamica ni I2C nuevo en el lazo de 2 ms.

## Limites conocidos

- **Sensores de ejes distintos cruzados con los dos ejes movidos juntos:** no se detectan (revision de Codex, 2026-11-06). Con movimientos diagonales las senales no distinguen el eje. Se acepta porque todo valor registrado es una muestra del recorrido observado: el tope aprendido nunca queda mas afuera que lo recorrido, asi que en el peor caso el limitador actua antes, y los sensores no se usan fuera de la calibracion. Rechazar las diagonales invalidaria calibraciones correctas sin ganar seguridad. El cableado se comprueba moviendo un eje por vez (F-calibracion, paso 4).
- Un sensor cuya zona activa sea mas ancha que la tolerancia hace rechazar siempre: la falla es segura y se ajusta con la constante.
- **Modo simulacion:** hay que tener un vuelo corriendo y sin pausa para terminar, con el puente armando la telemetria. Si el eje del simulador satura antes del tope fisico, el registro queda en ese valor y la zona se adelanta. Si se cambia la asignacion de ejes durante la sesion, hay que reiniciar.
- Un sensor que quede siempre accionado o desconectado (segun polaridad) impide terminar: los motores quedan sin fuerza. Es la falla segura.
- Si un sensor se dispara antes del tope real, el tope se registra antes y el limitador actua antes.
- El centro registrado no recentra el resorte: en encoders o AS5600 encendidos fuera del centro, el centrado sigue corrido.

## Pruebas

En `tests/endstop_checks.h`: las 24 ordenes posibles, topes asimetricos y centro, antirrebote, muestra invalida o no finita, valor mas extremo, espera a soltar, sensor que se vuelve a accionar durante el ultimo soltado, rechazos (NEG y POS cruzados, ejes cruzados, sensor invertido, recorrido corto, muestra suelta) con reinicio, el limite conocido de los ejes movidos juntos, estabilidad despues de listo, textos del LCD, y `commandWithStopPosition` (limita segun la posicion de topes sin cambiar el resorte; coincide con `commandWithPositionSource` si ambas posiciones son iguales).

Compilacion: nuevas variantes `myk_topes` (modo 1) y `myk_topes_as` (modo 3), con `probar.ps1 -Topes`, incluido en `-Todo`.

Ensayo con hardware (usuario, F-calibracion en `ENSAYOS.md`): sin fuerza antes de terminar; polaridad de los sensores y nivel de salida con pull-up interno (medir); orden libre; cables cruzados rechazados; despues, F-tope con los datos registrados.

## Documentacion afectada

- `LEEME md/AJUSTE_FUERZAS_Y_MOVIMIENTO.md`, seccion "Limitador de topes"; `LEEME md/ENSAYOS.md`; `LEEME md/LEEME_PICO2.md`, tabla de pines y compilacion; `LEEME md/INVENTARIO_CODIGO.md` si lista cabeceras.
- `tesis/capitulos/09_damper.tex`, seccion `sec:limite_topes`: la actualiza Claude al aprobar la revision; Codex no la toca. Al final se genera el PDF (pedido del usuario).

## Fuera de alcance

Guardar la calibracion en flash, usar los sensores en vuelo, recentrar el resorte, habilitacion OE y deteccion de 5 V de la placa.
