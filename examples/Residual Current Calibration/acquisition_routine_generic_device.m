close all
clear
format long

%% GENERIC DEVICE PARAMETERS CONFIGURATION
% Just change this section parameters to run device calibration and launch
% the script

COM_port = 'COM36';
device_ID = 1;
target_dir = 'calibration_data';

% max_closure_ticks: SoftHand V1.2 = 19000, 
% SoftHand V2.0.3 = 22000, SoftHandPro (V3.0) = 16000, 
% Gripper = 11000
max_closure_ticks = 16000;

% gear_ratio between first two encoders: SoftHand V1.2 = 15/14, 
% SoftHand V2.0.3 = 15/14, SoftHandPro (V3.0) = 35/3, 
% Gripper = 15/14
gear_ratio = 35/3;

% encoder FW resolution: SoftHand V1.2 = 3, 
% SoftHand V2.0.3 = 3, SoftHandPro (V3.0) = 3, 
% Gripper = 3, Cubes = 1, CUFF = 2
encoder_fw_resolution = 3;

%% Reference signal and simulation parameters
Vclosure = [20, 40, 60, 80, 100];
Vramptime = [0, 1, 5];
Vramp_count = [1, 5];

Ts = 0.002;     % Note: for SoftHandPro device run as fast as possible to not lose 2nd encoder values

r_zero = 200;

t_wait_in = 2;
t_hold = 2;
t_wait_out = 2;

mkdir(target_dir);

%% Run calibration and save files
for z = 1:length(Vramp_count)
    for i = 1:length(Vclosure)
        for j = 1:length(Vramptime)

            r_max = Vclosure(i)/100 * max_closure_ticks;
            t_ramp = Vramptime(j);
            n_ramp = Vramp_count(z);

            Tf = t_wait_in + (t_ramp + t_hold )*2*n_ramp - t_hold + t_wait_out;

            time = [0:Ts:Tf];
            
            steps = linspace(r_zero, r_max, n_ramp+1);
            
            r = r_zero * ones(1,(t_wait_in/Ts));
            for k = 1:n_ramp
                r = [r, linspace(steps(k), steps(k+1), t_ramp/Ts), ones(1,(t_hold/Ts)) * steps(k+1)];
            end
            
            steps = linspace(0, r_max, n_ramp+1);
            
            for k = n_ramp:-1:2
                r = [r, linspace(steps(k+1), steps(k), t_ramp/Ts), ones(1,(t_hold/Ts)) * steps(k)];
            end
            
            r = [r, linspace(steps(2), steps(1), t_ramp/Ts)];
            
            r = [r, zeros(1,(t_wait_out/Ts)+1)];

            reference = timeseries(r, time);

            sim('acquisition.slx')
            cd(target_dir);
            save(sprintf('T_C%3d_S%d_T%d', Vclosure(i),  Vramp_count(z), Vramptime(j) ));
            cd ..
            clearvars -except i j z Ts Vramptime Vclosure COM_port device_ID gear_ratio encoder_fw_resolution max_closure_ticks t_wait_in t_wait_out t_hold t_ramp r_max target_dir r_zero Vramp_count      
        end
    end
end

model_regression(target_dir)