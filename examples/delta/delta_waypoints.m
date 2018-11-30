function [] = delta_waypoints()
	waypoints = [
    0.0   3.0   4.0   5.0   6.0   7.0   8.0   9.0  11.0  12.0  13.0  14.0  15.0  16.0  17.0  19.0  20.0;   % time
    0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0   0.0;   % ee_x
    0.0   0.08  0.08  0.08  0.08  0.08  0.08  0.08 -0.08 -0.08 -0.08 -0.08 -0.08 -0.08 -0.08  0.0   0.0;   % ee_y
    0.13  0.13  0.13  0.13  0.16  0.16  0.13  0.13  0.13  0.13  0.16  0.16  0.13  0.13  0.13  0.13  0.13;  % ee_z
    0.0   0.0   0.0   1.0   1.0   0.0   0.0   0.0   0.0   0.0   0.0   1.0   1.0   1.0   0.0   0.0   0.0;   % gripper
  ];
  time_waypoints = waypoints(1,:);
  end_effector_position_waypoints = waypoints(2:4,:);
  gripper_position_waypoints = waypoints(5,:);
  pchip_time = time_waypoints(1):0.01:time_waypoints(end);

  end_effector_positions.time = pchip_time';
  end_effector_positions.signals.dimensions = 3;
  end_effector_positions.signals.values = pchip(time_waypoints, end_effector_position_waypoints, pchip_time)';
  assignin('base', 'end_effector_positions', end_effector_positions);

  gripper_positions.time = pchip_time';
  gripper_positions.signals.dimensions = 1;
  gripper_positions.signals.values = pchip(time_waypoints, gripper_position_waypoints, pchip_time)';
  assignin('base', 'gripper_positions', gripper_positions);

  motor_stiffnesses.time = time_waypoints(1);
  motor_stiffnesses.signals.dimensions = 3;
  motor_stiffnesses.signals.values = [0.7, 0.7, 0.7];
  assignin('base', 'motor_stiffnesses', motor_stiffnesses);

  gripper_stiffnesses.time = time_waypoints(1);
  gripper_stiffnesses.signals.dimensions = 1;
  gripper_stiffnesses.signals.values = 0.5;
  assignin('base', 'gripper_stiffnesses', gripper_stiffnesses);

end