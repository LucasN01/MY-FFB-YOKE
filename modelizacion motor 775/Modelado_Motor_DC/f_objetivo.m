function obj = f_objetivo(p,data)
    y_real = data(:,4);
    X = motor_simulate(p,data);
    % calculate objective
    obj =  sum((X(:,2)-y_real).^2);         
end