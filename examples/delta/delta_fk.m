function [ee_pose, pitches, yaws] = delta_fk(joint_positions)
  ee_pose = [];
  pitches = [];
  yaws = [];
  %TODO: improve checks
  if length(joint_positions) ~= 3
    return;
  end

  R_motors = 0.1;  % radius from the center of the motors plane to each motor [meters]
  R_ee = 0.0445;  % radius from the center of the end-effector plane to each arm [meters]
  l_motors = 0.09;  % upper arm length [meters]
  l_ee = 0.156;  % forearm length [meters]
  phi_1 = 0;  % motor 1 angular position in the motors plane (behind/right)
  phi_2 = 2*pi/3;  % motor 2 angular position in the motors plane (behind/left)
  phi_3 = 4*pi/3;  % motor 3 angular position in the motors plane (front/central)

  theta_1 = joint_positions(1)-1;  % includes motor offset
  theta_2 = joint_positions(2)-1;  % includes motor offset
  theta_3 = joint_positions(3)-1;  % includes motor offset
  R = R_motors - R_ee;

  % The forward kinematics for a 3-limb-delta structure comes from the intersection of the three spheres with radius
  % equals to the forearm and centered at the end of each upper limb.
  % Actually several assumptions are taken to simplify the system of equations, e.g. even if it is not true, we can
  % consider all the forearms to be linked in the center of the end-effector plane and use "R = R_motors - R_ee".
  %
  % The generic system is as follows:
  %
  %  { (x-[cos(phi_1)*(l_motors*cos(theta_1)+R)])^2 + (y-[-sin(phi_1)*(l_motors*cos(theta_1)+R)])^2 + (z-[-l_motors*sin(theta_1)])^2 = l_ee^2
  %  { (x-[cos(phi_2)*(l_motors*cos(theta_2)+R)])^2 + (y-[-sin(phi_2)*(l_motors*cos(theta_2)+R)])^2 + (z-[-l_motors*sin(theta_2)])^2 = l_ee^2
  %  { (x-[cos(phi_3)*(l_motors*cos(theta_3)+R)])^2 + (y-[-sin(phi_3)*(l_motors*cos(theta_3)+R)])^2 + (z-[-l_motors*sin(theta_3)])^2 = l_ee^2
  %
  % which can have in general 0, 1 or 2 solutions.
  % Note that if 2 solutions exist, only one is feasible.

  % eq_1: (x-x_1)^2 + (y)^2 + (z-z_1)^2 == l_ee^2  (phi_1 is hardcoded to simplify the computation)
  x_1 = l_motors*cos(theta_1) + R;
  z_1 = -l_motors*sin(theta_1);

  % eq_2: (x-x_2)^2 + (y-y_2)^2 + (z-z_2)^2 == l_ee^2
  x_2 = cos(phi_2)*(l_motors*cos(theta_2) + R);
  y_2 = -tan(phi_2)*x_2;
  z_2 = -l_motors*sin(theta_2);

  % eq_3: (x-x_3)^2 + (y-y_3)^2 + (z-z_3)^2 == l_ee^2
  x_3 = cos(phi_3)*(l_motors*cos(theta_3) + R);
  y_3 = -tan(phi_3)*x_3;
  z_3 = -l_motors*sin(theta_3);

  % "eq_4 = eq_1-eq2" and "eq_5 = eq1-eq3" produce two linear equations which are more easily to be solved
  %  { eq_4 - k_1*eq_5 --> x = (a_1*z + b_1)/d
  %  { eq_4 - k_2*eq_5 --> y = (a_2*z + b_2)/d
  w_1 = x_1^2 + z_1^2;
  w_2 = x_2^2 + y_2^2 + z_2^2;
  w_3 = x_3^2 + y_3^2 + z_3^2;

  d = (x_1-x_2)*y_3 - (x_1-x_3)*y_2;
  if d == 0
    return;
  end

  a_1 = y_2*(z_1-z_3) - y_3*(z_1-z_2);
  b_1 = (y_3*(w_1-w_2) - y_2*(w_1-w_3))/2;
  a_2 = -(x_1-x_3)*(z_1-z_2)+(x_1-x_2)*(z_1-z_3);
  b_2 = ((x_1-x_3)*(w_1-w_2)-(x_1-x_2)*(w_1-w_3))/2;

  % Now we can solve for z by sostitution, e.g. in eq_1
  a = a_1^2 + a_2^2 + d^2;
  b = 2*(a_1*b_1 + a_2*b_2 - z_1*d^2 - x_1*a_1*d);
  c = b_1^2 + b_2^2 + x_1^2*d^2 + z_1^2*d^2 - 2*x_1*b_1*d - l_ee^2*d^2;
  delta = b^2 - 4*a*c;
  if delta < 0
    return;
  end

  z = -0.5*(b-sqrt(delta))/a;
  x = (a_1*z + b_1)/d;
  y = -(a_2*z + b_2)/d;

  ee_pose = [x y z];

  % upperarm_poses: [cos(phi_i)*(l_motors*cos(theta_i)+R_motors) -sin(phi_i)*(l_motors*cos(theta_i)+R_motors) -l_motors*sin(theta_i)]
  upperarm_poses = [
    cos(phi_1)*(l_motors*cos(theta_1)+R_motors) sin(phi_1)*(l_motors*cos(theta_1)+R_motors) -l_motors*sin(theta_1);
    cos(phi_2)*(l_motors*cos(theta_2)+R_motors) sin(phi_2)*(l_motors*cos(theta_2)+R_motors) -l_motors*sin(theta_2);
    cos(phi_3)*(l_motors*cos(theta_3)+R_motors) sin(phi_3)*(l_motors*cos(theta_3)+R_motors) -l_motors*sin(theta_3);
  ];

  % forearm_poses: [cos(phi_i)*l_ee -sin(phi_i)*l_ee 0] + ee_pose
  forearm_poses = [
    cos(phi_1)*R_ee sin(phi_1)*R_ee 0;
    cos(phi_2)*R_ee sin(phi_2)*R_ee 0;
    cos(phi_3)*R_ee sin(phi_3)*R_ee 0;
  ] + ee_pose;

  function R = rotz(phi)
    R = [
      cos(phi) -sin(phi) 0;
      sin(phi) cos(phi) 0;
      0 0 1;
    ];
  end

  diff_poses = (forearm_poses - upperarm_poses);
  forearm_to_upperarm_vectors = [
    (rotz(-phi_1)*diff_poses(1,:)')';
    (rotz(-phi_2)*diff_poses(2,:)')';
    (rotz(-phi_3)*diff_poses(3,:)')';
  ];

  pitches = -atan2(forearm_to_upperarm_vectors(:,3), -forearm_to_upperarm_vectors(:,1));
  yaws = atan2(forearm_to_upperarm_vectors(:,2), hypot(forearm_to_upperarm_vectors(:,1), forearm_to_upperarm_vectors(:,3)));
end