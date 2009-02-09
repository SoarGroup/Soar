/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *
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

/* Copyright (C) 2004
 *   Toby Collett, University of Auckland Robotics Group
 */


/** @ingroup drivers */
/** @{ */
/** @defgroup driver_khepera khepera
 * @brief K-Team Khepera mobile robot

The khepera driver is used to interface to the K-Team khepera robot. 

This driver is experimental and should be treated with caution. At
this point it supports the @ref interface_position2d and 
@ref interface_ir interfaces.

TODO: 
 - Add support for position control (currently only velocity control)
 - Add proper calibration for IR sensors

@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d
- @ref interface_ir

@par Requires

- none

@par Supported configuration requests

- The @ref interface_position2d interface supports:
  - PLAYER_POSITION2D_REQ_GET_GEOM
  - PLAYER_POSITION2D_REQ_MOTOR_POWER
  - PLAYER_POSITION2D_REQ_VELOCITY_MODE
  - PLAYER_POSITION2D_REQ_RESET_ODOM
  - PLAYER_POSITION2D_REQ_SET_ODOM
- The @ref interface_ir interface supports:
  - PLAYER_IR_REQ_POSE

@par Configuration file options

- port (string)
  - Default: "/dev/ttyUSB0"
  - Serial port used to communicate with the robot.
- scale_factor (float)
  - Default: 10
  - As the khepera is so small the actual geometry doesnt make much sense
    with many of the existing defaults so the geometries can all be scaled
    by this factor.
- encoder_res (float)
  - Default: 1.0/12.0
  - The wheel encoder resolution.
- position_pose (float tuple)
  - Default: [0 0 0]
  - The pose of the robot in player coordinates (mm, mm, deg).
- position_size (float tuple)
  - Default: [57 57]
  - The size of the robot approximated to a rectangle (mm, mm).
- ir_pose_count (integer)
  - Default: 8
  - The number of ir poses.
- ir_poses (float tuple)
  - Default: [10 24 90 19 17 45 25 6 0 25 -6 0 19 -17 -45 10 -24 -90 -24 -10 180 -24 10 180]
  - Poses of the IRs (mm mm deg for each one)

@par Example 

@verbatim
driver
(
  name "khepera"
  provides ["position2d:0" "ir:0"]
)
@endverbatim

@author Toby Collett
*/
/** @} */

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>  /* for abs() */
#include <netinet/in.h>
#include <ctype.h>

#include <assert.h>

#include <khepera.h>

#include <libplayercore/playercore.h>
#include <libplayerxdr/playerxdr.h>

// we need to debug different things at different times
//#define DEBUG_POS
//#define DEBUG_SERIAL
#define DEBUG_CONFIG

// useful macros
#define DEG2RAD(x) (((double)(x))*0.01745329251994)
#define RAD2DEG(x) (((double)(x))*57.29577951308232)

//#define DEG2RAD_FIX(x) ((x) * 17453)
//#define RAD2DEG_FIX(x) ((x) * 57295780)
#define DEG2RAD_FIX(x) ((x) * 174)
#define RAD2DEG_FIX(x) ((x) * 572958)


/* initialize the driver.
 *
 * returns: pointer to new REBIR object
 */
Driver*
Khepera_Init(ConfigFile *cf, int section)
{
  return (Driver *) new Khepera( cf, section);
}

/* register the Khepera IR driver in the drivertable
 *
 * returns: 
 */
void
Khepera_Register(DriverTable *table) 
{
  table->AddDriver("khepera", Khepera_Init);
}

int Khepera::ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr, void * data)
{
	assert(hdr);
	assert(data);

	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_IR_REQ_POSE, ir_addr))
	{
		Publish(ir_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype, &geometry->ir, sizeof(geometry->ir));
		return 0;
	}
	else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, position_addr))
	{
		player_position2d_cmd_vel_t * poscmd = reinterpret_cast<player_position2d_cmd_vel_t *> (data);

		// need to calculate the left and right velocities
		int transvel = static_cast<int> (static_cast<int> (poscmd->vel.px) * geometry->encoder_res);
		int rotvel = static_cast<int> (static_cast<int> (poscmd->vel.pa) * geometry->encoder_res * geometry->position.size.sw* geometry->scale);
		int leftvel = transvel - rotvel;
		int rightvel = transvel + rotvel;

		// now we set the speed
		if (this->motors_enabled) 
			SetSpeed(leftvel,rightvel);
		else 
			SetSpeed(0,0);
		return 0;
	}
	else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_GET_GEOM, position_addr))
	{
		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype, &geometry->position, sizeof(geometry->position));
		return 0;
	}
	else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_MOTOR_POWER, position_addr))
	{
		this->motors_enabled = ((player_position2d_power_config_t *)data)->state;
		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,hdr->subtype);
		return 0;
	}
	else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_VELOCITY_MODE, position_addr))
	{
		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,hdr->subtype);
		return 0;
	}
	else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_RESET_ODOM, position_addr))
	{
		ResetOdometry();
		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,hdr->subtype);
		return 0;
	}
	else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_SET_ODOM, position_addr))
	{
		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK,hdr->subtype);
		return 0;
	}

	return -1;
}

Khepera::Khepera(ConfigFile *cf, int section) : Driver(cf, section)
{
  // zero ids, so that we'll know later which interfaces were requested
  memset(&position_addr, 0, sizeof(this->position_addr));
  memset(&ir_addr, 0, sizeof(ir_addr));

  this->position_subscriptions = this->ir_subscriptions = 0;

  // Do we create a robot position interface?
  if(cf->ReadDeviceAddr(&(this->position_addr), section, "provides", 
                      PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->position_addr) != 0)
    {
      this->SetError(-1);    
      return;
    }
  }

  // Do we create an ir interface?
  if(cf->ReadDeviceAddr(&(this->ir_addr), section, "provides", 
                      PLAYER_IR_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->ir_addr) != 0)
    {
      this->SetError(-1);    
      return;
    }
  }

  // Read config file options
  geometry = new player_khepera_geom_t;
  geometry->PortName= NULL;
  geometry->scale = 0;

  //set up the poll parameters... used for the comms
  // over the serial port to the Kam
  //write_pfd.events = POLLOUT;
  //read_pfd.events = POLLIN;

  // now we have to look up our parameters.  this should be given as an argument
  if (geometry->PortName == NULL)
    geometry->PortName = strdup(cf->ReadString(section, "port", KHEPERA_DEFAULT_SERIAL_PORT));
  if (geometry->scale == 0)
    geometry->scale = cf->ReadFloat(section, "scale_factor", KHEPERA_DEFAULT_SCALE);

  geometry->encoder_res = cf->ReadFloat(section,"encoder_res", KHEPERA_DEFAULT_ENCODER_RES);

  // Load position config
  geometry->position.pose.px = cf->ReadTupleFloat(section,"position_pose",0,0) * geometry->scale;
  geometry->position.pose.py = cf->ReadTupleFloat(section,"position_pose",1,0) * geometry->scale;
  geometry->position.pose.pyaw = cf->ReadTupleFloat(section,"position_pose",2,0) * geometry->scale;

  // load dimension of the base
  geometry->position.size.sw = cf->ReadTupleFloat(section,"position_size",0,57) * geometry->scale;
  geometry->position.size.sl = cf->ReadTupleFloat(section,"position_size",1,57) * geometry->scale;

  // load ir geometry config
  geometry->ir.poses_count = (cf->ReadInt(section,"ir_pose_count", 8));
  geometry->ir.poses = new player_pose3d_t[geometry->ir.poses_count];
  if (geometry->ir.poses_count == 8 && cf->ReadTupleFloat(section,"ir_poses",0,-1) == -1)
  {
    // load the default ir geometry
    geometry->ir.poses[0].px = 10/1000*geometry->scale;
    geometry->ir.poses[0].py = 24/1000*geometry->scale;
    geometry->ir.poses[0].pyaw = DTOR(90);

    geometry->ir.poses[1].px = 19/1000*geometry->scale;
    geometry->ir.poses[1].py = 17/1000*geometry->scale;
    geometry->ir.poses[1].pyaw = DTOR(45);

    geometry->ir.poses[2].px = 25/1000*geometry->scale;
    geometry->ir.poses[2].py = 6/1000*geometry->scale;
    geometry->ir.poses[2].pyaw = DTOR(0);

    geometry->ir.poses[3].px = 25/1000*geometry->scale;
    geometry->ir.poses[3].py = -6/1000*geometry->scale;
    geometry->ir.poses[3].pyaw = DTOR(0);

    geometry->ir.poses[4].px = 19/1000*geometry->scale;
    geometry->ir.poses[4].py = -17/1000*geometry->scale;
    geometry->ir.poses[4].pyaw = DTOR(-45);

    geometry->ir.poses[5].px = 10/1000*geometry->scale;
    geometry->ir.poses[5].py = -24/1000*geometry->scale;
    geometry->ir.poses[5].pyaw = DTOR(-90);

    geometry->ir.poses[6].px = -24/1000*geometry->scale;
    geometry->ir.poses[6].py = -10/1000*geometry->scale;
    geometry->ir.poses[6].pyaw = DTOR(180);

    geometry->ir.poses[7].px = -24/1000*geometry->scale;
    geometry->ir.poses[7].py = 10/1000*geometry->scale;
    geometry->ir.poses[7].pyaw = DTOR(180);
  }
  else
  {
    // laod geom from config file
    for (unsigned int i = 0; i < geometry->ir.poses_count; ++i)
    {
      geometry->ir.poses[i].px = cf->ReadTupleFloat(section,"ir_poses",3*i,0)*geometry->scale;
      geometry->ir.poses[i].py = cf->ReadTupleFloat(section,"ir_poses",3*i+1,0)*geometry->scale;
      geometry->ir.poses[i].pyaw = cf->ReadTupleFloat(section,"ir_poses",3*i+2,0);
    }				
  }
  // laod ir calibration from config file
  geometry->ir_calib_a = new double[geometry->ir.poses_count];
  geometry->ir_calib_b = new double[geometry->ir.poses_count];
  for (unsigned int i = 0; i < geometry->ir.poses_count; ++i)
  {
    geometry->ir_calib_a[i] = cf->ReadTupleFloat(section,"ir_calib_a", i, KHEPERA_DEFAULT_IR_CALIB_A);
    geometry->ir_calib_b[i] = cf->ReadTupleFloat(section,"ir_calib_b", i, KHEPERA_DEFAULT_IR_CALIB_B);
  }

  // zero position counters
  last_lpos = 0;
  last_rpos = 0;
  last_x_f=0;
  last_y_f=0;
  last_theta = 0.0;
  
  
}

Khepera::~Khepera()
{
  delete geometry->ir.poses;
  delete geometry->ir_calib_a;
  delete geometry->ir_calib_b;
  delete geometry;
}

int 
Khepera::Subscribe(player_devaddr_t addr)
{
  int setupResult;

  // do the subscription
  if((setupResult = Driver::Subscribe(addr)) == 0)
  {
    // also increment the appropriate subscription counter
    switch(addr.interf)
    {
      case PLAYER_POSITION2D_CODE:
        this->position_subscriptions++;
        break;
      case PLAYER_IR_CODE:
        this->ir_subscriptions++;
        break;
    }
  }

  return(setupResult);
}

int 
Khepera::Unsubscribe(player_devaddr_t addr)
{
  int shutdownResult;

  // do the unsubscription
  if((shutdownResult = Driver::Unsubscribe(addr)) == 0)
  {
    // also decrement the appropriate subscription counter
    switch(addr.interf)
    {
      case PLAYER_POSITION2D_CODE:
        assert(--this->position_subscriptions >= 0);
        break;
      case PLAYER_IR_CODE:
        assert(--this->ir_subscriptions >= 0);
        break;
    }
  }

  return(shutdownResult);
}

/* called the first time a client connects
 *
 * returns: 0 on success
 */
int 
Khepera::Setup()
{
  // open and initialize the serial to -> Khepera  
  printf("Khepera: connection initializing (%s)...", this->khepera_serial_port);
  fflush(stdout);
  Serial = new KheperaSerial(geometry->PortName,KHEPERA_BAUDRATE);
  if (Serial == NULL || !Serial->Open())
  {
    return 1;
  }
  printf("Done\n");

  refresh_last_position = false;
  motors_enabled = false;
  velocity_mode = true;
  direct_velocity_control = false;

  desired_heading = 0;

  /* now spawn reading thread */
  StartThread();
  return(0);
}


int 
Khepera::Shutdown()
{
  printf("Khepera: SHUTDOWN\n");
  StopThread();

  // Killing the thread seems to leave out serial
  // device in a bad state, need to fix this,
  // till them we just dont stop the robot
  // which is theoretically quite bad...but this is the khepera...
  // it cant do that much damage its only 7cm across
  //SetSpeed(0,0);
  delete Serial;
  Serial = NULL;

  return(0);
}

void 
Khepera::Main()
{
  int last_position_subscrcount=0;

  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

  while (1) 
  {
    // we want to reset the odometry and enable the motors if the first 
    // client just subscribed to the position device, and we want to stop 
    // and disable the motors if the last client unsubscribed.
    if(!last_position_subscrcount && this->position_subscriptions)
    {
      printf("Khepera: first pos sub. turn off and reset\n");
      // then first sub for pos, so turn off motors and reset odom
      SetSpeed(0,0);
      ResetOdometry();

    } 
    else if(last_position_subscrcount && !(this->position_subscriptions))
    {
      // last sub just unsubbed
      printf("Khepera: last pos sub gone\n");
      SetSpeed(0,0);
    }
    last_position_subscrcount = this->position_subscriptions;


	ProcessMessages();
    pthread_testcancel();

    // now lets get new data...
    UpdateData();

    pthread_testcancel();
  }
  pthread_exit(NULL);
}



/* this will update the data that is sent to clients
 * just call separate functions to take care of it
 *
 * returns:
 */
void
Khepera::UpdateData()
{
  player_position2d_data_t position_data;
  player_ir_data_t ir_data;
  ir_data.ranges_count = geometry->ir.poses_count;
  ir_data.voltages_count = geometry->ir.poses_count;
  ir_data.ranges = new float[ir_data.ranges_count];
  ir_data.voltages = new float[ir_data.voltages_count];

  UpdatePosData(&position_data);

  // put position data
  Publish(position_addr,PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, (unsigned char *) &position_data, sizeof(player_position2d_data_t),NULL);

  UpdateIRData(&ir_data);

  // put ir data
  Publish(position_addr,PLAYER_MSGTYPE_DATA, PLAYER_IR_DATA_RANGES, (unsigned char *) &ir_data, sizeof(ir_data),NULL);
  player_ir_data_t_cleanup(&ir_data);
}

/* this will update the IR part of the client data
 * it entails reading the currently active IR sensors
 * and then changing their state to off and turning on
 * 2 new IRs.  
 *
 * returns:
 */
void
Khepera::UpdateIRData(player_ir_data_t * d)
{
  ReadAllIR(d);

  for (unsigned int i =0; i < geometry->ir.poses_count; i++) 
  {
    d->ranges[i] = geometry->scale * geometry->ir_calib_a[i] * pow(d->voltages[i],geometry->ir_calib_b[i]);
    d->voltages[i] = d->voltages[i];
  }
}

  
/* this will update the position data.  this entails odometry, etc
 */ 
void
Khepera::UpdatePosData(player_position2d_data_t *d)
{
  // calculate position data
  int pos_left, pos_right;
  Khepera::ReadPos(&pos_left, &pos_right);
  int change_left = pos_left - last_lpos;
  int change_right = pos_right - last_rpos;
  last_lpos = pos_left;
  last_rpos = pos_right;

  double transchange = (change_left + change_right) * geometry->encoder_res / 2;
  double rotchange = (change_left - change_right) * geometry->encoder_res / 2;

  double dx,dy,Theta;
  double r = geometry->position.size.sw*geometry->scale/2;

  if (transchange == 0)
  {
    Theta = 360 * rotchange/(2 * M_PI * r);	
    dx = dy= 0;
  }
  else if (rotchange == 0)
  {
    dx = transchange;
    dy = 0;
    Theta = 0;
  }
  else
  {
    Theta = 360 * rotchange/(2 * M_PI * r);
    double R = transchange * r / rotchange;
    dy = R - R*cos(Theta*M_PI/180);
    dx = R*sin(Theta*M_PI/180);
  }

  // add code to read in the speed data
  int left_vel, right_vel;
  Khepera::ReadSpeed(&left_vel, &right_vel);
  double lv = left_vel * geometry->encoder_res;
  double rv = right_vel * geometry->encoder_res;
  double trans_vel = 100 * (lv + rv)/2;
  double rot_vel = (lv - rv)/2;
  double rot_vel_deg = 100 * 360 * rot_vel/(2 * M_PI * r);

  // now write data
  double rad_Theta = DTOR(yaw);
  x+=(dx*cos(rad_Theta) + dy*sin(rad_Theta));
  y+=(dy*cos(rad_Theta) + dx*sin(rad_Theta));
  d->pos.px = x/geometry->scale;
  d->pos.py = y/geometry->scale;
  yaw += Theta;
  while (yaw > 360) yaw -= 360;
  while (yaw < 0) yaw += 360;
  d->pos.pa = DTOR(yaw);
  d->vel.px = trans_vel/geometry->scale;
  d->vel.pa = DTOR(rot_vel_deg);
}

/* this will set the odometry to a given position
 * ****NOTE: assumes that the arguments are in network byte order!*****
 *
 * returns: 
 */
int
Khepera::ResetOdometry()
{
  printf("Reset Odometry\n");
  int Values[2];
  Values[0] = 0;
  Values[1] = 0;
  if (Serial->KheperaCommand('G',2,Values,0,NULL) < 0)
    return -1;

  last_lpos = 0;
  last_rpos = 0;

  player_position2d_data_t data;
  memset(&data,0,sizeof(player_position2d_data_t));
  Publish(position_addr, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, &data, sizeof(data),NULL);

  x=y=yaw=0;
  return 0;
}


/* this will read the given AD channel
 *
 * returns: the value of the AD channel
 */
/*unsigned short
REB::ReadAD(int channel) 
{
  char buf[64];

  sprintf(buf, "I,%d\r", channel);
  write_command(buf, strlen(buf), sizeof(buf));
  
  return atoi(&buf[2]);
}*/

/* reads all the IR values at once.  stores them
 * in the uint16_t array given as arg ir
 *
 * returns: 
 */
int
Khepera::ReadAllIR(player_ir_data_t* d)
{
  int * Values;

  Values = new int [geometry->ir.poses_count];
  if(Serial->KheperaCommand('N',0,NULL,geometry->ir.poses_count,Values) < 0)
    return -1;			
  for (unsigned int i=0; i< geometry->ir.poses_count; ++i)
  {
    d->voltages[i] = static_cast<short> (Values[i]);
  }
  delete [] Values;
  return 0;
}

/* this will set the desired speed for the given motor mn
 *
 * returns:
 */
int
Khepera::SetSpeed(int speed1, int speed2)
{
	int Values[2];
	Values[0] = speed1;
	Values[1] = speed2;
	return Serial->KheperaCommand('D',2,Values,0,NULL);
}

/* reads the current speed of motor mn
 *
 * returns: the speed of mn
 */
int
Khepera::ReadSpeed(int * left,int * right)
{
	int Values[2];
	if (Serial->KheperaCommand('E',0,NULL,2,Values) < 0)
		return -1;
	*left = Values[0];
	*right = Values[1];
	return 0;
}

/* this sets the desired position motor mn should go to
 *
 * returns:
 */
/*void
REB::SetPos(int mn, int pos)
{
  char buf[64];
  
  sprintf(buf,"C,%c,%d\r", '0'+mn,pos);

  write_command(buf, strlen(buf), sizeof(buf));
}*/

/* this sets the position counter of motor mn to the given value
 *
 * returns:
 */
int
Khepera::SetPosCounter(int pos1, int pos2)
{
	int Values[2];
	Values[0] = pos1;
	Values[1] = pos2;
	return Serial->KheperaCommand('G',2,Values,0,NULL);
}

/* this will read the current value of the position counter
 * for motor mn
 *
 * returns: the current position for mn
 */
int
Khepera::ReadPos(int * pos1, int * pos2)
{
	int Values[2];
	if (Serial->KheperaCommand('H',0,NULL,2,Values) < 0)
		return -1;
	*pos1 = Values[0];
	*pos2 = Values[1];
	return 0;
}

/* this will configure the position PID for motor mn
 * using paramters Kp, Ki, and Kd
 *
 * returns:
 */
/*void
REB::ConfigPosPID(int mn, int kp, int ki, int kd)
{ 
  char buf[64];

  sprintf(buf, "F,%c,%d,%d,%d\r", '0'+mn, kp,ki,kd);
  write_command(buf, strlen(buf), sizeof(buf));
}*/

/* this will configure the speed PID for motor mn
 *
 * returns:
 */
/*void
REB::ConfigSpeedPID(int mn, int kp, int ki, int kd)
{
  char buf[64];
  
  sprintf(buf, "A,%c,%d,%d,%d\r", '0'+mn, kp,ki,kd);
  
  write_command(buf, strlen(buf), sizeof(buf));
}*/

/* this will set the speed profile for motor mn
 * it takes the max velocity and acceleration
 *
 * returns:
 */
/*void
REB::ConfigSpeedProfile(int mn, int speed, int acc)
{
  char buf[64];
  
  sprintf(buf, "J,%c,%d,%d\r", '0'+mn, speed,acc);
  write_command(buf, strlen(buf), sizeof(buf));
}*/

/* this will read the status of the motion controller.
 * mode is set to 1 if in position mode, 0 if velocity mode
 * error is set to the position/speed error
 *
 * returns: target status: 1 is on target, 0 is not on target
 */
/*unsigned char
Khepera::ReadStatus(int mn, int *mode, int *error)
{
  char buf[64];

  sprintf(buf, "K,%c\r", '0'+mn);
  //write_command(buf, strlen(buf), sizeof(buf));

  // buf now has the form "k,target,mode,error"
  int target;

  sscanf(buf, "k,%d,%d,%d", &target, mode, error);

  return (unsigned char)target;
}*/
