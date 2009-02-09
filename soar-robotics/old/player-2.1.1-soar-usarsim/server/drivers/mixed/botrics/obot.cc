/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003
 *     Brian Gerkey
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
 * $Id: obot.cc 4135 2007-08-23 19:58:48Z gerkey $
 *
 *
 * Some of this code is borrowed and/or adapted from the 'cerebellum'
 * module of CARMEN; thanks to the authors of that module.
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_obot obot
 * @brief Botrics Obot mobile robot

The obot driver controls the Obot robot, made by Botrics.  It's a
small, very fast robot that can carry a SICK laser (talk to the laser
over a normal serial port using the @ref driver_sicklms200 driver).

@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d
- @ref interface_power

@par Requires

- none

@par Supported commands

- PLAYER_POSITION2D_CMD_VEL
- PLAYER_POSITION2D_CMD_CAR

@par Supported configuration requests

- PLAYER_POSITION2D_REQ_GET_GEOM
- PLAYER_POSITION2D_REQ_SET_ODOM
- PLAYER_POSITION2D_REQ_RESET_ODOM

@par Configuration file options

- offset (length tuple)
  - Default: [0.0 0.0 0.0]
  - Offset of the robot's center of rotation

- size (length tuple)
  - Default: [0.45 0.45]
  - Bounding box (length, width) of the robot

- port (string)
  - Default: "/dev/usb/ttyUSB1"
  - Serial port used to communicate with the robot.

- max_speed (length, angle tuple)
  - Default: [0.5 40.0]
  - Maximum (translational, rotational) velocities

- max_accel (integer)
  - Default: 5
  - Maximum acceleration/deceleration (units?)

- motors_swapped (integer)
  - Default: 0
  - If non-zero, then assume that the motors and encoders connections
    are swapped.

- car_angle_deadzone (angle)
  - Default: 5.0 degrees
  - Minimum angular error required to induce servoing when in car-like
    command mode.

- car_angle_p (float)
  - Default: 1.0
  - Value to be multiplied by angular error (in rad) to produce angular 
    velocity command (in rad/sec) when in car-like command mode

- watchdog_timeout (float, seconds)
  - Default: 1.0
  - How long since receiving the last command before the robot is stopped,
    for safety.  Set to -1.0 for no watchdog (DANGEROUS!).
  
@par Example 

@verbatim
driver
(
  name "obot"
  provides ["position2d:0"]
)
@endverbatim

@author Brian Gerkey
*/
/** @} */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <replace/replace.h>
#include <libplayercore/playercore.h>
#include "obot_constants.h"

static void StopRobot(void* obotdev);

class Obot : public Driver 
{
  private:
    // this function will be run in a separate thread
    virtual void Main();
    
    // bookkeeping
    bool fd_blocking;
    double px, py, pa;  // integrated odometric pose (m,m,rad)
    int last_ltics, last_rtics;
    bool odom_initialized;
    player_devaddr_t position_addr;
    player_devaddr_t power_addr;
    double max_xspeed, max_yawspeed;
    bool motors_swapped;
    int max_accel;

    // Minimum angular error required to induce servoing when in car-like
    // command mode.
    double car_angle_deadzone;
    // Value to be multiplied by angular error (in rad) to produce angular 
    // velocity command (in rad/sec) when in car-like command mode
    double car_angle_p;

    // How long since receiving the last command before we stop the robot, 
    // for safety.
    double watchdog_timeout;

    // Robot geometry (size and rotational offset)
    player_bbox3d_t robot_size;
    player_pose3d_t robot_pose;

    // methods for internal use
    int WriteBuf(unsigned char* s, size_t len);
    int ReadBuf(unsigned char* s, size_t len);
    int BytesToInt32(unsigned char *ptr);
    void Int32ToBytes(unsigned char* buf, int i);
    int ValidateChecksum(unsigned char *ptr, size_t len);
    int GetOdom(int *ltics, int *rtics, int *lvel, int *rvel);
    void UpdateOdom(int ltics, int rtics);
    unsigned char ComputeChecksum(unsigned char *ptr, size_t len);
    int SendCommand(unsigned char cmd, int val1, int val2);
    int ComputeTickDiff(int from, int to);
    int ChangeMotorState(int state);
    int OpenTerm();
    int InitRobot();
    int GetBatteryVoltage(int* voltage);
    double angle_diff(double a, double b);

    player_position2d_cmd_car_t last_car_cmd;
    int last_final_lvel, last_final_rvel;
    double last_cmd_time;
    bool sent_new_command;
    bool car_command_mode;

  public:
    int fd; // device file descriptor
    const char* serial_port; // name of dev file

    // public, so that it can be called from pthread cleanup function
    int SetVelocity(int lvel, int rvel);

    Obot(ConfigFile* cf, int section);

    void ProcessCommand(player_position2d_cmd_vel_t * cmd);
    void ProcessCarCommand(player_position2d_cmd_car_t * cmd);

    // Process incoming messages from clients 
    int ProcessMessage(QueuePointer & resp_queue, 
                       player_msghdr * hdr, 
                       void * data);

    virtual int Setup();
    virtual int Shutdown();
};


// initialization function
Driver* Obot_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new Obot( cf, section)));
}

// a driver registration function
void 
Obot_Register(DriverTable* table)
{
  table->AddDriver("obot",  Obot_Init);
}

Obot::Obot( ConfigFile* cf, int section) 
  : Driver(cf,section,true,PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
  memset(&this->position_addr,0,sizeof(player_devaddr_t));
  memset(&this->power_addr,0,sizeof(player_devaddr_t));

  // Do we create a robot position interface?
  if(cf->ReadDeviceAddr(&(this->position_addr), section, "provides",
                        PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->position_addr) != 0)
    {
      this->SetError(-1);    
      return;
    }

    this->robot_size.sl = cf->ReadTupleLength(section, "size", 
                                              0, OBOT_LENGTH);
    this->robot_size.sw = cf->ReadTupleLength(section, "size", 
                                              1, OBOT_WIDTH);
    this->robot_pose.px = cf->ReadTupleLength(section, "offset",
                                              0, OBOT_POSE_X);
    this->robot_pose.py = cf->ReadTupleLength(section, "offset",
                                              1, OBOT_POSE_Y);
    this->robot_pose.pyaw = cf->ReadTupleAngle(section, "offset",
                                             2, OBOT_POSE_A);

    this->max_xspeed = cf->ReadTupleLength(section, "max_speed",
                                           0, 0.5);
    this->max_yawspeed = cf->ReadTupleAngle(section, "max_speed",
                                            1, DTOR(40.0));
    this->motors_swapped = cf->ReadInt(section, "motors_swapped", 0);
    this->max_accel = cf->ReadInt(section, "max_accel", 5);

    this->car_angle_deadzone = cf->ReadAngle(section, "car_angle_deadzone",
                                             DTOR(5.0));
    this->car_angle_p = cf->ReadFloat(section, "car_angle_p", 1.0);
    this->watchdog_timeout = cf->ReadFloat(section, "watchdog_timeout", 1.0);
  }

  // Do we create a power interface?
  if(cf->ReadDeviceAddr(&(this->power_addr), section, "provides", 
                      PLAYER_POWER_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->power_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  this->fd = -1;
  this->serial_port = cf->ReadString(section, "port", OBOT_DEFAULT_PORT);
}

int
Obot::InitRobot()
{
  // initialize the robot
  unsigned char initstr[3];
  initstr[0] = OBOT_INIT1;
  initstr[1] = OBOT_INIT2;
  initstr[2] = OBOT_INIT3;
  unsigned char deinitstr[1];
  deinitstr[0] = OBOT_DEINIT;

  if(tcflush(this->fd, TCIOFLUSH) < 0 )
  {
    PLAYER_ERROR1("tcflush() failed: %s", strerror(errno));
    close(this->fd);
    this->fd = -1;
    return(-1);
  }

  if(WriteBuf(initstr,sizeof(initstr)) < 0)
  {
    PLAYER_WARN("failed to initialize robot; i'll try to de-initializate it");
    if(WriteBuf(deinitstr,sizeof(deinitstr)) < 0)
    {
      PLAYER_ERROR("failed on write of de-initialization string");
      return(-1);
    }
    if(WriteBuf(initstr,sizeof(initstr)) < 0)
    {
      PLAYER_ERROR("failed on 2nd write of initialization string; giving up");
      return(-1);
    }
  }
  return(0);
}

int
Obot::OpenTerm()
{
  struct termios term;
  
  // open it.  non-blocking at first, in case there's no robot
  if((this->fd = open(serial_port, O_RDWR | O_SYNC | O_NONBLOCK, S_IRUSR | S_IWUSR )) < 0 )
  {
    PLAYER_ERROR1("open() failed: %s", strerror(errno));
    return(-1);
  }  
 
  if(tcgetattr(this->fd, &term) < 0 )
  {
    PLAYER_ERROR1("tcgetattr() failed: %s", strerror(errno));
    close(this->fd);
    this->fd = -1;
    return(-1);
  }
  
  cfmakeraw(&term);
  cfsetispeed(&term, B57600);
  cfsetospeed(&term, B57600);
  
  if(tcsetattr(this->fd, TCSAFLUSH, &term) < 0 )
  {
    PLAYER_ERROR1("tcsetattr() failed: %s", strerror(errno));
    close(this->fd);
    this->fd = -1;
    return(-1);
  }

  fd_blocking = false;
  return(0);
}

int 
Obot::Setup()
{
  int flags;
  int ltics,rtics,lvel,rvel;

  this->px = this->py = this->pa = 0.0;
  this->odom_initialized = false;
  this->last_final_rvel = this->last_final_lvel = 0;
  this->last_cmd_time = -1.0;
  this->sent_new_command = false;
  this->car_command_mode = false;

  printf("Botrics Obot connection initializing (%s)...", serial_port);
  fflush(stdout);

  if(OpenTerm() < 0)
  {
    PLAYER_ERROR("failed to initialize robot");
    return(-1);
  }

  if(InitRobot() < 0)
  {
    PLAYER_ERROR("failed to initialize robot");
    close(this->fd);
    this->fd = -1;
    return(-1);
  }

  /* try to get current odometry, just to make sure we actually have a robot */
  if(GetOdom(&ltics,&rtics,&lvel,&rvel) < 0)
  {
    PLAYER_ERROR("failed to get odometry");
    close(this->fd);
    this->fd = -1;
    return(-1);
  }

  UpdateOdom(ltics,rtics);

  /* ok, we got data, so now set NONBLOCK, and continue */
  if((flags = fcntl(this->fd, F_GETFL)) < 0)
  {
    PLAYER_ERROR1("fcntl() failed: %s", strerror(errno));
    close(this->fd);
    this->fd = -1;
    return(-1);
  }
  if(fcntl(this->fd, F_SETFL, flags ^ O_NONBLOCK) < 0)
  {
    PLAYER_ERROR1("fcntl() failed: %s", strerror(errno));
    close(this->fd);
    this->fd = -1;
    return(-1);
  }
  fd_blocking = true;
  puts("Done.");

  // TODO: what are reasoanable numbers here?
  if(SendCommand(OBOT_SET_ACCELERATIONS,this->max_accel,this->max_accel) < 0)
  {
    PLAYER_ERROR("failed to set accelerations on setup");
    close(this->fd);
    this->fd = -1;
    return(-1);
  }

  // start the thread to talk with the robot
  StartThread();

  return(0);
}

int
Obot::Shutdown()
{
  unsigned char deinitstr[1];

  if(this->fd == -1)
    return(0);

  StopThread();

  usleep(OBOT_DELAY_US);

  deinitstr[0] = OBOT_DEINIT;
  if(WriteBuf(deinitstr,sizeof(deinitstr)) < 0)
    PLAYER_ERROR("failed to deinitialize connection to robot");

  if(close(this->fd))
    PLAYER_ERROR1("close() failed:%s",strerror(errno));
  this->fd = -1;
  puts("Botrics Obot has been shutdown");
  return(0);
}

void 
Obot::Main()
{
  player_position2d_data_t data;
  player_power_data_t charge_data;
  double lvel_mps, rvel_mps;
  int lvel, rvel;
  int ltics, rtics;
  double last_publish_time = 0.0;
  double t;
  bool stopped=false;

  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

  // push a pthread cleanup function that stops the robot
  pthread_cleanup_push(StopRobot,this);

  for(;;)
  {
    pthread_testcancel();
    
    this->sent_new_command = false;
    ProcessMessages();
    if(!this->sent_new_command)
    {
      // Have we received a command lately?
      GlobalTime->GetTimeDouble(&t);
      if((this->last_cmd_time > 0.0) &&  (this->watchdog_timeout > 0.0) &&
         ((t - this->last_cmd_time) >= this->watchdog_timeout))
      {
        if(!stopped)
        {
          PLAYER_WARN("Watchdog timer stopping robot");
          stopped = true;
        }
        if(this->SetVelocity(0,0) < 0)
          PLAYER_ERROR("failed to set velocity");
      }
      else
      {
        stopped = false;
        // Which mode are we in?
        if(this->car_command_mode)
        {
          // Car-like command mode.  Re-compute angular vel based on target
          // heading
          this->ProcessCarCommand(&this->last_car_cmd);
        }
        else
        {
          // Direct velocity command mode.  Re-send last set of velocities.
          if(this->SetVelocity(this->last_final_lvel, this->last_final_rvel) < 0)
            PLAYER_ERROR("failed to set velocity");
        }
      }
    }
    
    // Update and publish odometry info
    if(this->GetOdom(&ltics,&rtics,&lvel,&rvel) < 0)
    {
      PLAYER_ERROR("failed to get odometry");
      //pthread_exit(NULL);
    }
    else
      this->UpdateOdom(ltics,rtics);

    // Update and publish power info
    int volt;
    if(GetBatteryVoltage(&volt) < 0)
      PLAYER_WARN("failed to get voltage");
    
    GlobalTime->GetTimeDouble(&t);
    if((t - last_publish_time) > OBOT_PUBLISH_INTERVAL)
    {
      data.pos.px = this->px;
      data.pos.py = this->py;
      data.pos.pa = this->pa;

      data.vel.py = 0;
      lvel_mps = lvel * OBOT_MPS_PER_TICK;
      rvel_mps = rvel * OBOT_MPS_PER_TICK;
      data.vel.px = (lvel_mps + rvel_mps) / 2.0;
      data.vel.pa = (rvel_mps-lvel_mps) / OBOT_AXLE_LENGTH;
      data.stall = 0;
  
      //printf("publishing: %.3f %.3f %.3f\n",
             //data.pos.px,
             //data.pos.py,
             //RTOD(data.pos.pa));
      this->Publish(this->position_addr,  
                    PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, 
                    (void*)&data,sizeof(data),NULL);
      
      charge_data.valid = PLAYER_POWER_MASK_VOLTS | PLAYER_POWER_MASK_PERCENT;
      charge_data.volts = ((float)volt) / 1e1;
      charge_data.percent = 1e2 * (charge_data.volts / 
                                   OBOT_NOMINAL_VOLTAGE);
      this->Publish(this->power_addr,  
                    PLAYER_MSGTYPE_DATA,
                    PLAYER_POWER_DATA_STATE,
                    (void*)&charge_data, sizeof(player_power_data_t), NULL);
  
      last_publish_time = t;
    }

    //usleep(OBOT_DELAY_US);
  }
  pthread_cleanup_pop(0);
}

// Process car-like command, which sets an angular position target and
// translational velocity target.  The basic idea is to compute angular
// velocity so as to servo (with P-control) to target angle.  Then pass the
// two velocities to ProcessCommand() for thresholding and unit conversion.
void 
Obot::ProcessCarCommand(player_position2d_cmd_car_t * cmd)
{
  // Cache this command for later reuse
  this->last_car_cmd = *cmd;

  // Build up a cmd_vel structure to pass to ProcessCommand()
  player_position2d_cmd_vel_t vel_cmd;
  memset(&vel_cmd,0,sizeof(vel_cmd));

  // Pass through trans vel unmodified
  vel_cmd.vel.px = cmd->velocity;

  // Compute rot vel
  double da = this->angle_diff(cmd->angle, this->pa);
  if(fabs(da) < DTOR(this->car_angle_deadzone))
    vel_cmd.vel.pa = 0.0;
  else
    vel_cmd.vel.pa = this->car_angle_p * da;

  this->ProcessCommand(&vel_cmd);
}

void
Obot::ProcessCommand(player_position2d_cmd_vel_t * cmd)
{
  double rotational_term, command_lvel, command_rvel;
  int final_lvel, final_rvel;
  double xspeed, yawspeed;

  xspeed = cmd->vel.px;
  yawspeed = cmd->vel.pa;

  // Clamp velocities according to given maxima
  // TODO: test this to see if it does the right thing.  We could clamp
  //       individual wheel velocities instead.
  if(fabs(xspeed) > this->max_xspeed)
  {
    if(xspeed > 0)
      xspeed = this->max_xspeed;
    else
      xspeed = -this->max_xspeed;
  }
  if(fabs(yawspeed) > this->max_yawspeed)
  {
    if(yawspeed > 0)
      yawspeed = this->max_yawspeed;
    else
      yawspeed = -this->max_yawspeed;
  }

  // convert (tv,rv) to (lv,rv) and send to robot
  rotational_term = yawspeed * OBOT_AXLE_LENGTH / 2.0;
  command_rvel = xspeed + rotational_term;
  command_lvel = xspeed - rotational_term;

  // sanity check on per-wheel speeds
  if(fabs(command_lvel) > OBOT_MAX_WHEELSPEED)
  {
    if(command_lvel > 0)
    {
      command_lvel = OBOT_MAX_WHEELSPEED;
      command_rvel *= OBOT_MAX_WHEELSPEED/command_lvel;
    }
    else
    {
      command_lvel = - OBOT_MAX_WHEELSPEED;
      command_rvel *= -OBOT_MAX_WHEELSPEED/command_lvel;
    }
  }
  if(fabs(command_rvel) > OBOT_MAX_WHEELSPEED)
  {
    if(command_rvel > 0)
    {
      command_rvel = OBOT_MAX_WHEELSPEED;
      command_lvel *= OBOT_MAX_WHEELSPEED/command_rvel;
    }
    else
    {
      command_rvel = - OBOT_MAX_WHEELSPEED;
      command_lvel *= -OBOT_MAX_WHEELSPEED/command_rvel;
    }
  }

  final_lvel = (int)rint(command_lvel / OBOT_MPS_PER_TICK);
  final_rvel = (int)rint(command_rvel / OBOT_MPS_PER_TICK);

  // TODO: do this min threshold smarter, to preserve desired travel 
  // direction

  /* to account for our bad low-level PID motor controller */
  if(abs(final_rvel) > 0 && abs(final_rvel) < OBOT_MIN_WHEELSPEED_TICKS)
  {
    if(final_rvel > 0)
      final_rvel = OBOT_MIN_WHEELSPEED_TICKS;
    else
      final_rvel = -OBOT_MIN_WHEELSPEED_TICKS;
  }
  if(abs(final_lvel) > 0 && abs(final_lvel) < OBOT_MIN_WHEELSPEED_TICKS)
  {
    if(final_lvel > 0)
      final_lvel = OBOT_MIN_WHEELSPEED_TICKS;
    else
      final_lvel = -OBOT_MIN_WHEELSPEED_TICKS;
  }

  // Record that we got a command at this time
  GlobalTime->GetTimeDouble(&(this->last_cmd_time));

  if((final_lvel != last_final_lvel) ||
     (final_rvel != last_final_rvel))
  {
    if(SetVelocity(final_lvel,final_rvel) < 0)
    {
      PLAYER_ERROR("failed to set velocity");
      pthread_exit(NULL);
    }
    last_final_lvel = final_lvel;
    last_final_rvel = final_rvel;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int Obot::ProcessMessage(QueuePointer & resp_queue, 
                         player_msghdr * hdr, 
                         void * data)
{
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_POSITION2D_CMD_VEL, 
                           this->position_addr))
  {
    // Only take the first new command (should probably take the last,
    // but...)
    if(!this->sent_new_command)
    {
      assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
      this->ProcessCommand((player_position2d_cmd_vel_t*)data);
      this->sent_new_command = true;
      this->car_command_mode = false;
    }
    return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                                PLAYER_POSITION2D_CMD_CAR, 
                                this->position_addr))
  {
    // Only take the first new command (should probably take the last,
    // but...)
    if(!this->sent_new_command)
    {
      assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
      this->ProcessCarCommand((player_position2d_cmd_car_t*)data);
      this->sent_new_command = true;
      this->car_command_mode = true;
    }
    return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_POSITION2D_REQ_GET_GEOM, 
                                this->position_addr))
  {
    player_position2d_geom_t  geom;
  	
    geom.pose = this->robot_pose;
    geom.size = this->robot_size;

    this->Publish(this->position_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_POSITION2D_REQ_GET_GEOM,
                  (void*)&geom, sizeof(geom), NULL);
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_POSITION2D_REQ_MOTOR_POWER, 
                                this->position_addr))
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
    this->ChangeMotorState(power_config->state);

    this->Publish(this->position_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK, 
                  PLAYER_POSITION2D_REQ_MOTOR_POWER);
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_POSITION2D_REQ_SET_ODOM,
                                this->position_addr))
  {
    if(hdr->size != sizeof(player_position2d_set_odom_req_t))
    {
      PLAYER_WARN("Arg to odometry set requests wrong size; ignoring");
      return(-1);
    }
    player_position2d_set_odom_req_t* set_odom_req =
            (player_position2d_set_odom_req_t*)data;

    // Just overwrite our current odometric pose.
    this->px = set_odom_req->pose.px;
    this->py = set_odom_req->pose.py;
    this->pa = set_odom_req->pose.pa;

    this->Publish(this->position_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_SET_ODOM);
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_POSITION2D_REQ_RESET_ODOM,
                                this->position_addr))
  {
    if(hdr->size != sizeof(player_null_t))
    {
      PLAYER_WARN("Arg to odometry reset requests wrong size; ignoring");
      return(-1);
    }

    // Just overwrite our current odometric pose.
    this->px = 0.0;
    this->py = 0.0;
    this->pa = 0.0;

    this->Publish(this->position_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_RESET_ODOM);
    return(0);
  }
  else
    return -1;
}

int
Obot::ReadBuf(unsigned char* s, size_t len)
{
  int thisnumread;
  size_t numread = 0;
  int loop;
  int maxloops=10;

  loop=0;
  while(numread < len)
  {
    //printf("loop %d of %d\n", loop,maxloops);
    // apparently the underlying PIC gets overwhelmed if we read too fast
    // wait...how can that be?
    if((thisnumread = read(this->fd,s+numread,len-numread)) < 0)
    {
      if(!this->fd_blocking && errno == EAGAIN && ++loop < maxloops)
      {
        usleep(OBOT_DELAY_US);
        continue;
      }
      PLAYER_ERROR1("read() failed: %s", strerror(errno));
      return(-1);
    }
    if(thisnumread == 0)
      PLAYER_WARN("short read");
    numread += thisnumread;
  }
  /*
  printf("read: ");
  for(size_t i=0;i<numread;i++)
    printf("%d ", s[i]);
  puts("");
  */
  return(0);
}

int
Obot::WriteBuf(unsigned char* s, size_t len)
{
  size_t numwritten;
  int thisnumwritten;
  unsigned char ack[1];

  /*
  static double last = 0.0;
  double t;
  GlobalTime->GetTimeDouble(&t);
  printf("WriteBuf: %d bytes (time since last: %f)\n", 
         len, t-last);
  last=t;
  */

  for(;;)
  {
    numwritten=0;
    while(numwritten < len)
    {
      if((thisnumwritten = write(this->fd,s+numwritten,len-numwritten)) < 0)
      {
        if(!this->fd_blocking && errno == EAGAIN)
        {
          usleep(OBOT_DELAY_US);
          continue;
        }
        PLAYER_ERROR1("write() failed: %s", strerror(errno));
        return(-1);
      }
      numwritten += thisnumwritten;
    }

    // get acknowledgement
    if(ReadBuf(ack,1) < 0)
    {
      PLAYER_ERROR("failed to get acknowledgement");
      return(-1);
    }

    // TODO: re-init robot on NACK, to deal with underlying cerebellum reset 
    // problem
    switch(ack[0])
    {
      case OBOT_ACK:
        usleep(OBOT_DELAY_US);
        return(0);
      case OBOT_NACK:
        PLAYER_WARN("got NACK; reinitializing connection");
        usleep(OBOT_DELAY_US);
        if(close(this->fd) < 0)
          PLAYER_WARN1("close failed: %s", strerror(errno));
        if(OpenTerm() < 0)
        {
          PLAYER_ERROR("failed to re-open connection");
          return(-1);
        }
        if(InitRobot() < 0)
        {
          PLAYER_ERROR("failed to reinitialize");
          return(-1);
        }
        else
        {
          usleep(OBOT_DELAY_US);
          return(0);
        }
        break;
      default:
        PLAYER_WARN1("got unknown value for acknowledgement: %d",ack[0]);
        usleep(OBOT_DELAY_US);
        return(-1);
    }
  }
}

int 
Obot::BytesToInt32(unsigned char *ptr)
{
  unsigned char char0,char1,char2,char3;
  int data = 0;

  char0 = ptr[0];
  char1 = ptr[1];
  char2 = ptr[2];
  char3 = ptr[3];

  data |=  ((int)char0)        & 0x000000FF;
  data |= (((int)char1) << 8)  & 0x0000FF00;
  data |= (((int)char2) << 16) & 0x00FF0000;
  data |= (((int)char3) << 24) & 0xFF000000;

  return data;
}

int
Obot::GetBatteryVoltage(int* voltage)
{
  unsigned char buf[5];
  buf[0] = OBOT_GET_VOLTAGE;

  if(WriteBuf(buf,1) < 0)
  {
    PLAYER_ERROR("failed to send battery voltage command");
    return(-1);
  }

  if(ReadBuf(buf,5) < 0)
  {
    PLAYER_ERROR("failed to read battery voltage");
    return(-1);
  }

  if(ValidateChecksum(buf,5) < 0)
  {
    PLAYER_ERROR("checksum failed on battery voltage");
    return(-1);
  }

  *voltage = BytesToInt32(buf);
  return(0);
}

void
Obot::Int32ToBytes(unsigned char* buf, int i)
{
  buf[0] = (i >> 0)  & 0xFF;
  buf[1] = (i >> 8)  & 0xFF;
  buf[2] = (i >> 16) & 0xFF;
  buf[3] = (i >> 24) & 0xFF;
}

int
Obot::GetOdom(int *ltics, int *rtics, int *lvel, int *rvel)
{
  unsigned char buf[20];
  int index;

  buf[0] = OBOT_GET_ODOM;
  if(WriteBuf(buf,1) < 0)
  {
    PLAYER_ERROR("failed to send command to retrieve odometry");
    return(-1);
  }
  //usleep(OBOT_DELAY_US);
  
  // read 4 int32's, 1 error byte, and 1 checksum
  if(ReadBuf(buf, 18) < 0)
  {
    PLAYER_ERROR("failed to read odometry");
    return(-1);
  }

  if(ValidateChecksum(buf, 18) < 0)
  {
    PLAYER_ERROR("checksum failed on odometry packet");
    return(-1);
  }
  if(buf[16] == 1)
  {
    PLAYER_ERROR("Cerebellum error with encoder board");
    return(-1);
  }

  index = 0;
  *ltics = BytesToInt32(buf+index);
  index += 4;
  *rtics = BytesToInt32(buf+index);
  index += 4;
  *rvel = BytesToInt32(buf+index);
  index += 4;
  *lvel = BytesToInt32(buf+index);

  //printf("ltics: %d rtics: %d\n", *ltics, *rtics);

  //puts("got good odom packet");

  return(0);
}

int 
Obot::ComputeTickDiff(int from, int to) 
{
  int diff1, diff2;

  // find difference in two directions and pick shortest
  if(to > from) 
  {
    diff1 = to - from;
    diff2 = (-OBOT_MAX_TICS - from) + (to - OBOT_MAX_TICS);
  }
  else 
  {
    diff1 = to - from;
    diff2 = (from - OBOT_MAX_TICS) + (-OBOT_MAX_TICS - to);
  }

  if(abs(diff1) < abs(diff2)) 
    return(diff1);
  else
    return(diff2);
}

void
Obot::UpdateOdom(int ltics, int rtics)
{
  int ltics_delta, rtics_delta;
  double l_delta, r_delta, a_delta, d_delta;
  int max_tics;
  static struct timeval lasttime;
  struct timeval currtime;
  double timediff;

  if(this->motors_swapped)
  {
    int tmp = ltics;
    ltics = rtics;
    rtics = tmp;
  }

  if(!this->odom_initialized)
  {
    this->last_ltics = ltics;
    this->last_rtics = rtics;
    gettimeofday(&lasttime,NULL);
    this->odom_initialized = true;
    return;
  }
  
  // MAJOR HACK! 
  // The problem comes from one or the other encoder returning 0 ticks (always
  // the left, I think), we'll just throw out those readings.  Shouldn't have
  // too much impact.
  if(!ltics || !rtics)
  {
    PLAYER_WARN("Invalid odometry reading (zeros); ignoring");
    return;
  }

  //ltics_delta = ComputeTickDiff(last_ltics,ltics);
  //rtics_delta = ComputeTickDiff(last_rtics,rtics);
  ltics_delta = ltics - this->last_ltics;
  rtics_delta = rtics - this->last_rtics;

  // mysterious rollover code borrowed from CARMEN
/*
  if(ltics_delta > SHRT_MAX/2)
    ltics_delta += SHRT_MIN;
  if(ltics_delta < -SHRT_MIN/2)
    ltics_delta -= SHRT_MIN;
  if(rtics_delta > SHRT_MAX/2)
    rtics_delta += SHRT_MIN;
  if(rtics_delta < -SHRT_MIN/2)
    rtics_delta -= SHRT_MIN;
*/

  gettimeofday(&currtime,NULL);
  timediff = (currtime.tv_sec + currtime.tv_usec/1e6)-
             (lasttime.tv_sec + lasttime.tv_usec/1e6);
  max_tics = (int)rint(OBOT_MAX_WHEELSPEED / OBOT_M_PER_TICK / timediff);
  lasttime = currtime;

  //printf("ltics: %d\trtics: %d\n", ltics,rtics);
  //printf("ldelt: %d\trdelt: %d\n", ltics_delta, rtics_delta);
  //printf("maxtics: %d\n", max_tics);

  if(abs(ltics_delta) > max_tics || abs(rtics_delta) > max_tics)
  {
    PLAYER_WARN("Invalid odometry change (too big); ignoring");
    return;
  }

  l_delta = ltics_delta * OBOT_M_PER_TICK;
  r_delta = rtics_delta * OBOT_M_PER_TICK;

  //printf("Left speed: %f\n", l_delta / timediff);
  //printf("Right speed: %f\n", r_delta / timediff);


  a_delta = (r_delta - l_delta) / OBOT_AXLE_LENGTH;
  d_delta = (l_delta + r_delta) / 2.0;


  this->px += d_delta * cos(this->pa);
  this->py += d_delta * sin(this->pa);
  this->pa += a_delta;
  this->pa = NORMALIZE(this->pa);
  
  //printf("obot: pose: %f,%f,%f\n", this->px,this->py, RTOD(this->pa));

  this->last_ltics = ltics;
  this->last_rtics = rtics;
}

// Validate XOR checksum
int
Obot::ValidateChecksum(unsigned char *ptr, size_t len)
{
  size_t i;
  unsigned char checksum = 0;

  for(i = 0; i < len-1; i++)
    checksum ^= ptr[i];

  if(checksum == ptr[len-1])
    return(0);
  else
    return(-1);
}

// Compute XOR checksum
unsigned char
Obot::ComputeChecksum(unsigned char *ptr, size_t len)
{
  size_t i;
  unsigned char chksum = 0;

  for(i = 0; i < len; i++)
    chksum ^= ptr[i];

  return(chksum);
}

int
Obot::SendCommand(unsigned char cmd, int val1, int val2)
{
  unsigned char buf[10];
  int i;

  //printf("SendCommand: %d %d %d\n", cmd, val1, val2);
  i=0;
  buf[i] = cmd;
  i+=1;
  Int32ToBytes(buf+i,val1);
  i+=4;
  Int32ToBytes(buf+i,val2);
  i+=4;
  buf[i] = ComputeChecksum(buf,i);

  if(WriteBuf(buf,10) < 0)
  {
    PLAYER_ERROR("failed to send command");
    return(-1);
  }

  return(0);
}

int
Obot::SetVelocity(int lvel, int rvel)
{
  int retval;
  
  //printf("SetVelocity: %d %d\n", lvel, rvel);

  if(!this->motors_swapped)
    retval = SendCommand(OBOT_SET_VELOCITIES,lvel,rvel);
  else
    retval = SendCommand(OBOT_SET_VELOCITIES,rvel,lvel);
  if(retval < 0)
  {
    PLAYER_ERROR("failed to set velocities");
    return(-1);
  }

  return(0);
}

int 
Obot::ChangeMotorState(int state)
{
  unsigned char buf[1];
  if(state)
    buf[0] = OBOT_ENABLE_VEL_CONTROL;
  else
    buf[0] = OBOT_DISABLE_VEL_CONTROL;
  return(WriteBuf(buf,sizeof(buf)));
}

static void
StopRobot(void* obotdev)
{
  Obot* td = (Obot*)obotdev;

  tcflush(td->fd,TCIOFLUSH);
  if(td->SetVelocity(0,0) < 0)
    PLAYER_ERROR("failed to stop robot on thread exit");
}

// computes the signed minimum difference between the two angles.
double
Obot::angle_diff(double a, double b)
{
  double d1, d2;
  a = NORMALIZE(a);
  b = NORMALIZE(b);
  d1 = a-b;
  d2 = 2*M_PI - fabs(d1);
  if(d1 > 0)
    d2 *= -1.0;
  if(fabs(d1) < fabs(d2))
    return(d1);
  else
    return(d2);
}
