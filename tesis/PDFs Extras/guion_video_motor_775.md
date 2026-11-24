# Guion de video: Caracterización del motor DC 775

**Fuente:** `caracterizacion_motor_775.tex`
**Duración estimada:** 15 a 17 minutos (a unas 130 palabras por minuto de narración)
**Formato:** 16:9, 1920 × 1080
**Público:** quien vaya a hacer los ensayos y quiera entender el modelo antes de entrar al banco.

## Convenciones del guion

- **IMG-xx**: imagen ilustrativa que se genera con ChatGPT (DALL·E). Los prompts están en la sección final.
- **GRAF-xx**: gráfico exacto (fórmula, tabla, curva o diagrama). **No se le pide a DALL·E**, porque escribe mal texto, números y fórmulas. Se arma en PowerPoint, Canva o en el editor de video como texto superpuesto (o se puede generar con un script).
- **Texto en pantalla**: título o dato corto que aparece sobre la imagen.
- Todos los números son los **valores de ejemplo (supuestos)** del documento. El video lo aclara al empezar.

---

## Escena 0: Apertura (0:00 – 0:30)

| Audio | Visual |
|---|---|
| ¿Cuánta fuerza hace un motor por cada ampere que le damos? Parece una pregunta simple, pero de esa respuesta depende que un yugo de simulador de vuelo se sienta como un avión real o como un juguete. En este video vamos a ver, paso a paso, cómo se mide un motor de corriente continua 775: primero el motor solo, después con su reductora, y al final cómo se convierte todo en la fuerza que siente la mano. | **IMG-01**: motor 775 con reductora planetaria sobre el banco, luz dramática. Zoom lento hacia el motor. Texto en pantalla: «Caracterización del motor DC 775 – Proyecto MiYoke PD». |
| Un aviso antes de empezar: todos los números que aparecen son ejemplos supuestos. Sirven para mostrar cómo se hace la cuenta, no son mediciones reales. | Texto en pantalla sobre fondo oscuro: «Valores de ejemplo: no son mediciones reales». |

## Escena 1: Para qué sirve (0:30 – 1:30)

| Audio | Visual |
|---|---|
| El yugo funciona con control de corriente. El firmware decide qué fuerza tiene que sentir la mano en pitch, o qué torque en roll. Esa fuerza se convierte en una corriente, y un lazo PI ajusta el PWM del driver BTS7960 hasta que por el motor circule exactamente esa corriente. | **IMG-02**: manos de piloto sobre un yugo de simulador frente a una pantalla con la vista de cabina. Después, **GRAF-01**: diagrama de bloques animado que se arma de izquierda a derecha: «Efecto → Fuerza / Torque → Corriente → PI → PWM BTS7960 → Motor». |
| Para hacer esa conversión, y para que el PI sea estable y rápido, necesitamos conocer el motor por dentro. ¿Qué parámetros? La resistencia del bobinado, la caída en las escobillas, la constante de torque, la constante de fuerza contraelectromotriz, el rozamiento, la inercia del rotor, la inductancia y la corriente máxima que soporta sin recalentarse. | **IMG-03**: vista explotada del motor (carcasa, imanes, rotor bobinado, colector, escobillas). Sobre ella aparece **GRAF-02**: lista de parámetros, uno por vez: R, V_esc, K_t, K_e, τ_c y b, J_m, L, I_cont. |
| De todos ellos, hay uno que manda: la constante de torque, K t. Nos dice cuántos newton metro entrega el motor por cada ampere. | **GRAF-02** con K_t resaltado en naranja y agrandado. |

## Escena 2: Por qué medir sin la reductora (1:30 – 2:20)

| Audio | Visual |
|---|---|
| La primera decisión importante: medir el motor sin la caja reductora. La reductora agrega rozamiento, juego entre engranajes y un rendimiento que cambia con la carga y con el sentido de giro. Si medimos todo junto, esos efectos se mezclan y no sabemos qué parte es del motor y qué parte de la reductora. | **IMG-04**: motor y reductora planetaria separados, uno al lado del otro sobre el banco. |
| El plan tiene tres momentos. Medir: los parámetros del motor se miden una sola vez y no cambian al agregar la reductora. Calcular: con la cantidad de dientes de la reductora y el radio de la polea se calcula lo que siente la mano. Y verificar: con el yugo armado, una prueba corta con la celda de carga da la fuerza por ampere real. | **GRAF-03**: línea de tiempo con tres etapas: «1. Medir (motor solo)» → «2. Calcular (reductora y polea)» → «3. Verificar (yugo armado)». Cada una se ilumina cuando la nombra el narrador. |

## Escena 3: El modelo del motor (2:20 – 3:50)

| Audio | Visual |
|---|---|
| Un motor de corriente continua de imanes permanentes se describe con dos ecuaciones. La primera es eléctrica: dice en qué se reparte la tensión que aplicamos en los bornes. Una parte cae en la resistencia del cobre. Otra aparece solo cuando la corriente cambia, por la inductancia. Otra se pierde en el contacto de las escobillas. Y la última es la fuerza contraelectromotriz: la tensión que el propio motor genera al girar. | **GRAF-04**: ecuación eléctrica V = R·i + L·di/dt + V_esc + K_e·ω. Cada término se resalta con un color mientras se nombra, con su etiqueta debajo: «cobre», «solo en cambios», «escobillas», «f.c.e.m.». De fondo, tenue, **IMG-05** (bobina de cobre con líneas de campo). |
| La segunda es mecánica. El torque del motor es K t por la corriente. Ese torque acelera el rotor, vence el rozamiento interno y empuja la carga. El rozamiento tiene una parte constante, de Coulomb, que aparece apenas el eje gira, y una parte viscosa que crece con la velocidad. | **GRAF-05**: ecuación mecánica τ_m = K_t·i ; J_m·dω/dt = K_t·i − τ_f − τ_carga ; τ_f ≈ τ_c·sgn(ω) + b·ω. |
| La clave para medir es elegir situaciones donde estas ecuaciones se simplifican. Con el eje trabado no hay velocidad, y quedan solo la resistencia, las escobillas y K t. Con el motor girando como generador, sin corriente, queda solo K e. Y con el motor girando libre, toda la corriente se gasta en vencer su propio rozamiento. | Pantalla dividida en tres: **IMG-12** (eje trabado), **IMG-13** (taladro haciendo girar el motor), **IMG-14** (motor girando libre). Debajo de cada una, la ecuación simplificada (GRAF): «V = R·i + V_esc ; τ = K_t·i», «V = K_e·ω», «K_t·i₀ = τ_f(ω)». |

## Escena 4: K t igual a K e (3:50 – 4:30)

| Audio | Visual |
|---|---|
| Hay una propiedad muy útil. La potencia eléctrica que el motor convierte en mecánica es K e por la velocidad por la corriente. La potencia mecánica es el torque por la velocidad, o sea K t por la corriente por la velocidad. Como son la misma potencia, en unidades del sistema internacional K t y K e valen lo mismo. | **IMG-06**: balanza de dos platos en equilibrio, en un plato un rayo eléctrico estilizado y en el otro un engranaje girando. Superpuesto, **GRAF-06**: «K_e·ω·i = K_t·i·ω  ⇒  K_t = K_e». |
| Por eso los vamos a medir por separado, con ensayos distintos, y comparar. Si difieren menos de un cinco a diez por ciento, los ensayos están bien hechos. Es nuestra mejor verificación. | Texto en pantalla: «Control cruzado: diferencia < 5–10 %». |

## Escena 5: Sacar pendientes, no puntos (4:30 – 5:00)

| Audio | Visual |
|---|---|
| Casi todos los ensayos terminan igual: medir varios puntos y ajustar una recta. Nunca se calcula con un solo punto. Con varios se promedia el error y se ve enseguida si alguno está mal. En la planilla de cálculo alcanzan tres funciones: pendiente, intersección con el eje y coeficiente R cuadrado, que tiene que dar más de 0,99. | **GRAF-07**: gráfico de dispersión con cinco puntos y la recta ajustada; al costado, las fórmulas de planilla: =PENDIENTE(…), =INTERSECCION.EJE(…), =COEFICIENTE.R2(…) > 0,99. Un punto «malo» se aparta y se marca en rojo. |

## Escena 6: Seguridad (5:00 – 5:45)

| Audio | Visual |
|---|---|
| Antes de tocar nada, seguridad. Con una resistencia de medio ohm, veinticuatro volts con el rotor bloqueado darían unos cuarenta y cinco amperes: se queman el bobinado, los cables o el driver. Con el eje trabado se trabaja con uno a cuatro volts y siempre con el límite de corriente de la fuente ajustado de antemano. | **IMG-07**: banco de trabajo con fuente de laboratorio, motor fijado con una morsa, anteojos de seguridad sobre la mesa. Texto en pantalla: «Eje trabado: 1–4 V y límite de corriente». |
| Pocos segundos por punto y pausas, porque el bobinado se calienta mucho más rápido que la carcasa. El motor siempre fijo a la mesa. Anteojos de protección cuando gira. Nunca cortocircuitar los bornes de un motor que gira, porque actúa como generador. Y cuidado con el osciloscopio: el clip de masa va solo al negativo de la fuente. | Lista de íconos que aparece de a uno (GRAF): reloj «pocos segundos», morsa «motor fijo», anteojos, bornes tachados «no cortocircuitar», punta de osciloscopio «masa al negativo». |

## Escena 7: Materiales y preparación (5:45 – 7:00)

| Audio | Visual |
|---|---|
| El banco de ensayo no necesita nada exótico: una fuente regulable de veinticuatro volts, un multímetro, un osciloscopio, un medidor LCR, una celda de carga de veinte kilos con su amplificador HX711, un Arduino y una impresora 3D para fabricar soportes. | **IMG-08**: vista general del banco con todos los instrumentos ordenados. Paneo lento de izquierda a derecha. |
| Con la impresora se hacen la cuna del motor, que lo sostiene con el eje vertical, un brazo de palanca con el punto de apoyo a cien milímetros exactos del eje, la base de la celda y un disco ranurado para el tacómetro. | **IMG-09**: piezas impresas en 3D sobre la mesa: anillo de sujeción, brazo de palanca, base de celda y disco ranurado. |
| La celda de carga entrega una lectura cruda que no está en newtons. Hay que calibrarla con masas conocidas, por ejemplo botellas de agua pesadas en una balanza de cocina, y ajustar una recta. | **IMG-10**: celda de carga horizontal con una botella de agua apoyada en su extremo libre, cables al módulo HX711 y al Arduino. Texto: «F = m · 9,81 ; R² > 0,999». |
| Para la velocidad, un sensor óptico de herradura lee las ranuras del disco: con la frecuencia y la cantidad de ranuras se obtienen las revoluciones por minuto. | **IMG-11**: disco ranurado girando dentro de un sensor óptico de herradura. Texto: «n = 60·f / k». |
| Y una regla de oro: la tensión se mide en los bornes del motor, no en la fuente. Los cables y contactos tienen resistencias comparables a la del motor, y si medimos en la fuente se suman a R. | **GRAF-08**: esquema de conexión (fuente con límite de corriente, amperímetro en serie, motor, voltímetro directamente en los bornes del motor). El voltímetro parpadea en verde; una versión tachada con el voltímetro en la fuente. |

## Escena 8: Ensayo 1, rotor bloqueado (7:00 – 8:30)

| Audio | Visual |
|---|---|
| El primer ensayo es el más importante, porque en una sola prueba da tres parámetros: R, la caída en escobillas y K t. El motor va con el eje vertical, para que el peso del brazo no haga torque. El brazo apoya sobre la celda de carga, perpendicular a ella, a cien milímetros del centro del eje. Un error de un milímetro en esa distancia es un uno por ciento de error en K t. | **IMG-12**: vista superior del motor con el eje vertical, el brazo de palanca horizontal apoyado sobre la celda de carga. Superpuesto (GRAF): cota «L_b = 100 mm» y flecha de fuerza F. |
| Se fija la corriente con el límite de la fuente: uno, dos, tres, cuatro y cinco amperes. En cada punto se anotan la tensión en bornes, la corriente y la fuerza. Después se gira el motor unos treinta grados en su cuna y se repite, en cuatro a seis posiciones, porque las escobillas apoyan en distintos segmentos del colector. | Texto en pantalla que avanza: «1 A → 2 A → 3 A → 4 A → 5 A», luego «Girar 30° y repetir (4–6 posiciones)». |
| Con los datos se ajustan dos rectas. La tensión en función de la corriente da, como pendiente, la resistencia: 0,55 ohm. Y como ordenada al origen, la caída en escobillas: 0,6 volts. El torque en función de la corriente da, como pendiente, K t: 0,0325 newton metro por ampere. | **GRAF-09**: tabla de datos del ejemplo y dos gráficos lado a lado: V vs I (pendiente R = 0,55 Ω, ordenada V_esc = 0,60 V) y τ vs I (pendiente K_t = 0,0325 N·m/A). |
| La resistencia del cobre sube con la temperatura, casi un cuatro por mil por grado: a ciento veinte grados es un treinta y nueve por ciento más alta. Esto también sirve al revés: midiendo R en caliente, sabemos la temperatura del bobinado. | **GRAF-10**: R(T) = R₂₀·[1 + 0,0039·(T − 20 °C)], con una barra que crece «+39 % a 120 °C». |
| Cuidado: cinco amperes con el motor bloqueado son unos catorce watts de calor. No más de cinco segundos por punto. | Banda roja de advertencia: «5 A bloqueado ≈ 14 W: máximo 5 s por punto». |

## Escena 9: Ensayo 2, el motor como generador (8:30 – 9:30)

| Audio | Visual |
|---|---|
| Para K e damos vuelta el problema: en lugar de alimentar el motor, lo hacemos girar desde afuera con un taladro de velocidad regulable. Los bornes quedan abiertos, desconectados de todo salvo del instrumento. Como no circula corriente, ni la resistencia ni las escobillas influyen: la tensión es directamente K e por la velocidad. | **IMG-13**: taladro que sujeta el eje del motor fijado en su cuna, con el osciloscopio midiendo en los bornes. |
| Se mide en cuatro o cinco velocidades, entre mil y tres mil revoluciones por minuto, y en los dos sentidos. La pendiente de la recta da K e igual a 0,0322 volt segundo por radián. Comparado con el K t del ensayo anterior, 0,0325, la diferencia es menor al uno por ciento. Los dos ensayos se confirman entre sí. | **GRAF-11**: recta V vs ω con los tres puntos del ejemplo; luego dos barras casi iguales, «K_t = 0,0325» y «K_e = 0,0322», con el texto «diferencia 0,9 % ✓». |
| Un dato extra: en el osciloscopio, la tensión generada tiene un pequeño rizado cuya frecuencia es proporcional a la velocidad. Una vez calibrado, ese rizado funciona como tacómetro sin disco. | **GRAF-12**: traza de osciloscopio con tensión continua y rizado pequeño, con «f_r = z·n/60». |

## Escena 10: Ensayo 3, giro en vacío y rozamiento (9:30 – 10:20)

| Audio | Visual |
|---|---|
| Ahora el motor gira libre, sin carga, alimentado a seis, doce, dieciocho y veinticuatro volts. Toda la corriente que consume en vacío se gasta en vencer su propio rozamiento. Multiplicándola por K t obtenemos el torque de rozamiento a cada velocidad. Conviene medir con el motor tibio, después de diez minutos girando, porque el rozamiento baja cuando escobillas y rulemanes se calientan. | **IMG-14**: motor girando libre en su cuna, disco del tacómetro desenfocado por la velocidad. Texto: «τ_f = K_t · I₀». |
| La recta del rozamiento en función de la velocidad tiene dos partes. La ordenada es el rozamiento de Coulomb: 0,014 newton metro, que equivale a unos 0,43 amperes que el motor gasta solo para empezar a girar. La pendiente es el rozamiento viscoso, que crece con la velocidad. | **GRAF-13**: recta τ_f vs ω; la ordenada se marca «τ_c ≈ 0,014 N·m (≈ 0,43 A)» y la pendiente «b ≈ 2,4·10⁻⁵ N·m·s/rad». |

## Escena 11: Ensayo 4, inductancia (10:20 – 11:00)

| Audio | Visual |
|---|---|
| La inductancia dice qué tan rápido puede cambiar la corriente. Su cociente con la resistencia es la constante de tiempo eléctrica, que define cuán rápido puede ser el lazo de corriente. Hay dos formas de medirla: directamente con un medidor LCR, en varias posiciones del rotor, o aplicando un escalón de tensión y mirando la corriente en el osciloscopio. | **IMG-15**: medidor LCR con sus pinzas en los bornes del motor. |
| En el escalón, la corriente sube como una exponencial. El tiempo que tarda en llegar al sesenta y tres por ciento del valor final es la constante de tiempo. En el ejemplo, 0,26 milisegundos con 0,65 ohm de resistencia total dan una inductancia de 0,17 milihenry. | **GRAF-14**: curva exponencial i(t) = I_f·(1 − e^(−t/τ)), con línea punteada en 63 % y marca τ = 0,26 ms; luego «L = τ · R_total = 0,17 mH». |

## Escena 12: Ensayo 5, inercia del rotor (11:00 – 11:50)

| Audio | Visual |
|---|---|
| La inercia del rotor importa sobre todo en pitch, y tiene un truco elegante. Se hace girar el motor a veinticuatro volts y se desconecta el cable positivo. A partir de ahí, solo el rozamiento lo frena. Y como el motor con los bornes abiertos es un generador, la tensión en bornes es directamente la velocidad. El osciloscopio registra la desaceleración sin necesidad de tacómetro. | **IMG-16**: motor desacelerando con un volante de arandelas de acero en el eje y el osciloscopio al lado. **GRAF-15**: curva de tensión que cae hasta cero, con una tangente marcada en un punto. |
| Para no depender del modelo de rozamiento, se repite con un volante de inercia conocida. A la misma velocidad el rozamiento es el mismo, así que comparando las dos desaceleraciones sale la inercia del rotor. En el ejemplo, unos 1,9 por diez a la menos cinco kilogramo metro cuadrado. | **GRAF-15b**: J_m = J_v · a₂ / (a₁ − a₂), y el resultado «J_m ≈ 1,9·10⁻⁵ kg·m²». |

## Escena 13: Ensayo 6, límite térmico (11:50 – 12:40)

| Audio | Visual |
|---|---|
| El último ensayo del motor responde una pregunta crítica para un force feedback: ¿cuánta corriente puede circular sin parar? El centrado del yugo mantiene corriente durante mucho tiempo con el eje casi quieto, sin la ventilación que da el giro. | **IMG-17**: motor fotografiado con estética de cámara térmica (bobinado naranja y rojo, carcasa amarilla), con un termómetro infrarrojo apuntándole. |
| Con el eje trabado y corriente constante, cada dos minutos se anotan la temperatura de la carcasa y la tensión en bornes. Con la tensión se recalcula R, y con R la temperatura del bobinado por dentro. Se empieza en dos amperes y se sube de a uno. La corriente continua admisible es la mayor que estabiliza el bobinado por debajo de unos cien grados y la carcasa por debajo de sesenta o setenta. | **GRAF-16**: curvas de temperatura en función del tiempo para 2, 3 y 4 A que se aplanan; línea horizontal roja en 100 °C. Texto: «I_cont = mayor corriente que se estabiliza bajo 100 °C». |
| Si aparece olor a barniz o humo, se corta de inmediato. | Banda roja: «Olor a barniz o humo: cortar». |

## Escena 14: Resumen del motor (12:40 – 13:10)

| Audio | Visual |
|---|---|
| Con los seis ensayos tenemos el modelo completo del motor. Antes de seguir, tres comprobaciones cruzadas: K t parecido a K e, la velocidad en vacío predicha igual a la medida, y la resistencia del LCR del mismo orden que la del ensayo de rotor bloqueado. Si alguna falla, se repite ese ensayo antes de pasar a la reductora. | **GRAF-17**: tabla resumen (R 0,55 Ω; V_esc 0,60 V; K_t 0,0325; K_e 0,0322; τ_c 0,014; L 0,17 mH; J_m 1,9·10⁻⁵; I_cont por medir) y tres tildes verdes que aparecen con cada comprobación. |

## Escena 15: La reductora (13:10 – 14:30)

| Audio | Visual |
|---|---|
| Ahora volvemos a montar la reductora. Una reductora de relación N hace que el eje de salida gire N veces más lento y, sin pérdidas, entregue N veces más torque. Pero hay un detalle que sorprende: la inercia del rotor se multiplica por N al cuadrado. En pitch, con una relación de 19,2, la inercia del rotor se siente unas trescientas setenta veces más grande. Por eso el motor pesa en la mano aunque sea liviano. | **IMG-18**: corte de una reductora planetaria (engranaje sol central, satélites y corona dentada exterior). **GRAF-18**: tabla de transformaciones (ω_s = ω_m/N ; K_s = K_t·N·η ; J_m·N²), con «× 369 en pitch» destacado. |
| La relación real se obtiene contando dientes. En cada etapa planetaria con la corona fija, la relación es uno más los dientes de la corona sobre los del sol. Diez y veintisiete dan 3,7. Con dos etapas, se multiplican. | **GRAF-19**: N_etapa = 1 + Z_c / Z_s ; ejemplo 1 + 27/10 = 3,7. |
| La reductora además pierde torque, y la pérdida depende de quién empuja. Cuando el motor mueve la mano, el rendimiento es el directo. Cuando la mano vence al motor, es el inverso. La diferencia se siente como un rozamiento que cambia con el sentido del movimiento. Esta etapa en banco es opcional: se hace solo si la verificación final en el yugo da algo inesperado. | **GRAF-20**: dos flechas: «Motor empuja: τ_s = η_d·N·K_t·I» y «Mano vence: τ_s = N·K_t·I / η_i». Texto: «Etapa 2: opcional». |
| Para medir ángulos en el eje de salida se usa un encoder magnético AS5600: un imán diametral gira sobre un chip que entrega 4096 posiciones por vuelta. Sirve para medir la relación real, el juego entre engranajes y el radio efectivo de la polea. | **IMG-19**: capuchón impreso con un imán en la punta del eje de salida de la reductora, enfrentado a una placa pequeña con un chip, a un milímetro y medio. |

## Escena 16: De newton metro a la mano (14:30 – 15:40)

| Audio | Visual |
|---|---|
| Llegamos a lo que realmente importa: lo que siente la mano. En roll, la reductora mueve directamente el eje del yugo. K t por la relación 3,7 por el rendimiento da 0,109 newton metro por ampere. Para dos newton metro harían falta dieciocho amperes. Y si el ensayo térmico diera cuatro amperes continuos, el torque sostenido en roll quedaría en 0,44 newton metro. Ese es el dato que decide si la reducción de roll alcanza. | **IMG-20**: mecanismo interno del yugo: eje de roll con su motor, carro de pitch sobre guías con correa dentada y polea. **GRAF-21**: «K_roll = K_t·N_r·η_d = 0,109 N·m/A» ; «2 N·m → 18 A» ; «I_cont 4 A → 0,44 N·m». |
| En pitch, la salida de la reductora mueve una polea de 37,5 milímetros de radio que arrastra la correa del carro. Dividiendo el torque por el radio, obtenemos fuerza: unos catorce newton por ampere. A pitch le sobra fuerza. | **GRAF-22**: «F = τ_s·η_t / r» ; «K_F ≈ 13,9 N/A» ; «50 N → 3,6 A». |
| Pero aparecen dos costos. La inercia del rotor, reflejada por la reductora y la polea, se siente como unos cinco kilos de masa extra en la mano. Y el rozamiento del motor solo ya suma unos siete newton. Por eso el control necesita compensar rozamiento y, si hace falta, inercia. | **IMG-21**: mano sobre la empuñadura del yugo con una pesa de hierro fundido «fantasma», semitransparente, colgando del mango. Texto: «≈ 5 kg de masa equivalente» y «≈ 7 N de rozamiento». |

## Escena 17: Verificación final en el yugo (15:40 – 16:20)

| Audio | Visual |
|---|---|
| Con los motores montados, una sola prueba valida toda la cadena. La celda de carga se fija a la empuñadura, el BTS7960 impone uno, dos y tres amperes, y se mide la fuerza en los dos sentidos. También se mide, con el motor apagado, la fuerza necesaria para mover la empuñadura: el rozamiento total. Las pendientes son la fuerza por ampere real, y de ahí sale el rendimiento verdadero de reductora más correa. Esos valores medidos son los que van al firmware. | **IMG-22**: celda de carga sujeta entre una base fija y la empuñadura del yugo, con el Arduino y el driver en la mesa. **GRAF-23**: «η_total = K_F,d · r / (K_t · N)» ; «< 0,6–0,7 → revisar correa, alineación, lubricación». |

## Escena 18: Cómo se usa en el control (16:20 – 17:00)

| Audio | Visual |
|---|---|
| Cerramos el círculo. El modelo de efectos pide una fuerza; dividiéndola por la fuerza por ampere obtenemos la corriente de referencia, limitada por la corriente continua admisible. Si el eje se mueve, se suma una compensación de rozamiento. La ecuación eléctrica del principio nos da la tensión que haría circular esa corriente: resistencia, escobillas y fuerza contraelectromotriz. Y el PI, con ganancias calculadas a partir de L y R, corrige solo el error que queda. | **GRAF-24**: diagrama de bloques del control: «F_ref → ÷ K_F → I_ref (límite I_cont) → + compensación de rozamiento → V_ff = R·I + V_esc + K_e·N·ω → PI (K_p = L·ω_c ; K_i = R·ω_c) → PWM → motor», con la realimentación de corriente. |
| Cada número que medimos en el banco tiene un lugar en ese lazo. Y así, una pregunta simple, cuánta fuerza por ampere, se convierte en un yugo que se siente como un avión. | **IMG-23**: plano final, cabina de simulador al atardecer con las manos del piloto en el yugo. Fundido a negro con el texto: «Proyecto MiYoke PD». |

---

# Mensaje para ChatGPT (DALL·E)

Copiá y pegá todo lo que sigue en ChatGPT. Está pensado para que genere **una imagen por vez**; cuando termine una, escribile «siguiente».

> Los prompts van en inglés porque DALL·E respeta mejor los detalles así. Todas las imágenes piden **sin texto**: los títulos, números y fórmulas se agregan después en el editor de video (los GRAF-xx).

```text
Hola. Necesito que generes 23 imágenes para un video técnico educativo sobre cómo se miden los parámetros de un motor de corriente continua tipo 775 (motor DC de escobillas, de unos 42 mm de diámetro y 66 mm de largo, con una reductora planetaria cilíndrica) que se usa en un yugo de simulador de vuelo con force feedback.

Reglas para TODAS las imágenes:
- Formato horizontal 16:9 (1792x1024).
- Estilo común: "clean semi-realistic technical illustration, soft studio lighting, dark neutral gray workshop background, accent colors electric blue and warm orange, shallow depth of field, consistent style across the series".
- SIN texto, letras, números, etiquetas ni logos en la imagen.
- Generá una sola imagen por mensaje, con el nombre IMG-xx como título de tu respuesta. Cuando yo escriba "siguiente", pasá a la próxima.
- Si una imagen sale con texto o con algo muy incorrecto, la repito pidiéndote "rehacer IMG-xx".

Lista de prompts (agregá el estilo común a cada uno):

IMG-01: A 775 brushed DC motor with a cylindrical planetary gearbox attached, lying on a dark workbench, dramatic rim lighting, cinematic hero shot, slight low angle.

IMG-02: Close-up of a pilot's hands gripping a home-built flight simulator yoke, a blurred monitor in the background showing an aircraft cockpit view, warm ambient light.

IMG-03: Exploded view of a small brushed DC motor floating in mid-air in a straight line: steel housing, two curved permanent magnets, the wound copper rotor with its commutator, two carbon brushes with springs, end cap and ball bearings. Technical cutaway style.

IMG-04: A 775 DC motor and its detached planetary gearbox placed side by side on a workbench, separated by a small gap, the small sun gear pinion visible on the motor shaft, two screws lying next to them.

IMG-05: Abstract illustration of a glowing copper coil with blue magnetic field lines curving around it and a faint warm glow suggesting heat, dark background.

IMG-06: A classic two-pan balance scale in perfect equilibrium; on one pan a stylized glowing blue lightning bolt, on the other pan a stylized orange spinning gear. Minimalist, symbolic.

IMG-07: Electronics workbench with a benchtop laboratory power supply, a DC motor firmly clamped in a small bench vise, safety glasses lying in front, thick short red and black cables. Safety-focused composition.

IMG-08: Wide shot of a tidy electronics test bench: benchtop power supply, digital multimeter, digital oscilloscope, LCR meter, a small aluminum bar load cell with a small amplifier board, an Arduino Uno board, and a 3D printer in the background.

IMG-09: Set of 3D-printed parts in matte orange PLA laid out on a workbench: a split clamping ring with a screw, a long flat lever arm with a hole at one end and a vertical screw near the other end, a small mounting block, and a thin disc with two radial slots and a central hub.

IMG-10: A horizontal aluminum bar load cell mounted on a printed base, a small plastic water bottle resting on its free end, thin colored wires running to a small amplifier board and an Arduino Uno, kitchen scale nearby.

IMG-11: Macro shot of a thin slotted disc spinning through the gap of a U-shaped (horseshoe) optical slot sensor module, slight motion blur on the disc, small infrared glow inside the sensor gap.

IMG-12: Top-down view of a DC motor mounted with its shaft pointing straight up in a 3D-printed cradle; a long horizontal lever arm is clamped to the shaft, and the screw tip at the end of the arm presses on the face of a horizontal aluminum bar load cell fixed to the bench. Clean, diagram-like composition.

IMG-13: A cordless variable-speed drill whose chuck holds the shaft of a DC motor fixed in a cradle on the bench, the motor terminals connected only by two thin probe wires to a digital oscilloscope showing a flat line.

IMG-14: A DC motor spinning freely on its cradle, a slotted disc on its shaft blurred by high speed, an optical sensor next to it, power supply cables connected, sense of motion.

IMG-15: An LCR meter with its two Kelvin clip leads attached to the two terminals of a stationary DC motor on a workbench, close-up.

IMG-16: A DC motor with a heavy flywheel made of stacked steel washers on a printed hub mounted on its shaft, coasting down, slight motion blur, a digital oscilloscope next to it showing a smoothly falling curve.

IMG-17: Thermal-camera style image of a DC motor on a bench: the winding area glowing red and orange, the housing yellow, the background cool blue and purple; a handheld infrared thermometer pointing at the motor.

IMG-18: Cutaway view of a planetary gearbox: central sun gear, three planet gears on a carrier, and an outer ring gear with internal teeth, metallic steel with light grease, technical illustration.

IMG-19: Close-up of the output shaft of a planetary gearbox with a small 3D-printed cap at its tip holding a small round disc magnet; facing it, about 1.5 mm away, a small square blue sensor circuit board with a tiny chip, held by a printed bridge bracket.

IMG-20: Internal mechanism of a force feedback flight simulator yoke: a roll shaft driven directly by a geared DC motor, and a pitch carriage sliding on two linear rails, pulled by a toothed timing belt around a pulley driven by a second geared DC motor. Isometric technical illustration, no outer casing.

IMG-21: A hand holding the grip of a flight simulator yoke, with a translucent ghostly cast-iron weight hanging from the handle, symbolizing extra perceived mass, subtle blue glow.

IMG-22: A bar load cell clamped between a fixed bench base and the handle of a flight simulator yoke, with a motor driver board and an Arduino on the table beside it, cables neatly routed.

IMG-23: Cinematic final shot: a home flight simulator cockpit at sunset, a pilot's hands on the yoke, large screen showing clouds and the horizon, warm golden light.
```

---

# Cómo armar el video

## 1. Lo que necesitás (todo gratis)

| Para qué | Programa recomendado | Comentario |
|---|---|---|
| Editor de video | **Clipchamp** | Viene instalado en Windows 11 (buscalo en el menú Inicio). Tiene línea de tiempo, texto, transiciones y **voz en off por texto (texto a voz) con voces en español**. Es la opción más simple. |
| Alternativa más completa | DaVinci Resolve o CapCut | Resolve es profesional y gratuito, pero más difícil. CapCut es fácil y tiene voces y subtítulos automáticos. |
| Gráficos exactos (GRAF-xx) | PowerPoint o Canva | Una diapositiva 16:9 por GRAF; PowerPoint escribe bien las fórmulas (Insertar → Ecuación). Exportar como PNG (Archivo → Exportar → PNG, todas las diapositivas). |
| Voz (si no usás la de Clipchamp) | Tu propia voz grabada con el celular o con la Grabadora de sonido de Windows | Una voz real suele quedar mejor que el texto a voz para un video técnico. |

**No necesitás instalar ninguna skill de Claude** para esto. Claude no edita video; el armado se hace en el editor.

## 2. Pasos

1. **Imágenes**: pegá el mensaje en ChatGPT y descargá las 23 imágenes. Guardalas con su nombre (`IMG-01.png`, …).
2. **Gráficos**: armá los GRAF-xx en PowerPoint (una diapositiva cada uno, fondo gris oscuro y los mismos colores azul y naranja) y exportalos como PNG.
3. **Narración**: en Clipchamp, *Grabar y crear → Texto a voz*, elegí una voz en español, pegá el texto de la columna **Audio** de cada escena y generá un archivo por escena. (Si grabás tu voz, una toma por escena.)
4. **Montaje**: en la línea de tiempo, poné el audio de cada escena y encima las imágenes y gráficos que indica la columna **Visual**, ajustando su duración a la del audio.
5. **Movimiento**: a las imágenes fijas aplicales un zoom lento (efecto «Ken Burns»; en Clipchamp, *Efectos → Zoom*) para que no se vea como una presentación.
6. **Texto en pantalla**: los textos cortos indicados en el guion se agregan con la herramienta de texto del editor.
7. **Música de fondo** (opcional): la biblioteca de Clipchamp tiene música libre; volumen bajo (10 a 15 %) para no tapar la voz.
8. **Exportar**: 1080p, MP4.

## 3. Consejos

- DALL·E puede dibujar mal detalles técnicos (cantidad de dientes, conexiones). Las imágenes son **ilustrativas**; lo exacto va en los GRAF.
- Si una escena queda larga, partila en dos imágenes o alterná imagen y gráfico cada 6 a 10 segundos.
- Si el texto a voz pronuncia mal un símbolo, escribilo como se dice («K t», «newton metro por ampere»). En el guion ya están escritos así.
