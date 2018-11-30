function [delta] = delta_init(urdf_name)
  delta = importrobot([urdf_name '.urdf']);
  delta.DataFormat = 'row';
  delta.Gravity = [0 0 -9.81];
  assignin('base', 'delta', delta);

  delta_waypoints()
end