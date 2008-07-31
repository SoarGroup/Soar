// -*- mode:C++; tab-width:2; c-basic-offset:2; indent-tabs-mode:1; -*-

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

/**
  *  This driver is adapted from the p2os driver of player 1.6.
**/

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_erratic erratic
 * @brief Erratic

This driver talks to the embedded computer in the Erratic robot, which
mediates communication to the devices of the robot.

@par Compile-time dependencies

- none

@par Provides

The erratic driver provides the following device interfaces, some of
them named:

- "odometry" @ref interface_position2d
  - This interface returns odometry data, and accepts velocity commands.

- @ref interface_power
  - Returns the current battery voltage (12 V when fully charged).

- @ref interface_aio
  - Returns data from analog and digital input pins 

- @ref interface_ir
  - Returns ranges from IR sensors, assuming they're connected to the analog input pins

- @ref interface_sonar
  - Returns ranges from sonar sensors

- @ref interface_ptz : control of the servos that pan and tilt

@par Supported configuration requests

- "odometry" @ref interface_position2d :
  - PLAYER_POSITION_SET_ODOM_REQ
  - PLAYER_POSITION_MOTOR_POWER_REQ
  - PLAYER_POSITION_RESET_ODOM_REQ
  - PLAYER_POSITION_GET_GEOM_REQ
  - PLAYER_POSITION_VELOCITY_MODE_REQ
- @ref interface_ir :
  - PLAYER_IR_REQ_POSE
- @ref interface_sonar :
  - PLAYER_SONAR_GET_GEOM_REQ

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS0"
- direct_wheel_vel_control (integer)
  - Default: 0
  - Send direct wheel velocity commands to Erratic (as opposed to sending
    translational and rotational velocities and letting Erratic smoothly
    achieve them).
- max_trans_vel (length)
  - Default: 0.5 m/s
  - Maximum translational velocity
- max_rot_vel (angle)
  - Default: 100 deg/s
  - Maximum rotational velocity
- trans_acc (length)
  - Default: 0
  - Maximum translational acceleration, in length/sec/sec; nonnegative.
    Zero means use the robot's default value.
- trans_decel (length)
  - Default: trans_acc
  - Maximum translational deceleration, in length/sec/sec; nonpositive.
    Zero means use the robot's default value.
- rot_acc (angle)
  - Default: 0
  - Maximum rotational acceleration, in angle/sec/sec; nonnegative.
    Zero means use the robot's default value.
- rot_decel (angle)
  - Default: rot_acc
  - Maximum rotational deceleration, in angle/sec/sec; nonpositive.
    Zero means use the robot's default value.
- pid_trans_p (integer)
  - Default: -1
  - Translational PID setting; proportional gain.
    Negative means use the robot's default value.
- pid_trans_d (integer)
  - Default: -1
  - Translational PID setting; derivative gain.
    Negative means use the robot's default value.
- pid_rot_p (integer)
  - Default: -1
  - Rotational PID setting; proportional gain.
    Negative means use the robot's default value.
- pid_rot_d (integer)
  - Default: -1
  - Rotational PID setting; derivative gain.
    Negative means use the robot's default value.
- motor_pwm_frequency (integer)
  - Default: -1
  - Frequency of motor PWM.
    Bounds determined by robot.
    Negative means use the robot's default value.
- motor_pwm_max_on (float)
  - Default: 1
  - Maximum motor duty cycle.
- save_settings_in_robot (integer)
  - Default: 0
  - A value of 1 installs current settings as default values in the robot.


@par Example

@verbatim
driver
(
  name "erratic"
  plugin "erratic"

  provides [ "position2d:0"
             "power:0"
             "sonar:0"
             "aio:0"
             "ir:0"
             "ptz:0" 
             "ptz:1" ]

  port "/dev/erratic"

  max_trans_vel 3
  max_rot_vel 720

  trans_acc 1
  rot_acc 200

  direct_wheel_vel_control 0
)
@endverbatim

@author Joakim Arfvidsson, Brian Gerkey, Kasper Stoy, James McKenna
*/
/** @} */



// This must be first per pthread reference
#include <pthread.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <termios.h>
#include <stdlib.h> // for abs()

#include "erratic.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

bool debug_mode = FALSE;



/** Setting up, tearing down **/

// These get the driver in where it belongs, loading
Driver* Erratic_Init(ConfigFile* cf, int section) 
{
  return (Driver*)(new Erratic(cf,section));
}

void Erratic_Register(DriverTable* table) 
{
  table->AddDriver("erratic", Erratic_Init);
}

#if 0
extern "C" {
  int player_driver_init(DriverTable* table)
  {
    Erratic_Register(table);
    #define QUOTEME(x) #x
    //#define DATE "##VIDERE_DATE##\"
    printf("  Erratic robot driver %s, build date %s\n", ERRATIC_VERSION, ERRATIC_DATE);
    return 0;
  }
}
#endif

// Constructor of the driver from configuration entry
Erratic::Erratic(ConfigFile* cf, int section) 
  : Driver(cf,section,true,PLAYER_MSGQUEUE_DEFAULT_MAXLEN) 
{
  // zero ids, so that we'll know later which interfaces were requested
  memset(&this->position_id, 0, sizeof(player_devaddr_t));
  memset(&this->power_id, 0, sizeof(player_devaddr_t));
  memset(&this->aio_id, 0, sizeof(player_devaddr_t));
  memset(&this->sonar_id, 0, sizeof(player_devaddr_t));
  memset(&this->ptz_id, 0, sizeof(player_devaddr_t));
  memset(&this->ptz2_id, 0, sizeof(player_devaddr_t));

  memset(&this->last_position_cmd, 0, sizeof(player_position2d_cmd_vel_t));
  memset(&this->last_car_cmd, 0, sizeof(player_position2d_cmd_car_t));

  this->position_subscriptions = 0;
  this->aio_ir_subscriptions = 0;
  this->sonar_subscriptions = 0;
  this->ptz_subscriptions = 0;
  this->ptz2_subscriptions = 0;

  // intialise members
  motor_packet = NULL;
  mcount = 0;
  memset(&this->erratic_data,0,sizeof(this->erratic_data));

  // Do we create a robot position interface?
  if(cf->ReadDeviceAddr(&(this->position_id), section, "provides", PLAYER_POSITION2D_CODE, -1, NULL) == 0) {
    if(this->AddInterface(this->position_id) != 0) {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a power interface?
  if(cf->ReadDeviceAddr(&(this->power_id), section, "provides", PLAYER_POWER_CODE, -1, NULL) == 0) {
    if(this->AddInterface(this->power_id) != 0) {
      this->SetError(-1);
      return;
    }
  }
  
  // Do we create a aio interface?
  if(cf->ReadDeviceAddr(&(this->aio_id), section, "provides", PLAYER_AIO_CODE, -1, NULL) == 0) {
    if(this->AddInterface(this->aio_id) != 0) {
      this->SetError(-1);
      return;
    }
  }

  // Do we create an ir interface?
  if(cf->ReadDeviceAddr(&(this->ir_id), section, "provides", PLAYER_IR_CODE, -1, NULL) == 0) {
    if(this->AddInterface(this->ir_id) != 0) {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a sonar interface?
  if(cf->ReadDeviceAddr(&(this->sonar_id), section, "provides", PLAYER_SONAR_CODE, -1, NULL) == 0) {
    if(this->AddInterface(this->sonar_id) != 0) {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a tilt-only ptz interface?
	// Must have a key of "tilt", e.g., "tilt:::ptz:1"
	// This device must be listed after the default PTZ device
  if(cf->ReadDeviceAddr(&(this->ptz2_id), section, "provides", PLAYER_PTZ_CODE, -1, "tilt") == 0) {
    if(this->AddInterface(this->ptz2_id) != 0) {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a standard ptz interface?
	// NOTE: this interface has no key, it picks up the default PTZ device
	// MUST BE LISTED FIRST of all PTZ devices
  if(cf->ReadDeviceAddr(&(this->ptz_id), section, "provides", PLAYER_PTZ_CODE, -1, NULL) == 0) {
    if(this->AddInterface(this->ptz_id) != 0) {
      this->SetError(-1);
      return;
    }
  }

  // build the table of robot parameters.
  initialize_robot_params();

  // Read config file options
  this->psos_serial_port = cf->ReadString(section,"port",DEFAULT_VIDERE_PORT);
  this->direct_wheel_vel_control = cf->ReadInt(section, "direct_wheel_vel_control", 0);
  this->motor_max_speed = (int)rint(1e3 * cf->ReadLength(section, "max_trans_vel", MOTOR_DEF_MAX_SPEED));
  this->motor_max_turnspeed = (int)rint(RTOD(cf->ReadAngle(section, "max_rot_vel", MOTOR_DEF_MAX_TURNSPEED)));
  this->motor_max_trans_accel = (short)rint(1.0e3 * cf->ReadLength(section, "trans_acc", 0));
  this->motor_max_trans_decel = (short)rint(1.0e3 * cf->ReadLength(section, "trans_decel", 0));
  this->motor_max_rot_accel = (short)rint(RTOD(cf->ReadAngle(section, "rot_acc", 0)));
  this->motor_max_rot_decel = (short)rint(RTOD(cf->ReadAngle(section, "rot_decel", 0)));

  this->pid_trans_p = cf->ReadInt(section, "pid_trans_p", -1);
  this->pid_trans_v = cf->ReadInt(section, "pid_trans_v", -1);
  this->pid_trans_i = cf->ReadInt(section, "pid_trans_i", -1);
  this->pid_rot_p = cf->ReadInt(section, "pid_rot_p", -1);
  this->pid_rot_v = cf->ReadInt(section, "pid_rot_v", -1);
  this->pid_rot_i = cf->ReadInt(section, "pid_rot_i", -1);

  this->motor_pwm_frequency = cf->ReadInt(section, "motor_pwm_frequency", -1);
  this->motor_pwm_max_on = (uint16_t)(cf->ReadFloat(section, "motor_pwm_max_on", -1) * 1000.0);

  this->use_vel_band = 0;//cf->ReadInt(section, "use_vel_band", 0);

  this->print_all_packets = 0;
  this->print_status_summary = 1;

  debug_mode = cf->ReadInt(section, "debug", 0);
  save_settings_in_robot = cf->ReadInt(section, "save_settings_in_robot", 0);

  this->read_fd = -1;
  this->write_fd = -1;
  
  // Data mutexes and semaphores
  pthread_mutex_init(&send_queue_mutex, 0);
  pthread_cond_init(&send_queue_cond, 0);
  pthread_mutex_init(&motor_packet_mutex, 0);
  
  if (Connect()) 
    {
      printf("Error connecting to Erratic robot\n");
      exit(1);
    }
}

// Called by player when the driver is asked to connect
int Erratic::Setup() {
  // We don't care, we connect at startup anyway
  return 0;
}

// Establishes connection and starts threads
int Erratic::Connect() 
{
  printf("  Erratic connection initializing (%s)...",this->psos_serial_port); fflush(stdout);

  //Try opening both our file descriptors
  if ((this->read_fd = open(this->psos_serial_port, O_RDONLY, S_IRUSR | S_IWUSR)) < 0) {
    perror("Erratic::Setup():open():");
    return(1);
  };
  if ((this->write_fd = open(this->psos_serial_port, O_WRONLY, S_IRUSR | S_IWUSR)) < 0) {
    perror("Erratic::Setup():open():");
    close(this->read_fd); this->read_fd = -1;
    return(1);
  }

  // Populate term and set initial baud rate
  int bauds[] = {B38400, B115200};
  int numbauds = sizeof(bauds), currbaud = 0;
  struct termios read_term, write_term;
  {
    if(tcgetattr( this->read_fd, &read_term ) < 0 ) {
      perror("Erratic::Setup():tcgetattr():");
      close(this->read_fd); close(this->write_fd);
      this->read_fd = -1; this->write_fd = -1;
      return(1);
    } 
    
    cfmakeraw(&read_term);
    //cfsetspeed(&read_term, bauds[currbaud]);
    cfsetispeed(&read_term, bauds[currbaud]);
    cfsetospeed(&read_term, bauds[currbaud]);
      
    if(tcsetattr(this->read_fd, TCSAFLUSH, &read_term ) < 0) {
      perror("Erratic::Setup():tcsetattr(read channel):");
      close(this->read_fd); close(this->write_fd);
      this->read_fd = -1; this->write_fd = -1;
      return(1);
    }
  }
  
  
  // Empty buffers 
  if (tcflush(this->read_fd, TCIOFLUSH ) < 0) {
    perror("Erratic::Setup():tcflush():");
    close(this->read_fd); close(this->write_fd);
    this->read_fd = -1; this->write_fd = -1;
    return(1);
  }
  if (tcflush(this->write_fd, TCIOFLUSH ) < 0) {
    perror("Erratic::Setup():tcflush():");
    close(this->read_fd); close(this->write_fd);
    this->read_fd = -1; this->write_fd = -1;
    return(1);
  }
  
  // will send config packets until a response is gotten
  int num_sync_attempts = 10;
  int num_patience = 200;
  int failed = 0;
  enum { NO_SYNC, AFTER_FIRST_SYNC, AFTER_SECOND_SYNC, READY } communication_state = NO_SYNC;
  ErraticPacket received_packet;
  while(communication_state != READY && num_patience-- > 0) {
    // Send a configuration request
    if (num_patience % 5 == 0) {
      ErraticPacket packet;
      unsigned char command = (command_e)configuration;
      packet.Build(&command, 1);
      packet.Send(this->write_fd);
    }

    // Receive a reply to see if we got it yet
    uint8_t receive_error;
    if((receive_error = received_packet.Receive(this->read_fd))) {
      if (receive_error == (receive_result_e)failure)
        printf("Error receiving\n");
      // If we still have retries, just get another packet
      if((communication_state == NO_SYNC) && (num_sync_attempts >= 0)) {
        num_sync_attempts--;
        usleep(ROBOT_CYCLETIME);
        continue;
      }
      // Otherwise, try next speed or give up completely
      else {
      // Couldn't connect; try different speed.
        if(++currbaud < numbauds) {
        // Set speeds for the descriptors
          cfsetispeed(&read_term, bauds[currbaud]);
          cfsetospeed(&write_term, bauds[currbaud]);
          if( tcsetattr(this->read_fd, TCSAFLUSH, &read_term ) < 0 ) {
            perror("Erratic::Setup():tcsetattr():");
            close(this->read_fd); close(this->write_fd);
            this->read_fd = -1; this->write_fd = -1;
            return(1);
          }
          if( tcsetattr(this->write_fd, TCSAFLUSH, &write_term ) < 0 ) {
            perror("Erratic::Setup():tcsetattr():");
            close(this->read_fd); close(this->write_fd);
            this->read_fd = -1; this->write_fd = -1;
            return(1);
          }

          // Flush the descriptors
          if(tcflush(this->read_fd, TCIOFLUSH ) < 0 ) {
            perror("Erratic::Setup():tcflush():");
            close(this->read_fd); close(this->write_fd);
            this->read_fd = -1; this->write_fd = -1;
            return(1);
          }
          if(tcflush(this->write_fd, TCIOFLUSH ) < 0 ) {
            perror("Erratic::Setup():tcflush():");
            close(this->read_fd); close(this->write_fd);
            this->read_fd = -1; this->write_fd = -1;
            return(1);
          }

          // Give same slack to new speed
          num_sync_attempts = 10;
          continue;
        }
        // Tried all speeds, abort
        else failed = 1;
      }
    }

    // If we gave up
    if (failed) break;

    // If we successfully got a packet, check if it's the one we're waiting for
    if (received_packet.packet[3] == 0x20) communication_state = READY;

    usleep(ROBOT_CYCLETIME);
  }

  if(failed) {
    printf("  Couldn't synchronize with Erratic robot.\n");
    printf("  Most likely because the robot is not connected to %s\n", this->psos_serial_port);
    close(this->read_fd); close(this->write_fd);
    this->read_fd = -1; this->write_fd = -1;
    return(1);
  }
  else if (communication_state != READY) {
    printf("  Couldn't synchronize with Erratic robot.\n");
    printf("  We heard something though. Is the sending part of the cable dead?\n");
    close(this->read_fd); close(this->write_fd);
    this->read_fd = -1; this->write_fd = -1;
    return(1);
  }

  char name[20], type[20], subtype[20];
  sprintf(name, "%s", &received_packet.packet[5]);
  sprintf(type, "%s", &received_packet.packet[25]);
  sprintf(subtype, "%s", &received_packet.packet[45]);

  // Open the driver, and tickle it a bit
  {
    ErraticPacket packet;

    unsigned char command = (command_e)open_controller;
    packet.Build(&command, 1);
    packet.Send(this->write_fd);
    usleep(ROBOT_CYCLETIME);

    command = (command_e)pulse;
    packet.Build(&command, 1);
    packet.Send(this->write_fd);
    usleep(ROBOT_CYCLETIME);
  }

  printf(" done.\n  Connected to <%s>, an %s %s\n", name, type, subtype);

  // Set the robot type
	if (strcmp(subtype,"Rev H")==0)
		param_idx = 2;
	else 	if (strcmp(subtype,"Rev G")==0)
		param_idx = 1;
	else 	if (strcmp(subtype,"Rev E")==0)
		param_idx = 0;
	else
		param_idx = 0;

  // Create a packet and set initial odometry position (the ErraticMotorPacket is persistent)
  this->motor_packet = new ErraticMotorPacket(param_idx);
  this->motor_packet->x_offset = 0;
  this->motor_packet->y_offset = 0;
  this->motor_packet->angle_offset = 0;

  // If requested, set max accel/decel limits
  {
    ErraticPacket *accel_packet;
    unsigned char accel_command[4];
    if(this->motor_max_trans_accel != 0) {
      accel_packet = new ErraticPacket();
      accel_command[0] = (command_e)set_max_trans_acc;
      accel_command[1] = (argtype_e)argint;
      accel_command[2] = this->motor_max_trans_accel & 0x00FF;
      accel_command[3] = (this->motor_max_trans_accel & 0xFF00) >> 8;
      accel_packet->Build(accel_command, 4);
      this->Send(accel_packet);
    }
    if(this->motor_max_trans_decel != 0) {
      accel_packet = new ErraticPacket();
      accel_command[0] = (command_e)set_max_trans_acc;
      accel_command[1] = (argtype_e)argnint;
      accel_command[2] = abs(this->motor_max_trans_decel) & 0x00FF;
      accel_command[3] = (abs(this->motor_max_trans_decel) & 0xFF00) >> 8;
      accel_packet->Build(accel_command, 4);
      this->Send(accel_packet);
    } else if(this->motor_max_trans_accel != 0) {
        accel_packet = new ErraticPacket();
        accel_command[0] = (command_e)set_max_trans_acc;
        accel_command[1] = (argtype_e)argnint;
        accel_command[2] = this->motor_max_trans_accel & 0x00FF;
        accel_command[3] = (this->motor_max_trans_accel & 0xFF00) >> 8;
        accel_packet->Build(accel_command, 4);
        this->Send(accel_packet);
    }
    
    if(this->motor_max_rot_accel != 0) {
      accel_packet = new ErraticPacket();
      accel_command[0] = (command_e)set_max_rot_acc;
      accel_command[1] = (argtype_e)argint;
      accel_command[2] = this->motor_max_rot_accel & 0x00FF;
      accel_command[3] = (this->motor_max_rot_accel & 0xFF00) >> 8;
      accel_packet->Build(accel_command, 4);
      this->Send(accel_packet);
    }
    if(this->motor_max_rot_decel != 0) {
      accel_packet = new ErraticPacket();
      accel_command[0] = (command_e)set_max_rot_acc;
      accel_command[1] = (argtype_e)argnint;
      accel_command[2] = abs(this->motor_max_rot_decel) & 0x00FF;
      accel_command[3] = (abs(this->motor_max_rot_decel) & 0xFF00) >> 8;
      accel_packet->Build(accel_command, 4);
      this->Send(accel_packet);
    } else if(this->motor_max_rot_accel != 0) {
        accel_packet = new ErraticPacket();
        accel_command[0] = (command_e)set_max_rot_acc;
        accel_command[1] = (argtype_e)argnint;
        accel_command[2] = this->motor_max_rot_accel & 0x00FF;
        accel_command[3] = (this->motor_max_rot_accel & 0xFF00) >> 8;
        accel_packet->Build(accel_command, 4);
        this->Send(accel_packet);
    }
  }
  
  {
    ErraticPacket *packet;
    unsigned char payload[4];

    // If requested, download custom PID settings
    if(this->pid_trans_p >= 0) {
      packet = new ErraticPacket();
      payload[0] = (command_e)set_pid_trans_p;
      payload[1] = (argtype_e)argint;
      payload[2] = this->pid_trans_p & 0x00FF;
      payload[3] = (this->pid_trans_p & 0xFF00) >> 8;
      packet->Build(payload, 4);
      this->Send(packet);
    }
    if(this->pid_trans_v >= 0) {
      packet = new ErraticPacket();
      payload[0] = (command_e)set_pid_trans_v;
      payload[1] = (argtype_e)argint;
      payload[2] = this->pid_trans_v & 0x00FF;
      payload[3] = (this->pid_trans_v & 0xFF00) >> 8;
      packet->Build(payload, 4);
      this->Send(packet);
    }
    if(this->pid_trans_i >= 0) {
      packet = new ErraticPacket();
      payload[0] = (command_e)set_pid_trans_i;
      payload[1] = (argtype_e)argint;
      payload[2] = this->pid_trans_i & 0x00FF;
      payload[3] = (this->pid_trans_i & 0xFF00) >> 8;
      packet->Build(payload, 4);
      this->Send(packet);
    }
    if(this->pid_rot_p >= 0) {
      packet = new ErraticPacket();
      payload[0] = (command_e)set_pid_rot_p;
      payload[1] = (argtype_e)argint;
      payload[2] = this->pid_rot_p & 0x00FF;
      payload[3] = (this->pid_rot_p & 0xFF00) >> 8;
      packet->Build(payload, 4);
      this->Send(packet);
    }
    if(this->pid_rot_v >= 0) {
      packet = new ErraticPacket();
      payload[0] = (command_e)set_pid_rot_v;
      payload[1] = (argtype_e)argint;
      payload[2] = this->pid_rot_v & 0x00FF;
      payload[3] = (this->pid_rot_v & 0xFF00) >> 8;
      packet->Build(payload, 4);
      this->Send(packet);
    }
    if(this->pid_rot_i >= 0) {
      packet = new ErraticPacket();
      payload[0] = (command_e)set_pid_rot_i;
      payload[1] = (argtype_e)argint;
      payload[2] = this->pid_rot_i & 0x00FF;
      payload[3] = (this->pid_rot_i & 0xFF00) >> 8;
      packet->Build(payload, 4);
      this->Send(packet);
    }

    // If requested, set PWM parameters
    if (this->motor_pwm_frequency > 0) {
      packet = new ErraticPacket();
      payload[0] = (command_e)set_pwm_freq;
      payload[1] = (argtype_e)argint;
      payload[2] = this->motor_pwm_frequency & 0x00FF;
      payload[3] = (this->motor_pwm_frequency & 0xFF00) >> 8;
      packet->Build(payload, 4);
      this->Send(packet);
    }
    if (this->motor_pwm_max_on > 0) {
      packet = new ErraticPacket();
      payload[0] = (command_e)set_pwm_max_on;
      payload[1] = (argtype_e)argint;
      payload[2] = this->motor_pwm_max_on & 0x00FF;
      payload[3] = (this->motor_pwm_max_on & 0xFF00) >> 8;
      packet->Build(payload, 4);
      this->Send(packet);
    }

    // If requested, make robot save all settings
    if (save_settings_in_robot) {
      packet = new ErraticPacket();
      payload[0] = (command_e)save_config;
      payload[1] = 0;
      payload[2] = 0;
      payload[3] = 0;
      packet->Build(payload, 4);
      this->Send(packet);
    }
  }
  
  // TODO: figure out what the right behavior here is

  // zero position command buffer
    //player_position_cmd_t zero;
    //memset(&zero,0,sizeof(player_position_cmd_t));
    //this->PutCommand(this->position_id,(void*)&zero,
      //sizeof(player_position_cmd_t),NULL);


  /* now spawn reading thread */
  ((Erratic*)this)->StartThreads();
  return(0);
}

// Called by player when the driver is supposed to disconnect
int Erratic::Shutdown() {
  // We don't care, we'll never disconnect
  // Oh, yes, you will!
  this->Disconnect();
  return 0;
}

// Is theoretically able to disconnect, but we don't do that
int Erratic::Disconnect() {
  printf("Shutting Erratic driver down\n");
  
  this->StopThreads();
  
  // If we're connected, send some kill commands before killing connections
  if (this->write_fd > -1) {
    unsigned char command[20],buffer[20];
    ErraticPacket packet;

    memset(buffer,0,20);
  
    command[0] = (command_e)stop;
    packet.Build(command, 1);
    packet.Send(this->write_fd);
    usleep(ROBOT_CYCLETIME);

    command[0] = (command_e)close_controller;
    packet.Build(command, 1);
    packet.Send(this->write_fd);
    usleep(ROBOT_CYCLETIME);
  
    close(this->write_fd);
    this->write_fd = -1;
  }
  if (this->read_fd > -1) {
    close(this->read_fd);
    this->read_fd = -1;
  }

  if (this->motor_packet) {
    delete this->motor_packet;
    this->motor_packet = NULL;
  }
  
  printf("Erratic has been shut down");

  return(0);
}

// These call the supplied Driver::StartThread() method, but adds additional threads
void Erratic::StartThreads() {
  StartThread();
  pthread_create(&send_thread, 0, &SendThreadDummy, this);
  pthread_create(&receive_thread, 0, &ReceiveThreadDummy, this);
}
void Erratic::StopThreads() {
  pthread_cancel(send_thread);
  pthread_cancel(receive_thread);
  //TODO destroy threads?
  //TODO tickle threads to cancel point
  StopThread();
  
  // Cleanup mutexes
  pthread_mutex_trylock(&send_queue_mutex);
  pthread_mutex_unlock(&send_queue_mutex);
  
  pthread_mutex_trylock(&motor_packet_mutex);
  pthread_mutex_unlock(&motor_packet_mutex);
}

// Subscription is overridden to add a subscription count of our own
int Erratic::Subscribe(player_devaddr_t id) {
  int setupResult;                     
                                       
  // do the subscription               
  if((setupResult = Driver::Subscribe(id)) == 0)
  {
    // also increment the appropriate subscription counters

    if(Device::MatchDeviceAddress(id, this->position_id))
      this->position_subscriptions++;
    
    if(Device::MatchDeviceAddress(id, this->aio_id))
      this->aio_ir_subscriptions++;

    if(Device::MatchDeviceAddress(id, this->ir_id))
      this->aio_ir_subscriptions++;

    if(Device::MatchDeviceAddress(id, this->sonar_id))
      this->sonar_subscriptions++;

    if(Device::MatchDeviceAddress(id, this->ptz_id))
      this->ptz_subscriptions++;

    if(Device::MatchDeviceAddress(id, this->ptz2_id))
      this->ptz2_subscriptions++;
  }                                    
                                       
  return(setupResult);                 
}                                      
int Erratic::Unsubscribe(player_devaddr_t id) {
  int shutdownResult;

  // do the unsubscription
  if((shutdownResult = Driver::Unsubscribe(id)) == 0)
  {
    // also decrement the appropriate subscription counter
    if(Device::MatchDeviceAddress(id, this->position_id))
    {
      this->position_subscriptions--;
      assert(this->position_subscriptions >= 0);
    }

    if(Device::MatchDeviceAddress(id, this->aio_id))
      this->aio_ir_subscriptions--;

    if(Device::MatchDeviceAddress(id, this->ir_id))
      this->aio_ir_subscriptions--;

    if(Device::MatchDeviceAddress(id, this->sonar_id))
      this->sonar_subscriptions--;

    if(Device::MatchDeviceAddress(id, this->ptz_id))
      this->ptz_subscriptions--;

    if(Device::MatchDeviceAddress(id, this->ptz2_id))
      this->ptz2_subscriptions--;

  }

  return(shutdownResult);
}




/** Talking to the robot **/

// Listens to the robot
void Erratic::ReceiveThread() 
{
  for (;;) {
    pthread_testcancel();


    // Receive next packet from robot
    ErraticPacket packet;
    uint32_t waited = 0;
    uint8_t error_code;
    while ((error_code = packet.Receive(this->read_fd, 5000))) {
      waited += 5;
      printf("Lost serial communication with Erratic (%d) - no data received for %i seconds\n", error_code, waited);
    }
		
    if (waited)
      printf("Connection re-established\n");
		
		
    if (print_all_packets) {
      printf("Got: ");
      packet.PrintHex();
    }
		
    // Process the packet
    //Lock();
		
    int count, maxcount;
    switch(packet.packet[3]) 
      {
      case (reply_e)motor:
      case (reply_e)motor+2:
      case (reply_e)motor+3:
	if (motor_packet->Parse(&packet.packet[3], packet.size-3)) {
	  motor_packet->Fill(&erratic_data);
	  PublishPosition2D();
	  PublishPower();
	}
	break;

      case (reply_e)config:
	break;

      case (reply_e)ain:
	// This data goes in two places, analog input and ir rangers
        if(erratic_data.aio.voltages_count != (unsigned int)(packet.packet[4]+4))
        {
          erratic_data.aio.voltages_count = packet.packet[4]+4;
          if(erratic_data.aio.voltages)
            delete[] erratic_data.aio.voltages; 
          erratic_data.aio.voltages = new float[erratic_data.aio.voltages_count];
        }
        if(erratic_data.ir.voltages_count != 
           (unsigned int)RobotParams[this->param_idx]->NumIR)
        {
          erratic_data.ir.voltages_count = RobotParams[this->param_idx]->NumIR;
          if(erratic_data.ir.voltages)
            delete[] erratic_data.ir.voltages; 
          erratic_data.ir.voltages = new float[erratic_data.ir.voltages_count];
        }
	if(erratic_data.ir.ranges_count != 
           (unsigned int)RobotParams[this->param_idx]->NumIR)
        {
          erratic_data.ir.ranges_count = RobotParams[this->param_idx]->NumIR;
          if(erratic_data.ir.ranges)
            delete[] erratic_data.ir.ranges;
          erratic_data.ir.ranges = new float[erratic_data.ir.ranges_count];
        }
	unsigned int i_voltage;
	for (i_voltage = 0; i_voltage < erratic_data.aio.voltages_count ;i_voltage++) 
	  {
	    erratic_data.aio.voltages[i_voltage] = (packet.packet[5+i_voltage*2]
						    + 256*packet.packet[6+i_voltage*2]) * (1.0 / 1024.0) * CPU_VOLTAGE;
	    erratic_data.ir.voltages[i_voltage] = (packet.packet[5+i_voltage*2]
						   + 256*packet.packet[6+i_voltage*2]) * (1.0 / 1024.0) * CPU_VOLTAGE;
	    //						erratic_data.ir.ranges[i_voltage] = IRRangeFromVoltage(erratic_data.ir.voltages[i_voltage]);
	  }

	// add digital inputs, four E port bits
	for (int i=0; i < 4; i++) 
	  {
	    erratic_data.aio.voltages[i_voltage+i] = 
	      (packet.packet[5+i_voltage*2] & (0x1 << i+4)) ? 1.0 : 0.0;
	    erratic_data.ir.voltages[i] = 
	      (packet.packet[5+i_voltage*2] & (0x1 << i+4)) ? 1.0 : 0.0;
	    erratic_data.ir.ranges[i] = 
	      IRFloorRange(erratic_data.ir.voltages[i]);
	  }

					

	PublishAIn();
	PublishIR();
	break;

      case (reply_e)sonar:
	// Sonar packet
	maxcount = RobotParams[this->param_idx]->NumSonars;
	count = packet.packet[4];
        if(erratic_data.sonar.ranges_count != (unsigned int)maxcount)
        {
          erratic_data.sonar.ranges_count = maxcount;
          if(erratic_data.sonar.ranges)
            delete[] erratic_data.sonar.ranges;
          erratic_data.sonar.ranges = new float[erratic_data.sonar.ranges_count];
        }
	for (int i = 0; i < count; i++) 
	  {
	    int ch = packet.packet[5+i*3]; // channel number
	    if (ch >= maxcount)
	      continue;		// bad channel number
	    erratic_data.sonar.ranges[ch] = .001 * (double)(packet.packet[6+i*3]
							    + 256*packet.packet[7+i*3]);
//			printf("Sonar %d: %.2f\n", ch, erratic_data.sonar.ranges[ch]);
	  }

	PublishSonar();
	break;

      case (reply_e)debug:
	if (debug_mode) {
	  printf("Debug message: ");
	  for (uint8_t i = 3; i < packet.size-2 ;i++)
	    printf("%c", packet.packet[i]);
	  printf("\n");
	}
	break;

      default:
	if (debug_mode) {
	  printf("Unrecognized packet: ");
	  packet.Print();
	}
      }
		
    //Unlock();
  }
}

void* Erratic::ReceiveThreadDummy(void *driver) {
  ((Erratic*)driver)->ReceiveThread();
  return 0;
}

// Sends to the robot
void Erratic::SendThread() {
  for (;;) {
    pthread_testcancel();
    
    // Get rights to the queue
    pthread_mutex_lock(&send_queue_mutex);
    
    // If there is nothing, we wait for signal
    if (!send_queue.size())
      pthread_cond_wait(&send_queue_cond, &send_queue_mutex);

    // Get first element and give the queue back
    ErraticPacket *packet = 0;
    if (send_queue.size()) {
      packet = send_queue.front();
      send_queue.pop();
    }
    pthread_mutex_unlock(&send_queue_mutex);
    
    // Send the packet and destroy it
    if (packet) {
      if (print_all_packets) {
        printf("Just about to send: ");
        packet->Print();
      }
      packet->Send(this->write_fd);
      // To not overload buffers on the robot, hold off a little bit
      //if (debug_mode)
      //  printf("Just sent it, I guess\n");
      // To not overload buffers on the robot, hold off a little bit
      usleep(15000);
    }
    delete(packet);
  }
}
void* Erratic::SendThreadDummy(void *driver) {
  ((Erratic*)driver)->SendThread();
  return 0;
}

// Queues for sending
void Erratic::Send(ErraticPacket *packet) {
  pthread_mutex_lock(&send_queue_mutex); {
    send_queue.push(packet);
    pthread_cond_signal(&send_queue_cond);
  } pthread_mutex_unlock(&send_queue_mutex);
}

// Tells the robot to reset the odometry center
// Just resets accumulation in this driver, not the controller
void Erratic::ResetRawPositions() {
  ErraticPacket *pkt;
  unsigned char erraticcommand[4];

  //**************************
  //  printf("Reset raw odometry\n");

  if(this->motor_packet) {
    pkt = new ErraticPacket();
		// don't reset raw values, else there will be a jump
		//    this->motor_packet->rawxpos = 0;
		//    this->motor_packet->rawypos = 0;
    this->motor_packet->xpos = 0;
    this->motor_packet->ypos = 0;
    this->motor_packet->x_offset = 0;
    this->motor_packet->y_offset = 0;
    this->motor_packet->angle_offset = 0;
    erraticcommand[0] = (command_e)reset_origo;
    erraticcommand[1] = (argtype_e)argint;

    //TODO this is removed as a hack
    //pkt->Build(erraticcommand, 2);
    //this->Send(pkt);
  }
}

// Enable or disable motors
void Erratic::ToggleMotorPower(unsigned char val) {
  unsigned char command[4];
  ErraticPacket *packet = new ErraticPacket();

  command[0] = (command_e)enable_motors;
  command[1] = (argtype_e)argint;
  command[2] = val;
  command[3] = 0;
  packet->Build(command, 4);
  
  Send(packet);
}

// Enable or disable analog input reporting
void Erratic::ToggleAIn(unsigned char val) 
{
  unsigned char command[4];
  ErraticPacket *packet = new ErraticPacket();

  command[0] = (command_e)set_analog;
  command[1] = (argtype_e)argint;
  command[2] = val ? 1 : 0;
  command[3] = 0;
  packet->Build(command, 4);
  
  Send(packet);  
}

// Enable or disable sonar reporting
void Erratic::ToggleSonar(unsigned char val) 
{
  unsigned char command[4];
  ErraticPacket *packet = new ErraticPacket();

  command[0] = (command_e)set_sonar;
  command[1] = (argtype_e)argint;
  command[2] = val ? 1 : 0;
  command[3] = 0;
  packet->Build(command, 4);
  
  Send(packet);  
}

// This describes the IR hardware
float Erratic::IRRangeFromVoltage(float voltage) 
{
  // This is for the Sharp 2Y0A02, 150 cm ranger
  return -.2475 + .1756 * voltage + .7455 / voltage - .0446 * voltage * voltage;
}

float Erratic::IRFloorRange(float value) 
{
  // floor range is 1 (floor present) or 0 (no floor)
  if (value >= 0.9)
    return 0.10;                // approximate location of floor from sensors
  else
    return 1.0;                  // far away
}


/** Talking to the Player architecture **/

// Main entry point for the worker thread
void Erratic::Main() {
  int last_position_subscrcount=0;
  int last_aio_ir_subscriptions=0;
  int last_sonar_subscriptions=0;

  for(;;)
  {
    pthread_testcancel();

    // Wait for some instructions
    //    Wait();
    usleep(10000);              // Wait() blocks too much, doesn't get subscriptions

    this->Lock();

    // we want to reset the odometry and enable the motors if the first
    // client just subscribed to the position device, and we want to stop
    // and disable the motors if the last client unsubscribed.
    if(!last_position_subscrcount && this->position_subscriptions)
    {
      this->ToggleMotorPower(0);
      this->ResetRawPositions();
    }
    else if(last_position_subscrcount && !(this->position_subscriptions))
    {
      // enable motor power
      this->ToggleMotorPower(1);
    }
    last_position_subscrcount = this->position_subscriptions;


    // We'll ask the robot to enable analog packets if we just got our
    // first subscriber
    if(!last_aio_ir_subscriptions && this->aio_ir_subscriptions)
      this->ToggleAIn(1);
    else if(last_aio_ir_subscriptions && !(this->aio_ir_subscriptions))
      this->ToggleAIn(0);
    last_aio_ir_subscriptions = this->aio_ir_subscriptions;


    // We'll ask the robot to enable sonar packets if we just got our
    // first subscriber
    if(!last_sonar_subscriptions && this->sonar_subscriptions)
      this->ToggleSonar(1);
    else if(last_sonar_subscriptions && !(this->sonar_subscriptions))
      this->ToggleSonar(0);
    last_sonar_subscriptions = this->sonar_subscriptions;

    this->Unlock();

    // handle pending messages
    //printf( "Will look for incoming messages\n" );
    if(!this->InQueue->Empty())
    {
      ProcessMessages();
    }
    else
    {
      // if no pending msg, resend the last position cmd.
      //TODO reenable?
      //this->HandlePositionCommand(this->last_position_cmd);
    }
    
    // Do some little nice printout
    if (print_status_summary) {
      
    }
  }
}

// Publishes the indicated interface data
void Erratic::PublishPosition2D() {
  this->Publish(this->position_id,
    PLAYER_MSGTYPE_DATA,
    PLAYER_POSITION2D_DATA_STATE,
    (void*)&(this->erratic_data.position),
    sizeof(player_position2d_data_t),
    NULL);
}
void Erratic::PublishPower() {
  this->Publish(this->power_id,
    PLAYER_MSGTYPE_DATA,
    PLAYER_POWER_DATA_STATE,
    (void*)&(this->erratic_data.power),
    sizeof(player_power_data_t),
    NULL);
}
void Erratic::PublishAIn() {
  this->Publish(this->aio_id,
    PLAYER_MSGTYPE_DATA,
    PLAYER_AIO_DATA_STATE,
    (void*)&(this->erratic_data.aio),
    sizeof(player_aio_data_t),
    NULL);
}
void Erratic::PublishIR() {
  this->Publish(this->ir_id,
    PLAYER_MSGTYPE_DATA,
    PLAYER_IR_DATA_RANGES,
    (void*)&(this->erratic_data.ir),
    sizeof(player_ir_data_t),
    NULL);
}
void Erratic::PublishSonar() {
  this->Publish(this->sonar_id,
    PLAYER_MSGTYPE_DATA,
    PLAYER_SONAR_DATA_RANGES,
    (void*)&(this->erratic_data.sonar),
    sizeof(player_sonar_data_t),
    NULL);
}
void Erratic::PublishAllData() {
  this->PublishPosition2D();
  this->PublishPower();
  this->PublishAIn();
  this->PublishIR();
  this->PublishSonar();
}

// Gets called from ProcessMessages to handle one message
int Erratic::ProcessMessage(QueuePointer &resp_queue, player_msghdr * hdr, void * data) {
  // Look for configuration requests
  if(hdr->type == PLAYER_MSGTYPE_REQ)
    return(this->HandleConfig(resp_queue,hdr,data));
  else if(hdr->type == PLAYER_MSGTYPE_CMD)
    return(this->HandleCommand(hdr,data));
  else
    return(-1);
}

// Handles one Player command with a configuration request
int Erratic::HandleConfig(QueuePointer &resp_queue, player_msghdr * hdr, void * data) {

  // check for position config requests
  if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                               PLAYER_POSITION2D_REQ_SET_ODOM,
                               this->position_id))
  {
    if(hdr->size != sizeof(player_position2d_set_odom_req_t))
    {
      PLAYER_WARN("Arg to odometry set requests wrong size; ignoring");
      return(-1);
    }
    player_position2d_set_odom_req_t* set_odom_req =
      (player_position2d_set_odom_req_t*)data;

    this->motor_packet->x_offset = ((int )rint(set_odom_req->pose.px*1e3))
                                    + this->motor_packet->xpos;
    this->motor_packet->y_offset = ((int)rint(set_odom_req->pose.py*1e3))
                                    + this->motor_packet->ypos;
    this->motor_packet->angle_offset = ((int)rint(RTOA(set_odom_req->pose.pa)))
                                     + this->motor_packet->angle;

    //**************************
		//    printf("Reset odometry: %f %f %f\n", set_odom_req->pose.px, set_odom_req->pose.py, set_odom_req->pose.pa);
		//    printf("Reset odometry: %d %d %d\n", motor_packet->x_offset, motor_packet->y_offset, motor_packet->angle_offset);
    

    this->Publish(this->position_id, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_SET_ODOM);
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                    PLAYER_POSITION2D_REQ_MOTOR_POWER,
                                    this->position_id)) {
    if(hdr->size != sizeof(player_position2d_power_config_t)) {
      PLAYER_WARN("Arg to motor state change request wrong size; ignoring");
      return(-1);
    }
    player_position2d_power_config_t* power_config =
      (player_position2d_power_config_t*)data;
    this->ToggleMotorPower(power_config->state);

    this->Publish(this->position_id, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_POSITION2D_REQ_MOTOR_POWER);
    return(0);
  } else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                      PLAYER_POSITION2D_REQ_RESET_ODOM,
                                      this->position_id)) {
    if(hdr->size != 0) {
      PLAYER_WARN("Arg to reset position request is wrong size; ignoring");
      return(-1);
    }

		this->motor_packet->x_offset = this->motor_packet->xpos;
		this->motor_packet->y_offset = this->motor_packet->ypos;;
		this->motor_packet->angle_offset = this->motor_packet->angle;;
		printf("Resetting odometry offsets: %d %d %d\n", motor_packet->xpos,
					 motor_packet->ypos, motor_packet->angle);

    this->Publish(this->position_id, resp_queue,
      PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_RESET_ODOM);
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                    PLAYER_POSITION2D_REQ_GET_GEOM,
                                    this->position_id)) {
    /* Return the robot geometry. */
    if(hdr->size != 0) {
      PLAYER_WARN("Arg get robot geom is wrong size; ignoring");
      return(-1);
    }
    player_position2d_geom_t geom;

    geom.pose.px = -RobotParams[param_idx]->RobotAxleOffset / 1e3;
    geom.pose.py = 0.0;
    geom.pose.pyaw = 0.0;

    geom.size.sl = RobotParams[param_idx]->RobotLength / 1e3;
    geom.size.sw = RobotParams[param_idx]->RobotWidth / 1e3;

    this->Publish(this->position_id, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_POSITION2D_REQ_GET_GEOM,
                  (void*)&geom, sizeof(geom), NULL);
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                    PLAYER_POSITION2D_REQ_VELOCITY_MODE,
                                    this->position_id)) {
    /* velocity control mode:
     *   0 = direct wheel velocity control
     *   1 = separate translational and rotational control
     */
      if(hdr->size != sizeof(player_position2d_velocity_mode_config_t))
    {
      PLAYER_WARN("Arg to velocity control mode change request is wrong "
        "size; ignoring");
      return(-1);
    }
    player_position2d_velocity_mode_config_t* velmode_config =
      (player_position2d_velocity_mode_config_t*)data;

    if(velmode_config->value)
      direct_wheel_vel_control = false;
    else
      direct_wheel_vel_control = true;

    this->Publish(this->position_id, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_POSITION2D_REQ_VELOCITY_MODE);
    return(0);
  }
  
  // Request for getting IR locations
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                    PLAYER_IR_REQ_POSE,
                                    this->ir_id)) {
    player_ir_pose_t pose;
    pose.poses_count = RobotParams[param_idx]->NumIR;
    for (uint16_t i = 0; i < pose.poses_count ;i++)
      pose.poses[i] = RobotParams[param_idx]->IRPose[i];
    
    this->Publish(this->ir_id, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_IR_REQ_POSE,
                  (void*)&pose, sizeof(pose), NULL);
    return(0);
  }
  // Request for getting sonar locations
  // two types of requests...
  else if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                    PLAYER_SONAR_DATA_GEOM,
                                    this->sonar_id)) 
    {
      player_sonar_geom_t pose;
      pose.poses_count = RobotParams[param_idx]->NumSonars;
      for (uint16_t i = 0; i < pose.poses_count ;i++)
        pose.poses[i] = RobotParams[param_idx]->sonar_pose[i];
    
      this->Publish(this->sonar_id, resp_queue,
                    PLAYER_MSGTYPE_RESP_ACK,
                    PLAYER_SONAR_DATA_GEOM,
                    (void*)&pose, sizeof(pose), NULL);
      return(0);
    }

  // Request for getting sonar locations
  // two types of requests...
  else if (Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                    PLAYER_SONAR_REQ_GET_GEOM,
                                    this->sonar_id)) 
    {
      player_sonar_geom_t pose;
      pose.poses_count = RobotParams[param_idx]->NumSonars;
      for (uint16_t i = 0; i < pose.poses_count ;i++)
        pose.poses[i] = RobotParams[param_idx]->sonar_pose[i];
    
      this->Publish(this->sonar_id, resp_queue,
                    PLAYER_MSGTYPE_RESP_ACK,
                    PLAYER_SONAR_REQ_GET_GEOM,
                    (void*)&pose, sizeof(pose), NULL);
      return(0);
    }

  else
    {
      PLAYER_WARN("unknown config request to erratic driver");
      return(-1);
    }
}

// Process car-like command, which sets an angular position target and
// translational velocity target.
// Erratic handles this natively
//
void 
Erratic::HandleCarCommand(player_position2d_cmd_car_t cmd)
{
  int speedDemand, angleDemand;
  unsigned short absspeedDemand, absangleDemand;
  unsigned char motorcommand[4];
  ErraticPacket *motorpacket;

  speedDemand = (int)rint(cmd.velocity * 1e3);  // convert to mm/s
  angleDemand = (int)rint(RTOD(cmd.angle)); // convert to deg heading
  angleDemand += (int)rint(ATOD(this->motor_packet->angle_offset));  // check for angle offset of odometry
	while (angleDemand > 360)        // normalize
    angleDemand -= 360;
  while (angleDemand < 0)
    angleDemand += 360;

  // do separate trans and rot vels
  motorcommand[0] = (command_e)trans_vel;
  if(speedDemand >= 0)
    motorcommand[1] = (argtype_e)argint;
  else
    motorcommand[1] = (argtype_e)argnint;

  absspeedDemand = (unsigned short)abs(speedDemand);
  if(absspeedDemand < this->motor_max_speed)
    {
      motorcommand[2] = absspeedDemand & 0x00FF;
      motorcommand[3] = (absspeedDemand & 0xFF00) >> 8;
    }
  else
    {
      motorcommand[2] = this->motor_max_speed & 0x00FF;
      motorcommand[3] = (this->motor_max_speed & 0xFF00) >> 8;
    }
    
    motorpacket = new ErraticPacket();
    motorpacket->Build(motorcommand, 4);
    Send(motorpacket);


  // do separate trans and rot vels
  motorcommand[0] = (command_e)rot_pos;
  if(angleDemand >= 0)
    motorcommand[1] = (argtype_e)argint;
  else
    motorcommand[1] = (argtype_e)argnint;

  absangleDemand = (unsigned short)abs(angleDemand);
  motorcommand[2] = absangleDemand & 0x00FF;
  motorcommand[3] = (absangleDemand & 0xFF00) >> 8;
    
  motorpacket = new ErraticPacket();
  motorpacket->Build(motorcommand, 4);
  Send(motorpacket);
}


// function to get the time in ms
int getms()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*1000 + tv.tv_usec/1000;
}


// Handles one Player command detailing a velocity
void Erratic::HandlePositionCommand(player_position2d_cmd_vel_t position_cmd) 
{
  int speedDemand, turnRateDemand;
  double leftvel, rightvel;
  double rotational_term;
  unsigned short absspeedDemand, absturnRateDemand;
  unsigned char motorcommand[4];
  ErraticPacket *motorpacket;

  speedDemand = (int)rint(position_cmd.vel.px * 1e3);
  turnRateDemand = (int)rint(RTOD(position_cmd.vel.pa));

  //speedDemand = 0;
  //turnRateDemand = 768;

  // throttle back on commands
  int ms = getms();
  if (mcount == 0) mcount = ms-200;
  if (ms < mcount + 50)          // at least 100 ms have to elapse
    return;
  mcount = ms;

  //if (debug_mode)
  //  printf("Will VW, %i and %i\n", speedDemand, turnRateDemand);

  // TODO: interpret this command as 2 velocities
  if(this->direct_wheel_vel_control)
  {
    // convert xspeed and yawspeed into wheelspeeds
    rotational_term = (M_PI/180.0) * turnRateDemand /
      RobotParams[param_idx]->DiffConvFactor;

    //printf("rotational_term: %g\n", rotational_term);

    leftvel = (speedDemand - rotational_term);
    rightvel = (speedDemand + rotational_term);

    //printf("Speeds in ticks: %g %g\n", leftvel, rightvel);

    // Apply wheel speed bounds
    if(fabs(leftvel) > this->motor_max_speed)
    {
      if(leftvel > 0)
      {
        leftvel = this->motor_max_speed;
        rightvel *= this->motor_max_speed/leftvel;
      }
      else
      {
        leftvel = -this->motor_max_speed;
        rightvel *= -this->motor_max_speed/leftvel;
      }
    }
    if(fabs(rightvel) > this->motor_max_speed)
    {
      if(rightvel > 0)
      {
        rightvel = this->motor_max_speed;
        leftvel *= this->motor_max_speed/rightvel;
      }
      else
      {
        rightvel = -this->motor_max_speed;
        leftvel *= -this->motor_max_speed/rightvel;
      }
    }

    // Apply control band bounds
    if(this->use_vel_band)
    {
      // This band prevents the wheels from turning in opposite
      // directions
      //TODO WHY is this here??
      // It's disabled elsewhere now.
      if (leftvel * rightvel < 0)
      {
        if (leftvel + rightvel >= 0)
        {
          if (leftvel < 0)
            leftvel = 0;
          if (rightvel < 0)
            rightvel = 0;
        }
        else
        {
          if (leftvel > 0)
            leftvel = 0;
          if (rightvel > 0)
            rightvel = 0;
        }
      }
    }

    // Apply byte range bounds
    if (leftvel / RobotParams[param_idx]->Vel2Divisor > 126)
      leftvel = 126 * RobotParams[param_idx]->Vel2Divisor;
    if (leftvel / RobotParams[param_idx]->Vel2Divisor < -126)
      leftvel = -126 * RobotParams[param_idx]->Vel2Divisor;
    if (rightvel / RobotParams[param_idx]->Vel2Divisor > 126)
      rightvel = 126 * RobotParams[param_idx]->Vel2Divisor;
    if (rightvel / RobotParams[param_idx]->Vel2Divisor < -126)
      rightvel = -126 * RobotParams[param_idx]->Vel2Divisor;

    // send the speed command
    motorcommand[0] = (command_e)wheel_vel;
    motorcommand[1] = (argtype_e)argint;
    motorcommand[2] = (char)(rightvel /
      RobotParams[param_idx]->Vel2Divisor);
    motorcommand[3] = (char)(leftvel /
      RobotParams[param_idx]->Vel2Divisor);

    motorpacket = new ErraticPacket();
    motorpacket->Build(motorcommand, 4);
    Send(motorpacket);
  }
  else
  {
    // do separate trans and rot vels
    motorcommand[0] = (command_e)trans_vel;
    if(speedDemand >= 0)
      motorcommand[1] = (argtype_e)argint;
    else
      motorcommand[1] = (argtype_e)argnint;

    absspeedDemand = (unsigned short)abs(speedDemand);
    if(absspeedDemand < this->motor_max_speed)
    {
      motorcommand[2] = absspeedDemand & 0x00FF;
      motorcommand[3] = (absspeedDemand & 0xFF00) >> 8;
    }
    else
    {
      motorcommand[2] = this->motor_max_speed & 0x00FF;
      motorcommand[3] = (this->motor_max_speed & 0xFF00) >> 8;
    }
    
    motorpacket = new ErraticPacket();
    motorpacket->Build(motorcommand, 4);
    Send(motorpacket);

    motorcommand[0] = (command_e)rot_vel;
    if(turnRateDemand >= 0)
      motorcommand[1] = (argtype_e)argint;
    else
      motorcommand[1] = (argtype_e)argnint;

    absturnRateDemand = (unsigned short)abs(turnRateDemand);
    if(absturnRateDemand < this->motor_max_turnspeed)
    {
      motorcommand[2] = absturnRateDemand & 0x00FF;
      motorcommand[3] = (absturnRateDemand & 0xFF00) >> 8;
    }
    else
    {
      motorcommand[2] = this->motor_max_turnspeed & 0x00FF;
      motorcommand[3] = (this->motor_max_turnspeed & 0xFF00) >> 8;
    }

    motorpacket = new ErraticPacket();
    motorpacket->Build(motorcommand, 4);
    Send(motorpacket);

  }
}

//
// Process PTZ command
// Zoom is not used
// For PTZ #0:
//    Pan is on servo 1, tilt on servo 2
// For PTZ #1:
//    Tilt is on servo 0
//
// Commands are in degrees, positive and negative
// Conversions are handled through the Erratic parameter structure,
//   which has parameters for conversion of degrees to servo counts
//   For now use fixed values
// Currently no position or velocity feedback
//

#define SERVO_NEUTRAL 1650	// in servo counts
#define SERVO_COUNTS_PER_DEGREE 6.5
#define SERVO_MAX_COUNT 2300
#define SERVO_MIN_COUNT 1100


void 
Erratic::HandlePtzCommand(player_ptz_cmd_t cmd, player_devaddr_t id)
{
  int pan, tilt, servo=0;
  unsigned char payload[6];
  ErraticPacket *packet;

  // send pan command
  pan = (int)(RTOD(cmd.pan));
  pan = (int)((double)pan*SERVO_COUNTS_PER_DEGREE + SERVO_NEUTRAL);
  if (pan > SERVO_MAX_COUNT)
    pan = SERVO_MAX_COUNT;
  if (pan < SERVO_MIN_COUNT)
    pan = SERVO_MIN_COUNT;

	//	printf("Send command to servo %d: %d / %d\n", 0, (int)(RTOD(cmd.pan)), pan);
	if (Device::MatchDeviceAddress(id,this->ptz_id))				// 1st pan/tilt unit
		{
			packet = new ErraticPacket();
			payload[0] = (command_e)servo_pos;
			payload[1] = (argtype_e)argstr;	
			payload[2] = 3;						// 3 bytes in string
			payload[3] = 2;						// servo #2
			payload[4] = pan&0xff;
			payload[5] = (pan&0xff00)>>8;
			packet->Build(payload, 6);
			this->Send(packet);
		}

  // send tilt command
  tilt = (int)(RTOD(cmd.tilt));
  tilt = (int)((double)tilt*SERVO_COUNTS_PER_DEGREE + SERVO_NEUTRAL);
  if (tilt > SERVO_MAX_COUNT)
    tilt = SERVO_MAX_COUNT;
  if (tilt < SERVO_MIN_COUNT)
    tilt = SERVO_MIN_COUNT;

	// printf("Send command to servo %d: %d / %d\n", 1, (int)(RTOD(cmd.tilt)), tilt);
	if (Device::MatchDeviceAddress(id,this->ptz_id))
		servo = 1;
	else if (Device::MatchDeviceAddress(id,this->ptz2_id))
		servo = 0;

  packet = new ErraticPacket();
  payload[0] = (command_e)servo_pos;
  payload[1] = (argtype_e)argstr;
  payload[2] = 3;								// 3 bytes in string
  payload[3] = servo;						// servo number
  payload[4] = tilt&0xff;
  payload[5] = (tilt&0xff00)>>8;
  packet->Build(payload, 6);
  this->Send(packet);
}


// Switchboard for robot commands, called from ProcessMessage
// Note that these don't perform a response...
int Erratic::HandleCommand(player_msghdr * hdr, void* data) 
{
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, this->position_id))
    {
      player_position2d_cmd_vel_t position_cmd = *(player_position2d_cmd_vel_t*)data;
      this->HandlePositionCommand(position_cmd);
    } 
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_CAR, this->position_id))
    {
      player_position2d_cmd_car_t position_cmd = *(player_position2d_cmd_car_t*)data;
      this->HandleCarCommand(position_cmd);
    } 
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL_HEAD, this->position_id))
    {
      player_position2d_cmd_car_t position_cmd = *(player_position2d_cmd_car_t*)data;
      this->HandleCarCommand(position_cmd);
    } 
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_PTZ_CMD_STATE, this->ptz_id))
    {
      player_ptz_cmd_t ptz_cmd = *(player_ptz_cmd_t*)data;
      this->HandlePtzCommand(ptz_cmd, this->ptz_id);
    }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_PTZ_CMD_STATE, this->ptz2_id))
    {
      player_ptz_cmd_t ptz_cmd = *(player_ptz_cmd_t*)data;
      this->HandlePtzCommand(ptz_cmd, this->ptz2_id);
    }
  else
    return(-1);

  return(0);
}
