function buf = pingBoard(s, ID)

    CMD_GET_INFO = 6;

    buf = [];
    buf(1) = CMD_GET_INFO;
    buf(2) = 0;
    buf(3) = 0;
    chs = checksum(buf, 3);
    write(s,[':' ':' ID 4 buf(1) buf(2) buf(3) chs],"uint8")
    buf = [];
    i = 0;
    pause(0.2);
    while s.NumBytesAvailable > 0
        i = i+1;
        buf(i) = read(s,1,"char");
    end
end