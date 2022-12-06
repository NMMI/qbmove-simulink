% Init RS485 Device
s = initSerialDevice('COM2');

ID = 1;

% Ping
disp(char(pingBoard(s, ID)));

% Activate Device
activateDevice(s, ID, true);

% Read encoders
encoders = getEncoders(s, ID)

% Set Motor inputs (ticks)
setMotorInputs(s, ID, 10000, -2000);

% Deactivate Device
activateDevice(s, ID, false);

% Close RS485 Device
clear s;