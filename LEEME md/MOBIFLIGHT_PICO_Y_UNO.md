# Pico 2 sin joystick y banco de pruebas con Arduino Uno

Revisión del proyecto: 01/10/2026.

> Corrección de compatibilidad tras comprobar el ejecutable local: esta PC tiene
> Prepar3D **4.0.23.21468**. No instalar FSUIPC6 6.2.2 en esta versión: produjo
> un aviso de incompatibilidad y el usuario recuperó el arranque al desinstalarlo.
> El soporte oficial recomienda la última v4.5 para FSUIPC6 actual. Tampoco
> instalar a ciegas FSUIPC5 5.157: el manual incluido en su descarga oficial
> indica **4.1 como versión mínima**. Las instrucciones genéricas de instalación
> de FSUIPC6 que aparecen más abajo requieren primero actualizar el simulador
> a una versión compatible. No se actualizó Prepar3D ni se instaló FSUIPC5.
> Fuente: https://fsuipc.com/community/fsuipc6-support/fsuipc6-2-2-p3dv4-simconnect-error/
> Manual verificado: C:/Users/norie/Downloads/FSUIPC5-verificacion/Installing and Registering FSUIPC5.pdf

## Qué está comprobado y qué falta comprobar

El sketch V2.3 usa USB CDC (puerto serie), sin crear un joystick HID.
El puente recibe datos de SimConnect y los transmite a la Pico; no escribe
posiciones de los mandos en el simulador. La respuesta serie actual es YOKE17SC4.
No hace falta quitar más funciones del firmware para eliminar el joystick.
Esto no demuestra qué firmware está cargado en la placa de la otra PC.

La foto muestra errores de mapeo de ejes de un dispositivo llamado Arduino
Leonardo. No permite identificarlo como la Pico. También muestra FSUIPC error
14 (no está ejecutándose), seguido aproximadamente un segundo después por
Connected to Prepar3D [FSUIPC]. Puede ser una desconexión transitoria; hay que
observar si vuelve a ocurrir durante la falla de los mandos.

## Cargar la versión correcta

1. Cerrar el puente y el monitor serie. Para cargar firmware, mantener apagada
   la alimentación de potencia de los motores.
2. Desconectar la Pico del USB. Mantener BOOTSEL mientras se vuelve a conectar.
3. Copiar `build-pico2/MiYoke_PD_V2.3.ino.uf2` a la unidad de arranque RP2350.
   Este archivo usa la posición de SimConnect (YOKE_USE_SIMULATION=1), como
   está configurado actualmente. No elegir los archivos V2.1 o V2.2 que también
   están en esa carpeta. La variante de encoders es para sensores locales.
4. Dejar que reinicie y anotar el COM que aparece en Administrador de dispositivos.
5. Abrir Win+R, escribir `joy.cpl` y comparar la lista con la Pico conectada y
   desconectada. Con este firmware no debe aparecer un joystick adicional.
   El puerto COM sí debe aparecer: es necesario para recibir la telemetría.
6. Usar exclusivamente ese COM en el puente. No instalar firmware MobiFlight
   en esta Pico; reemplazaría el programa de PWM y LCD.

## Prueba en la PC de los Mega

Guardar una copia de las configuraciones de MobiFlight antes de modificarlas.

1. Con Pico desconectada, abrir un vuelo y MobiFlight. Confirmar que yugo,
   acelerador y pedales funcionan y que FSUIPC está conectado.
2. Conectar la Pico dejando cerrado el puente. Repetir la prueba.
3. Abrir el puente con el COM de la Pico y sin habilitar fuerza inicialmente:

   ```powershell
   .\YokeSimConnectBridge.exe --port COM7 --seconds 60
   ```

   Sustituir COM7 por el COM comprobado; nunca por el de un Mega o Pro Micro.
4. Si falla ya en el paso 2, revisar qué dispositivo aparece/desaparece en
   `joy.cpl`, si los Mega se desconectan y si cambia el estado de FSUIPC.
   Si hay reinicios USB, probar otro puerto/cable y sin hub compartido.
5. Si falla únicamente en el paso 3, verificar el COM elegido y guardar el log
   de MobiFlight y bridge.log. El puente no debe abrir el COM de los mandos.
6. Si solo falla al habilitar potencia/PWM, revisar alimentación y perturbaciones
   eléctricas; eso no se corrige ocultando un joystick.

En versiones que ofrecen esta opción, MobiFlight permite excluir un joystick
en Extras > Controller Management > Peripherals, desmarcando ese dispositivo.
Excluir solo el dispositivo identificado; otros mandos pueden depender de HID.
Esta opción no oculta el joystick a Prepar3D. Para la Pico, la solución es usar
el firmware CDC. No deshabilitar su puerto serie en Windows.

Prepar3D v3 utiliza FSUIPC4 (la distribución oficial 4.977 indica P3D 1.4–3.4).
FSUIPC6 corresponde a P3D 4–6. Que el puente muestre telemetría no comprueba
que FSUIPC esté disponible para MobiFlight: son dos conexiones distintas.

## Banco local con Arduino Uno R3

Confirmado por el usuario: Uno R3/ATmega328P, Prepar3D v4 y sin MobiFlight instalado.
La documentación actual admite seis
entradas analógicas; el Uno R4 figura como no soportado. El Uno lleva el firmware
de MobiFlight; la Pico mantiene MiYoke. No conectar señales de 5 V del Uno a GPIO
de la Pico.

Flujo de control: potenciómetros -> Uno -> USB -> MobiFlight -> FSUIPC -> Prepar3D.
Flujo de fuerzas: Prepar3D -> SimConnect -> puente -> USB -> Pico -> PWM/LCD.

1. Descargar MobiFlight Connector desde su sitio oficial. Si se busca reproducir
   el problema de la otra PC, anotar también su versión de Connector y su perfil.
2. Para la primera configuración conectar solamente el Uno. Seleccionar esa
   placa en la gestión de módulos/controladores y cargar el firmware MobiFlight.
   Esto reemplaza cualquier sketch que tuviera el Uno.
3. Con USB desconectado, conectar cada potenciómetro: extremos a 5 V y GND del
   Uno, terminal central a su entrada analógica. Propuesta:

   | Entrada | Función |
   | --- | --- |
   | A0 | Alerones / giro del yugo |
   | A1 | Elevador / avance del yugo |
   | A2 | Acelerador |
   | A3 | Timón / pedales |

4. Reconectar. Crear dispositivos Analog Input/Potentiometer para esos pines y
   subir la configuración al Uno. Comprobar la lectura real de cada uno; el
   rango nominal es 0–1023, pero se deben calibrar extremos y centro medidos.
5. Crear una configuración de entrada por potenciómetro, seleccionando el Uno
   y su dispositivo. Para Prepar3D, usar acciones FSUIPC (Event ID o el offset
   adecuado), no copiar directamente los presets exclusivos de MSFS2020/2024.
   Empezar por acelerador de un avión predeterminado. La selección exacta de
   evento, parámetro y fórmula depende de la versión de MobiFlight y del avión.
6. Escalar la lectura al rango del evento elegido y comprobar inversión de
   sentido. Para yugo/pedales, calibrar también el centro y una pequeña zona
   muerta. No asumir que todos los eventos de acelerador usan el mismo rango.
7. Activar las entradas y Run. Probar primero sin PS5 y sin Pico; luego agregar
   la Pico y el puente siguiendo la prueba anterior. Evitar asignaciones
   duplicadas del mismo eje en otros dispositivos.

### Preparación específica de esta PC

Se encontró FSUIPC4.DLL versión 4.977 en
`C:\Program Files\Lockheed Martin\Prepar3D v4\Modules`.
FSUIPC4 no corresponde a Prepar3D v4. No se borró ni modificó esa instalación.
Instalar FSUIPC6 para P3D v4 desde la distribución oficial y verificar que
aparezca en el menú Add-ons del simulador con un vuelo cargado. FSUIPC5 también
existió para v4, pero la distribución oficial recomienda FSUIPC6.

El instalador oficial de MobiFlight quedó en
`C:\Users\norie\Downloads\mobiflight-setup-1.5.0.exe`.
1.5.0 es la versión del instalador, no la de Connector (la web anuncia 11.3).
Ejecutarlo, seguir el asistente y conectar solo el Uno para su primera carga.
Después de instalar FSUIPC6 y abrir P3D, comprobar la conexión en MobiFlight.
El tutorial web de acelerador usa MSFS2020/2024: para este banco seleccionar
FSUIPC al definir la acción de entrada de Prepar3D.

Pendiente para el perfil definitivo: avión utilizado, calibración real de los
potenciómetros y comprobación física. No se instaló software ni se cargó ninguna
placa durante esta revisión. El firmware Pico se recompiló correctamente:
88.564 bytes de programa y 17.652 bytes de RAM global; el ELF no enlaza clases
de joystick ni Adafruit_USBD_HID y sí incluye USB CDC. Falta comprobar la
enumeración USB real y la coexistencia con los Mega en la PC de destino.

## Referencias oficiales

- Uno R3: https://docs.mobiflight.com/boards/supported/arduino-uno/
- Placas compatibles: https://docs.mobiflight.com/boards/
- Cableado: https://docs.mobiflight.com/devices/potentiometer/wiring/
- Entradas: https://docs.mobiflight.com/devices/potentiometer/configuring-device/
- Eventos FSUIPC: https://docs.mobiflight.com/features/input-action-types/fsuipc-event-id/
- Exclusión de joysticks: https://docs.mobiflight.com/game-controllers/disabling/
- Descarga Connector: https://www.mobiflight.com/
- Versiones FSUIPC: https://fsuipc.com/downloadspage/
