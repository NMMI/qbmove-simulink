% Set Motor inputs (ticks)

CODE_FREQ = 16;       % 16 Hz -> 0.0625 s     % 50 Hz -> 0.02 sec.
HAND_FREQ_MULT = 3;
TOTAL_TIME_SEC = 10;

HAND_FREQ = CODE_FREQ * HAND_FREQ_MULT; 
CODE_SAMPLE_TIME = 1/CODE_FREQ;
HAND_SAMPLE_TIME = 1/HAND_FREQ;

time_vector = 0:HAND_SAMPLE_TIME:TOTAL_TIME_SEC;

input_vector1 = 1500 * (0:CODE_SAMPLE_TIME:TOTAL_TIME_SEC);
input_vector2 = -8000*ones(CODE_FREQ * TOTAL_TIME_SEC, 1);

s = initDevice('COM12');
ID = 1;

activateDevice(s, ID, true);


t0 = tic;
in = 0;
j = 1;
% Inputs loop
for i = 1:(HAND_FREQ * TOTAL_TIME_SEC)-1
    t1 = tic;
    
    rem = mod(i,HAND_FREQ_MULT);
    
    if (rem == 0)
        
        %% Code at CODE_FREQ frequency
    
        % [All stuff]
    
    
        j = i/HAND_FREQ_MULT;
    end

    %% Code at HAND_FREQ frequency
    in1 = input_vector1(j) + rem*(input_vector1(j+1) - input_vector1(j))/HAND_FREQ_MULT;
    in2 = input_vector2(j) + rem*(input_vector2(j+1) - input_vector2(j))/HAND_FREQ_MULT;
    
    fprintf("Sample %d out of %d\tinputs: %d %d\n", i, length(time_vector), in1, in2);
    setMotorInputs(s, ID, in1, in2);
    
    pauses(HAND_SAMPLE_TIME, t1);
%     toc(t1);
end

toc(t0);

setMotorInputs(s, ID, 0, 0);        % Open hand device
pause(1);

activateDevice(s, ID, false);
clear s;