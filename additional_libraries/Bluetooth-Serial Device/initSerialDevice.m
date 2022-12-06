function [s] = initSerialDevice(port)

    s = serialport(port,2000000);
    set(s,'StopBits',1);
    set(s,'DataBits',8);
    set(s,'Timeout',0.01);  % 10 msec. timeout on read
end

