// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:1; -*-

/**
  *  Copyright (C) 2006
  *     Videre Design
  *  Copyright (C) 2000  
  *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
  *
  *  Videre Erratic robot driver for Player
  *
	*  This program is free software; you can redistribute it and/or modify
	*  it under the terms of the GNU General Public License as published by
	*  the Free Software Foundation; either version 2 of the License, or
	*  (at your option) any later version.
	*
	*  This program is distributed in the hope that it will be useful,
	*  but WITHOUT ANY WARRANTY; without even the implied warranty of
	*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*  GNU General Public License for more details.
	*
	*  You should have received a copy of the GNU General Public License
	*  along with this program; if not, write to the Free Software
	*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**/

#ifndef _ERRATICDEVICE_H
#define _ERRATICDEVICE_H

#ifndef ERRATIC_VERSION
#define ERRATIC_VERSION "1.0b"
#endif

#ifndef ERRATIC_DATE
#define ERRATIC_DATE "2006-05-07"
#endif

#include <pthread.h>
#include <sys/time.h>
#include <queue>

#include <libplayercore/playercore.h>
#include <replace/replace.h>

#include "packet.h"
#include "robot_params.h"

//#include <stdint.h>

#define CPU_VOLTAGE 3.5

// angular constants, angular units are 4096 / rev
#define ATOR(x) (M_PI * ((double)(x)) / 2048.0)
#define ATOD(x) (180.0 * ((double)(x)) / 2048.0)
#define RTOA(x) ((short)((x) * 2048.0) / M_PI)

// Default max speeds
#define MOTOR_DEF_MAX_SPEED 0.5
#define MOTOR_DEF_MAX_TURNSPEED DTOR(100)

// This merely sets a delay policy in the initial connection
#define ROBOT_CYCLETIME 20000

/* Erratic constants */

#define VIDERE_NOMINAL_VOLTAGE 12.0


// Commands for the robot
typedef enum command {
	pulse =                     0,
	open_controller =           1,
	close_controller =          2,
	enable_motors =             4,
	set_max_trans_acc =         5,
	set_max_position_velocity = 6,
	reset_origo =               7,
	trans_vel =                 11,	// mm/s
	rot_pos   =                 12,	// deg
	rot_dpos  =                 13,	// deg
	configuration =             18,
	rot_vel =                   21,	// deg/s
	set_max_rot_acc =           23,
	set_sonar =                 28,
	stop =                      29,
	wheel_vel =                 32,	// mm/s
	set_analog =                71,
	save_config =               72,
	set_pwm_freq =              73,
	set_pwm_max_on =            74,
	servo_pos      =            75,
	set_pid_trans_p =           80,
	set_pid_trans_v =           81,
	set_pid_trans_i =           82,
	set_pid_rot_p =             83,
	set_pid_rot_v =             84,
	set_pid_rot_i =             85,
	
} command_e;

// Argument types used in robot commands
typedef enum argtype {
	argint =  0x3B,
	argnint = 0x1B,
	argstr =  0x2B
} argtype_e;

// Types of replies from the robot
typedef enum reply {
	debug =   0x15,
	config =  0x20,
	stopped = 0x32,
	moving =  0x33,
	motor =   0x80,
	encoder = 0x90,
	ain =     0x9a,
	sonar =   0x9b
} reply_e;


#define DEFAULT_VIDERE_PORT "/dev/erratic"

typedef struct player_erratic_data
{
  player_position2d_data_t position;
  player_power_data_t power;
  player_aio_data_t aio;
  player_ir_data ir;
  player_sonar_data sonar;
} __attribute__ ((packed)) player_erratic_data_t;

// this is here because we need the above typedef's before including it.
#include "motorpacket.h"

extern bool debug_mode;

class ErraticMotorPacket;

class Erratic : public Driver 
{
private:
  int mcount;
  player_erratic_data_t erratic_data;

  player_devaddr_t position_id;
  player_devaddr_t power_id;
  player_devaddr_t aio_id;
  player_devaddr_t ir_id;
  player_devaddr_t sonar_id;
  player_devaddr_t ptz_id, ptz2_id;

  int position_subscriptions;
  int aio_ir_subscriptions;
  int sonar_subscriptions;
  int ptz_subscriptions;
  int ptz2_subscriptions;

  //ErraticMotorPacket* sippacket;
  ErraticMotorPacket *motor_packet;
  pthread_mutex_t motor_packet_mutex;
		
  int Connect();
  int Disconnect();
		
  void ResetRawPositions();
  void ToggleMotorPower(unsigned char val);

  void ToggleAIn(unsigned char val);
  void ToggleSonar(unsigned char val);

  int HandleConfig(QueuePointer &resp_queue, player_msghdr * hdr, void* data);
  int HandleCommand(player_msghdr * hdr, void * data);
  void HandlePositionCommand(player_position2d_cmd_vel_t position_cmd);
  void HandleCarCommand(player_position2d_cmd_car_t position_cmd);
  void HandlePtzCommand(player_ptz_cmd_t ptz_cmd, player_devaddr_t id);

  void PublishAllData();
  void PublishPosition2D();
  void PublishPower();
  void PublishAIn();
  void PublishIR();
  void PublishSonar();
		
  float IRRangeFromVoltage(float voltage);
  float IRFloorRange(float value);
		
  void StartThreads();
  void StopThreads();
		
  void Send(ErraticPacket *packet);
  void SendThread();
  static void *SendThreadDummy(void *driver);
  void ReceiveThread();
  static void *ReceiveThreadDummy(void *driver);

  int read_fd, write_fd;
  const char* psos_serial_port;

  player_position2d_cmd_vel_t last_position_cmd;
  player_position2d_cmd_car_t last_car_cmd;

  std::queue<ErraticPacket *> send_queue;
  pthread_mutex_t send_queue_mutex;
  pthread_cond_t send_queue_cond;

  pthread_t send_thread;
  pthread_t receive_thread;

  // Parameters

  bool direct_wheel_vel_control;

  bool print_all_packets;
  bool print_status_summary;
		
  bool save_settings_in_robot;

  int param_idx;  // index in the RobotParams table for this robot
		
  // Max motor speeds (mm/sec,deg/sec)
  int motor_max_speed;
  int motor_max_turnspeed;

  // Customized control settings for the robot
  int16_t pid_trans_p, pid_trans_v, pid_trans_i;
  int16_t pid_rot_p, pid_rot_v, pid_rot_i;

  // This is a fairly low-level setting that is exposed
  uint16_t motor_pwm_frequency, motor_pwm_max_on;

  // Bound the command velocities
  bool use_vel_band; 

  // Max motor accel/decel (mm/sec/sec, deg/sec/sec)
  short motor_max_trans_accel, motor_max_trans_decel;
  short motor_max_rot_accel, motor_max_rot_decel;

public:

  Erratic(ConfigFile* cf, int section);

  virtual int Subscribe(player_devaddr_t id);
  virtual int Unsubscribe(player_devaddr_t id);

  /* the main thread */
  virtual void Main();

  virtual int Setup();
  virtual int Shutdown();

  // MessageHandler
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data);
};


#endif
