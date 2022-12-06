function setMotorInputs(s, ID, val1, val2)

    CMD_SET_INPUTS = 130;
    
    val1_16 = typecast(int16(val1),'uint8');
    val2_16 = typecast(int16(val2),'uint8');
    
    buf = [];
    buf(1) = CMD_SET_INPUTS;
    buf(2) = val1_16(2);
    buf(3) = val1_16(1);
    buf(4) = val2_16(2);
    buf(5) = val2_16(1); 
    chs = checksum(buf, 5);
    write(s,[':' ':' ID 6 buf(1) buf(2) buf(3) buf(4) buf(5) chs],"uint8");
end

