function activateDevice(s, ID,activation)
    
    CMD_ACTIVATE = 128;
    
    if (activation)
        ACTIVE = 3;
    else
        ACTIVE = 0;
    end
    buf = [];
    buf(1) = CMD_ACTIVATE;
    buf(2) = ACTIVE;
    chs = checksum(buf, 2);
    write(s,[':' ':' ID 3 buf(1) buf(2) chs],"uint8");
end

