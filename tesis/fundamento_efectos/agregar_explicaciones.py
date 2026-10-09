from pathlib import Path
p=Path('tesis/fundamento_efectos/generar_documento.py')
s=p.read_text(encoding='utf-8')
insertions={
r'\subsection{Por qué todavía no se puede indicar una fuerza en newtons}':r'''
\subsection{Explicación sencilla: cómo leer los símbolos comunes}
Una fórmula de este documento combina tres clases de datos: \textbf{entrada del simulador}, \textbf{ajuste del usuario} y \textbf{resultado calculado}. No todas las letras son variables de SimConnect.
\begin{description}
\item[$R$, $P$ y $a$.] $R$ indica roll y $P$ pitch. El subíndice $a$ significa ``el eje que se está calculando''. No son datos recibidos.
\item[$G$ y $g_i$.] $G$ es Ganancia total: 60\% significa $G=0.60$. $g_i$ es la ganancia del efecto $i$, por ejemplo 0.120 para Damper. Ambos vienen del menú, no de Prepar3D.
\item[$e_i$ y $\widetilde g_i$.] El interruptor vale uno si el efecto está activado y cero si está desactivado. Multiplicarlo por la ganancia permite apagar el efecto sin borrar el ajuste.
\item[$u$ y PWM.] $u$ es el comando calculado antes de convertirlo a una salida entera. Su signo selecciona sentido y su magnitud determina PWM. No representa newtons. Por ejemplo, $u=0.10$ produce $\lfloor25.5+0.5\rfloor=26$ PWM.
\item[$\clip$, $\sat$ y $\mathcal R$.] Limitar con $\clip$ impide salir de dos extremos; $\sat$ limita entre cero y uno. $\mathcal R(z;a,b)$ es una rampa: vale cero hasta $a$, crece linealmente y llega a uno en $b$. Por ejemplo, $\mathcal R(125;100,150)=0.5$. En esta función $a,b$ son extremos, no ejes.
\item[$|z|$, $\sgn(z)$ y $\min$.] $|z|$ toma la magnitud sin signo; $\sgn$ devuelve la dirección; $\min$ elige el menor valor. $\lfloor z\rfloor$ es la parte entera hacia abajo. Sumando 0.5 antes se redondea la magnitud al entero más cercano.
\item[$\sum_i$, $L$, $d_a$ y $\mathcal P$.] Se suman los comandos de todos los efectos y se limita a $\pm L=\pm220/255$. $d_a$ corrige el sentido según el cableado/configuración. $\mathcal P(z)$ convierte una magnitud normalizada en el máximo PWM entero permitido. Es una abreviatura matemática, no una variable de SimConnect.
\end{description}
El intervalo $[-M,M]$ significa que la salida puede tener ambos signos y no superar la magnitud $M$. No significa que esté oscilando permanentemente entre esos extremos. Las unidades y la condición de vuelo determinan si se llega a ellos.

''',
r'\section{Variables de Prepar3D obtenidas por SimConnect}':r'''
\subsection{Explicación sencilla: del PWM a la fuerza física}
La tensión aplicada $V_m$ se reparte entre resistencia del bobinado ($RI$), inductancia ($L_m\,dI/dt$) y tensión generada por el motor al girar ($K_e\omega_m$). $I$ es corriente; $R$ resistencia eléctrica, que en esta ecuación no significa roll; $L_m$ inductancia; $\omega_m$ velocidad angular del motor y $K_e$ su constante eléctrica. Ninguna de estas magnitudes se recibe actualmente desde SimConnect.

El torque del motor $\tau_m$ depende de la corriente mediante $K_t$, su constante de torque. Las reducciones 3 y 19 multiplican aproximadamente ese torque y $\eta_R,\eta_P$ representan pérdidas de eficiencia. En pitch, dividir por el radio efectivo $r_{\rm ef}$ convierte torque en fuerza lineal $F_P$. Si la transmisión no es una polea debe usarse la relación mecánica correspondiente. Por eso un PWM conocido no permite deducir por sí solo una fuerza: falta saber cuánta corriente circula y cómo transmite el esfuerzo el mecanismo.

''',
r'\section{Turbulencia y ráfagas}':r'''
\subsection{Explicación sencilla}
El centrado se calcula en cuatro pasos: leer presión, compararla con una referencia, obtener una rigidez y multiplicarla por cuánto se desplazó el mando.
\begin{description}
\item[$q_{\rm psf}$ y $q$.] \textbf{Entrada SimConnect:} \code{DYNAMIC PRESSURE}, solicitada en psf. $q_{\rm psf}$ es el número recibido; $q$ es ese mismo dato convertido a pascales multiplicando por 47.88025898. No se lee $q$ de una variable diferente ni se calcula aquí a partir de IAS.
\item[$q_{\rm ref}$.] Es una presión de referencia calculada por el puente. 1.225 es la densidad de referencia en kg/m$^3$; 120 es la velocidad elegida en kt y 0.514444444 convierte kt a m/s. No son tres mediciones del simulador. La ecuación física $q=\rho V^2/2$ explica la referencia: $\rho$ es densidad del aire y $V$ velocidad respecto al aire; el puente utiliza directamente la presión recibida para el cálculo operativo.
\item[$p$.] Cociente adimensional $q/q_{\rm ref}$, limitado entre 0 y 4. $p=0.5$ significa la mitad de la presión de referencia, no la mitad de la velocidad.
\item[$r$ y $k^*$.] $r$ es la ganancia \textbf{Centro reposo}. $k^*$ es la rigidez deseada: parte de $r$ a presión cero y agrega $(1-r)p$ al aumentar la presión. No son SimVars.
\item[$k_n$, $k_{n-1}$ y $\Delta t$.] $k_n$ es la rigidez suavizada actual y $k_{n-1}$ la anterior. $\Delta t$ es el tiempo entre cálculos del firmware, en segundos. El término $\Delta t/(0.100+\Delta t)$ decide cuánto se avanza hacia la rigidez deseada; 0.100 s es el ajuste de suavizado. Así no cambia de golpe al llegar una nueva muestra.
\item[$x_a$.] \textbf{Entrada SimConnect:} \code{YOKE X POSITION} para $x_R$ y \code{YOKE Y POSITION} para $x_P$. Cero es el centro; magnitud uno es el extremo normalizado del simulador. No se mide todavía el desplazamiento mecánico mediante encoders.
\item[$g_{C,a}$ y $u_{C,a}$.] La ganancia es \textbf{Centrado roll} o \textbf{Centrado pitch}. El producto $Gg_{C,a}k_nx_a$ calcula el comando. El signo menos hace que se oponga al desplazamiento: con posición positiva, comando negativo.
\end{description}
\textbf{Ejemplo:} con total 60\%, pitch 0.800, reposo 0.150, $p=0.5$ y $x_P=0.5$, la rigidez estabilizada es $0.575$. El comando es $-0.60\cdot0.800\cdot0.575\cdot0.5=-0.138$, aproximadamente $-35$ PWM. Si soltamos hasta posición cero, el centrado pasa a cero aunque la rigidez siga siendo alta. Si se mantiene el mando desplazado, el centrado persiste.

''',
r'\section{Síntesis común de vibraciones}':r'''
\subsection{Explicación sencilla}
Primero se separa la parte cambiante del viento de su valor habitual; luego se transforma esa variación en empujes de roll y pitch y se suaviza la salida.
\begin{description}
\item[$w_X,w_Y,w_Z$.] \textbf{Entradas SimConnect:} \code{AIRCRAFT WIND X}, \code{AIRCRAFT WIND Y} y \code{AIRCRAFT WIND Z}, en kt y ejes del avión. $i$ sólo permite escribir la misma operación para cualquiera de las tres componentes.
\item[$\mu_i$ y $h_i$.] $\mu_i$ es un promedio que el puente va actualizando. $h_i=w_i-\mu_i$ es lo que se aparta de ese promedio. Un viento constante termina dando $h_i$ cercano a cero; un cambio produce una perturbación transitoria.
\item[$a=e^{-\Delta t/1.0}$.] $a$ es un coeficiente de memoria, no el eje de salida. $e$ es la base de la función exponencial, no el interruptor $e_i$. $\Delta t$ es el tiempo entre muestras del puente y 1.0 s fija qué tan rápido sigue el promedio al viento.
\item[$z_R,z_P$.] Son comandos provisionales. Dividir por 5 kt fija la sensibilidad. 0.70 pondera el viento lateral; en pitch se suma la variación vertical y un 25\% de la longitudinal. Son coeficientes del algoritmo, no fuerzas medidas.
\item[$p_T$ y $\gamma$.] $p_T$ reutiliza la presión derivada de \code{DYNAMIC PRESSURE}, pero limitada a uno para este efecto. $\gamma$ es \code{--gust-gain} del puente. La ganancia del menú se aplica después; son ajustes distintos.
\item[$\mathbf z$ y $\|\mathbf z\|$.] El vector reúne los dos comandos provisionales y su norma es $\sqrt{z_R^2+z_P^2}$. Dividir ambos por el mismo máximo limita la intensidad conjunta sin cambiar su dirección relativa.
\item[$b$, $\mathbf s$ y 0.08.] $b$ es el coeficiente del suavizado de salida; 0.08 s es su constante temporal. $\mathbf s$ guarda la salida filtrada. Estos valores los calcula el puente, no vienen de Prepar3D.
\item[$D(s)$, 0.02 y 0.98.] El firmware ignora magnitudes hasta 0.02 y reescala lo que queda dividiendo por $1-0.02=0.98$. Por ejemplo, $s=0.51$ da $D(s)=0.5$.
\item[$g_{T,a}$ y $u_{T,a}$.] Son la ganancia \textbf{Turbul. roll/pitch} y el comando final del efecto en ese eje. Se multiplica la señal filtrada por esa ganancia y por Ganancia total.
\end{description}
Con $D(s_P)=0.5$, total 60\% y Turbul. pitch=0.700, el comando es $0.21$, aproximadamente 54 PWM. El signo cambia con la perturbación calculada; no se trata de una vibración fija de una frecuencia determinada.

''',
r'\section{Buffet de pérdida}':r'''
\subsection{Explicación sencilla: cómo se construye una vibración}
La \textbf{envolvente} $A$ indica qué tan intensa debería ser la vibración entre cero y uno. La \textbf{portadora} $T$ cambia periódicamente de signo para hacer vibrar el mando. Multiplicarlas produce una vibración pequeña cuando $A$ es pequeño y grande cuando se aproxima a uno.
\begin{description}
\item[$\bar A$, $\beta$ y $\Delta t$.] La barra significa envolvente suavizada. En cada ciclo se calcula $\bar A_n=\bar A_{n-1}+\beta(A-\bar A_{n-1})$. $\beta=\Delta t/(0.100+\Delta t)$ y $\Delta t$ está en segundos. Es el mismo tipo de suavizado que el de rigidez.
\item[$\phi$, $f$ y $T_f$.] $\phi$ es la fase interna de la onda, entre 0 y 999; no es un ángulo del avión. $f$ es su frecuencia en Hz. $T_f$ es la triangular generada con esa frecuencia. Aquí $f$ no significa fracción de flap como en la sección de pérdida.
\item[$\Delta t_{\rm ms}$ y módulo 1000.] Para avanzar la fase se utiliza el tiempo del firmware en milisegundos. El módulo hace que la fase vuelva al principio al completar un ciclo. La primera rama sube de $-1$ a $+1$ y la segunda baja de $+1$ a $-1$.
\item[$u_P,u_R$.] Se multiplica la onda por envolvente, ganancia del efecto y ganancia total. Para los tres buffets, roll recibe la mitad del comando pitch. Esa relación es un ajuste del dispositivo.
\end{description}
Ni la fase ni la onda triangular se reciben por SimConnect: se generan en la Pico. SimConnect aporta las condiciones que determinan la envolvente de cada efecto.

''',
r'\section{Buffet del tren retráctil}':r'''
\subsection{Explicación sencilla}
La intensidad crece cuando el ángulo de ataque se acerca a la referencia de pérdida. La posición de flaps modifica desde qué ángulo empieza y qué velocidad se usa como referencia.
\begin{description}
\item[$f_L,f_R,f$.] \textbf{SimConnect:} \code{TRAILING EDGE FLAPS LEFT PERCENT} y \code{TRAILING EDGE FLAPS RIGHT PERCENT}, solicitadas como fracciones 0--1. $f$ es su promedio calculado; no son grados de flap.
\item[$\alpha,\alpha_s$.] \textbf{SimConnect:} \code{INCIDENCE ALPHA} y \code{STALL ALPHA}, en grados. Son el ángulo actual y el ángulo de pérdida suministrado por el modelo.
\item[$\alpha_0$.] Ángulo de inicio del buffet calculado por el puente. Es el 50\% de $\alpha_s$ con flaps limpios y el 70\% con flaps completos. No existe una SimVar que entregue directamente este umbral en la implementación.
\item[$V_{s0},V_{s1},IAS$.] \textbf{SimConnect:} \code{DESIGN SPEED VS0}, \code{DESIGN SPEED VS1} y \code{AIRSPEED INDICATED}, todas en kt. Son referencias de configuración y velocidad indicada actual; las velocidades de diseño no son una medición instantánea de la pérdida con el peso actual.
\item[$V_s(f)$.] Referencia interpolada: empieza en $V_{s1}$ y se acerca a $V_{s0}$ al extender flaps. Se calcula localmente.
\item[$\mathcal R(\alpha;\alpha_0,\alpha_s)$.] Vale cero antes del inicio, aumenta linealmente y llega a uno en el ángulo de pérdida. Representa cuánto se ha avanzado dentro de la zona de buffet.
\item[$(IAS/V_s(f))^2$ y 1.5.] Escalan la envolvente con velocidad. La división elimina las unidades; elevar al cuadrado aproxima una dependencia cuadrática. El factor se limita a 1.5 y después la envolvente total se limita a uno.
\item[$A_s$, $\bar A_s$ y $g_s$.] $A_s$ es la envolvente calculada y $\bar A_s$ su versión suavizada. Con $g_s=$ \textbf{Buffet perdida}, se usa $u_{s,P}=Gg_s\bar A_sT_{13}$ y $u_{s,R}=u_{s,P}/2$.
\end{description}
Si el ángulo está a mitad del intervalo entre inicio y pérdida, y $IAS=V_s(f)$, resulta $A_s=0.5$. Con total 60\% y ganancia 0.240, una vez estabilizado el filtro los picos nominales son unos 18 PWM en pitch y 9 en roll. Esto describe el cálculo, no demuestra que ese umbral coincida con el buffet natural del R182.

''',
r'\section{Buffet de spoilers}':r'''
\subsection{Explicación sencilla}
Se multiplica cuánto tren está extendido por cuánto avanzó la velocidad dentro del intervalo elegido para el buffet.
\begin{description}
\item[$g_{\rm ext}$.] \textbf{SimConnect:} \code{GEAR LEFT POSITION}, entre cero y uno. Es extensión física representada por el simulador, \textbf{no una ganancia del menú}, aunque se utilice la letra $g$.
\item[$IAS$ y $\mathcal R(IAS;100,150)$.] IAS es \code{AIRSPEED INDICATED}, en kt. La rampa es cero a 100 kt o menos, 0.5 a 125 kt y uno a 150 kt o más. Los números 100/150 son parámetros locales del efecto.
\item[$A_g,\bar A_g$.] Son la envolvente resultante y su versión suavizada. Por ejemplo, medio tren y media rampa producen $A_g=0.25$.
\item[$g_g,G,T_{10}$.] $g_g$ es \textbf{Buffet tren}; $G$ es Ganancia total; $T_{10}$ es la triangular de 10 Hz generada en la Pico. No son datos del simulador.
\item[$u_{g,P},u_{g,R}$.] Comandos de este buffet: pitch recibe el producto completo y roll la mitad.
\end{description}
También se consulta \code{IS GEAR RETRACTABLE} para confirmar tren retráctil y \code{SIM ON GROUND} para exigir que el avión esté en vuelo. Con tren completo e IAS=125 kt, la envolvente es 0.5. Con total 60\% y Buffet tren=0.160, los picos nominales estabilizados son unos 12 PWM pitch y 6 roll.

''',
r'\section{Vibración de rodaje}':r'''
\subsection{Explicación sencilla}
La intensidad depende de cuánto spoiler está extendido y de la velocidad. No depende del movimiento del yugo.
\begin{description}
\item[$s_L,s_R$.] \textbf{SimConnect:} \code{SPOILERS LEFT POSITION} y \code{SPOILERS RIGHT POSITION}, entre cero y uno. Su promedio describe la extensión conjunta usada por el algoritmo.
\item[$IAS$ y 80/140.] IAS es \code{AIRSPEED INDICATED}, en kt. Los umbrales 80 y 140 son parámetros del algoritmo: a 110 kt la rampa vale 0.5.
\item[$A_{sp},\bar A_{sp}$.] Envolvente calculada y filtrada. Si el promedio de extensión es 0.5 y la rampa también 0.5, $A_{sp}=0.25$.
\item[$g_{sp},G,T_{15}$.] Son \textbf{Buffet spoiler}, Ganancia total y triangular de 15 Hz. La Pico aplica $u_{sp,P}=Gg_{sp}\bar A_{sp}T_{15}$ y entrega la mitad en roll.
\end{description}
\code{SIM ON GROUND} debe indicar vuelo. Si las posiciones son inválidas, no se inventa una extensión: el resultado se anula. Con total 60\%, ganancia 0.160 y envolvente 0.25, serían aproximadamente 6 PWM pitch y 3 roll en un avión equipado. \textbf{En el addon R182 analizado, este ejemplo es sólo demostrativo: no tiene spoilers y la salida esperada es cero.}

''',
r'\section{Amortiguación y fricción}':r'''
\subsection{Explicación sencilla}
El rodaje aumenta su intensidad al aumentar la velocidad sobre el suelo. Usa una onda fija para representar vibración; no está leyendo cada bache de la pista.
\begin{description}
\item[$V_{\rm suelo}$.] \textbf{SimConnect:} \code{GROUND VELOCITY}, en kt. Es velocidad respecto al suelo, diferente de \code{AIRSPEED INDICATED}.
\item[Condición de tierra.] \code{SIM ON GROUND} debe valer uno. Si está en vuelo o la velocidad no supera 0.5 kt, $A_t=0$.
\item[0.5 y 39.5.] 0.5 kt es el umbral para empezar. El divisor 39.5 es $40-0.5$: la rampa llega a uno a 40 kt. Son elecciones del programa.
\item[$A_t,\bar A_t,T_8$.] Son intensidad calculada, intensidad filtrada y onda triangular de 8 Hz. La onda es interna a la Pico, no proviene de la pista simulada.
\item[$g_t,G$ y $40/255$.] $g_t$ es \textbf{Vib. suelo}; $G$ es Ganancia total. El producto se limita a $40/255$ de comando normalizado para que roll no supere 40 PWM.
\item[$u_{t,R},u_{t,P}$ y 10.] El comando pitch es una décima parte del comando roll antes de convertir a enteros. La división por 10 es la relación entre ejes elegida para este efecto, no una reducción mecánica.
\end{description}
Por ejemplo, a 20.25 kt la envolvente vale 0.5. Con total 60\% y Vib. suelo=0.180, una vez estabilizado el filtro, roll alcanza aproximadamente 14 PWM y pitch 1 PWM. Al detenerse se borra inmediatamente el efecto, sin esperar a que desaparezca lentamente el filtro.

''',
r'\subsection{Damper}':r'''
\subsubsection{Explicación sencilla}
Se calcula qué tanto cambió la posición desde la muestra anterior y se divide por el tiempo transcurrido. Es la misma idea de velocidad igual a distancia dividida por tiempo, pero la posición está normalizada.
\begin{description}
\item[$x_n,x_{n-1}$.] Posición actual y anterior obtenidas de \code{YOKE X POSITION} o \code{YOKE Y POSITION}. El subíndice $n$ identifica la muestra, no una unidad de medida.
\item[$\Delta t$.] Tiempo entre muestras nuevas recibidas por el firmware, en segundos. No se deriva una velocidad nueva al reutilizar el mismo paquete.
\item[$v_{\rm bruto}$.] Velocidad calculada antes del filtro. Si la posición pasa de 0 a 0.2 en 0.1 s, resulta 2 recorridos centro--extremo por segundo. No son m/s ni rad/s.
\item[$-10,10$.] Límites numéricos de velocidad; evitan que un salto muy grande genere valores arbitrarios.
\item[$v_n,v_{n-1}$ y 0.040.] Velocidades filtradas actual y anterior. La constante de 0.040 s suaviza los cambios de la derivada. La velocidad que usan los efectos es $v_n$.
\end{description}
El estimador se reinicia ante pausas largas de datos y detecta detención e inversión de sentido. Estos pasos evitan arrastrar una velocidad ficticia. \textbf{No hay una SimVar de velocidad del yugo leída por el puente}: se calcula a partir de las dos posiciones mencionadas.

''',
r'\subsection{Fricción suavizada}':r'''
\subsubsection{Explicación sencilla}
El damper se parece a un amortiguador: ofrece poca resistencia cuando se mueve lento y más cuando se mueve rápido, hasta su techo. La posición por sí sola no lo activa.
\begin{description}
\item[$v_a$ y $m_a$.] $v_a$ es la velocidad filtrada calculada a partir de \code{YOKE X/Y POSITION}. $m_a=\max(|v_a|-0.02,0)$ descuenta una pequeña zona muerta de 0.02 recorridos/s para no responder a movimientos mínimos.
\item[$g_d$.] Es la ganancia \textbf{Damper}. Multiplica esa rapidez útil: con igual velocidad, mayor ganancia significa mayor resistencia antes de saturar.
\item[$\min(g_dm_a,0.4)$.] Limita la contribución de movimiento a 0.4 antes de aplicar la ganancia total y la relación de ejes. Es un límite de software, no una fuerza de 0.4 N.
\item[$G,c_a,-\sgn(v_a)$.] $G$ es Ganancia total. $c_P=1$ y $c_R=0.5$ hacen que roll tenga la mitad de amplitud para igual velocidad. El signo negativo hace que el comando se oponga al movimiento, independientemente de la posición del mando.
\item[9.98 y $PWM_{\max,a}$.] La máxima velocidad permitida es 10; al restar la zona muerta queda $10-0.02=9.98$. Se inserta en la fórmula para hallar una cota de comando y luego $\mathcal P$ la convierte a PWM. No es el PWM que se produce a cualquier velocidad.
\end{description}
Con $v_P=1$, $m_P=0.98$, Damper=0.120 y total 60\%, resulta $u_{d,P}=-0.07056$, aproximadamente $-18$ PWM. Si el movimiento se invierte, cambia el signo; si se detiene, el estimador acaba en cero y desaparece la resistencia.

''',
r'\subsection{Suma, fundamento y procedencia}':r'''
\subsubsection{Explicación sencilla}
La fricción suavizada también se opone al movimiento, pero su intensidad se aproxima a un valor casi constante al aumentar la rapidez. Se diferencia del damper porque no crece linealmente durante todo el intervalo previo al límite.
\begin{description}
\item[$v_a$ y $m_a$.] Son los mismos valores calculados a partir de posiciones SimConnect que utiliza el damper. No se recibe una fuerza de fricción desde Prepar3D.
\item[$g_f$.] Es la ganancia \textbf{Friccion}, conservada aunque se desactive el efecto.
\item[$m_a/(0.2+m_a)$.] Es un factor entre cero y casi uno. Vale cero en reposo; con $m_a=0.2$ vale 0.5; con velocidades mayores se aproxima a uno. El 0.2 tiene las mismas unidades de velocidad normalizada que $m_a$ y fija la transición suave.
\item[$0.4,G,c_a,-\sgn(v_a)$.] El 0.4 limita la magnitud del término; $G$ aplica Ganancia total; $c_a$ establece la relación entre ejes y el signo se opone al movimiento. Son parámetros u operaciones del programa.
\item[$h$ y $PWM_{\max,a}$.] Aquí $h=9.98/(0.2+9.98)$ es el mayor factor suavizado permitido por el límite de velocidad. No es el residuo de viento $h_i$ de turbulencia. Se usa sólo para calcular el máximo teórico.
\end{description}
Con $v_P=1$, $m_P=0.98$, Friccion=0.030 y total 60\%, el comando es aproximadamente $-0.01495$, unos $-4$ PWM. Por eso con esas ganancias es normal obtener números pequeños. Al mantener el mando quieto en una posición desplazada, este efecto se anula: no simula la fuerza necesaria para romper una adherencia estática.

''',
r'\section{Ganancias y rangos de salida}':r'''
\subsubsection{Explicación sencilla de la suma de resistencias}
Dentro del paréntesis se suman el término de damper $g_dm_a$ y el de fricción $g_fm_a/(0.2+m_a)$. Después se aplica \textbf{una sola vez} el techo 0.4. No se pueden sumar los dos máximos aislados para afirmar que juntos generan 0.8. Finalmente se aplican sentido, ganancia total y factor de eje.

$u_{m,a}v_a\leq0$ expresa que el comando resistente no apunta en el mismo sentido que el movimiento. Es una propiedad del modelo normalizado, no una medición de potencia eléctrica. En la interpretación física $F_d=-B\dot x$, $F_d$ sería una fuerza, $B$ un coeficiente viscoso y $\dot x$ una velocidad física. Esos valores con unidades todavía no están identificados para el mecanismo; la ganancia Damper es adimensional y no debe llamarse directamente $B$ en N\,s/m.

'''
}
for anchor,extra in insertions.items():
 assert s.count(anchor)==1,(anchor,s.count(anchor))
 s=s.replace(anchor,extra+anchor)
p.write_text(s,encoding='utf-8')
