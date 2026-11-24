%% Simula Control_Corriente_FFB.slx en varios casos: tiempo de llegada y error antes del comando siguiente
eje = 'ROLL'; Parametros_Control; m = 'Control_Corriente_FFB'; load_system(m);
casos = {'conmutado, trabado',''
    'promediado, trabado','usar_conmutado=0;'
    'rotor libre','eje_trabado=false;'
    'rotor libre + FCEM','eje_trabado=false; usar_FCEM=1;'
    'I_FS=10','I_FS=10;'
    'I_FS=10 sin anti-windup','I_FS=10; antiwindup=0;'
    'motor R +30 %','R=1.3*R;'
    'motor L -30 %','L=0.7*L;'
    'motor L +30 %','L=1.3*L;'
    'driver pierde 1.5 us','t_perdido=1.5e-6;'
    'muestra sin desfasar','t_adc=1e-9;'
    'I_FS=10, libre, FCEM','I_FS=10; eje_trabado=false; usar_FCEM=1;'
    'libre, J_eje 0.01','eje_trabado=false; J_eje=0.01;'
    'libre, J_eje 0.01, mano','eje_trabado=false; J_eje=0.01; tl_ws=[0 0; .02 1.5; .05 -1.5];'
    'fc = 500 Hz','Kp=Kp/2; Ki=Ki/2;'};
fprintf('%-26s %12s %12s %14s\n','Caso','t5 peor ms','t5 medio ms','err final A');
for k = 1:size(casos,1)
    evalc('Parametros_Control'); eval(casos{k,2}); sim(m);
    [tp,tm,ef] = medir(I_ref,I_real,Ts,Ts_cmd);
    fprintf('%-26s %12.2f %12.2f %14.3f\n',casos{k,1},tp,tm,ef);
    if k==1
        figure('Position',[50 50 1100 700]);
        subplot(3,1,1); plot(I_real.Time*1e3,I_real.Data(:),'b'); hold on; stairs(I_ref.Time*1e3,I_ref.Data(:),'k'); grid on
        ylabel('A'); legend('I','I*'); title('ROLL, rotor trabado, PWM conmutado (Simulink)');
        subplot(3,1,2); stairs(V_motor.Time*1e3,V_motor.Data(:)); grid on; ylabel('V');
        subplot(3,1,3); plot(T_eje.Time*1e3,T_eje.Data(:)); grid on; ylabel('N m (eje, ideal)'); xlabel('ms');
        saveas(gcf,'simulink_control.png');
    end
end
evalc('Parametros_Control');

function [tp,tm,ef] = medir(r,i,Ts,Tc)
    t = (Ts/2:Ts/20:r.Time(end))'; [ti,u] = unique(i.Time);
    x = movmean(interp1(ti,i.Data(u),t,'linear','extrap'),20);   % media de un periodo de PWM
    rd = r.Data(:); T = []; E = []; ant = 0;
    for j = 1:floor(r.Time(end)/Tc+1e-6)
        ref = rd(find(r.Time<=(j-.5)*Tc,1,'last')); q = t>=(j-1)*Tc & t<j*Tc-Ts;
        tt = t(q)-(j-1)*Tc; y = x(q);
        E(end+1) = abs(mean(y(tt>Tc-5*Ts))-ref); %#ok<AGROW>
        if abs(ref-ant)>.4
            b = find(abs(y-ref)>max(.05*abs(ref-ant),.15),1,'last'); if isempty(b), b = 1; end
            T(end+1) = tt(b)*1e3; %#ok<AGROW>
        end
        ant = ref;
    end
    tp = max(T); tm = mean(T); ef = max(E);
end
