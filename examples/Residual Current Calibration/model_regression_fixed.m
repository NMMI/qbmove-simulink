function [params, residual] = model_regression_fixed(target_dir)

if nargout < 1
    close all
end

format long

%% load all data
cd(target_dir)
fnames = dir('*.mat');

speedfiltergain = 1024 * (1- ((1024-128)/1024)^10);


for ii = 1:length(fnames)
    load(fnames(ii).name)
    d.I = I_raw_m.Data;
    d.pos = pos_m.Data;
    d.time = I_raw_m.Time;
    d.vel = myfilter([0;diff(d.pos)./diff(d.time)],speedfiltergain);
    d.ref = squeeze(ref);
    d.Ts = Ts;
    
    d.t_wait_in = t_wait_in;
    d.t_hold = t_hold;
    d.t_wait_out = t_wait_out;
    d.t_ramp = t_ramp;
    d.r_max = r_max;

    data(ii) = d;
end

clearvars -except data

N_data = length(data)

%% find intervals of interest

ttol = 1;

for i = 1:N_data
    
    d = data(i);
    
%     d.i_rise_start = find(d.time > (d.t_wait_in - ttol), 1);
%     d.i_rise_end = find(d.time > (d.t_wait_in + d.t_ramp + ttol), 1);
%     d.i_fall_start = find(d.time > (d.t_wait_in + d.t_ramp + d.t_hold - ttol), 1);
%     d.i_fall_end = find(d.time > (d.t_wait_in + d.t_ramp + d.t_hold + d.t_ramp + ttol), 1);
    
    tmp(i) = d;
    
end
clear data
data = tmp;
clearvars -except data
N_data = length(data);

%% rebundle data

I_tot = [];
pos_tot = [];
v_tot = [];
r_tot = [];

% use_limits = 1;

for i = 1:N_data
    
%     if (use_limits)
%         I_tot = [I_tot; data(i).I(data(i).i_rise_start:data(i).i_fall_end); ];
%         pos_tot = [pos_tot; data(i).pos(data(i).i_rise_start:data(i).i_fall_end); ];
%         v_tot = [v_tot; data(i).vel(data(i).i_rise_start:data(i).i_fall_end); ];
%         r_tot = [r_tot; data(i).ref(data(i).i_rise_start:data(i).i_fall_end); ];
%     else
        I_tot = [I_tot; data(i).I; ];
        pos_tot = [pos_tot; data(i).pos; ];
        v_tot = [v_tot; data(i).vel; ];
        r_tot = [r_tot; data(i).ref; ];
%     end
    
    
end

%% modelling and fitting

rdyn_speed = 1024 * (1- ((1024-20)/1024)^10);

pos_f  = @(a,p) ( a(1) * p + a(2) * p.^2 + a(3) * my_fric(p, 5) );
vel_f  = @(a,v) ( a(4) * v + a(5) * v.*abs(v) );
acc_f  = @(a,ref) ( a(6) * fakedyn(ref, rdyn_speed) );

model_f = @(a,p,v,ref) (pos_f(a,p) + vel_f(a,v) + acc_f(a,ref));

e = 2;
goal_f = @(a) ((sum( abs(I_tot - model_f(a, pos_tot, v_tot, r_tot )).^e )/length(I_tot)).^1/e);

R1 = pos_tot';
R2 = pos_tot'.^2;
R3 = my_fric(pos_tot, 5)'; 
R4 = v_tot'; 
R5 = v_tot'.*abs(v_tot'); 
R6 = fakedyn(r_tot', rdyn_speed);

R = [R1; R2; R3; R4; R5; R6];

params = ((R * R')^-1)* R  * I_tot ;

params = fminsearch(goal_f, params);


%%% debug

%% fitting results check

if nargout < 2
    disp('residual:')
    disp(goal_f(params))
else
    residual = goal_f(params);
end

figure
plot(pos_tot)

if nargout < 1    
    for i = 1:3: N_data
        figure
        subplot(3,1,1)
        hold on
        grid on
        box on
        plot(data(i).time,  data(i).I);
        plot(data(i).time,  model_f(params, data(i).pos, data(i).vel , data(i).ref) , 'r');
        plot(data(i).time,  extimate_current_fixed(params, data(i).pos, data(i).vel, data(i).ref ) , 'g');
       
        legend('data', 'model', 'fixed point');
%         if (use_limits)
%             plot( data(i).time(data(i).i_rise_start)*[1 1], [-1000 1000], 'k--')
%             plot( data(i).time(data(i).i_rise_end)*[1 1], [-1000 1000], 'k:')
%             plot( data(i).time(data(i).i_fall_start)*[1 1], [-1000 1000], 'b--')
%             plot( data(i).time(data(i).i_fall_end)*[1 1], [-1000 1000], 'b:')
%         end
        ylabel('current [mA]')
        
        subplot(3,1,2)
        hold on
        grid on
        box on
        plot(data(i).time, pos_f( params, data(i).pos), 'r');
        plot(data(i).time, vel_f( params, data(i).vel), 'g');
        plot(data(i).time, acc_f( params, data(i).ref), 'b');
        plot(data(i).time,  extimate_current_fixed( params, data(i).pos, data(i).vel, data(i).ref ) , 'k');
        legend('position contribution', 'velocity contribution', 'reference contribution', 'fixed point hell');
%         if (use_limits)
%             plot( data(i).time(data(i).i_rise_start)*[1 1], [-1000 1000], 'k--')
%             plot( data(i).time(data(i).i_rise_end)*[1 1], [-1000 1000], 'k:')
%             plot( data(i).time(data(i).i_fall_start)*[1 1], [-1000 1000], 'b--')
%             plot( data(i).time(data(i).i_fall_end)*[1 1], [-1000 1000], 'b:')
%         end
        ylabel('current [mA]')
        
        
        subplot(3,1,3)
        hold on
        grid on
        box on
        
%         extimate_current_fixed(params, data(i).pos, data(i).vel, data(i).ref );
        
        plot(data(i).time, data(i).I - extimate_current_fixed( params, data(i).pos, data(i).vel, data(i).ref ), 'k');
        plot(data(i).time, myfilter( data(i).I - extimate_current_fixed( params, data(i).pos, data(i).vel, data(i).ref ), (1024 * (1- ((1024-16)/1024)^10)) ), 'r');
        plot(data(i).time, data(i).I - model_f( params, data(i).pos, data(i).vel, data(i).ref ), 'g');
        plot(data(i).time, myfilter( data(i).I - model_f( params, data(i).pos, data(i).vel, data(i).ref ), (1024 * (1- ((1024-16)/1024)^10)) ), 'b');
        plot(data(i).time, 100*ones( size(data(i).time) ), 'k--');
        plot(data(i).time, -100*ones( size(data(i).time) ), 'k--');
        
        ylabel('current residual [mA]')
%         if (use_limits)
%             plot( data(i).time(data(i).i_rise_start)*[1 1], [-1000 1000], 'k--')
%             plot( data(i).time(data(i).i_rise_end)*[1 1], [-1000 1000], 'k:')
%             plot( data(i).time(data(i).i_fall_start)*[1 1], [-1000 1000], 'b--')
%             plot( data(i).time(data(i).i_fall_end)*[1 1], [-1000 1000], 'b:')
%         end
        xlabel('time [s]')
%         ylim([-200 200])
        
    end
    
end

    
params(1) = params(1);
params(2) = params(2) / params(1);
params(3) = params(3);
params(4) = params(4);
params(5) = params(5) / params(4);
params(6) = params(6);

    
if nargout < 1 
    params    
    
    scaled_parames = round(params .* [ 32768, 1048576 , 8, 8192, 8388608, 2048]')
end

cd ..

end

function y = myfilter(u, n)

    y = zeros(size(u));
    for i = 1:(length(u)-1)
        y(i+1) = (y(i)* (1024 - n) + u(i) * n)/1024;
    end

end

function y = fakedyn(x, k)

    y = zeros(size(x));
    for i = 2:length(x)
        y(i) = (k * x(i) + (1024 - k) * y(i-1)) / 1024;
    end

    y = x - y;
end

function z = my_fric(x, zmax)

    zmax = abs(zmax);

    w = zeros(size(x));
    for i = 2:length(x)
        if (x(i) - w(i-1)) > zmax
            w(i) =  x(i) - zmax;
        elseif (w(i-1) - x(i)) > zmax
            w(i) =  x(i) + zmax;
        else
            w(i) = w(i-1);
        end
    end

    z = (x - w);
end

function i_e = extimate_current_fixed(params, pos, vel, ref )


    params(1) = params(1);
    params(2) = params(2) / params(1);
    params(3) = params(3);
    params(4) = params(4);
    params(5) = params(5) / params(4);
    params(6) = params(6);

%     vel = [0;diff(pos)];
    rdyn_speed = 1024 * (1- ((1024-20)/1024)^10);
    acc = fakedyn(ref, rdyn_speed);

    
%     curr_estimate = pos * params(1) .* (1 + pos * params(2)) + (my_fric(pos,5)) * (params(3) ) + vel * params(4) .* (1 + abs(vel) * params(5)) + (acc) * params(6);
    
    scales = [ 32768, 1048576 , 8, 8192, 8388608, 2048]';

    params = round(params .* scales);
         
    curr_estimate = round(scales(2) + (pos * params(2)));                             
           %// curr_estimate is an int+20
    curr_estimate = round(curr_estimate / scales(1)) ;
           %// curr_estimate is now an int+5
    curr_estimate = round(curr_estimate .* (pos * params(1))) ;
           %// curr_estimate is now an int+20
    curr_estimate = round(curr_estimate / scales(2)) ;
           %// curr_estimate is now an int+0
    curr_estimate = round(curr_estimate + (my_fric(pos,5) * params(3)) / scales(3));
           %// curr_estimate is an int+0
    tmp =  round(scales(5) + abs(vel) * params(5)); 
           %// tmp is an int+23
    tmp = round(tmp / scales(4));
           %// tmp is now an int+10
    tmp = round(tmp .* vel * params(4) );
           %// tmp is now an int+23
    tmp = round(tmp / scales(5));
           %// tmp is now an int+0
    curr_estimate = round(curr_estimate + tmp);
           %// curr_estimate is now an int+0
    curr_estimate = round(curr_estimate + acc * params(6) / scales(6));
           %// curr_estimate is now an int+0


    i_e = curr_estimate;

    a = params;
    p = pos;
    v = vel;
    
%     a(2) = a(2)*a(1);
%     a(2) = a(5)*a(4);
%     a(3) = a(3)/5;
    
%     pos_f  = ( a(1) * p .* ( 1 + a(2) * p ) + a(3) * my_fric(p, 5)/5 );
%     vel_f  = ( a(4) * v .* ( 1 + a(5) * abs(v) ) );
%     acc_f  = ( a(6) * fakedyn(ref, rdyn_speed) );

%     pos_f  = ( a(1) * p + a(2) * p.^2 + a(3) * my_fric(p, 5) );
%     vel_f  = ( a(4) * v + a(5) * v.*abs(v) );
%     acc_f  = ( a(6) * acc );
    
%     i_e = pos_f + vel_f + acc_f ;
           
end

