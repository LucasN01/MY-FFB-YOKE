# Guion de video, parte 2 (práctica): Ensayos del motor DC 775 paso a paso

**Fuente:** `caracterizacion_motor_775.tex`
**Complementa a:** `guion_video_motor_775.md` (parte 1, conceptual). Conviene ver la parte 1 primero.
**Formato:** serie de 11 videos cortos (P0 a P10), 16:9, 1920 × 1080. Duración total: unos 45 minutos.
**Objetivo:** que se puedan hacer los ensayos en el banco **sin tener el instructivo escrito al lado**.

## Convenciones

- **IMG-01 a IMG-23**: imágenes de la parte 1 (se reutilizan).
- **IMG-24 a IMG-42**: imágenes nuevas de esta parte (prompts al final).
- **P2-GRAF-xx**: gráficos exactos (tablas, esquemas, fórmulas). Se arman en PowerPoint o Canva, **no con DALL·E**.
- **Banda roja**: advertencia de seguridad. Conviene dejarla en pantalla al menos 4 segundos.
- **Pausa**: el video sugiere pausarlo para hacer el paso.
- Al final de cada video hay una **tabla a completar**: se puede imprimir desde PowerPoint y usar como planilla en el banco.
- Los números de los ejemplos son **supuestos**, como en la parte 1.

| Video | Tema | Duración |
|---|---|---|
| P0 | Preparar el banco: fuente, conexiones, motores y desarme | 5:00 |
| P1 | Celda de carga, HX711 y programa del Arduino | 4:30 |
| P2 | Tacómetro óptico y encoder AS5600 | 4:00 |
| P3 | Ensayo 1: rotor bloqueado (R, V_esc, K_t) | 5:00 |
| P4 | Ensayo 2: el motor como generador (K_e) | 3:30 |
| P5 | Ensayo 3: giro en vacío y rozamiento | 3:00 |
| P6 | Ensayo 4: inductancia L | 3:30 |
| P7 | Ensayo 5: inercia del rotor J_m | 3:30 |
| P8 | Ensayo 6: límite térmico I_cont | 3:30 |
| P9 | La reductora: relación, rendimiento, rozamiento y juego (opcional) | 5:00 |
| P10 | Cálculo en el yugo, verificación final y hoja de registro | 5:00 |

---

## P0: Preparar el banco (5:00)

**Materiales en pantalla al inicio:** fuente regulable ±24 V, multímetro, cables de al menos 1 mm², cinta de papel y marcador, destornillador, celular para fotos, bolsa para la reductora.

| Audio | Visual |
|---|---|
| Esta es la parte práctica. En este primer video dejamos el banco listo: la fuente protegida, la forma de conectar, los motores identificados y la reductora separada. Hacelo una sola vez y después todos los ensayos arrancan desde acá. | **IMG-08** (banco completo). Título: «P0 – Preparar el banco». |
| Lo primero y más importante es el límite de corriente de la fuente. Con la fuente apagada, uní con un cable los dos bornes de salida, en cortocircuito. Encendé la fuente y subí la perilla de corriente hasta el valor que pide el ensayo: por ejemplo, un ampere. Apagá, sacá el cortocircuito y recién ahí conectá el motor. Desde ese momento, pase lo que pase, la fuente no va a entregar más que ese valor. | **IMG-24**: mano girando la perilla de corriente con los bornes en corto. **P2-GRAF-01**: pasos numerados: «1. Fuente apagada → 2. Bornes en corto → 3. Encender y ajustar perilla de corriente → 4. Apagar y sacar el corto → 5. Conectar el motor». Pausa. |
| Si tu fuente no tiene límite de corriente, poné en serie un fusible del valor máximo del ensayo. Otras opciones son una resistencia de potencia de un ohm y diez watts, una lámpara de auto de doce volts, o un módulo reductor con límite de corriente, tipo XL4015, alimentado desde veinticuatro volts. | Caja verde «Si no tenés…»: fusible, resistencia 1 Ω 10 W, lámpara 12 V, módulo XL4015 CC-CV. |
| Tu fuente es de más y menos veinticuatro volts. Usá un solo canal: entre el borne positivo y el común, de cero a veinticuatro. Nunca conectes el motor entre más veinticuatro y menos veinticuatro, porque serían cuarenta y ocho volts. | **P2-GRAF-02**: esquema de los tres bornes de la fuente (+24, COM, −24); conexión correcta en verde entre +24 y COM; conexión entre +24 y −24 tachada en rojo con «48 V». |
| La conexión de todos los ensayos es esta. El amperímetro va en serie, con el multímetro en la entrada de diez amperes, o leyendo el display de la fuente. Y el voltímetro va directamente en los bornes del motor, nunca en la fuente. Los cables tienen décimas de ohm, igual que el motor: si medís en la fuente, esa resistencia se te suma a R. | **P2-GRAF-03**: esquema fuente → amperímetro en serie → motor, con el voltímetro en los bornes del motor (recreación del diagrama del instructivo). |
| Si tenés un solo multímetro, usalo como voltímetro en los bornes y leé la corriente en el display de la fuente. Pero antes verificá una vez ese display: poné el multímetro en serie y compará a dos y a cuatro amperes. | Texto: «Un solo multímetro: voltímetro en bornes + display de la fuente (verificado a 2 A y 4 A)». |
| Si te falta un amperímetro confiable, una resistencia shunt sirve. Con una de 0,1 ohm y cinco watts, la corriente es la tensión del shunt dividida por 0,1: cinco amperes dan medio volt. También sirve un ACS712 de treinta amperes, que da 66 milivolts por ampere centrado en 2,5 volts; o el pin IS del BTS7960. Cualquiera de los dos se calibra contra el multímetro. | **IMG-31**: shunt en serie con el motor y el multímetro midiendo su tensión. Texto: «I = V_shunt / R_shunt ; 0,1 Ω: 5 A → 0,5 V y 2,5 W». |
| El osciloscopio comparte la masa de su punta con la tierra de la red. El clip de masa va solo al negativo de la fuente del ensayo. Si lo ponés en otro punto, podés cortocircuitar la fuente a través de la tierra. | Banda roja: «Clip de masa del osciloscopio: SOLO al negativo de la fuente». |
| Ahora identificá los motores. Con cinta y marcador: motor R para roll, el de la reductora 3,7, y motor P para pitch, el de la reductora 19,2. Marcá también el borne positivo. Con más en ese borne, el eje gira en sentido positivo: anotá si es horario mirando desde el eje. Cada motor se mide por separado, porque dos motores del mismo lote pueden diferir un diez por ciento. | **IMG-25**: dos motores con tiras de cinta de colores distintos y un marcador. Texto: «Motor R: roll, N 3,7 · Motor P: pitch, N 19,2 · Marcar borne +». |
| Para separar la reductora, sacá primero fotos de cada paso. Aflojá los dos o cuatro tornillos de la tapa frontal y separá la reductora sin girarla ni forzarla. El piñón sol de la primera etapa queda prensado en el eje del motor: no lo saques, se mide con el piñón puesto. Guardá la reductora boca arriba en una bolsa para que no pierda grasa ni junte polvo. Y aprovechá para contar los dientes: lo usamos en el video P9. | **IMG-26**: manos separando la reductora del motor con un destornillador, un celular sacando foto. Lista P2-GRAF-04: «Fotos · Tornillos de la tapa · Separar sin girar · Piñón sol: NO sacar · Reductora boca arriba en bolsa · Contar dientes». |
| Si la reductora no sale sin dañarla, se puede medir todo con ella puesta y referir al motor. Usando la velocidad del motor como N por la de salida, K e sale bien. Pero el torque que medís es K t por N por el rendimiento, no K t solo: en ese caso tomá K t igual a K e y deducí el rendimiento de la comparación. | Caja verde: «Reductora no desmontable: K_e = V / (N·ω_s) ✓ ; torque medido = K_t·N·η → usar K_t ≈ K_e». |
| Por último, las piezas impresas. La cuna abraza la carcasa de unos cuarenta y dos milímetros, medila, con el eje vertical hacia arriba y un tornillo de apriete que permita girar el motor sobre su eje. El brazo de palanca mide ciento veinte milímetros, con el agujero para un acople rígido de cinco milímetros y, a cien milímetros exactos del centro, un tornillo con punta redondeada que apoya sobre la celda. Imprimilo macizo, con cien por ciento de relleno o cinco perímetros. Hacé otro brazo de ciento cincuenta milímetros para el eje de salida de la reductora. Además: la base de la celda, un disco de cuarenta milímetros con dos o cuatro ranuras para el tacómetro, el acople motor con motor y el soporte del AS5600. | **IMG-09** (piezas impresas) y **IMG-28**: calibre midiendo el diámetro del eje (normalmente 5 mm, a veces con plano en D). **P2-GRAF-05**: lista de piezas con medidas clave: cuna Ø ≈ 42 mm eje vertical · brazo 120 mm, apoyo a 100,0 mm, 100 % relleno · brazo de 150 mm · base de celda · disco Ø 40 mm, 2–4 ranuras · acople 5 mm · porta-imán y soporte AS5600. |
| Si no querés imprimir, sirven madera terciada, un perfil de aluminio y una abrazadera de caño. Lo importante es que el brazo no flexione y que midas con calibre la distancia del eje al punto de apoyo. | Caja verde con el texto resumido. |

---

## P1: Celda de carga, HX711 y programa (4:30)

**Materiales:** celda de 20 kg, módulo HX711, Arduino UNO (o Pico), cables Dupont, base impresa, balanza de cocina, 3 o 4 botellas o bolsas de arena, computadora con el IDE de Arduino.

| Audio | Visual |
|---|---|
| La celda de carga es nuestra forma de medir torque. Entrega pocos milivolts, el HX711 los amplifica y digitaliza unas diez veces por segundo. Pero su lectura no está en newtons: hay que calibrarla. | **IMG-10**. Título: «P1 – Celda de carga y programa». |
| Montaje mecánico: la base impresa se atornilla al extremo fijo de la celda, el lado del cable, con sus dos tornillos. El otro extremo queda libre en el aire, con un separador de dos o tres milímetros. La celda va horizontal, con la flecha de la etiqueta apuntando en el sentido en que va a recibir la carga. | **P2-GRAF-06**: dibujo lateral de la celda: extremo fijo con tornillos sobre la base, separador de 2–3 mm, extremo libre, flecha de carga. |
| Conexión. De la celda al HX711: rojo a E más, negro a E menos, verde a A más y blanco a A menos. Del HX711 al Arduino UNO: VCC a cinco volts, GND a GND, DT al pin cuatro y SCK al pin cinco. Si usás la Pico, alimentá el HX711 con 3,3 volts, porque sus salidas no deben llevar cinco volts a la Pico. | **IMG-29**: primer plano de los cuatro cables de colores en el HX711. **P2-GRAF-07**: tabla de conexión: rojo→E+, negro→E−, verde→A+, blanco→A− ; VCC→5 V (Pico: 3,3 V), GND→GND, DT→4, SCK→5. Pausa. |
| Cargá el programa del anexo del instructivo. Necesita la biblioteca HX711 de Bogdan Necula, que se instala desde el gestor de bibliotecas del IDE. Dejá el factor de la celda en 1,0. Abrí el monitor serie a 115200 baudios, con «nueva línea» como fin de línea. | **IMG-30**: laptop con el IDE y el monitor serie abierto (texto borroso). Texto: «Biblioteca HX711 (Bogdan Necula) · factorCelda = 1.0 · 115200 baudios · “nueva línea”». |
| El programa imprime cada medio segundo seis columnas: ciclo útil, fuerza, revoluciones del tacómetro, cuentas del AS5600, revoluciones del eje de salida y estado del imán. Los comandos son simples: la letra t pone la celda a cero, la letra z pone a cero el ángulo del AS5600, y un número fija el ciclo útil del driver, que solo usamos al final, en el yugo. Los sensores que no estén conectados simplemente leen cero. | **P2-GRAF-08**: «duty, fuerza_N, rpm, cuentas_AS, rpm_salida, iman» con una flecha a cada columna; recuadro de comandos: «t = tara · z = cero AS5600 · número = ciclo útil (solo en el yugo)». |
| Calibración. Sin carga, enviá t. Prepará tres o cuatro cargas pesadas en la balanza de cocina: por ejemplo doscientos, quinientos, mil y dos mil gramos. Apoyalas exactamente en el punto donde va a apoyar el brazo. Para cada una, anotá la lectura cruda y calculá la fuerza: la masa en kilos por 9,81. | **IMG-10** con zoom al punto de apoyo. Texto: «F = m [kg] · 9,81». Pausa. |
| Ajustá la recta de la lectura cruda en función de la fuerza. La pendiente es el factor de la celda, en cuentas por newton. Ponelo en el programa, volvé a cargarlo y verificá con una carga nueva que la lectura sea correcta. | **P2-GRAF-09**: recta «lectura cruda C vs F [N]», con «pendiente = factorCelda [cuentas/N]». |
| Si al descargar la lectura sube en lugar de bajar, intercambiá los cables A más y A menos. | Texto: «¿La lectura sube al descargar? → intercambiar A+ y A−». |
| Cómo saber si salió bien: el R cuadrado da más de 0,999, y al sacar la carga la lectura vuelve a cero con menos de 0,02 newton de diferencia. Si deriva, dejá todo encendido diez minutos para que caliente y repetí. | Caja naranja: «✓ R² > 0,999 · vuelve a cero ± 0,02 N · si deriva: 10 min de calentamiento». |
| Tabla a completar: masa, fuerza y lectura cruda de cada carga, y al final el factor obtenido. | **P2-GRAF-10**: tabla vacía «m [g] · F [N] · C [cuentas]» con 4 filas y «factorCelda = ____». |

---

## P2: Tacómetro óptico y encoder AS5600 (4:00)

**Materiales:** sensor óptico de herradura (H206/FC-03) o TCRT5000, disco ranurado impreso, módulo AS5600 con su imán, porta-imán y soporte impresos, osciloscopio, Arduino.

| Audio | Visual |
|---|---|
| Para medir velocidad del motor solo usamos un tacómetro óptico: un disco ranurado en el eje que pasa por la herradura del sensor, fijo a la cuna, sin rozar. | **IMG-11**. Título: «P2 – Tacómetro y AS5600». |
| Alimentá el sensor con cinco volts. La forma preferida de leerlo es el osciloscopio: punta en la salida del sensor, medís la frecuencia. Las revoluciones por minuto son sesenta por la frecuencia dividido por la cantidad de ranuras. También podés llevar la salida al pin dos del Arduino: el programa imprime las revoluciones si configurás la cantidad de ranuras. | **P2-GRAF-11**: «n = 60·f / k» con ejemplo: disco de 2 ranuras, f = 100 Hz → 3000 rpm. Texto: «Arduino: OUT → pin 2 · RANURAS = 2». |
| Control: a cualquier velocidad, el Arduino y el osciloscopio tienen que coincidir. Si la frecuencia sale el doble, el sensor está viendo los dos flancos de cada ranura o hay rebote: usá un disco de bordes netos y mirá la forma de onda. | Caja naranja: «✓ Arduino = osciloscopio · ¿frecuencia doble? → flancos o rebote». |
| El AS5600 es un encoder magnético absoluto: un chip mide la dirección del campo de un imán que gira sobre él y da 4096 posiciones por vuelta, 0,088 grados por cuenta. Lo usamos solo en el eje de salida de la reductora, porque el imán tiene que ir en la punta del eje y en el motor solo esa punta está ocupada por el brazo o el taladro. | **IMG-19**. Texto: «4096 cuentas/vuelta · 0,088°/cuenta · I2C 0x36 · solo en el eje de SALIDA». |
| Montaje. El imán tiene que ser de magnetización diametral, con los polos a los lados. El que trae el módulo, de seis por 2,5 milímetros, sirve. Pegalo en el centro del capuchón impreso, centrado con el eje de giro y separado del acero por uno o dos milímetros de plástico. La placa va en el puente impreso, con el chip paralelo al imán, entre 0,5 y 3 milímetros. Empezá con 1,5. | **P2-GRAF-12**: corte lateral: eje de salida → capuchón (1–2 mm de plástico) → imán diametral (N y S a los lados) → separación 0,5–3 mm (empezar 1,5 mm) → chip AS5600. Pausa. |
| Conexión al UNO: VCC a cinco volts, revisando el puente de tensión del módulo; GND; SDA a A4; SCL a A5. El pin DIR va a GND, y entonces el ángulo crece en sentido horario visto desde el imán, o a VCC para el sentido contrario. El pin OUT no se usa. Con la Pico, 3,3 volts. | **P2-GRAF-13**: tabla: VCC→5 V (Pico 3,3 V) · GND→GND · SDA→A4 · SCL→A5 · DIR→GND (horario) o VCC · OUT: sin conectar. |
| El programa lee el estado del imán. Si dice DEBIL, acercalo. Si dice FUERTE, alejalo. No midas nada hasta ver OK. | Tres etiquetas de color: «DEBIL → acercar» (amarillo), «FUERTE → alejar» (rojo), «OK → medir» (verde). |
| El AS5600 da el ángulo dentro de una vuelta, pero el programa lo lee cada milisegundo y suma o resta vueltas, así entrega el ángulo acumulado. Funciona mientras el eje gire menos de treinta mil revoluciones por minuto, y en la salida de la reductora hay unos pocos cientos. Con la letra z lo ponés a cero. | Texto: «Ángulo acumulado: lectura cada 1 ms · límite 30 000 rpm · z = cero». |
| Tres controles. Con el eje quieto, el ángulo fluctúa como mucho una o dos cuentas. Una vuelta exacta a mano, con una marca en el capuchón y otra fija, da 4096 más o menos diez cuentas: si da bastante menos o avanza a saltos, el imán está descentrado o lejos. Y con el taladro a velocidad constante, el AS5600 coincide con el tacómetro óptico. | Caja naranja: «✓ quieto: ± 1–2 cuentas · 1 vuelta: 4096 ± 10 · coincide con el tacómetro». |

---

## P3: Ensayo 1, rotor bloqueado (5:00)

**Obtiene:** R, V_esc y K_t. **Materiales:** cuna, brazo de 100 mm, celda calibrada, fuente con límite, dos instrumentos (amperímetro y voltímetro en bornes), calibre, termómetro.

| Audio | Visual |
|---|---|
| El ensayo más importante: en una sola prueba sacamos R, la caída en escobillas y K t. Repetilo igual para el motor R y para el motor P. | **IMG-12**. Título: «P3 – Ensayo 1: rotor bloqueado». |
| Antes de empezar, la regla de este ensayo: con el eje trabado, nunca tensión alta. Con medio ohm, veinticuatro volts darían cuarenta y cinco amperes. Trabajamos entre uno y cuatro volts, siempre con el límite de corriente puesto. | Banda roja: «Eje trabado: 1–4 V, siempre con límite de corriente». |
| Montaje. Motor en la cuna con el eje vertical: así el brazo gira en un plano horizontal y su peso no hace torque. El brazo, bien apretado al eje. La punta del tornillo apoya sobre la celda, cerca de su extremo libre, y el brazo queda perpendicular a la celda. Si forma otro ángulo, el torque es la fuerza por la distancia por el seno de ese ángulo. Medí con calibre la distancia del centro del eje al punto de contacto: un milímetro de error en cien es un uno por ciento de error en K t. | **IMG-12** con **P2-GRAF-14** superpuesto: «L_b = 100 mm (medir con calibre)», ángulo de 90° marcado, «τ = F·L_b·sen θ». **IMG-28** (calibre). |
| Conectá según el esquema del video P0, con el voltímetro en los bornes. La polaridad tiene que hacer que el motor empuje el brazo contra la celda. | **P2-GRAF-03** (esquema) con una flecha de giro hacia la celda. |
| Paso a paso. Fuente apagada, límite en un ampere y tensión en cero. Con el brazo apenas apoyado, enviá t para tarar. Encendé y subí la tensión despacio hasta que la fuente entre en límite: se prende la luz CC. Esperá dos o tres segundos a que el HX711 se estabilice y anotá tensión en bornes, corriente y fuerza. | **P2-GRAF-15**: pasos 1 a 4 numerados: «Límite 1 A, tensión 0 → tara (t) → subir tensión hasta luz CC → esperar 2–3 s y anotar V, I, F». Pausa. |
| Bajá a cero, esperá diez a veinte segundos, y repetí para dos, tres, cuatro y cinco amperes, subiendo solo el límite de corriente. | Texto que avanza: «1 A → 2 A → 3 A → 4 A → 5 A · pausa 10–20 s entre puntos». |
| Cinco amperes bloqueado son unos catorce watts dentro del bobinado. No más de cinco segundos por punto. Si la carcasa se entibia, esperá. | Banda roja: «5 A ≈ 14 W · máximo 5 s por punto». |
| Ahora cambiá la posición del rotor: aflojá el anillo de la cuna, girá el motor unos treinta grados sobre su eje, dejando el brazo donde está, apretá y repetí toda la serie. Hacelo en cuatro a seis posiciones, porque las escobillas apoyan en distintos segmentos del colector. | **IMG-32**: manos aflojando el anillo y girando el motor en la cuna. Texto: «Girar 30° · 4 a 6 posiciones». |
| Y el sentido contrario: invertí la polaridad y pasá la celda al otro lado del brazo. Repetí en dos posiciones. Sirve para ver si el motor es simétrico. | Texto: «Sentido contrario: invertir polaridad + celda del otro lado · 2 posiciones». |
| Anotá también la temperatura ambiente, porque la resistencia cambia con ella. | Texto: «Anotar temperatura ambiente». |
| Cálculo, en cada posición. La tensión en función de la corriente: la pendiente es R y la ordenada es la caída en escobillas. El torque, que es la fuerza por cero coma uno metros, en función de la corriente: la pendiente es K t. Su ordenada debería ser casi cero; si supera el diez por ciento del torque a cinco amperes, revisá el montaje. Al final promediá R, la caída en escobillas y K t de todas las posiciones, y anotá también el mínimo y el máximo. | **GRAF-09** de la parte 1 (tabla y dos rectas del ejemplo: R = 0,55 Ω, V_esc = 0,60 V, K_t = 0,0325 N·m/A). Texto: «ordenada de τ > 10 % de τ(5 A) → revisar montaje». |
| Cómo saber si salió bien: R cuadrado mayor que 0,99 en las dos rectas. R entre 0,2 y 2 ohm, y la caída en escobillas entre 0,2 y 2 volts. Si da muy afuera, seguramente estás midiendo la tensión en la fuente y no en los bornes. | Caja naranja: «✓ R² > 0,99 · R: 0,2–2 Ω · V_esc: 0,2–2 V · fuera de rango → ¿voltímetro en la fuente?». |
| Si no tenés celda, R se mide igual trabando el brazo contra un tope rígido. Y K t se puede sacar colgando una masa de un hilo enrollado en una polea impresa de radio conocido: subí la corriente hasta que el motor apenas levante la masa, después hasta que apenas la deje bajar, y promediá. K t es la masa por 9,81 por el radio, dividido por la corriente. El medidor LCR o el multímetro en ohm dan una idea de R, pero solo como control: miden con muy poca corriente, donde las escobillas tienen una resistencia alta e inestable. | **IMG-41**: hilo enrollado en una polea impresa en el eje del motor levantando una botella. Caja verde: «Sin celda: R con tope rígido · K_t ≈ m·g·r / I (promedio subir/bajar) · LCR: solo control, 6 posiciones». |
| Tabla a completar, una por posición. | **P2-GRAF-16**: tabla «I [A] · V en bornes [V] · F [N] · τ = F·L_b [N·m]», 5 filas, casillas «posición __ · sentido + / − · T ambiente __ °C». |

---

## P4: Ensayo 2, el motor como generador (3:30)

**Obtiene:** K_e. **Materiales:** taladro de velocidad regulable (u otro motor 775 con acople), tacómetro, osciloscopio o multímetro, anteojos.

| Audio | Visual |
|---|---|
| Ahora el motor no se alimenta: lo hacemos girar desde afuera y medimos la tensión que genera. Con los bornes abiertos no circula corriente, así que la tensión es directamente K e por la velocidad. | **IMG-13**. Título: «P4 – Ensayo 2: K_e». |
| Montaje. Motor fijo en la cuna, puede ser con el eje horizontal. Los bornes, desconectados de la fuente y del driver: solo el osciloscopio o el multímetro. El eje en el mandril del taladro, que tiene que quedar sujeto o sostenido firme y bien alineado, porque una desalineación carga los rulemanes. Y el tacómetro en el eje. | Lista: «Bornes: solo al instrumento · taladro firme y alineado · tacómetro en el eje». Banda roja: «Bornes DESCONECTADOS de fuente y driver». |
| Sin taladro, el otro motor 775 hace de motor de arrastre, unido eje con eje con el acople. Alimentalo de a poco y nunca conectes entre sí los bornes de los dos motores. | **IMG-42**: dos motores 775 alineados y unidos eje con eje por un acople. |
| Paso a paso. Hacé girar a una velocidad estable y medí la tensión en bornes, valor medio en el osciloscopio o volts de continua en el multímetro, y la velocidad. Repetí a mil, mil quinientas, dos mil, dos mil quinientas y tres mil revoluciones por minuto. Después en sentido contrario: la tensión cambia de signo, pero el valor tiene que coincidir. | Texto que avanza: «1000 → 1500 → 2000 → 2500 → 3000 rpm · luego sentido contrario». Pausa. |
| No pases de tres mil a cuatro mil revoluciones por minuto con el taladro: la tensión generada crece con la velocidad. Anteojos puestos. | Banda roja: «Máximo 3000–4000 rpm · anteojos». |
| Cálculo. Pasá las revoluciones a radianes por segundo multiplicando por 0,10472. La pendiente de la tensión en función de la velocidad es K e. La ordenada tiene que ser casi cero. | **GRAF-11** de la parte 1. Texto: «ω = 0,10472·n». |
| El control más importante del procedimiento: K e tiene que coincidir con el K t del ensayo 1 dentro de un cinco a diez por ciento. | Caja naranja: «✓ |K_t − K_e| < 5–10 %». |
| Dato extra, opcional. Con el osciloscopio en acoplamiento AC y pocos milivolts por división, vas a ver un rizado. Calculá z como sesenta por la frecuencia del rizado dividido por las revoluciones. Con z conocido, ese rizado te sirve como tacómetro sin disco. | **GRAF-12** de la parte 1. Texto: «z = 60·f_r / n». |
| Sin tacómetro, tomá K e igual a K t. O medilo con la reductora puesta y el AS5600 en la salida: la velocidad del motor es N por la de salida, siempre que N se conozca bien. | Caja verde con el resumen. |
| Tabla a completar. | **P2-GRAF-17**: tabla «n [rpm] · ω [rad/s] · V [V] · sentido», 5 filas por sentido. |

---

## P5: Ensayo 3, giro en vacío y rozamiento (3:00)

**Obtiene:** τ_c, b, I_0 y n_0. **Materiales:** cuna, disco del tacómetro, fuente, amperímetro y voltímetro, anteojos.

| Audio | Visual |
|---|---|
| El motor gira libre, solo con el disco del tacómetro, alimentado directo de la fuente. Amperímetro en serie y voltímetro en bornes. | **IMG-14**. Título: «P5 – Ensayo 3: vacío y rozamiento». |
| Poné el límite de corriente en tres amperes, porque el arranque pide un pico. | Texto: «Límite: 3 A (pico de arranque)». |
| Aplicá seis, doce, dieciocho y veinticuatro volts. En cada tensión esperá treinta segundos a que la velocidad se estabilice y anotá tensión, corriente y revoluciones. | Texto que avanza: «6 V → 12 V → 18 V → 24 V · esperar 30 s». Pausa. |
| Hacelo primero con el motor frío y repetí después de diez minutos girando a doce volts. El rozamiento baja cuando escobillas y rulemanes se calientan: usá los valores en tibio. Y repetí en sentido contrario. | Texto: «Frío → 10 min a 12 V → tibio (usar este) · ambos sentidos». |
| Cálculo. El rozamiento a cada velocidad es K t por la corriente en vacío. La recta del rozamiento en función de la velocidad da, en la ordenada, el rozamiento de Coulomb, y en la pendiente, el viscoso. | **GRAF-13** de la parte 1 y la tabla del ejemplo (6 V: 0,55 A, 1512 rpm … 24 V: 0,95 A, 6785 rpm). |
| Dos controles. Primero, la velocidad a veinticuatro volts tiene que coincidir con la predicha, veinticuatro menos R por I cero menos la caída en escobillas, todo dividido por K e, dentro de un cinco por ciento. Segundo, compará la corriente en vacío a veinticuatro volts con el dato del vendedor, cerca de un ampere. | Caja naranja: «✓ ω₀ = (24 − R·I₀ − V_esc) / K_e (± 5 %) · I₀ a 24 V ≈ 1 A (dato del vendedor)». |
| Sin tacómetro, calculá la velocidad con esa misma fórmula, o usá el rizado de la corriente sobre un shunt del lado negativo con el z del ensayo anterior. | Caja verde con el resumen. |
| Tabla a completar. | **P2-GRAF-18**: tabla «V [V] · I₀ [A] · n [rpm] · ω [rad/s] · τ_f = K_t·I₀ [N·m]» para frío y tibio. |

---

## P6: Ensayo 4, inductancia L (3:30)

**Obtiene:** L y τ_e = L/R. **Materiales:** medidor LCR; para el método B: shunt de 0,1 Ω, osciloscopio, interruptor, capacitor de 4700 µF 35 V, montaje del ensayo 1.

| Audio | Visual |
|---|---|
| Hay dos métodos. El A es el más simple: medidor LCR, motor desconectado y quieto. Medí a un kilohertz, y a diez si el medidor lo permite, en seis posiciones del rotor, girando el eje a mano entre una y otra. Anotá mínimo, máximo y promedio. | **IMG-15**. Título: «P6 – Ensayo 4: inductancia». Texto: «1 kHz (y 10 kHz) · 6 posiciones · mín/prom/máx». |
| El LCR mide con poquísima corriente y el valor real puede ser algo menor. Por eso conviene confirmarlo con el método B, el escalón. | Texto: «Confirmar con el método B». |
| Método B. Eje trabado con el montaje del ensayo 1. En serie con el motor, del lado negativo, el shunt de 0,1 ohm. La punta del osciloscopio sobre el shunt y la masa en el negativo de la fuente: la tensión en el shunt es la corriente por 0,1. Fuente a dos volts con límite de cinco amperes. Para que el escalón sea limpio, poné el capacitor de 4700 microfaradios en paralelo con la salida de la fuente, cerca del interruptor. | **IMG-33**: punta del osciloscopio sobre el shunt, capacitor grande en la salida de la fuente, interruptor de palanca. **P2-GRAF-19**: esquema fuente (2 V, límite 5 A) ∥ capacitor 4700 µF → interruptor → motor → shunt 0,1 Ω → negativo; punta sobre el shunt, masa al negativo. |
| Osciloscopio en disparo único, flanco de subida, 0,2 milisegundos por división. Cerrá el circuito de golpe con el interruptor. | Texto: «Disparo único · flanco de subida · 0,2 ms/div». Pausa. |
| La corriente sube como una exponencial. Medí el tiempo que tarda en llegar al sesenta y tres por ciento del valor final: esa es la constante de tiempo. La inductancia es esa constante por la resistencia total del circuito, que se calcula fácil como la tensión de la fuente dividida por la corriente final, medidas en el mismo ensayo. Repetí en varias posiciones del rotor. | **GRAF-14** de la parte 1. Texto: «R_total = V_fuente / I_f». |
| Control: los dos métodos coinciden dentro de un veinte a treinta por ciento, y la constante de tiempo eléctrica da entre décimas de milisegundo y pocos milisegundos. | Caja naranja: «✓ A y B: ± 20–30 % · τ_e: 0,1 ms a pocos ms». |
| Tabla a completar. | **P2-GRAF-20**: tabla «posición · L LCR 1 kHz · L LCR 10 kHz · τ escalón [ms] · R_total [Ω] · L escalón [mH]». |

---

## P7: Ensayo 5, inercia del rotor J_m (3:30)

**Obtiene:** J_m (opcional, importa en pitch). **Materiales:** osciloscopio, fuente a 24 V, volante: buje impreso con arandelas de acero, balanza, calibre.

| Audio | Visual |
|---|---|
| El motor gira libre a veinticuatro volts con el osciloscopio en los bornes. Configuralo en disparo único, flanco de bajada, cincuenta a cien milisegundos por división. | **IMG-16**. Título: «P7 – Ensayo 5: inercia». Texto: «Disparo único · flanco de bajada · 50–100 ms/div». |
| Desconectá el cable positivo de la fuente. No apagues la fuente, porque su salida puede descargar al motor. La traza muestra la tensión cayendo hasta cero, y como los bornes están abiertos, esa tensión es la velocidad: dividila por K e. | Banda roja: «Desconectar el cable +, NO apagar la fuente». Texto: «ω(t) = V(t) / K_e». Pausa. |
| Primer método. En tres puntos de la curva medí la velocidad y la pendiente de la tangente. La inercia es el rozamiento a esa velocidad, Coulomb más viscoso por la velocidad, dividido por el valor absoluto de la desaceleración. Promediá los tres. | **GRAF-15** de la parte 1 y **P2-GRAF-21**: «J_m = (τ_c + b·ω) / |dω/dt|» ; ejemplo: ω = 500 rad/s, |dω/dt| = 1350 rad/s² → J_m ≈ 1,9·10⁻⁵ kg·m². |
| Segundo método, el recomendado, porque no depende del modelo de rozamiento. Armá un volante: un buje impreso con arandelas de acero. Pesalo en la balanza y medí los radios exterior e interior. Su inercia es un medio de la masa por la suma de los cuadrados de los dos radios. | **IMG-34**: buje impreso con arandelas de acero sobre una balanza de cocina, calibre al lado. Texto: «J_v = ½·m·(r_e² + r_i²)». |
| Repetí la desaceleración con el volante puesto. A la misma velocidad el rozamiento es el mismo, así que con la desaceleración sin volante, a uno, y con volante, a dos, la inercia del rotor es la del volante por a dos sobre a uno menos a dos. | **GRAF-15b** de la parte 1: «J_m = J_v · a₂ / (a₁ − a₂)» (a₁ y a₂ a la misma ω). |
| Control: los dos métodos coinciden dentro de un veinte por ciento. | Caja naranja: «✓ métodos 1 y 2: ± 20 %». |
| Tabla a completar. | **P2-GRAF-22**: tabla «punto · ω [rad/s] · |dω/dt| sin volante · |dω/dt| con volante · J_m». |

---

## P8: Ensayo 6, límite térmico I_cont (3:30)

**Obtiene:** la corriente continua admisible. **Materiales:** tope rígido, fuente con límite, voltímetro en bornes, termómetro infrarrojo o termocupla, reloj.

| Audio | Visual |
|---|---|
| Este ensayo es largo, pero decide cuánto puede hacer el yugo de forma sostenida. Eje trabado contra un tope, sin celda, y el motor en la misma posición en que va a quedar en el yugo. | **IMG-35**. Título: «P8 – Ensayo 6: límite térmico». |
| Fijá la corriente con el límite de la fuente, empezando por dos amperes. Es importante que sea con límite: a medida que el motor se calienta, la tensión sube, y la fuente lo compensa sola. Sin límite, la corriente bajaría y el ensayo sería falso. | Texto: «Empezar en 2 A · SIEMPRE con límite de corriente». |
| Cada dos minutos anotá la temperatura de la carcasa y la tensión en bornes. Con la tensión calculá R: tensión menos caída en escobillas, dividido por la corriente. Y con R, la temperatura del bobinado: veinte más el cociente entre R sobre R a veinte grados menos uno, y 0,0039. | **P2-GRAF-23**: «R = (V − V_esc) / I» ; «T_bob = 20 + (R/R₂₀ − 1) / 0,0039». Ejemplo: R/R₂₀ = 1,30 → T_bob ≈ 97 °C. |
| Seguí hasta que la temperatura deje de subir, entre veinte y cuarenta minutos, o hasta que el bobinado llegue a cien grados. Dejá enfriar y repetí con tres, cuatro amperes, y así. | **GRAF-16** de la parte 1 (curvas que se aplanan). Texto: «2 A → enfriar → 3 A → enfriar → 4 A …». |
| Si aparece olor a barniz o humo, cortá de inmediato. | Banda roja: «Olor a barniz o humo: CORTAR». |
| El criterio: la corriente continua admisible es la mayor que estabiliza el bobinado por debajo de unos cien grados y la carcasa por debajo de sesenta o setenta. Si conseguís la hoja de datos del fabricante, usá su valor. | Caja naranja: «I_cont = mayor I con T_bob < 100 °C y T_carcasa < 60–70 °C». |
| Tabla a completar, una por corriente. | **P2-GRAF-24**: tabla «t [min] · T carcasa [°C] · V [V] · R [Ω] · T bobinado [°C]», filas cada 2 min hasta 40. |

---

## P9: La reductora (opcional) (5:00)

**Obtiene:** N real, η_d, η_i, rozamiento de la reductora y juego. **Materiales:** reductora vuelta a montar, brazo de 150 mm, celda sobre carro con varilla roscada M8, AS5600 en la salida (o puntero de 150 mm y regla).

| Audio | Visual |
|---|---|
| Esta etapa es opcional. Para el control alcanza con el motor, la relación contada por dientes y un rendimiento supuesto, más la verificación final en el yugo. Los ensayos de banco de la reductora se hacen si esa verificación da algo raro y querés saber si la pérdida está en la reductora o en la correa. Lo que sí hay que hacer siempre es contar los dientes. | **IMG-18**. Título: «P9 – La reductora (opcional)». Texto: «Siempre: contar dientes · Banco: solo si la verificación final falla». |
| Relación real. «3,7» y «19,2» son valores redondeados. Con la reductora separada, en cada etapa planetaria con corona fija: uno más los dientes de la corona sobre los del sol. Diez y veintisiete dan 3,7. Con dos etapas, se multiplican. Es exacto. | **IMG-27**: punta de lápiz contando dientes del sol y de la corona sobre un paño. **GRAF-19** de la parte 1. Texto: «2 etapas: N = N₁·N₂». |
| Otra forma, si ya tenés K e: hacé girar el eje de salida con el taladro, despacio, a no más de ciento cincuenta revoluciones por minuto en pitch, y medí la tensión del motor y la velocidad de salida con el AS5600. N es la tensión dividida por K e por la velocidad de salida. | Texto: «N = V / (K_e·ω_s) · n_s ≤ 150 rpm en pitch». |
| Volvé a montar la reductora en el motor, con los mismos tornillos y posiciones. | Texto: «Rearmar: mismos tornillos y posiciones». |
| Ensayo 7, rendimiento. Igual que el ensayo 1, pero con el brazo de ciento cincuenta milímetros en el eje de salida. La celda de veinte kilos aguanta casi treinta newton metro con ese brazo: sobra. Montá la base de la celda sobre una varilla roscada M8, para poder moverla muy despacio girándola a mano. | **IMG-36**: celda montada sobre un carro con varilla roscada empujando un brazo largo en la salida de la reductora. |
| Para cada corriente, uno, dos, tres y cuatro amperes, medí dos fuerzas. La directa: subí la corriente, dejá que el motor empuje la celda hasta asentarse, retrocedé la celda unas décimas de milímetro y leé. La inversa: avanzá la celda muy despacio contra el motor, que el motor pierda, y leé en movimiento lento. Repetí en cuatro posiciones del eje de salida y en los dos sentidos. | **P2-GRAF-25**: dos columnas: «F_d: motor empuja → retroceder celda décimas de mm» y «F_i: celda avanza → motor pierde». Texto: «1–4 A · 4 posiciones · 2 sentidos». Pausa. |
| Cálculo. La pendiente de la fuerza directa por cero coma quince da el torque de salida por ampere. Dividido por N por K t da el rendimiento directo. Para el inverso, N por K t dividido por la pendiente inversa por cero coma quince. En el ejemplo de pitch: 0,83 y 0,85. Lo normal es 0,90 a 0,95 por etapa a carga nominal, algo menos con poca carga. Si da mucho menos, por ejemplo 0,5, revisá grasa, alineación o montaje. | **P2-GRAF-26**: «η_d = (pend. F_d · 0,15) / (N·K_t)» ; «η_i = N·K_t / (pend. F_i · 0,15)» ; ejemplo pitch: N·K_t = 0,624 → η_d = 0,83 ; η_i = 0,85 ; «referencia 0,90–0,95 por etapa · ≈ 0,5 → revisar». |
| Ensayo 8, rozamiento. Repetí el giro en vacío del ensayo 3 con la reductora puesta. La corriente sube de I cero a I cero con reductora, y el rozamiento agregado, visto desde la salida, es K t por esa diferencia por N por el rendimiento directo. Además, con el motor desconectado, mové el brazo de salida empujando despacio con la celda y leé la fuerza mínima para que gire en cada sentido: es el rozamiento total que siente la mano con el motor apagado. | Texto: «τ_f,red ≈ K_t·(I₀,red − I₀)·N·η_d» ; «A mano: fuerza mínima para girar, en cada sentido». |
| Ensayo 9, juego. Motor desconectado, AS5600 en la salida. Mové el eje a mano, suave, en un sentido hasta que tome, es decir, hasta que empiece a arrastrar al motor, y anotá. Después en el otro sentido hasta que tome. La diferencia es el juego. Repetí cinco veces y promediá: suele ser de pocas cuentas. La histéresis interna del AS5600 tiene que estar desactivada, que es como viene de fábrica. Sin AS5600, un puntero de ciento cincuenta milímetros y una regla: un milímetro son 0,38 grados. | **IMG-37**: puntero largo en el eje de salida sobre una regla. Texto: «juego = posición «toma» (+) − posición «toma» (−) · 5 repeticiones · 1 cuenta = 0,088° · puntero 150 mm: 1 mm = 0,38°». |
| En pitch, el juego se traduce en milímetros en la empuñadura: el radio de la polea por el ángulo, en radianes. | Texto: «Δx = r_polea · Δθ». |
| Tabla a completar. | **P2-GRAF-27**: tabla «I [A] · F_d [N] · F_i [N]» por posición y sentido; casillas «N (dientes) · η_d · η_i · I₀,red · τ_f,red · juego [°]». |

---

## P10: Cálculo en el yugo, verificación final y hoja de registro (5:00)

**Materiales:** yugo armado, celda con base fija, BTS7960, Arduino con el programa, multímetro en serie, AS5600 en el eje de pitch, regla.

| Audio | Visual |
|---|---|
| Último paso: llevar todo a la mano. Si no hiciste la etapa de la reductora, usá N contado por dientes y un rendimiento supuesto: 0,90 en roll, que tiene una etapa, y 0,81 en pitch, que tiene dos. Es una primera estimación; los valores definitivos salen de la verificación de este video. | **IMG-20**. Título: «P10 – Yugo, verificación y registro». Texto: «Sin etapa 2: η_d ≈ 0,90 (roll) · 0,81 (pitch)». |
| En roll: K roll es K t por N por el rendimiento, en newton metro por ampere. La corriente es el torque pedido dividido por K roll. Si querés la fuerza en la empuñadura, medí el radio desde el eje de roll hasta el centro de la mano y dividí. | **GRAF-21** de la parte 1. Texto: «I = τ_pedido / K_roll ; F = τ / r_emp». |
| En pitch necesitás el radio efectivo de la polea, el radio sobre el que trabaja la correa, no el exterior. Con la transmisión armada, enviá z, mové el carro una distancia medida con regla, por ejemplo doscientos milímetros, y leé las cuentas acumuladas del AS5600. El radio es la distancia dividida por dos pi por las cuentas sobre 4096. Repetí tres veces en cada sentido y promediá. Para una polea dentada debe dar el radio primitivo, dientes por paso sobre dos pi: con la de setenta y cinco milímetros, 37,5 milímetros. | **IMG-38**: carro de pitch con una regla a lo largo de la guía. **P2-GRAF-28**: «r = s / (2π·c/4096)» ; ejemplo: s = 200 mm → c ≈ 3477 cuentas → r ≈ 37,5 mm ; «polea dentada: r = Z·p / 2π». Pausa. |
| Con el radio, la fuerza por ampere en pitch es K t por N por el rendimiento sobre el radio. | **GRAF-22** de la parte 1 y la tabla de pitch (13,9 N/A ; 50 N → 3,6 A ; m_eq ≈ 5 kg ; rozamiento ≈ 7 N). |
| La verificación final valida toda la cadena con una sola prueba. Celda de carga en la empuñadura, perpendicular al movimiento, sujeta a una base fija, con el yugo cerca del centro. | **IMG-22**. |
| Acá sí se usa el driver. Conexión: RPWM al pin nueve, LPWM al diez, R EN y L EN a cinco volts, VCC a cinco volts y GND común con la fuente. El programa limita el ciclo útil a veinticinco sobre doscientos cincuenta y cinco, unos 2,4 volts con veinticuatro de bus. Con el eje trabado, la corriente queda en el ciclo útil por la tensión de bus, menos la caída en escobillas, dividido por R: no subas ese límite hasta conocer R y hacer la cuenta. Y dejá igual el límite de corriente de la fuente. | **IMG-39**: módulo BTS7960 cableado al Arduino y al motor. **P2-GRAF-29**: «RPWM→9 · LPWM→10 · R_EN, L_EN→5 V · VCC→5 V · GND común» ; «MAX_DUTY = 25 → ≈ 2,4 V» ; «I ≈ (D·V_bus − V_esc) / R». Banda roja: «No subir MAX_DUTY sin conocer R · mantener el límite de la fuente». |
| Con la Pico en lugar del UNO, la lógica del BTS7960, el HX711 y el AS5600 van a 3,3 volts. | Texto: «Pico: todo a 3,3 V». |
| Enviá un número de ciclo útil y ajustalo hasta tener uno, dos y tres amperes medidos con el multímetro en serie. En cada corriente y en los dos sentidos, medí la fuerza directa y la inversa, como en el ensayo 7. Después, con el motor desconectado, medí la fuerza necesaria para mover la empuñadura despacio en cada sentido: es el rozamiento total, motor más reductora más correa. | Texto: «1 A · 2 A · 3 A · ambos sentidos · F_d y F_i» ; «Motor desconectado: rozamiento total». Pausa. |
| Ajustá las rectas de fuerza en función de corriente: las pendientes son la fuerza por ampere real, en cada sentido. En roll, multiplicalas por el radio de la empuñadura. Y con K t y N calculás el rendimiento de toda la cadena. Si da menos de 0,6 a 0,7, revisá la tensión de la correa, la alineación y la lubricación. | **GRAF-23** de la parte 1 y **P2-GRAF-30**: «pitch: η_d,total = K_F,d·r / (K_t·N) ; η_i,total = K_t·N / (K_F,i·r)» ; «roll: η_d,total = K_roll,d / (K_t·N) ; η_i,total = K_t·N / K_roll,i». |
| Reemplazá el rendimiento supuesto por el medido. Los valores medidos en el yugo son los que van al firmware. El modelo del motor sigue sirviendo para la prealimentación, con R, la caída en escobillas y K e, y para los límites térmicos. | Texto: «Firmware ← valores medidos en el yugo · Prealimentación y límites ← modelo del motor». |
| Para cerrar, la hoja de registro. Completala para los dos motores, R y P: en el motor solo, temperatura ambiente, R con mínimo, promedio y máximo, caída en escobillas, K t en los dos sentidos, K e, la diferencia entre ambos, corriente y velocidad en vacío, rozamientos, inductancia, inercia y corriente continua. En la reductora, N, los rendimientos, el rozamiento y el juego. Y en el yugo, los radios, la fuerza por ampere predicha y medida, y la corriente máxima elegida. | **IMG-40**: planilla en blanco sobre el banco con una lapicera. **P2-GRAF-31**: hoja de registro completa (copia de la tabla «Hoja de registro» del instructivo) con columnas Motor R y Motor P. |
| Con la hoja completa tenés todo lo que el control de corriente necesita. Buen trabajo en el banco. | **IMG-23** de la parte 1. Fundido a negro con el texto: «Proyecto MiYoke PD – Fin de la parte práctica». |

---

# Mensaje para ChatGPT (DALL·E), imágenes nuevas IMG-24 a IMG-42

Mismo criterio que en la parte 1: copiá todo el bloque. Es la versión para el **modo cowork** (genera todas seguidas y te las entrega juntas). Si no puede, cambiá la línea «Generá las 19 imágenes seguidas…» por «Generá una por vez y esperá que yo escriba siguiente».

```text
Hola. Necesito 19 imágenes más para la segunda parte (práctica) de un video técnico educativo sobre los ensayos de un motor de corriente continua tipo 775 (motor DC de escobillas, de unos 42 mm de diámetro y 66 mm de largo, con reductora planetaria cilíndrica) en un banco de electrónica. Tienen que mantener el mismo estilo que la serie anterior (IMG-01 a IMG-23).

Reglas para TODAS las imágenes:
- Formato horizontal 16:9 (1792x1024).
- Estilo común: "clean semi-realistic technical illustration, soft studio lighting, dark neutral gray workshop background, accent colors electric blue and warm orange, shallow depth of field, consistent style across the series".
- SIN texto, letras, números, etiquetas ni logos en la imagen. Los displays de los instrumentos deben verse apagados o con formas sin números legibles.
- Generá las 19 imágenes seguidas, sin esperar confirmación entre una y otra.
- Guardá cada una como PNG con su nombre exacto: IMG-24.png ... IMG-42.png.
- Antes de pasar a la siguiente, revisá que no tenga texto ni letras; si tiene, rehacela.
- Al terminar, entregame todas en un ZIP y una lista con el nombre de cada imagen y una línea que diga qué muestra.

Lista de prompts (agregá el estilo común a cada uno):

IMG-24: Close-up of a hand turning the current-limit knob of a benchtop laboratory power supply, its two output terminals shorted together by a short red cable, a small indicator light glowing.

IMG-25: Two identical 775 DC motors side by side on a workbench, one wrapped with a strip of blue masking tape and the other with orange masking tape, a red dot painted next to one terminal of each, a marker pen lying beside them.

IMG-26: Hands using a small screwdriver to separate a planetary gearbox from a 775 DC motor on a workbench, a smartphone on a small stand photographing the process, removed screws in a small tray.

IMG-27: Close-up of a disassembled planetary gearbox stage on a soft cloth: a small sun gear and an outer ring gear with internal teeth, a pencil tip pointing at one tooth as if counting.

IMG-28: Digital calipers measuring the diameter of a small motor shaft, close-up, sharp focus on the caliper jaws and the shaft.

IMG-29: Close-up of a small HX711 amplifier board with four colored wires (red, black, green, white) coming from a bar load cell, and four jumper wires going to an Arduino Uno.

IMG-30: A laptop on a workbench showing a code editor and a serial monitor window with blurred unreadable lines, connected by USB to an Arduino Uno with a load cell amplifier.

IMG-31: A small power resistor (shunt) connected in series with a DC motor on a bench, the probes of a digital multimeter measuring across the resistor, thick cables to a power supply.

IMG-32: Hands loosening the clamping screw of a 3D-printed ring cradle and rotating a DC motor about its vertical shaft, while a lever arm attached to the shaft stays resting on a load cell.

IMG-33: An oscilloscope probe clipped across a small shunt resistor in a motor circuit, a large cylindrical electrolytic capacitor connected across the output terminals of a power supply, a toggle switch in the circuit, oscilloscope in the background showing a rising curve.

IMG-34: A 3D-printed hub with a stack of steel washers forming a small flywheel, sitting on a digital kitchen scale, digital calipers lying beside it.

IMG-35: A DC motor clamped on a bench with its shaft locked against a rigid metal stop, a thermocouple probe taped to the motor housing, a handheld infrared thermometer and a stopwatch nearby.

IMG-36: A bar load cell mounted on a small sliding carriage moved by a long threaded rod with a hand knob, pressing against a long lever arm attached to the output shaft of a planetary gearbox motor.

IMG-37: A long thin pointer attached to the output shaft of a planetary gearbox, its tip resting just above a steel ruler fixed to the bench, hand gently turning the shaft.

IMG-38: The pitch carriage of a flight simulator yoke mechanism on two linear rails with a toothed belt and pulley, a steel ruler laid along the rail to measure carriage travel.

IMG-39: A BTS7960 dual H-bridge motor driver module (red board with a large heatsink) wired to an Arduino Uno with jumper wires and to a geared DC motor with thick cables.

IMG-40: A clipboard with a blank printed grid log sheet (empty table, no writing) lying on an electronics workbench next to a pen, a multimeter and a DC motor.

IMG-41: A DC motor clamped with its shaft horizontal, a small 3D-printed pulley on the shaft with a thin thread wound around it, the thread lifting a small water bottle hanging below.

IMG-42: Two 775 DC motors aligned shaft-to-shaft on a bench, joined by a short aluminum rigid shaft coupler with set screws, both clamped in 3D-printed cradles.
```

---

# Cómo armar esta parte

Igual que la parte 1 (Clipchamp, texto a voz y gráficos en PowerPoint), con estas diferencias:

1. **Un archivo de video por cada P0 a P10.** Así, en el banco abrís solo el del ensayo que vas a hacer.
2. **Pausas**: en las filas marcadas con «Pausa», dejá la imagen quieta 2 o 3 segundos sin narración antes de seguir. Le avisa al que mira que es el momento de pausar y hacer el paso.
3. **Bandas rojas de seguridad**: un rectángulo rojo con texto blanco en la parte inferior, al menos 4 segundos.
4. **Tablas a completar** (P2-GRAF-10, 16, 17, 18, 20, 22, 24, 27 y 31): armalas en PowerPoint y, además de ponerlas en el video, imprimilas. Son la planilla del banco.
5. **Capítulos**: si lo subís a YouTube (aunque sea como no listado), podés juntar los once en un solo video y poner marcas de tiempo en la descripción: cada línea `0:00 P0 Preparar el banco`, `5:00 P1 Celda de carga…` se vuelve un capítulo navegable.
