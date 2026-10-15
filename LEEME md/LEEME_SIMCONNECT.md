> Nuevo: [menu de calibracion de ganancias y cableado de pulsadores/LED](MENU_CALIBRACION.md). Guarda las ganancias en la Pico y arranca en modo normal.

> Ajustes actuales (01/10/2026): [AJUSTE_FUERZAS_Y_MOVIMIENTO.md](AJUSTE_FUERZAS_Y_MOVIMIENTO.md). Fuente unica simulacion/encoders, sin potenciometros.

> Configuracion actual: esta Pico recibe SimConnect por USB CDC y controla motores, sin joystick HID ni envio de ejes a Prepar3D. Otra placa maneja los ejes del simulador. La lectura de sensores y la velocidad filtrada local (YokeMotion.h) se conservan.
> Se utiliza la respuesta YOKE17SC5 que identifica la placa ante el puente y el diagnostico serie; ninguno controla ejes del simulador. Verificar el puerto COM tras cargar el nuevo firmware.

# Pico 2 y Prepar3D v3: solo SimConnect

Actualizacion 30/09/2026. No requiere habilitar FFB nativo en Prepar3D.

## Comportamiento

- Eliminados CONSTANT/SINE por USB y amortiguacion local. Centrado dinamico y rodaje por SimConnect: ver CENTRADO_Y_RODAJE.md.
- Conservados turbulencia, buffet de perdida, tren desplegado y spoilers, todos calculados desde SimConnect.
- El buffet de tren es aerodinamico en vuelo, no vibracion de rodaje.
- En pausa, slew o simulacion detenida, el puente envia el estado desarmado. El firmware retira toda la fuerza al recibirlo, incluso las envolventes de vibracion pendientes.
- Sin paquetes validos/armados durante 300 ms, o al caer DTR del puerto serie, el comando de ambos motores es cero. No hay fuerza de respaldo sin telemetria.
- La habilitacion USB PID/FFB ya no interviene. CDC recibe telemetria; no se registran interfaces joystick HID.

## Cargar firmware

Ejecutar `./Build-Pico2.ps1` para la fuente elegida por YOKE_USE_SIMULATION (1=simulacion, 0=encoders, 2=potenciometros), o `./Build-Pico2.ps1 -Encoders` para forzar encoders; `./Build-Pico2.ps1 -Potentiometers` fuerza ADC0/ADC1.
Con BOOTSEL, copiar a la unidad RP2350 el archivo correspondiente:

- `build-pico2/MiYoke_PD_V2.3.ino.uf2`
- `build-pico2-encoders/MiYoke_PD_V2.3.ino.uf2`

No usar los antiguos archivos V2.1 que puedan seguir en esas carpetas. Ver LEEME_PICO2.md para pines y configuracion del IDE. Verificar el nuevo puerto COM si Windows lo reasigna al retirar la interfaz HID. Los ejes del simulador se configuran en la otra placa.

## PC con Prepar3D v3

El puente es Win32 y enlaza contra la interfaz heredada FSX-SP2-XPACK, no contra una API exclusiva de Prepar3D v4. La ruta v4 del proyecto corresponde al SDK disponible en la PC de compilacion; no es una ruta que necesite el EXE al ejecutarse.
Prepar3D v3 documenta soporte para clientes heredados y sus instaladores:
[Documentacion oficial SimConnect v3](https://prepar3d.com/SDKv3/LearningCenter/utilities/simconnect/simconnect.html).

1. Copiar `SimConnectBridge/bin/Win32/Release/YokeSimConnectBridge.exe` a una carpeta con permiso de escritura en la PC de destino.
2. Instalar el cliente SimConnect FSX-SP2-XPACK desde `redist/Interface/FSX-SP2-XPACK/retail/lib/SimConnect.msi` de Prepar3D si no esta instalado. El puente tambien requiere el runtime de Visual C++ x86 correspondiente a v141.
3. Conectar la Pico y cerrar el Monitor Serial u otro programa que ocupe su COM.
4. Abrir Prepar3D v3 y cargar un vuelo. Ejecutar en PowerShell, cambiando COM7 por el puerto real:

```powershell
.\YokeSimConnectBridge.exe --port COM7 --enable --gusts --gust-gain 1
```

`--enable` autoriza fuerza, `--gusts` activa el viento y los tres buffets se calculan por defecto. `--no-buffets` desactiva perdida/tren/spoilers; conserva centrado y rodaje. Ctrl+C termina el puente. Sin `--enable` no se arma la fuerza.

Para revisar la conexion sin enviar fuerzas ni abrir el COM:

```powershell
.\YokeSimConnectBridge.exe --seconds 60 --gusts
```

`bridge.log`, `telemetry.csv`, `pico.log`, `pico.csv` y `ganancias.csv`, junto al EXE, se reemplazan en cada ejecucion; si el `pico.log` anterior no estaba vacio, la generacion previa se conserva como `*.anterior.*`. Comprobar conexion y variables del avion en destino: no se ha probado fisicamente con Prepar3D v3 ni con los motores.

## Ajustes

En `mis_defines.h`:

- `SIMCONNECT_TOTAL_GAIN`: ganancia global (90%, rango 0..100).
- `SIMCONNECT_ADDITIONS_ENABLED`: interruptor general conservado por compatibilidad de configuracion; en 0 todos los motores quedan sin comando.
- `SIMCONNECT_TURBULENCE_ENABLED`, `SIMCONNECT_STALL_BUFFET_ENABLED`, `SIMCONNECT_GEAR_BUFFET_ENABLED`, `SIMCONNECT_SPOILER_BUFFET_ENABLED`: interruptores independientes.
- `SIMCONNECT_GUST_ROLL/PITCH` y `SIMCONNECT_*_BUFFET_GAIN`: intensidades individuales.
- `SIMCONNECT_EXTRA_LIMIT`: limite conjunto conservado, 220/255 = 0,862745 del comando normalizado.

No existen ya `FFB_CONSTANT_GAIN`, `FFB_PERIODIC_GAIN` ni ajustes activos de amortiguacion. La telemetria de diagnostico `f` muestra el comando SimConnect en unidades de 1/10000; `p` indica PWM con polaridad.

Se utiliza protocolo SC5 e identificacion `YOKE17SC5`. Actualizar puente y firmware juntos. Tierra permite rodaje y centrado; los efectos aerodinamicos se calculan solo en vuelo. Ver CENTRADO_Y_RODAJE.md para las dos banderas nuevas y ajustes.

## Compilacion del puente y pruebas

`Build-Bridge.ps1` utiliza Visual Studio/v141 y el SDK FSX-SP2-XPACK disponible en esta PC. El proyecto permite sobreescribir `P3DSdkDir` desde MSBuild con una carpeta equivalente que contenga `inc` y `lib`.
Las pruebas ejecutables se compilan con `tests/simconnect_checks.vcxproj`. Cubren protocolo, CRC, secuencias, watchdog, desarmado, DTR, ganancias y los cuatro efectos. La compilacion no carga firmware ni acciona motores.

