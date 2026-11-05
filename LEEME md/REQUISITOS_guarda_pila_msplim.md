# Requisitos: guarda de pila con MSPLIM (firmware Pico 2)

Documento comun para las dos ramas de la comparacion de flujos. Dice que debe lograrse, no como.

## Contexto

- El 13/10 hubo un HardFault real (`reset=WDT falla=1 etapa=0`) que no se reprodujo. El ensayo posterior midio entre 448 y 688 B libres de los 2048 B de pila, por debajo del umbral de 512 B en algun caso: es evidencia a favor de un desborde (ver "Ensayos del usuario" en `HANDOFF.md`).
- Hoy un desborde corrompe la RAM contigua sin aviso, porque `MSPLIM` queda en 0 despues del arranque del SDK.
- Punto de partida: `isr_hardfault`, `hardfaultCapture`, `setup()` y `YOKE_TEST_HANG` en `src/main.cpp`; `YokeHealth` en `include/LoopSupervisor.h`; `herramientas/ensayos/FALLA.ps1`; `LEEME md/DISENO_diagnostico_hardfault.md`.

## Requisitos

- **R1.** El nucleo 0 ejecuta con `MSPLIM` activo durante toda la operacion normal, de modo que un desborde de la pila principal se detecte en el momento.
- **R2.** Ante un desborde, los motores quedan seguros (EN y PWM en bajo) y el diagnostico se guarda; luego el watchdog reinicia la placa. Esto debe cumplirse tambien si el desborde ocurre durante una interrupcion o al apilar una excepcion.
- **R3.** En el arranque siguiente, la linea `BOOT` y `FALLA.ps1` permiten distinguir un desborde de pila de otra falla. El diagnostico nunca presenta como valido un dato que no lo sea. Las fallas que no son de pila siguen informando lo mismo que hoy.
- **R4.** La medicion de pila libre (linea `S`) sigue funcionando y su valor sigue siendo interpretable; documentar si cambia su significado.
- **R5.** Existe un ensayo controlado de desborde, compilado solo en una variante de prueba, para que el usuario lo valide con hardware. Sin efecto en las variantes normales.
- **R6.** La logica que pueda probarse en PC tiene pruebas en `tests/`.
- **R7.** Sin cambios en constantes de fuerza, tiempos del lazo, orden de etapas ni watchdog. El firmware sigue dentro del limite de flash.
- **R8.** Se actualiza la documentacion `LEEME` afectada, solo en las partes afectadas. `tesis/` no se toca.

## Verificacion

- `powershell -ExecutionPolicy Bypass -File herramientas/compilacion/probar.ps1 -Todo -PruebaFalla` termina con codigo 0.
- Se entrega el procedimiento del ensayo con hardware (R5) y que resultado esperar. La confirmacion final con hardware queda a cargo del usuario.

## Reglas de esta comparacion

- Trabajar solo en la rama indicada. No integrar a otras ramas ni hacer push.
- No ejecutar `nueva-version.ps1`: solo la rama elegida genera version, despues de la comparacion (decision del usuario, 2026-11-05).
- No editar `HANDOFF.md` en la rama de trabajo, salvo que el plan lo pida.
