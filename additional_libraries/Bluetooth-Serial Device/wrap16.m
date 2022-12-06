function y = wrap16(u)
    % Elements of u are in the range [0,65535]
    y = u;    
    for i=1:length(u)
        if u(i) > 32768
            y(i) = u(i) - 65536;
        end
    end
end