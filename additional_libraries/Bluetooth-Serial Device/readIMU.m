function [imu_values] = readIMU(s, ID, n_imu, ids, mag_cal, imu_table)

    CMD_GET_IMU_READINGS = 161;

    imu_values = zeros(n_imu, 14);

    write(s,[':' ':' ID 2 CMD_GET_IMU_READINGS CMD_GET_IMU_READINGS],"uint8");
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

    acc_sf 	= 0.000061037;			% Ticks to G
	gyro_sf = 0.007629627 * 8;		% Ticks to deg/s with FS +/- 2000 °/s
	mag_sf 	= 0.1465;				% Ticks to uT
	
	temp_sf = 0.00294118;
	temp_off = 36.53;
	temp_div = 2.0;

    values = buf(2:end);
   
    c = 1;
    for i = 0 : (n_imu-1)
        if (values(c) == ':')
            if imu_table(5*i + 0 +1) ~= 0
                wvalues = wrap8(values(c+1:c+6));
                aux_f = ((bitsll(wvalues(1),8) + wvalues(2)));
                imu_values((3*3+4+1)*i +1) = acc_sf * aux_f;

                aux_f = ((bitsll(wvalues(3),8) + wvalues(4)));
                imu_values((3*3+4+1)*i+1 +1) = acc_sf * aux_f;

                aux_f = ((bitsll(wvalues(5),8) + wvalues(6)));
                imu_values((3*3+4+1)*i+2 +1) = acc_sf * aux_f;

                c = c+6;
            end
            if imu_table(5*i + 1 +1) ~= 0
                wvalues = wrap8(values(c+1:c+6));
                aux_f = ((bitsll(wvalues(1),8) + wvalues(2)));
                imu_values((3*3+4+1)*i+3 +1) = gyro_sf * aux_f;

                aux_f = ((bitsll(wvalues(3),8) + wvalues(4)));
                imu_values((3*3+4+1)*i+4 +1) = gyro_sf * aux_f;

                aux_f = ((bitsll(wvalues(5),8) + wvalues(6)));
                imu_values((3*3+4+1)*i+5 +1) = gyro_sf * aux_f;

                c = c+6;
            end
            if imu_table(5*i + 2 +1) ~= 0
                wvalues = wrap8(values(c+1:c+6));
                aux_f = ((bitsll(wvalues(1),8) + wvalues(2)));
                imu_values((3*3+4+1)*i+7 +1) = - (aux_f * mag_sf * mag_cal(3*i+0 +1));

                aux_f = ((bitsll(wvalues(3),8) + wvalues(4)));
                imu_values((3*3+4+1)*i+6 +1) = - (aux_f * mag_sf * mag_cal(3*i+1 +1));

                aux_f = ((bitsll(wvalues(5),8) + wvalues(6)));
                imu_values((3*3+4+1)*i+8 +1) = (aux_f * mag_sf * mag_cal(3*i+2 +1));

                c = c+6;
            end
            if imu_table(5*i + 3 +1) ~= 0
                aux_buf = wrap8(values(c+1:c+4));
                aux_i = int32(bitsll(aux_buf(1), 24) + bitsll(aux_buf(2), 16) + bitsll(aux_buf(3), 8) + aux_buf(4));
                imu_values((3*3+4+1)*i+9 +1) = typecast(aux_i, 'single');
                aux_buf = wrap8(values(c+5:c+8));
                aux_i = int32(bitsll(aux_buf(1), 24) + bitsll(aux_buf(2), 16) + bitsll(aux_buf(3), 8) + aux_buf(4));
                imu_values((3*3+4+1)*i+10 +1) = typecast(aux_i, 'single');
                aux_buf = wrap8(values(c+9:c+12));
                aux_i = int32(bitsll(aux_buf(1), 24) + bitsll(aux_buf(2), 16) + bitsll(aux_buf(3), 8) + aux_buf(4));
                imu_values((3*3+4+1)*i+11 +1) = typecast(aux_i, 'single');
                aux_buf = wrap8(values(c+13:c+16));
                aux_i = int32(bitsll(aux_buf(1), 24) + bitsll(aux_buf(2), 16) + bitsll(aux_buf(3), 8) + aux_buf(4));
                imu_values((3*3+4+1)*i+12 +1) = typecast(aux_i, 'single');
                c = c+16;
            end
            if imu_table(5*i + 4 +1) ~= 0
                wvalues = wrap8(values(c+1:c+2));
                aux_f = ((bitsll(wvalues(1),8) + wvalues(2)));
                imu_values((3*3+4+1)*i+13 +1) = (temp_sf * aux_f + temp_off)/temp_div;
                c = c+2;
            end
            c = c+1;
        end
        if (values(c) == ':')
            c = c+1;
        else
            break;
        end
    end

    for i=0:(n_imu-1)
        fprintf("IMU: %d\r\n", ids(i+1));

        if imu_table(5*i + 0 +1)
            fprintf("Accelerometer\n");
            fprintf("%f, %f, %f\n", imu_values((3*3+4+1)*i +1), imu_values((3*3+4+1)*i+1 +1), imu_values((3*3+4+1)*i+2 +1));
        end
        if imu_table(5*i + 1 +1)
            fprintf("Gyroscope\n");
            fprintf("%f, %f, %f\n", imu_values((3*3+4+1)*i+3 +1), imu_values((3*3+4+1)*i+4 +1), imu_values((3*3+4+1)*i+5 +1));
        end
        if imu_table(5*i + 2 +1)
            fprintf("Magnetometer\n");
            fprintf("%f, %f, %f\n", imu_values((3*3+4+1)*i+6 +1), imu_values((3*3+4+1)*i+7 +1), imu_values((3*3+4+1)*i+8 +1));
        end
        if imu_table(5*i + 3 +1)
            fprintf("Quaternion\n");
            fprintf("%f, %f, %f, %f\n", imu_values((3*3+4+1)*i+9 +1), imu_values((3*3+4+1)*i+10 +1), imu_values((3*3+4+1)*i+11 +1), imu_values((3*3+4+1)*i+12 +1));
        end
        if imu_table(5*i + 4 +1)
            fprintf("Temperature\n");
            fprintf("%f\n", imu_values((3*3+4+1)*i+13 +1));
        end

        fprintf("\n");
    end
end