%% Analisis en regimen de un barrido de duty (Capturar_Barrido.m)
% Con la tension medida en bornes y la velocidad del tacometro de cada duty:
%   - Ke: pendiente de (v - R_motor*I) en funcion de omega (ecuacion electrica en regimen);
%   - Tc y B: ordenada y pendiente de Kt*I en funcion de omega (ecuacion mecanica en
%     regimen, Kt = Ke);
%   - J: ajuste de omega(t) = wf*(1 - exp(-t/tau)) a los puntos del tacometro de todos los
%     escalones de cada duty (indicativo: la tension del arranque no es un escalon ideal;
%     el valor final de J sale de Ajuste_Modelo_Captura.m).
% Correr desde Modelado_Motor_DC. Las figuras se guardan en ../registros/capturas.

Parametros_Motor_FFB          % R_motor, L, J, B, Tc (valores de comparacion)

archivo_barrido = '';         % '' = el barrido mas reciente
t_regimen = 0.4;              % s: desde aca se considera regimen permanente

%% Lectura
carp = carpeta_capturas();    % ../registros/capturas (ROLL) o capturas_<motor>
if isempty(archivo_barrido)
    lb = [dir(fullfile(carp, 'barrido_*.mat')); dir(fullfile(carp, 'reg', 'barrido_*.mat'))];
    if isempty(lb), error('No hay barridos en %s.', carp); end
    [~, k] = max([lb.datenum]); archivo_barrido = fullfile(lb(k).folder, lb(k).name);
end
Sb = load(archivo_barrido); archivos = Sb.archivos;
fprintf('Barrido %s: %d capturas\n', archivo_barrido, numel(archivos));
n = numel(archivos);
res = struct('archivo', archivos(:), 'duty', NaN, 'v', NaN, 'I', NaN, 'w', NaN, 'w_desv', NaN, ...
             'npuntos', 0, 'tau', NaN, 'wf', NaN, 'Rs', NaN);
for k = 1:n
    f = fullfile(carp, archivos{k}); if ~isfile(f), f = fullfile(carp, 'reg', archivos{k}); end
    S = load(f, 'cap'); c = S.cap;
    if ~(isfield(c, 'tension_en_bornes') && c.tension_en_bornes)
        warning('%s no tiene la tension medida en bornes: Ke saldria afectado por el driver.', archivos{k});
    end
    s = c.t > t_regimen; m = c.taco_t > t_regimen;
    res(k).duty = c.duty; res(k).v = mean(c.v(s)); res(k).I = mean(c.i(s));
    res(k).w = mean(c.taco_rpm(m))*pi/30; res(k).w_desv = std(c.taco_rpm(m))*pi/30;
    res(k).npuntos = numel(c.taco_t);
    % constante de tiempo con los puntos del tacometro (todos los escalones juntos)
    tt = c.taco_t(:); ww = c.taco_rpm(:)*pi/30;
    modelo = @(p, t) p(1)*(1 - exp(-t/p(2)));
    p = fminsearch(@(p) sum((modelo(p, tt) - ww).^2), [res(k).w, 0.015]);
    res(k).wf = p(1); res(k).tau = p(2);
    % Caida de la alimentacion con la corriente (fuente + driver): durante el arranque la
    % tension en bornes baja cuando sube la corriente; la pendiente -dv/di actua como una
    % resistencia en serie Rs que alarga la constante de tiempo: tau = J*(R+Rs)/(Ke*Kt).
    ma = c.t > 0.004 & c.t < 0.15;
    pr = polyfit(c.i(ma), c.v(ma), 1); res(k).Rs = max(-pr(1), 0);
    res(k).cap = c;
end
[~, o] = sort([res.duty]); res = res(o);

%% Regresiones en regimen
w = [res.w]'; I = [res.I]'; v = [res.v]';
pk = polyfit(w, v - R_motor*I, 1);              % Ke y caida fija (escobillas)
Ke_m = pk(1); Vesc = pk(2);
Ke0 = w \ (v - R_motor*I);                       % Ke forzando ordenada cero
pf = polyfit(w, Ke_m*I, 1);                      % B y Tc
B_m = pf(1); Tc_m = pf(2);
rk = corrcoef(w, v - R_motor*I); rf = corrcoef(w, Ke_m*I);
tau_m = [res.tau]'; Rt = R_motor + [res.Rs]';
J_m = tau_m.*(Ke_m^2 + Rt*B_m)./Rt;

fprintf('\n duty  V bornes   I [A]   taco [rpm]  desv  puntos  tau [ms]  Rs [ohm]  J [kg.m2]\n');
for k = 1:numel(res)
    fprintf('%4g %%  %6.2f V  %6.3f   %7.0f    %4.0f   %4d    %5.1f     %5.3f    %.2e\n', res(k).duty, ...
        res(k).v, res(k).I, res(k).w*30/pi, res(k).w_desv*30/pi, res(k).npuntos, ...
        res(k).tau*1e3, res(k).Rs, J_m(k));
end
fprintf('\nKe = %.5f V.s/rad (R2 %.5f), caida fija %.3f V | forzando ordenada 0: Ke = %.5f\n', ...
        Ke_m, rk(1,2)^2, Vesc, Ke0);
fprintf('Rozamiento: Tc = %.2e N.m (%.3f A), B = %.2e N.m.s/rad (R2 %.3f)\n', Tc_m, Tc_m/Ke_m, ...
        B_m, rf(1,2)^2);
fprintf('J (constante de tiempo del tacometro): media %.2e, desvio %.2e kg.m2\n', mean(J_m), std(J_m));
fprintf('Valores actuales del modelo: Ke %.4f, Tc %.2e, B %.2e, J %.2e\n', Ka, Tc, B, J);
barrido = struct('archivo', archivo_barrido, 'res', rmfield(res, 'cap'), 'Ke', Ke_m, ...
    'Vesc', Vesc, 'Ke0', Ke0, 'Tc', Tc_m, 'B', B_m, 'J', J_m, 'R_motor', R_motor);
[~, base_] = fileparts(archivo_barrido);
save(fullfile(carp, ['analisis_' base_ '.mat']), 'barrido');

%% Figuras
f1 = figure('Name', 'Ke y rozamiento', 'Position', [50 50 1000 420]);
subplot(1,2,1)
plot(w*30/pi, v - R_motor*I, 'o', w*30/pi, polyval(pk, w), '-'); grid on
xlabel('Velocidad del tacometro [rpm]'); ylabel('v - R_{motor} I [V]')
title(sprintf('K_e = %.4f V s/rad', Ke_m)); legend('medido (cada duty)', 'recta ajustada', 'Location', 'northwest')
subplot(1,2,2)
plot(w*30/pi, Ke_m*I*1e3, 'o', w*30/pi, polyval(pf, w)*1e3, '-'); grid on
xlabel('Velocidad del tacometro [rpm]'); ylabel('K_t I [mN m]')
title(sprintf('\\tau_c = %.2f mN m,  B = %.2e N m s/rad', Tc_m*1e3, B_m))
legend('medido (cada duty)', 'recta ajustada', 'Location', 'northwest')
set(findall(f1, 'Type', 'axes'), 'Toolbar', []);    % sin la barra de herramientas
exportgraphics(f1, fullfile(carp, ['ke_rozamiento_' base_ '.png']), 'Resolution', 130);

f2 = figure('Name', 'Aceleracion', 'Position', [80 80 1000 450]); hold on
col = lines(numel(res));
for k = 1:numel(res)
    c = res(k).cap; tt = linspace(0, 0.3, 300);
    plot(c.taco_t*1e3, c.taco_rpm, '.', 'Color', col(k,:));
    plot(tt*1e3, res(k).wf*(1 - exp(-tt/res(k).tau))*30/pi, '-', 'Color', col(k,:), 'LineWidth', 1.2);
end
hold off; grid on; xlim([0 300]); xlabel('t [ms]'); ylabel('Velocidad [rpm]')
leg = arrayfun(@(r) {sprintf('%g %%: tacometro', r.duty), sprintf('%g %%: \\tau = %.1f ms', r.duty, r.tau*1e3)}, res, 'uni', 0);
legend([leg{:}], 'Location', 'southeast')
title('Puntos del tacometro de todos los escalones y ajuste de primer orden')
set(findall(f2, 'Type', 'axes'), 'Toolbar', []);
exportgraphics(f2, fullfile(carp, ['aceleracion_' base_ '.png']), 'Resolution', 130);
fprintf('Figuras y resultados guardados en %s\n', carp);
