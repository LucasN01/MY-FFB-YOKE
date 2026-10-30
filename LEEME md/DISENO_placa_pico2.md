# DISEÑO: placa base de la Raspberry Pi Pico 2 (KiCad 10)

Rama: `hardware/placa-pico2` (desde master `d71d0c3`, con AS5600 integrado).
Estado: diseño con decisiones del usuario (2026-10-30), revisado por Master Plan Architect. Los puntos marcados **[A CONFIRMAR]** (medidas de módulos y alimentación del AS5600) se completan antes de lanzar a Codex.

## 1. Objetivo

Placa de dos capas donde se enchufa la Pico 2 y que reúne, con conectores robustos, todo lo que hoy va por protoboard:
drivers BTS7960 (módulos IBT-2), encoders magnéticos AS5600, sensado de corriente ACS712 + ADS1115,
4 finales de carrera ópticos, LCD I2C y panel de 5 pulsadores + 2 LEDs.

Reparto: Codex genera el proyecto KiCad 10 (esquemático completo, símbolos y huellas asignados, ERC limpio).
El usuario hace "Actualizar PCB desde esquemático", ubica y rutea siguiendo la sección 8.

## 2. Decisiones del usuario (no modificar sin preguntar)

| Tema | Decisión |
|---|---|
| Sensor de posición | Solo AS5600 (modo 3). Se eliminan conectores de potes y de cuadratura. |
| Corriente | ACS712 (30 A, 66 mV/A) por conector, leídos por módulo ADS1115 a 5 V. |
| Alimentación | Pico por USB. Fuente de protoboard MB102 enchufada en la placa (jack 12 V propio) alimenta el riel de 5 V de sensores, LCD, lógica de drivers y lado alto de traductores. Los 24 V de motor no pasan por la placa. |
| Tensión de sensores | Todos los periféricos a 5 V; se traduce a 3,3 V hacia la Pico. |
| 3,3 V lado bajo | `3V3 OUT` de la Pico (pin 36) alimenta VA del TXS0108E y LV de los BSS138: con la Pico apagada no hay inyección por los pines. |
| Traductores | TXS0108E: 6 señales de drivers (2 canales libres). BSS138 4 canales #1: I2C0 + I2C1. BSS138 4 canales #2: 4 finales de carrera. Sin I2C por el TXS0108E (TI SCEA044). |
| Buses I2C | I2C0 (GP4/GP5, 100 kHz): LCD PCF8574 + AS5600 pitch. I2C1 (GP14/GP15, 400 kHz): AS5600 roll + ADS1115 (0x36 y 0x48, sin conflicto). |
| Finales de carrera | Ópticos de 3 pines (VCC, GND, señal) a 5 V, 4 entradas a GP0–GP3. Firmware en tarea aparte. |
| Panel | LCD, pulsadores y LEDs en panel externo por conectores. |
| Conectores | JST-XH (paso 2,50 mm) para cables; módulos en zócalos hembra 2,54 mm; Pico en zócalo hembra. |
| Seguridad drivers | Pull-down 47 kΩ en cada señal del lado 5 V. OE del TXS0108E a GP21 con pull-down 10 kΩ (arranca deshabilitado). Divisor +5V → 10 kΩ / 15 kΩ → GP28 para detectar la MB102. El firmware (tarea aparte, obligatoria antes de conectar motores) pone GP6–GP11 en bajo y habilita OE solo con 5 V presentes; si caen, deshabilita OE. Orden de encendido: MB102 primero, USB después. |
| Pull-ups I2C1 externos | Huella sin montar (DNP); el valor se calcula con los pull-ups reales medidos de los módulos (§3). |
| Entrada MB102 | Adaptador de 9 V por el jack; no usar el USB de la MB102. |
| Firmware requerido | La placa exige `YOKE_POSITION_SOURCE 3` (AS5600). GP0–GP3 dejan de ser encoders de cuadratura; GP21 (OE) y GP28 (detección 5 V) son nuevos. Todo en la tarea de firmware aparte. |
| Fabricación | Fabricante, 2 capas, plano de masa, THT. |
| Cables | Drivers ≤ 30 cm. AS5600: 30 cm a 1 m. |
| Carpeta | `hardware/placa-pico2/`. |
| Versionado | Serie aparte `PCB-Vx.y`, copia solo `hardware/placa-pico2/` (sección 10). |

## 3. Fundamento de las elecciones técnicas

- **TXS0108E, pull-ups internos (TI, hoja de datos TXS0108E, §7.3.5):** cada I/O tiene pull-up a su VCC de 4 kΩ cuando pasa un "1" y 40 kΩ cuando pasa un "0"; se desactivan con OE bajo. Con pull-down externo R, el "1" en B vale 5·R/(R+4k): 10 kΩ → 3,57 V (límite del 74HC244 del IBT-2, VIH ≈ 0,7·VCC = 3,5 V a 5 V); **47 kΩ → 4,61 V** (margen). En alta impedancia (OE bajo o Pico apagada) los 47 kΩ fijan las entradas en "0" → motores apagados.
- **TXS0108E, OE y secuencia (hoja de datos, §1, §7.3.4 y "Power Supply Recommendations"):** "ensure that VCCA ≤ VCCB at all times"; para Hi-Z en encendido/apagado, OE a GND por pull-down y habilitarlo recién con las dos fuentes estables. Con OE fijo a VCCA, en reset/BOOTSEL los pines A flotan y el pull-up interno de 40 kΩ puede pasar un "1" a los drivers (hallazgo de la revisión). Por eso OE = GP21 con 10 kΩ a GND y detección de 5 V en GP28 (divisor 10k/15k: 5 V → 3,0 V; ADC2).
- **BSS138 con la MB102 apagada:** el diodo de cuerpo pasa corriente de LV (3V3) a HV y alimenta en parte el riel de 5 V (~0,1 mA por canal). No daña, pero deja periféricos en estado indefinido: se documenta el orden de encendido y el firmware ignora sensores sin 5 V.
- **TXS0108E, carga (hoja de datos, §7.3.3):** pistas y conectores suman capacidad; one-shot de ~30 ns. Por eso drivers con cable ≤ 30 cm y el TXS0108E cerca de los conectores de drivers. Hipótesis a validar con osciloscopio (flanco y nivel en RPWM/LPWM del IBT-2 a 490 Hz y a la frecuencia PWM final).
- **I2C sin TXS (TI SCEA044):** los aceleradores de flanco del TXS se llevan mal con pull-ups externos y cargas de bus; el BSS138 (traductor de NXP AN10441) es el esquema estándar para I2C.
- **I2C1 con cable de 30 cm a 1 m:** el tiempo de subida máximo a 400 kHz es 300 ns (NXP UM10204, tabla 10) con t_r ≈ 0,847·R·C. Con ~100 pF/m de cable + entradas, 10 kΩ solo da ~850 ns (no cumple). Pero el bus ya suma los pull-ups del BSS138 (10 kΩ), del ADS1115 (10 kΩ) y del AS5600 (típ. 4,7 kΩ): con 2,2 kΩ extra quedaría ≈ 1,15 kΩ → 4,3 mA > 3 mA (UM10204, VOL 0,4 V). Regla: R total del lado HV ≥ 5 V / 3 mA ≈ 1,67 kΩ. R_I2C1 queda DNP y se calcula con los valores medidos de los módulos; luego se mide el flanco con osciloscopio. Si no cumple 300 ns, I2C1 a 100 kHz. En I2C0 cuentan también los ~4,7 kΩ del PCF8574.
- **ADS1115 a 5 V:** el ACS712 da 2,5 V ± 66 mV/A (0–5 V); la entrada del ADS1115 no puede superar VDD + 0,3 V (TI ADS1115, "Absolute Maximum Ratings"), por eso VDD = 5 V.
- **AS5600 a 5 V:** el chip admite 5 V por VDD5V con VDD3V3 solo con 1 µF a GND (ams AS5600, "Power Supply"). Muchos módulos unen VDD3V3 con VDD5V y entonces solo admiten 3,3 V: **[A CONFIRMAR: continuidad entre patas 1 y 2 del chip del módulo del usuario]**. La salida OUT y DIR no se cablean a la placa: DIR se fija en el propio módulo.
- **Masa:** GND de la placa, del USB (PC) y del negativo de 24 V de los IBT-2 se unen en un solo punto (el negativo de la fuente de 24 V), para no formar lazos de masa con la corriente de motor.

## 4. Asignación de pines (Pico 2, número GPIO / pata física)

| Señal (red KiCad) | GPIO | Pata | Pasa por | Conector |
|---|---|---|---|---|
| FC_ROLL_NEG | GP0 | 1 | BSS138 #2 canal 1 | J_FC1 |
| FC_ROLL_POS | GP1 | 2 | BSS138 #2 canal 2 | J_FC2 |
| FC_PITCH_NEG | GP2 | 4 | BSS138 #2 canal 3 | J_FC3 |
| FC_PITCH_POS | GP3 | 5 | BSS138 #2 canal 4 | J_FC4 |
| I2C0_SDA | GP4 | 6 | BSS138 #1 canal 1 | J_LCD, J_AS_PITCH |
| I2C0_SCL | GP5 | 7 | BSS138 #1 canal 2 | J_LCD, J_AS_PITCH |
| ROLL_PWM_L (RPWM) | GP6 | 9 | TXS0108E A1→B1 | J_DRV_ROLL |
| ROLL_PWM_R (LPWM) | GP7 | 10 | TXS0108E A2→B2 | J_DRV_ROLL |
| PITCH_PWM_L | GP8 | 11 | TXS0108E A3→B3 | J_DRV_PITCH |
| PITCH_PWM_R | GP9 | 12 | TXS0108E A4→B4 | J_DRV_PITCH |
| ROLL_EN | GP10 | 14 | TXS0108E A5→B5 | J_DRV_ROLL |
| PITCH_EN | GP11 | 15 | TXS0108E A6→B6 | J_DRV_PITCH |
| LED_NORMAL | GP12 | 16 | R 330 Ω | J_PANEL |
| LED_CALIB | GP13 | 17 | R 330 Ω | J_PANEL |
| I2C1_SDA | GP14 | 19 | BSS138 #1 canal 3 | J_AS_ROLL, ADS1115 |
| I2C1_SCL | GP15 | 20 | BSS138 #1 canal 4 | J_AS_ROLL, ADS1115 |
| BTN_PLUS | GP16 | 21 | directo (pull-up interno) | J_PANEL |
| BTN_MINUS | GP17 | 22 | directo | J_PANEL |
| BTN_OK | GP18 | 24 | directo | J_PANEL |
| BTN_BACK | GP19 | 25 | directo | J_PANEL |
| BTN_MODE | GP20 | 26 | directo | J_PANEL |
| TXS_OE | GP21 | 27 | pull-down 10 kΩ a GND; a OE del TXS0108E | — |
| ADS_ALERT (reservado) | GP22 | 29 | pull-up 10 kΩ a 3V3 (ALERT es colector abierto) | ADS1115 |
| V5_SENSE | GP28 / ADC2 | 34 | divisor +5V → 10 kΩ → GP28 → 15 kΩ → GND, 100 nF a GND | — |
| Libres a tira de pines J_EXP | GP26, GP27 | 31, 32 | — | J_EXP |
| TXS A7/A8, B7/B8 libres | — | — | a J_EXP (A7, A8, B7, B8) | J_EXP |
| 3V3 OUT | — | 36 | VA TXS, LV BSS138 #1 y #2, pull-ups ALERT | — |
| GND | — | 3, 8, 13, 18, 23, 28, 33, 38 | todas a GND | — |
| VSYS, VBUS, 3V3_EN, RUN, ADC_VREF | — | 39, 40, 37, 30, 35 | sin conectar (VSYS/VBUS **nunca** al riel de 5 V) | — |

Los pines coinciden con `mis_defines.h` (rama RP2040) salvo GP0–GP3 (de encoders de cuadratura a finales de carrera), GP21 (OE) y GP28 (detección de 5 V): cambio de firmware en tarea aparte. El cableado de roll aún no existe: GP6 → RPWM queda como convención de la placa y el signo se ajusta con `ROLL_MOTOR_DIRECTION` en la puesta en marcha.

**Nombres de red:** etiquetas globales con sufijo de lado: `_3V3` entre la Pico y el traductor, `_5V` entre el traductor y el conector (ej.: `ROLL_RPWM_3V3`, `ROLL_RPWM_5V`, `I2C1_SDA_3V3`, `I2C1_SDA_5V`, `FC_ROLL_NEG_3V3`, `FC_ROLL_NEG_5V`). Las señales que no se traducen (botones, LEDs, OE, ALERT, V5_SENSE) llevan el nombre de la tabla sin sufijo. Así las redes no dependen de la hoja jerárquica.

## 5. Conectores

| Ref | Huella | Pin 1 → n |
|---|---|---|
| J_DRV_ROLL, J_DRV_PITCH | JST_XH_B6B-XH-A_1x06_P2.50mm_Vertical | RPWM, LPWM, EN (va a R_EN y L_EN del IBT-2), +5V, GND, GND |
| J_AS_ROLL, J_AS_PITCH | JST_XH_B4B-XH-A_1x04 | +5V, GND, SDA, SCL |
| J_ACS_ROLL, J_ACS_PITCH | JST_XH_B3B-XH-A_1x03 | +5V, GND, OUT |
| J_FC1..J_FC4 | JST_XH_B3B-XH-A_1x03 | +5V, GND, SEÑAL |
| J_LCD | JST_XH_B4B-XH-A_1x04 | GND, +5V, SDA, SCL (orden del módulo PCF8574) |
| J_PANEL | JST_XH_B8B-XH-A_1x08 | BTN_PLUS, BTN_MINUS, BTN_OK, BTN_BACK, BTN_MODE, LED_NORMAL (ánodo vía R), LED_CALIB (ánodo vía R), GND |
| J_EXP | PinHeader_1x10_P2.54mm | 3V3, GND, GP26, GP27, TXS_A7, TXS_A8, TXS_B7, TXS_B8, ADS_A2, ADS_A3 |
| U_PICO | Module:RaspberryPi_Pico_Common_THT (en zócalo hembra 2×20) | símbolo MCU_Module:RaspberryPi_Pico |
| U_TXS, U_BSS1, U_BSS2, U_ADS | PinSocket 2,54 mm con la distribución del módulo comercial **[A CONFIRMAR: medir módulos]** | — |
| U_MB102 | huella propia (2 tiras hembra en la posición de los rieles) **[A CONFIRMAR: medir]** | +5V, GND en ambos extremos |

Conexiones de módulos:
- **ADS1115:** VDD = +5V, GND, SDA/SCL = lado HV de I2C1, ADDR = GND (0x48), ALERT = ADS_ALERT, A0 = OUT de J_ACS_ROLL, A1 = OUT de J_ACS_PITCH, A2 y A3 a GND por 10 kΩ y a J_EXP (ADS_A2, ADS_A3).
- **TXS0108E:** VA = 3V3 OUT, VB = +5V, OE = TXS_OE (GP21, pull-down 10 kΩ), A1–A6 según tabla 4.
- **BSS138 #1 y #2:** LV = 3V3 OUT, HV = +5V, GND común.
- **MB102:** ambos puentes en 5 V; su salida de 3,3 V no se usa.

## 6. Pasivos

| Ref | Valor | Función |
|---|---|---|
| R_PD1..R_PD6 | 47 kΩ, THT 1/4 W | pull-down de las 6 señales lado 5 V (§3) |
| R_I2C1_SDA, R_I2C1_SCL | DNP (valor a calcular, §3) | huella de pull-up a +5V del lado HV de I2C1; campo "DNP" activo en el símbolo |
| R_OE | 10 kΩ | pull-down de TXS_OE |
| R_V5A, R_V5B, C_V5 | 10 kΩ, 15 kΩ, 100 nF | divisor de V5_SENSE |
| R_LED1, R_LED2 | 330 Ω | ≈ 4 mA con LED rojo desde 3,3 V |
| R_ALERT | 10 kΩ | pull-up de ADS_ALERT a 3V3 |
| C_BULK | 100 µF / 16 V electrolítico | riel +5V junto a la MB102 |
| C_5V_x | 100 nF cerámico | uno por conector de sensor, LCD y driver, junto al conector |
| C_3V3 | 10 µF + 100 nF | riel 3V3 junto a los traductores |
| H1..H4 | MountingHole_3.2mm_M3 | fijación |
| TP_GND, TP_5V, TP_3V3, TP_SDA0, TP_SCL0, TP_SDA1, TP_SCL1 | TestPoint | medición con osciloscopio |

## 7. Archivos que genera Codex

```
hardware/placa-pico2/
  placa-pico2.kicad_pro
  placa-pico2.kicad_sch        (raíz con hojas jerárquicas)
  pico.kicad_sch               (Pico, 3V3, J_EXP)
  traductores.kicad_sch        (TXS0108E, BSS138 #1 y #2, pull-downs, pull-ups)
  conectores.kicad_sch         (drivers, AS5600, ACS712, ADS1115, finales, LCD, panel)
  alimentacion.kicad_sch       (MB102, capacitores, test points, agujeros)
  placa-pico2.kicad_pcb        (vacío: solo contorno; el usuario actualiza desde el esquemático)
  placa-pico2.kicad_sym        (símbolos de módulos propios)
  placa-pico2.pretty/          (huellas propias de módulos)
  sym-lib-table, fp-lib-table  (rutas relativas ${KIPRJMOD})
  BOM.csv                      (exportado con kicad-cli)
```

`.gitignore`: agregar `hardware/**/*-backups/`, `hardware/**/*.lck`, `hardware/**/fp-info-cache`, `hardware/**/*.kicad_prl`.

## 8. Reglas de ubicación y ruteo (para el usuario)

- Plano de GND continuo en la capa inferior; no cortarlo bajo las pistas I2C ni las de PWM.
- TXS0108E a ≤ 3 cm de los conectores de drivers; pistas A de la Pico al TXS cortas.
- Traductores BSS138 entre la Pico y los conectores de su bus.
- Pistas de señal 0,3 mm; riel +5V y GND de conectores 0,8 mm; separación 0,25 mm (mínimos de JLCPCB/PCBWay en 2 capas).
- Capacitores de 100 nF pegados a cada conector; C_BULK pegado a la MB102.
- Pico con el USB hacia el borde de la placa. Conectores JST en los bordes.
- Cable de AS5600: trenzar SDA con GND y SCL con GND (o cable con malla a GND en un solo extremo).

## 9. Criterios de aceptación (Codex)

1. `kicad-cli sch erc` sobre `placa-pico2.kicad_sch`: 0 errores (las advertencias se justifican en HANDOFF). Símbolo `MCU_Module:RaspberryPi_Pico` (no hay símbolo Pico 2; el pinout es igual): banderas de no conexión en 30, 35, 37, 39 y 40; PWR_FLAG en +5V, 3V3 y GND; AGND (33) a GND con la excepción de ERC justificada si el tipo de pin la genera.
2. `kicad-cli sch export netlist` sin errores. Codex escribe en HANDOFF una tabla red → (ref, pata) derivada de la tabla 4 y la verifica con un script temporal en `tmp/` (no se commitea), resultado en una línea.
3. Toda huella asignada existe en la biblioteca de KiCad 10 o en `placa-pico2.pretty`.
4. `BOM.csv` exportado con `kicad-cli sch export bom`.
5. Ningún archivo fuera de `hardware/`, `.gitignore`, documentación de la sección 11, `herramientas/flujo/nueva-version.ps1` y `CLAUDE.md` (estructura y versionado).

## 10. Versionado de la placa (herramienta del flujo)

`herramientas/flujo/nueva-version.ps1` gana el parámetro `-Placa`:
- Cuenta solo cambios en `hardware/placa-pico2/` desde la última etiqueta `PCB-V*`.
- Etiqueta `PCB-Vx.y` (empieza en `PCB-V1.0`, misma numeración 1.0…1.9, 2.0) e imprime `Version PCB-Vx.y creada`.
- Copia con `git archive HEAD hardware/placa-pico2` a `..\MiYoke_PD_VERSIONES\PCB_Vx_y\` y escribe `ACTUALIZACION_PCB_Vx_y.txt`.
- Sin `-Placa`, el comportamiento actual no cambia: la serie `V` ignora `hardware/` (agregar `hardware/.*` a `$flujo`) y busca solo etiquetas `V\d+\.\d+`.
- Correcciones necesarias del script (revisión): el chequeo `git tag --points-at HEAD` filtra solo la serie en uso (`^V\d` o `^PCB-V`); con `-Placa` la verificación de la copia cuenta `git ls-files hardware/placa-pico2` y no aplica el filtro de extensiones; `git archive` deja la ruta anidada `hardware/placa-pico2/` dentro de `PCB_Vx_y\` (aceptado).
- Pruebas del script (en una copia temporal del repo bajo `tmp/`, nunca en el repo real): (a) sin `-Placa` y solo `hardware/` cambiado → "Sin cambios", sin etiqueta; (b) con `-Placa` → crea `PCB-V1.0` y `git tag --list 'V*'` queda igual antes y después; (c) un commit con etiqueta `V*` no impide `-Placa`.
- `CLAUDE.md`: agregar a VERSIONADO que los cambios en `hardware/` crean versión con `-Placa`, y `hardware/` a ESTRUCTURA DE CARPETAS.

## 11. Documentación afectada

- `LEEME md/LEEME_PICO2.md`, tabla de pines: GP0–GP3 pasan a "Final de carrera (placa base)"; agregar filas de GP21 (OE del TXS0108E), GP22 (ADS_ALERT, reservado) y GP28 (detección de 5 V), y el orden de encendido (MB102 primero) y nota de que los periféricos van a 5 V con traductores en la placa.
- `LEEME md/LEEME_PLACA.md` (nuevo): resumen de la placa, conectores con su pinout (sección 5), cómo abrir el proyecto en KiCad 10, configuración de la MB102 y comando de versión `-Placa`.
- `CLAUDE.md`: ESTRUCTURA DE CARPETAS y VERSIONADO (sección 10).
- Tesis: no afecta (no cambia el cálculo de efectos ni de fuerzas).
