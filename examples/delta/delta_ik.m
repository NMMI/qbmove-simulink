function [isvalid, joint_positions] = delta_ik(ee_pose)
  joint_positions = zeros(3,1);
  %TODO: improve checks
  if length(ee_pose) ~= 3
    return;
  end

  R_motors = 0.1;  % radius from the center of the motors plane to each motor [meters]
  R_ee = 0.0375;  % radius from the center of the end-effector plane to each arm [meters]
  l_motors = 0.09;  % upper arm length [meters]
  l_ee = 0.16;  % forearm length [meters]in the motors plane (behind/left)
  phi_1 = 0;  % motor 1 angular position in the motors plane (behind/right)
  phi_2 = 2*pi/3;  % motor 2 angular position 
  phi_3 = 4*pi/3;  % motor 3 angular position in the motors plane (front/central)

  ee_x = ee_pose(1);
  ee_y = ee_pose(2);
  ee_z = ee_pose(3);
  R = R_motors - R_ee;

  function [isvalid, joint_position] = arm_ik(ee_x, ee_y, ee_z, phi)
    x = cos(phi)*ee_x + sin(phi)*ee_y - R;
    y = -sin(phi)*ee_x + cos(phi)*ee_y;
    z = ee_z;
    [isvalid_1, theta_1] = acos_safe(y, l_ee);
    [isvalid_2, theta_2] = acos_safe((x^2 + y^2 + z^2 - l_motors^2 - l_ee^2), (2*l_motors*l_ee*sin(theta_1)));
    isvalid = all([isvalid_1, isvalid_2]);
    if ~isvalid
      joint_position = NaN;
      return;
    end
    c_1 = l_ee*cos(theta_2)*sin(theta_1) + l_motors;
    c_2 = l_ee*sin(theta_2)*sin(theta_1);

    joint_position = -(atan2(-c_2*x + c_1*z, c_1*x + c_2*z) - 1);  % includes motor offset
  end

  function [isvalid, arc] = acos_safe(num, den)
    isvalid = abs(num) < abs(den);
    arc = acos(num/den);
  end

  [isvalid(1), joint_positions(1)] = arm_ik(ee_x, ee_y, ee_z, phi_1);
  [isvalid(2), joint_positions(2)] = arm_ik(ee_x, ee_y, ee_z, phi_2);
  [isvalid(3), joint_positions(3)] = arm_ik(ee_x, ee_y, ee_z, phi_3);
  isvalid = all(isvalid);
end