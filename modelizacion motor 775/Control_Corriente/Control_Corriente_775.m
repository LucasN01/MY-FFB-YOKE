%% Lazo de corriente del motor DC 775 (ROLL): diseno y simulacion conmutada
% Metodo: apunte de catedra, sec. 7.9.4.1 (ec. 7.67-7.71), llevado a discreto.
% Parametros medidos: cap. 12 de la tesis / Parametros_Motor_FFB.m (23/11/2026).
clear; close all; clc
p.R = 0.869; p.L = 0.50e-3; p.Ke = 0.0225; p.J = 8.58e-6; p.B = 3.7e-6; p.Tc = 4.7e-3;
p.Vbus = 24; p.Dmax = 220/255;            % techo MOTOR_PWM_HARD_LIMIT
p.I_FS = 5;                               % A que representa el comando 1.0000 (ROLL, hipotesis)
p.cmax = 220/255;                         % SIMCONNECT_EXTRA_LIMIT: |comando| maximo real
p.fs = 20e3; p.Ts = 1/p.fs;               % PWM centrado = muestreo
p.fc = 1000;                              % cruce: fs/20
wc = 2*pi*p.fc; p.Kp = p.L*wc; p.Ki = p.R*wc;   % ec. 7.68 y 7.71 con kPWM = 1 (PI en volts)
p.ffR = 0;                                % 1 = sumar R*Iref (se compara abajo)
p.aw = 1;  % anti-windup: 0 = congelamiento (apunte 7.9.7), 1 = recalculo
p.tl = 0;                                 % s de pulso que pierde el driver por periodo
p.lsb = 3.3/4096/0.6/0.1; p.ruido = 0.05; % ADC 12 bits, divisor 0.6, ACS712 100 mV/A; ruido A rms supuesto

% --- margen discreto (ZOH + 1 periodo de calculo)
G = tf(1,[p.L p.R]); z = tf('z',p.Ts);
Lz = pid(p.Kp,p.Ki,0,0,p.Ts)*c2d(G,p.Ts,'zoh')/z;
[Gm,Pm,~,Wc] = margin(Lz);
fprintf('Kp=%.3f V/A  Ki=%.0f V/(A s)  PM=%.1f deg  GM=%.1f dB  fc=%.0f Hz  rizado max=%.2f A\n',...
  p.Kp,p.Ki,Pm,20*log10(Gm),Wc/2/pi,p.Vbus/(4*p.L*p.fs));
for f=[300 500 800 1000 1500 2000]
  L2 = pid(p.L*2*pi*f,p.R*2*pi*f,0,0,p.Ts)*c2d(G,p.Ts,'zoh')/z; [~,P2]=margin(L2);
  S=stepinfo(feedback(L2,1)); fprintf('  fc=%4d: PM=%.0f  t2%%=%.2f ms  sobrepico=%.1f%%\n',f,P2,S.SettlingTime*1e3,S.Overshoot);
end
fprintf('\nCorriente maxima en regimen a rotor trabado: %.1f A\n',p.Dmax*p.Vbus/p.R);
Im=p.Dmax*p.Vbus/p.R; te=p.L/p.R; A=p.cmax*p.I_FS;
fprintf('Minimo fisico (tension maxima): 0->%.2f A %.2f ms ; -%.2f->%.2f A %.2f ms\n',A,te*log(Im/(Im-A))*1e3,A,A,te*log((Im+A)/(Im-A))*1e3);

%% Escenarios. comando: un valor cada 2 ms
rng(1); nc = 40; azar = (2*rand(1,nc)-1)*p.cmax;
esc = [0 p.cmax*[1 1 -1 -1 1 1] 0 0];  chico=[0 .05 .05 -.05 -.05 0];
w0=@(t)0; mano=@(t) 100*sin(2*pi*2*t);     % rad/s en el eje del MOTOR
casos = {
 'A escalones max, trabado',        p, esc, w0
 'B azar cada 2 ms, trabado',       p, azar, w0
 'C como B, R +30%',  set_(p,'R',1.3*p.R), azar, w0
 'D como B, L -30%',  set_(p,'L',.7*p.L), azar, w0
 'E como B, L +30%',  set_(p,'L',1.3*p.L), azar, w0
 'F como B, driver pierde 1.5 us',  set_(p,'tl',1.5e-6), azar, w0
 'G como B, Vbus 21 V sin medir',   set_(p,'Vbus',21), azar, w0
 'H como B, la mano mueve 100 rad/s',p, azar, mano
 'I como B, mano 400 rad/s',        p, azar, @(t)400+0*t
 'J como B, rotor libre',           p, azar, []
 'K como B, ff R*Iref',             set_(p,'ffR',1), azar, w0
 'R como A, anti-windup congelamiento', set_(p,'aw',0), esc, w0
 'S como B, anti-windup congelamiento', set_(p,'aw',0), azar, w0
 'L como B, fc = 500',              set_(set_(p,'Kp',p.Kp/2),'Ki',p.Ki/2), azar, w0
 'M comandos 0.05',                 p, chico, w0
 'N comandos 0.05, pierde 1.5 us',  set_(p,'tl',1.5e-6), chico, w0
 'Q como B, I_FS = 15',             set_(p,'I_FS',15), azar, w0};
% En C-G cambia la planta; el PI conserva ganancias y Vbus nominales.
fprintf('\n%-36s %10s %10s %10s %10s\n','Caso','t5%% peor ms','t5%% med ms','err fin A','sobrep A');
RR={};
for k=1:size(casos,1)
  r = simular(casos{k,2},casos{k,3},casos{k,4},24); RR{k}=r; %#ok<SAGROW>
  m = metricas(r); fprintf('%-36s %10.2f %10.2f %10.3f %10.2f\n',casos{k,1},m);
end

%% Comparacion: ADS1115 (430 muestras/s por eje)
rA = promedio(p,azar,1); rO = promedio(p,azar,0); mA=metricas(rA); mO=metricas(rO);
fprintf('ADS1115 (prealim + I lenta, R real +15%%): error final max %.2f A, t5 medio %.2f ms; solo prealim: error final max %.2f A\n',mA(3),mA(2),mO(3));
figure('Position',[50 50 1000 800]);
subplot(3,1,1); pl(RR{1}); title('A: 0 / +4.31 / -4.31 A, rotor trabado (PI 1 kHz, PWM 20 kHz)');
subplot(3,1,2); pl(RR{2}); xlim([0 40]); title('B: comando al azar cada 2 ms');
subplot(3,1,3); plot(rA.t*1e3,rA.ref,'k',rA.t*1e3,rA.i,'r'); grid on; xlim([0 40]); ylabel('A'); xlabel('ms');
title('Mismo comando con ADS1115: prealimentacion R I* y correccion lenta'); legend('I*','I');
saveas(gcf,'lazo_corriente_escalones.png');
figure('Position',[50 50 1000 500]);
subplot(2,1,1); pl(RR{strncmp(casos(:,1),'M ',2)}); title('M: comandos de 0.05 (0.25 A), driver ideal');
subplot(2,1,2); pl(RR{strncmp(casos(:,1),'N ',2)}); title('N: idem, driver pierde 1.5 us por pulso');
saveas(gcf,'lazo_corriente_chico.png');

function p=set_(p,f,v), p.(f)=v; end
function pl(r), plot(r.t*1e3,r.i,'Color',[.6 .75 1]); hold on; plot(r.tm*1e3,r.im,'b.-'); stairs(r.tm*1e3,r.ref,'k','LineWidth',1);
  grid on; ylabel('A'); xlabel('ms'); legend('I instantanea','I media por periodo','I*'); end
function r=simular(p,cmd,wf,Vnom)
  sub=100; dt=p.Ts/sub; nk=round(numel(cmd)*2e-3/p.Ts); per=round(2e-3/p.Ts);
  i=0; w=0; integ=0; u=0; r.i=zeros(1,nk*sub); r.t=(1:nk*sub)*dt; r.im=zeros(1,nk); r.ref=r.im; r.u=r.im;
  for k=1:nk
    ref=max(min(cmd(ceil(k/per)),1),-1)*p.I_FS; tk=(k-1)*p.Ts;
    if ~isempty(wf), w=wf(tk); end
    im=round((i+p.ruido*randn)/p.lsb)*p.lsb;       % muestra en el borde (centro del tiempo apagado)
    e=ref-im; v=p.Kp*e+integ+p.ffR*p.R*ref;        % sin prealim. de FCEM: peor caso
    Vm=p.Dmax*Vnom; vs=max(min(v,Vm),-Vm);
    if p.aw, integ=integ+p.Ki*p.Ts*(e+(vs-v)/p.Kp);   % error que la tension real pudo atender
    elseif ~(v~=vs && sign(e)==sign(v)), integ=integ+p.Ki*p.Ts*e; end
    D=u/Vnom; u=vs;                                 % se aplica en el periodo siguiente
    Da=max(abs(D)-p.tl/p.Ts,0); a=0.5-Da/2; b=0.5+Da/2; s=0;
    for m=1:sub
      x=(m-.5)/sub; va=0; on=min(max(min(x+.5/sub,b)-max(x-.5/sub,a),0)*sub,1);
      va=sign(D)*p.Vbus*on;
      i=i+dt*(va-p.R*i-p.Ke*w)/p.L; s=s+i;
      if isempty(wf), tq=p.Ke*i-p.B*w-p.Tc*tanh(w/1); w=w+dt*tq/p.J; end
      r.i((k-1)*sub+m)=i;
    end
    r.im(k)=s/sub; r.ref(k)=ref; r.u(k)=u;
  end
  r.tm=(1:nk)*p.Ts; r.per=per; r.cond=isempty(wf);
end
function m=metricas(r)
  n=numel(r.ref)/r.per; t5=[]; ef=[]; ov=[]; last=0;
  for j=1:n
    q=(j-1)*r.per+(1:r.per); ref=r.ref(q(1)); st=ref-last; x=r.im(q);
    ef(end+1)=mean(x(end-3:end))-ref; %#ok<*AGROW>
    if abs(st)>0.4
      tol=max(.05*abs(st),.15); f=find(abs(x-ref)>tol,1,'last'); if isempty(f), f=0; end
      t5(end+1)=(f+1)*2/r.per; ov(end+1)=max((x-ref)*sign(st));
    end
    last=ref;
  end
  m=[max(t5) mean(t5) max(abs(ef)) max(ov)];
end
function r=promedio(p,cmd,integra)
  dt=10e-6; n=round(numel(cmd)*2e-3/dt); i=0; c=0; tn=0; imed=0; r.t=(1:n)*dt; r.i=zeros(1,n); r.ref=r.i; Rreal=1.15*p.R;
  for k=1:n
    ref=cmd(ceil(k*dt/2e-3-1e-9))*p.I_FS;
    if integra && k*dt>=tn, c=c+0.1*p.R*(refd(r,k,ref)-imed); imed=i; tn=tn+1/430; end
    v=max(min(p.R*ref+c,p.Dmax*p.Vbus),-p.Dmax*p.Vbus);
    i=i+dt*(v-Rreal*i)/p.L; r.i(k)=i; r.ref(k)=ref;
  end
  r.im=r.i; r.per=round(2e-3/dt);
end
function x=refd(r,k,ref), j=k-232; if j<1, x=ref; else, x=r.ref(j); end, end
