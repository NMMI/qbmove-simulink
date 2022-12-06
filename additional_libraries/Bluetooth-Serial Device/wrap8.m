function y = wrap8(u)
    % Elements of u are in the range [0,255]
    y = u;    
    for i=1:length(u)
        if u(i) > 128
            y(i) = u(i) - 256;
        end
    end
end