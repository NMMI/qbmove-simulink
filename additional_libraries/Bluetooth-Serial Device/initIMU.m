function [n_imu, ids, mag_cal, imu_table] = initIMU(s, ID)
    
    PARAM_SLOT_BYTES = 50;
    new_board = 1;
    num_imus_id_params = 7;
    num_mag_cal_params = 0;
    first_imu_parameter = 2;

    %% Try commGetIMUParamList
    CMD_GET_IMU_PARAM = 162;
    chksum = CMD_GET_IMU_PARAM; % xor between 0 0 CMD_GET_IMU_PARAM
    write(s,[':' ':' ID 4 CMD_GET_IMU_PARAM 0 0 chksum],"uint8");
    buf = [];    
    pause(0.2);
    while (s.NumBytesAvailable < 4)
    end
    % Receive incoming packet
    i = 0;
    while (s.NumBytesAvailable > 0) 
         i = i+1;
         buf(i) = read(s,1,"uint8");
    end
    if buf(5) ~= CMD_GET_IMU_PARAM
        % If commGetIMUParamList returns -1, the connected board is a PSoC3
        % board instead of a STM32 or PSoC5 board, so call the 
        % commGetParamList instead
		new_board = 0;
        CMD_GET_PARAM_LIST = 12;
          chksum = CMD_GET_PARAM_LIST; % xor between 0 0 CMD_GET_PARAM_LIST
        write(s,[':' ':' ID 4 CMD_GET_PARAM_LIST 0 0],"uint8")
        buf = [];    
        pause(0.2);

		% Receive incoming packet
        while (s.NumBytesAvailable > 0) 
             i = i+1;
             buf(i) = read(s,1,"uint8");
        end
        num_imus_id_params = 6;
    end

    n_imu = buf(9);
    fprintf("Number of connected IMUs: %d\r\n", n_imu);
    
    num_mag_cal_params = floor(n_imu / 2);
    if (n_imu - 2*num_mag_cal_params) > 0
        num_mag_cal_params = num_mag_cal_params + 1;
    end
    
    ids = zeros(1, n_imu);
    i = 1;
    for k=1:num_imus_id_params
        if buf(k*PARAM_SLOT_BYTES + 8 +1) ~= 255
            ids(i) = buf(k*PARAM_SLOT_BYTES + 8 +1);
            i = i+1;
        end
        if buf(k*PARAM_SLOT_BYTES + 9 +1) ~= 255
            ids(i) = buf(k*PARAM_SLOT_BYTES + 9 +1);
            i = i+1;
        end
        if buf(k*PARAM_SLOT_BYTES + 10 +1) ~= 255
            ids(i) = buf(k*PARAM_SLOT_BYTES + 10 +1);
            i = i+1;
        end
    end
    
    mag_cal = zeros(n_imu, 3);
    i = 0;
    for k=1:num_mag_cal_params
        mag_cal(3*i + 0 +1) = buf(num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 8 +1);
        mag_cal(3*i + 1 +1) = buf(num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 9 +1);
        mag_cal(3*i + 2 +1) = buf(num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 10 +1);
        fprintf("MAG PARAM: %d %d %d\r\n", mag_cal(3*i + 0 +1), mag_cal(3*i + 1 +1), mag_cal(3*i + 2 +1));
        i = i+1;

        if buf(num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 7 +1) == 6
            mag_cal(3*i + 0 +1) = buf(num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 11 +1);
            mag_cal(3*i + 1 +1) = buf(num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 12 +1);
            mag_cal(3*i + 2 +1) = buf(num_imus_id_params*PARAM_SLOT_BYTES + k*PARAM_SLOT_BYTES + 13 +1);
            fprintf("MAG PARAM: %d %d %d\r\n", mag_cal(3*i + 0 +1), mag_cal(3*i + 1 +1), mag_cal(3*i + 2 +1));
            i = i+1;
        end
    end

    first_imu_parameter = 1 + num_imus_id_params + num_mag_cal_params + 1;
    imu_table = zeros(n_imu, 5);
    for i=0:(n_imu-1)
        imu_table(5*i + 0 +1) = buf(first_imu_parameter*PARAM_SLOT_BYTES + 8 + 50*i +1);
        imu_table(5*i + 1 +1) = buf(first_imu_parameter*PARAM_SLOT_BYTES + 9 + 50*i +1);
        imu_table(5*i + 2 +1) = buf(first_imu_parameter*PARAM_SLOT_BYTES + 10 + 50*i +1);
        imu_table(5*i + 3 +1) = buf(first_imu_parameter*PARAM_SLOT_BYTES + 11 + 50*i +1);
        imu_table(5*i + 4 +1) = buf(first_imu_parameter*PARAM_SLOT_BYTES + 12 + 50*i +1);
        fprintf("ID: %d - %d, %d, %d, %d, %d\n", ids(i+1), imu_table(5*i + 0 +1), imu_table(5*i + 1 +1), imu_table(5*i + 2 +1), imu_table(5*i + 3 +1), imu_table(5*i + 4 +1));
    end

    if (new_board == 0 && n_imu > 1)
        idx = 0;
        for i=0:(n_imu-1)
            if (imu_table(5*i + 3 +1) ~= 0)
                idx = idx+1;
            end
            if idx > 0
                fprintf("[WARNING] Quaternion will not be read as it is computed only if there is ONLY 1 IMU connected to the board.\n\n");
            end
        end
    end
end