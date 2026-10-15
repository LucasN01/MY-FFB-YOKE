# Ensayos del Yoke SC8

Los scripts de ensayo (ENSAYO, CICLOS, ARRANQUE, BANCO, FALLA) están en herramientas/ensayos/; los resultados quedan en ensayos/ de la raíz.

Primera sesión: fuente de potencia de motores apagada. Con motores, solo después de instalar el pull-down de EN pendiente de V1.8.

Antes de cada ensayo, comprobar en el menú que REGISTRO DATOS esté en Activado; si no, los CSV quedan vacíos y el informe da SIN DATOS. No cambiarlo durante un ensayo.

## Vuelo

Abrir `herramientas\ensayos\ENSAYO.cmd` o ejecutar desde la raíz del repositorio:

```powershell
powershell -ExecutionPolicy Bypass -File herramientas/ensayos/ENSAYO.ps1 -Nombre vuelo1 -Rutina corta
```

`-Rutina completa` es el valor predeterminado; `corta` incluye F1, F3, F4, F5, F7, F8 y F11. Preparar C182RG o avión habitual en cabecera, motor en marcha, frenos puestos, clima sin turbulencia y potes centrados, con `INICIAR SIMULADOR V3.cmd`.

El script crea `ensayos/AAAAMMDD-HHMMSS_<Nombre>/`, `rutina.txt` y `marcas.csv`. Presionar Enter una vez antes de F1. Las fases se detectan automáticamente: durante el vuelo no hace falta volver a la consola. Opcionalmente, escribir un número de fase y Enter en pausa o tierra agrega una marca con el mismo reloj `host_time_ms` de los registros. Al terminar, cerrar el simulador o puente y escribir `FIN`. Tras esperar hasta 120 s, copia los cinco registros, ejecuta el comparador y muestra `informe.txt`.

| Fase | Duración | Qué hacer | Qué debería verse (lo verifica `comparar_ensayo`) |
| --- | --- | --- | --- |
| F1 Pausa | 15 s | Pausar el simulador (P). | `armed=0`, `cmd=0`, `pwm=0` (B6). |
| F2 Menú | 30 s | En pausa: entrar al menú con los pulsadores, recorrer 2-3 pantallas y **salir sin guardar** (guardar en flash produce un hueco del lazo y pérdidas legítimas). | `flags&Menu`, `cmd=0` (B2); `ganancias.csv` con los valores vigentes. |
| F3 Parado | 20 s | Quitar pausa, quieto con frenos. | `gs≤0,5` ⇒ `env=0`, suelo 0 (B5). |
| F4 Potes parado | 30 s | Roll a tope izq/der y pitch adelante/atrás, despacio, 2 veces. | Centrado con signo opuesto a la posición (C); `q≈0`, rigidez = reposo. |
| F5 Rodaje | 60 s | Soltar frenos, rodar 10→30 kt en recta. | `env` crece con la velocidad; `ground` ≠ 0; `peak` roll ≤ 40, pitch ≤ 4 por suelo. |
| F6 Despegue | ~60 s | Potencia máxima, rotar a ~60 kt, subir a 1000 ft. | Al despegar (`on_ground=0`) suelo 0 inmediato (B4). |
| F7 Crucero | 60 s | Nivelado, 100-120 kt, potes quietos. | `pressure≈(ias/ref)²`; buffets 0; `gust` 0 sin turbulencia. |
| F8 Potes en vuelo | 30 s | Mismo movimiento que F4. | Centrado mayor que en F4 (`center_stiffness` > F4). |
| F9 Tren y flaps | 60 s | Bajar tren y flaps por etapas dentro de los límites; luego subir. | `buffet_gear` > 0 con tren extendido y velocidad; 0 al retraer. |
| F10 Pérdida | 60 s | Potencia baja, levantar nariz despacio hasta la bocina/pérdida, recuperar. | `buffet_stall` crece al acercarse `aoa` a `stall_alpha`. |
| F11 Pausa en vuelo | 10 s | Pausar (P) y quitar. | `cmd=0` en ≤150 ms (B6), luego retoma. |
| F12 Slew | 10 s | Activar slew (Y) 5 s y desactivar. | Modelos reiniciados y `cmd=0` durante slew. |
| F13 Turbulencia (opcional) | 60 s | Clima con turbulencia moderada. | `gust` ≠ 0 (`--gusts` ya lo pasa el lanzador, `Iniciar-Simulador.ps1:255`). |
| F14 Aterrizaje | libre | Aterrizar y rodar hasta parar. | Suelo vuelve al tocar; 0 al detenerse. |

## Banco sin simulador

Cerrar Prepar3D y el puente, conectar la Pico y ejecutar:

```powershell
powershell -ExecutionPolicy Bypass -File herramientas/ensayos/BANCO.ps1 -Entrada tmp\vuelo_corto\telemetry.csv
```

Detecta el COM con el lanzador; se puede indicar `-Puerto COMn`. Crea una carpeta con sufijo `_banco`, conserva la entrada como `entrada.csv`, ejecuta el replay y archiva `telemetry.csv`, `pico.csv`, `ganancias.csv`, `pico.log` y `bridge.log` junto al informe.

Por defecto funciona en monitor: B se evalúa y A queda `SIN DATOS`. `-Armar` genera PWM real y exige escribir `APAGADA` con la fuente de motores apagada; de lo contrario sale sin armar. Codex solo puede usar `-Armar -Confirmado` si HANDOFF contiene la confirmación humana de fuente apagada con fecha del día.

## Columnas de pico.csv

Cada fila es una muestra calculada o el registro periódico de al menos cada 100 ms. `host_time_ms` es el reloj de Windows y `sample_time_ms` coincide con la telemetría enviada; `seq` es la secuencia del registro y `revision` identifica la muestra modelada.

- `flags`: 1 efectos activos, 2 armado, 4 pausa, 8 menú, 16 DTR, 32 telemetría cruda, 64 valor no finito, 128 modelo calculado.
- `pressure`, `gust_roll`, `gust_pitch`, `buffet_stall`, `buffet_gear`, `buffet_spoiler`, `ground_roll`, `ground_pitch`, `ground_envelope`: resultados de los modelos.
- `center_stiffness`, `pos_roll`, `pos_pitch`, `vel_roll`, `vel_pitch`: centrado y movimiento por eje.
- `cmd_roll`, `cmd_pitch`, `pwm_roll`, `pwm_pitch`, `peak_pwm_roll`, `peak_pwm_pitch`: comandos, PWM y picos desde la fila anterior.
- `dropped`, `skipped`: registros perdidos y muestras recibidas que no llegaron a modelarse.

## Informe

`informe.txt` contiene A (fidelidad frente al mismo modelo en PC: 99 % de coincidencias por señal y al menos 95 % de muestras esperadas emparejadas), B1–B9 (invariantes de seguridad, pausa, pérdidas y registros), C (tiempos y comportamiento informativo) y el resumen de fases. B6 mide transiciones armado→desarmado en hasta 150 ms; un rearmado anterior al primer cero dentro de esa ventana no se evalúa. B7 exige pérdidas de Pico y puente de hasta 2 %.

Los tramos con `skipped` se excluyen de A hasta recuperar el estado y se informan, junto a las muestras sin pareja. Si solo hay `pico.log`, B6 y las métricas Pico por fase quedan `SIN DATOS` porque falta el reloj de Windows. `resumen_fases.csv` incluye orden cronológico (`03 RODAJE`), marca opcional, duración, actividad y medias/máximos. El orden cronológico es independiente del número de rutina.

`RESULTADO: OK` exige A y B; `FALLO` indica incumplimientos; `SIN DATOS` corresponde a registros sin filas armadas (por ejemplo, monitor), salvo que falle B. El comparador devuelve respectivamente 0, 1 y 2. Si `telemetry.csv` no tiene filas (REGISTRO DATOS desactivado), el informe agrega la línea `REGISTRO:`, da `RESULTADO: SIN DATOS` y devuelve 1.

## Ciclos lentos (CICLOS.cmd)

| Paso | Acción | Duración | Para qué |
| --- | --- | --- | --- |
| P0 | UF2 nuevo; lanzador; avión en pista; pausa | — | base |
| P1 | No tocar nada (en pausa) | 10 min | control negativo: 0 `W` |
| P2 | REGISTRO DATOS → Desactivado → OK; luego → Activado → OK | 2 min | control positivo: 2 `W` FLASH; prueba la opción |
| P3 | Menú: recorrer las 4 listas y salir sin guardar, 3 veces, con 1 min entre cada una | 5 min | H2 con puente |
| P4 | Quitar pausa y volar | 5 min | operación |
| P5 | Cerrar el simulador. La Pico queda sola, conectada | 15 min | caso del 12/10 |
| P6 | Sin simulador: menú, cambiar de lista 3 veces, sin guardar | 5 min | H2 + H3 |
| P7 | Sin simulador: `powershell -ExecutionPolicy Bypass -File Iniciar-Simulador.ps1 -DetectarPlaca`, 3 veces, con 1 min entre cada una | 5 min | H1 aislada |
| P8 | Sin simulador: REGISTRO DATOS → Desactivado → OK; esperar 2 min | 3 min | control de `hace` (~120 s) |
| P9 | Abrir el simulador; pausa 2 min; REGISTRO DATOS → Activado → OK; cerrar | 4 min | recibe los eventos de P5-P8 |

## Arranque y fallas (ARRANQUE.cmd, motores apagados)

Sin -Elf, ARRANQUE elige el .elf por la linea BOOT: etapa=15:PRUEBA usa build-pico2-prueba-falla; si no, build-pico2 (configuracion de mis_defines.h; para otro UF2 pasar -Elf).

| Paso | Acción | Qué se espera |
| --- | --- | --- |
| V1 | Cargar `build-pico2-prueba-falla\MiYoke_PD.ino.uf2` (BOOTSEL). **Esperar 30 s.** Ejecutar `ARRANQUE.cmd` | `BOOT reset=WDT ... etapa=15:PRUEBA falla=1 exc=0 pc=...`; `FALLA.ps1` con `-Elf build-pico2-prueba-falla\MiYoke_PD.ino.elf` lleva el PC a `loop` |
| R1 | Cargar el UF2 normal `build-pico2\MiYoke_PD.ino.uf2`. Esperar 30 s. `ARRANQUE.cmd` | `ENCENDIDO` (o `WDT falla=1`: se captura) |
| R2 | Desenchufar y enchufar la Pico, esperar 30 s, `ARRANQUE.cmd`; 3 veces | `ENCENDIDO` |
| R3 | Volver a cargar el **mismo** UF2 normal, esperar 30 s, `ARRANQUE.cmd`; 2 veces | `ENCENDIDO`; si aparece `falla=1`, está ligada al primer arranque tras la carga |
| R4 | Uso normal en los días siguientes | Cada vuelo deja su `BOOT` y su `stack libre minimo` en `bridge.log` |

- **Decisión:**
  - con `falla=1` en R1-R4, `exc` (hilo o IRQ), PC y LR con `FALLA.ps1` indican dónde se produjo, y la corrección se
    diseña en otra tarea;
  - si el stack libre mínimo cae por debajo de 512 B, se toma como evidencia de H-b;
  - sin fallas, "no reproducido", y el diagnóstico queda activo.
- No recompilar entre la carga del UF2 y el uso de `FALLA.ps1`.
- Si `ARRANQUE.cmd` se ejecuta antes de los 20 s en V1, la falla ocurre con el puente conectado (código 4): esperar y
  repetir `ARRANQUE.cmd`.
