# Índice de archivos y funciones de fuerzas TelemFFB

Generado desde el AST de la copia local el 29/09/2026. Complementa la guía explicativa; incluye todos los archivos Python bajo `telemffb/sim`, más adquisición, matemática y backends relevantes.

La columna «operaciones» identifica llamadas directas dentro de la función. Un guion no significa que la función sea irrelevante: puede calcular parámetros o delegar en otros métodos. Las llamadas en funciones anidadas no se atribuyen a su contenedor. No es una traza de ejecución ni garantiza activación del efecto.

## [telemffb/sim/aircraft_base.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircraft_base.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [AircraftBase.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircraft_base.py:71>) | 71 | — |
| [AircraftBase.on_event](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircraft_base.py:79>) | 79 | — |
| [AircraftBase.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircraft_base.py:83>) | 83 | — |
| [AircraftBase.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircraft_base.py:102>) | 102 | — |

## [telemffb/sim/aircrafts_dcs.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [DCSCommands.dcs_send_commands](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:58>) | 58 | — |
| [DCSCommands.dcs_cmd_set_rudder](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:62>) | 62 | — |
| [DCSCommands.dcs_cmd_set_stick_x](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:73>) | 73 | — |
| [DCSCommands.dcs_cmd_set_stick_y](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:84>) | 84 | — |
| [Aircraft.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:169>) | 169 | spring |
| [Aircraft.ac_update_gforce_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:181>) | 181 | — |
| [Aircraft.ac_update_runway_rumble](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:187>) | 187 | — |
| [Aircraft.ac_update_decel_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:192>) | 192 | — |
| [Aircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:198>) | 198 | — |
| [Aircraft.on_event](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:249>) | 249 | — |
| [Aircraft.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:255>) | 255 | — |
| [Aircraft.dcs_update_damage](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:269>) | 269 | periodic |
| [Aircraft.dcs_override_collective_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:285>) | 285 | setCondition, set_coefficient, set_offset, spring |
| [Aircraft.ac_update_pedal_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:344>) | 344 | — |
| [Aircraft.dcs_update_pedal_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:347>) | 347 | setCondition, set_offset, spring |
| [Aircraft.dcs_update_stick_position](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:365>) | 365 | setCondition, set_coefficient, set_offset, spring |
| [Aircraft.dcs_update_stick_shaker](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:401>) | 401 | periodic |
| [Aircraft.dcs_update_ap_deadzone](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:431>) | 431 | — |
| [Aircraft.dcs_override_copilot_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:455>) | 455 | set_coefficient, spring |
| [Aircraft.dcs_override_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:501>) | 501 | setCondition, set_coefficient, set_offset, spring |
| [PropellerAircraft.dcs_update_actual_rpm](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:611>) | 611 | — |
| [PropellerAircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:622>) | 622 | — |
| [JetAircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:641>) | 641 | — |
| [TurbopropAircraft.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:649>) | 649 | — |
| [TurbopropAircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:653>) | 653 | — |
| [Helicopter.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:665>) | 665 | — |
| [Helicopter.dcs_update_trim_damper](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_dcs.py:675>) | 675 | setCondition, set_coefficient, set_offset |

## [telemffb/sim/aircrafts_il2.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [dbg](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:68>) | 68 | — |
| [Aircraft.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:131>) | 131 | spring |
| [Aircraft._il2_engine_value](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:150>) | 150 | — |
| [Aircraft.il2_update_engine_shake](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:176>) | 176 | periodic |
| [Aircraft.ac_update_cm_weapons](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:240>) | 240 | periodic |
| [Aircraft.ac_update_runway_rumble](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:287>) | 287 | — |
| [Aircraft.ac_update_buffeting](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:306>) | 306 | periodic |
| [Aircraft.il2_update_damage](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:321>) | 321 | periodic |
| [Aircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:343>) | 343 | — |
| [Aircraft.on_event](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:378>) | 378 | — |
| [Aircraft.gun_effect_from_mv](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:383>) | 383 | — |
| [Aircraft.il2_override_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:435>) | 435 | setCondition, set_coefficient, set_offset, spring |
| [Aircraft.il2_ffb_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:499>) | 499 | setCondition, set_coefficient, set_offset, spring |
| [Aircraft.il2_ffb_forces](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:550>) | 550 | constant, damper |
| [PropellerAircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:634>) | 634 | — |
| [JetAircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_il2.py:644>) | 644 | — |

## [telemffb/sim/aircrafts_msfs_xp.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/aircrafts_msfs_xp.py:1>)

Sin funciones propias de cálculo en este archivo.

## [telemffb/sim/base/AdvancedSpringMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [AdvancedSpringMixIn.adv_spr_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:36>) | 36 | — |
| [AdvancedSpringMixIn.adv_spr_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:40>) | 40 | — |
| [AdvancedSpringMixIn.spring_mode](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:48>) | 48 | — |
| [AdvancedSpringMixIn.spring_mode](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:52>) | 52 | — |
| [AdvancedSpringMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:85>) | 85 | — |
| [AdvancedSpringMixIn.spring_mode_is](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:97>) | 97 | — |
| [AdvancedSpringMixIn._adv_spr_device_has_adjuster](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:101>) | 101 | — |
| [AdvancedSpringMixIn._tap_feeds_advanced_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:105>) | 105 | — |
| [AdvancedSpringMixIn._adv_spr_via_adjuster](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:115>) | 115 | — |
| [AdvancedSpringMixIn.ac_modify_game_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:121>) | 121 | setCondition, set_coefficient, set_offset, spring_adjuster |
| [AdvancedSpringMixIn.ac_update_gforce_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:207>) | 207 | — |
| [AdvancedSpringMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AdvancedSpringMixIn.py:213>) | 213 | — |

## [telemffb/sim/base/AircraftEffectUtilsBase.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [AircraftEffectUtilsBase.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:15>) | 15 | — |
| [AircraftEffectUtilsBase.effects](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:28>) | 28 | — |
| [AircraftEffectUtilsBase.telem_data](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:42>) | 42 | — |
| [AircraftEffectUtilsBase.aircraft_type](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:46>) | 46 | — |
| [AircraftEffectUtilsBase.check_master_button_press](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:52>) | 52 | — |
| [AircraftEffectUtilsBase.check_button_press](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:56>) | 56 | — |
| [AircraftEffectUtilsBase.step_value_over_time](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:66>) | 66 | — |
| [AircraftEffectUtilsBase.apply_settings](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:138>) | 138 | — |
| [AircraftEffectUtilsBase.has_changed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:169>) | 169 | — |
| [AircraftEffectUtilsBase.flag_error](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:202>) | 202 | — |
| [AircraftEffectUtilsBase.is_joystick](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:232>) | 232 | — |
| [AircraftEffectUtilsBase.is_pedals](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:240>) | 240 | — |
| [AircraftEffectUtilsBase.is_collective](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:248>) | 248 | — |
| [AircraftEffectUtilsBase.is_trimwheel](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:256>) | 256 | — |
| [AircraftEffectUtilsBase.anything_has_changed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:265>) | 265 | — |
| [AircraftEffectUtilsBase._sim_is_msfs](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:295>) | 295 | — |
| [AircraftEffectUtilsBase._sim_is_xplane](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:303>) | 303 | — |
| [AircraftEffectUtilsBase._sim_is_dcs](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:311>) | 311 | — |
| [AircraftEffectUtilsBase._sim_is_bms](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:319>) | 319 | — |
| [AircraftEffectUtilsBase._sim_is_il2](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:328>) | 328 | — |
| [AircraftEffectUtilsBase._sim_is](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:336>) | 336 | — |
| [AircraftEffectUtilsBase._get_device_axes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:349>) | 349 | — |
| [AircraftEffectUtilsBase._get_device_CP_XY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:359>) | 359 | — |
| [AircraftEffectUtilsBase._get_device_raw_axes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:369>) | 369 | — |
| [AircraftEffectUtilsBase._get_device_report](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:381>) | 381 | — |
| [AircraftEffectUtilsBase._get_device_forces](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:388>) | 388 | — |
| [AircraftEffectUtilsBase._device_feeding](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:401>) | 401 | — |
| [AircraftEffectUtilsBase._get_random_direction](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:415>) | 415 | — |
| [AircraftEffectUtilsBase._get_effect_direction](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:423>) | 423 | — |
| [AircraftEffectUtilsBase._should_skip_joystick_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:429>) | 429 | — |
| [AircraftEffectUtilsBase._should_skip_airborne_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:433>) | 433 | — |
| [AircraftEffectUtilsBase._should_skip_no_airspeed_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:444>) | 444 | — |
| [AircraftEffectUtilsBase._is_telemetry_spike](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:448>) | 448 | — |
| [AircraftEffectUtilsBase.is_jet_aircraft](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:453>) | 453 | — |
| [AircraftEffectUtilsBase.is_propeller_aircraft](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:461>) | 461 | — |
| [AircraftEffectUtilsBase.is_helicopter](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:469>) | 469 | — |
| [AircraftEffectUtilsBase.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:479>) | 479 | — |
| [AircraftEffectUtilsBase.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:483>) | 483 | — |
| [AircraftEffectUtilsBase.on_event](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:487>) | 487 | — |
| [AircraftEffectUtilsBase.on_shutdown](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftEffectUtilsBase.py:491>) | 491 | — |

## [telemffb/sim/base/AircraftParamsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AircraftParamsMixIn.py:1>)

Sin funciones propias de cálculo en este archivo.

## [telemffb/sim/base/AoAEffectsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AoAEffectsMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [AoAEffectsMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AoAEffectsMixIn.py:33>) | 33 | — |
| [AoAEffectsMixIn.ac_update_aoa_reduction_force_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AoAEffectsMixIn.py:38>) | 38 | constant |
| [AoAEffectsMixIn.ac_update_aoa_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AoAEffectsMixIn.py:73>) | 73 | constant |
| [AoAEffectsMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/AoAEffectsMixIn.py:109>) | 109 | — |

## [telemffb/sim/base/BuffetingEffectMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/BuffetingEffectMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [BuffetingEffectMixIn.ac_update_buffeting](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/BuffetingEffectMixIn.py:93>) | 93 | periodic |
| [BuffetingEffectMixIn._dynamic_buffet_render](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/BuffetingEffectMixIn.py:224>) | 224 | — |
| [BuffetingEffectMixIn.ac_update_drag_buffet](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/BuffetingEffectMixIn.py:263>) | 263 | — |
| [BuffetingEffectMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/BuffetingEffectMixIn.py:269>) | 269 | — |

## [telemffb/sim/base/DeadzoneMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DeadzoneMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [pct2dz](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DeadzoneMixIn.py:8>) | 8 | — |
| [DeadzoneMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DeadzoneMixIn.py:17>) | 17 | — |
| [DeadzoneMixIn._write_device_deadzone](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DeadzoneMixIn.py:25>) | 25 | — |
| [DeadzoneMixIn.ac_set_deadzone_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DeadzoneMixIn.py:42>) | 42 | — |
| [DeadzoneMixIn.ac_update_deadzone](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DeadzoneMixIn.py:54>) | 54 | — |
| [DeadzoneMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DeadzoneMixIn.py:82>) | 82 | — |
| [DeadzoneMixIn.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DeadzoneMixIn.py:87>) | 87 | — |

## [telemffb/sim/base/DecelerationEffectMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DecelerationEffectMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [DecelerationEffectMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DecelerationEffectMixIn.py:24>) | 24 | — |
| [DecelerationEffectMixIn.ac_update_decel_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DecelerationEffectMixIn.py:31>) | 31 | constant |
| [DecelerationEffectMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DecelerationEffectMixIn.py:135>) | 135 | — |

## [telemffb/sim/base/DynamicSpringMixin.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DynamicSpringMixin.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [DynamicSpringMixin.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DynamicSpringMixin.py:6>) | 6 | — |
| [DynamicSpringMixin._spring_handle](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/DynamicSpringMixin.py:14>) | 14 | spring |

## [telemffb/sim/base/ElevatorDroopEffectMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/ElevatorDroopEffectMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [ElevatorDroopEffectMixIn.ac_override_elevator_droop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/ElevatorDroopEffectMixIn.py:13>) | 13 | constant |
| [ElevatorDroopEffectMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/ElevatorDroopEffectMixIn.py:36>) | 36 | — |

## [telemffb/sim/base/EngineRumbleMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [EngineRumbleMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:32>) | 32 | — |
| [EngineRumbleMixIn.ac_update_piston_engine_rumble](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:38>) | 38 | periodic |
| [EngineRumbleMixIn.ac_calc_engine_intensity](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:113>) | 113 | — |
| [EngineRumbleMixIn.ac_update_jet_engine_rumble](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:139>) | 139 | periodic |
| [EngineRumbleMixIn.ac_update_ab_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:183>) | 183 | periodic |
| [EngineRumbleMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/EngineRumbleMixIn.py:223>) | 223 | — |

## [telemffb/sim/base/FFBForcesMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FFBForcesMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [FFBForcesMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FFBForcesMixIn.py:23>) | 23 | — |
| [FFBForcesMixIn.ac_update_ffb_forces](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FFBForcesMixIn.py:29>) | 29 | damper, friction, inertia |

## [telemffb/sim/base/FfbTapMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [FfbTapMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:80>) | 80 | — |
| [FfbTapMixIn.ffb_tap_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:90>) | 90 | setCondition, spring |
| [FfbTapMixIn.ffb_tap_advanced_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:166>) | 166 | setCondition, spring |
| [FfbTapMixIn._warn_if_tap_is_missing](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:212>) | 212 | — |
| [FfbTapMixIn._warn_if_tap_is_swallowing](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:247>) | 247 | — |
| [FfbTapMixIn._tap_type_settings_suffix](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:305>) | 305 | — |
| [FfbTapMixIn._tap_type_enabled](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:317>) | 317 | — |
| [FfbTapMixIn._tap_type_gain](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:328>) | 328 | — |
| [FfbTapMixIn._tap_correct_direction](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:332>) | 332 | — |
| [FfbTapMixIn._tap_effects_teardown](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:343>) | 343 | — |
| [FfbTapMixIn._tap_forget](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:350>) | 350 | — |
| [FfbTapMixIn._ffb_tap_game_effects](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:355>) | 355 | — |
| [FfbTapMixIn._tap_type_pct_key](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:435>) | 435 | — |
| [FfbTapMixIn._render_tap_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/FfbTapMixIn.py:445>) | 445 | constant, periodic, setCondition |

## [telemffb/sim/base/GForceEffectMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [GForceEffectProperties.gforce_effect_mode](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:19>) | 19 | — |
| [GForceEffectProperties.gforce_effect_mode](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:23>) | 23 | — |
| [GForceEffectProperties.gforce_effect_adv_curve](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:53>) | 53 | — |
| [GForceEffectProperties.gforce_effect_adv_curve](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:57>) | 57 | — |
| [GForceEffectMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:97>) | 97 | — |
| [GForceEffectMixIn.__gforce_dispose_all](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:104>) | 104 | — |
| [GForceEffectMixIn.__check_firmware_support](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:107>) | 107 | — |
| [GForceEffectMixIn.gforce_effect_mode_is](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:137>) | 137 | — |
| [GForceEffectMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:141>) | 141 | — |
| [GForceEffectMixIn._get_gs_data](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:145>) | 145 | — |
| [GForceEffectMixIn._ac_run_new_gforce_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:181>) | 181 | constant |
| [GForceEffectMixIn.g_effect_get_adv_mode](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:279>) | 279 | — |
| [GForceEffectMixIn.ac_update_gforce_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/GForceEffectMixIn.py:282>) | 282 | constant, setCondition, set_offset, spring_adjuster |

## [telemffb/sim/base/HelicopterEffectsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HelicopterEffectsMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [HelicopterEffectsMixIn.ac_calc_etl_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HelicopterEffectsMixIn.py:77>) | 77 | periodic |
| [HelicopterEffectsMixIn.ac_update_vrs_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HelicopterEffectsMixIn.py:148>) | 148 | periodic |
| [HelicopterEffectsMixIn.ac_update_heli_engine_rumble](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HelicopterEffectsMixIn.py:192>) | 192 | periodic |
| [HelicopterEffectsMixIn.ac_collective_force_trim_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HelicopterEffectsMixIn.py:251>) | 251 | setCondition, set_coefficient, set_offset |
| [HelicopterEffectsMixIn._blade_slap_signal](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HelicopterEffectsMixIn.py:350>) | 350 | — |
| [HelicopterEffectsMixIn.ac_update_blade_slap](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HelicopterEffectsMixIn.py:407>) | 407 | periodic |
| [HelicopterEffectsMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HelicopterEffectsMixIn.py:450>) | 450 | — |

## [telemffb/sim/base/HydraulicLossMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [HydraulicLossMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:26>) | 26 | — |
| [HydraulicLossMixIn._hydraulic_source_active](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:36>) | 36 | — |
| [HydraulicLossMixIn._hydraulic_source_transform](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:40>) | 40 | — |
| [HydraulicLossMixIn._custom_hydraulic_health](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:55>) | 55 | — |
| [HydraulicLossMixIn._hydraulic_health](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:78>) | 78 | — |
| [HydraulicLossMixIn._ramp_hydraulic_factor](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:116>) | 116 | — |
| [HydraulicLossMixIn._stop_hydraulic_loss_effects](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:127>) | 127 | — |
| [HydraulicLossMixIn._reset_hydraulic_loss](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:131>) | 131 | — |
| [HydraulicLossMixIn.ac_update_hydraulic_loss_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:138>) | 138 | damper, friction |
| [HydraulicLossMixIn.ac_update_hydraulic_and_ffb_forces](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:189>) | 189 | — |
| [HydraulicLossMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/HydraulicLossMixIn.py:203>) | 203 | — |

## [telemffb/sim/base/MotionEffectsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [MotionEffectsMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:69>) | 69 | — |
| [MotionEffectsMixIn._bms_taxi_bumps](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:82>) | 82 | periodic |
| [MotionEffectsMixIn.ac_update_flaps](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:93>) | 93 | periodic |
| [MotionEffectsMixIn.ac_update_runway_rumble](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:116>) | 116 | constant |
| [MotionEffectsMixIn.ac_update_touchdown_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:160>) | 160 | constant |
| [MotionEffectsMixIn.ac_update_canopy](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:194>) | 194 | periodic |
| [MotionEffectsMixIn.ac_update_landing_gear](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:215>) | 215 | periodic |
| [MotionEffectsMixIn.ac_update_speed_brakes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:273>) | 273 | periodic |
| [MotionEffectsMixIn.ac_update_spoilers](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:321>) | 321 | periodic |
| [MotionEffectsMixIn.ac_update_tailhook_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:372>) | 372 | periodic |
| [MotionEffectsMixIn.ac_update_fuelboom_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:396>) | 396 | — |
| [MotionEffectsMixIn.ac_update_wingfold_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:417>) | 417 | — |
| [MotionEffectsMixIn._create_motion_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:442>) | 442 | periodic |
| [MotionEffectsMixIn._create_buffeting_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:502>) | 502 | periodic |
| [MotionEffectsMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py:544>) | 544 | — |

## [telemffb/sim/base/PedalSpringOverrideMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/PedalSpringOverrideMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [PedalSpringOverrideMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/PedalSpringOverrideMixIn.py:28>) | 28 | — |
| [PedalSpringOverrideMixIn.ac_update_pedal_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/PedalSpringOverrideMixIn.py:34>) | 34 | — |
| [PedalSpringOverrideMixIn.ac_update_pedal_force_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/PedalSpringOverrideMixIn.py:40>) | 40 | set_coefficient, set_offset |
| [PedalSpringOverrideMixIn.ac_override_pedal_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/PedalSpringOverrideMixIn.py:131>) | 131 | setCondition, set_coefficient, set_offset, spring |
| [PedalSpringOverrideMixIn.verify_pedal_axis](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/PedalSpringOverrideMixIn.py:201>) | 201 | — |
| [PedalSpringOverrideMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/PedalSpringOverrideMixIn.py:228>) | 228 | — |

## [telemffb/sim/base/WeaponsEffectMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WeaponsEffectMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [WeaponsEffectMixIn.ac_update_cm_weapons](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WeaponsEffectMixIn.py:26>) | 26 | periodic |
| [WeaponsEffectMixIn._create_weapon_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WeaponsEffectMixIn.py:74>) | 74 | periodic |
| [WeaponsEffectMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WeaponsEffectMixIn.py:102>) | 102 | — |

## [telemffb/sim/base/WindEffectMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WindEffectMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [WindEffectMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WindEffectMixIn.py:20>) | 20 | — |
| [WindEffectMixIn.ac_update_wind_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WindEffectMixIn.py:26>) | 26 | constant |
| [WindEffectMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/base/WindEffectMixIn.py:54>) | 54 | — |

## [telemffb/sim/BaseTelemetryData.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [BaseTelemetryData.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1530>) | 1530 | — |
| [BaseTelemetryData._known_fields](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1540>) | 1540 | — |
| [BaseTelemetryData.__getitem__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1544>) | 1544 | — |
| [BaseTelemetryData.__setitem__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1547>) | 1547 | — |
| [BaseTelemetryData.__delitem__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1550>) | 1550 | — |
| [BaseTelemetryData.__contains__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1553>) | 1553 | — |
| [BaseTelemetryData.__len__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1556>) | 1556 | — |
| [BaseTelemetryData.__iter__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1559>) | 1559 | — |
| [BaseTelemetryData.__bool__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1562>) | 1562 | — |
| [BaseTelemetryData.__repr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1565>) | 1565 | — |
| [BaseTelemetryData.get](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1571>) | 1571 | — |
| [BaseTelemetryData.pop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1574>) | 1574 | — |
| [BaseTelemetryData.setdefault](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1577>) | 1577 | — |
| [BaseTelemetryData.keys](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1580>) | 1580 | — |
| [BaseTelemetryData.items](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1583>) | 1583 | — |
| [BaseTelemetryData.values](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1586>) | 1586 | — |
| [BaseTelemetryData.clear](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1589>) | 1589 | — |
| [BaseTelemetryData.update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1592>) | 1592 | — |
| [BaseTelemetryData.to_dict](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1600>) | 1600 | — |
| [BaseTelemetryData.from_dict](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1604>) | 1604 | — |
| [BaseTelemetryData.copy](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1607>) | 1607 | — |
| [BaseTelemetryData.__getattr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1611>) | 1611 | — |
| [BaseTelemetryData.__setattr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1618>) | 1618 | — |
| [BaseTelemetryData.__delattr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/BaseTelemetryData.py:1629>) | 1629 | — |

## [telemffb/sim/msfs_xp/Aircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [Aircraft.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:91>) | 91 | — |
| [Aircraft.msfs_update_stick_shaker](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:118>) | 118 | periodic |
| [Aircraft.msfs_override_collective_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:134>) | 134 | setCondition, set_coefficient, set_offset, spring |
| [Aircraft.find_xp_gear_orientation](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:156>) | 156 | — |
| [Aircraft.on_event](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:160>) | 160 | — |
| [Aircraft._probe_command_scale](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:190>) | 190 | — |
| [Aircraft._probe_axis_jitter](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:214>) | 214 | — |
| [Aircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:242>) | 242 | — |
| [Aircraft.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Aircraft.py:291>) | 291 | setCondition, set_coefficient, set_offset, spring |

## [telemffb/sim/msfs_xp/CowanSimHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/CowanSimHelicopter.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [CowanSimHelicopter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/CowanSimHelicopter.py:22>) | 22 | — |

## [telemffb/sim/msfs_xp/FFBApiHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [FFBApiHelicopter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:199>) | 199 | — |
| [FFBApiHelicopter.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:216>) | 216 | — |
| [FFBApiHelicopter.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:231>) | 231 | — |
| [FFBApiHelicopter.on_shutdown](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:242>) | 242 | — |
| [FFBApiHelicopter.subscribe_simvars](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:258>) | 258 | — |
| [FFBApiHelicopter._ffb_api_reset](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:266>) | 266 | — |
| [FFBApiHelicopter._subscribe_ffb_api_simvars](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:282>) | 282 | — |
| [FFBApiHelicopter._ffb_api_latch_discovery](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:301>) | 301 | — |
| [FFBApiHelicopter._ffb_api_warn_missing_version](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:374>) | 374 | — |
| [FFBApiHelicopter._ffb_api_control](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:398>) | 398 | — |
| [FFBApiHelicopter._ffb_api_supported](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:412>) | 412 | — |
| [FFBApiHelicopter._ffb_api_active](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:421>) | 421 | — |
| [FFBApiHelicopter._ffb_api_has_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:434>) | 434 | — |
| [FFBApiHelicopter._ffb_api_write_lvar](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:449>) | 449 | — |
| [FFBApiHelicopter._ffb_api_update_enabled_flag](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:454>) | 454 | — |
| [FFBApiHelicopter._ffb_api_disable](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:483>) | 483 | — |
| [FFBApiHelicopter._ffb_api_release](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:504>) | 504 | — |
| [FFBApiHelicopter._ffb_api_write_fly_through](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:509>) | 509 | — |
| [FFBApiHelicopter._ffb_api_detect_fly_through](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:529>) | 529 | — |
| [FFBApiHelicopter._ffb_api_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:549>) | 549 | — |
| [FFBApiHelicopter._ffb_api_to_device_sign](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:554>) | 554 | — |
| [FFBApiHelicopter._ffb_api_tr_on](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:568>) | 568 | — |
| [FFBApiHelicopter._ffb_api_apply_hydraulic_loss](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:578>) | 578 | — |
| [FFBApiHelicopter._ffb_api_on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:614>) | 614 | — |
| [FFBApiHelicopter._ffb_api_restart_recapture](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:653>) | 653 | — |
| [FFBApiHelicopter._ffb_api_recapture](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:658>) | 658 | — |
| [FFBApiHelicopter._ffb_api_resume_point](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:683>) | 683 | — |
| [FFBApiHelicopter._ffb_api_discovering](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:701>) | 701 | — |
| [FFBApiHelicopter._update_cyclic_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:712>) | 712 | — |
| [FFBApiHelicopter.msfs_update_heli_controls](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:729>) | 729 | setCondition, set_coefficient, set_offset |
| [FFBApiHelicopter.msfs_update_collective](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:813>) | 813 | setCondition, set_coefficient, set_offset |
| [FFBApiHelicopter.msfs_update_pedals](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FFBApiHelicopter.py:874>) | 874 | setCondition, set_coefficient, set_offset |

## [telemffb/sim/msfs_xp/FlyInsideHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FlyInsideHelicopter.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [FlyInsideHelicopter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FlyInsideHelicopter.py:34>) | 34 | — |
| [FlyInsideHelicopter.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FlyInsideHelicopter.py:42>) | 42 | — |
| [FlyInsideHelicopter.ac_calc_etl_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FlyInsideHelicopter.py:49>) | 49 | — |
| [FlyInsideHelicopter.ac_update_vrs_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FlyInsideHelicopter.py:54>) | 54 | — |
| [FlyInsideHelicopter._update_vibration](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/FlyInsideHelicopter.py:58>) | 58 | constant |

## [telemffb/sim/msfs_xp/GliderAircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/GliderAircraft.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [GliderAircraft.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/GliderAircraft.py:31>) | 31 | — |
| [GliderAircraft.msfs_update_force_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/GliderAircraft.py:34>) | 34 | setCondition, set_coefficient, set_offset |
| [GliderAircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/GliderAircraft.py:117>) | 117 | — |

## [telemffb/sim/msfs_xp/Helicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [Helicopter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:77>) | 77 | — |
| [Helicopter.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:90>) | 90 | — |
| [Helicopter.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:97>) | 97 | — |
| [Helicopter.msfs_update_trimwheel](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:112>) | 112 | — |
| [Helicopter.subscribe_simvars](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:115>) | 115 | — |
| [Helicopter.check_hands_on](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:123>) | 123 | — |
| [Helicopter._dispatch_hands_on_state](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:151>) | 151 | — |
| [Helicopter._get_msfs_pedal_axis_config](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:186>) | 186 | — |
| [Helicopter._get_msfs_collective_axis_config](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:191>) | 191 | — |
| [Helicopter._initialize_pedals_if_needed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:196>) | 196 | setCondition, set_coefficient, set_offset |
| [Helicopter._update_pedal_spring_mode](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:226>) | 226 | setCondition, set_coefficient |
| [Helicopter.ac_update_pedal_force_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:237>) | 237 | set_coefficient, set_offset |
| [Helicopter._send_pedal_outputs](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:268>) | 268 | — |
| [Helicopter.msfs_send_heli_pedal_pos](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:282>) | 282 | — |
| [Helicopter._handle_collective_controls_lock](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:285>) | 285 | setCondition, set_coefficient, set_offset |
| [Helicopter._initialize_collective_if_needed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:325>) | 325 | setCondition, set_coefficient, set_offset |
| [Helicopter._update_collective_spring_mode](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:363>) | 363 | setCondition, set_coefficient, set_offset |
| [Helicopter._send_collective_outputs](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:376>) | 376 | — |
| [Helicopter.msfs_send_heli_collective_pos](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:391>) | 391 | — |
| [Helicopter.msfs_update_pedals](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:394>) | 394 | — |
| [Helicopter.msfs_update_collective](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/Helicopter.py:429>) | 429 | — |

## [telemffb/sim/msfs_xp/HPGHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [HPGHelicopter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:126>) | 126 | — |
| [HPGHelicopter.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:147>) | 147 | — |
| [HPGHelicopter.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:153>) | 153 | — |
| [HPGHelicopter.check_feet_on](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:171>) | 171 | — |
| [HPGHelicopter.msfs_update_heli_controls](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:217>) | 217 | setCondition, set_offset |
| [HPGHelicopter._update_cyclic_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:457>) | 457 | — |
| [HPGHelicopter.msfs_update_pedals](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:467>) | 467 | setCondition, set_coefficient, set_offset |
| [HPGHelicopter.msfs_update_collective](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:630>) | 630 | setCondition, set_coefficient, set_offset |
| [HPGHelicopter.ac_update_vrs_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/HPGHelicopter.py:881>) | 881 | periodic |

## [telemffb/sim/msfs_xp/JetAircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/JetAircraft.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [JetAircraft.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/JetAircraft.py:28>) | 28 | — |
| [JetAircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/JetAircraft.py:32>) | 32 | — |

## [telemffb/sim/msfs_xp/MsfsXpFBWFlightControlsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFBWFlightControlsMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [MsfsXpFBWFlightControlsMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFBWFlightControlsMixIn.py:65>) | 65 | — |
| [MsfsXpFBWFlightControlsMixIn.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFBWFlightControlsMixIn.py:71>) | 71 | — |
| [MsfsXpFBWFlightControlsMixIn.update_fbw_flight_controls](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFBWFlightControlsMixIn.py:76>) | 76 | setCondition, set_coefficient, set_offset |

## [telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [MsfsXpFlightControlsMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:55>) | 55 | — |
| [MsfsXpFlightControlsMixIn._sync_controls_lock_simvar](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:67>) | 67 | — |
| [MsfsXpFlightControlsMixIn._get_msfs_axis_config](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:80>) | 80 | — |
| [MsfsXpFlightControlsMixIn._scale_msfs_axis_value](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:88>) | 88 | — |
| [MsfsXpFlightControlsMixIn._send_msfs_axis_value](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:97>) | 97 | — |
| [MsfsXpFlightControlsMixIn._get_controls_lock_state](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:106>) | 106 | — |
| [MsfsXpFlightControlsMixIn._lock_effects_started](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:115>) | 115 | — |
| [MsfsXpFlightControlsMixIn._stop_lock_effects](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:118>) | 118 | — |
| [MsfsXpFlightControlsMixIn._prepare_controls_lock](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:122>) | 122 | — |
| [MsfsXpFlightControlsMixIn._engage_controls_lock_detents](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:148>) | 148 | detent, setCondition, set_coefficient |
| [MsfsXpFlightControlsMixIn._apply_joystick_controls_lock](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:175>) | 175 | setCondition, set_coefficient |
| [MsfsXpFlightControlsMixIn._apply_pedal_controls_lock](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:210>) | 210 | setCondition, set_coefficient |
| [MsfsXpFlightControlsMixIn.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:241>) | 241 | — |
| [MsfsXpFlightControlsMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:267>) | 267 | — |
| [MsfsXpFlightControlsMixIn.ac_modify_game_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:292>) | 292 | — |
| [MsfsXpFlightControlsMixIn.ac_update_gforce_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:296>) | 296 | — |
| [MsfsXpFlightControlsMixIn.get_trim_calibrator](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:308>) | 308 | — |
| [MsfsXpFlightControlsMixIn._calculate_airspeeds](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:319>) | 319 | — |
| [MsfsXpFlightControlsMixIn._calculate_angles](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:348>) | 348 | — |
| [MsfsXpFlightControlsMixIn._calculate_prop_airflow](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:388>) | 388 | — |
| [MsfsXpFlightControlsMixIn._calculate_dynamic_pressures](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:437>) | 437 | — |
| [MsfsXpFlightControlsMixIn._calculate_vne_and_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:482>) | 482 | — |
| [MsfsXpFlightControlsMixIn.elevator_droop_term_for](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:556>) | 556 | — |
| [MsfsXpFlightControlsMixIn._calculate_control_coefficients](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:563>) | 563 | — |
| [MsfsXpFlightControlsMixIn._calculate_g_term](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:635>) | 635 | — |
| [MsfsXpFlightControlsMixIn._calculate_rudder_force](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:670>) | 670 | — |
| [MsfsXpFlightControlsMixIn._calculate_joystick_trim_offsets](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:714>) | 714 | — |
| [MsfsXpFlightControlsMixIn._send_joystick_axis_commands](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:781>) | 781 | — |
| [MsfsXpFlightControlsMixIn._calculate_aoa_offset](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:827>) | 827 | — |
| [MsfsXpFlightControlsMixIn._calculate_joystick_spring_coefficients](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:857>) | 857 | — |
| [MsfsXpFlightControlsMixIn.apply_joystick_constant_forces](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:892>) | 892 | — |
| [MsfsXpFlightControlsMixIn._apply_joystick_constant_forces](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:901>) | 901 | constant |
| [MsfsXpFlightControlsMixIn._update_joystick_controls](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:923>) | 923 | setCondition, set_coefficient |
| [MsfsXpFlightControlsMixIn._calculate_rudder_trim_offsets](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:954>) | 954 | — |
| [MsfsXpFlightControlsMixIn._calculate_rudder_spring_coefficient](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:973>) | 973 | set_coefficient |
| [MsfsXpFlightControlsMixIn._send_rudder_axis_commands](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:1006>) | 1006 | — |
| [MsfsXpFlightControlsMixIn._update_pedals_controls](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:1029>) | 1029 | constant, setCondition |
| [MsfsXpFlightControlsMixIn.msfs_update_flight_controls](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py:1056>) | 1056 | — |

## [telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [MsfsXpHeliControlsMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:26>) | 26 | — |
| [MsfsXpHeliControlsMixIn._sync_force_trim_simvar](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:29>) | 29 | — |
| [MsfsXpHeliControlsMixIn._force_trim_configured](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:37>) | 37 | — |
| [MsfsXpHeliControlsMixIn._initialize_cyclic_if_needed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:46>) | 46 | setCondition, set_coefficient, set_offset |
| [MsfsXpHeliControlsMixIn._handle_cyclic_trim_reset](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:95>) | 95 | set_coefficient, set_offset |
| [MsfsXpHeliControlsMixIn._update_cyclic_force_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:124>) | 124 | setCondition, set_coefficient, set_offset |
| [MsfsXpHeliControlsMixIn._send_cyclic_axis_output](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:253>) | 253 | — |
| [MsfsXpHeliControlsMixIn.msfs_send_heli_cyclic_pos](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:291>) | 291 | — |
| [MsfsXpHeliControlsMixIn.msfs_update_heli_controls](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:295>) | 295 | setCondition, set_coefficient, set_offset |
| [MsfsXpHeliControlsMixIn._update_cyclic_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:365>) | 365 | — |
| [MsfsXpHeliControlsMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpHeliControlsMixIn.py:402>) | 402 | — |

## [telemffb/sim/msfs_xp/MsfsXpNosewheelShimmyMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpNosewheelShimmyMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [MsfsXpNosewheelShimmyMixIn.msfs_update_nosewheel_shimmy](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpNosewheelShimmyMixIn.py:18>) | 18 | periodic |
| [MsfsXpNosewheelShimmyMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpNosewheelShimmyMixIn.py:59>) | 59 | — |

## [telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [SimConnectProxy.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:46>) | 46 | — |
| [SimConnectProxy.__getattr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:49>) | 49 | — |
| [SimConnectProxy.__bool__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:56>) | 56 | — |
| [MsfsXpSimConnectMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:87>) | 87 | — |
| [MsfsXpSimConnectMixIn.__del__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:104>) | 104 | — |
| [MsfsXpSimConnectMixIn.joystick_trim_follow_curve_y](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:123>) | 123 | — |
| [MsfsXpSimConnectMixIn.joystick_trim_follow_curve_y](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:127>) | 127 | — |
| [MsfsXpSimConnectMixIn._trim_curve_offset](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:157>) | 157 | — |
| [MsfsXpSimConnectMixIn._trim_follow_virtual_offset_y](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:167>) | 167 | — |
| [MsfsXpSimConnectMixIn._trim_follow_center_y](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:223>) | 223 | — |
| [MsfsXpSimConnectMixIn._simconnect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:282>) | 282 | — |
| [MsfsXpSimConnectMixIn._sync_runtime_simvar](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:285>) | 285 | — |
| [MsfsXpSimConnectMixIn.send_xp_command](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:310>) | 310 | — |
| [MsfsXpSimConnectMixIn._axis_control_enabled](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:317>) | 317 | — |
| [MsfsXpSimConnectMixIn._firmware_axis_override_supported](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:320>) | 320 | — |
| [MsfsXpSimConnectMixIn._use_firmware_axis_backend](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:327>) | 327 | — |
| [MsfsXpSimConnectMixIn._use_sim_axis_backend](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:339>) | 339 | — |
| [MsfsXpSimConnectMixIn._send_firmware_axis_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:342>) | 342 | — |
| [MsfsXpSimConnectMixIn._clear_firmware_axis_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:358>) | 358 | — |
| [MsfsXpSimConnectMixIn._send_firmware_fixed_axes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:366>) | 366 | — |
| [MsfsXpSimConnectMixIn.write_xp_dataref](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:387>) | 387 | — |
| [MsfsXpSimConnectMixIn.trigger_xp_event](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:392>) | 392 | — |
| [MsfsXpSimConnectMixIn._xp_role](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:415>) | 415 | — |
| [MsfsXpSimConnectMixIn._xp_reported_override_state](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:424>) | 424 | — |
| [MsfsXpSimConnectMixIn._send_xp_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:448>) | 448 | — |
| [MsfsXpSimConnectMixIn.release_xp_axis_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:462>) | 462 | — |
| [MsfsXpSimConnectMixIn.toggle_xp_control](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSimConnectMixIn.py:479>) | 479 | — |

## [telemffb/sim/msfs_xp/MsfsXpSteeringFrictionMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSteeringFrictionMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [MsfsXpSteeringFrictionMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSteeringFrictionMixIn.py:69>) | 69 | — |
| [MsfsXpSteeringFrictionMixIn.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSteeringFrictionMixIn.py:74>) | 74 | — |
| [MsfsXpSteeringFrictionMixIn._update_pedals_spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSteeringFrictionMixIn.py:77>) | 77 | — |
| [MsfsXpSteeringFrictionMixIn._apply_steering_friction](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSteeringFrictionMixIn.py:94>) | 94 | set_coefficient, set_offset |
| [MsfsXpSteeringFrictionMixIn.msfs_update_steering_friction_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpSteeringFrictionMixIn.py:138>) | 138 | friction |

## [telemffb/sim/msfs_xp/MsfsXpTrimwheelMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpTrimwheelMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [MsfsXpTrimwheelMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpTrimwheelMixIn.py:26>) | 26 | — |
| [MsfsXpTrimwheelMixIn.msfs_update_trimwheel](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpTrimwheelMixIn.py:49>) | 49 | setCondition, set_coefficient, set_offset |
| [MsfsXpTrimwheelMixIn._trimwheel_trim_limits](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpTrimwheelMixIn.py:238>) | 238 | — |
| [MsfsXpTrimwheelMixIn._trimwheel_sim_pos](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpTrimwheelMixIn.py:262>) | 262 | — |

## [telemffb/sim/msfs_xp/PropellerAircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/PropellerAircraft.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [PropellerAircraft.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/PropellerAircraft.py:28>) | 28 | — |
| [PropellerAircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/PropellerAircraft.py:33>) | 33 | — |

## [telemffb/sim/msfs_xp/SASHelicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/SASHelicopter.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [SASHelicopter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/SASHelicopter.py:45>) | 45 | — |
| [SASHelicopter.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/SASHelicopter.py:52>) | 52 | — |
| [SASHelicopter.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/SASHelicopter.py:56>) | 56 | — |
| [SASHelicopter.msfs_update_heli_controls](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/SASHelicopter.py:61>) | 61 | setCondition, set_offset |
| [SASHelicopter._update_cyclic_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/SASHelicopter.py:114>) | 114 | — |

## [telemffb/sim/msfs_xp/TaogH500Helicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TaogH500Helicopter.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [TaogH500Helicopter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TaogH500Helicopter.py:33>) | 33 | — |
| [TaogH500Helicopter.msfs_send_heli_pedal_pos](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TaogH500Helicopter.py:37>) | 37 | — |
| [TaogH500Helicopter.msfs_send_heli_cyclic_pos](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TaogH500Helicopter.py:44>) | 44 | — |
| [TaogH500Helicopter.msfs_send_heli_collective_pos](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TaogH500Helicopter.py:51>) | 51 | — |

## [telemffb/sim/msfs_xp/TrimCalibrator.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [TrimCalibrator.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:436>) | 436 | — |
| [TrimCalibrator._reset_runtime](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:492>) | 492 | — |
| [TrimCalibrator.active](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:621>) | 621 | — |
| [TrimCalibrator.can_start](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:626>) | 626 | — |
| [TrimCalibrator._vs_err](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:655>) | 655 | — |
| [TrimCalibrator.start](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:666>) | 666 | — |
| [TrimCalibrator.begin_sweep](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:704>) | 704 | — |
| [TrimCalibrator.set_gain_scale](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:711>) | 711 | — |
| [TrimCalibrator.stop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:725>) | 725 | — |
| [TrimCalibrator.force_abort](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:735>) | 735 | — |
| [TrimCalibrator.update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:760>) | 760 | — |
| [TrimCalibrator._set_state](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:825>) | 825 | — |
| [TrimCalibrator._track_throttle](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:834>) | 834 | — |
| [TrimCalibrator._thr_context](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:877>) | 877 | — |
| [TrimCalibrator._log_trim_method](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:894>) | 894 | — |
| [TrimCalibrator._trace_frame](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:900>) | 900 | — |
| [TrimCalibrator._dump_trace](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:940>) | 940 | — |
| [TrimCalibrator._ap_engaged](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:967>) | 967 | — |
| [TrimCalibrator._precheck](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:974>) | 974 | — |
| [TrimCalibrator._capture_takeover_baseline](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1016>) | 1016 | — |
| [TrimCalibrator._probe_value](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1052>) | 1052 | — |
| [TrimCalibrator._do_probe](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1055>) | 1055 | — |
| [TrimCalibrator._advance_probe_stage](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1179>) | 1179 | — |
| [TrimCalibrator._enter_stabilize](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1197>) | 1197 | — |
| [TrimCalibrator._do_stabilize](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1206>) | 1206 | — |
| [TrimCalibrator._enter_trim_neutral](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1251>) | 1251 | — |
| [TrimCalibrator._begin_neut_step](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1275>) | 1275 | — |
| [TrimCalibrator._do_trim_neutral](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1281>) | 1281 | — |
| [TrimCalibrator._detect_overshoot_cycle](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1376>) | 1376 | — |
| [TrimCalibrator._inert_trim_reason](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1406>) | 1406 | — |
| [TrimCalibrator._neutral_give_up](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1414>) | 1414 | — |
| [TrimCalibrator._finish_neutral](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1454>) | 1454 | — |
| [TrimCalibrator._enter_assist_hold](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1479>) | 1479 | — |
| [TrimCalibrator._do_assist_hold](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1512>) | 1512 | — |
| [TrimCalibrator._assist_mean_load](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1853>) | 1853 | — |
| [TrimCalibrator._assist_seq_flip_check](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1917>) | 1917 | — |
| [TrimCalibrator._assist_step_from](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1957>) | 1957 | — |
| [TrimCalibrator._enter_speed_settle](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1975>) | 1975 | — |
| [TrimCalibrator._do_speed_settle](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1981>) | 1981 | — |
| [TrimCalibrator._enter_sweep](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:1996>) | 1996 | — |
| [TrimCalibrator._begin_step](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2028>) | 2028 | — |
| [TrimCalibrator._next_station_target](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2038>) | 2038 | — |
| [TrimCalibrator._advance_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2085>) | 2085 | — |
| [TrimCalibrator._do_sweep](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2143>) | 2143 | — |
| [TrimCalibrator._accept_station](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2264>) | 2264 | — |
| [TrimCalibrator._finish_station_on_deadline](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2319>) | 2319 | — |
| [TrimCalibrator._solve](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2360>) | 2360 | — |
| [TrimCalibrator._enter_restore](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2454>) | 2454 | — |
| [TrimCalibrator._do_restore](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2458>) | 2458 | — |
| [TrimCalibrator._walk_hold_toward](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2480>) | 2480 | — |
| [TrimCalibrator._release_hold_targets](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2495>) | 2495 | — |
| [TrimCalibrator._fit](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2521>) | 2521 | — |
| [TrimCalibrator._run_leveling](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2541>) | 2541 | — |
| [TrimCalibrator._run_pitch_loop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2558>) | 2558 | — |
| [TrimCalibrator._engage_pitch_loop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2593>) | 2593 | — |
| [TrimCalibrator._set_pitch_gain_scale](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2612>) | 2612 | — |
| [TrimCalibrator._watch_oscillation](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2619>) | 2619 | — |
| [TrimCalibrator._backoff_pitch_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2684>) | 2684 | — |
| [TrimCalibrator._command_axes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2706>) | 2706 | — |
| [TrimCalibrator._hold_stick_centered](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2735>) | 2735 | setCondition, set_coefficient, set_offset |
| [TrimCalibrator._set_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2765>) | 2765 | — |
| [TrimCalibrator._abort](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2852>) | 2852 | — |
| [TrimCalibrator._finish](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2858>) | 2858 | — |
| [TrimCalibrator._set_trimwheel_hold](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TrimCalibrator.py:2876>) | 2876 | — |

## [telemffb/sim/msfs_xp/TurbopropAircraft.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbopropAircraft.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [TurbopropAircraft.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbopropAircraft.py:26>) | 26 | — |
| [TurbopropAircraft.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbopropAircraft.py:34>) | 34 | — |

## [telemffb/sim/msfs_xp/TurbulenceMixIn.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbulenceMixIn.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [TurbulenceMixIn.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbulenceMixIn.py:23>) | 23 | — |
| [TurbulenceMixIn.update_turbulence](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbulenceMixIn.py:27>) | 27 | — |
| [TurbulenceMixIn._apply_joystick_turbulence](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbulenceMixIn.py:62>) | 62 | constant |
| [TurbulenceMixIn._apply_pedal_turbulence](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbulenceMixIn.py:80>) | 80 | constant |
| [TurbulenceMixIn.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/TurbulenceMixIn.py:93>) | 93 | — |

## [telemffb/sim/msfs_xp/XAW109Helicopter.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/XAW109Helicopter.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [XAW109Helicopter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/XAW109Helicopter.py:57>) | 57 | — |
| [XAW109Helicopter.on_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/XAW109Helicopter.py:64>) | 64 | — |
| [XAW109Helicopter.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/XAW109Helicopter.py:68>) | 68 | — |
| [XAW109Helicopter.msfs_update_heli_controls](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/XAW109Helicopter.py:71>) | 71 | setCondition, set_offset |
| [XAW109Helicopter._update_cyclic_trim](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/XAW109Helicopter.py:101>) | 101 | — |
| [XAW109Helicopter.msfs_update_pedals](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/XAW109Helicopter.py:133>) | 133 | setCondition, set_offset |
| [XAW109Helicopter.msfs_update_collective](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/XAW109Helicopter.py:237>) | 237 | setCondition, set_coefficient, set_offset |

## [telemffb/telem/SimConnectManager.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [SimVar.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:142>) | 142 | — |
| [SimVar._build_scale_transform](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:172>) | 172 | — |
| [SimVar._calculate](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:186>) | 186 | — |
| [SimVar.__repr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:206>) | 206 | — |
| [SimVar.c_type](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:216>) | 216 | — |
| [SimVarArray.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:254>) | 254 | — |
| [SimVarArray.clone](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:297>) | 297 | — |
| [is_input_event](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:312>) | 312 | — |
| [SimConnectManager.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:460>) | 460 | — |
| [SimConnectManager.add_simvar](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:514>) | 514 | — |
| [SimConnectManager.remove_simvar](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:544>) | 544 | — |
| [SimConnectManager.has_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:550>) | 550 | — |
| [SimConnectManager.clear_runtime_simvars](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:555>) | 555 | — |
| [SimConnectManager.substitute_simvars](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:563>) | 563 | — |
| [SimConnectManager._subscribe](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:640>) | 640 | — |
| [SimConnectManager._resubscribe](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:708>) | 708 | — |
| [SimConnectManager.set_simdatum_to_msfs](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:717>) | 717 | — |
| [SimConnectManager.send_event_to_msfs](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:728>) | 728 | — |
| [SimConnectManager.tx_simdatums_to_msfs](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:742>) | 742 | — |
| [SimConnectManager.tx_events_to_msfs](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:761>) | 761 | — |
| [SimConnectManager._note_sent_datum](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:785>) | 785 | — |
| [SimConnectManager._exception_detail](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:795>) | 795 | — |
| [SimConnectManager._exception_name](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:800>) | 800 | — |
| [SimConnectManager._reset_input_events](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:812>) | 812 | — |
| [SimConnectManager._note_aircraft_title](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:828>) | 828 | — |
| [SimConnectManager._drop_input_event_subscriptions](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:850>) | 850 | — |
| [SimConnectManager._sync_input_events](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:859>) | 859 | — |
| [SimConnectManager._request_input_event_enumeration](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:891>) | 891 | — |
| [SimConnectManager._tick_input_events](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:910>) | 910 | — |
| [SimConnectManager._on_input_events_enumerated](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:925>) | 925 | — |
| [SimConnectManager._log_flight_control_input_events](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:963>) | 963 | — |
| [SimConnectManager._resolve_input_events](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:985>) | 985 | — |
| [SimConnectManager._warn_unresolved](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1015>) | 1015 | — |
| [SimConnectManager._store_input_event_value](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1021>) | 1021 | — |
| [SimConnectManager._set_input_event](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1033>) | 1033 | — |
| [SimConnectManager._flush_input_event_writes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1055>) | 1055 | — |
| [SimConnectManager._handle_recv](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1063>) | 1063 | — |
| [SimConnectManager._read_telem](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1170>) | 1170 | — |
| [SimConnectManager.emit_packet](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1217>) | 1217 | — |
| [SimConnectManager.emit_event](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1229>) | 1229 | — |
| [SimConnectManager.quit](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1242>) | 1242 | — |
| [SimConnectManager.run](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1251>) | 1251 | — |
| [SimConnectManager.get_var_name](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1289>) | 1289 | — |
| [SimConnectTest.emit_packet](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/SimConnectManager.py:1315>) | 1315 | — |

## [telemffb/telem/TelemManager.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [trim_cal_applies](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:68>) | 68 | — |
| [aircraft_module_for_source](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:89>) | 89 | — |
| [resolve_aircraft_config](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:107>) | 107 | — |
| [build_aircraft](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:158>) | 158 | — |
| [config_has_changed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:223>) | 223 | — |
| [force_config_change](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:251>) | 251 | — |
| [TelemManager.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:286>) | 286 | — |
| [TelemManager.set_paused](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:323>) | 323 | — |
| [TelemManager.frame_hold](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:326>) | 326 | — |
| [TelemManager.reset_sim_connected](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:343>) | 343 | — |
| [TelemManager.gain_overrides_active](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:353>) | 353 | — |
| [TelemManager.gain_overrides_active](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:357>) | 357 | — |
| [TelemManager.notify_sim_exited](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:365>) | 365 | — |
| [TelemManager._retire_current_aircraft](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:401>) | 401 | — |
| [TelemManager.on_shutdown](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:423>) | 423 | — |
| [TelemManager.set_simconnect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:434>) | 434 | — |
| [TelemManager.simconnect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:438>) | 438 | — |
| [TelemManager.refresh_aircraft_profile](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:441>) | 441 | — |
| [TelemManager.get_aircraft_config](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:460>) | 460 | — |
| [TelemManager._note_profile_change](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:492>) | 492 | — |
| [TelemManager.recheck_profile_offer](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:506>) | 506 | — |
| [TelemManager._collision_offer](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:519>) | 519 | — |
| [TelemManager.quit](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:538>) | 538 | — |
| [TelemManager.flush_deferred_startup_frame](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:542>) | 542 | — |
| [TelemManager.request_configurator_override_reapply](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:561>) | 561 | — |
| [TelemManager._reapply_configurator_overrides](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:581>) | 581 | — |
| [TelemManager.submit_frame](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:597>) | 597 | — |
| [TelemManager.process_events](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:640>) | 640 | — |
| [TelemManager.get_changed_params](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:659>) | 659 | — |
| [TelemManager.process_data](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:670>) | 670 | — |
| [TelemManager._parse_telemetry_data](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:686>) | 686 | — |
| [TelemManager._update_frame_timing](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:709>) | 709 | — |
| [TelemManager._merge_ipc_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:730>) | 730 | — |
| [TelemManager._extract_aircraft_info](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:738>) | 738 | — |
| [TelemManager._handle_aircraft_changes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:761>) | 761 | — |
| [TelemManager._initialize_new_aircraft](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:771>) | 771 | — |
| [TelemManager._handle_device_selection](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:808>) | 808 | — |
| [TelemManager._stamp_trim_cal_availability](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:854>) | 854 | — |
| [TelemManager._resolve_aircraft_class](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:872>) | 872 | — |
| [TelemManager._device_has_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:884>) | 884 | — |
| [TelemManager._handle_vpconf_setup](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:890>) | 890 | — |
| [TelemManager._handle_global_vpconf_default](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:918>) | 918 | — |
| [TelemManager._handle_command_runner](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:934>) | 934 | — |
| [TelemManager._handle_configurator_overrides](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:944>) | 944 | — |
| [TelemManager._current_class_name](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:981>) | 981 | — |
| [TelemManager._setup_simconnect_overrides](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:989>) | 989 | — |
| [TelemManager._setup_xpplugin_overrides](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1001>) | 1001 | — |
| [TelemManager._xplane_conversion](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1014>) | 1014 | — |
| [TelemManager._handle_config_changes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1033>) | 1033 | — |
| [TelemManager._handle_configurator_overrides_update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1061>) | 1061 | — |
| [TelemManager._recreate_aircraft_with_new_type](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1068>) | 1068 | — |
| [TelemManager._process_current_aircraft_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1082>) | 1082 | — |
| [TelemManager._handle_ipc_and_plotting](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1109>) | 1109 | — |
| [TelemManager._emit_telemetry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1127>) | 1127 | — |
| [TelemManager.resolve_aircraft_class_from_sc](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1134>) | 1134 | — |
| [TelemManager.getTelemValue](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1194>) | 1194 | — |
| [TelemManager.force_config_change](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1198>) | 1198 | — |
| [TelemManager.on_timeout](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1205>) | 1205 | — |
| [TelemManager._check_sim_process](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1242>) | 1242 | — |
| [TelemManager._safe_call](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1286>) | 1286 | — |
| [TelemManager.run](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/telem/TelemManager.py:1301>) | 1301 | — |

## [telemffb/utils/TurbulenceModulator.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/TurbulenceModulator.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [TurbulenceModulator.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/TurbulenceModulator.py:63>) | 63 | — |
| [TurbulenceModulator.update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/TurbulenceModulator.py:75>) | 75 | — |

## [telemffb/utils/_math.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [Interp1D.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:66>) | 66 | — |
| [Interp1D.__call__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:75>) | 75 | — |
| [Akima1DInterpolator.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:103>) | 103 | — |
| [Akima1DInterpolator.__call__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:117>) | 117 | — |
| [Smoother.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:131>) | 131 | — |
| [Smoother.get_average](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:134>) | 134 | — |
| [Smoother.get_rolling_average](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:147>) | 147 | — |
| [mix](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:167>) | 167 | — |
| [convert_between_units](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:188>) | 188 | — |
| [to_number](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:204>) | 204 | — |
| [clamp](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:232>) | 232 | — |
| [clamp_minmax](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:236>) | 236 | — |
| [scale](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:240>) | 240 | — |
| [scale_clamp](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:255>) | 255 | — |
| [piecewise_linear](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:264>) | 264 | — |
| [_parse_trim_curve_entry](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:285>) | 285 | — |
| [parse_trim_follow_family](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:321>) | 321 | — |
| [parse_trim_follow_curve](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:395>) | 395 | — |
| [suggest_calibration_speeds](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:406>) | 406 | — |
| [trim_follow_blend](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:488>) | 488 | — |
| [non_linear_scaling](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:521>) | 521 | — |
| [gaussian_scaling](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:536>) | 536 | — |
| [sine_point_in_time](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:556>) | 556 | — |
| [interpolate_curve_y_point](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:575>) | 575 | — |
| [get_gain_from_gs](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:605>) | 605 | — |
| [get_gain_from_speed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:624>) | 624 | — |
| [pressure_from_altitude](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:668>) | 668 | — |
| [average](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:680>) | 680 | — |
| [polar_to_cartesian_deg](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:686>) | 686 | — |
| [add_vectors_deg](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:693>) | 693 | — |
| [LowPassFilter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:708>) | 708 | — |
| [LowPassFilter.__call__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:714>) | 714 | — |
| [LowPassFilter.update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:717>) | 717 | — |
| [LowPassFilter.value](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:727>) | 727 | — |
| [HighPassFilter.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:732>) | 732 | — |
| [HighPassFilter.__call__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:739>) | 739 | — |
| [HighPassFilter.update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:742>) | 742 | — |
| [HighPassFilter.reset](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:756>) | 756 | — |
| [Derivative.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:761>) | 761 | — |
| [Derivative.update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:770>) | 770 | — |
| [Derivative.dampen_value](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:783>) | 783 | — |
| [Dampener.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:807>) | 807 | — |
| [Dampener.update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:811>) | 811 | — |
| [PID.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:840>) | 840 | — |
| [PID.reset](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:851>) | 851 | — |
| [PID.set_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:857>) | 857 | — |
| [PID.update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:874>) | 874 | — |
| [RandomDirectionModulator.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:916>) | 916 | — |
| [RandomDirectionModulator.update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:921>) | 921 | — |
| [expocurve](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/_math.py:932>) | 932 | — |

## [telemffb/utils/conversions.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/conversions.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [to_device_units](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/utils/conversions.py:45>) | 45 | — |

## [telemffb/hw/ffb_rhino.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [_next_reconnect_delay](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:56>) | 56 | — |
| [BaseStructure.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:173>) | 173 | — |
| [BaseStructure.__repr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:186>) | 186 | — |
| [FFBReport_SetCondition.set_offset](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:301>) | 301 | — |
| [FFBReport_SetCondition.set_saturation](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:330>) | 330 | — |
| [FFBReport_SetCondition.set_coefficient](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:350>) | 350 | — |
| [FFBReport_SetCondition.set_coefficients](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:371>) | 371 | — |
| [FFBReport_Input.isButtonPressed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:469>) | 469 | — |
| [FFBReport_Input.getPressedButtons](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:502>) | 502 | — |
| [FFBReport_Input.buttons](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:530>) | 530 | — |
| [FFBReport_Input.axisXY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:543>) | 543 | — |
| [FFBReport_Input.rawAxisXY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:549>) | 549 | — |
| [FFBReport_Input.axisOverrideActive](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:555>) | 555 | — |
| [FFBReport_Input.CP_XY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:561>) | 561 | — |
| [FFBReport_Input.forceXY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:571>) | 571 | — |
| [FFBReport_Input.CP_scaled_axisXY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:577>) | 577 | — |
| [FFBEffectHandle.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:664>) | 664 | — |
| [FFBEffectHandle.invalidate](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:674>) | 674 | — |
| [FFBEffectHandle._device_alive](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:681>) | 681 | — |
| [FFBEffectHandle._data_changed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:692>) | 692 | — |
| [FFBEffectHandle.forget_playback](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:708>) | 708 | — |
| [FFBEffectHandle._write](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:712>) | 712 | — |
| [FFBEffectHandle.__del__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:725>) | 725 | — |
| [FFBEffectHandle.__bool__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:728>) | 728 | — |
| [FFBEffectHandle.started](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:733>) | 733 | — |
| [FFBEffectHandle.name](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:737>) | 737 | — |
| [FFBEffectHandle.__repr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:740>) | 740 | — |
| [FFBEffectHandle.start](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:743>) | 743 | — |
| [FFBEffectHandle.stop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:752>) | 752 | — |
| [FFBEffectHandle.destroy](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:767>) | 767 | — |
| [FFBEffectHandle.setConstantForce](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:781>) | 781 | — |
| [FFBEffectHandle.setEffect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:820>) | 820 | — |
| [FFBEffectHandle.setCondition](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:833>) | 833 | — |
| [FFBEffectHandle.setEnvelope](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:853>) | 853 | — |
| [FFBEffectHandle.setPeriodic](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:864>) | 864 | — |
| [DeviceInfo.vidpid](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:907>) | 907 | — |
| [DeviceInfo.ident](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:912>) | 912 | — |
| [FFBRhino.caps](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:922>) | 922 | — |
| [FFBRhino.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:925>) | 925 | — |
| [FFBRhino.reconnect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:962>) | 962 | — |
| [FFBRhino.shutdown](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:993>) | 993 | — |
| [FFBRhino.connected](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1015>) | 1015 | — |
| [FFBRhino.serial](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1024>) | 1024 | — |
| [FFBRhino.product](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1029>) | 1029 | — |
| [FFBRhino.manufacturer](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1034>) | 1034 | — |
| [FFBRhino.enumerate](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1040>) | 1040 | — |
| [FFBRhino.get_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1058>) | 1058 | — |
| [FFBRhino.set_gain](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1066>) | 1066 | — |
| [FFBRhino.set_deadzone](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1077>) | 1077 | — |
| [FFBRhino.supports_axis_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1090>) | 1090 | — |
| [FFBRhino.send_axis_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1094>) | 1094 | — |
| [FFBRhino.clear_axis_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1113>) | 1113 | — |
| [FFBRhino._emit_connection_state](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1116>) | 1116 | — |
| [FFBRhino._schedule_reconnect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1128>) | 1128 | — |
| [FFBRhino._try_reconnect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1142>) | 1142 | — |
| [FFBRhino.timerEvent](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1176>) | 1176 | — |
| [FFBRhino._process_hats](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1192>) | 1192 | — |
| [FFBRhino.on_hid_report_received](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1209>) | 1209 | — |
| [FFBRhino.get_firmware_version](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1263>) | 1263 | — |
| [FFBRhino.reset_effects](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1318>) | 1318 | — |
| [FFBRhino.create_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1337>) | 1337 | — |
| [FFBRhino.write](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1355>) | 1355 | — |
| [FFBRhino.pump_input](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1361>) | 1361 | — |
| [FFBRhino.read_reports](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1375>) | 1375 | — |
| [FFBRhino.get_report](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1389>) | 1389 | — |
| [FFBRhino.get_input](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1406>) | 1406 | — |
| [HapticEffect.device_alive](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1427>) | 1427 | — |
| [HapticEffect.get_device_input](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1439>) | 1439 | — |
| [HapticEffect.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1462>) | 1462 | — |
| [HapticEffect.__repr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1484>) | 1484 | — |
| [HapticEffect.id](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1489>) | 1489 | — |
| [HapticEffect.intensity](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1494>) | 1494 | — |
| [HapticEffect.axis_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1501>) | 1501 | — |
| [HapticEffect.type_id](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1507>) | 1507 | — |
| [HapticEffect.open](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1518>) | 1518 | — |
| [HapticEffect.open_dinput](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1541>) | 1541 | — |
| [HapticEffect._ensure_effect_created](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1559>) | 1559 | setEnvelope |
| [HapticEffect.setCondition](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1593>) | 1593 | setCondition |
| [HapticEffect._conditional_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1625>) | 1625 | — |
| [HapticEffect.inertia](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1692>) | 1692 | — |
| [HapticEffect.damper](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1705>) | 1705 | — |
| [HapticEffect.friction](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1718>) | 1718 | — |
| [HapticEffect.spring](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1731>) | 1731 | — |
| [HapticEffect.spring_adjuster](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1744>) | 1744 | — |
| [HapticEffect.detent](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1757>) | 1757 | — |
| [HapticEffect.periodic](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1900>) | 1900 | setPeriodic |
| [HapticEffect.constant](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1950>) | 1950 | setConstantForce |
| [HapticEffect.envelope](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:1989>) | 1989 | setEnvelope |
| [HapticEffect.started](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:2057>) | 2057 | — |
| [HapticEffect.start](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:2061>) | 2061 | — |
| [HapticEffect.stop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:2122>) | 2122 | setEnvelope |
| [HapticEffect.destroy](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:2173>) | 2173 | — |
| [HapticEffect.__del__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:2196>) | 2196 | — |
| [HapticEffect.destroy_all](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_rhino.py:2201>) | 2201 | — |

## [telemffb/hw/ffb_dinput.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [_trace_enabled](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:161>) | 161 | — |
| [_installed_dll_path](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:182>) | 182 | — |
| [_describe_params](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:218>) | 218 | — |
| [shared_bridge](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:249>) | 249 | — |
| [bridge_availability](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:257>) | 257 | — |
| [_version_tuple](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:323>) | 323 | — |
| [version_is_at_least](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:339>) | 339 | — |
| [bridge_status](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:389>) | 389 | — |
| [axis_setting_key](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:466>) | 466 | — |
| [invert_setting_key](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:472>) | 472 | — |
| [resolve_axis_map](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:479>) | 479 | — |
| [DIBridge.installed_location](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:525>) | 525 | — |
| [DIBridge.library_paths](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:535>) | 535 | — |
| [DIBridge._load_library](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:573>) | 573 | — |
| [DIBridge.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:592>) | 592 | — |
| [DIBridge._read_build_info](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:605>) | 605 | — |
| [DIBridge._log_build_identity](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:626>) | 626 | — |
| [DIBridge.last_error](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:672>) | 672 | — |
| [DIBridge.enumerate](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:677>) | 677 | — |
| [DIBridge.open](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:684>) | 684 | — |
| [DIBridge.release](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:690>) | 690 | — |
| [DIBridge.autocenter_state](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:699>) | 699 | — |
| [DIBridge.ffb_axes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:712>) | 712 | — |
| [DIBridge.query_ffb_axes](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:720>) | 720 | — |
| [DIBridge._axis_names](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:731>) | 731 | — |
| [DIBridge.set_axis_map](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:737>) | 737 | — |
| [DIBridge.device_reset](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:748>) | 748 | — |
| [DIBridge.poll](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:760>) | 760 | — |
| [DIBridge.effect_create](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:767>) | 767 | — |
| [DIBridge.effect_update](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:777>) | 777 | — |
| [DIBridge.effect_start](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:785>) | 785 | — |
| [DIBridge.effect_stop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:791>) | 791 | — |
| [DIBridge.effect_destroy](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:796>) | 796 | — |
| [DIDeviceInfo.vidpid](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:820>) | 820 | — |
| [DIDeviceInfo.ident](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:824>) | 824 | — |
| [DInputInputSnapshot.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:835>) | 835 | — |
| [DInputInputSnapshot.buttons](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:844>) | 844 | — |
| [DInputInputSnapshot.isButtonPressed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:847>) | 847 | — |
| [DInputInputSnapshot.getPressedButtons](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:858>) | 858 | — |
| [DInputInputSnapshot.axisXY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:866>) | 866 | — |
| [DInputInputSnapshot.rawAxisXY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:869>) | 869 | — |
| [DInputInputSnapshot.axisOverrideActive](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:873>) | 873 | — |
| [DInputInputSnapshot.CP_XY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:876>) | 876 | — |
| [DInputInputSnapshot.forceXY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:879>) | 879 | — |
| [DInputInputSnapshot.CP_scaled_axisXY](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:883>) | 883 | — |
| [DInputEffectHandle.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:910>) | 910 | — |
| [DInputEffectHandle.__bool__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:927>) | 927 | — |
| [DInputEffectHandle.__del__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:930>) | 930 | — |
| [DInputEffectHandle.name](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:934>) | 934 | — |
| [DInputEffectHandle.__repr__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:937>) | 937 | — |
| [DInputEffectHandle.started](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:941>) | 941 | — |
| [DInputEffectHandle.invalidate](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:948>) | 948 | — |
| [DInputEffectHandle.forget_playback](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:957>) | 957 | — |
| [DInputEffectHandle._note_call_failed](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:967>) | 967 | — |
| [DInputEffectHandle._push](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:989>) | 989 | — |
| [DInputEffectHandle._is_zero_condition](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1012>) | 1012 | — |
| [DInputEffectHandle._sync_device_playing](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1021>) | 1021 | — |
| [DInputEffectHandle.start](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1053>) | 1053 | — |
| [DInputEffectHandle.stop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1081>) | 1081 | — |
| [DInputEffectHandle.destroy](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1088>) | 1088 | — |
| [DInputEffectHandle.setEffect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1097>) | 1097 | — |
| [DInputEffectHandle.setCondition](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1106>) | 1106 | — |
| [DInputEffectHandle.setConstantForce](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1134>) | 1134 | — |
| [DInputEffectHandle.setPeriodic](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1146>) | 1146 | — |
| [DInputEffectHandle.setEnvelope](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1162>) | 1162 | — |
| [_default_params](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1171>) | 1171 | — |
| [DInputFFBDevice.caps](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1199>) | 1199 | — |
| [DInputFFBDevice.connected](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1203>) | 1203 | — |
| [DInputFFBDevice.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1208>) | 1208 | — |
| [DInputFFBDevice._desired_axis_map](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1249>) | 1249 | — |
| [DInputFFBDevice._apply_axis_map](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1291>) | 1291 | — |
| [DInputFFBDevice.request_axis_map_reapply](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1325>) | 1325 | — |
| [DInputFFBDevice._log_autocenter_state](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1334>) | 1334 | — |
| [DInputFFBDevice._find_info](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1360>) | 1360 | — |
| [DInputFFBDevice.enumerate](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1378>) | 1378 | — |
| [DInputFFBDevice.serial](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1393>) | 1393 | — |
| [DInputFFBDevice.product](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1397>) | 1397 | — |
| [DInputFFBDevice.manufacturer](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1401>) | 1401 | — |
| [DInputFFBDevice.timerEvent](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1407>) | 1407 | — |
| [DInputFFBDevice.pump_input](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1422>) | 1422 | — |
| [DInputFFBDevice._poll_once](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1429>) | 1429 | — |
| [DInputFFBDevice._process_buttons](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1435>) | 1435 | — |
| [DInputFFBDevice._process_hats](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1451>) | 1451 | — |
| [DInputFFBDevice.shutdown](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1468>) | 1468 | — |
| [DInputFFBDevice.recover_effects](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1495>) | 1495 | — |
| [DInputFFBDevice._reset_and_invalidate](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1512>) | 1512 | — |
| [DInputFFBDevice._invalidate_effects](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1527>) | 1527 | — |
| [DInputFFBDevice.note_priority_loss](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1536>) | 1536 | — |
| [DInputFFBDevice._begin_reconnect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1563>) | 1563 | — |
| [DInputFFBDevice._try_reconnect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1574>) | 1574 | — |
| [DInputFFBDevice.note_spring_condition](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1593>) | 1593 | — |
| [DInputFFBDevice._cp_for_axis](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1597>) | 1597 | — |
| [DInputFFBDevice.get_input](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1603>) | 1603 | — |
| [DInputFFBDevice.note_effect_started](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1614>) | 1614 | — |
| [DInputFFBDevice._evict_one_periodic](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1618>) | 1618 | — |
| [DInputFFBDevice.create_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1634>) | 1634 | — |
| [DInputFFBDevice.reset_effects](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_dinput.py:1696>) | 1696 | — |

## [telemffb/hw/ffb_backend.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [BaseEffectHandle.__init__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:94>) | 94 | — |
| [BaseEffectHandle.started](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:99>) | 99 | — |
| [BaseEffectHandle.invalidate](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:102>) | 102 | — |
| [BaseEffectHandle.forget_playback](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:107>) | 107 | — |
| [BaseEffectHandle.start](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:114>) | 114 | — |
| [BaseEffectHandle.stop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:117>) | 117 | — |
| [BaseEffectHandle.destroy](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:120>) | 120 | — |
| [BaseEffectHandle.setEffect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:123>) | 123 | — |
| [BaseEffectHandle.setCondition](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:126>) | 126 | — |
| [BaseEffectHandle.setPeriodic](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:131>) | 131 | — |
| [BaseEffectHandle.setConstantForce](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:134>) | 134 | — |
| [BaseEffectHandle.setEnvelope](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:137>) | 137 | — |
| [BaseEffectHandle.intensity](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:141>) | 141 | — |
| [BaseEffectHandle.axis_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:153>) | 153 | — |
| [BaseFFBDevice.caps](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:181>) | 181 | — |
| [BaseFFBDevice.connected](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:185>) | 185 | — |
| [BaseFFBDevice.serial](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:199>) | 199 | — |
| [BaseFFBDevice.create_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:202>) | 202 | — |
| [BaseFFBDevice.get_input](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:207>) | 207 | — |
| [BaseFFBDevice.pump_input](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:212>) | 212 | — |
| [BaseFFBDevice.reset_effects](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:225>) | 225 | — |
| [BaseFFBDevice.supports_axis_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:228>) | 228 | — |
| [BaseFFBDevice.set_deadzone](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:232>) | 232 | — |
| [BaseFFBDevice.get_gains](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:235>) | 235 | — |
| [BaseFFBDevice.set_gain](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:238>) | 238 | — |
| [BaseFFBDevice.send_axis_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:241>) | 241 | — |
| [BaseFFBDevice.clear_axis_override](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:244>) | 244 | — |
| [BaseFFBDevice.get_firmware_version](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_backend.py:247>) | 247 | — |

## [telemffb/hw/ffb_sdl.py](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_sdl.py:1>)

| Función | Línea | Operaciones directas sobre efectos |
|---|---:|---|
| [HapticEffectSDL._create_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_sdl.py:104>) | 104 | — |
| [HapticEffectSDL._update_effect](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_sdl.py:112>) | 112 | — |
| [HapticEffectSDL.periodic](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_sdl.py:123>) | 123 | — |
| [HapticEffectSDL.constant](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_sdl.py:141>) | 141 | — |
| [HapticEffectSDL.status](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_sdl.py:168>) | 168 | — |
| [HapticEffectSDL.start](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_sdl.py:172>) | 172 | — |
| [HapticEffectSDL.stop](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_sdl.py:179>) | 179 | — |
| [HapticEffectSDL.__del__](<D:/LUCAS/UTN/PROYECTO FINAL/PROYECTO ALEMAN/PROYECTO MODIFICADO/MiYoke_PD_V2.0/VPforce-TelemFFB-refactor/telemffb/hw/ffb_sdl.py:185>) | 185 | — |

