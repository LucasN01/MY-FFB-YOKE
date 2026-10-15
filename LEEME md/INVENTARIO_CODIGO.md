# Inventario de archivos de código

Relevamiento del 2026-10-15 (rama `cambio/posicion-simconnect`, versión V2.3). Indica para qué se usa cada archivo de código y si quedó de versiones anteriores.
Método: cadena de `#include` desde `MiYoke_PD.ino`, `SimConnectBridge/main.cpp` y `tests/`, archivos que compila cada `.vcxproj` y llamadas entre scripts.

Estados:
- **USO**: forma parte del producto o de su operación.
- **PRUEBA**: solo lo usan las pruebas automáticas.
- **COMPILAR**: compila o verifica, pero no se ejecuta en el yugo.
- **FLUJO**: herramienta del trabajo entre Claude y Codex; no forma parte del producto.
- **TESIS**: herramienta para generar o respaldar documentos de la tesis.
- **SIN USO**: nada lo incluye ni lo llama; quedó de etapas anteriores.

## 1. Firmware de la Pico 2 (`MiYoke_PD.ino` y cabeceras de la raíz)

| Archivo | Estado | Para qué |
|---|---|---|
| `MiYoke_PD.ino` | USO | Programa principal de la Pico 2. Lazo de control de 2 ms: lee la posición, recibe la telemetría SC8, calcula los efectos y comanda los motores. También maneja el menú, el LCD, el registro y la supervisión. |
| `mis_defines.h` | USO | Configuración única del hardware: pines, signos, límites, fuente de posición (`YOKE_USE_SIMULATION`: 0 encoders, 1 SimConnect, 2 potenciómetros) y activación de efectos. |
| `SimEffects.h` | USO | Núcleo del cálculo de fuerzas: centrado, damper, fricción, turbulencia, buffets y suelo. Combina efectos y ganancias en el comando PWM por eje. |
| `SimTelemetry.h` | USO | Protocolo serie con el puente (tramas, CRC y receptor). Documenta las versiones v1 a v7 del protocolo y el SC8 actual. |
| `RawTelemetry.h` | USO | Formato de la trama SC8: 22 variables de SimConnect en unidades originales (float32). También lo usa el puente. |
| `PicoForceModels.h` | USO | Corre en la Pico con cada muestra SC8 nueva. Calcula presión, ráfagas, buffets y suelo con los modelos de abajo. |
| `GustModel.h` | USO | Modelo de turbulencia/ráfagas basado en `TurbulenceModulator.py` de TelemFFB. |
| `BuffetModel.h` | USO | Envolventes de buffet de pérdida, tren y spoiler. Las vibraciones las genera la Pico al ritmo del lazo. |
| `GroundModel.h` | USO | Envolvente de vibración de rodaje en suelo según la velocidad respecto del suelo. |
| `NarrowbandNoise.h` | USO | Ruido de banda angosta (resonador de 2.º orden) para vibraciones aleatorias de buffet y textura de suelo. |
| `FrictionModel.h` | USO | Fricción háptica tipo Dahl (Hayward y Armstrong). Con posición SimConnect se usa la ley por velocidad, no este modelo. |
| `YokeMotion.h` | USO | Posición normalizada y velocidad del yugo, para todas las fuentes de posición. |
| `GainSettings.h` | USO | Tabla de ganancias de cada efecto (nombres para el LCD, pasos y valores iniciales). |
| `MotorDriver.h` | USO | Convierte el PWM con signo (-255..255) en las dos salidas del puente H de cada motor. |
| `CalibrationControls.h` | USO | Pulsadores y LEDs del modo calibración, con lectura de pines y antirrebote. |
| `CalibrationMenu.h` | USO | Menú de calibración en el LCD: ganancias, efectos, potenciómetros y registro. Guarda en la EEPROM emulada. |
| `PotCalibration.h` | USO | Calibración de los potenciómetros (extremos y centro) y su guardado. Se compila siempre porque la usa el menú. |
| `RecordSetting.h` | USO | Opción del menú "registro de datos" (activado o desactivado) y su guardado. |
| `LcdTelemetry.h` | USO | Controlador del LCD 16x2 por I2C (PCF8574), sin librerías externas. |
| `PicoLog.h` | USO | Formato de las líneas de registro que la Pico envía al puente para los ensayos. |
| `LoopSupervisor.h` | USO | Supervisa la etapa del lazo y las demoras, y registra la causa de reinicios y fallas (diagnóstico de HardFault). |
| `UsbGuard.h` | USO | Bloqueo del mutex USB de TinyUSB para que el lazo no choque con la tarea USB. |
| `AxisEncoders.h` | USO (condicional) | Lectura de encoders de roll y pitch. Se compila solo con `YOKE_USE_SIMULATION 0` (hoy está en 1). |
| `PicoEncoder.h` | USO (condicional) | Cuadratura x4 por interrupción para la Pico. Lo usa `AxisEncoders.h`. |
| `AxisPotentiometers.h` | USO (condicional) | Lectura de los potenciómetros por ADC0/ADC1. Se compila solo con `YOKE_USE_SIMULATION 2`. |
| `PotentiometerPosition.h` | USO | Convierte la lectura ADC en posición y valida la calibración en tiempo de compilación. |
| `AxisMotion.h` | SIN USO | Medición de posición de la "Etapa 2" (versión vieja). Ningún archivo lo incluye; lo reemplazó `YokeMotion.h`. Se puede eliminar. |
| `filters.h` / `filters.cpp` | SIN USO | Filtro pasa bajos (τ = 18 ms, `VELOCITY_FILTER_TAU_S`) que usaba `AxisMotion.h`. La velocidad del damper hoy la filtra `YokeMotion.h` con la misma fórmula de primer orden, pero con su propio código y τ = 40 ms (`YOKE_MOTION_FILTER_TAU_S`). Se puede eliminar junto con `AxisMotion.h` y `VELOCITY_FILTER_TAU_S`. |

## 2. Puente SimConnect para Windows (`SimConnectBridge/`)

| Archivo | Estado | Para qué |
|---|---|---|
| `main.cpp` | USO | Programa `YokeSimConnectBridge.exe`: lee 22 variables de Prepar3D por SimConnect, las envía a la Pico (SC8) y guarda la telemetría CSV de los ensayos. |
| `YokeSimConnectBridge.vcxproj` | USO | Proyecto de Visual Studio del puente. Lo compila `Build-Bridge.ps1`. |
| `SerialLink.h` | USO | Manejo del puerto COM: tiempos de espera, reconexión y rearmado. |
| `PicoCsv.h` | USO | Guarda en CSV las líneas de registro que envía la Pico. |
| `Replay.h` | USO | Formato de las filas de telemetría CSV, compartido con el comparador de ensayos. |
| `WaitForFlight.cpp` | USO | Programa `WaitForFlight.exe`: el lanzador lo usa para esperar a que el vuelo esté listo antes de abrir el puente. Solo lee, no abre el COM. |
| `WaitForFlight.vcxproj` | USO | Proyecto de `WaitForFlight.exe`. Lo compila `Build-WaitForFlight.ps1`. |
| `GustModel.h`, `BuffetModel.h`, `GroundModel.h` | PRUEBA | Una línea cada uno que reenvía al modelo de la raíz. Quedaron para que las pruebas viejas sigan compilando; el puente no los usa. |
| `GustGain.h` | PRUEBA | Escala de ganancia de ráfagas que usaba el puente antes de SC8. Hoy solo lo usan las pruebas. |

## 3. Pruebas automáticas (`tests/`)

| Archivo | Estado | Para qué |
|---|---|---|
| `simconnect_checks.cpp` + `.vcxproj` | PRUEBA | Suite principal (más de 140 000 comprobaciones). Incluye los `*_checks.h`. La ejecuta `probar.ps1`. |
| `legacy_effects_checks.cpp` | PRUEBA | Verifica que, con los indicadores de realismo apagados, los efectos den lo mismo que la fórmula anterior. |
| `sim_position_friction_checks.cpp` | PRUEBA | Verifica que con posición SimConnect se use la fricción por velocidad. |
| `calibration_checks.h`, `ground_checks.h`, `health_checks.h`, `link_checks.h`, `picolog_checks.h`, `raw_checks.h`, `realism_checks.h` | PRUEBA | Comprobaciones por tema: menú y guardado, suelo, supervisor, enlace serie, registro, trama SC8 y ruido/realismo. |
| `comparar_ensayo.h` / `.cpp` / `.vcxproj` | USO | Programa `comparar_ensayo.exe`: al terminar un ensayo, compara la telemetría con el registro de la Pico. Lo usan `ENSAYO.ps1` y `BANCO.ps1`; lo compila `probar.ps1`. |
| `launcher_port_checks.ps1` | PRUEBA | Prueba la detección del puerto COM del lanzador con puertos simulados, sin tocar puertos reales. |

## 4. Scripts de uso del yugo (raíz)

| Archivo | Estado | Para qué |
|---|---|---|
| `INICIAR SIMULADOR.cmd` | USO | Acceso de doble clic que arranca `Iniciar-Simulador.ps1` (Prepar3D v4). |
| `INICIAR SIMULADOR V3.cmd` | USO | Igual que el anterior, pero para Prepar3D v3.4. Usarlo solo si se vuela en V3. |
| `Iniciar-Simulador.ps1` | USO | Lanzador: detecta la Pico, configura MobiFlight, abre Prepar3D, espera el vuelo con `WaitForFlight.exe` y abre el puente. |
| `PROBAR REALISMO.cmd` | SIN USO (redundante) | Llama al lanzador con `-Realismo`, que hoy ya es el valor por defecto; su título dice "SC6". Hace lo mismo que `INICIAR SIMULADOR.cmd`. Se puede eliminar. |
| `ENSAYO.cmd` / `ENSAYO.ps1` | USO | Guía un ensayo en vuelo por fases, guarda los datos en `ensayos/` y ejecuta el comparador. |
| `CICLOS.cmd` / `CICLOS.ps1` | USO (ensayo puntual) | Ensayo de ciclos lentos (pausa, menú, flash y Pico sola) del diagnóstico de reinicios. Útil si se repite ese ensayo. |
| `ARRANQUE.cmd` / `ARRANQUE.ps1` | USO (diagnóstico) | Repite un vuelo corto grabado contra la Pico para reproducir fallas de arranque, y decodifica el HardFault con `FALLA.ps1`. |
| `FALLA.ps1` | USO (diagnóstico) | Traduce las direcciones de un HardFault (PC/LR) a archivo y línea con `addr2line` y el `.elf`. |
| `BANCO.ps1` | USO (diagnóstico) | Banco de prueba sin simulador: reproduce telemetría grabada hacia la Pico, con resguardo de fuente de motores apagada. |

## 5. Compilación y verificación

| Archivo | Estado | Para qué |
|---|---|---|
| `probar.ps1` | COMPILAR | Comando único de verificación: suite de pruebas, comparador y, con opciones, firmware y puente. Salida de una línea por paso. |
| `Build-Pico2.ps1` | COMPILAR | Compila el firmware de la Pico 2 con `arduino-cli`, con variantes de encoders, potenciómetros o prueba de falla. |
| `Build-Bridge.ps1` | COMPILAR | Compila `YokeSimConnectBridge.exe` con MSBuild. |
| `Build-WaitForFlight.ps1` | COMPILAR | Compila `WaitForFlight.exe`. `probar.ps1` no lo incluye; se ejecuta a mano si cambia `WaitForFlight.cpp`. |

## 6. Herramientas del flujo Claude–Codex (no generan versión)

| Archivo | Estado | Para qué |
|---|---|---|
| `lanzar-codex.py` | FLUJO | Lanza a Codex como chat visible en la app de Codex, con el título de la tarea. |
| `nueva-version.ps1` | FLUJO | Crea la versión (etiqueta y copia en `MiYoke_PD_VERSIONES`) cuando cambió código del proyecto. |
| `compilar-tesis.ps1` | FLUJO | Compila y verifica la tesis LaTeX. Con `-CopiarPdf` deja `main.pdf` junto a `main.tex`. |
| `graphify-sync.cmd` | FLUJO | Copia el `.ino` a `graphify-ino/` y actualiza el grafo de código que usan los agentes. |
| `.claude/settings.json`, `.codex/hooks.json` | FLUJO | Configuración de Claude y Codex (permisos y hook de graphify). |

## 7. Scripts de la tesis (`tesis/`, no generan versión)

| Archivo | Estado | Para qué |
|---|---|---|
| `fundamento_efectos/generar_documento.py` | TESIS (uso único) | Generó el documento de fundamento de efectos y `fuentes_sha256.json`. Lee `MiYoke_PD_V2.3.ino`, que ya no existe: no se puede volver a ejecutar sin adaptarlo. |
| `fundamento_efectos/agregar_explicaciones.py` | TESIS (uso único) | Parche que insertó explicaciones dentro de `generar_documento.py`. Ya se aplicó; no hace falta ejecutarlo de nuevo. |
| `auditoria_c182rg/auditar_modelo.py` | TESIS (respaldo) | Audita el Cessna 182RG instalado, TelemFFB y la telemetría guardada; genera `evidencia.json`. Conservarlo para reproducir la evidencia citada. |
| `ensayos_centrado_damper_friccion/analizar.py` | TESIS (respaldo) | Analiza los CSV de los ensayos de centrado, damper y fricción, y genera `resultados.json`. Conservarlo como respaldo de esos resultados. |

## 8. Carpetas sin seguimiento en git (no forman parte del proyecto)

| Carpeta | Estado | Para qué |
|---|---|---|
| `VPforce-TelemFFB-refactor/` | REFERENCIA | Código fuente de TelemFFB (Python, unos 300 archivos). Fuente de los modelos citados en diseños y tesis, y la leen `auditar_modelo.py` y `generar_documento.py`. No se compila ni se ejecuta. Conservarla mientras se cite. |
| `backups/antes-sc8/`, `backups/suelo-antes-sc7/` | SIN USO | Copias manuales de archivos de antes de SC7/SC8 (`MiYoke_PD_V2.4.ino`, `main.cpp`, `SimEffects.h`, etc.). Lo mismo está en git y en `MiYoke_PD_VERSIONES`. Se pueden eliminar. |
| `graphify-ino/` | FLUJO | Copia automática del `.ino` para el grafo. La regenera `graphify-sync.cmd`. |

## Resumen: se pueden eliminar

- `AxisMotion.h`, `filters.h` y `filters.cpp`: código sin uso de la etapa 2.
- `PROBAR REALISMO.cmd`: duplica `INICIAR SIMULADOR.cmd`.
- `backups/`: copias que ya están en git y en las versiones.
- Opcional: `SimConnectBridge/GustModel.h`, `BuffetModel.h` y `GroundModel.h`, si las pruebas pasan a incluir los modelos de la raíz. `fundamento_efectos/agregar_explicaciones.py`, porque ya se aplicó.

Antes de borrar archivos de código, hay que actualizar la documentación que los nombra. `AxisMotion.h` y `filters` aparecen en `LEEME.md`, `LEEME_ETAPAS.md`, `LEEME_PICO2.md`, `LEEME_SIMCONNECT.md`, `LEEME_AMORTIGUACION.md` y `VALIDACION_SIMCONNECT_V3.md`. Además, eliminarlos genera una versión nueva.
