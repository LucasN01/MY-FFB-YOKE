# Rodaje reactivo con progresion por velocidad (SC7)

La fuerza de suelo responde a cambios de aceleracion del avion. Se conserva
exactamente la envolvente anterior, usando velocidad respecto al suelo:

- Hasta 0,5 kt inclusive: cero.
- Entre 0,5 y 40 kt: (velocidad - 0,5) / 39,5.
- Desde 40 kt: multiplicador 1.

El maximo desde 40 kt es la ganancia disponible: la fuerza instantanea sigue
variando con las sacudidas. Una pista sin movimiento medido puede quedar silenciosa.
La velocidad por si sola no genera una vibracion continua.

## Adaptacion para Prepar3D

TelemFFB filtra compresion de ruedas. El SDK de P3D v4 no enumera
CONTACT POINT COMPRESSION; si documenta ACCELERATION BODY X/Y, en ft/s^2:
https://www.prepar3d.com/SDKv4/sdk/references/variables/simulation_variables.html

Este puente usa esas aceleraciones, no una variable de MSFS sin verificar.
No identifica cada rueda ni mide directamente baches: responde al movimiento
que el modelo de vuelo efectivamente entrega. Tambien puede responder a cambios
rapidos de maniobra en suelo. Es una aproximacion haptica, no un modelo de la
transmision mecanica del tren al yoke.

GroundModel.h elimina la componente lenta con un pasaaltos de 3 Hz y suaviza
las muestras con un pasabajos de constante de tiempo 20 ms. Usa el dt real.
Una aceleracion filtrada de 0,25 g alcanza la senal normalizada maxima.
El mapeo elegido conserva la predominancia de roll del equipo:

    roll = limitar((0,3 * lateral_filtrada + 0,7 * vertical_filtrada) / 8,043512)
    pitch = limitar(vertical_filtrada / 8,043512)
    fuerza_enviada = senal * envolvente_velocidad

El firmware aplica la ganancia Ground del menu, la global, el techo de 40/255
y la reduccion de pitch por 10 que ya existian. Las senales son independientes:
no se garantiza que la fuerza de roll sea siempre diez veces la de pitch.
No se agrega ruido aleatorio ni una portadora periodica en SC7.

Se reinicia el filtro al parar, salir del suelo, pausar, activar slew,
recibir datos invalidos o una separacion de muestras >=250 ms. La primera
muestra valida establece una referencia sin producir un golpe de arranque.
Esto tambien evita convertir la primera muestra tras aterrizar en un golpe
artificial; no se implementa un efecto de touchdown separado.

## Protocolo y puesta en uso

SC7 extiende el paquete de 26 a 30 bytes: fuerzas de suelo con signo en 24/26
(-10000..10000) y CRC en 28. El campo ground_amp conserva la envolvente de velocidad.
Las fuerzas ya incluyen esa envolvente y no se multiplican nuevamente en la placa.
El receptor mantiene SC1-SC6 para compatibilidad, incluido su rodaje antiguo.
El puente nuevo exige identificacion YOKE17SC7; no envia paquetes nuevos a SC6.

1. Compilar con Build-Bridge.ps1 y Build-Pico2.ps1 (configuracion actual: potenciometros).
2. Cargar el UF2 nuevo de build-pico2/MiYoke_PD_V2.4.ino.uf2 en la Pico 2.
3. Usar INICIAR SIMULADOR.cmd o INICIAR SIMULADOR V3.cmd. El lanzador apunta a
   SimConnectBridge/bin/Win32/RealismoSC7/YokeSimConnectBridge.exe.

No se cargo firmware ni se accionaron motores durante esta modificacion.
Los ejecutables anteriores permanecen en sus carpetas. Copias del codigo anterior
estan en backups/suelo-antes-sc7. Si se desea volver a SC6, hay que restaurar
tambien el lanzador y usar el firmware correspondiente, no mezclar versiones.

## Validacion

Compilacion del puente Win32 correcta (solo advertencias signed/unsigned del
encabezado SimConnect). Suite completa: 85.521 comprobaciones correctas,
incluidas las nuevas de SC7; el texto final del ejecutable conserva la etiqueta
historica SC1-SC6. Pico 2 ARM/TinyUSB con configuracion actual de potenciometros:
91.772 bytes de programa y 17.884 bytes de RAM global. UF2 generado correctamente.
Sintaxis PowerShell del lanzador comprobada y columnas CSV verificadas (42).

Pruebas automaticas: envolvente exacta, misma respuesta con mitad de ganancia a
20,25 kt, saturacion desde 40 kt, respuesta a impulsos y silencio ante carga
constante, reinicios, CRC, valores con signo, limites, ausencia de oscilador,
watchdog, pausa, desconexion y compatibilidad con versiones anteriores.

El CSV agrega ground_accel_x_ft_s2, ground_accel_y_ft_s2, ground_roll y
ground_pitch. Vibracion_Tierra indica senal calculada no nula estando armado;
no es una confirmacion de PWM fisico.

Pendiente: prueba real de rodaje en Prepar3D y ajuste de sensibilidad al avion.
Comparar pista lisa y terreno irregular, verificar cero hasta 0,5 kt y al
despegar. Si las aceleraciones no varian, no se inventan baches: primero revisar
el CSV. El resultado fisico no queda validado por la compilacion.
