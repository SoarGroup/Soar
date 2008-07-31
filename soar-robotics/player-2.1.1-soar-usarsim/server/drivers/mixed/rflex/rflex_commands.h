#ifndef RFLEX_COMMANDS_H
#define RFLEX_COMMANDS_H

  int rflex_open_connection(char *dev_name, int *fd);
  int rflex_close_connection(int *fd);

  void rflex_sonars_on(int fd);
  void rflex_sonars_off(int fd);

  void rflex_ir_on(int fd);
  void rflex_ir_off(int fd);

  void rflex_brake_on(int fd);
  void rflex_brake_off(int fd);

  void rflex_odometry_off( int fd );
  void rflex_odometry_on( int fd, long period );

  void rflex_motion_set_defaults(int fd);

  void rflex_initialize(int fd, int trans_acceleration,
			       int rot_acceleration,
			       int trans_pos,
			       int rot_pos);

  void rflex_update_status(int fd, int *distance, 
				  int *bearing, int *t_vel,
				  int *r_vel);

  void rflex_update_system(int fd, int *battery,
				   int *brake);

  int rflex_update_sonar(int fd, int num_sonars,
				 int *ranges);
  void rflex_update_bumpers(int fd, int num_bumpers,
				   char *values);
  void rflex_update_ir(int fd, int num_irs,
			     unsigned char *ranges);

  void rflex_set_velocity(int fd, long t_vel, long r_vel, 
				 long acceleration);
  void rflex_stop_robot(int fd, int deceleration);

//int clear_incoming_data(int fd);

#endif
