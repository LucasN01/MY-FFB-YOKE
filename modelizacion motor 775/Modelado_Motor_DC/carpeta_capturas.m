function c = carpeta_capturas()
% Carpeta de capturas del motor en uso (appdata 'motor_actual', que fijan
% Parametros_Motor_FFB.m y Caracterizar_Motor.m):
%   ROLL  -> ../registros/capturas        (la de siempre)
%   PITCH -> ../registros/capturas_pitch  (y asi para otro nombre)
% Asi las capturas, barridos y ajustes de un motor no se mezclan con los del otro.
m = 'ROLL';
if isappdata(0, 'motor_actual'), m = getappdata(0, 'motor_actual'); end
if strcmpi(m, 'ROLL')
    c = '../registros/capturas';
else
    c = ['../registros/capturas_' lower(m)];
end
if ~isfolder(c), mkdir(c); end
end
