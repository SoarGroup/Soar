/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003  Brian Gerkey gerkey@usc.edu
 *                           Andrew Howard ahoward@usc.edu
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
 *
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_roboteq roboteq
 * @brief Motor control driver for Roboteq AX2550
 
Provides position2d interface to the Roboteq AX2550 motor controller
http://www.roboteq.com/ax2550-folder.html
This driver ignores all configuration requests and produces no data
although the hardware device supports a number of configurations
and data over the serial link. It simply accepts 2 types of commmands
translation and rotation, then converts these to the syntax used by
the Roboteq. This driver uses the configuration file options
max_rot_spd and max_trans_spd to scale commands sent to the controller.
These values can be determined by testing with RC -- or closed loop
could be implemented by integrating dead-reckoning devices.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d

@par Requires

- None

@par Configuration requests

- none

@par Configuration file options

- devicepath (string)
  - Default: none
  - The serial port to be used.

- baud (integer)
  - Default: 9600
  - The baud rate to be used.

- max_rot_spd (float)
  - Default: none
  - maximum rotational speed (in rad/sec) that would be achieved
	by vehicle if full power where applied to that channel.

- max_trans_spd (float)
  - Default: none
  - maximum translational speed (in meters/sec) that would be achieved
	by vehicle if full power where applied to that channel.

@par Example

@verbatim
driver
(
  name "roboteq"
  provides ["position2d:0"] 
  devicepath "/dev/ttyS0"
  max_trans_spd 6.0
  max_rot_spd 4.0
)
@endverbatim

@author Pablo Rivera rivera@cse.unr.edu

*/
/** @} */

#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h> // ioctl
#include <unistd.h> // close(2),fcntl(2),getpid(2),usleep(3),execvp(3),fork(2)
#include <netdb.h> // for gethostbyname(3) 
#include <netinet/in.h>  // for struct sockaddr_in, htons(3) 
#include <sys/types.h>  // for socket(2) 
#include <sys/socket.h>  // for socket(2) 
#include <signal.h>  // for kill(2) 
#include <fcntl.h>  // for fcntl(2) 
#include <string.h>  // for strncpy(3),memcpy(3) 
#include <stdlib.h>  // for atexit(3),atoi(3) 
#include <pthread.h>  // for pthread stuff 
#include <math.h>

#include <libplayercore/playercore.h>

// settings
#define SERIAL_BUFF_SIZE		128
#define MAX_MOTOR_SPEED			127
#define ROBOTEQ_CON_TIMEOUT		10      // seconds to time-out on setting RS-232 mode
#define ROBOTEQ_DEFAULT_BAUD	9600 

#ifndef CRTSCTS
#ifdef IHFLOW
#ifdef OHFLOW
#define CRTSCTS ((IHFLOW) | (OHFLOW))
#endif
#endif
#endif

// *************************************
// some assumptions made by this driver:

// ROBOTEQ is in "mixed mode" where
// channel 1 is translation
// channel 2 is rotation

// ROBOTEQ is set to be in RC mode by default

// the robot is a skid-steer vehicle where
// left wheel(s) are on one output,
// right wheel(s) on the other.
// directionality is implied by the following
// macros (FORWARD,REVERSE,LEFT,RIGHT)
// so outputs may need to be switched

// *************************************

#define FORWARD "!A"
#define REVERSE "!a"
#define LEFT "!B"
#define RIGHT "!b"

///////////////////////////////////////////////////////////////////////////

class roboteq:public Driver
{
  private:
      int roboteq_fd;
      char serialin_buff[SERIAL_BUFF_SIZE];
      char serialout_buff[SERIAL_BUFF_SIZE];
      const char* devicepath;
      int roboteq_baud;
      double speed_scaling_factor, rot_scaling_factor;

      int FormMotorCmd(char* cmd_str, short trans_command, short rot_command);

      // current data
      player_position2d_data_t data;
      player_devaddr_t position2d_id;

  public:
    roboteq( ConfigFile* cf, int section);
    
    virtual int ProcessMessage(QueuePointer &resp_queue, 
						player_msghdr * hdr, void * data);
    virtual int Setup();
    virtual int Shutdown();
    virtual void Main();
};


///////////////////////////////////////////////////////////////////////////
// initialization function
Driver* roboteq_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new roboteq( cf, section)));
}


///////////////////////////////////////////////////////////////////////////
// a driver registration function
void roboteq_Register(DriverTable* table)
{
  table->AddDriver("roboteq",  roboteq_Init);
}

///////////////////////////////////////////////////////////////////////////
roboteq::roboteq( ConfigFile* cf, int section) : Driver(cf, section)
{  
	memset (&this->position2d_id, 0, sizeof (player_devaddr_t));

    // Outgoing position 2d interface
	if(cf->ReadDeviceAddr(&(this->position2d_id), section, "provides",
	               PLAYER_POSITION2D_CODE, -1, NULL) == 0){
		if(this->AddInterface(this->position2d_id) != 0){
			this->SetError(-1);
			return;		
	}   }

    double max_trans_spd, max_rot_spd;

  // required parameter(s)   
  if(!(this->devicepath = (char*)cf->ReadString(section, "devicepath", NULL))){
    PLAYER_ERROR("must specify devicepath");
    this->SetError(-1);
    return;
  }
  if(!(max_trans_spd = (double)cf->ReadFloat(section, "max_trans_spd", 0))){
    PLAYER_ERROR("must specify maximum translational speed");
    this->SetError(-1);
    return;
  }
  if(!(max_rot_spd = (double)cf->ReadFloat(section, "max_rot_spd", 0))){
    PLAYER_ERROR("must specify maximum rotational speed");
    this->SetError(-1);
    return;
  }
  fprintf(stderr, "Roboteq: using a max translational speed of %f m/s\n\tand max rotational speed of %f rad/s to scale motor commands\n", max_trans_spd, max_rot_spd);

  speed_scaling_factor = ((double)(MAX_MOTOR_SPEED / max_trans_spd));
  rot_scaling_factor = ((double)(MAX_MOTOR_SPEED / max_rot_spd));

  // optional parameter(s)
  roboteq_baud = cf->ReadInt(section, "baud", ROBOTEQ_DEFAULT_BAUD);

  memset(&data,0,sizeof(data));
  roboteq_fd = -1;

  return;
}

///////////////////////////////////////////////////////////////////////////
int
roboteq::Setup()
{
  int ret, i;

  fprintf(stderr, "Configuring Roboteq serial port at %s..\n", devicepath);
  roboteq_fd = open(devicepath, O_RDWR|O_NDELAY);
  if (roboteq_fd == -1){
    fputs("Unable to configure serial port for RoboteQ!", stderr);
    return 0; 
  }else{
      struct termios options;
      
      tcgetattr(roboteq_fd, &options);

      // default is 9600 unless otherwise specified

      if (roboteq_baud == 4800){
        cfsetispeed(&options, B4800);
        cfsetospeed(&options, B4800);
      }
      else if (roboteq_baud == 19200){
        cfsetispeed(&options, B19200);
        cfsetospeed(&options, B19200);
      }
      else if (roboteq_baud == 38400){
        cfsetispeed(&options, B38400);
        cfsetospeed(&options, B38400);
      }
      else{
        cfsetispeed(&options, B9600);
        cfsetospeed(&options, B9600);
      }

      // set to 7bit even parity, no flow control
      options.c_cflag |= (CLOCAL | CREAD);
      options.c_cflag |= PARENB;
      options.c_cflag &= ~PARODD;
      options.c_cflag &= ~CSTOPB;
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS7;
      options.c_cflag &= ~CRTSCTS;

      options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    // non-canonical

      tcsetattr(roboteq_fd, TCSANOW, &options);
      ioctl(roboteq_fd, TCIOFLUSH, 2);
  }


  // initialize RoboteQ to RS-232 mode
  strcpy(serialout_buff, "\r");
  for (i=0; i<10; i++){ 
    write(roboteq_fd, serialout_buff, 1);
    tcdrain(roboteq_fd);
	usleep(25000);
  }

    // check response from RoboteQ
	memset(serialin_buff, 0, SERIAL_BUFF_SIZE);
	ret = read(roboteq_fd, serialin_buff, SERIAL_BUFF_SIZE);
	int beg_time = time(NULL);
	bool mode_changed = true;
	while (! strchr(serialin_buff, 'W')){
		if ((time(NULL) - beg_time)>ROBOTEQ_CON_TIMEOUT){
			 mode_changed = false;
			 break;
		}
		memset(serialin_buff, 0, SERIAL_BUFF_SIZE);
		ret = read(roboteq_fd, serialin_buff, SERIAL_BUFF_SIZE);
	}
	if (!mode_changed)
		fputs("Failed to set Roboteq to RS-232 mode!\n", stderr);
	else
		fputs("Successfully initialized Roboteq connection.\n", stderr);

	fputs("Done.\n", stderr); 

  // now spawn reading thread 
  StartThread();

  return(0);
}


///////////////////////////////////////////////////////////////////////////
int roboteq::Shutdown()
{
  int ret;

  StopThread();

  // return RoboteQ to RC mode
  strcpy(serialout_buff, "^00 00\r");
  write(roboteq_fd, serialout_buff, 7);
  tcdrain(roboteq_fd);
  usleep(25000);
  strcpy(serialout_buff, "%rrrrrr\r");
  write(roboteq_fd, serialout_buff, 8);
  tcdrain(roboteq_fd);
  usleep(25000);

    // check response from RoboteQ
	memset(serialin_buff, 0, SERIAL_BUFF_SIZE);
	ret = read(roboteq_fd, serialin_buff, SERIAL_BUFF_SIZE);
	int beg_time = time(NULL);
	while (! strchr(serialin_buff, 'W')){
		if ((time(NULL) - beg_time)>ROBOTEQ_CON_TIMEOUT){
			// no 'W's for ROBOTEQ_CON_TIMEOUT seconds
			//		means we're probably in RC mode again

			// 07-09-07 
			// this test may need to change since the reset
			// appears to fail quite often. is it really
			// failing or is the test bad?
			return 0; 
		}
		memset(serialin_buff, 0, SERIAL_BUFF_SIZE);
		ret = read(roboteq_fd, serialin_buff, SERIAL_BUFF_SIZE);
	}
	fputs("Unable to reset Roboteq to RC mode!", stderr);

    close(roboteq_fd);
  
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
////////////////////////////////////////////////////////////////////////////////
int roboteq::ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
    assert(hdr);
	assert(data);
/*
    fprintf(stderr, "ProcessMessage: type=%d subtype=%d\n", 
            hdr->type, hdr->subtype);
  */      
	if (Message::MatchMessage(hdr, PLAYER_POSITION2D_REQ_MOTOR_POWER, 
                                PLAYER_POSITION2D_CMD_VEL, position2d_id)){         
        assert(hdr->size == sizeof(player_position2d_cmd_vel_t));

        player_position2d_cmd_vel_t & command 
            = *reinterpret_cast<player_position2d_cmd_vel_t *> (data);
        
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // convert from the generic position interface 
        // to the Roboteq-specific command
        // assumes "Mixed Mode" -- 
        // channel 1 : FW/BW speed
        // channel 2 : rotation
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        float vel_xtrans = command.vel.px;
        //float vel_ytrans = command.vel.py;
        float vel_yawspd = command.vel.pa;

        //fprintf(stderr, "ProcessMessage: trans=%f, steer=%f\n", vel_trans, vel_turret);
        
        // scale and translate to Roboteq command
        vel_xtrans = (double)vel_xtrans * speed_scaling_factor;
        vel_yawspd = (double)vel_yawspd * rot_scaling_factor;
        FormMotorCmd(serialout_buff, (short)vel_xtrans, (short)vel_yawspd);

        // write motor cmd
        write(roboteq_fd, serialout_buff, strlen(serialout_buff)+1);
        tcdrain(roboteq_fd);
		/*
        char* temp;
        while (temp = strchr(serialout_buff, '\r')) *temp = 32;
        puts(serialout_buff);
        fflush(stdout);
        */
        return 0;
    }
      
  return -1;
}

///////////////////////////////////////////////////////////////////////////
// Main driver thread runs here.
///////////////////////////////////////////////////////////////////////////
void roboteq::Main()
{
  double position_time=0;
    
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

  for(;;){
    ProcessMessages();
	pthread_testcancel();
    // publish dummy data
    Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, 
			  (unsigned char*) &data, sizeof(data), &position_time);
    usleep(10);
  }

  pthread_exit(NULL);
  
  return;
}


///////////////////////////////////////////////////////////////////////////
int roboteq::FormMotorCmd(char* cmd_str, short trans_command, short rot_command)
{
  char speed[8];
  char heading[8];

  if (trans_command > MAX_MOTOR_SPEED) trans_command = MAX_MOTOR_SPEED;
  else if (trans_command < -MAX_MOTOR_SPEED) trans_command = -MAX_MOTOR_SPEED;
  if (rot_command > MAX_MOTOR_SPEED) rot_command = MAX_MOTOR_SPEED;
  else if (rot_command < -MAX_MOTOR_SPEED) rot_command = -MAX_MOTOR_SPEED;
  
  if (trans_command > 0)
    strcpy(speed, FORWARD);
  else strcpy(speed, REVERSE);
  if (rot_command > 0)
    strcpy(heading, LEFT);
  else strcpy(heading, RIGHT);
  
  // form motor cmd string
  strcpy(cmd_str, speed);
  snprintf(cmd_str+2, 4, "%.2x", abs(trans_command)); // start at char 3
  strcat(cmd_str, "\r");	
  strcat(cmd_str, heading);
  snprintf(cmd_str + strlen(cmd_str), 4, "%.2x", abs(rot_command));
  strcat(cmd_str, "\r");
  
  return 0;
}




