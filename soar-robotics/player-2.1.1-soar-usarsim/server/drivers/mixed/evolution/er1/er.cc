/*
*  Player - One Hell of a Robot Server
*  Copyright (C) 2000-2003
*
*		David Feil-Seifer
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
* Driver for the "ER" robots, made by Evolution Robotics.
*
* Some of this code is borrowed and/or adapted from the player
* module of trogdor; thanks to the author of that module.
*/

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_er1 er1
* @brief Evolution ER1 mobile robot

The er1 driver provides position control of the Evolution Robotics'
ER1 and ERSDK robots.

This driver is new and not thoroughly tested.  The odometry cannot be
trusted to give accurate readings.

You will need a kernel driver to allow the serial port to be seen.
This driver, and news about the player driver can be found <a
href="http://www-robotics.usc.edu/~dfseifer/project-erplayer.php">here</a>.

@todo Implement IR and power interfaces.

NOT DOING: I don't have a gripper, if someone has code for a gripper,
by all means contribute it.  It would be welcome to the mix.

@par Compile-time dependencies

- &lt;asm/ioctls.h&gt;

@par Provides

- @ref interface_position2d

@par Requires

- none

@par Supported configuration requests

- PLAYER_POSITION_GET_GEOM_REQ
- PLAYER_POSITION_MOTOR_POWER_REQ

@par Configuration file options

- port (string)
  - Default: "/dev/usb/ttyUSB0"
  - Serial port used to communicate with the robot.
- axle (length)
  - Default: 0.38 m
  - The distance between the motorized wheels
- motor_dir (integer)
  - Default: 1
  - Direction of the motors; should be 1 or -1.  If the left motor is
    plugged in to the motor 1 port on the RCM, put -1 here instead
- debug (integer)
  - Default: 0
  - Should the driver print out debug messages?

@par Example

@verbatim
driver
(
  name "er1"
  provides ["odometry:::position2d:0"]
)
@endverbatim

@author David Feil-Seifer
*/
/** @} */


#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "er.h"
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>  /* for struct sockaddr_in, htons(3) */
#include <math.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#define DEG2RAD_CONV(x) ((x)*(M_PI/180))
#include <libplayercore/playercore.h>

static float lastlvel, lastrvel;

static void StopRobot(void* erdev);

// initialization function
Driver* ER_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new ER(cf, section)));
}


// a driver registration function
void
    ER_Register(DriverTable* table)
{
  table->AddDriver("er1", ER_Init);
}

ER::ER(ConfigFile* cf, int section)
  : Driver(cf, section)
{
    // zero ids, so that we'll know later which interfaces were requested
  memset(&this->position_id, 0, sizeof(position_id));
  this->position_subscriptions = 0;

  printf( "discovering drivers\n" );

    // Do we create a position interface?
  if(cf->ReadDeviceAddr(&(this->position_id), section, "provides",
    PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->position_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

    //::initialize_robot_params();

  _fd = -1;
  _tc_num = new int[3];
  _tc_num[0] = 2;
  _tc_num[1] = 0;
  _tc_num[2] = 185;
  this->_serial_port = cf->ReadString(section, "port", ER_DEFAULT_PORT);

    //read in axle radius
  _axle_length = cf->ReadFloat( section, "axle", ER_DEFAULT_AXLE_LENGTH );

    //read in left motor and right motor direction
  int dir = cf->ReadInt(section,"motor_dir", 1);
  _motor_0_dir = dir * ER_DEFAULT_MOTOR_0_DIR;
  _motor_1_dir = dir * ER_DEFAULT_MOTOR_1_DIR;

  _debug = (1 == cf->ReadInt( section, "debug", 0 ));

  _powered = false; // don't send out constant msgs.
  _resting = false; // send out *one* 0,0 msg.
}


/////////////////////
      // bus fcns
/////////////////////
int
    ER::ReadBuf(unsigned char* s, size_t len)
{
  int thisnumread;
  int errorprinted = 0;
  size_t numread = 0;

  for (int i = 0; i < 50; i++) {
    usleep (100);
    thisnumread = read (this->_fd, s + numread, len - numread);
    if (thisnumread > 0) {
      numread += thisnumread;
      errorprinted = 0;
    } else if (thisnumread < 0 && !errorprinted) {
      perror ("read");
      errorprinted = 1;
    } else if (numread != 0) {
            // didn't read as many as we hoped. oh well...
      return numread;
    }
        // wait until we read *something*, at least
  }

  return 0;
}

int
    ER::WriteBuf(unsigned char* s, size_t len)
{
  size_t numwritten;
  int thisnumwritten;
  numwritten=0;
  while(numwritten < len)
  {
    if((thisnumwritten = write(this->_fd,s+numwritten,len-numwritten)) < 0)
    {
      if(!this->_fd_blocking && errno == EAGAIN)
      {
        usleep(ER_DELAY_US);
        continue;
      }
      printf("write() failed: %s\n", strerror(errno));
      return(-1);
    }
    numwritten += thisnumwritten;
  }

  ioctl( this->_fd, TIOCMSET, _tc_num );
  if( _tc_num[0] == 2 ) { _tc_num[0] = 0; }
  else { _tc_num[0] = 2; }

  return numwritten;
}

int
    ER::checksum_ok (unsigned char *buf, int len)
{
  unsigned char csum = 0;
  for (int i = 0; i < len; i++) {
    csum += buf[i];
  }
  return (csum == 0);
}

int
    ER::send_command( unsigned char address, unsigned char c, int ret_num, unsigned char * ret )
{
  unsigned char cmd[4];

  cmd[0] = address;
  cmd[2] = 0x00;
  cmd[3] = c;

    //compute checksum
  int chk = 0x100;
  chk -= cmd[0];
  chk -= cmd[2];
  chk -= cmd[3];

  cmd[1] = (unsigned char) chk;

  if (WriteBuf (cmd, 4) != 4) return -1;
  if (ReadBuf (ret, ret_num) <= 0) return -1;

    //	PLAYER_WARN1( "cmd: 0x%4x", *((int *) cmd) );

  return 4;
}



int
    ER::send_command_2_arg( unsigned char address, unsigned char c, int arg, int ret_num, unsigned char * ret )
{
  unsigned char cmd[6];

  cmd[0] = address;
  cmd[2] = 0x00;
  cmd[3] = c;

  int a = htons( arg );

  cmd[5] = (a >> 8) & 0xFF;
  cmd[4] = (a >> 0) & 0xFF;

    //compute checksum
  int chk = 0x100;
  chk -= cmd[0];
  chk -= cmd[2];
  chk -= cmd[3];
  chk -= cmd[4];
  chk -= cmd[5];

  cmd[1] = (unsigned char) chk;

  if (WriteBuf (cmd, 6) != 6) return -1;
  if (ReadBuf (ret, ret_num) <= 0) return -1;

    //	PLAYER_WARN1( "cmd: 0x%4x", *((int *) cmd) );
    //	PLAYER_WARN1( "cmd: 0x%4x", *((int *) &(cmd[4])) );
  return 6;
}

int
    ER::send_command_4_arg( unsigned char address, unsigned char c, int arg, int ret_num, unsigned char * ret )
{
  unsigned char cmd[8];

  cmd[0] = address;
  cmd[2] = 0x00;
  cmd[3] = c;

  int a = htonl( arg );
  cmd[7] = (a >> 24) & 0xFF;
  cmd[6] = (a >> 16) & 0xFF;
  cmd[5] = (a >> 8 ) & 0xFF;
  cmd[4] = (a >> 0 ) & 0xFF;

    //compute checksum
  int chk = 0x100;
  chk -= cmd[0];
  chk -= cmd[2];
  chk -= cmd[3];
  chk -= cmd[4];
  chk -= cmd[5];
  chk -= cmd[6];
  chk -= cmd[7];

  cmd[1] = (unsigned char) chk;

  if (WriteBuf (cmd, 8) != 8) return -1;
  if (ReadBuf (ret, ret_num) <= 0) return -1;

    //	PLAYER_WARN1( "cmd: 0x%4x", *((int *) cmd) );
    //	PLAYER_WARN1( "cmd: 0x%4x", *((int *) &(cmd[4])) );

  return 8;
}

//////////////////////////////
// robot initializations
//////////////////////////////

int
    ER::InitRobot()
{
    // initialize the robot
  unsigned char buf[6];
  usleep(ER_DELAY_US);
  if(send_command( MOTOR_0, GETVERSION, 6, buf ) < 0)
  {
    return -1;
  }

  if (!checksum_ok (buf, 6)) {
    printf ("invalid checksum\n");
    errno = ENXIO;
    return -1;
  }

  if(send_command( MOTOR_1, GETVERSION, 6, buf ) < 0)
  {
    return -1;
  }

  _tc_num[2] = 25;
  _stopped = true;
  return(0);
}

int
    ER::InitOdom()
{
  unsigned char ret[8];

    //try leaving the getVersion out
  send_command( MOTOR_0, RESET, 2, ret );
  send_command_2_arg( MOTOR_0, SETMOTORCMD, 0, 2, ret );
  send_command_2_arg( MOTOR_0, SETLIMITSWITCHMODE, 0, 2, ret );
  send_command_2_arg( MOTOR_0, SETPROFILEMODE, 0x0001, 2, ret );
  send_command_4_arg( MOTOR_0, SETVEL, 0, 2, ret );
  send_command_4_arg( MOTOR_0, SETACCEL, 0, 2, ret );
  send_command_4_arg( MOTOR_0, SETDECEL, 0, 2, ret );

    //same for motor 1
  send_command( MOTOR_1, RESET, 2, ret );
  send_command_2_arg( MOTOR_1, SETMOTORCMD, 0, 2, ret );
  send_command_2_arg( MOTOR_1, SETLIMITSWITCHMODE, 0, 2, ret );
  send_command_2_arg( MOTOR_1, SETPROFILEMODE, 0x0001, 2, ret );
  send_command_4_arg( MOTOR_1, SETVEL, 0, 2, ret );
  send_command_4_arg( MOTOR_1, SETACCEL, 0, 2, ret );
  send_command_4_arg( MOTOR_1, SETDECEL, 0, 2, ret );

    //update values
  send_command( MOTOR_0, UPDATE, 2, ret );
  send_command( MOTOR_1, UPDATE, 2, ret );

  _last_ltics = 0;
  _last_rtics = 0;

  return 0;
}

int
    ER::Setup()
{
  struct termios term;
  int flags;
    //int ltics,rtics,lvel,rvel;

  this->_px = this->_py = this->_pa = 0.0;
  this->_odom_initialized = false;

  printf("Evolution Robotics evolution_rcm connection initializing (%s)...\n", _serial_port);
  fflush(stdout);

    // open it.  non-blocking at first, in case there's no robot
  if((this->_fd = open(_serial_port, O_RDWR | O_NONBLOCK, S_IRUSR | S_IWUSR )) < 0 )
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

  cfmakeraw( &term );
  term.c_cc[VTIME] = 20;
  term.c_cc[VMIN] = 0;
  cfsetispeed(&term, B230400);
  cfsetospeed(&term, B230400);
  if(tcsetattr(this->_fd, TCSADRAIN, &term) < 0 )
  {
    printf("tcsetattr() failed: %s\n", strerror(errno));
    close(this->_fd);
    this->_fd = -1;
    return(-1);
  }

  _fd_blocking = false;
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

  flags &= ~O_NONBLOCK;
  if(fcntl(this->_fd, F_SETFL, flags) < 0)
  {
    printf("fcntl() failed: %s\n", strerror(errno));
    close(this->_fd);
    this->_fd = -1;
    return(-1);
  }
  _fd_blocking = true;

  /*  This might be a good time to reset the odometry values */
  if( InitOdom() < 0 ) {
    printf("InitOdom failed\n" );
    close(this->_fd);
    this->_fd = -1;
    return -1;
  }

    // start the thread to talk with the robot
  this->StartThread();

  return(0);
}

void InvokeMain (void *arg)
{
  ER *er = (ER *)arg;
  er->Main();
}

int
    ER::Shutdown()
{
  if(this->_fd == -1)
    return(0);

  StopThread();

    // the robot is stopped by the thread cleanup function StopRobot(), which
    // is called as a result of the above StopThread()

  if(SetVelocity(0,0) < 0)
    printf("failed to stop robot while shutting down\n");


  usleep(ER_DELAY_US);

  if(close(this->_fd))
    printf("close() failed:%s\n",strerror(errno));
  this->_fd = -1;
  if( _debug )
    printf("ER has been shutdown\n\n");

  return(0);
}

void
    ER::Stop( int StopMode )
{
  unsigned char ret[8];

  printf( "Stop\n" );
  /* Start with motor 0*/
  _stopped = true;
  if( StopMode == FULL_STOP )
  {
    /* motor 0 */
    send_command_2_arg( MOTOR_0, RESETEVENTSTATUS, 0x0000, 2, ret );
    send_command_2_arg( MOTOR_0, SETMOTORCMD, 0x0000, 2, ret );
    send_command_2_arg( MOTOR_0, SETPROFILEMODE, 0x0001, 2, ret );
    send_command_2_arg( MOTOR_0, SETSTOPMODE, 0x0001, 2, ret );
    send_command_4_arg( MOTOR_0, SETVEL, 0, 2, ret );
    send_command_4_arg( MOTOR_0, SETACCEL, 0, 2, ret );
    send_command_4_arg( MOTOR_0, SETDECEL, 0, 2, ret );
    send_command( MOTOR_0, UPDATE, 2, ret );
    send_command( MOTOR_0, RESET, 2, ret );

    /* motor 1 */
    send_command_2_arg( MOTOR_1, RESETEVENTSTATUS, 0x0000, 2, ret );
    send_command_2_arg( MOTOR_1, SETMOTORCMD, 0x0000, 2, ret );
    send_command_2_arg( MOTOR_1, SETPROFILEMODE, 0x0001, 2, ret );
    send_command_2_arg( MOTOR_1, SETSTOPMODE, 0x0001, 2, ret );
    send_command_4_arg( MOTOR_1, SETVEL, 0, 2, ret );
    send_command_4_arg( MOTOR_1, SETACCEL, 0, 2, ret );
    send_command_4_arg( MOTOR_1, SETDECEL, 0, 2, ret );
    send_command( MOTOR_1, UPDATE, 2, ret );
    send_command( MOTOR_1, RESET, 2, ret );
  } else {
    /* motor 0 */
    send_command_2_arg( MOTOR_0, RESETEVENTSTATUS, 0x0700, 2, ret );
    send_command_4_arg( MOTOR_0, SETVEL, 0, 2, ret );
    send_command( MOTOR_0, UPDATE, 2, ret );
    send_command( MOTOR_0, RESET, 2, ret );

    /* motor 1 */
    send_command_2_arg( MOTOR_1, RESETEVENTSTATUS, 0x0700, 2, ret );
    send_command_4_arg( MOTOR_1, SETVEL, 0, 2, ret );
    send_command( MOTOR_1, UPDATE, 2, ret );
    send_command( MOTOR_1, RESET, 2, ret );
  }
}

////////////////////
// periodic fcns
////////////////////

//============================
// function GetOdom
//   - Retrieves tick counts for both wheels
//============================
int ER::GetOdom(int *ltics, int *rtics)
{
    // Variable declaration
  unsigned char ret[6];

    // Determine number of ticks for motor 0 (left)
  send_command( MOTOR_0, GETCMDPOS, 6, ret );
  *ltics = _motor_0_dir * BytesToInt32(&(ret[2]));

    // Determine number of ticks for motor 1 (right)
  send_command( MOTOR_1, GETCMDPOS, 6, ret );
  *rtics = _motor_1_dir * BytesToInt32(&(ret[2]));

    // Output debug (motor tick count) data?
  if (_debug)
  {
    printf("lticks: %d, %2x %2x %2x %2x %2x %2x\n",
          *ltics, ret[0], ret[1], ret[2], ret[3], ret[4], ret[5]);
    printf("rticks: %d, %2x %2x %2x %2x %2x %2x\n",
          *rtics, ret[0], ret[1], ret[2], ret[3], ret[4], ret[5]);
  }

    // Read motor 0 again since first response seems to be crap
    //   - adding a delay (~75000usec) between reads doesn't help
    //   - alternating reads (L,R,R,L or R,L,L,R) causes problems
    //   - don't really need to read the other motor, but for completeness

    // Determine number of ticks for motor 0 (left)
  send_command( MOTOR_0, GETCMDPOS, 6, ret );
  *ltics = _motor_0_dir * BytesToInt32(&(ret[2]));

    // Determine number of ticks for motor 1 (right)
  send_command( MOTOR_1, GETCMDPOS, 6, ret );
  *rtics = _motor_1_dir * BytesToInt32(&(ret[2]));

    // Output debug (motor tick count) data?
  if (_debug)
  {
    printf("lticks: %d, %2x %2x %2x %2x %2x %2x\n",
          *ltics, ret[0], ret[1], ret[2], ret[3], ret[4], ret[5]);
    printf("rticks: %d, %2x %2x %2x %2x %2x %2x\n",
          *rtics, ret[0], ret[1], ret[2], ret[3], ret[4], ret[5]);
  }

    // Output debug (motor tick count) data?
  if( _debug )
  {
    printf("ltics: %d rtics: %d\n", *ltics, *rtics);
  }

  return(0);
}

int
    ER::GetBatteryVoltage(int* voltage)
{
  unsigned char ret[4];

  send_command_2_arg( MOTOR_1, READANALOG, 0x0001, 6, ret );

  if( _debug )
    printf( "voltage?: %f\n", (float) BytesToFloat(&(ret[2])) );

    //yeah and do something with this voltage???

  return(0);
}

int
    ER::GetRangeSensor(int s, float * val )
{
  unsigned char ret[6];

  send_command_2_arg( s / 8, READANALOG, s % 8, 6, ret );

  /* this is definately wrong, need to fix this */
  float range = (float) BytesToFloat( &(ret[2]) );

  if( _debug )
    printf( "sensor value?: %d\n", s );

  val = &range;

  return 0;
}


//============================
//
//============================
int
    ER::SetVelocity(double lvel, double rvel)
{
  lastlvel = lvel;
  lastrvel = rvel;

  if (_debug)
    printf("Setting velocity to %f %f\n",lvel,rvel);

  if (lvel != 0 || rvel != 0) {
    this->_powered = true;
  }
  this->_resting = false;

  /* fixed the no-stop bug via hack (ES,6/30) */
  static bool stopped = false;
  if ( (lvel == 0) && (rvel == 0) )
  {
        // asked to stop moving... are we already stopped?
    if (stopped)
    {
            // already stopped, do not listen to this velocity zero command
      return 0;
    }
        // haven't stopped, we will this time
    stopped = true;
  } else {
        // not stopping
    stopped = false;
  }
  /* hack off */

    // resume normal function!

  unsigned char ret[8];
  if( _debug )
    printf( "lvel: %f rvel: %f\n", lvel, rvel );

    // Motor 0

  send_command( MOTOR_0, GETEVENTSTATUS, 4, ret );

  if( _stopped )
  {
    send_command_2_arg( MOTOR_0, RESETEVENTSTATUS, 0x0300, 2, ret );
    send_command_2_arg( MOTOR_0, SETMOTORCMD, 0x6590, 2, ret );
    send_command_2_arg( MOTOR_0, SETPROFILEMODE, 0x0001, 2, ret );
  } else {
    send_command_2_arg( MOTOR_0, RESETEVENTSTATUS, 0x0700, 2, ret );
  }

  SpeedCommand( MOTOR_0, lvel, _motor_0_dir );

  send_command_4_arg( MOTOR_0, SETACCEL, 0x0000007E, 2, ret );

    // Motor 1

  send_command( MOTOR_1, GETEVENTSTATUS, 4, ret );

  if( _stopped )
  {
    send_command_2_arg( MOTOR_1, RESETEVENTSTATUS, 0x0300, 2, ret );
    send_command_2_arg( MOTOR_1, SETMOTORCMD, 0x6590, 2, ret );
    send_command_2_arg( MOTOR_1, SETPROFILEMODE, 0x0001, 2, ret );
  } else {
    send_command_2_arg( MOTOR_1, RESETEVENTSTATUS, 0x0700, 2, ret );
  }

  SpeedCommand( MOTOR_1, rvel, _motor_1_dir );
  send_command_4_arg( MOTOR_1, SETACCEL, 0x0000007E, 2, ret );

    // and UPDATE both
  send_command( MOTOR_0, UPDATE, 2, ret );
  send_command( MOTOR_1, UPDATE, 2, ret );

  _stopped = false;
  return 0;
}


void
    ER::Main()
{
  player_position2d_data_t data;
  int rtics, ltics;
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

    // push a pthread cleanup function that stops the robot
  pthread_cleanup_push(StopRobot,this);

  for(;;)
  {
    pthread_testcancel();
    ProcessMessages();

    /* get the odometry values */
    GetOdom (&ltics, &rtics);
    UpdateOdom (ltics, rtics);

        // Current position (odometry)
    data.pos.px = this->_px;
    data.pos.py = this->_py;
    data.pos.pa = this->_pa;

        // Current velocity
    data.vel.px = (lastlvel + lastrvel) / 2.0;
    data.vel.py = 0;
    data.vel.pa = (lastrvel - lastlvel) / _axle_length;

    data.stall = _stopped;

    if (this->_powered || !this->_resting) {
      Publish(position_id, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, &data, sizeof(data), NULL);
      if (!this->_powered) this->_resting = true;
    }

    usleep (50000);

  } /* for */
  pthread_cleanup_pop(1);
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int ER::ProcessMessage(QueuePointer & resp_queue,
                      player_msghdr * hdr,
                      void * data)
{
  assert(hdr);
  assert(data);

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_GET_GEOM, position_id))
  {
    player_position2d_geom_t geom;

        //TODO : get values from somewhere.
    geom.pose.px = -0.1;//htons((short) (-100));
    geom.pose.py = 0;//htons((short) (0));
    geom.pose.pyaw = 0;//htons((short) (0));
    geom.size.sw = 0.25;//htons((short) (250));
    geom.size.sl = 0.425;//htons((short) (425));
    Publish(position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_GET_GEOM, &geom, sizeof(geom),NULL);

    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_MOTOR_POWER, position_id))
  {
    player_position2d_power_config_t * powercfg = reinterpret_cast<player_position2d_power_config_t *> (data);

    printf("got motor power req: %d\n", powercfg->state);
    if(ChangeMotorState(powercfg->state) < 0)
      Publish(position_id, resp_queue, PLAYER_MSGTYPE_RESP_NACK, PLAYER_POSITION2D_REQ_MOTOR_POWER);
    else
      Publish(position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_MOTOR_POWER);
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_SET_ODOM, position_id))
  {
    player_position2d_set_odom_req_t * odom = (player_position2d_set_odom_req_t *)data;
    this->_px = odom->pose.px;
    this->_py = odom->pose.py;
    this->_pa = odom->pose.pa;
    while (this->_pa > M_PI) this->_pa -= M_PI;
    while (this->_pa < -M_PI) this->_pa += M_PI;
    this->_resting = false;
    Publish (position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_SET_ODOM);
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_RESET_ODOM, position_id))
  {
    this->_px = 0.0;
    this->_py = 0.0;
    this->_pa = 0.0;
    this->_resting = false;
    Publish (position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_RESET_ODOM);
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, position_id) ||
      Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_CAR, position_id))
  {
    double velocity, angle;
    if (hdr->subtype == PLAYER_POSITION2D_CMD_VEL) {
      player_position2d_cmd_vel_t *position_cmd = (player_position2d_cmd_vel_t *)data;
      velocity = position_cmd->vel.px;
      angle = position_cmd->vel.pa;
    } else {
      player_position2d_cmd_car_t *carlike_cmd = (player_position2d_cmd_car_t *)data;
      velocity = carlike_cmd->velocity;
      angle = carlike_cmd->angle;
    }

    double rotational_term;
    double command_rvel, command_lvel;
    double final_lvel = 0, final_rvel = 0;
    double last_final_lvel = 0, last_final_rvel = 0;

        // convert (tv,rv) to (lv,rv) and send to robot
    rotational_term = angle * _axle_length / 2.0;
    command_rvel = velocity + rotational_term;
    command_lvel = velocity - rotational_term;

        // sanity check on per-wheel speeds
    if (fabs (command_lvel) > ER_MAX_WHEELSPEED) {
      command_rvel = command_rvel * ER_MAX_WHEELSPEED / fabs (command_lvel);
      command_lvel = command_lvel * ER_MAX_WHEELSPEED / fabs (command_lvel);
    }
    if (fabs (command_rvel) > ER_MAX_WHEELSPEED) {
      command_lvel = command_lvel * ER_MAX_WHEELSPEED / fabs (command_rvel);
      command_rvel = command_rvel * ER_MAX_WHEELSPEED / fabs (command_rvel);
    }

    final_lvel = command_lvel;
    final_rvel = command_rvel;

    if( _debug )
      printf( "final_lvel: %f, final_rvel: %f\n", final_lvel, final_rvel );

    if ((final_lvel != last_final_lvel) || (final_rvel != last_final_rvel)) {
      if (final_lvel * last_final_lvel < 0 || final_rvel * last_final_rvel < 0) {
                //				PLAYER_WARN( "Changing motor direction, soft stop\n" );
        SetVelocity(0,0);
      }
      if (SetVelocity (final_lvel, final_rvel) < 0) {
        printf ("failed to set velocity\n");
        pthread_exit (NULL);
      }
      if (velocity == 0 && angle == 0)
      {
        if (_debug) printf ("STOP\n");
        Stop (STOP);
      }

      last_final_lvel = final_lvel;
      last_final_rvel = final_rvel;
      MotorSpeed();
    }

        // do a hack to stop the vehicle if the speeds are zero (ES, 6/30)
    if( (velocity == 0) && (angle == 0) )
    {
            //			printf( "implicit stop\n" );
      SetVelocity(0,0);
    }

    return 0;
  }

  return -1;
}



////////////////////
// util fcns
////////////////////

void
    ER::MotorSpeed()
{
  unsigned char ret[8];

  send_command( MOTOR_0, GETEVENTSTATUS, 4, ret );
  send_command_2_arg( MOTOR_0, RESETEVENTSTATUS, 0x0700, 2, ret );
  send_command_4_arg( MOTOR_0, SETACCEL, 0x0000007A, 2, ret );
  send_command( MOTOR_1, GETEVENTSTATUS, 4, ret );
  send_command_2_arg( MOTOR_1, RESETEVENTSTATUS, 0x0700, 2, ret );
  send_command_4_arg( MOTOR_1, SETACCEL, 0x0000007A, 2, ret );

  send_command( MOTOR_0, UPDATE, 2, ret );
  send_command( MOTOR_1, UPDATE, 2, ret );
}

void
    ER::SpeedCommand( unsigned char address, double speed, int dir ) {

  unsigned char ret[2];

  int whole = dir * (int) (speed / ER_M_PER_TICK);

  send_command_4_arg( address, SETVEL, whole, 2, ret );

    //	printf( "speed: %f checksum: 0x%02x value: 0x%08x\n", speed, cmd[1], whole );
    }

    int
        ER::BytesToInt32(unsigned char *ptr)
    {
      unsigned char char0,char1,char2,char3;
      int data = 0;

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

    float
        ER::BytesToFloat(unsigned char *ptr)
    {
      float res;
      int intermediate = BytesToInt32 (ptr);
      memcpy(&res,&intermediate,sizeof(res));
      return res;
    }

    int
        ER::ComputeTickDiff(int from, int to)
    {
      int diff1, diff2;

    // find difference in two directions and pick shortest
      if(to > from)
      {
        diff1 = to - from;
        diff2 = (-ER_MAX_TICKS - from) + (to - ER_MAX_TICKS);
      }
      else
      {
        diff1 = to - from;
        diff2 = (from - ER_MAX_TICKS) + (-ER_MAX_TICKS - to);
      }

      if(abs(diff1) < abs(diff2))
        return(diff1);
      else
        return(diff2);

      return 0;
    }

    void
        ER::UpdateOdom(int ltics, int rtics)
    {
      int ltics_delta, rtics_delta;
      double l_delta, r_delta, a_delta, d_delta;
      int max_tics;
      static struct timeval lasttime;
      struct timeval currtime;
      double timediff;

      if(!this->_odom_initialized)
      {
        this->_last_ltics = ltics;
        this->_last_rtics = rtics;
        gettimeofday(&lasttime,NULL);
        this->_odom_initialized = true;
        return;
      }

    //  ltics_delta = ComputeTickDiff(this->_last_ltics,ltics);
    //  rtics_delta = ComputeTickDiff(this->_last_rtics,rtics);
      ltics_delta = ltics - this->_last_ltics;
      rtics_delta = rtics - this->_last_rtics;

      gettimeofday(&currtime,NULL);
      timediff = (currtime.tv_sec + currtime.tv_usec/1e6) - (lasttime.tv_sec + lasttime.tv_usec/1e6);
      max_tics = (int)rint(ER_MAX_WHEELSPEED / ER_M_PER_TICK / timediff);
      lasttime = currtime;

      if( _debug ) {
        printf("ltics: %d\trtics: %d\n", ltics,rtics);
        printf("ldelt: %d\trdelt: %d\n", ltics_delta, rtics_delta);
      }

      l_delta = ltics_delta * ER_M_PER_TICK;
      r_delta = rtics_delta * ER_M_PER_TICK;

      a_delta = (r_delta - l_delta) / _axle_length;
      d_delta = (l_delta + r_delta) / 2.0;

      this->_px += d_delta * cos(this->_pa + (a_delta / 2));
      this->_py += d_delta * sin(this->_pa + (a_delta / 2));
      this->_pa += a_delta;
      while (this->_pa > M_PI) this->_pa -= 2*M_PI;
      while (this->_pa < -M_PI) this->_pa += 2*M_PI;

      if( _debug ) {
        printf("er: pose: %f,%f,%f\n", this->_px,this->_py,this->_pa);
      }
      this->_last_ltics = ltics;
      this->_last_rtics = rtics;
    }


    int
        ER::ChangeMotorState(int state)
    {
      if (!state) {
        SetVelocity (0, 0);
        this->_powered = false;
      } else {
        this->_powered = true;
      }
      return 0;
    }

    static void
        StopRobot(void* erdev)
    {
      ER* er = (ER*)erdev;

      er->Stop (FULL_STOP);
    }

