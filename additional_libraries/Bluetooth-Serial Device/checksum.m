function chs = checksum(data, len)
    chs = 0;
    
    for i = 1 : len
        A = uint8(chs);
        B = uint8(data(i));
        chs = bitxor(A, B);
    end
end

