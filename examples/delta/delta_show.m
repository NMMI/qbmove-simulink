function [] = delta_show(delta, joint_positions, gripper_joint_position)
  [ee_pose, pitches, yaws] = delta_fk(joint_positions);

  gripper_references = [0 0 gripper_joint_position 0];
  arm_references = [pitches(1) yaws(1) yaws(1) yaws(1) pitches(2) yaws(2) yaws(2) yaws(2) pitches(3) yaws(3) yaws(3) yaws(3)];
  actuator_references = [0 0 joint_positions(1) 0 0 0 joint_positions(2) 0 0 0 joint_positions(3) 0];
  references = [ee_pose gripper_references arm_references actuator_references];

  show(delta, references, 'PreservePlot', false, 'Frames', 'off');
  hold on;
end