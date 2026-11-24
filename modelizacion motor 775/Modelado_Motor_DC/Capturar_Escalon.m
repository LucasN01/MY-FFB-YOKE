%% Captura escalones con el Pro Micro (sketch captura_escalon.ino) y los compara con el modelo
% Antes: cargar captura_escalon.ino en el Pro Micro, cerrar el monitor serie del IDE
% (MATLAB no puede abrir el puerto si el IDE lo tiene abierto), encender la fuente.
% Correr desde la carpeta Modelado_Motor_DC. Editar la configuracion de abajo.
%
% Se aplican "reps" escalones iguales, separados por "espera" segundos para que el
% motor se detenga, y se promedian muestra a muestra (como el modo Average de un
% osciloscopio): el ruido aleatorio baja en raiz(reps) sin deformar la respuesta,
% porque todos los escalones arrancan sincronizados con el PWM.

Parametros_Motor_FFB          % J, B, Tc, Ka, Km, R, L (hace clear all)

%% Configuracion del ensayo
puerto      = 'COM9';  % ver en el IDE (Herramientas > Puerto) o con serialportlist
duty        = 50;      % % del escalon (vacio a 24 V: <= 50; eje trabado: <= 15)
N           = 20;       % decimacion: 1 -> 40 ms a 10 kHz; 5 -> 200 ms; 20 -> 800 ms
reps        = 4;       % escalones iguales a promediar (1 a 20; el ruido baja en raiz(reps))
espera      = 3;       % s entre escalones (el motor tiene que quedar quieto: 3 s a 24 V)
eje_trabado = false;   % true si el rotor esta bloqueado (omega = 0)
calibrar_cero = true;  % calibra el cero del ACS712 antes de los escalones (motor quieto)
% Que mide A1 (divisor 27k / 5.6k):
%   false: la fuente B+; la tension del motor se calcula como duty*B+ (sobreestima: el
%          BTS7960 entrega ~0,6-0,8 V menos a 40 %).
%   true : los bornes del motor (divisor en M+, capacitor en A1 para promediar el PWM);
%          A1 ya es la tension media del motor, con las perdidas del driver incluidas.
%          En este caso el modelo usa R_motor (el driver queda fuera de la medicion).
tension_en_bornes = true;
tau_rc_bornes = 4.6e3*0.1e-6;  % s: R equivalente del divisor (27k||5.6k) * C de A1 (0,1 uF)
% Tension de la fuente (solo con tension_en_bornes = false): [] usa la medida en A1.
% Si el divisor no esta conectado, poner aca la tension de B+ medida con multimetro.
vbus_manual = [];
% Encoder AS5600: poner true SOLO si el sketch se cargo con USAR_AS5600 = 1
usar_as5600 = false;
relacion_as5600 = 1;   % vueltas del motor por vuelta del iman (1 si el iman va en el eje del motor)
% Tacometro LM393 en el pin 4 (sketch con USAR_TACOMETRO 1): velocidad a partir del
% tiempo entre flancos. Solo tiene sentido con el eje libre.
usar_tacometro    = true;
ranuras_tacometro = 1;  % marcas del disco por vuelta del motor (igual que RANURAS del ensayo 3)
% Ventana de suavizado de la corriente para di/dt en la velocidad estimada [s]
t_suav = 0.5e-3;

% Barrido de duty (Capturar_Barrido.m): sus valores reemplazan a los de arriba. Se pasan
% con setappdata porque Parametros_Motor_FFB puede hacer clear all.
car_ = getappdata(0, 'caracterizacion');                  % Caracterizar_Motor.m
if ~isempty(car_), puerto = car_.puerto; end
barr_ = getappdata(0, 'barrido_actual');
if isappdata(0, 'barrido_actual'), rmappdata(0, 'barrido_actual'); end   % se usa una sola vez
if ~isempty(barr_)
    duty = barr_.duty; N = barr_.N; reps = barr_.reps; espera = barr_.espera;
    fprintf('Barrido: duty %g %%, N %d, %d escalones, espera %g s\n', duty, N, reps, espera);
end

%% Comunicacion
s = serialport(puerto, 115200, 'Timeout', 10 + espera + 0.04*N);
configureTerminator(s, 'LF');
pause(0.5); flush(s);
if calibrar_cero
    writeline(s, 'z'); disp(readline(s));
end
writeline(s, sprintf('e %g %d %d %g', duty, N, reps, espera));
crudo = {}; actual = strings(0);
while true
    l = strtrim(readline(s));
    if l == "LISTO", break, end
    if startsWith(l, "# ERROR"), clear s; error(char(l)); end
    if l == "FIN"
        crudo{end+1} = actual(:); actual = strings(0); %#ok<SAGROW>
        fprintf('Escalon %d de %d recibido\n', numel(crudo), reps);
    else
        actual(end+1) = l; %#ok<SAGROW>
    end
end
clear s
if isempty(crudo), error('No llego ninguna captura.'); end

%% Lectura de cada escalon
nr = numel(crudo);
for r = 1:nr
    lin = crudo{r};
    cab = lin(startsWith(lin, '#'));
    disp(cab)
    % Sin corriente el ACS712 da VCC/2 (~512 cuentas del ADC). Si el cero quedo lejos,
    % el sensor no esta alimentado o su salida no llega a A0: la corriente no sirve.
    cero = sscanf(extractAfter(cab(end), 'cero='), '%f', 1);
    if cero < 410 || cero > 615
        error(['Cero del ACS712 = %.1f cuentas (deberia ser ~512, o sea ~2,5 V en A0). ' ...
               'Revisar VCC (5 V), GND y OUT -> A0 del ACS712 y repetir.'], cero);
    end
    if contains(cab(end), 'disparo=1')
        warning('Escalon %d: se disparo el corte por sobrecorriente; revisar duty y conexiones.', r);
    end
    columnas = lin(startsWith(lin, 't_s'));
    if usar_as5600 && ~contains(columnas, 'ang_cuentas')
        error('usar_as5600 = true pero el Arduino no manda el angulo: cargar el sketch con USAR_AS5600 1.');
    end
    es_flanco = startsWith(lin, 'F,');            % flancos del tacometro
    datos = lin(~startsWith(lin, '#') & ~startsWith(lin, 't_s') & ~es_flanco);
    M = str2double(split(datos(:), ','));     % columna: split da una fila por muestra
    if r == 1
        n = size(M,1);
        cap.t = M(:,1); I = zeros(n,nr); VB = I; ANG = I;
        cap.cabecera = cab;
        FL = cell(1, nr);
    end
    FL{r} = str2double(extractAfter(lin(es_flanco), 'F,'));
    n = min(n, size(M,1));
    I(1:n,r) = M(1:n,2); VB(1:n,r) = M(1:n,3);
    if usar_as5600, ANG(1:n,r) = M(1:n,5); end
end
cap.t = cap.t(1:n); I = I(1:n,:); VB = VB(1:n,:); ANG = ANG(1:n,:);
dtc = cap.t(2) - cap.t(1);
cap.duty = duty; cap.N = N; cap.reps = nr; cap.eje_trabado = eje_trabado;

% Correccion del cero por escalon: antes de cada escalon hay muestras con el motor
% apagado (corriente cero). Su promedio es el desvio del cero del ACS712 en ese escalon
% y se resta. Corrige el corrimiento entre escalones de una misma tanda (en la captura
% 20261123_142034 llego a 0,3-0,5 A). No se usa la muestra mas cercana a t = 0.
pre = cap.t < -cap.t(2) + cap.t(1);
cap.i_cero = mean(I(pre,:), 1);
I = I - cap.i_cero;
fprintf('Cero de corriente por escalon (restado): %s A\n', mat2str(cap.i_cero, 2));
if any(abs(cap.i_cero) > 0.15)
    warning(['El cero del ACS712 se corrio mas de 0,15 A en algun escalon (%s A). Se corrigio, ' ...
             'pero conviene revisar GND y VCC del sensor.'], mat2str(cap.i_cero, 2));
end

% Si el ACS712 esta conectado al reves, la corriente sale negativa
if mean(mean(I(cap.t > 0,:))) < 0, I = -I; end

% Promedio de los escalones (modo Average) y dispersion entre escalones
cap.i_rep = I;
cap.i = mean(I, 2);
cap.vbus = mean(VB, 2);
if nr > 1
    cap.i_desvio = std(I, 0, 2);
    fprintf('Diferencia entre escalones: desvio medio %.3f A (el promedio de %d baja el ruido %.1f veces)\n', ...
        mean(cap.i_desvio), nr, sqrt(nr));
end

%% Control de la tension de la fuente
% Si A1 esta sin conectar, el ADC repite la tension del ACS712 (canal anterior):
% Vbus sigue a la corriente en lugar de quedar casi constante.
cs = corrcoef(cap.vbus, cap.i);
a1_suelta = cs(1,2) > 0.95 && max(cap.vbus) - min(cap.vbus) > 2;
if isempty(vbus_manual)
    if a1_suelta
        error(['A1 parece desconectada: la tension "medida" sigue a la corriente (r = %.3f). ' ...
               'Conectar el divisor (B+ -> 47k -> A1 -> 10k -> GND) o poner vbus_manual = <V de B+>.'], cs(1,2));
    end
else
    if ~a1_suelta
        fprintf('Aviso: se usa vbus_manual = %.2f V aunque A1 parece conectada (media medida %.2f V).\n', ...
            vbus_manual, mean(cap.vbus));
    end
    cap.vbus(:) = vbus_manual;
end
dutyReal = sscanf(extractAfter(cap.cabecera(end), 'duty_pct='), '%f', 1);
cap.tension_en_bornes = tension_en_bornes;
if tension_en_bornes
    % A1 mide M+ promediado por el filtro RC (tau_rc_bornes). Se compensa el filtro de
    % primer orden: v = v_A1 + tau*dv_A1/dt (con una media movil corta contra el ruido).
    if ~isempty(vbus_manual), error('vbus_manual no se usa con tension_en_bornes = true.'); end
    va1 = movmean(cap.vbus, 3);
    cap.v = va1 + tau_rc_bornes*gradient(va1, dtc);
    R = R_motor;                         % el driver queda fuera de la medicion
    fprintf(['Tension medida en bornes: media final %.2f V (duty*B+ hubiera supuesto ~%.2f V ' ...
             'con 23 V). Modelo con R_motor = %.3f ohm.\n'], mean(cap.v(cap.t > 0.8*cap.t(end))), ...
             0.23*dutyReal, R);
else
    cap.v = cap.vbus*dutyReal/100;
    % El BTS7960 a 20 kHz pierde ~9 % de duty en retardos de conmutacion: con eje trabado,
    % 12 % de 24 V dio 0,26 A (0,2 V efectivos) en lugar de 3,6 A, y en el Ensayo 3 no hubo
    % corriente hasta 11 %. Con duty bajo la tension real es mucho menor que duty*B+.
    if dutyReal < 100
        warning(['La tension del modelo es duty*B+ y el BTS7960 entrega menos (bastante menos ' ...
                 'por debajo de ~25 %). Para medir la tension real: tension_en_bornes = true ' ...
                 '(divisor en M+).']);
    end
end
cap.v(cap.t < 0) = 0;

%% Velocidad estimada con la fuerza contraelectromotriz
% De la ecuacion electrica: Ka*w = v - R*i - L*di/dt. La corriente se suaviza con
% una media movil de t_suav antes de derivar (el ACS712 tiene ruido y la derivada
% lo amplifica). Es poco confiable mientras la corriente es grande (arranque):
% un error de 5 % en R con 10 A da ~0,4 V, o sea ~170 rpm de error.
ns = max(1, round(t_suav/dtc));
is = movmean(cap.i, ns);
didt = gradient(is, dtc);
cap.w_est = (cap.v - R*is - L*didt)/Ka;
cap.w_est(cap.t < 0) = 0;
if eje_trabado, cap.w_est(:) = 0; end

%% Velocidad medida con el AS5600 (opcional): se calcula en cada escalon y se promedia
cap.w_as5600 = [];
if usar_as5600
    W5 = zeros(n, nr);
    for r = 1:nr
        a = ANG(:,r); a(a == 65535) = NaN;               % muestras sin lectura
        ok = ~isnan(a);
        if nnz(ok) < 10, error('El AS5600 no devolvio lecturas: revisar con el comando ? del sketch.'); end
        % Primero se "desenrolla" el angulo (paso de 4095 a 0) con las lecturas validas
        % y recien despues se interpola en las muestras que no tienen lectura.
        av = a(ok);
        d = diff(av); d(d > 2048) = d(d > 2048) - 4096; d(d < -2048) = d(d < -2048) + 4096;
        th = [0; cumsum(d)];
        theta = interp1(cap.t(ok), th, cap.t, 'linear', 'extrap')*2*pi/4096*relacion_as5600;
        w5 = gradient(movmean(theta, ns), dtc);
        if mean(w5(cap.t > 0)) < 0, w5 = -w5; end         % el sentido depende del montaje
        W5(:,r) = w5;
    end
    cap.w_as5600 = mean(W5, 2);
    cap.ang_cuentas = ANG;
end

%% Velocidad medida con el tacometro LM393 (opcional)
% Cada par de flancos consecutivos da la velocidad media en ese intervalo:
% rpm = 60/(dt*ranuras), asignada al punto medio del intervalo. Los puntos de todos los
% escalones se guardan juntos (taco_t, taco_rpm) y, para promediar, cada escalon se
% interpola sobre cap.t solo entre su primer y su ultimo punto (NaN fuera).
cap.taco_t = []; cap.taco_rpm = []; cap.w_taco = [];
if usar_tacometro && ~eje_trabado
    WT = nan(n, nr);
    for r = 1:nr
        f = sort(FL{r}(:)); f = f(f > 0);             % flancos despues del escalon
        if numel(f) < 2, continue, end
        tp = (f(1:end-1) + f(2:end))/2;
        rp = 60./(diff(f)*ranuras_tacometro);
        cap.taco_t = [cap.taco_t; tp]; cap.taco_rpm = [cap.taco_rpm; rp];
        if numel(tp) >= 2
            m = cap.t >= tp(1) & cap.t <= tp(end);
            WT(m, r) = interp1(tp, rp, cap.t(m))*pi/30;
        end
    end
    [cap.taco_t, o] = sort(cap.taco_t); cap.taco_rpm = cap.taco_rpm(o);
    if isempty(cap.taco_t)
        warning(['El tacometro no registro flancos: revisar D0 -> pin 4, VCC y GND del LM393, ' ...
                 'y que el sketch tenga USAR_TACOMETRO 1.']);
    else
        cap.w_taco = mean(WT, 2, 'omitnan');           % NaN donde ningun escalon tiene datos
        fprintf('Tacometro: %d puntos de velocidad; ultimo %.0f rpm (estimada por FCEM: %.0f rpm)\n', ...
            numel(cap.taco_t), cap.taco_rpm(end), cap.w_est(end)*30/pi);
    end
end

%% Guardado
carpeta = carpeta_capturas();       % ../registros/capturas (ROLL) o capturas_<motor>
if ~exist(carpeta, 'dir'), mkdir(carpeta); end
nombre = sprintf('escalon_d%g_N%d_x%d_%s%s', duty, N, nr, ...
    char(datetime('now','Format','yyyyMMdd_HHmmss')), ternario(eje_trabado,'_trabado',''));
save(fullfile(carpeta, [nombre '.mat']), 'cap', 'crudo');
tab = table(cap.t, cap.i, cap.vbus, cap.v, cap.w_est*30/pi, ...
    'VariableNames', {'t_s','i_A','vbus_V','v_motor_V','rpm_estimadas'});
for r = 1:nr, tab.(sprintf('i_A_rep%d', r)) = I(:,r); end
if usar_as5600, tab.rpm_as5600 = cap.w_as5600*30/pi; end
if ~isempty(cap.w_taco), tab.rpm_tacometro = cap.w_taco*30/pi; end
writetable(tab, fullfile(carpeta, [nombre '.csv']));
fprintf('Guardado en %s.mat/.csv\n', fullfile(carpeta, nombre));

%% Modelo con la tension medida como entrada
% Cada muestra es el promedio de N pares en una ventana de N*0,1 ms y su tiempo es el
% centro de la ventana. La tension se aplica desde el inicio de cada ventana, asi el
% escalon del modelo arranca en t = 0 (antes arrancaba (N-1)*0,05 ms tarde).
Tw = N*1e-4; t_ini = cap.t - (N-1)*0.5e-4;
dt = 1e-6; tm = (t_ini(1):dt:cap.t(end) + Tw/2)';
vm = interp1(t_ini, cap.v, tm, 'previous', 'extrap');   % mantiene el ultimo valor
i = 0; w = 0; im = zeros(size(tm)); wm = im;
for k = 1:numel(tm)
    di = (vm(k) - R*i - Ka*w)/L;
    if eje_trabado
        dw = 0;
    else
        dw = (Km*i - Tc*tanh(w/w_tc) - B*w)/J;
    end
    i = i + dt*di; w = w + dt*dw; im(k) = i; wm(k) = w;
end

%% Estimaciones a partir de la medicion (sobre el promedio)
p = cap.t > 0;
if eje_trabado
    % Corriente final y constante de tiempo electrica
    If = mean(cap.i(cap.t > 0.8*cap.t(end)));
    Vf = mean(cap.v(cap.t > 0.8*cap.t(end)));
    k = find(p & cap.i >= 0.632*If, 1);          % cruce del 63 %, interpolado
    t63 = interp1(cap.i(k-1:k), cap.t(k-1:k), 0.632*If);
    fprintf('Eje trabado: I final %.2f A, V media %.2f V -> R = %.3f ohm\n', If, Vf, Vf/If);
    fprintf('tau_e (63%%) = %.3f ms -> L = tau_e*R = %.3f mH (modelo: %.3f ms)\n', ...
        t63*1e3, t63*Vf/If*1e3, L/R*1e3);
else
    [Ip, kp] = max(cap.i);
    % El modelo se promedia en las mismas ventanas que la medicion para comparar picos
    im_v = arrayfun(@(a) mean(im(tm >= a & tm < a + Tw)), t_ini);
    [Ipm, kpm] = max(im_v);
    fprintf('Vacio: pico %.2f A a %.2f ms (modelo, promediado igual: %.2f A a %.2f ms)\n', ...
        Ip, cap.t(kp)*1e3, Ipm, cap.t(kpm)*1e3);
    if cap.t(end) > 0.1
        Iss = mean(cap.i(cap.t > 0.8*cap.t(end)));
        k63 = find(cap.t > cap.t(kp) & cap.i <= Iss + 0.368*(Ip - Iss), 1);
        fprintf('Caida de la corriente al 36.8%%: %.1f ms desde el pico (tau_mec del modelo: %.1f ms)\n', ...
            (cap.t(k63) - cap.t(kp))*1e3, J*R/(Km*Ka + R*B)*1e3);
        fprintf('Corriente de regimen: %.3f A (modelo %.3f A)\n', Iss, mean(im(tm > 0.8*tm(end))));
    else
        disp('Para ver la parte mecanica usar N >= 5 (ventana de 200 ms o mas).');
    end
    % Constante de tiempo de la velocidad (63 % del valor final)
    fuentes = {'estimada', cap.w_est};
    if usar_as5600, fuentes(end+1,:) = {'AS5600', cap.w_as5600}; end
    if cap.t(end) > 4*J*R/(Km*Ka + R*B)
        for f = 1:size(fuentes,1)
            wv = fuentes{f,2}; wf = mean(wv(cap.t > 0.8*cap.t(end)));
            k = find(p & wv >= 0.632*wf, 1);
            t63w = interp1(wv(k-1:k), cap.t(k-1:k), 0.632*wf);
            fprintf('Velocidad %s: final %.0f rpm, t63 = %.1f ms -> J = %.2e kg.m2 (modelo: %.0f rpm, %.1f ms)\n', ...
                fuentes{f,1}, wf*30/pi, t63w*1e3, t63w*(Km*Ka + R*B)/R, ...
                wm(end)*30/pi, J*R/(Km*Ka + R*B)*1e3);
        end
    end
end

%% Grafico
figure('Name', nombre);
subplot(3,1,1)
if nr > 1, plot(cap.t*1e3, I, 'Color', [0.75 0.75 0.75]); hold on; end
h1 = plot(cap.t*1e3, cap.i, '.-', 'Color', [0 0.45 0.74]); hold on
h2 = plot(tm*1e3, im, 'Color', [0.85 0.33 0.10]); hold off; grid on
ylabel('Corriente [A]')
legend([h1 h2], {sprintf('medida, promedio de %d (gris: cada escalon)', nr), 'modelo'}, 'Location', 'best')
title(strrep(nombre, '_', ' '))
subplot(3,1,2)
plot(cap.t*1e3, cap.w_est*30/pi, tm*1e3, wm*30/pi); hold on
leyenda = {'estimada (v - R i - L di/dt)/Ka', 'modelo'};
if usar_as5600
    plot(cap.t*1e3, cap.w_as5600*30/pi, '.-'); leyenda{end+1} = 'AS5600';
end
if ~isempty(cap.taco_t)
    plot(cap.taco_t*1e3, cap.taco_rpm, 'o', 'MarkerSize', 4); leyenda{end+1} = 'tacometro LM393';
end
hold off; grid on; ylabel('Velocidad [rpm]'); legend(leyenda, 'Location', 'best')
subplot(3,1,3)
plot(cap.t*1e3, cap.vbus, cap.t*1e3, cap.v); grid on
ylabel('Tension [V]'); xlabel('t [ms]')
if tension_en_bornes
    legend('bornes del motor (A1, filtrada)', 'bornes, filtro compensado (entrada del modelo)', ...
        'Location', 'best')
else
    legend(ternario(isempty(vbus_manual), 'fuente B+ (A1)', 'fuente B+ (vbus\_manual)'), ...
        'media en bornes (duty*B+)', 'Location', 'best')
end

function r = ternario(c, a, b)
if c, r = a; else, r = b; end
end
