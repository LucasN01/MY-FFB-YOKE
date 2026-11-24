%% Genera Control_Corriente_FFB.slx copiando el bloque "Motor DC" de MotorModeloFFB.slx
% No pisa un modelo existente: borrarlo o renombrarlo antes de regenerar.
d = fileparts(mfilename('fullpath')); m = 'Control_Corriente_FFB'; f = fullfile(d,[m '.slx']);
if isfile(f), error('Ya existe %s: renombrarlo antes de regenerar.',f); end
load_system(fullfile(d,'..','Modelado_Motor_DC','MotorModeloFFB.slx'));
load_system('simulink');
new_system(m);
S = 'simulink/';
A = @(src,n,pos,varargin) add_block([S src],[m '/' n],'Position',pos,varargin{:});

%% Nivel superior
A('Sources/From Workspace','Comando +-1',[20 185 100 215],'VariableName','cmd_ws','Interpolate','off','OutputAfterFinalValue','Holding final value','SampleTime','Ts_cmd');
A('Discontinuities/Saturation','Recorte',[130 185 165 215],'UpperLimit','cmd_max','LowerLimit','-cmd_max');
A('Math Operations/Gain','I_FS',[195 182 245 218],'Gain','I_FS');
A('Signal Attributes/Rate Transition','2 ms a Ts',[270 185 310 215],'OutPortSampleTime','Ts');
A('Math Operations/Sum','Error',[340 190 360 210],'Inputs','|+-','IconShape','round');
A('Ports & Subsystems/Subsystem','PI corriente',[400 170 510 260]);
A('Discrete/Unit Delay','Retardo calculo',[545 175 585 215],'SampleTime','Ts');
A('Ports & Subsystems/Subsystem','BTS7960 PWM',[620 165 720 225]);
add_block('MotorModeloFFB/Motor DC',[m '/Motor DC'],'Position',[770 150 880 310]);
% Entradas de carga agregadas solo a esta copia: T_L (par externo) y J_L (inercia), en el eje del motor
mo=[m '/Motor DC']; C=@(src,n,pos,varargin) add_block([S src],[mo '/' n],'Position',pos,varargin{:});
C('Sources/In1','T_L',[20 420 50 434]); C('Sources/In1','J_L',[20 470 50 484]);
pj=get_param([mo '/J'],'Position');
C('Math Operations/Sum','J total',[pj(3)+12 pj(2)-2 pj(3)+24 pj(4)+18],'Inputs','++','IconShape','rectangular','ShowName','off');
set_param([mo '/Add1'],'Inputs','-+--'); add_line(mo,'T_L/1','Add1/4','autorouting','smart');
pc=get_param([mo '/Divide1'],'PortHandles'); delete_line(get_param(pc.Inport(2),'Line'));
con(mo,{'J/1','J total/1';'J_L/1','J total/2';'J total/1','Divide1/2'});
A('Sources/From Workspace','T_L en el eje',[540 250 620 276],'VariableName','tl_ws','Interpolate','off','OutputAfterFinalValue','Holding final value');
A('Math Operations/Gain','1 sobre N',[650 248 700 278],'Gain','1/N_red');
A('Sources/Constant','J_L reflejada',[620 315 710 339],'Value','J_eje/N_red^2');
A('Math Operations/Gain','Kt',[940 60 985 90],'Gain','Km');
A('Math Operations/Gain','Reductora N',[1010 60 1060 90],'Gain','N_red');
A('Ports & Subsystems/Subsystem','Sensor ACS712 + ADC',[600 385 730 445],'Orientation','left');
A('Discrete/Zero-Order Hold','Encoder 2 ms',[770 480 820 510],'SampleTime','Ts_cmd','Orientation','left');
A('Signal Attributes/Rate Transition','w a Ts',[690 480 730 510],'OutPortSampleTime','Ts','Orientation','left');
A('Math Operations/Gain','Ka (FCEM)',[590 477 650 513],'Gain','Ka*usar_FCEM','Orientation','left');
A('Signal Routing/Mux','Mux corrientes',[1150 20 1155 80],'Inputs','3');
A('Sinks/Scope','Scope control',[1240 40 1290 220]);
set_param([m '/Scope control'],'NumInputPorts','4');
q = {'I_ref','I_real','I_adc','V_motor','w_motor','T_eje','theta_motor'};
for k=1:numel(q)
    A('Sinks/To Workspace',['a ' q{k}],[1180 300+45*k 1280 326+45*k],'VariableName',q{k},'SaveFormat','Timeseries');
end
borrar_contenido([m '/PI corriente']); borrar_contenido([m '/BTS7960 PWM']); borrar_contenido([m '/Sensor ACS712 + ADC']);

%% PI: v = Kp*e + integral + FCEM, saturado, anti-windup por recalculo
s=[m '/PI corriente']; B=@(src,n,pos,varargin) add_block([S src],[s '/' n],'Position',pos,varargin{:});
B('Sources/In1','e',[20 53 50 67]);
B('Sources/In1','FCEM',[20 213 50 227]);
B('Math Operations/Gain','Kp',[120 45 165 75],'Gain','Kp');
B('Math Operations/Sum','Suma aw',[120 125 140 145],'Inputs','|++','IconShape','round');
B('Discrete/Discrete-Time Integrator','Integral Ki',[180 115 235 155],'IntegratorMethod','Integration: Forward Euler','gainval','Ki','SampleTime','Ts');
B('Math Operations/Sum','Suma PI',[300 110 320 160],'Inputs','+++','IconShape','rectangular');
B('Discontinuities/Saturation','Limite Vmax',[370 120 405 150],'UpperLimit','Vmax','LowerLimit','-Vmax');
B('Math Operations/Sum','vsat - v',[450 270 470 290],'Inputs','|+-','IconShape','round');
B('Math Operations/Gain','1 sobre Kp',[220 262 290 298],'Gain','antiwindup/Kp','Orientation','left');
B('Sinks/Out1','v',[520 128 550 142]);
con(s,{'e/1','Kp/1';'e/1','Suma aw/1';'Kp/1','Suma PI/1';'Suma aw/1','Integral Ki/1';'Integral Ki/1','Suma PI/2'
 'FCEM/1','Suma PI/3';'Suma PI/1','Limite Vmax/1';'Limite Vmax/1','v/1';'Limite Vmax/1','vsat - v/1'
 'Suma PI/1','vsat - v/2';'vsat - v/1','1 sobre Kp/1';'1 sobre Kp/1','Suma aw/2'});

%% Puente H: PWM unipolar centrado, o tension promediada
s=[m '/BTS7960 PWM']; B=@(src,n,pos,varargin) add_block([S src],[s '/' n],'Position',pos,varargin{:});
B('Sources/In1','v*',[20 93 50 107]);
B('Math Operations/Gain','1 sobre Vbus',[85 85 140 115],'Gain','1/Vbus');
B('Math Operations/Abs','Abs',[175 85 205 115],'ZeroCross','off');
B('Math Operations/Bias','Pulso perdido',[230 85 275 115],'Bias','-t_perdido/Ts');
B('Discontinuities/Saturation','D 0..1',[300 85 335 115],'UpperLimit','1','LowerLimit','0','ZeroCross','off');
B('Sources/Repeating Sequence','Portadora',[300 155 340 185],'rep_seq_t','[0 Ts/2-t_adc Ts-t_adc Ts]','rep_seq_y','[1-2*t_adc/Ts 0 1 1-2*t_adc/Ts]');
B('Logic and Bit Operations/Relational Operator','Comparador',[385 95 420 130],'Operator','>');
B('Signal Attributes/Data Type Conversion','a double',[440 100 475 125],'OutDataTypeStr','double');
B('Math Operations/Sign','Signo',[175 20 205 50],'ZeroCross','off');
B('Sources/Constant','conmutado',[440 150 500 180],'Value','usar_conmutado');
B('Signal Routing/Switch','Elegir',[540 90 580 240],'Criteria','u2 ~= 0','ZeroCross','off');
B('Math Operations/Product','Prod',[620 60 650 100]);
B('Math Operations/Gain','Vbus',[680 65 725 95],'Gain','Vbus');
B('Sinks/Out1','v(t)',[760 73 790 87]);
con(s,{'v*/1','1 sobre Vbus/1';'1 sobre Vbus/1','Abs/1';'1 sobre Vbus/1','Signo/1';'Abs/1','Pulso perdido/1'
 'Pulso perdido/1','D 0..1/1';'D 0..1/1','Comparador/1';'Portadora/1','Comparador/2';'Comparador/1','a double/1'
 'a double/1','Elegir/1';'conmutado/1','Elegir/2';'D 0..1/1','Elegir/3';'Signo/1','Prod/1';'Elegir/1','Prod/2'
 'Prod/1','Vbus/1';'Vbus/1','v(t)/1'});

%% Sensor: ACS712 -> divisor RC -> ADC 12 bits -> amperes
% Los 2.5 V se suman despues del filtro para que arranque cargado.
s=[m '/Sensor ACS712 + ADC']; B=@(src,n,pos,varargin) add_block([S src],[s '/' n],'Position',pos,varargin{:});
B('Sources/In1','i',[20 63 50 77]);
B('Math Operations/Gain','ACS712 0.1 V por A',[80 52 130 88],'Gain','k_acs');
B('Continuous/Transfer Fcn','Divisor 4.7k 6.8k 2.2nF',[160 50 240 90],'Numerator','k_div','Denominator','[tau_div 1]');
B('Math Operations/Bias','Cero',[265 55 305 85],'Bias','V0_acs*k_div');
B('Discrete/Zero-Order Hold','Muestreo',[330 55 370 85],'SampleTime','Ts');
B('Sources/Random Number','Ruido',[330 130 370 160],'Variance','(ruido_A*k_acs*k_div)^2','SampleTime','Ts');
B('Math Operations/Sum','Suma',[395 60 415 80],'Inputs','|++','IconShape','round');
B('Discontinuities/Saturation','Rango 0-3.3 V',[440 55 475 85],'UpperLimit','3.3','LowerLimit','0');
B('Discontinuities/Quantizer','ADC 12 bits',[500 55 535 85],'QuantizationInterval','lsb_adc');
B('Math Operations/Bias','Resta cero',[560 55 600 85],'Bias','-V0_acs*k_div');
B('Math Operations/Gain','a amperes',[625 52 690 88],'Gain','1/(k_acs*k_div)');
B('Sinks/Out1','i medida',[720 63 750 77]);
con(s,{'i/1','ACS712 0.1 V por A/1';'ACS712 0.1 V por A/1','Divisor 4.7k 6.8k 2.2nF/1';'Divisor 4.7k 6.8k 2.2nF/1','Cero/1'
 'Cero/1','Muestreo/1';'Muestreo/1','Suma/1';'Ruido/1','Suma/2';'Suma/1','Rango 0-3.3 V/1'
 'Rango 0-3.3 V/1','ADC 12 bits/1';'ADC 12 bits/1','Resta cero/1';'Resta cero/1','a amperes/1';'a amperes/1','i medida/1'});

%% Conexiones del nivel superior
con(m,{'T_L en el eje/1','1 sobre N/1';'1 sobre N/1','Motor DC/2';'J_L reflejada/1','Motor DC/3'
 'Comando +-1/1','Recorte/1';'Recorte/1','I_FS/1';'I_FS/1','2 ms a Ts/1';'2 ms a Ts/1','Error/1'
 'Error/1','PI corriente/1';'PI corriente/1','Retardo calculo/1';'Retardo calculo/1','BTS7960 PWM/1'
 'BTS7960 PWM/1','Motor DC/1';'Motor DC/1','Kt/1';'Kt/1','Reductora N/1'
 'Motor DC/1','Sensor ACS712 + ADC/1';'Sensor ACS712 + ADC/1','Error/2'
 'Motor DC/2','Encoder 2 ms/1';'Encoder 2 ms/1','w a Ts/1';'w a Ts/1','Ka (FCEM)/1';'Ka (FCEM)/1','PI corriente/2'
 '2 ms a Ts/1','Mux corrientes/1';'Motor DC/1','Mux corrientes/2';'Sensor ACS712 + ADC/1','Mux corrientes/3'
 'Mux corrientes/1','Scope control/1';'Retardo calculo/1','Scope control/2';'Reductora N/1','Scope control/3';'Motor DC/2','Scope control/4'
 '2 ms a Ts/1','a I_ref/1';'Motor DC/1','a I_real/1';'Sensor ACS712 + ADC/1','a I_adc/1';'Retardo calculo/1','a V_motor/1'
 'Motor DC/2','a w_motor/1';'Reductora N/1','a T_eje/1';'Motor DC/3','a theta_motor/1'});
set_param(m,'SolverType','Variable-step','Solver','ode45','MaxStep','Ts/50','RelTol','1e-6','StopTime','t_fin_control', ...
    'InitFcn','if ~exist(''Kp'',''var'') || ~exist(''cmd_ws'',''var''), Parametros_Control; end', ...
    'ReturnWorkspaceOutputs','off','SignalLogging','off');
save_system(m,f); fprintf('Creado %s\n',f);
close_system('MotorModeloFFB',0);   % solo se leyo

function borrar_contenido(s)
    delete_line(find_system(s,'FindAll','on','SearchDepth',1,'Type','line'));
    b = find_system(s,'SearchDepth',1,'Type','block');
    for k=1:numel(b), if ~strcmp(b{k},s), delete_block(b{k}); end, end
end
function con(sys,w)
    for k=1:size(w,1), add_line(sys,w{k,1},w{k,2},'autorouting','smart'); end
end
