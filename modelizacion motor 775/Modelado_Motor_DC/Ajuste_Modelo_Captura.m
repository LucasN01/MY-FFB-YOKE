%% Ajuste fino de los parametros del motor contra capturas del Pro Micro (fmincon)
% Version propia de la idea de Regresion_Modelo.m (Castano Giraldo), con estos cambios:
%   - compara la CORRIENTE medida (ACS712) y, si se pide, la VELOCIDAD del tacometro
%     LM393 (puntos de todos los escalones) o del AS5600;
%   - usa el mismo modelo que MotorModeloFFB: friccion seca Tc*tanh(w/w_tc), Ka = Km,
%     y eje trabado (w = 0) en las capturas hechas con rotor bloqueado;
%   - ajusta varias capturas a la vez (por ejemplo, un barrido de duty), cada una con su
%     tension medida como entrada.
% Se ajustan R y J; opcionalmente L, B, Ke (= Kt) y Tc. Con un barrido de varios duty,
% Ke, Tc y B quedan bien definidos porque cada duty da un punto distinto de velocidad.
% Correr desde la carpeta Modelado_Motor_DC. No modifica Parametros_Motor_FFB.m: los
% valores ajustados se informan y se guardan en ../registros/capturas/ajuste_*.mat.

Parametros_Motor_FFB          % valores iniciales: J, B, Tc, w_tc, Ka, R, L

%% Configuracion
% Capturas a ajustar: si archivo_barrido no esta vacio, se usan las capturas de ese
% barrido ('ultimo' = el mas reciente); si no, la lista "capturas".
archivo_barrido = 'ultimo';
capturas = {'escalon_d40_N20_x4_20261123_145124.mat', 'escalon_d40_N20_x4_20261123_151529.mat'};
ajustar_L  = false;    % L queda fija (0,5 mH, confirmada a rotor trabado)
ajustar_B  = false;    % B y Tc fijos: la recta de regimen (Analizar_Barrido) es mas directa
ajustar_Ke = false;    % Ke = Kt fijo: el de regimen (Analizar_Barrido) es mas directo
ajustar_Tc = false;    % rozamiento seco
margen     = 0.30;     % limites: valor inicial * (1 -/+ margen); B admite el doble
usar_tacometro = true; % suma al error la velocidad del tacometro (eje libre)
peso_taco  = 1;        % peso del tacometro respecto de la corriente (ambos normalizados)
% Encoder AS5600: true para sumar la velocidad medida (capturas con USAR_AS5600 = 1)
usar_as5600 = false;
peso_vel    = 1;
dt_sim      = 1e-5;    % s, paso de integracion (tau_e ~ 0,6 ms)
% true: no ajusta; solo compara las capturas con el modelo de Parametros_Motor_FFB.m
% (figuras <captura>_modelo.png, para la tesis)
solo_comparar = false;
% Caracterizar_Motor.m fija estas opciones (appdata 'ajuste_cfg'); tienen prioridad.
cfg_ = getappdata(0, 'ajuste_cfg');
if ~isempty(cfg_)
    for f_ = fieldnames(cfg_)', eval([f_{1} ' = cfg_.(f_{1});']); end
end

%% Lectura de las capturas
carpeta = carpeta_capturas();   % ../registros/capturas (ROLL) o capturas_<motor>
if ~isempty(archivo_barrido)
    if strcmp(archivo_barrido, 'ultimo')
        lb = [dir(fullfile(carpeta, 'barrido_*.mat')); dir(fullfile(carpeta, 'reg', 'barrido_*.mat'))];
        if isempty(lb), error('No hay barridos; poner archivo_barrido = '''' para usar "capturas".'); end
        [~, kb] = max([lb.datenum]); archivo_barrido = fullfile(lb(kb).folder, lb(kb).name);
    end
    Sb = load(archivo_barrido); capturas = Sb.archivos;
    fprintf('Barrido %s: %d capturas\n', archivo_barrido, numel(capturas));
end
C = struct('nombre', {}, 't', {}, 't_ini', {}, 'Tw', {}, 'v', {}, 'i', {}, 'w', {}, ...
           'taco_t', {}, 'taco_w', {}, 'trabado', {}, 'duty', {}, 'reps', {});
en_bornes = false(1, numel(capturas));
for k = 1:numel(capturas)
    % se busca en registros/capturas y en registros/capturas/reg
    archivo_k = fullfile(carpeta, capturas{k});
    if ~isfile(archivo_k), archivo_k = fullfile(carpeta, 'reg', capturas{k}); end
    if ~isfile(archivo_k), error('No se encontro la captura %s.', capturas{k}); end
    S = load(archivo_k, 'cap'); cap = S.cap;
    en_bornes(k) = isfield(cap, 'tension_en_bornes') && cap.tension_en_bornes;
    Nc = 1; if isfield(cap, 'N'), Nc = cap.N; end
    c = struct();
    c.nombre = capturas{k};
    c.duty = cap.duty; c.reps = 1; if isfield(cap, 'reps'), c.reps = cap.reps; end
    c.t = cap.t;
    c.Tw = Nc*1e-4;                                % ventana de cada muestra promediada
    c.t_ini = cap.t - (Nc-1)*0.5e-4;               % inicio de cada ventana
    c.v = cap.v; c.i = cap.i;
    c.trabado = isfield(cap, 'eje_trabado') && cap.eje_trabado;
    c.w = []; c.taco_t = []; c.taco_w = [];
    if usar_as5600 && ~c.trabado
        if ~isfield(cap, 'w_as5600') || isempty(cap.w_as5600)
            error(['%s no tiene velocidad del AS5600: capturarla con USAR_AS5600 = 1 en el ' ...
                   'sketch y usar_as5600 = true en Capturar_Escalon.m.'], capturas{k});
        end
        c.w = cap.w_as5600;
    end
    if usar_tacometro && ~c.trabado && isfield(cap, 'taco_t') && ~isempty(cap.taco_t)
        c.taco_t = cap.taco_t(:); c.taco_w = cap.taco_rpm(:)*pi/30;
    end
    C(end+1) = c; %#ok<SAGROW>
    fprintf('%s: %s, %.0f ms, %d muestras, %d puntos de tacometro\n', c.nombre, ...
            ternario(c.trabado,'eje trabado','eje libre'), (c.t(end) - c.t_ini(1))*1e3, ...
            numel(c.t), numel(c.taco_t));
end

% Con la tension medida en bornes del motor, el driver queda fuera y la R a ajustar es
% la del motor (con ACS712 y cables); con duty*B+, es la del circuito con el driver.
if any(en_bornes) && ~all(en_bornes)
    error(['No mezclar capturas con tension en bornes y con duty*B+: la R que se ajusta ' ...
           'es distinta en cada caso.']);
end
if all(en_bornes)
    R = R_motor;
    fprintf('Capturas con tension en bornes: se ajusta la R del motor (inicial %.3f ohm).\n', R);
elseif ajustar_Ke
    warning(['Ke con duty*B+ sale afectada por las perdidas del driver: conviene usar ' ...
             'capturas con tension en bornes.']);
end

%% Parametros a ajustar
nombres = {'R', 'J'}; p0 = [R, J];
if ajustar_L,  nombres{end+1} = 'L';  p0(end+1) = L;  end
if ajustar_B,  nombres{end+1} = 'B';  p0(end+1) = B;  end
if ajustar_Ke, nombres{end+1} = 'Ka'; p0(end+1) = Ka; end
if ajustar_Tc, nombres{end+1} = 'Tc'; p0(end+1) = Tc; end
fijos = struct('R', R, 'L', L, 'J', J, 'B', B, 'Ka', Ka, 'Tc', Tc, 'w_tc', w_tc);
lb = p0*(1 - margen); ub = p0*(1 + margen);
kB = strcmp(nombres, 'B'); lb(kB) = p0(kB)*0.2; ub(kB) = p0(kB)*3;   % B esta mal definido
esc = p0;                                          % fmincon trabaja con p/p0 (todo ~1)

obj = @(x) error_total(x.*esc, nombres, fijos, C, dt_sim, peso_vel, peso_taco);
e0 = obj(ones(size(p0)));
if solo_comparar
    fprintf('Error (RMS normalizado) del modelo de Parametros_Motor_FFB: %.4f\n', e0);
    for k = 1:numel(C)
        c = C(k); [i0, w0] = simular(p0, nombres, fijos, c, dt_sim);
        f = figure('Name', c.nombre, 'Position', [100 100 700 520]);
        subplot(2,1,1)
        plot(c.t*1e3, c.i, '.', c.t*1e3, i0, 'LineWidth', 1.2); grid on; xlim([-10 200])
        ylabel('Corriente [A]'); legend('medida', 'modelo', 'Location', 'northeast')
        title(sprintf('Escalon al %g %% (%d repeticiones)', c.duty, c.reps))
        subplot(2,1,2); leg = {};
        if ~isempty(c.taco_t), plot(c.taco_t*1e3, c.taco_w*30/pi, 'o', 'MarkerSize', 3); hold on; leg{end+1} = 'tacometro'; end
        plot(c.t*1e3, w0*30/pi, 'LineWidth', 1.2); hold off; grid on; xlim([-10 200])
        ylabel('Velocidad [rpm]'); xlabel('t [ms]'); legend([leg, {'modelo'}], 'Location', 'southeast')
        set(findall(f, 'Type', 'axes'), 'Toolbar', []);
        [~, nb_] = fileparts(c.nombre);
        exportgraphics(f, fullfile(carpeta, [nb_ '_modelo.png']), 'Resolution', 130);
    end
    return
end
% Paso de derivada numerica de 1e-3 (sobre p/p0 ~ 1): el promedio por vuelta del tacometro
% hace el error escalonado a escala muy fina, y con el paso por defecto (~1e-8) fmincon toma
% esos escalones como un minimo y se detiene antes de tiempo.
opciones = optimoptions('fmincon', 'Display', 'iter', 'MaxFunctionEvaluations', 600, ...
                        'FiniteDifferenceStepSize', 1e-3);
x = fmincon(obj, ones(size(p0)), [], [], [], [], lb./esc, ub./esc, [], opciones);
p = x.*esc; e1 = obj(x);

%% Resultados
fprintf('\nError (RMS normalizado): inicial %.4f -> ajustado %.4f\n', e0, e1);
for k = 1:numel(nombres)
    lim = '';
    if abs(p(k) - lb(k)) < 1e-3*abs(p0(k)) || abs(p(k) - ub(k)) < 1e-3*abs(p0(k))
        lim = '  <- CONTRA EL LIMITE: revisar la medicion, no aceptar el valor';
    end
    fprintf('  %-2s: %.4g -> %.4g (%+.1f %%)%s\n', nombres{k}, p0(k), p(k), 100*(p(k)/p0(k) - 1), lim);
end
fij = setdiff({'R','J','L','B','Ka','Tc'}, nombres);
fprintf('Fijos: %s.\n', strjoin(fij, ', '));
ajuste = struct('capturas', {capturas}, 'nombres', {nombres}, 'p0', p0, 'p', p, ...
                'error_inicial', e0, 'error_final', e1, 'usar_as5600', usar_as5600, ...
                'usar_tacometro', usar_tacometro, 'en_bornes', all(en_bornes));
archivo = fullfile(carpeta, sprintf('ajuste_%s.mat', char(datetime('now','Format','yyyyMMdd_HHmmss'))));
save(archivo, 'ajuste');
fprintf('Guardado en %s\n', archivo);
fprintf('Si los valores tienen sentido, pasarlos a Parametros_Motor_FFB.m (y a la tesis y la planilla).\n');

%% Graficos: medido, modelo inicial y modelo ajustado (una figura por captura)
for k = 1:numel(C)
    c = C(k);
    [i0, w0, t0w] = simular(p0, nombres, fijos, c, dt_sim);
    [i1, w1, t1w] = simular(p, nombres, fijos, c, dt_sim);
    figure('Name', c.nombre);
    nsub = 1 + ~c.trabado;
    subplot(nsub,1,1)
    plot(c.t*1e3, c.i, '.', c.t*1e3, i0, c.t*1e3, i1, 'LineWidth', 1.2); grid on
    ylabel('Corriente [A]'); legend('medida', 'modelo inicial', 'modelo ajustado', 'Location', 'best')
    title(strrep(c.nombre, '_', ' '))
    if ~c.trabado
        subplot(2,1,2); leg = {};
        if ~isempty(c.w), plot(c.t*1e3, c.w*30/pi, '.'); hold on; leg{end+1} = 'AS5600'; end
        if ~isempty(c.taco_t), plot(c.taco_t*1e3, c.taco_w*30/pi, 'o', 'MarkerSize', 3); hold on; leg{end+1} = 'tacometro'; end
        plot(c.t*1e3, w0*30/pi, c.t*1e3, w1*30/pi, 'LineWidth', 1.2); hold off; grid on
        leg = [leg, {'modelo inicial', 'modelo ajustado'}];
        ylabel('Velocidad [rpm]'); xlabel('t [ms]'); legend(leg, 'Location', 'best')
    end
    set(findall(gcf, 'Type', 'axes'), 'Toolbar', []);
    set(findall(gcf, 'Type', 'axes'), 'XLim', [-10 200]);   % el transitorio
    [~, nb_] = fileparts(c.nombre);
    exportgraphics(gcf, fullfile(carpeta, [nb_ '_ajuste.png']), 'Resolution', 130);
end

%% Funciones
function e = error_total(p, nombres, fijos, C, dt_sim, peso_vel, peso_taco)
% RMS de la corriente (normalizada por su pico) y, si hay, de la velocidad del AS5600 y
% de los puntos del tacometro (normalizadas por su maximo).
s = 0; n = 0;
for k = 1:numel(C)
    c = C(k);
    [im, wm, wt] = simular(p, nombres, fijos, c, dt_sim);
    m = c.t > 0;                                 % solo despues del escalon
    s = s + mean(((im(m) - c.i(m))/max(abs(c.i))).^2); n = n + 1;
    if ~isempty(c.w)
        s = s + peso_vel*mean(((wm(m) - c.w(m))/max(abs(c.w))).^2); n = n + peso_vel;
    end
    if ~isempty(c.taco_t)
        s = s + peso_taco*mean(((wt - c.taco_w)/max(c.taco_w)).^2); n = n + peso_taco;
    end
end
e = sqrt(s/n);
end

function [i_v, w_v, w_taco] = simular(p, nombres, fijos, c, dt)
% Simula el modelo con la tension medida y promedia en las mismas ventanas que el Arduino.
% w_taco: velocidad del modelo promediada entre flancos del tacometro, como la medicion
% (cada punto es la velocidad media de una vuelta, centrada en taco_t).
q = fijos;
for k = 1:numel(nombres), q.(nombres{k}) = p(k); end
tm = (c.t_ini(1):dt:c.t_ini(end) + c.Tw)';
vm = interp1(c.t_ini, c.v, tm, 'previous', 'extrap');
im = zeros(size(tm)); wm = im; i = 0; w = 0;
for k = 1:numel(tm)
    di = (vm(k) - q.R*i - q.Ka*w)/q.L;
    if c.trabado
        dw = 0;
    else
        dw = (q.Ka*i - q.Tc*tanh(w/q.w_tc) - q.B*w)/q.J;
    end
    i = i + dt*di; w = w + dt*dw; im(k) = i; wm(k) = w;
end
% promedio por ventana con la integral acumulada
Ii = cumtrapz(tm, im); Wi = cumtrapz(tm, wm);
i_v = (interp1(tm, Ii, c.t_ini + c.Tw) - interp1(tm, Ii, c.t_ini))/c.Tw;
w_v = (interp1(tm, Wi, c.t_ini + c.Tw) - interp1(tm, Wi, c.t_ini))/c.Tw;
w_taco = [];
if ~isempty(c.taco_t)
    % la vuelta dura 2*pi/w: se promedia el modelo en +-media vuelta alrededor de taco_t
    Th = cumtrapz(tm, wm);                         % angulo del modelo
    th_c = interp1(tm, Th, c.taco_t, 'linear', 'extrap');
    [Thu, iu] = unique(Th);                        % interp1 necesita valores distintos
    t_a = interp1(Thu, tm(iu), max(th_c - pi, Thu(1)), 'linear', 'extrap');
    t_b = interp1(Thu, tm(iu), th_c + pi, 'linear', 'extrap');
    w_taco = 2*pi./max(t_b - t_a, 1e-4);
end
end

function r = ternario(c, a, b)
if c, r = a; else, r = b; end
end
