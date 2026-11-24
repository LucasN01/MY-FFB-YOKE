%% Modelado de Motor DC
% By: Sergio Andrés Castaño Giraldo
% https://controlautomaticoeducacion.com/analisis-de-sistemas/modelo-de-motor-dc/

%clc
%clear all
%close all

% Valores de J, B, Tc, Ka y R_motor: barrido en vacio con tension en bornes y tacometro
% (barrido_20261123_182847: duty 20/30/40/50 % con 24 V, 50 escalones por duty; 2026-11-23).
% Ka, Tc y B salen del regimen (Analizar_Barrido.m: Ka R2 0.99994; Kt*I = Tc + B*w, R2 0.83);
% J y R_motor del ajuste fmincon con Ka, Tc y B fijos (Ajuste_Modelo_Captura.m,
% ajuste_20261123_191118; error RMS normalizado 0.0158).
% momento de inercia del rotor:
J  = 8.58e-06; % Kg.m^2
% Rozamiento del motor solo, modelo Coulomb + viscoso (Armstrong-Helouvry, Dupont y
% Canudas de Wit, Automatica 30(7), 1994). Validos de 1250 a 4870 rpm. El Ensayo 3 (2500 a
% 10300 rpm) daba Tc 6.0e-3 y B 1.76e-6: la pendiente baja a alta velocidad.
% constante de fricción viscosa del motor:
B  = 3.7e-6; % N.m.s
% fricción seca (Coulomb): par = Tc*tanh(w/w_tc) en el bloque Motor DC
Tc   = 4.7e-3; % N.m
w_tc = 1;      % rad/s, suaviza el cambio de signo en w = 0 (evita oscilaciones numéricas)
% fuerza electromotriz constante:
Ka = 0.0225; %V/rad/s
% par motor constante:
Km = Ka; % N.m/A
% R_motor: resistencia entre bornes con corriente alta (3 a 13 A, motor frio), del ajuste
% del barrido. Depende de la corriente por el contacto de escobillas: con fuente continua a
% ~1 A da 0.787 Ω (hoja Medicion R); a rotor trabado entre 2.6 y 5.7 A, 0.56 a 0.69 Ω.
R_motor = 0.66; %Ω
% resistencia eléctrica del CIRCUITO con el driver (bobinado + escobillas + BTS7960 +
% ACS712 + cables), ajustada con Ajuste_Modelo_Captura.m sobre 2 escalones en vacío al
% 40 % con 24 V (escalon_d40_N5_x2_20261122_194752 y _x4_20261122_205204; 2026-11-22).
R  = 0.8687; %Ω
% inductancia eléctrica:
L  = 0.50e-3;
%Voltaje Leido en los Terminales del motor
Vm = 9;

%% Motor a simular
% 'ROLL': valores de arriba. Otro motor ('PITCH'): se cargan los parametros que dejo
% Caracterizar_Motor.m en ../registros/capturas_<motor>/parametros_<MOTOR>.mat. Las
% capturas que usa Cargar_Ensayo3 tambien se buscan en la carpeta de ese motor.
motor = 'ROLL';
car_ = getappdata(0, 'caracterizacion');   % Caracterizar_Motor.m en curso: manda
pm_ = [];
if ~isempty(car_)
    motor = car_.motor; pm_ = car_.p;
elseif ~strcmpi(motor, 'ROLL')
    f_ = fullfile(['../registros/capturas_' lower(motor)], ['parametros_' upper(motor) '.mat']);
    if isfile(f_)
        S_ = load(f_, 'p'); pm_ = S_.p;
    else
        warning('No esta %s: se usan los valores de ROLL. Correr Caracterizar_Motor.m.', f_);
    end
end
setappdata(0, 'motor_actual', upper(motor));
if ~isempty(pm_)
    J = pm_.J; B = pm_.B; Tc = pm_.Tc; Ka = pm_.Ka; Km = Ka;
    R_motor = pm_.R_motor; R = pm_.R; L = pm_.L;
end
clear car_ pm_ f_ S_

%% Que se simula con la llave "Fuente" ABAJO (tension medida)
% 'ensayo3': Ensayo 3 (vacio, rampas hasta 24 V, 190 s).
% 'captura': escalon capturado con el Pro Micro (Capturar_Escalon.m).
fuente_medida   = 'captura';
archivo_captura = '';     % .mat de ../registros/capturas; '' = la captura mas reciente
% Eje del motor: false = libre; true = trabado (omega = 0, para los ensayos con rotor
% bloqueado). Tiene que coincidir con la captura; si no, Cargar_Ensayo3 avisa.
eje_trabado = false;


%% Función de transferencias
den = [L*J R*J+L*B R*B+Km*Ka];

if ~eje_trabado
    %Velocidad angular
    Gwv = tf(Km,den);
    %Corriente
    Giv = tf([J B],den);
    %Posición
    Gwt = tf(Km,conv(den,[1 0]));
else
    % Eje trabado: no gira y la corriente solo depende del circuito R-L
    Gwv = tf(0,1);
    Giv = tf(1,[L R]);
    Gwt = tf(0,1);
end

%% Señales medidas (bloques "Tension medida", "Corriente medida" y "Velocidad medida")
% Matrices de dos columnas: [tiempo (s), valor]. Valen cero hasta cargar un ensayo
% con Cargar_Ensayo3.m (que tambien pasa la llave "Fuente" a la tension medida).
% Ejemplo de corriente desde un CSV del osciloscopio midiendo el ACS712 (100 mV/A, 2.5 V en 0 A):
%   M = readmatrix('archivo.csv','NumHeaderLines',2);
%   i_med = [M(:,1) - M(1,1) + 2, (M(:,2) - 2.5)/0.1];
v_med = [0 0; 9 0];   % V
i_med = [0 0; 9 0];   % A
w_med = [0 0; 9 0];   % rad/s
% Bloque "Tacometro LM393": velocidad del tacometro en rpm (el bloque la pasa a rad/s)
rpm_taco = [0 0; 9 0];   % rpm


%Cargar valores de corriente de Ensayo 3
Cargar_Ensayo3;

% sim('MotorModelo6v.slx')

