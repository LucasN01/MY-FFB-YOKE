%% Figuras del capitulo de control de corriente (tesis/figuras/control_corriente)
close all; evalc('Parametros_Control'); m='Control_Corriente_FFB'; load_system(m);
dirf=fullfile(fileparts(mfilename('fullpath')),'..','..','tesis','figuras','control_corriente');
if ~isfolder(dirf), mkdir(dirf); end
guardar=@(n) print(gcf,fullfile(dirf,n),'-dpng','-r150');
az=[0 .35 .75]; ro=[.8 .25 .1];
%% validacion del metodo con el ejemplo del apunte (Tabla 7.3, pag. 7.41)
kPWM=60/5; kiI=2*pi*1000*2/kPWM; kpI=5.2e-3/2*kiI; GA=tf([kpI kiI],[1 0])*kPWM*tf(1,[5.2e-3 2]);
[~,pmA,~,wA]=margin(GA); fprintf('Apunte: kiI=%.1f kpI=%.3f cruce=%.1f Hz PM=%.1f\n',kiI,kpI,wA/2/pi,pmA);
%% Bode
w=2*pi*logspace(1,4,600); [mc,fc_]=bode(C_cont*G_cont,w); [md,fd]=bode(L_disc,w);
figure('Position',[50 50 900 560]);
subplot(2,1,1); semilogx(w/2/pi,20*log10(mc(:)),'Color',az,'LineWidth',1.5); hold on; semilogx(w/2/pi,20*log10(md(:)),'--','Color',ro,'LineWidth',1.5); grid on; yline(0,'k'); ylabel('Magnitud [dB]'); legend('Continuo (apunte)','Discreto con retardo de calculo');
subplot(2,1,2); semilogx(w/2/pi,fc_(:),'Color',az,'LineWidth',1.5); hold on; semilogx(w/2/pi,fd(:),'--','Color',ro,'LineWidth',1.5); grid on; ylim([-270 -60]); ylabel('Fase [grados]'); xlabel('Frecuencia [Hz]'); yline(-180,'k'); guardar('bode.png');
for f=[300 500 800 1000 1500 2000], LL=pid(L*2*pi*f,R*2*pi*f,0,0,Ts)*c2d(G_cont,Ts,'zoh')/tf('z',Ts); [gm,pm]=margin(LL); s=stepinfo(feedback(LL,1)); fprintf('fc %d PM %.1f GM %.1f t2 %.2f ov %.1f\n',f,pm,20*log10(gm),s.SettlingTime*1e3,s.Overshoot); end
%% escalones
sim(m); figure('Position',[50 50 900 700]);
subplot(3,1,1); plot(I_real.Time*1e3,I_real.Data(:),'Color',az); hold on; stairs(I_ref.Time*1e3,I_ref.Data(:),'k','LineWidth',1); grid on; ylabel('Corriente [A]'); legend('i','I*','Location','southwest');
subplot(3,1,2); stairs(V_motor.Time*1e3,V_motor.Data(:),'Color',az); grid on; ylabel('Tension pedida [V]');
subplot(3,1,3); plot(T_eje.Time*1e3,T_eje.Data(:),'Color',az); grid on; ylabel('Torque ideal [N m]'); xlabel('Tiempo [ms]'); guardar('escalones.png');
figure('Position',[50 50 900 420]); q=I_real.Time>11.9e-3&I_real.Time<13.1e-3; plot(I_real.Time(q)*1e3,I_real.Data(q),'Color',az); hold on; q=I_adc.Time>11.9e-3&I_adc.Time<13.1e-3; plot(I_adc.Time(q)*1e3,I_adc.Data(q),'.','Color',ro,'MarkerSize',12); q=I_ref.Time>11.9e-3&I_ref.Time<13.1e-3; stairs(I_ref.Time(q)*1e3,I_ref.Data(q),'k','LineWidth',1); grid on; ylabel('Corriente [A]'); xlabel('Tiempo [ms]'); legend('i instantanea','muestra del ADC','I*','Location','southeast'); guardar('detalle.png');
%% anti-windup
figure('Position',[50 50 900 420]); hold on; I_FS=10;
for aw=[1 0], antiwindup=aw; sim(m); if aw, c=az; e='-'; else, c=ro; e='--'; end, x=movmean(interp1(I_real.Time([true;diff(I_real.Time)>0]),I_real.Data([true;diff(I_real.Time)>0]),(0:2.5e-6:18e-3)'),20); plot((0:2.5e-6:18e-3)*1e3,x,e,'Color',c,'LineWidth',1.3); end
stairs(I_ref.Time*1e3,I_ref.Data(:),'k'); xlim([7.5 16.5]); ylim([-12.5 12.5]); grid on; ylabel('Corriente media [A]'); xlabel('Tiempo [ms]'); legend('con recalculo','sin anti-windup','I*','Location','northwest'); guardar('antiwindup.png');
%% carga externa
evalc('Parametros_Control'); eje_trabado=false; J_eje=.01; tl_ws=[0 0;.02 1.5;.05 -1.5]; sim(m);
figure('Position',[50 50 900 620]); subplot(3,1,1); plot(I_real.Time*1e3,I_real.Data(:),'Color',az); hold on; stairs(I_ref.Time*1e3,I_ref.Data(:),'k'); grid on; ylabel('Corriente [A]');
subplot(3,1,2); stairs([tl_ws(:,1);t_fin_control]*1e3,[tl_ws(:,2);tl_ws(end,2)],'Color',ro,'LineWidth',1.5); hold on; plot(T_eje.Time*1e3,T_eje.Data(:),'Color',az); grid on; ylabel('Torque [N m]'); legend('T_L externo','motor (ideal)');
subplot(3,1,3); plot(w_motor.Time*1e3,w_motor.Data(:)/N_red,'Color',az); grid on; ylabel('Velocidad del eje [rad/s]'); xlabel('Tiempo [ms]'); guardar('carga.png');
fprintf('carga: w eje min %.1f max %.1f rad/s\n',min(w_motor.Data)/N_red,max(w_motor.Data)/N_red);
%% diagramas
evalc('Parametros_Control'); s={'','/PI corriente','/BTS7960 PWM','/Sensor ACS712 + ADC','/Motor DC'}; n={'modelo','pi','pwm','sensor','motor_dc'};
for k=1:5, print(['-s' m s{k}],'-dpng','-r150',fullfile(dirf,['simulink_' n{k} '.png'])); end
