function [currents] = getCurrents(s, ID)

    CMD_GET_CURRENTS = 133;

    write(s,[':' ':' ID 2 CMD_GET_CURRENTS CMD_GET_CURRENTS],"uint8")
    buf = [];
    while (s.NumBytesAvailable < 4)
    end
    % Parse incoming packet
    hdr = read(s,4,"uint8");
    pk_l = -1;
    if (hdr(1) == ':' && hdr(2) == ':' && hdr(3) == ID)
        pk_l = hdr(4);
    end
    i = 0;
    while (s.NumBytesAvailable > 0 && i <= pk_l) 
         i = i+1;
         buf(i) = read(s,1,"uint8");
    end
    for i = 0 : 1
        currents(i+1) = wrap16((256*buf(2 + 2*i) + buf(3 + 2*i)));
    end
end