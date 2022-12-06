function [s] = initBluetoothDevice(deviceName)

    s = bluetooth(deviceName,1,'Timeout',1);  % 1 sec. timeout on read

end

