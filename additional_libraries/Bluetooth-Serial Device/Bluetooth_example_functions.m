% Init Bluetooth Device
s = initBluetoothDevice("NMMI Dev001");

ID = 2;

% Ping
disp(char(pingBoard(s, ID)));

% Activate Device
activateDevice(s, ID, true);

% Read encoders
encoders = getEncoders(s, ID)

% Read currents
currents = getCurrents(s, ID)

% Read EMG sensors
emg = getEMG(s, ID)

% Set Motor inputs (ticks)
setMotorInputs(s, ID, 10000, -2000);

% Deactivate Device
activateDevice(s, ID, false);

% Initialize IMU
[n_imu, ids, mag_cal, imu_table] = initIMU(s, ID);

% Read IMU
readIMU(s, ID, n_imu, ids, mag_cal, imu_table);

% Close RS485 Device
clear s;