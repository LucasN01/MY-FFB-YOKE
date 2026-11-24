%% Compara el modelo con el Ensayo 3 (vacio, rampas hasta 24 V) del motor roll
% Carga el ensayo (Cargar_Ensayo3), simula MotorModeloFFB con la tension medida
% y grafica corriente y velocidad medidas contra el modelo. Correr desde Modelado_Motor_DC.

Parametros_Motor_FFB                       % parametros (y la carga elegida ahi)
fuente_medida = 'ensayo3';                 % este script compara siempre con el Ensayo 3
if eje_trabado, error('Comparar_Ensayo3 necesita eje_trabado = false en Parametros_Motor_FFB.'); end
Cargar_Ensayo3
sim('MotorModeloFFB');
t = data(:,1);

fig = figure('Position',[0 0 1100 650]);
subplot(2,1,1)
plot(i_med(:,1), i_med(:,2), '.', t, data(:,6))
ylabel('Corriente [A]'); ylim([-0.1 0.8]); grid on
legend('medida (ACS712, 100 ms)', 'modelo Motor DC (con Tc)', 'Location', 'northwest')
title('Ensayo 3, HOR M2: tension del ensayo aplicada al modelo')
subplot(2,1,2)
plot(w_med(:,1), w_med(:,2)*30/pi, '.', t, data(:,3)*30/pi, t, data(:,2)*30/pi, '--')
ylabel('Velocidad [rpm]'); xlabel('t [s]'); grid on
legend('medida (tacometro)', 'modelo Motor DC (con Tc)', 'Gwv (lineal, sin Tc)', 'Location', 'northwest')
exportgraphics(fig, 'comparacion_E3.png', 'Resolution', 110);
% Los picos de corriente en las rampas son reales: el duty sube de a 2.5 % cada 100 ms
% y cada salto produce un transitorio de pocos ms que el registro de 100 ms no ve.
