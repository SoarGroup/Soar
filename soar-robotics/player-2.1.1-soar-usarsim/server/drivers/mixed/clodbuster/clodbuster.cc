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

/*
 * $Id: clodbuster.cc 4135 2007-08-23 19:58:48Z gerkey $
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_clodbuster clodbuster
 * @brief Clodbuster mobile robot

@todo This driver is currently disabled because it needs to be updated to
the Player 2.0 API.

The clodbuster driver controls the Clodbuster robot.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d

@par Requires

- none

@par Supported configuration requests

- PLAYER_POSITION2D_REQ_SET_ODOM
- PLAYER_POSITION2D_REQ_GET_GEOM
- PLAYER_POSITION2D_REQ_MOTOR_POWER
- PLAYER_POSITION2D_REQ_VELOCITY_MODE
- PLAYER_POSITION2D_REQ_RESET_ODOM
- PLAYER_POSITION2D_REQ_SPEED_PID

@par Configuration file options

- port (string)
  - Default: ""/dev/ttyUSB0" 
  - Serial port used to communicate with the robot.
  
@par Example 

@verbatim
driver
(
  name "clodbuster"
  provides ["position2d:0"]
)
@endverbatim

@author Ben Grocholsky
*/
/** @} */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h> // POSIX file i/o
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <math.h>
#include <stdlib.h>  /* for abs() */
#include <netinet/in.h> // socket things...
#include <termios.h> // serial port things

#include <libplayercore/playercore.h>
#include <replace/replace.h>
#include "clodbuster.h"
#include "packet.h" // What's this for?

// initialization function
Driver* ClodBuster_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new ClodBuster( cf, section)));
}

// a driver registration function
void 
ClodBuster_Register(DriverTable* table)
{
  table->AddDriver("clodbuster",  ClodBuster_Init);
}


ClodBuster::ClodBuster( ConfigFile* cf, int section)
        : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_POSITION2D_CODE)
{
  clodbuster_fd = -1;

  speedDemand=0, turnRateDemand=0;

  
  strncpy(clodbuster_serial_port,
          cf->ReadString(section, "port", DEFAULT_CLODBUSTER_PORT),
          sizeof(clodbuster_serial_port));

  // set parameters
  CountsPerRev = 408;
  WheelRadius = 0.076;
  WheelBase = .2921;
  WheelSeparation = .275;
  Kenc = 2*M_PI*WheelRadius/CountsPerRev;
  LoopFreq = 5;

  // set PID gains
  //  Kv = new PIDGains(-1.0,-1.5,0.0,LoopFreq);
  //  Kw = new PIDGains(-0.5112,-1.5,0.0,LoopFreq);
  Kv = new PIDGains(-10,-20.0,0.0,LoopFreq);
  Kw = new PIDGains(-5,-20.0,0.0,LoopFreq);
}

ClodBuster::~ClodBuster()
{
  delete Kv;
  delete Kw;
}

int ClodBuster::Setup()
{
  //  int i;
  // this is the order in which we'll try the possible baud rates. we try 9600
  // first because most robots use it, and because otherwise the radio modem
  // connection code might not work (i think that the radio modems operate at
  // 9600).
  //int baud = B38400;
  //  int numbauds = sizeof(bauds);
  // int currbaud = 0;

  struct termios term;
  //unsigned char command;
  // GRASPPacket packet, receivedpacket;
  int flags;
  //bool sent_close = false;
  
  printf("clodbuster connection initializing (%s)...",clodbuster_serial_port);
  fflush(stdout);

  if((clodbuster_fd = open(clodbuster_serial_port, 
			   O_RDWR | O_SYNC, S_IRUSR | S_IWUSR )) < 0 ) // O_NONBLOCK later..
    {
      perror("ClodBuster::Setup():open():");
      return(1);
    }  
 
  if( tcgetattr( clodbuster_fd, &term ) < 0 )
    {
      perror("ClodBuster::Setup():tcgetattr():");
      close(clodbuster_fd);
      clodbuster_fd = -1;
      return(1);
    }
  
  cfmakeraw( &term );
  term.c_cc[VTIME] = 10; /* wait 1 second on port A */
  term.c_cc[VMIN] = 0;  

  cfsetispeed(&term, B38400);
  cfsetospeed(&term, B38400);
  
  if( tcsetattr( clodbuster_fd, TCSAFLUSH, &term ) < 0 )
    {
      perror("ClodBuster::Setup():tcsetattr():");
      close(clodbuster_fd);
      clodbuster_fd = -1;
      return(1);
    }

  if( tcflush( clodbuster_fd, TCIOFLUSH ) < 0 )
    {
      perror("ClodBuster::Setup():tcflush():");
      close(clodbuster_fd);
      clodbuster_fd = -1;
      return(1);
    }

  if((flags = fcntl(clodbuster_fd, F_GETFL)) < 0)
    {
      perror("ClodBuster::Setup():fcntl()");
      close(clodbuster_fd);
      clodbuster_fd = -1;
      return(1);
    }
  /* turn on blocking mode */
  flags &= ~O_NONBLOCK;
  fcntl(clodbuster_fd, F_SETFL, flags);

  
  usleep(CLODBUSTER_CYCLETIME_USEC);
  
  GRASPPacket packet; 
  // disable motor power
  packet.Build(SET_SLEEP_MODE,SLEEP_MODE_OFF);
  packet.Send(clodbuster_fd);
  // reset odometry
  ResetRawPositions();
 
  direct_command_control = true;
  
  /* now spawn reading thread */
  StartThread();
  return(0);
}

int ClodBuster::Shutdown()
{
  GRASPPacket packet; 

  if(clodbuster_fd == -1)
    {
      return(0);
    }

  StopThread();

  packet.Build(SET_SLEEP_MODE,SLEEP_MODE_OFF);
  packet.Send(clodbuster_fd);
  usleep(CLODBUSTER_CYCLETIME_USEC);

  close(clodbuster_fd);
  clodbuster_fd = -1;
  puts("ClodBuster has been shutdown");
 
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int ClodBuster::ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{

  assert(hdr);
  assert(data);
	
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_SET_ODOM, device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_set_odom_req_t));
    player_position2d_set_odom_req_t & set_odom_req = *((player_position2d_set_odom_req_t*)data);
    
    this->position_data.pos = set_odom_req.pose;

    Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_SET_ODOM);
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_GET_GEOM, device_addr))
  {
    player_position2d_geom_t geom={{0}};
	
    // TODO : get values from somewhere.
    geom.pose.px = -0.1;//htons((short) (-100));
    geom.pose.py = 0;//htons((short) (0));
    geom.pose.pyaw = 0;//htons((short) (0));
    geom.size.sw = 0.5;//htons((short) (2 * 250));
    geom.size.sl = 0.45;//htons((short) (2 * 225));

    Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_GET_GEOM, &geom, sizeof(geom));
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_MOTOR_POWER, device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_power_config_t));
    player_position2d_power_config_t & power_config = *((player_position2d_power_config_t*)data);
    GRASPPacket packet; 
		
    if(power_config.state==1)
      packet.Build(SET_SLEEP_MODE,SLEEP_MODE_OFF);
    else 
       packet.Build(SET_SLEEP_MODE,SLEEP_MODE_ON);

    packet.Send(clodbuster_fd);

    Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_MOTOR_POWER);
    return 0;
  }

  /* velocity control mode:
   *   0 = direct wheel velocity control (default)
   *   1 = separate translational and rotational control
   */
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_VELOCITY_MODE, device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_velocity_mode_config_t));
    player_position2d_velocity_mode_config_t & velmode_config = *((player_position2d_velocity_mode_config_t*)data);
		
    if(velmode_config.value)
      direct_command_control = false;
    else
      direct_command_control = true;

    Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_VELOCITY_MODE);
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_RESET_ODOM, device_addr))
  {
    ResetRawPositions();

    Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_RESET_ODOM);
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_SPEED_PID, device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_speed_pid_req_t));
	player_position2d_speed_pid_req_t & pid = *((player_position2d_speed_pid_req_t*)data);
		
    kp = static_cast<int> (pid.kp);
    ki = static_cast<int> (pid.ki);
    kd = static_cast<int> (pid.kd);

    Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_SPEED_PID);
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
	player_position2d_cmd_vel_t & command = *((player_position2d_cmd_vel_t*)data);
		
    newmotorspeed = false;
    if( speedDemand != (int) command.vel.px)
      newmotorspeed = true;
    speedDemand = (int) command.vel.px;
      
    newmotorturn = false;
    if(turnRateDemand != (int) command.vel.pa)
      newmotorturn = true;
    turnRateDemand = (int) command.vel.pa;	

    return 0;
  }	
  
  return -1;
}

void 
ClodBuster::Main()
{
//  player_position_speed_pid_req_t pid;
//  unsigned char config[CLODBUSTER_CONFIG_BUFFER_SIZE];
  
  
//  int config_size;

  GetGraspBoardParams();

  // memory for PID
  int uVmax=max_limits[SET_SERVO_THROTTLE];//-center_limits[SET_SERVO_THROTTLE];
  int uVmin=min_limits[SET_SERVO_THROTTLE];//-center_limits[SET_SERVO_THROTTLE];
  int uV0=center_limits[SET_SERVO_THROTTLE];
  int uWmax=max_limits[SET_SERVO_FRONTSTEER];//-center_limits[SET_SERVO_FRONTSTEER];
  int uWmin=min_limits[SET_SERVO_FRONTSTEER];//-center_limits[SET_SERVO_FRONTSTEER];
  int uW0=center_limits[SET_SERVO_FRONTSTEER];
  float uV = uV0, uW=uW0, errV[3]={0}, errW[3]={0}, uVlast=uV0, uWlast=uW0,V;

  printf("V max min centre %d %d %d\n",uVmax,uVmin,uV0);
  printf("W max min centre %d %d %d\n",uWmax,uWmin,uW0);

  for(;;)
    {
      ProcessMessages();
      pthread_testcancel();
      
      // read encoders & update pose and velocity
      encoder_measurement = ReadEncoders();
      
      DifferenceEncoders();
      IntegrateEncoders();
      
      // remember old values
      old_encoder_measurement = encoder_measurement;
      
      Publish(device_addr,PLAYER_MSGTYPE_DATA,PLAYER_POSITION2D_DATA_STATE,(void*)&this->position_data,
                    sizeof(player_position2d_data_t),NULL);
      //      printf("left: %d , right %d count: %u\n",encoder_measurement.left,encoder_measurement.right,encoder_measurement.time_count);
      //      printf("left: %d , right %d\n",encoder_measurement.left-encoder_offset.left,encoder_measurement.right-encoder_offset.right);


      // do control
      /* NEXT, write commands */
      if(!direct_command_control) // fix this it's reversed for playerv
	{
	  // do direct wheel velocity control here
	  //printf("speedDemand: %d\t turnRateDemand: %d\n",
	  //speedDemand, turnRateDemand);
	  unsigned char v,w;
	  v = SetServo(SET_SERVO_THROTTLE,speedDemand);
	  w = SetServo(SET_SERVO_FRONTSTEER,turnRateDemand);
	  printf("The vel/turn command numbers : v:%d (%u) w:%d (%u)\n",center_limits[SET_SERVO_THROTTLE]+speedDemand,v,center_limits[SET_SERVO_FRONTSTEER]+turnRateDemand,w);
    
	  
	}
      else
	{
	     //	  printf("Do PID !\n");
	     // find tracking errors
	     errV[0] = EncV - speedDemand*1e-3;
	     errW[0] = EncOmega - turnRateDemand*M_PI/180.0;
	     
	     // find actions
	     uV = uVlast + Kv->K1()*errV[0] + + Kv->K2()*errV[1] + Kv->K3()*errV[2];
	     if (uV > uVmax)
		  {
		       uV = uVmax;
		       printf("+V control saturated!\n");
		  }
	     if (uV < uVmin)
		  {
		       uV = uVmin;
		       printf("-V control saturated!\n");
		  }
	     printf("V loop err: %f, u = %f, r = %f, x = %f\n",errV[0], uV,speedDemand*1e-3,EncV);
	     //	     if (speedDemand*1e-3 > .0125) 
	     if (fabs(EncV) > .0125) 
		  {
		    if (fabs(EncV) <.1)
		      if (EncV <.1)
			V = -.1;
		      else
			V = .1;
		    else
		      V = EncV;
		    // NB "-" sign for wrong convention +ve -> left 
		    uW = uWlast - WheelBase/V*(Kw->K1()*errW[0] + + Kw->K2()*errW[1] + Kw->K3()*errW[2]);
		    //   uW = uWlast - 1e3*WheelBase/speedDemand*(Kw->K1()*errW[0] + + Kw->K2()*errW[1] + Kw->K3()*errW[2]);
		  }
	     else
		  {
		       // set it to zero (center)
		       uW = uW0;
		       uWlast = uW0;
		       for (int i=0;i<3;i++)
			    errW[i] = 0.;
		  } 
	     if (uW > uWmax)
		  {
		       uW = uWmax;
		       printf("+W control saturated!\n");
		  }
	     if (uW < uWmin)
		  {
		       uW = uWmin;
		       printf("-W control saturated!\n");
		  }
	     printf("W loop err: %f, u = %f, r = %f, x = %f\n",errW[0], uW,turnRateDemand*M_PI/180.0,EncOmega);
	     
	     // Write actions to control board
	     SetServo(SET_SERVO_THROTTLE,(unsigned char) uV);
	     SetServo(SET_SERVO_FRONTSTEER,(unsigned char) uW);
	     
	     for (int i=1;i<3;i++)
	       {
		 errW[i] = errW[i-1];
		 errV[i] = errV[i-1];
	       }
	     uVlast = uV;
	     uWlast = uW;

	}
      usleep(200000);
    }
  pthread_exit(NULL);
}

void
ClodBuster::ResetRawPositions()
{
  encoder_offset=ReadEncoders();
}

clodbuster_encoder_data_t  ClodBuster::ReadEncoders()
{ GRASPPacket packet,rpacket;
 clodbuster_encoder_data_t temp;

 packet.Build(ECHO_ENCODER_COUNTS_TS);
 packet.Send(clodbuster_fd);

 rpacket.Receive(clodbuster_fd,ECHO_ENCODER_COUNTS_TS);
 rpacket.size=rpacket.retsize;
 rpacket.PrintHex();

 // make 24bit signed int a 32bit signed int
 temp.left = (rpacket.packet[0]<<16)|(rpacket.packet[1]<<8)|(rpacket.packet[2]);
 if (rpacket.packet[0] & 0x80)
   temp.left |= 0xff<<24;
 temp.right = (rpacket.packet[3]<<16)|(rpacket.packet[4]<<8)|(rpacket.packet[5]);
 if (rpacket.packet[3] & 0x80)
   temp.right |= 0xff<<24;
 // temp.left = (((char) rpacket.packet[0])<<16)|(rpacket.packet[1]<<8)|(rpacket.packet[2]);
 // temp.right = (((char) rpacket.packet[3])<<16)|(rpacket.packet[4]<<8)|(rpacket.packet[5]);

 // get timer count 32bit unsigned int
 temp.time_count = (rpacket.packet[6] << 24)|(rpacket.packet[7] <<16)|(rpacket.packet[8]<<8)|(rpacket.packet[9]);

 return temp;

}

void ClodBuster::GetGraspBoardParams()
{
  GRASPPacket packet,rpacket;
  packet.Build(ECHO_MAX_SERVO_LIMITS);
  packet.Send(clodbuster_fd);
  printf("Servo Limit Enquiry: ");

  packet.PrintHex();

  rpacket.Receive(clodbuster_fd,ECHO_MAX_SERVO_LIMITS);
  
  memcpy(max_limits,rpacket.packet,8);
  rpacket.size = rpacket.retsize;
  
  rpacket.PrintHex();

  packet.Build(ECHO_MIN_SERVO_LIMITS);
  packet.Send(clodbuster_fd);
   printf("Servo Limit Enquiry: ");

  packet.PrintHex();
 
  rpacket.Receive(clodbuster_fd,ECHO_MIN_SERVO_LIMITS);
  rpacket.size = rpacket.retsize;
  
  rpacket.PrintHex();
  memcpy(min_limits,rpacket.packet,8);
  packet.Build(ECHO_CEN_SERVO_LIMITS);
  packet.Send(clodbuster_fd);
  printf("Servo Limit Enquiry: ");

  packet.PrintHex();

  rpacket.Receive(clodbuster_fd,ECHO_CEN_SERVO_LIMITS);
  
  memcpy(center_limits,rpacket.packet,8);
  rpacket.size = rpacket.retsize;
  
  rpacket.PrintHex();
}

unsigned char 
ClodBuster::SetServo(unsigned char chan, int value)
{
  GRASPPacket spacket;
  unsigned char cmd;
  int demanded = center_limits[chan]+value/10;

  if (demanded > max_limits[chan])
    cmd = (unsigned char) max_limits[chan];
  else if (demanded < min_limits[chan])
    cmd = (unsigned char) min_limits[chan];
  else
    cmd = (unsigned char)demanded;
  spacket.Build(chan,cmd);
  spacket.Send(clodbuster_fd);
  // spacket.PrintHex();

  return(cmd);
}
void 
ClodBuster::SetServo(unsigned char chan, unsigned char cmd)
{
  GRASPPacket spacket;
  spacket.Build(chan,cmd);
  spacket.Send(clodbuster_fd);
  // spacket.PrintHex();
}

void ClodBuster::IntegrateEncoders()
{
  // assign something to xpos,ypos, theta
  float dEr = encoder_measurement.right-old_encoder_measurement.right; 
  float dEl = encoder_measurement.left-old_encoder_measurement.left; 
  float L = Kenc*(dEr+dEl)*.5;
  float D = Kenc*(dEr-dEl)/WheelSeparation;
  float Phi = this->position_data.pos.pa+0.5*D;

  this->position_data.pos.px += L*cos(Phi);
  this->position_data.pos.py += L*sin(Phi);
  this->position_data.pos.pa += D;
}

void ClodBuster::DifferenceEncoders()
{
  float dEr = encoder_measurement.right-old_encoder_measurement.right; 
  float dEl = encoder_measurement.left-old_encoder_measurement.left; 
  int64_t dtc =  encoder_measurement.time_count-old_encoder_measurement.time_count;
  if (dtc< -2147483648LL) 
       {
	    dtc += 4294967296LL; //(1<<32);
	    printf("encoder timer rollover caught\n");
       }
  float dt = dtc*1.6e-6;
  if (dt < 20e-3)
       printf("dt way too short %f s\n",dt);
  else if  (dt > 2.0/LoopFreq)
       printf("dt way too long %f s\n",dt);
       
  EncV = Kenc*(dEr+dEl)*.5/dt;
  EncOmega = Kenc*(dEr-dEl)/WheelSeparation/dt;     
  EncVleft = dEl/dt;
  EncVright = dEr/dt;
  printf("EncV = %f, EncW = %f, dt = %f\n",EncV,EncOmega*180/M_PI,dt);

}
