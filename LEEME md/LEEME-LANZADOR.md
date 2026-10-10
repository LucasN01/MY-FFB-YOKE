# Inicio con doble clic

## Deteccion automatica de la Pico 2 (version actual, 08/10/2026)

`Iniciar-Simulador.ps1` ya no tiene un COM fijo. En cada inicio:

1. Enumera los puertos USB presentes y selecciona solo `VID_2E8A&PID_000F`,
   correspondiente a la Pico 2 ARM/TinyUSB de este proyecto. No abre puertos
   Arduino, CH340 ni otros identificadores para buscarlos por prueba y error.
2. Consulta las candidatas a 115200 baudios con `?` y exige la respuesta
   `YOKE17SC8` para el puente actual (`YOKE17SC5` en el modo historico sin Realismo).
   El identificador USB solo no basta para confirmar que sea el yoke.
3. Usa el COM detectado tanto para el puente como para la exclusion de MobiFlight.
   No cambia el numero COM asignado por Windows. COM12, COM50 o COM103 son validos.
4. Si no encuentra la Pico, el puerto esta ocupado, el firmware no coincide o
   hay dos yokes compatibles, muestra el motivo. No elige una placa arbitraria.
5. Antes de iniciar el puente, vuelve a verificar que la conexion no haya cambiado
   durante la carga del simulador. Si cambio, pide reiniciar el lanzador.

Segui usando **INICIAR SIMULADOR.cmd** o **INICIAR SIMULADOR V3.cmd**.
La version SC8 requiere cargar el nuevo firmware de `build-pico2` y usar el puente SC8.
La deteccion del COM sigue siendo automatica. Ver [calculo en Pico](LEEME%20md/CALCULO_PICO_SC8.md).
El ejecutable actual esta en `SimConnectBridge/bin/Win32/RealismoSC8`.

### Probar solo la deteccion

Desde PowerShell en la carpeta del proyecto:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Iniciar-Simulador.ps1 -DetectarPlaca
```

Esta opcion no exige que existan las rutas de Prepar3D o MobiFlight. Abre brevemente
solo puertos candidatos para identificarlos, los cierra y no envia tramas de fuerza,
no modifica configuraciones y no abre programas. Si el puente ya esta funcionando,
no consulta la placa para no interferir con su puerto.

`-Comprobar` verifica tambien rutas/configuraciones e identifica la placa, sin
modificarlas. `-SoloPreparar` prepara las configuraciones sin iniciar programas.

### MobiFlight y reconexion

Para el primer inicio tras cambiar de puerto, cerrar MobiFlight y el monitor serie.
El lanzador agregara el COM actual a la lista de exclusiones antes de abrir MobiFlight.
Si MobiFlight ya esta abierto y no tiene ese COM excluido en su configuracion,
se solicita cerrarlo y volver a ejecutar; no se cierra automaticamente.
Se conservan las exclusiones previas del usuario: si un COM antiguo del yoke pasa
ahora a un Arduino, revisar esa entrada antigua en la lista de puertos ignorados
 de MobiFlight y retirarla si corresponde.

La deteccion se realiza en cada inicio; no se agrego reconexion automatica durante
una sesion activa del puente. Si se desenchufa la Pico durante el uso, terminar
la sesion del puente y volver a iniciar con la placa conectada.
En BOOTSEL la placa no expone el puerto serie del firmware: terminar la carga y
conectarla normalmente. Un firmware con otro VID/PID requerira actualizar el filtro
USB; este filtro coincide con la placa y compilacion comprobadas.

### Otra PC y validacion

El COM no requiere configuracion manual. Las rutas de programas, del proyecto
MobiFlight y del escenario siguen dependiendo de cada PC; por ejemplo,
`$MobiProyecto` conserva la ruta local existente y debe ajustarse si no existe alli.
Copiar el proyecto completo, no solamente el archivo CMD.

Prueba real de identificacion con Windows PowerShell 5.1: Pico 2 encontrada en
COM12, respuesta YOKE17SC7 (verificacion anterior a la migracion SC8; la placa no se reflasheo en esta modificacion). Pruebas simuladas: 19 comprobaciones correctas,
incluyendo otros fabricantes, cambios de COM, puerto ocupado, firmware incompatible,
ambiguedad y exclusiones MobiFlight. No se inicio el simulador ni se enviaron fuerzas.

Copia del lanzador anterior: `Iniciar-Simulador.ps1.antes-autocom.bak`.

---

Los apartados siguientes son antecedentes de instalacion y pruebas anteriores.
Sus referencias a COM13 y SC6 son historicas; para el puerto y protocolo actuales
rige la seccion anterior.


## Prepar3D V3.4 y V4 en paralelo (06/10/2026)

### Correccion posterior de rutas y validacion

La instalacion activa de V3 ahora esta en **Prepar3D-v3 dentro de este proyecto**: `Modules` contiene FSUIPC4 y `SimObjects\Airplanes` contiene el Cessna. Los registros `dll.xml` y `simobjects.cfg` de Roaming apuntan a estas rutas. Las copias anteriores en AppData\Local\MiYoke ya no se utilizan. No mover el proyecto sin actualizar ambos registros.

El lanzador lee la ruta FSUIPC desde dll.xml y registra tambien los errores iniciales en logs. WaitForFlight fue recompilado para leer TITLE y rechazar explicitamente Viewer; la antigua comprobacion de tierra/pausa podia aceptar el modo de visualizacion tras fallar el avion. Los errores SimConnect ahora incluyen solicitud e indice y esperan cinco segundos antes de reconectar.

La prueba posterior a esta correccion confirmo por SimConnect **Vehiculo: Cessna Skylane FSX_RG**, tierra y simulacion activa. El puente SC6 tambien obtuvo telemetria de V3 y abrio COM13 en modo monitor. La validacion anterior no garantizaba que el vehiculo no fuera Viewer.

- **INICIAR SIMULADOR V3.cmd** y el acceso directo del mismo nombre en el escritorio inician V3.4.
- **INICIAR SIMULADOR.cmd** conserva V4 como predeterminado.
- Tambien se puede ejecutar `Iniciar-Simulador.ps1 -Version 3` o `-Version 4`.
- Cerrar una version antes de abrir la otra. El lanzador rechaza otra version abierta para evitar conectar SimConnect y MobiFlight al simulador equivocado.

Ambas versiones usan el mismo puente Win32 RealismoSC6, detector WaitForFlight y proyecto `prepar lucas.mfproj`. No se cambio el firmware.

Para V3 se instalo el Cessna de Downloads en `C:\Users\norie\AppData\Local\MiYoke\Prepar3D v3\SimObjects\Airplanes\Cessna182RG_fsx`, registrado mediante `simobjects.cfg` en AppData\Roaming\Lockheed Martin\Prepar3D v3. Nombre del avion: **Cessna Skylane FSX_RG**. Se conservaron modelo, instrumentos y dinamica del paquete. El sonido apunta al Mooney_Bravo instalado porque falta el C172 al que apuntaba el original; los fallbacks de textura se adaptaron a la ubicacion externa. Los archivos originales modificados tienen copia `.original.bak`.

FSUIPC4 4.977 y SimConnectP3D3.dll se copiaron de los archivos ya disponibles en esta PC a `C:\Users\norie\AppData\Local\MiYoke\Prepar3D v3\Modules`, y se registraron en el `dll.xml` de Roaming para V3. El modulo se cargo y conecto correctamente. No se modificaron los archivos de V4.

Se preparo **CESSNA 182 APAGADO V3.fxml** y su archivo meteorologico en Documentos\Prepar3D v3 Files a partir del vuelo de V4, adaptando sus metadatos. V3 lo tiene como predeterminado y omite la pantalla inicial. El respaldo previo es `Prepar3D.cfg.antes-miyoke-v3.bak`.

Validacion real en V3.4.22.19868: avion cargado, FSUIPC conectado, WaitForFlight termino con codigo 0 (en tierra, activo y sin pausa). MobiFlight 11.3.0 registro `Connected to Prepar3D. [FSUIPC]` y detecto `Cessna Skylane FSX_RG`. Tambien se comprobo el rechazo de V4 mientras V3 estaba abierto. **No habia puertos serie ni modulos MobiFlight conectados: queda pendiente probar entradas fisicas y fuerzas SC6 con COM13 y la placa MobiFlight conectadas.** El ejecutable que aplica fuerzas no se ejecuto.

Para retirar esta instalacion de V3: cerrar los programas, restaurar el respaldo de Prepar3D.cfg, retirar los archivos `simobjects.cfg` y `dll.xml` creados en Roaming para V3 (si posteriormente se agregaron otras entradas, retirar solo las de MiYoke/FSUIPC), y borrar la carpeta de usuario MiYoke\Prepar3D v3 y el vuelo V3. V4 no depende de estos archivos.

Referencias: [configuracion de complementos en V3](https://www.prepar3d.com/SDKv3/LearningCenter/add-ons/add-on_configuration_files.html), [FSUIPC4 para P3D 1.4–3.4](https://fsuipc.com/downloadspage/).

## Inicio original de V4

Ejecutar **INICIAR SIMULADOR.cmd**. No mover ese archivo solo: necesita los otros archivos del proyecto.

El lanzador:

1. Comprueba los ejecutables, el proyecto y la presencia de COM13.
2. Configura MobiFlight para ignorar COM13, reservado para el puente, y activa AutoRun.
3. Abre MobiFlight con `prepar lucas.mfproj` y `/autoRun`.
4. Abre Prepar3D v4 con su escenario predeterminado, omitiendo la pantalla de seleccion.
5. Espera datos reales de SimConnect: simulacion activa, avion en tierra, sin pausa y fuera de Slew durante dos segundos. No usa una demora fija para adivinar cuando termino de cargar.
6. Desde `SimConnectBridge\bin\Win32\RealismoSC6`, ejecuta exactamente:

```powershell
.\YokeSimConnectBridge.exe --port COM13 --enable --gusts --gust-gain 1
```

Dejar abierta la consola mientras se usa el simulador. Al cerrar Prepar3D, el puente termina por la desconexion de SimConnect. MobiFlight queda abierto. No se reinician automaticamente procesos que fallen.

## Escenario y programas ya abiertos

Actualmente el predeterminado es **CESSNA 182 APAGADO.fxml**. Se conserva el estado guardado del avion, incluidos los motores apagados. Para comenzar en otra pista, guardar ese vuelo como predeterminado en Prepar3D; el lanzador lo leera en el siguiente inicio. El detector confirma que el avion esta en tierra, no distingue una pista de una plataforma.

Si Prepar3D ya esta abierto, espera su vuelo actual. Si MobiFlight ya esta abierto, conserva su proyecto y estado Run: debe estar ejecutando el proyecto correcto. Para el inicio totalmente automatico desde cero, cerrar MobiFlight antes del doble clic. Si el puente o este lanzador ya estan abiertos, no crea duplicados.

Si queda esperando, comprobar que el vuelo no este pausado (P) ni en modo Slew. El limite de espera es 30 minutos. Los errores quedan visibles y se guardan en `logs`; los del puente estan en `SimConnectBridge\bin\Win32\RealismoSC6\bridge.log`.

Para calibrar: esperar primero a que arranque el puente; luego pausar Prepar3D, esperar un segundo y pulsar calibracion. La placa necesita recibir la confirmacion de pausa por el puente. El lanzador usa SC6 por defecto, igual que la placa comprobada en COM13. No usar el ejecutable SC5 de Release con firmware SC6. No hace falta recargar firmware para esta correccion del lanzador.

## Configuracion y respaldo

Las rutas, puerto y tiempo de espera se editan al comienzo de `Iniciar-Simulador.ps1`. `$Escenario = ''` utiliza el vuelo predeterminado; puede reemplazarse por una ruta completa a un `.fxml`.

Antes de modificar `Prepar3D.cfg` o `user.config` de MobiFlight se crea una copia junto al original con extension `.antes-del-lanzador.bak`. Los cambios persistentes son `SHOW_SCENARIO_WINDOW=0`, AutoRun y la exclusion de COM13. Para deshacerlos, cerrar los programas y restaurar esas copias. El proyecto `.mfproj` y el firmware no se modifican.

Comprobacion sin abrir programas ni modificar configuraciones:

```powershell
.\Iniciar-Simulador.ps1 -Comprobar
```

`-SoloPreparar` valida y prepara las configuraciones sin iniciar los programas. El detector ya esta compilado; `Build-WaitForFlight.ps1` permite recompilarlo con las mismas herramientas de Visual Studio y SDK que usa el puente.

Validado: compilacion del detector, ejecucion sin simulador (no informa listo), salida con PID inexistente y comprobacion de rutas/COM13 con Windows PowerShell 5.1. Falta la prueba completa con el vuelo cargado, MobiFlight en Run y el hardware del yoke activo.

Referencias: [parametros oficiales de MobiFlight](https://docs.mobiflight.com/features/command-line-parameters/) y [parametros de inicio de Prepar3D](https://prepar3d.net/SDKv4/sdk/sdk_overview.html).
