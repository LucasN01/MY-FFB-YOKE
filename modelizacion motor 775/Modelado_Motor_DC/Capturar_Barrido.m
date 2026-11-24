%% Barrido de escalones en vacio a varios ciclos de trabajo
% Ejecuta Capturar_Escalon.m una vez por cada duty de la lista, con muchos escalones
% por duty, y guarda la lista de capturas en ../registros/capturas/barrido_<fecha>.mat
% para Analizar_Barrido.m y Ajuste_Modelo_Captura.m.
% Lo demas (puerto, tension en bornes, tacometro) se toma de Capturar_Escalon.m.
% Correr desde la carpeta Modelado_Motor_DC, con el eje libre.

%% Configuracion del barrido
cfg.duties = [20 30 40 50];  % %; en vacio a 24 V no pasar de 50 (pico ~ duty*24/0.87 A)
cfg.reps   = 50;             % escalones por duty (1 a 100)
cfg.N      = 20;             % ventana de 800 ms, una muestra cada 2 ms
cfg.espera = 3;              % s entre escalones (el motor tiene que detenerse)
cfg.pausa  = 60;             % s entre un duty y el siguiente (enfriamiento)
cfg.orden_alternado = true;  % 20, 40, 30, 50: reparte el calentamiento entre duties
% Caracterizar_Motor.m pasa su propia configuracion (appdata 'barrido_cfg_externa')
ext_ = getappdata(0, 'barrido_cfg_externa');
if ~isempty(ext_)
    for f_ = fieldnames(ext_)', cfg.(f_{1}) = ext_.(f_{1}); end
end

d = cfg.duties;
if cfg.orden_alternado, d = [d(1:2:end) d(2:2:end)]; end
cfg.duties_orden = d;
t_total = numel(d)*cfg.reps*(cfg.espera + 0.04*cfg.N + 1) + (numel(d)-1)*cfg.pausa;
fprintf('Barrido: duties %s, %d escalones cada uno. Duracion estimada: %.0f min.\n', ...
        mat2str(d), cfg.reps, t_total/60);

% Estado guardado fuera del workspace (Parametros_Motor_FFB puede hacer clear all)
setappdata(0, 'barrido_cfg', cfg);
setappdata(0, 'barrido_archivos', {});
for kb_ = 1:numel(cfg.duties_orden)
    cfg = getappdata(0, 'barrido_cfg');
    setappdata(0, 'barrido_actual', struct('duty', cfg.duties_orden(kb_), 'N', cfg.N, ...
        'reps', cfg.reps, 'espera', cfg.espera));
    setappdata(0, 'barrido_k', kb_);
    Capturar_Escalon                          % captura y guarda (variable "nombre")
    kb_ = getappdata(0, 'barrido_k'); cfg = getappdata(0, 'barrido_cfg');
    arch = getappdata(0, 'barrido_archivos'); arch{end+1} = [nombre '.mat'];
    setappdata(0, 'barrido_archivos', arch);
    if kb_ < numel(cfg.duties_orden)
        fprintf('Pausa de %d s antes del duty siguiente...\n', cfg.pausa); pause(cfg.pausa);
    end
end
cfg = getappdata(0, 'barrido_cfg'); archivos = getappdata(0, 'barrido_archivos');
nombre_barrido = sprintf('barrido_%s.mat', char(datetime('now','Format','yyyyMMdd_HHmmss')));
save(fullfile(carpeta_capturas(), nombre_barrido), 'cfg', 'archivos');
fprintf('Barrido terminado: %s (%d capturas).\n', nombre_barrido, numel(archivos));
rmappdata_seguro();

function rmappdata_seguro()
for n = {'barrido_actual', 'barrido_k', 'barrido_cfg', 'barrido_archivos'}
    if isappdata(0, n{1}), rmappdata(0, n{1}); end
end
end
