%% Caracterizacion completa de un motor DC 775 (pensado para el motor de PITCH)
% Hace con un motor nuevo el mismo camino que se hizo con el de ROLL:
%   1. parte de los parametros INICIALES (los medidos al principio, antes de pulir);
%   2. captura un barrido de escalones en vacio a varios duty (Capturar_Barrido.m);
%   3. en regimen saca Ke, Tc y B (Analizar_Barrido.m);
%   4. con Ke, Tc y B fijos ajusta R_motor y J con fmincon (Ajuste_Modelo_Captura.m);
%   5. dibuja medicion contra modelo y guarda los parametros finales.
% Las capturas y resultados de cada motor van a su propia carpeta (carpeta_capturas.m):
% ROLL en ../registros/capturas y PITCH en ../registros/capturas_pitch.
% Los parametros finales quedan en <carpeta>/parametros_<MOTOR>.mat; para simular ese
% motor en Simulink, poner motor = 'PITCH' en Parametros_Motor_FFB.m.
%
% Antes de correrlo (con el motor nuevo montado como estaba el de ROLL):
%   - sketch captura_escalon.ino cargado en el Pro Micro (monitor serie del IDE cerrado);
%   - divisor de tension en M+ con 0,1 uF en A1, tacometro LM393 en el pin 4 con su disco;
%   - fuente de 24 V y 20 A, eje LIBRE y sin carga; primero el USB y despues la fuente;
%   - en Capturar_Escalon.m: tension_en_bornes = true y usar_tacometro = true.
% Correr desde la carpeta Modelado_Motor_DC.

%% 1. Configuracion
cm.motor  = 'PITCH';   % nombre del motor: define la carpeta de capturas y el archivo final
cm.puerto = 'COM9';    % puerto del Pro Micro (serialportlist para verlo)

% Parametros INICIALES. Son los que se midieron al principio en ROLL, antes de pulir con
% las capturas (multimetro, LCR, taladro, Ensayos 3 y 5). Si el motor nuevo se mide con
% esos mismos ensayos, reemplazarlos aca por sus valores. El ajuste busca R y J dentro de
% +/-30 % de estos valores, de modo que conviene que esten cerca.
cm.p0.R_motor = 0.787;     % ohm: v/i con fuente continua a ~1 A, en bornes
cm.p0.R       = 0.869;     % ohm: circuito con el BTS7960 (solo para tension = duty*B+)
cm.p0.L       = 0.50e-3;   % H: LCR a 10 kHz (queda fija: no se ajusta)
cm.p0.Ka      = 0.0222;    % V.s/rad = N.m/A (Ke = Kt)
cm.p0.J       = 8.06e-6;   % kg.m2
cm.p0.B       = 1.76e-6;   % N.m.s/rad
cm.p0.Tc      = 6.0e-3;    % N.m

% Pasos a ejecutar. Para repetir solo el analisis de un barrido ya capturado, poner
% capturar = false (se usa el barrido mas reciente de la carpeta del motor).
cm.pasos.capturar = true;
cm.pasos.analizar = true;
cm.pasos.ajustar  = true;
cm.pasos.figuras  = true;

% Barrido (los mismos valores que se usaron con ROLL; ~15 min)
cm.barrido.duties = [20 30 40 50];  % %; en vacio a 24 V no pasar de 50
cm.barrido.reps   = 50;             % escalones por duty (1 a 100)
cm.barrido.N      = 20;             % ventana de 800 ms
cm.barrido.espera = 3;              % s entre escalones
cm.barrido.pausa  = 60;             % s entre duties

%% 2. Preparacion
cm.p = cm.p0;                                      % parametros en uso, se van actualizando
setappdata(0, 'caracterizacion', cm);
setappdata(0, 'motor_actual', upper(cm.motor));
cm.carpeta = carpeta_capturas();
setappdata(0, 'caracterizacion', cm);
fprintf('\n=== Caracterizacion del motor %s (carpeta %s) ===\n', upper(cm.motor), cm.carpeta);

try
    %% 3. Captura del barrido
    if cm.pasos.capturar
        fprintf(['\nPaso 1: barrido %s %%, %d escalones por duty. Eje libre, fuente de 24 V ' ...
                 'encendida, USB conectado.\n'], mat2str(cm.barrido.duties), cm.barrido.reps);
        input('Enter para empezar (Ctrl+C para cancelar): ', 's');
        setappdata(0, 'barrido_cfg_externa', cm.barrido);
        Capturar_Barrido
        cm = getappdata(0, 'caracterizacion');
    end

    %% 4. Regimen: Ke, Tc y B
    if cm.pasos.analizar
        fprintf('\nPaso 2: analisis en regimen (Ke, Tc, B)\n');
        archivo_barrido = '';                      % el mas reciente de la carpeta del motor
        Analizar_Barrido
        cm = getappdata(0, 'caracterizacion');
        if ~all(arrayfun(@(r) r.npuntos > 0, barrido.res))
            error('Hay duty sin puntos del tacometro: revisar el sensor y el disco.');
        end
        cm.regimen = barrido;
        cm.p.Ka = round(barrido.Ke, 5);
        cm.p.Tc = round(barrido.Tc, 6);
        cm.p.B  = round(barrido.B, 8);
        if cm.p.B <= 0 || cm.p.Tc <= 0
            error('Tc o B salieron negativos (Tc %.2e, B %.2e): revisar las capturas.', cm.p.Tc, cm.p.B);
        end
        setappdata(0, 'caracterizacion', cm);
    end

    %% 5. Dinamica: R_motor y J con Ke, Tc y B fijos
    if cm.pasos.ajustar
        % R y J se compensan entre si (el error casi no cambia a lo largo de un valle) y
        % fmincon puede detenerse antes del minimo. Por eso el ajuste se repite partiendo
        % del resultado anterior hasta que ningun valor cambie mas de 1 % (maximo 4 rondas).
        % Esto tambien corre los limites de +/-30 % si un valor quedo contra uno de ellos.
        setappdata(0, 'ajuste_cfg', struct('archivo_barrido', 'ultimo', 'ajustar_L', false, ...
            'ajustar_B', false, 'ajustar_Ke', false, 'ajustar_Tc', false, 'solo_comparar', false));
        for ronda_ = 1:4
            cm = getappdata(0, 'caracterizacion');
            fprintf('\nPaso 3, ronda %d: ajuste de R_motor y J con fmincon (unos minutos)\n', ronda_);
            Ajuste_Modelo_Captura
            cm = getappdata(0, 'caracterizacion');
            cambio_ = max(abs(ajuste.p./ajuste.p0 - 1));
            for k_ = 1:numel(ajuste.nombres)
                switch ajuste.nombres{k_}
                    case 'R', cm.p.R_motor = ajuste.p(k_);
                    case 'J', cm.p.J = ajuste.p(k_);
                end
            end
            cm.ajuste = ajuste; cm.rondas_ajuste = ronda_;
            setappdata(0, 'caracterizacion', cm);
            if cambio_ < 0.01, break; end
        end
        if cambio_ >= 0.01
            warning('El ajuste no se estabilizo en 4 rondas (ultimo cambio %.1f %%).', 100*cambio_);
        end
        cm.p.R_motor = round(cm.p.R_motor, 3); cm.p.J = round(cm.p.J, 9);
        setappdata(0, 'caracterizacion', cm);
    end

    %% 6. Figuras medicion contra modelo con los parametros finales
    if cm.pasos.figuras
        fprintf('\nPaso 4: figuras de medicion contra modelo\n');
        setappdata(0, 'ajuste_cfg', struct('archivo_barrido', 'ultimo', 'solo_comparar', true));
        Ajuste_Modelo_Captura
        cm = getappdata(0, 'caracterizacion');
    end
catch err_
    limpiar_caracterizacion();
    rethrow(err_);
end
limpiar_caracterizacion();

%% 7. Resultados
p = cm.p; p_inicial = cm.p0; motor = upper(cm.motor); fecha = char(datetime('now'));
info = struct('regimen', [], 'ajuste', []);
if isfield(cm, 'regimen'), info.regimen = cm.regimen; end
if isfield(cm, 'ajuste'),  info.ajuste  = cm.ajuste;  end
archivo_p = fullfile(cm.carpeta, ['parametros_' motor '.mat']);
save(archivo_p, 'p', 'p_inicial', 'motor', 'fecha', 'info');

nom = {'R_motor', 'L', 'Ka', 'J', 'B', 'Tc', 'R'};
uni = {'ohm', 'H', 'V.s/rad', 'kg.m2', 'N.m.s/rad', 'N.m', 'ohm'};
org = {'ajuste fmincon', 'fija (LCR)', 'regimen', 'ajuste fmincon', 'regimen', 'regimen', ...
       'sin cambio (circuito con driver)'};
fprintf('\n=== Motor %s: parametros iniciales y finales ===\n', motor);
fprintf('%-8s %12s %12s %8s   %s\n', 'Param.', 'Inicial', 'Final', 'Cambio', 'Origen');
for k_ = 1:numel(nom)
    a_ = p_inicial.(nom{k_}); b_ = p.(nom{k_});
    fprintf('%-8s %12.4g %12.4g %+7.1f%%   %s [%s]\n', nom{k_}, a_, b_, 100*(b_/a_ - 1), org{k_}, uni{k_});
end
tau_e = p.L/p.R_motor; tau_m = p.J*p.R_motor/(p.Ka^2 + p.R_motor*p.B);
fprintf('tau_e = %.2f ms, tau_mec = %.1f ms, Tc equivale a %.3f A\n', tau_e*1e3, tau_m*1e3, p.Tc/p.Ka);
if isfield(cm, 'ajuste')
    fprintf('Error RMS normalizado del ajuste: %.4f\n', cm.ajuste.error_final);
end
fprintf('\nGuardado en %s\n', archivo_p);
fprintf('Para simularlo: motor = ''%s'' en Parametros_Motor_FFB.m.\n', motor);
fprintf('Figuras: %s (ke_rozamiento_*, aceleracion_*, *_modelo.png)\n', cm.carpeta);

function limpiar_caracterizacion()
% Quita el estado compartido para que los otros scripts vuelvan a su comportamiento normal
for n = {'caracterizacion', 'ajuste_cfg', 'barrido_cfg_externa', 'motor_actual'}
    if isappdata(0, n{1}), rmappdata(0, n{1}); end
end
end
