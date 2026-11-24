%% Margenes, Nyquist, lugar de raices y comprobacion en Simulink del lazo de corriente
% Figuras en tesis/figuras/control_corriente
close all; evalc('Parametros_Control');
dirf = fullfile(fileparts(mfilename('fullpath')),'..','..','tesis','figuras','control_corriente');
guardar = @(n) print(gcf,fullfile(dirf,n),'-dpng','-r150');
az=[0 .35 .75]; ro=[.8 .25 .1]; gr=[.4 .4 .4];
z = tf('z',Ts); s = tf('s'); Cd = pid(Kp,Ki,0,0,Ts);
lazo = @(R_,L_,k,H) k*Cd*c2d(H/(L_*s+R_),Ts,'zoh')/z;   % ganancias nominales, planta variable
H1 = tf(1); Hs = 1/(tau_div*s+1)/(s/(2*pi*80e3)+1);     % divisor RC y ACS712 (80 kHz)
w = logspace(1,log10(pi/Ts),40000);

fprintf('%-26s %7s %7s %7s %7s %6s %6s %8s\n','Caso','PM','GM dB','fgc','fpc','sm','Ms','DM us');
c = {'nominal',R,L,1,H1;'con divisor RC y ACS712',R,L,1,Hs;'R +30 %',1.3*R,L,1,Hs;'R -25 % (solo motor)',.66,L,1,Hs;
     'L +30 %',R,1.3*L,1,Hs;'L -30 %',R,.7*L,1,Hs;'L -30 %, R +30 %',1.3*R,.7*L,1,Hs;'Vbus +10 %',R,L,1.1,Hs;'Vbus -10 %',R,L,.9,Hs};
for k=1:size(c,1), informe(c{k,1},lazo(c{k,2:5}),w); end
for f=[300 500 800 1000 1500 2000]
    Lf = pid(L*2*pi*f,R*2*pi*f,0,0,Ts)*c2d(1/(L*s+R),Ts,'zoh')/z; informe(sprintf('fc=%d',f),Lf,w);
end
Ct = c2d(tf([Kp Ki],[1 0]),Ts,'tustin'); informe('PI por Tustin',Ct*c2d(1/(L*s+R),Ts,'zoh')/z,w);
Lz = lazo(R,L,1,H1); T = feedback(Lz,1);
fprintf('polos lazo cerrado (modulo): '); fprintf('%.4f ',abs(pole(T))); fprintf('\n'); disp(pole(T));
fprintf('ancho de banda -3 dB = %.0f Hz\n',bandwidth(T)/2/pi);
fprintf('rampa de FCEM 10 V/s: error analitico %.4f A\n',10/Ki);

%% Bode con margenes
[m,fa] = bode(Lz,w); m=20*log10(m(:)); fa=fa(:); [gm,pm,wpc,wgc]=margin(Lz);
figure('Position',[50 50 900 600]);
subplot(2,1,1); semilogx(w/2/pi,m,'Color',az,'LineWidth',1.5); hold on; grid on; yline(0,'k'); xlim([10 1e4]);
plot(wpc/2/pi*[1 1],[-20*log10(gm) 0],'Color',ro,'LineWidth',2.5); text(wpc/2/pi*1.08,-4,sprintf('MG = %.1f dB',20*log10(gm)),'Color',ro);
ylabel('Magnitud [dB]'); title(sprintf('Cruce de ganancia %.0f Hz, cruce de fase %.0f Hz',wgc/2/pi,wpc/2/pi));
subplot(2,1,2); semilogx(w/2/pi,fa,'Color',az,'LineWidth',1.5); hold on; grid on; yline(-180,'k'); xlim([10 1e4]); ylim([-270 -60]);
plot(wgc/2/pi*[1 1],[-180 -180+pm],'Color',ro,'LineWidth',2.5); text(wgc/2/pi*.28,-155,sprintf('MF = %.1f grados',pm),'Color',ro);
ylabel('Fase [grados]'); xlabel('Frecuencia [Hz]'); guardar('margenes.png');

%% Nyquist
h = squeeze(freqresp(Lz,w)); [sm,ii]=min(abs(1+h)); th=linspace(0,2*pi,200);
figure('Position',[50 50 640 560]); plot(real(h),imag(h),'Color',az,'LineWidth',1.6); hold on; plot(real(h),-imag(h),'--','Color',az);
plot(cos(th),sin(th),':','Color',gr); plot(-1+sm*cos(th),sm*sin(th),'Color',ro); plot(-1,0,'+','Color',ro,'MarkerSize',12,'LineWidth',2);
plot([-1 real(h(ii))],[0 imag(h(ii))],'Color',ro,'LineWidth',1.5); axis equal; axis([-2 1.2 -1.6 1.6]); grid on;
xlabel('Re L'); ylabel('Im L'); legend('L, frecuencias positivas','L, frecuencias negativas','circulo unidad',sprintf('radio s_m = %.2f',sm),'punto critico -1','Location','southeast'); guardar('nyquist.png');

%% Lugar de raices en z
figure('Position',[50 50 640 560]); [r,kk]=rlocus(Lz,linspace(0,5,3000)); plot(real(r).',imag(r).','Color',az,'LineWidth',1.3); hold on; plot(cos(th),sin(th),'k:');
p1=pole(T); h1=plot(real(p1),imag(p1),'s','Color',ro,'MarkerFaceColor',ro); pk=pole(feedback(gm*Lz,1)); h2=plot(real(pk),imag(pk),'kd','MarkerSize',9);
axis equal; axis([-.4 1.2 -1.1 1.1]); grid on; xlabel('Re z'); ylabel('Im z'); legend([h1 h2],{'diseno (k = 1)',sprintf('limite (k = %.2f)',gm)},'Location','southwest'); guardar('lugar_raices.png');

%% Simulink: escalon chico (sin saturar) con la ganancia multiplicada hasta el limite
md='Control_Corriente_FFB'; load_system(md); Kp0=Kp; Ki0=Ki; ruido_A=0; usar_conmutado=0; lsb_adc=1e-7; cmd_ws=[0 0;2e-3 .06]; t_fin_control=12e-3;
fg=figure('Position',[50 50 900 620]); kv=[1 2 3 3.5];
[gr_,~]=margin(lazo(R,L,1,1/(tau_div*s+1))); fprintf('limite previsto con RC: k=%.2f\n',gr_);
for j=1:4
    Kp=Kp0*kv(j); Ki=Ki0*kv(j); antiwindup=0; sim(md);
    figure(fg); subplot(2,2,j); plot(I_real.Time*1e3,I_real.Data(:),'Color',az); hold on; stairs(I_ref.Time*1e3,I_ref.Data(:),'k'); grid on; xlim([1.5 8]);
    if j<4, ylim([-.1 .75]); end
    title(sprintf('Ganancia x %.1f (%.1f dB)',kv(j),20*log10(kv(j)))); xlabel('ms'); ylabel('A');
    x=I_real.Data(I_real.Time>6e-3 & I_real.Time<12e-3); fprintf('k=%.1f: oscilacion pp entre 6 y 12 ms = %.4f A\n',kv(j),max(x)-min(x));
end
print(fg,fullfile(dirf,'margen_simulink.png'),'-dpng','-r150');

%% Ruido mayor en el modelo completo
for rn=[.05 .1 .2], evalc('Parametros_Control'); ruido_A=rn; cmd_ws=[0 .4]; t_fin_control=.06; usar_conmutado=0; sim(md);
    q=I_real.Time>.01; tt=.01:1e-5:.06; y=interp1(I_real.Time(q),I_real.Data(q),tt); fprintf('ruido %.2f A rms medido -> corriente %.4f A rms\n',rn,std(y(2:end-1))); end
evalc('Parametros_Control');

function informe(n,Lz,w)
    [gm,pm,wpc,wgc]=margin(Lz); sm=min(abs(1+squeeze(freqresp(Lz,w)))); e=isstable(feedback(Lz,1));
    fprintf('%-26s %7.1f %7.1f %7.0f %7.0f %6.2f %6.2f %8.1f  estable=%d\n',n,pm,20*log10(gm),wgc/2/pi,wpc/2/pi,sm,1/sm,pm*pi/180/wgc*1e6,e);
end
