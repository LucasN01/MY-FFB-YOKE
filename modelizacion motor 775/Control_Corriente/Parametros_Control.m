%% Parametros y calculo del lazo de corriente (modelo Control_Corriente_FFB.slx)
% Correr este script antes de simular; despues se puede cambiar cualquier variable
% en el workspace (Kp, Ki, I_FS, usar_conmutado...) y volver a simular.

%% 1) Que se simula
if ~exist('eje','var'), eje = 'ROLL'; end   % 'ROLL' o 'PITCH'
eje_trabado    = true;    % true = rotor trabado; false = rotor libre
usar_conmutado = 1;       % 1 = PWM conmutado; 0 = tension promediada (k_PWM del apunte, mas rapido)
usar_FCEM      = 0;       % 1 = prealimentar Ka*w con la velocidad del encoder
antiwindup     = 1;       % 1 = recalculo; 0 = sin anti-windup
ruido_A        = 0.05;    % A rms del ACS712 (supuesto); 0 = sin ruido
t_perdido      = 0;       % s de pulso que pierde el driver por periodo (probar 1.5e-6)

%% 2) Motor: se leen de Parametros_Motor_FFB.m (solo la parte de valores medidos)
dir_ = fileparts(mfilename('fullpath'));
txt_ = fileread(fullfile(dir_,'..','Modelado_Motor_DC','Parametros_Motor_FFB.m'));
eval(txt_(1:strfind(txt_,'%% Motor a simular')-1));
if strcmpi(eje,'PITCH')
    f_ = fullfile(dir_,'..','registros','capturas_pitch','parametros_PITCH.mat');
    if isfile(f_), S_ = load(f_,'p'); p_ = S_.p;
        J=p_.J; B=p_.B; Tc=p_.Tc; Ka=p_.Ka; Km=Ka; R=p_.R; L=p_.L;
    else, warning('PITCH sin caracterizar: se usan los valores de ROLL.'); end
    N_red = 19.2; I_FS = 4;                 % I_FS: hipotesis a validar
else
    N_red = 13.7; I_FS = 5;                 % I_FS: hipotesis a validar
end

%% 3) Convertidor, muestreo y sensor
Vbus = 24; Dmax = 220/255; Vmax = Dmax*Vbus; % techo de PWM del firmware
cmd_max = 220/255;                           % recorte del comando (SIMCONNECT_EXTRA_LIMIT)
fs = 20e3; Ts = 1/fs;                        % PWM centrado y lazo de corriente
Ts_cmd = 2e-3;                               % lazo de efectos
k_acs = 0.1; V0_acs = 2.5;                   % ACS712-20A
k_div = 6.8/(4.7+6.8);                       % divisor 4.7k / 6.8k
tau_div = (4.7e3*6.8e3/(4.7e3+6.8e3))*2.2e-9;% filtro con 2.2 nF
lsb_adc = 3.3/4096;
t_adc = tau_div;                              % la muestra se toma t_adc despues del centro del tiempo apagado (compensa el RC)

J_eje = 0;                                   % kg m^2, inercia de reductora + mando en el eje de salida (sin medir)
tl_ws = [0 0];                               % [t, par externo en el eje de salida, N m]; + se opone al giro +

%% 4) Calculo del PI (apunte 7.9.4.1; con la salida en volts k_PWM = 1)
fc = 1000;  wc = 2*pi*fc;                    % cruce: fs/20
tau_e = L/R;                                 % polo que cancela el cero del PI (ec. 7.68)
Ki = wc*R;                                   % ec. 7.71
Kp = tau_e*Ki;                               % ec. 7.68

G_cont = tf(1,[L R]);  C_cont = tf([Kp Ki],[1 0]);              % para margin/bode/sisotool
z_ = tf('z',Ts);
L_disc = pid(Kp,Ki,0,0,Ts)*c2d(G_cont,Ts,'zoh')/z_;             % con 1 periodo de calculo
[~,PM_cont] = margin(C_cont*G_cont);
[GM_d,PM_d,~,wc_d] = margin(L_disc);
fprintf('%s: tau_e=%.3f ms  Kp=%.3f V/A  Ki=%.0f V/(A s)  PM continuo=%.0f  PM discreto=%.1f (%.0f Hz)  GM=%.1f dB\n',...
    eje,tau_e*1e3,Kp,Ki,PM_cont,PM_d,wc_d/2/pi,20*log10(GM_d));

%% 5) Comando de prueba: [tiempo, valor -1..+1], un valor cada 2 ms
rng(1);
esc_ = [0 1 1 -1 -1 1 1 0 0 .05 .05 -.05 -.05 0 0];              % escalones grandes y chicos
az_  = 2*rand(1,20)-1;                                           % al azar cada 2 ms
val_ = [esc_ az_ 0]';
cmd_ws = [(0:numel(val_)-1)'*Ts_cmd, val_];
t_fin_control = numel(val_)*Ts_cmd;
clear dir_ txt_ f_ S_ p_ z_ esc_ az_ val_
