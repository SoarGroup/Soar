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
 *
 *   The White Box Robotics Model 914 robot
 *
 */


/** @ingroup drivers */
/** @{ */
/** @defgroup driver_wbr914 wbr914
 * @brief White Box Robotics Model 914 robot

The White Box Robotics Model 914 computer communicates with the M3 I/O and
motor control board over a serial-to-USB driver. The serial commands are
used to communicate with two PMD motion control chips that drive the
stepper motors and control the onboard I/O.

@par Compile-time dependencies

- none

@par Provides

The wbr914 driver provides the following device interfaces, some of
them named:

- @ref interface_position2d
  - This interface returns position data, and accepts velocity commands.
    - PLAYER_POSITION2D_CMD_VEL

- @ref interface_ir
  - This interface returns the IR range data.

- @ref interface_aio
  - This interface returns the analog input data from the optional 2nd I/O board.

- @ref interface_dio
  - This interface returns the digital input information and allows control of the digital outputs on all installed White Box Robotics I/O boards. The first I/O board supplies 8 digital inputs and outputs and the optional second I/O board supplies an additional 8 digital inputs and outputs.


@par Supported configuration requests

- @ref interface_position2d :
  - PLAYER_POSITION2D_REQ_SET_ODOM
  - PLAYER_POSITION2D_REQ_MOTOR_POWER
  - PLAYER_POSITION2D_REQ_RESET_ODOM
  - PLAYER_POSITION2D_REQ_GET_GEOM

- @ref interface_ir :
  - PLAYER_IR_REQ_POSE

@par Supported commands

- @ref interface_position2d :
  - PLAYER_POSITION2D_CMD_VEL

- @ref interface_dio :
  - PLAYER_DIO_CMD_VALUES


@par Configuration file options

- port (string)
  - Default: "/dev/ttyUSB0"

@par Example

@verbatim
driver
(
  name "wbr914"
  provides [ "position2d:0" "ir:0" "aio:0" "dio:0" ]
  port "/dev/ttyUSB0"
)

@endverbatim

@author Ian Gough, White Box Robotics
*/
/** @} */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>  /* for abs() */

#include <fcntl.h>
#include <linux/serial.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <termio.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include "wbr914.h"

#undef PLAYER_MSG0
#define PLAYER_MSG0(a,b) LogMe(b)

static void LogMe( const char* s )
{
  FILE* fp = fopen( "plog", "a+" );
  fprintf( fp, s );
  fclose( fp );
}



Driver* wbr914_Init(ConfigFile* cf, int section)
{
  return (Driver*)(new wbr914(cf,section));
}

void wbr914_Register(DriverTable* table)
{
  table->AddDriver("wbr914", wbr914_Init);
}

wbr914::wbr914(ConfigFile* cf, int section)
  : Driver(cf,section,true,PLAYER_MSGQUEUE_DEFAULT_MAXLEN),
    _tioChanged( false ),
    _stopped( true ), _motorsEnabled( false ), _lastDigOut( 0 )
{
  last_lpos = 0;
  last_rpos = 0;

  // Baud rate
  _baud = 416666;

  ErrorInit( 9 );

  // zero ids, so that we'll know later which interfaces were
  // requested
  memset(&this->position_id, 0, sizeof(player_devaddr_t));
  memset(&this->ir_id, 0, sizeof(player_devaddr_t));

  memset(&this->last_position_cmd, 0, sizeof(player_position2d_cmd_vel_t));

 this->position_subscriptions = 0;
  this->ir_subscriptions       = 0;

  // Do we create a robot position interface?
  if(cf->ReadDeviceAddr(&(this->position_id), section, "provides",
                        PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->position_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create an ir interface?
  if(cf->ReadDeviceAddr(&(this->ir_id), section, "provides",
                      PLAYER_IR_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->ir_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create an Analog I/O interface?
  if(cf->ReadDeviceAddr(&(this->aio_id), section, "provides",
                      PLAYER_AIO_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->aio_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a Digital I/O interface?
  if(cf->ReadDeviceAddr(&(this->dio_id), section, "provides",
                      PLAYER_DIO_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->dio_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }


  // Read config file options
  _serial_port = cf->ReadString(section,"port",DEFAULT_M3_PORT);
  _percentTorque = cf->ReadInt(section,"torque", DEFAULT_PERCENT_TORQUE );
  _debug = cf->ReadInt(section,"debug",0 );
  _fd = -1;

  // Constrain torque (power to motor phases) between 0 and 100.
  // Smaller numbers mean less torque, but less power used and less
  // heat generated. Not much use reducing the torque setting below
  // 20%.
  if ( _percentTorque > 100 )
  {
    _percentTorque = 100;
  }
  else if ( _percentTorque < 20 )
  {
    _percentTorque = 20;
  }

  // Set up the robot geometry

  memset(&_robot2d_geom, 0, sizeof(_robot2d_geom));
  // X location in meters
//   _robot2d_geom.pose.px     = 0.0;
  // Y location in meters
//   _robot2d_geom.pose.py     = 0.0;
  // yaw in rads
//   _robot2d_geom.pose.pyaw     = 0.0;

  // Width in meters
  _robot2d_geom.size.sw     = 0.37;
  // Length in meters
  _robot2d_geom.size.sl     = 0.42;
  _robot2d_geom.size.sh     = 0.3;

  _robot3d_geom.pose             = _robot2d_geom.pose;
  _robot3d_geom.size             = _robot2d_geom.size;

/*
  _robot3d_geom.size.sw     = _robot2d_geom.size.sw;
  _robot3d_geom.size.sl     = _robot2d_geom.size.sl;
  _robot3d_geom.size.sh     = 0.3;*/

  // Set up the IR array geometry
  _ir_geom.poses_count = NUM_IR_SENSORS;
  _ir_geom.poses = new player_pose3d_t[_ir_geom.poses_count];

  _ir_geom.poses[ 0 ].px = 0.030;
  _ir_geom.poses[ 0 ].py = -0.190;
  _ir_geom.poses[ 0 ].pyaw = DTOR( -90 );

  _ir_geom.poses[ 1 ].px = 0.190;
  _ir_geom.poses[ 1 ].py = -0.090;
  _ir_geom.poses[ 1 ].pyaw = DTOR( -30 );

  _ir_geom.poses[ 2 ].px = 0.210;
  _ir_geom.poses[ 2 ].py = 0.0;
  _ir_geom.poses[ 2 ].pyaw = DTOR( 0 );

  _ir_geom.poses[ 3 ].px = 0.190;
  _ir_geom.poses[ 3 ].py = 0.090;
  _ir_geom.poses[ 3 ].pyaw = DTOR( 30 );

  _ir_geom.poses[ 4 ].px = 0.030;
  _ir_geom.poses[ 4 ].py = 0.190;
  _ir_geom.poses[ 4 ].pyaw = DTOR( 90 );

  // These 3 sensor have a z value of 0.35m and a pitch of 30 degrees down
  // This type of pose is not currently handled by Player so do the best we
  // can with what we have.
  _ir_geom.poses[ 5 ].px = 0.200;
  _ir_geom.poses[ 5 ].py = -0.060;
  _ir_geom.poses[ 5 ].pyaw = DTOR( -60 );

  _ir_geom.poses[ 6 ].px = 0.210;
  _ir_geom.poses[ 6 ].py = 0.0;
  _ir_geom.poses[ 6 ].pyaw = DTOR( 0 );

  _ir_geom.poses[ 7 ].px = 0.200;
  _ir_geom.poses[ 7 ].py = 0.060;
  _ir_geom.poses[ 7 ].pyaw = DTOR( 60 );
  
  _data.ir.ranges_count = NUM_IR_SENSORS;
  _data.ir.voltages_count = _data.ir.ranges_count;
  _data.ir.ranges = new float [_data.ir.ranges_count];
  _data.ir.voltages = new float [_data.ir.voltages_count];
  
}

/**
   Clean up any resources
 */
wbr914::~wbr914()
{
  if ( _tioChanged )
    tcsetattr( this->_fd, TCSADRAIN, &_old_tio);
  Shutdown();
  delete [] _ir_geom.poses;
  delete [] _data.ir.ranges;
  delete [] _data.ir.voltages;
  
}

int wbr914::Setup()
{
  struct termios term;
  int flags;
  //int ltics,rtics,lvel,rvel;

  printf( "Initializing White Box Robotics Controller on %s...\n", _serial_port);
  fflush(stdout);

  PLAYER_MSG0( 0, "Starting WBR driver\n" );

  // open it.  non-blocking at first, in case there's no robot
  if((this->_fd = open(_serial_port, O_RDWR | O_NOCTTY, S_IRUSR | S_IWUSR )) < 0 )
  {
    printf("open() failed: %s\n", strerror(errno));
    return(-1);
  }

  if(tcgetattr(this->_fd, &term) < 0 )
  {
    printf("tcgetattr() failed: %s\n", strerror(errno));
    close(this->_fd);
    this->_fd = -1;
    return(-1);
  }

  tcgetattr( this->_fd, &_old_tio);

  cfmakeraw( &term );
  cfsetispeed( &term, B38400 );
  cfsetospeed( &term, B38400 );

  // 2 stop bits
  term.c_cflag |= CSTOPB | CLOCAL | CREAD;
  term.c_iflag |= IGNPAR;

  // Set timeout to .1 sec
  term.c_cc[ VTIME ] = 1;
  term.c_cc[ VMIN ]  = 0;

  if(tcsetattr(this->_fd, TCSADRAIN, &term) < 0 )
  {
    printf("tcsetattr() failed: %s\n", strerror(errno));
    close(this->_fd);
    this->_fd = -1;
    return(-1);
  }

  _tioChanged = true;

  {
    struct serial_struct serial_info;

    if ( ioctl( _fd, TIOCGSERIAL, &serial_info ) < 0)
    {
      // get the serial info
      perror("config_serial_port: ioctl TIOCGSERIAL");
      return(-1);
    }

    // Custom baud rate of 416666 baud, the max the
    // motor controller will handle.
    // round off to get the closest divisor.
    serial_info.flags = ASYNC_SPD_CUST | ASYNC_LOW_LATENCY;
    serial_info.custom_divisor = (int)((float)24000000.0/(float)_baud + 0.5);
    if ( _debug )
      printf( "Custom divisor = %d\n", serial_info.custom_divisor );

    if ( ioctl( _fd, TIOCSSERIAL, &serial_info ) < 0)
    {
      perror("config_serial_port: ioctl TIOCSSERIAL");
      return(-1);
    }
  }

  _fd_blocking = false;

  if ( _debug )
    printf( "InitRobot\n" );
  fflush(stdout);
  if(InitRobot() < 0)
  {
    printf("failed to initialize robot\n");
    close(this->_fd);
    this->_fd = -1;
    return(-1);
  }

  /* ok, we got data, so now set NONBLOCK, and continue */
  if((flags = fcntl(this->_fd, F_GETFL)) < 0)
  {
    printf("fcntl() failed: %s\n", strerror(errno));
    close(this->_fd);
    this->_fd = -1;
    return(-1);
  }

  if(fcntl(this->_fd, F_SETFL, flags ^ O_NONBLOCK) < 0)
  {
    printf("fcntl() failed: %s\n", strerror(errno));
    close(this->_fd);
    this->_fd = -1;
    return(-1);
  }
  _fd_blocking = true;

  _usCycleTime = 154;

  unsigned char ret[4];
  if( sendCmd0( LEFT_MOTOR, GETSAMPLETIME, 4,ret ) < 0)
  {
    printf("failed to get cycle time\n");
    return -1;
  }
  _usCycleTime = BytesToInt16( &(ret[2]) );

  _velocityK = (GEAR_RATIO * MOTOR_TICKS_PER_REV * _usCycleTime * 65536)/(WHEEL_CIRC * 1000000);

  SetMicrosteps();

  // PWM sign magnitude mode
  if ( (sendCmd16( LEFT_MOTOR, SETOUTPUTMODE, 1, 2, ret ) < 0 ) ||
       (sendCmd16( RIGHT_MOTOR, SETOUTPUTMODE, 1, 2, ret ) < 0 ))
  {
    printf( "Error setting sign-magnitude mode\n" );
  }
  
  /*  This might be a good time to reset the odometry values */
  if ( _debug )
    printf( "ResetRawPositions\n" );
  fflush( stdout );
  ResetRawPositions();

  if ( _debug )
    printf( "SetAccelerationProfile\n" );
  SetAccelerationProfile();
  UpdateM3();

  /* now spawn reading thread */
  if ( _debug )
    printf( "Starting Thread...\n" );
  StartThread();
  return(0);
}

int wbr914::InitRobot()
{

  // initialize the robot
  unsigned char buf[6];
  usleep( DELAY_US);

  if ( (sendCmd0( LEFT_MOTOR, RESET, 2, buf )<0 ) ||
       (sendCmd0( RIGHT_MOTOR, RESET, 2, buf )<0 ))
  {
    printf( "Error Resetting motors\n" );
  }

  if ( _debug )
    printf( "GetVersion\n" );
  if( (sendCmd0( LEFT_MOTOR, GETVERSION, 6, buf ) < 0)||
      (sendCmd0( RIGHT_MOTOR, GETVERSION, 6, buf )<0 ))
  {
    printf("Cannot get version\n");
    return -1;
  }

  _stopped = true;
  return(0);
}


int wbr914::Shutdown()
{
  if( this->_fd == -1 )
    return(0);

  // Stop any more processing
  StopThread();

  // Stop the robot
  StopRobot();

  EnableMotors( false );

  // Close the connection to the M3
  int fd = _fd;
  this->_fd = -1;
  close( fd );

  puts( "914 has been shut down" );

  return(0);
}

int wbr914::Subscribe( player_devaddr_t id )
{
  // do the subscription
  int rc = Driver::Subscribe(id);

  if( rc == 0)
  {
    // also increment the appropriate subscription counter
    if(Device::MatchDeviceAddress(id, this->position_id))
    {
      this->position_subscriptions++;
    }
    else if(Device::MatchDeviceAddress(id, this->ir_id))
    {
      this->ir_subscriptions++;
    }
    else if(Device::MatchDeviceAddress(id, this->aio_id))
    {
      this->aio_subscriptions++;
    }
    else if(Device::MatchDeviceAddress(id, this->dio_id))
    {
      this->dio_subscriptions++;
    }
  }

  return( rc );
}

int wbr914::Unsubscribe( player_devaddr_t id )
{
  int shutdownResult = Driver::Unsubscribe(id);

  // do the unsubscription
  // and decrement the appropriate subscription counter
  if( shutdownResult == 0 )
  {
    if(Device::MatchDeviceAddress(id, this->position_id))
    {
      this->position_subscriptions--;
      assert(this->position_subscriptions >= 0);
    }
    else if(Device::MatchDeviceAddress(id, this->ir_id))
    {
      this->ir_subscriptions--;
      assert(this->ir_subscriptions >= 0);
    }
    else if(Device::MatchDeviceAddress(id, this->aio_id))
    {
      this->aio_subscriptions--;
      assert(this->aio_subscriptions >= 0);
    }
    else if(Device::MatchDeviceAddress(id, this->dio_id))
    {
      this->dio_subscriptions--;
      assert(this->dio_subscriptions >= 0);
    }
  }

  return(shutdownResult);
}

void wbr914::PublishData(void)
{
  // TODO: something smarter about timestamping.

  if ( position_subscriptions )
  {
    // put odometry data
    this->Publish(this->position_id,
		  PLAYER_MSGTYPE_DATA,
		  PLAYER_POSITION2D_DATA_STATE,
		  (void*)&(this->_data.position),
		  sizeof(player_position2d_data_t),
		  NULL);
  }

  if ( ir_subscriptions )
  {
    // put ir data
    this->Publish(this->ir_id,
		  PLAYER_MSGTYPE_DATA,
		  PLAYER_IR_DATA_RANGES,
		  (void*)&(_data.ir),
		  sizeof(_data.ir),
		  NULL);
  }

  if ( aio_subscriptions )
  {
    // put Analog Input data
    this->Publish(this->aio_id,
		  PLAYER_MSGTYPE_DATA,
		  PLAYER_AIO_DATA_STATE,
		  (void*)&(_data.aio),
		  sizeof(_data.aio),
		  NULL);
  }

  if ( dio_subscriptions )
  {
    // put Digital Input data
    this->Publish(this->dio_id,
		  PLAYER_MSGTYPE_DATA,
		  PLAYER_DIO_DATA_VALUES,
		  (void*)&(_data.dio),
		  sizeof(_data.dio),
		  NULL);
  }
}

void wbr914::Main()
{
  int last_position_subscrcount=0;

  if ( _debug )
    PLAYER_MSG0( 0, "Main\n" );

  for(;;)
  {
    pthread_testcancel();

    // we want to reset the odometry and enable the motors if the first
    // client just subscribed to the position device, and we want to stop
    // and disable the motors if the last client unsubscribed.
    if(!last_position_subscrcount && this->position_subscriptions)
    {
      this->EnableMotors( false );
      this->ResetRawPositions();
    }
    else if(last_position_subscrcount && !(this->position_subscriptions))
    {
      // enable motor power
      if ( _debug )
	PLAYER_MSG0( 0, "enabling motors\n" );
      this->EnableMotors( true );
    }
    last_position_subscrcount = this->position_subscriptions;

    this->Unlock();

    // handle pending messages
    if(!this->InQueue->Empty())
    {
      if ( _debug )
	PLAYER_MSG0( 0, "processing messages\n" );
      ProcessMessages();
    }
    else
    {
      // if no pending msg, resend the last position cmd.
      // TODO: check if this is initialized
      //      this->HandlePositionCommand( this->last_position_cmd );
    }

    if ( _debug )
      PLAYER_MSG0( 0, "GetAllData\n" );

    GetAllData();

    if ( _debug )
      PLAYER_MSG0( 0, "PublishData\n" );

    PublishData();
  }
}

int wbr914::ProcessMessage(QueuePointer &resp_queue,
			   player_msghdr * hdr,
			   void * data)
{
  // Look for configuration requests
  if(hdr->type == PLAYER_MSGTYPE_REQ)
    return(this->HandleConfig(resp_queue,hdr,data));
  else if(hdr->type == PLAYER_MSGTYPE_CMD)
    return(this->HandleCommand(hdr,data));


  return(-1);
}

int wbr914::HandleConfig(QueuePointer &resp_queue,
			 player_msghdr * hdr,
			 void * data)
{
  if ( _debug )
    printf( "HandleConfig\n" );

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

    SetOdometry( (player_position2d_set_odom_req_t*)data );
    Publish( position_id, resp_queue,
	     PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_SET_ODOM);
    UpdateM3();
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_POSITION2D_REQ_MOTOR_POWER,
                                this->position_id))
  {
    /* motor state change request
     *   1 = enable motors
     *   0 = disable motors (default)
     */
    if(hdr->size != sizeof(player_position2d_power_config_t))
    {
      PLAYER_WARN("Arg to motor state change request wrong size; ignoring");
      return(-1);
    }
    player_position2d_power_config_t* power_config =
            (player_position2d_power_config_t*)data;
    this->EnableMotors( power_config->state == 1 );

    this->Publish(this->position_id, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_MOTOR_POWER);
    UpdateM3();
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_POSITION2D_REQ_RESET_ODOM,
                                this->position_id))
  {
    /* reset position to 0,0,0: no args */
    if(hdr->size != 0)
    {
      PLAYER_WARN("Arg to reset position request is wrong size; ignoring");
      return(-1);
    }
    ResetRawPositions();

    this->Publish(this->position_id, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_RESET_ODOM);
    UpdateM3();
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_POSITION2D_REQ_GET_GEOM,
                                this->position_id))
  {
    /* Return the robot geometry. */
    if(hdr->size != 0)
    {
      PLAYER_WARN("Arg get robot geom is wrong size; ignoring");
      return(-1);
    }

    Publish(this->position_id, resp_queue,
	    PLAYER_MSGTYPE_RESP_ACK,
	    PLAYER_POSITION2D_REQ_GET_GEOM,
	    (void*)&_robot2d_geom, sizeof(_robot2d_geom), NULL);
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_IR_REQ_POSE,
                                ir_id))
  {
    /* Return the ir pose info */
    Publish( ir_id, resp_queue,
	     PLAYER_MSGTYPE_RESP_ACK,
	     hdr->subtype,
	     (void*)&_ir_geom, sizeof( _ir_geom ), NULL);
    return(0);
  }
  else
  {
    PLAYER_WARN("unknown config request to wbr914 driver");
    return(-1);
  }
}

int wbr914::HandleCommand(player_msghdr * hdr, void* data)
{
  if(Message::MatchMessage(hdr,
                           PLAYER_MSGTYPE_CMD,
                           PLAYER_POSITION2D_CMD_VEL,
                           this->position_id))
  {
    /*
    struct timeval now;
    gettimeofday( &now, NULL );
    double t = now.tv_sec + now.tv_usec/1e9;
    printf( "Vel cmd at %lf\n", t );
    */
    HandleVelocityCommand( (player_position2d_cmd_vel_t*)data );
    UpdateM3();
    return(0);
  }
  else if ( Message::MatchMessage(hdr,
				  PLAYER_MSGTYPE_CMD,
				  PLAYER_DIO_CMD_VALUES,
				  this->dio_id))
  {
    HandleDigitalOutCommand( (player_dio_data_t*)data );
    return(0);
  }


  return(-1);
}

void wbr914::HandleVelocityCommand(player_position2d_cmd_vel_t* velcmd)
{
  // need to calculate the left and right velocities
  float trans = velcmd->vel.px;
  float rot = velcmd->vel.pa * DEFAULT_AXLE_LENGTH/2.0;
  float l = trans - rot;
  float r = trans + rot;

  // Reduce the speed on each wheel to the max
  // speed to prevent the stepper motors from stalling
  // Scale the speed of the other wheel to keep the
  // turn of the same rate. Note the turn geometry
  // will be affected.
  if ( fabs( l ) > MOTOR_DEF_MAX_SPEED )
  {
    float dir = l/fabs(l);
    float scale = l/MOTOR_DEF_MAX_SPEED;
    l = dir*MOTOR_DEF_MAX_SPEED;
    r = r/fabs(scale);
  }

  if ( fabs( r ) > MOTOR_DEF_MAX_SPEED )
  {
    float dir = r/fabs(r);
    float scale = r/MOTOR_DEF_MAX_SPEED;
    r = dir*MOTOR_DEF_MAX_SPEED;
    l = l/fabs(scale);
  }

  int32_t leftvel = MPS2Vel( l );
  int32_t rightvel = MPS2Vel( r );


  //  printf( "VelCmd: px=%1.3f, pa=%1.3f, trvel=%d, rotvel=%d, rvel=%d, lvel=%d\n", velcmd->vel.px, velcmd->vel.pa, transvel, rotvel, leftvel, rightvel );
  SetContourMode( VelocityContouringProfile );

  // now we set the speed
  if ( this->_motorsEnabled )
  {
    SetVelocityInTicks( leftvel, rightvel );
  }
  else
  {
    SetVelocityInTicks( 0, 0 );
    printf( "Motors not enabled\n" );
  }
}

void wbr914::HandleDigitalOutCommand( player_dio_data_t* doutCmd )
{
  SetDigitalData( doutCmd );
}

void wbr914::GetAllData( void )
{
  // Don't bother reading them if nobody is subscribed to them
  if ( position_subscriptions )
  {
    GetPositionData( &_data.position );
  }
  
  if ( ir_subscriptions )
  {
    GetIRData( &_data.ir );
  }

  if ( aio_subscriptions )
  {
    GetAnalogData( &_data.aio );
  }

  if ( dio_subscriptions )
  {
    GetDigitalData( &_data.dio );
  }
}

void wbr914::GetPositionData( player_position2d_data_t* d )
{
  // calculate position data
  int32_t left_pos = -57;
  int32_t right_pos = -57;
  const double TWOPI = 2.0*M_PI;

  GetPositionInTicks( &left_pos, &right_pos );

  int32_t change_left  = left_pos - last_lpos;
  int32_t change_right = right_pos - last_rpos;

  last_lpos = left_pos;
  last_rpos = right_pos;

  // Calculate translational and rotational change
  // translational change = avg of both changes
  // rotational change is half the diff between both changes
  double transchange = Ticks2Meters( (change_left + change_right)>>1 );
  double rotchange = Ticks2Meters( (change_left - change_right)>>1 );

  // Radius of the robotwheels
  double r = DEFAULT_AXLE_LENGTH/2.0;

  // calc total yaw, constraining from 0 to 2pi
  _yaw += rotchange/r;
  if ( _yaw < 0 )
    _yaw += TWOPI;

  if ( _yaw > TWOPI )
    _yaw -= TWOPI;

  // calc current x and y position
  _x += ( transchange * cos( _yaw )); 
  _y += ( transchange * sin( _yaw )); 


  // add code to read in the speed data
  int32_t left_vel, right_vel;

  GetVelocityInTicks( &left_vel, &right_vel );

  double lv = Vel2MPS( left_vel );
  double rv = Vel2MPS( right_vel );
  double trans_vel = (lv + rv)/2;
  double rot_vel = (lv - rv)/2;
  double rot_vel_rad = rot_vel/(r);

  d->pos.px = _x;
  d->pos.py = _y;
  d->pos.pa = _yaw;
  d->vel.px = trans_vel;
  d->vel.pa = rot_vel_rad;
}

/* this will update the IR part of the client data
 * returns:
 */
void wbr914::GetIRData(player_ir_data_t * d)
{
  // At 80cm Vmin=0.25V Vtyp=0.4V Vmax=0.55V
  // At 10cm delta increase in voltage Vmin=1.75V Vtyp=2.0V Vmax=2.25V
  // Therefore lets choose V=0.25V at 80cm and V=2.8V (2.25+0.55) at 10cm
  // Assume the formula for mm = 270 * (voltage)^-1.1 for 80cm to 10cm
  // Assume ADC input of 5.0V gives max value of 1023

  float adcLo = 0.0;
  float adcHi = 5.0;
  float vPerCount = (adcHi-adcLo)/1023.0;
  //  float v80 = 0.25;
  //  float deltaV = 2.25;
  //  float v10 = v80+deltaV;
  //  float mmPerVolt = (800.0-100.0)/(v80-v10); 

  for (uint32_t i=0; i < d->ranges_count; i++)
  {
    int16_t val = 0;

    GetAnalogSensor( i+8, &val );
    float voltage = (float)val*vPerCount;
    d->voltages[ i ] = voltage;

    // Range values are useless further out than 80-90 cm
    // with the Sharp sensors, so truncate them accordingly
    if ( val < 80 )
    {
      val = 80;
    }

    // Convert 10 bit value to a distance in meters
    float meters;

    // Formula for range conversion is different for long range
    // sensors than short range ones. Use appropriate formula.
    if ( i == 5 || i == 7 )
    {
      // Beak side firing sensors are longer range sensors
      // Sharp GP2Y0A02 sensors 20-150cm
      meters = ((16933.0/((float)val - 8.0)) - 13.0)/100.0;
    }
    else
    {
      // Sharp GP2Y0A21 sensors 10-80cm
      meters = ((6787.0/((float)val - 3.0)) - 4.0)/100.0;
    }
    d->ranges[ i ] = meters;
  }
}

/*
  Update the Analog input part of the client data

  We cannot reliably detect whether there is an I/O
  board attached to the M3 so blindly return the data.
 */
void wbr914::GetAnalogData(player_aio_data_t * d)
{
  // Read the 8 analog inputs on the second I/O board
  d->voltages_count = 8;

  float adcLo = 0.0;
  float adcHi = 5.0;
  float vPerCount = (adcHi-adcLo)/1023.0;

  for (uint32_t i=0; i < d->voltages_count; i++)
  {
    int16_t val = 0;

    GetAnalogSensor( i, &val );
    float voltage = (float)val*vPerCount;
    d->voltages[ i ] = voltage;
  }
}

/*
  Update the Digital input part of the client data

  We cannot reliably detect whether there is an I/O
  board attached to the M3 so blindly return the data.
 */
void wbr914::GetDigitalData(player_dio_data_t * d)
{
  // Read the 16 digital inputs
  uint16_t din;

  d->count = 16;
  GetDigitalIn( &din );

  // Byte flip the data to make the Input from the
  // optional I/O board show up in the upper byte.
  d->bits = (uint32_t)( (din>>8) | (din<<8));
}

/*
  Set the Digital outputs on the robot

  We cannot reliably detect whether there is an I/O
  board attached to the M3 so blindly set the data.
 */
void wbr914::SetDigitalData( player_dio_data_t * d )
{
  // We only have 16 bits of Dig out, so strip extra bits
  uint16_t data = d->bits & 0xFFFF;

  // Different number of digital bits being requested to
  // be set than we must actually set in the hardware.
  // Handle by using part of the last sent data.
  if ( d->count < 16 )
  {
    // Keep the last dig out bits that have not changed
    uint16_t mask = (0xffff << d->count);
    uint16_t oldPart = _lastDigOut & mask;

    // Invert the mask and keep the bits that are to change.
    mask = mask ^ 0xFFFF;
    data &= mask;

    // Build the output data
    data |= oldPart;
  }

  _lastDigOut = data;

  // Byte flip the data to make the Output to from the
  // optional I/O board show up in the upper byte.
  data = ( (data>>8) | (data<<8) );

  // Always set 16 bits of data
  SetDigitalOut( data );
}

//-------------------------------------------------------

// TODO: Make the bogusRC temporally stable and threadsafe
const char* wbr914::GetPMDErrorString( int rc )
{
  static const char* errorStrings[] =
  {
    "Success",
    "Reset",
    "Invalid Instruction",
    "Invalid Axis",
    "Invalid Parameter",
    "Trace Running",
    "Flash",
    "Block Out of Bounds",
    "Trace buffer zero",
    "Bad checksum",
    "Not primary port",
    "Invalid negative value",
    "Invalid parameter change",
    "Limit event pending",
    "Invalid move into limit"
  };
  static char bogusRC[ 80 ];

  if ( rc < (int)sizeof( errorStrings ) )
  {
    return errorStrings[ rc ];
  }

  sprintf( bogusRC, "Unknown error %d", rc );
  return bogusRC;
}

int wbr914::ResetRawPositions()
{
  if ( _debug )
    printf("Reset Odometry\n");
  int Values[2];
  Values[0] = 0;
  Values[1] = 0;

  if ( _debug )
    printf( "SetActualPositionInTicks\n" );
  SetActualPositionInTicks( 0, 0 );
  UpdateM3();

  last_lpos = 0;
  last_rpos = 0;
  /*
  player_position2d_data_t data;
  memset(&data,0,sizeof(player_position2d_data_t));
  Publish( position_id, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, &data, sizeof(data),NULL);
  */
  _x   = 0;
  _y   = 0;
  _yaw = 0;
  return 0;
}

bool wbr914::RecvBytes( unsigned char*s, int len )
{
  int nbytes;
  int bytesRead = 0;

  // max 10 retries
  for ( int i=0; i<10; i++ )
  {
    nbytes = read( _fd, s+bytesRead, len-bytesRead );
    if ( nbytes < 0 )
    {
      if ( errno != EAGAIN )
      {
	printf( "M3 Read error: %s\n", strerror( errno ));
	return false;
      }
      else
      {
	nbytes = 0;
      }
    }

    bytesRead += nbytes;
    if ( bytesRead == len )
    {
      return true;
    }

    // wait for the rest of the byte
    // calc time based on number of bytes left to read,
    // baud rate and num bits/byte
    int t = ( len-bytesRead )*11*1000/_baud;
    usleep( t*1000 );
  }

  printf( "Read timeout; Got %d bytes, expected %d\n",
	  bytesRead, len );

  return false;
}

int wbr914::ReadBuf(unsigned char* s, size_t len)
{
  // Get error code
  bool rc = RecvBytes( s, 1 );
  if ( !rc )
  {
    return -1;
  }

  // PMD 3410 error code in first byte
  if ( *s != 0 && *s != 1 )
  {
    const char* err = GetPMDErrorString( *s );
    printf( "Cmd error: %s\n", err );
    return( -(*s) );
  }

  // Read the rest of the response
  rc = RecvBytes( s+1, len-1 );
  if ( !rc )
  {
    return -1;
  }

  uint8_t chksum = 0;

  // Verify the checksum is correct
  for ( unsigned i=0; i<len; i++ )
  {
    chksum += *(s+i);
  }
  if ( chksum != 0 )
  {
    printf( "Read checksum error\n" );
    return -1;
  }

  return len;

#ifdef FOO
//      printf( "len=%d numread=%d\n", len, numread );
  while ( numread < (int)len )
  {
    if ( ( newread = read( this->_fd, s, len )) == -1 )
    {
      if( !_fd_blocking && errno == EAGAIN)
      {
	if ( readSlipped == false )
	  printf( "read() slipped\n" );
	else
	{
	  printf("read() failed: %s\n", strerror(errno));
	  return(-1);
	}

	readSlipped = true;

	usleep( DELAY_US);
	continue;
      }

      printf("read() failed: %s\n", strerror(errno));
      return(-1);
    }
    else
    {
      numread += newread;
      if ( _fd_blocking )
      {
	break;
      }
    }
  }

#ifdef DEBUG
  printf("read: %d of %d bytes - ", numread, len );
  ssize_t i;
  for( i=0; i<numread; i++)
  {
    printf( "0x%02x ", s[i] );
  }
  printf( "\n" );
#endif

  uint8_t chksum = 0;

  // Verify the checksum is correct
  for ( i=0; i<numread; i++ )
  {
    chksum += s[i];
  }
  if ( chksum != 0 )
  {
    printf( "Read checksum error\n" );
  }

  // PMD 3410 error code in first byte
  if ( s[0] != 0 && s[0] != 1 )
  {
    const char* err = GetPMDErrorString( s[0] );
    printf( "Cmd error: %s\n", err );
    return( -s[0] );
  }

  return( numread );
#endif
}

int wbr914::WriteBuf(unsigned char* s, size_t len)
{
  size_t numwritten;
  int thisnumwritten;
  numwritten=0;

#ifdef DEBUG
  printf("write: %d bytes - ", len );
  size_t i;
  for( i=0; i<len; i++)
  {
    printf( "0x%02x ", s[i] );
  }
  printf( "\n" );
#endif

  for ( int i=0; i<10; i++ )
  {
    thisnumwritten = write( _fd, s+numwritten, len-numwritten);
    numwritten += thisnumwritten;

    if ( numwritten == len )
    {
      return numwritten;
    }
    else if ( thisnumwritten < 0 )
    {
      printf( "Write error; %s\n", strerror( errno ));
      return -1;
    }
  }
    
  printf( "Write timeout; wrote %ld bytes, tried to write %ld\n",
	  numwritten, len );

  return numwritten;

#ifdef FOO
  while(numwritten < len)
  {
    if((thisnumwritten = write(this->_fd,s+numwritten,len-numwritten)) < 0)
    {
      if(!this->_fd_blocking && errno == EAGAIN)
      {
	printf( "write() slipped\n" );
        usleep( DELAY_US);
        continue;
      }
      printf("write() failed: %s\n", strerror(errno));
      return(-1);
    }
    numwritten += thisnumwritten;
  }


#endif

  return numwritten;
}

int wbr914::sendCmdCom( unsigned char address, unsigned char c, 
			int cmd_num, unsigned char* arg, 
			int ret_num, unsigned char * ret )
{
  assert( cmd_num<=4 );

  unsigned char cmd[8];
  //bool retry = true;
  unsigned char savecs;
  
  cmd[0] = address;
  cmd[1] = 0;          // checksum. to be overwritten
  cmd[2] = 0x00;
  cmd[3] = c;

  // Add arguments to command
  int i;
  for ( i=0; i<cmd_num; i++ )
  {
    cmd[4+i] = arg[i];
  }

  // compute checksum. Ignore cmd[1] since it is the checksum location
  int chk = 0;
  for ( i=0; i<cmd_num+4; i++ )
  {
    chk += cmd[i];
  }

  // Set the checksum
  int cs = -chk;
  cmd[1] = (unsigned char) (cs & 0xff);
  savecs = cmd[1];

  int result;

  // Flush the receive buffer. It should be empty so lets make it be so.
  tcflush( _fd, TCIFLUSH );

  result = WriteBuf( cmd, 4+cmd_num );

  if( result != 4+cmd_num )
  {
    printf( "failed to send command %d\n", (int)c );
    return( -1 );
  }


  if( ret != NULL && ret_num > 0 )
  {
    //    if ( _fd_blocking == false )
      usleep( DELAY_US );

    int rc; 
    if( (rc = ReadBuf( ret, ret_num )) < 0 )
    {
      //      printf( "failed to read response\n" );
      result = -1;
    }
  }

//      PLAYER_WARN1( "cmd: 0x%4x", *((int *) cmd) );
//      PLAYER_WARN1( "cmd: 0x%4x", *((int *) &(cmd[4])) );
  return result;
}

int wbr914::sendCmd0( unsigned char address, unsigned char c, 
		      int ret_num, unsigned char * ret )
{
  return sendCmdCom( address, c, 0, NULL, ret_num, ret );
}

int wbr914::sendCmd16( unsigned char address, unsigned char c, 
		       int16_t arg, int ret_num, unsigned char * ret )
{

  unsigned char args[2];
  uint16_t a = htons( arg );

  args[1] = (a >> 8) & 0xFF;
  args[0] = (a >> 0) & 0xFF;

  return sendCmdCom( address, c, 2, args, ret_num, ret );
}

int wbr914::sendCmd32( unsigned char address, unsigned char c, 
		       int32_t arg, int ret_num, unsigned char * ret )
{
  unsigned char args[4];
  uint32_t a = htonl( arg );
  args[3] = (a >> 24) & 0xFF;
  args[2] = (a >> 16) & 0xFF;
  args[1] = (a >> 8 ) & 0xFF;
  args[0] = (a >> 0 ) & 0xFF;

  return sendCmdCom( address, c, 4, args, ret_num, ret );
}


void wbr914::SetOdometry( player_position2d_set_odom_req_t* od )
{
  unsigned char ret[2];

  _x = od->pose.px;
  _y = od->pose.py;
  _yaw = od->pose.pa;

  int32_t leftPos = Meters2Ticks( _x );
  int32_t rightPos = Meters2Ticks( _y );

  if ( (sendCmd32( LEFT_MOTOR, SETACTUALPOS, leftPos, 2, ret )<0)||
       (sendCmd32( LEFT_MOTOR, SETACTUALPOS, rightPos, 2, ret )<0 ))
  {
    printf( "Error setting actual position\n" );
  }
}

int wbr914::GetAnalogSensor(int s, short * val )
{
  unsigned char ret[6];

  if ( sendCmd16( s / 8, READANALOG, s % 8, 4, ret )<0 )
  {
    printf( "Error reading Analog values\n" );
  }

  // Analog sensor values are 10 bit values that have been left shifted
  // 6 bits, so right-shift them back
  uint16_t v = ( (uint16_t)BytesToInt16(  &(ret[2]) ) >> 6) & 0x03ff;

  if ( _debug )
    printf( "sensor %d value: 0x%hx\n", s, v );

  *val = (uint16_t)v;

  return 0;
}


void wbr914::GetDigitalIn( uint16_t* d )
{
  unsigned char ret[6];

  if ( sendCmd16( 0, READDIGITAL, 0, 4, ret )<0)
  {
    printf( "Error reading Digital input values\n" );
  }

  *d = (uint16_t)BytesToInt16(  &(ret[2]) );
}

void wbr914::SetDigitalOut( uint16_t d )
{
  unsigned char ret[2];

  if ( sendCmd32( 0, WRITEDIGITAL, d, 2, ret ) < 0 )
  {
    printf( "Error setting Digital output values\n" );
  }
}

/*
  Robot commands
 */

void wbr914::UpdateM3()
{
  unsigned char ret[2];

  if ( (sendCmd0( LEFT_MOTOR,  UPDATE, 2, ret )<0 ) ||
       (sendCmd0( RIGHT_MOTOR, UPDATE, 2, ret )<0 ))
  {
    printf( "Error updating M3\n" );
  }
}

void wbr914::SetVelocity( uint8_t chan, float mps )
{
  uint8_t ret[2];
  int flip = 1;
  if ( chan == LEFT_MOTOR )
  {
    flip = -1;
  }
  if ( sendCmd32( chan, SETVEL, MPS2Vel( mps )*flip, 2, ret )<0 )
  {
    printf( "Error setting velocity\n" );
  }
}

void wbr914::SetVelocity( float mpsL, float mpsR )
{
  uint8_t ret[2];

  if ( (sendCmd32( LEFT_MOTOR,  SETVEL, -MPS2Vel( mpsL ), 2, ret )<0)||
       (sendCmd32( RIGHT_MOTOR, SETVEL, MPS2Vel( mpsR ), 2, ret )<0))
  {
    printf( "Error setting L/R velocity\n" );
  }
}

void wbr914::SetVelocityInTicks( int32_t left, int32_t right )
{
  uint8_t ret[2];
  if ( (sendCmd32( LEFT_MOTOR,  SETVEL, -left, 2, ret )<0)||
       (sendCmd32( RIGHT_MOTOR, SETVEL, right, 2, ret )<0))
  {
    printf( "Error setting velocity in ticks\n" );
  }
}

void wbr914::Move( uint8_t chan, float meters )
{
  uint8_t ret[6];
  int flip = 1;

  if ( chan == LEFT_MOTOR )
  {
    flip = -1;
  }

  if ( sendCmd0( chan, GETCMDPOS, 6, ret )< 0 )
  {
    printf( "Error getting actual position\n" );
  }
  int32_t loc = BytesToInt32( &ret[2] );
  loc += (flip*Meters2Ticks( meters ));
  if ( ( sendCmd32( chan, SETPOS, loc, 2, ret )<0))
  {
    printf( "Error setting actual position for Move\n" );
  }
}

void wbr914::Move( float metersL, float metersR )
{
  Move( LEFT_MOTOR,   metersL );
  Move( RIGHT_MOTOR,  metersR );
}

void wbr914::SetPosition( uint8_t chan, float meters )
{
  uint8_t ret[6];
  int flip = 1;
  if ( chan == LEFT_MOTOR )
  {
    flip = -1;
  }

  if ( sendCmd32( chan, SETPOS, flip*Meters2Ticks( meters ), 2, ret )<0)
  {
    printf( "Error issuing SetPosition\n" );
  }
}

void wbr914::SetPosition( float metersL, float metersR )
{
  SetPosition( LEFT_MOTOR,   metersL );
  SetPosition( RIGHT_MOTOR,  metersR );
}

void wbr914::SetActualPositionInTicks( int32_t left, int32_t right )
{
  uint8_t ret[6];
  if ( (sendCmd32( LEFT_MOTOR, SETACTUALPOS, -left, 2, ret )<0)||
       (sendCmd32( RIGHT_MOTOR, SETACTUALPOS, right, 2, ret )<0))
  {
    printf( "Error in SetActualPositionInTicks\n" );
  }
}

void wbr914::SetActualPosition( float metersL, float metersR )
{
  uint8_t ret[6];
  if ( (sendCmd32( LEFT_MOTOR, SETACTUALPOS, -Meters2Ticks( metersL ), 2, ret )<0)||
       (sendCmd32( RIGHT_MOTOR, SETACTUALPOS, Meters2Ticks( metersL ), 2, ret )<0))
  {
    printf( "Error in L/R SetActualPosition\n" );
  }
}

void wbr914::GetPositionInTicks( int32_t* left, int32_t* right )
{
  uint8_t ret[6];
  if ( sendCmd0( LEFT_MOTOR, GETCMDPOS, 6, ret )<0)
  {
    printf( "Error in Left GetPositionInTicks\n" );
  }
  *left = -BytesToInt32( &ret[2] );
  if ( sendCmd0( RIGHT_MOTOR, GETCMDPOS, 6, ret )<0 )
  {
    printf( "Error in Right GetPositionInTicks\n" );
  }
  *right = BytesToInt32( &ret[2] );
}

void wbr914::GetVelocityInTicks( int32_t* left, int32_t* right )
{
  uint8_t ret[6];
  if ( sendCmd0( LEFT_MOTOR, GETCMDVEL, 6, ret )<0 )
  {
    printf( "Error in Left GetVelocityInTicks\n" );
  }
  *left = -BytesToInt32( &ret[2] );
  if ( sendCmd0( RIGHT_MOTOR, GETCMDVEL, 6, ret )<0 )
  {
    printf( "Error in Left GetVelocityInTicks\n" );
  }
  *right = BytesToInt32( &ret[2] );
}

void wbr914::SetAccelerationProfile()
{
  uint8_t ret[2];
  //  int32_t accel = (int32_t)MOTOR_TICKS_PER_STEP*2;

  // Decelerate faster than accelerating.
  if ( (sendCmd32( LEFT_MOTOR,  SETACCEL, ACCELERATION_DEFAULT, 2, ret )<0)||
       (sendCmd32( RIGHT_MOTOR, SETACCEL, ACCELERATION_DEFAULT, 2, ret )<0))
  {
    printf( "Error setting Accelleration profile\n" );
  }
  if ((sendCmd32( LEFT_MOTOR,  SETDECEL, DECELERATION_DEFAULT, 2, ret )<0)||
      (sendCmd32( RIGHT_MOTOR, SETDECEL, DECELERATION_DEFAULT, 2, ret )<0))
  {
    printf( "Error setting Decelleration profile\n" );
  }
  SetContourMode( TrapezoidalProfile );
}

void wbr914::Stop( int StopMode ) {
        
  unsigned char ret[8];
  
  if ( _debug )
    printf( "Stop\n" );

  /* Start with motor 0*/
  _stopped = true;
  
  if( StopMode == FULL_STOP )
  {
    if (sendCmd16( LEFT_MOTOR, RESETEVENTSTATUS, 0x0000, 2, ret )<0 )
    {
      printf( "Error resetting event status\n" );
    }
    if ( sendCmd16( LEFT_MOTOR, SETSTOPMODE, AbruptStopMode, 2, ret )<0 )
    {
      printf( "Error setting stop mode\n" );
    }
    if ( sendCmd32( LEFT_MOTOR, SETVEL, 0, 2, ret )<0)
    {
      printf( "Error resetting motor velocity\n" );
    }
    if ( sendCmd32( LEFT_MOTOR, SETACCEL, 0, 2, ret )<0 )
    {
      printf( "Error resetting acceleration\n" );
    }
    if ( sendCmd32( LEFT_MOTOR, SETDECEL, 0, 2, ret )<0 )
    {
      printf( "Error resetting deceleration\n" );
    }


    if ( sendCmd16( RIGHT_MOTOR, RESETEVENTSTATUS, 0x0000, 2, ret )<0 )
    {
      printf( "Error resetting event status\n" );
    }
    if ( sendCmd16( RIGHT_MOTOR, SETSTOPMODE, AbruptStopMode, 2, ret )<0 )
    {
      printf( "Error setting stop mode\n" );
    }
    if ( sendCmd32( RIGHT_MOTOR, SETVEL, 0, 2, ret )<0 )
    {
      printf( "Error resetting motor velocity\n" );
    }
    if ( sendCmd32( RIGHT_MOTOR, SETACCEL, 0, 2, ret )<0 )
    {
      printf( "Error resetting acceleration\n" );
    }
    if ( sendCmd32( RIGHT_MOTOR, SETDECEL, 0, 2, ret )<0 )
    {
      printf( "Error resetting deceleration\n" );
    }

    SetContourMode( VelocityContouringProfile );

    EnableMotors( false );
    UpdateM3();

    if ((sendCmd0( LEFT_MOTOR, RESET, 2, ret )<0)||
	(sendCmd0( RIGHT_MOTOR, RESET, 2, ret )<0))
    {
      printf( "Error resetting motor\n" );
    }

  }
  else
  {
    if ( sendCmd16( LEFT_MOTOR, RESETEVENTSTATUS, 0x0700, 2, ret )<0 )
    {
      printf( "Error resetting event status\n" );
    }
    if ( sendCmd32( LEFT_MOTOR, SETVEL, 0, 2, ret )<0 )
    {
      printf( "Error resetting motor velocity\n" );
    }

    if ( sendCmd16( RIGHT_MOTOR, RESETEVENTSTATUS, 0x0700, 2, ret )<0 )
    {
      printf( "Error resetting event status\n" );
    }
    if ( sendCmd32( RIGHT_MOTOR, SETVEL, 0, 2, ret )<0)
    {
      printf( "Error resetting motor velocity\n" );
    }

    UpdateM3();

    if ((sendCmd0( LEFT_MOTOR, RESET, 2, ret )<0)||
	(sendCmd0( RIGHT_MOTOR, RESET, 2, ret )<0))
    {
      printf( "Error resetting motor\n" );
    }
  }
}


void wbr914::StopRobot()
{
  Stop( FULL_STOP );
}


bool wbr914::EnableMotors( bool enable )
{
  unsigned char ret[2];
  long torque = 0;

  if ( enable )
  {
    torque = _percentTorque*0x8000/100;
    if ( torque > 0x8000 )
      torque = 0x8000;
  }

  // Need to turn off motors to change the torque
  if ( ( sendCmd16( LEFT_MOTOR,  SETMOTORMODE, 0, 2, ret )<0)||
       ( sendCmd16( RIGHT_MOTOR, SETMOTORMODE, 0, 2, ret )<0))
  {
    printf( "Error resetting motor mode\n" );
  }

  if ((sendCmd16( LEFT_MOTOR, SETMOTORCMD, (short)torque, 2, ret )<0)||
      (sendCmd16( RIGHT_MOTOR, SETMOTORCMD, (short)torque, 2, ret )<0))
  {
    printf( "Error setting motor mode\n" );
  }

  // Update the torque setting
  UpdateM3();

  if ( enable )
  {
    if ((sendCmd16( LEFT_MOTOR,  SETMOTORMODE, 1, 2, ret )<0)||
	(sendCmd16( RIGHT_MOTOR, SETMOTORMODE, 1, 2, ret )<0))
    {
      printf( "Error setting motor mode\n" );
    }
  }

  _motorsEnabled = enable;

  return ( true );
}

void wbr914::SetContourMode( ProfileMode_t prof )
{
  uint8_t ret[2];

  if ( (sendCmd16( LEFT_MOTOR, SETPROFILEMODE, prof, 2, ret)<0)||
       (sendCmd16( RIGHT_MOTOR, SETPROFILEMODE, prof, 2, ret)<0))
  {
    printf( "Error setting profile mode\n" );
  }
}


void wbr914::SetMicrosteps()
{
  uint8_t ret[2];

  if ( (sendCmd16( LEFT_MOTOR, SETPHASECOUNTS, (short)MOTOR_TICKS_PER_STEP*4, 2, ret)<0)||
       (sendCmd16( RIGHT_MOTOR, SETPHASECOUNTS, (short)MOTOR_TICKS_PER_STEP*4, 2, ret)<0))
  {
    printf( "Error setting phase counts\n" );
  }
}


int32_t wbr914::Meters2Ticks( float meters )
{
  return (int32_t)( (double)meters * GEAR_RATIO * MOTOR_TICKS_PER_REV / WHEEL_CIRC);
}

float wbr914::Ticks2Meters( int32_t ticks )
{
  return (float)( WHEEL_CIRC*((double)ticks/GEAR_RATIO)/ MOTOR_TICKS_PER_REV );
}

int32_t wbr914::MPS2Vel( float mps )
{
  return (int32_t)( (double)mps * _velocityK );

}

float wbr914::Vel2MPS( int32_t count )
{
  return (float)( (double)count/_velocityK );

}

int32_t wbr914::BytesToInt32(unsigned char *ptr)
{
  unsigned char char0,char1,char2,char3;
  int32_t data = 0;

  char0 = ptr[3];
  char1 = ptr[2];
  char2 = ptr[1];
  char3 = ptr[0];

  data |=  ((int)char0)        & 0x000000FF;
  data |= (((int)char1) << 8)  & 0x0000FF00;
  data |= (((int)char2) << 16) & 0x00FF0000;
  data |= (((int)char3) << 24) & 0xFF000000;

  //this could just be ntohl

  return data;
}

int16_t wbr914::BytesToInt16(unsigned char *ptr)
{
  unsigned char char0,char1;
  int16_t data = 0;

  char0 = ptr[1];
  char1 = ptr[0];

  data |=  ((int)char0)        & 0x000000FF;
  data |= (((int)char1) << 8)  & 0x0000FF00;

  //this could just be ntohl

  return data;
}
