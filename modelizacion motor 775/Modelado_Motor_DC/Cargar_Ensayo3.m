%% Carga el Ensayo 3 (vacio, rampas hasta 24 V) segun la posicion de la llave "Fuente"
% El modelo lo corre solo al apretar Run (callback InitFcn), asi que alcanza con
% mover la llave "Fuente" (doble clic) y simular:
%   - Llave ABAJO (tension medida): se reproduce todo el ensayo (190 s) con la
%     tension del ensayo, y se cargan la corriente y la velocidad medidas.
%   - Llave ARRIBA (pulso): el motor recibe el pulso de 24 V y, como referencia,
%     solo se carga el tramo del ensayo a 24 V (duty 100 %), en regimen, ubicado
%     dentro de cada pulso. Fuera de los pulsos la referencia vale cero.
% Con fuente_medida = 'captura' (en Parametros_Motor_FFB) y la llave ABAJO, en lugar
% del Ensayo 3 se carga un escalon capturado con el Pro Micro (Capturar_Escalon.m):
% su tension (duty*B+), la corriente promediada y la velocidad (AS5600 si la hay; si
% no, la estimada por FCEM; cero con eje trabado).
% El tiempo de simulacion lo fija este script (variable t_fin, usada como Stop Time),
% igual que el paso de guardado de "data" (variable Ts_data del bloque To Workspace).

% Parametros: si todavia no estan cargados, se corre Parametros_Motor_FFB, que hace
% clear all y al final vuelve a llamar a este script (por eso el return).
if ~exist('J','var')
    Parametros_Motor_FFB
    return
end
if ~exist('fuente_medida','var'), fuente_medida = 'ensayo3'; end
if ~exist('archivo_captura','var'), archivo_captura = ''; end
if ~exist('eje_trabado','var'), eje_trabado = false; end

archivo = '../registros/Ens3-Med2-ROLL/roll_E3_HOR_M2_20261115_142242_combinado.csv';
mdl = 'MotorModeloFFB';
load_system(mdl);
llave_pulso = strcmp(get_param([mdl '/Fuente'],'sw'),'1');   % 1 = arriba = pulso
Ts_data = 1e-4;                                % s, paso de guardado de data

if strcmp(fuente_medida, 'captura')
    if llave_pulso
        warning(['fuente_medida = ''captura'' necesita la llave "Fuente" ABAJO; con la llave ' ...
                 'arriba se simula el pulso de 24 V con la referencia del Ensayo 3.']);
    else
        %% Escalon capturado con el Pro Micro
        % Las capturas se buscan en registros/capturas y en registros/capturas/reg
        carp_ = carpeta_capturas();   % ../registros/capturas (ROLL) o capturas_<motor>
        lista = [dir(fullfile(carp_, 'escalon_*.mat')); dir(fullfile(carp_, 'reg', 'escalon_*.mat'))];
        if isempty(lista), error('No hay capturas en %s ni en su carpeta reg.', carp_); end
        if isempty(archivo_captura)
            [~, kmax] = max([lista.datenum]);
        else
            kmax = find(strcmp({lista.name}, archivo_captura), 1);
            if isempty(kmax), error('No se encontro la captura %s.', archivo_captura); end
        end
        archivo_captura = lista(kmax).name;
        S = load(fullfile(lista(kmax).folder, archivo_captura), 'cap');
        cap = S.cap;
        Nc = 1; if isfield(cap, 'N'), Nc = cap.N; end
        % Cada muestra es el promedio de una ventana de Nc*0,1 ms y su tiempo es el centro;
        % la tension se aplica desde el inicio de la ventana. El tiempo de simulacion
        % arranca en el inicio de la primera ventana (el escalon queda en 20 muestras).
        t_ini = cap.t - (Nc-1)*0.5e-4;
        t0 = t_ini(1);
        if isfield(cap, 'w_as5600') && ~isempty(cap.w_as5600)
            wc = cap.w_as5600; origen_w = 'AS5600';
        else
            wc = cap.w_est;    origen_w = 'estimada por FCEM';
        end
        % Tension medida en bornes: el driver queda fuera, el modelo usa la R del motor
        if isfield(cap, 'tension_en_bornes') && cap.tension_en_bornes && exist('R_motor', 'var') ...
                && R ~= R_motor
            R = R_motor;
            den = [L*J R*J+L*B R*B+Km*Ka];
            if eje_trabado, Giv = tf(1,[L R]); else, Gwv = tf(Km,den); Giv = tf([J B],den); end
            fprintf(['Captura con tension en bornes: el modelo usa R = R_motor = %.3f ohm ' ...
                     '(correr Parametros_Motor_FFB para volver a la R del circuito).\n'], R);
        end
        trab_cap = isfield(cap, 'eje_trabado') && cap.eje_trabado;
        if trab_cap, wc(:) = 0; origen_w = 'cero (eje trabado)'; end
        if trab_cap ~= eje_trabado
            if trab_cap, txt_eje = 'trabado'; else, txt_eje = 'libre'; end
            warning(['La captura es con eje %s pero eje_trabado = %d en Parametros_Motor_FFB: ' ...
                     'cambiarlo para que el modelo corresponda al ensayo.'], txt_eje, eje_trabado);
        end
        v_med = [t_ini - t0, cap.v];
        i_med = [cap.t - t0, cap.i];
        w_med = [cap.t - t0, wc];
        t_fin = cap.t(end) - t0 + (Nc-1)*0.5e-4;
        % Tacometro LM393 (bloque "Tacometro LM393"): rpm en los puntos medios entre flancos
        if isfield(cap, 'taco_t') && ~isempty(cap.taco_t)
            % puntos de todos los escalones juntos; los de igual tiempo se promedian
            [tu, ~, g] = unique(cap.taco_t);
            rpm_taco = [0 0; tu - t0, accumarray(g, cap.taco_rpm, [], @mean)];
            if rpm_taco(end,1) < t_fin, rpm_taco(end+1,:) = [t_fin, rpm_taco(end,2)]; end
        else
            rpm_taco = [0 0; t_fin 0];
        end
        Ts_data = 1e-5;                            % el arranque dura pocos ms
        fprintf('Llave ABAJO: captura %s (%.0f ms, escalon en t = %.1f ms, velocidad %s).\n', ...
                archivo_captura, t_fin*1e3, -t0*1e3, origen_w);
        clear S cap t_ini t0 wc trab_cap origen_w lista kmax Nc tu g
        return
    end
end
if eje_trabado
    warning('eje_trabado = true pero el Ensayo 3 y el pulso son con el eje libre.');
end

T = readtable(archivo);
T = T(strcmp(T.estado,'V'),:);          % solo el tramo con el motor alimentado
t = (T.tiempo_ms - T.tiempo_ms(1))/1000; % s, desde el inicio del tramo
duty = T.duty_pct;                       % %, con signo segun el sentido
rpm = T.rpm; rpm(isnan(rpm)) = 0;        % el tacometro no da sentido
w = sign(duty).*rpm*pi/30;

if ~llave_pulso
    %% Ensayo completo
    % Tension en bornes: v_est del registro (24*d - 1.2 V) subestima la real.
    % Se usa la recta calibrada con el regimen del propio ensayo (V = Ke*w + R*I en
    % cada escalon, sentido horario): V = 0.2567*duty - 1.09, error max 0.7 V.
    v = sign(duty).*max(0, 0.2567*abs(duty) - 1.09);
    v_med = [t v];
    i_med = [t T.i_med_A];
    w_med = [t w];
    rpm_taco = [t, sign(duty).*rpm];             % la velocidad del ensayo 3 ya es del tacometro
    t_fin = t(end);
    fprintf('Llave ABAJO: ensayo completo (%.0f s, %d muestras).\n', t_fin, numel(t));
else
    %% Solo el tramo a 24 V, dentro de cada pulso
    k = find(abs(duty) >= 99.9);                 % duty 100 %
    k = k(t(k) >= t(k(1)) + 2);                  % descarta los 2 s de llegada a 24 V
    i24 = T.i_med_A(k);  w24 = w(k);
    % Ventanas de los pulsos, leidas del Pulse Generator
    pg = [mdl '/Pulse Generator'];
    per = evalin('base', get_param(pg,'Period'));
    anc = evalin('base', get_param(pg,'PulseWidth'))/100*per;
    ret = evalin('base', get_param(pg,'PhaseDelay'));
    t_fin = 9;
    ti = []; ii = []; wi = [];
    for t0 = ret:per:t_fin
        t1 = min(t0 + anc, t_fin);
        tt = (t0:0.1:t1)';                       % muestras cada 100 ms, como el registro
        n = numel(tt);
        idx = mod((0:n-1)', numel(k)) + 1;       % recorre las muestras medidas a 24 V
        ti = [ti; t0-1e-6; tt; t1+1e-6];         %#ok<AGROW> cero justo fuera del pulso
        ii = [ii; 0; i24(idx); 0];               %#ok<AGROW>
        wi = [wi; 0; w24(idx); 0];               %#ok<AGROW>
    end
    i_med = [0 0; ti ii; t_fin 0];
    w_med = [0 0; ti wi; t_fin 0];
    rpm_taco = [w_med(:,1), w_med(:,2)*30/pi];   % tacometro del ensayo 3 a 24 V
    v_med = [0 0; t_fin 0];                      % no se usa con la llave arriba
    fprintf(['Llave ARRIBA: pulso de 24 V; referencia = %d muestras del ensayo a 24 V ' ...
             '(I media %.3f A, %.0f rpm).\n'], numel(k), mean(i24), mean(rpm(k)));
end
