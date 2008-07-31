/** @ingroup drivers */
/** @{ */
/** @defgroup driver_nd nd
 * @brief Nearness Diagram Navigation

This driver implements the Nearness Diagram Navigation algorithm.
This algorithm handles local collision-avoidance and goal-seeking and is
designed for non-holonomic, non-circular robots operating in tight spaces.
The algorithm is in the following papers:

- J. Minguez, L. Montano. Nearness Diagram Navigation (ND): Collision
Avoidance in Troublesome Scenarios. IEEE Transactions on Robotics and
Automation, pp 154, 2004.
<a href="http://webdiis.unizar.es/~jminguez/TRAND.pdf">PDF</a>

- J. Minguez, J. Osuna, L. Montano.  A Divide and Conquer Strategy based
on Situations to Achieve Reactive Collision Avoidance in Troublesome
Scenarios.  IEEE International Conference on Robotics and Automation
(ICRA 2004), 2004. New Orleans, USA.
<a href="http://webdiis.unizar.es/~jminguez/810.pdf">PDF</a>

This driver reads pose information from a @ref interface_position2d
device, sensor data from a @ref interface_laser device and/or @ref
interface_sonar device, and writes commands to a @ref interface_position2d
device.  The two @ref interface_position2d devices can be the same.
At least one device of type @ref interface_laser or @ref interface_sonar must
be provided.

The driver itself supports the @interface_position2d interface.  Send
@ref PLAYER_POSITION2D_CMD_POS commands to set the goal pose.  The driver
also accepts @ref PLAYER_POSITION2D_CMD_VEL commands, simply passing them
through to the underlying output device.


@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d

@par Requires

- "input" @ref interface_position2d : source of pose and velocity information
- "output" @ref interface_position2d : sink for velocity commands to control the robot
- @ref interface_laser : the laser to read from
- @ref interface_sonar : the sonar to read from

@par Configuration requests

- all @ref interface_position2d requests are passed through to the
underlying "output" @ref interface_position2d device.

@par Configuration file options

- goal_tol (tuple: [length angle])
  - Default: [0.5 10.0] (m deg)
  - Respectively, translational and rotational goal tolerance.  When the
    robot is within these bounds of the current target pose, it will
    be stopped.

- max_speed (tuple: [length/sec angle/sec])
  - Default: [0.3 45.0] (m/s deg/s)
  - Respectively, maximum absolute translational and rotational velocities
    to be used in commanding the robot.

- min_speed (tuple: [length/sec angle/sec])
  - Default: [0.05 10.0] (m/s deg/s)
  - Respectively, minimum absolute non-zero translational and rotational
    velocities to be used in commanding the robot.

- avoid_dist (length)
  - Default: 0.5 m
  - Distance at which obstacle avoidance begins

- safety_dist (length)
  - Default: 0.0 m
  - Extra extent added to the robot on all sides when doing obstacle
    avoidance.

- rotate_stuck_time (float)
  - Default: 2.0 seconds
  - How long the robot is allowed to rotate in place without making any
    progress toward the goal orientation before giving up.

- translate_stuck_time (float)
  - Default: 2.0 seconds         
  - How long the robot is allowed to translate without making sufficient
    progress toward the goal position before giving up.

- translate_stuck_dist (length)
  - Default: 0.25 m
  - How far the robot must translate during translate stuck time in
    order to not give up.

- translate_stuck_angle (angle)
  - Default: 20 deg
  - How far the robot must rotate during translate stuck time in order
    to not give up.

- wait_on_stall (integer)
  - Default: 0
  - Should local navigation be paused if the stall flag is set on the
    input:::position2d device? This option is useful if the robot's pose
    is being read from a SLAM system that sets the stall flag when it
    is performing intensive computation and can no longer guarantee the
    validity of pose estimates.

- laser_buffer (integer)
  - Default: 10          
  - How many recent laser scans to consider in the local navigation.


- sonar_buffer (integer)
  - Default: 10
  - How many recent sonar scans to consider in the local navigation

- sonar_bad_transducers (tuple [integers])
  - Default: [] (empty tuple)
  - Indices of sonar transducers that should be ignored. This option is
    useful when particular sonars are known to be bad, or if they tend
    to give returns from the robot's body (usually the wheels), or if they
    are just not needed because of overlap with the laser.

@par Example

@verbatim
driver
(
  name "nd"
  provides ["position2d:1"]
  requires ["output:::position2d:0" "input:::position2d:0" "laser:0" "sonar:0"]

  max_speed [0.3 30.0]
  min_speed [0.1 10.0]
  goal_tol [0.3 15.0]
  wait_on_stall 1

  rotate_stuck_time 5.0
  translate_stuck_time 5.0
  translate_stuck_dist 0.15
  translate_stuck_angle 10.0

  avoid_dist 0.4
  safety_dist 0.0

  laser_buffer 1
  sonar_buffer 1
)
@endverbatim

@author Javier Minguez (underlying algorithm), Brian Gerkey (driver integration)

*/
/** @} */
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <float.h>

#include <libplayercore/playercore.h>
#include "nd.h"

#ifndef SIGN
  #define SIGN(x) (((x) == 0) ? 0 : (((x) > 0) ? 1 : -1))
#endif

extern PlayerTime *GlobalTime;

class ND : public Driver 
{
  public:
    // Constructor
    ND( ConfigFile* cf, int section);

    // Destructor
    virtual ~ND();

    // Setup/shutdown routines.
    virtual int Setup();
    virtual int Shutdown();

    // Process incoming messages from clients 
    virtual int ProcessMessage(QueuePointer &resp_queue, 
                               player_msghdr * hdr, 
                               void * data);
    // Main function for device thread.
    virtual void Main();

  private:
    bool active_goal;
    int dir;
    player_pose2d_t goal;
    player_pose2d_t last_odom_pose;
    player_pose2d_t odom_pose;
    player_pose2d_t odom_vel;
    bool odom_stall;
    int current_dir;
    TParametersND NDparametros;

    double rotate_start_time;
    double rotate_min_error;
    double rotate_stuck_time;

    double translate_start_time;
    double translate_min_error;
    double translate_stuck_time;
    double translate_stuck_dist;
    double translate_stuck_angle;
    bool wait_on_stall;
    bool waiting;

    bool stall;
    bool turning_in_place;

    TInfoEntorno obstacles;

    TInfoEntorno* laser_obstacles;
    int num_laser_scans;

    TInfoEntorno* sonar_obstacles;
    int num_sonar_scans;

    double vx_max, va_max;
    double vx_min, va_min;
    double avoid_dist;
    player_position2d_geom_t robot_geom;
    double safety_dist;
    
    double Threshold(double v, double v_min, double v_max);
    void SetDirection(int dir);
    int SetupOdom();
    int ShutdownOdom();
    int SetupLaser();
    int ShutdownLaser();
    int SetupSonar();
    int ShutdownSonar();

    void ProcessOutputOdom(player_msghdr_t* hdr, player_position2d_data_t* data);
    void ProcessInputOdom(player_msghdr_t* hdr, player_position2d_data_t* data);
    void ProcessLaser(player_msghdr_t* hdr, player_laser_data_t* data);
    void ProcessSonar(player_msghdr_t* hdr, player_sonar_data_t* data);
    void ProcessCommand(player_msghdr_t* hdr, player_position2d_cmd_vel_t* cmd);
    void ProcessCommand(player_msghdr_t* hdr, player_position2d_cmd_pos_t* cmd);
    // Send a command to the motors
    void PutPositionCmd(double vx, double va);

    // Computes the signed minimum difference between the two angles.
    double angle_diff(double a, double b);
    
    // Odometry device info
    Device *odom;
    player_devaddr_t odom_addr;
    Device *localize;
    player_devaddr_t localize_addr;
    double dist_eps;
    double ang_eps;

    // Laser device info
    Device *laser;
    player_devaddr_t laser_addr;
    player_pose3d_t laser_pose;
    int laser_buffer;

    // Sonar device info
    Device *sonar;
    player_devaddr_t sonar_addr;
    int num_sonars;
    player_pose3d_t * sonar_poses;
    // indices of known bad sonars
    int * bad_sonars;
    int bad_sonar_count;
    int sonar_buffer;
};

// Initialization function
Driver* 
ND_Init(ConfigFile* cf, int section) 
{
  return ((Driver*) (new ND( cf, section)));
} 

// a driver registration function
void ND_Register(DriverTable* table)
{
  table->AddDriver("nd",  ND_Init);
  return;
} 

////////////////////////////////////////////////////////////////////////////////
// Constructor
ND::ND( ConfigFile* cf, int section)
  : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_POSITION2D_CODE)
{
  this->dist_eps = cf->ReadTupleLength(section, "goal_tol", 0, 0.5);
  this->ang_eps = cf->ReadTupleAngle(section, "goal_tol", 1, DTOR(10.0));

  this->vx_max = cf->ReadTupleLength(section, "max_speed", 0, 0.3);
  this->va_max = cf->ReadTupleAngle(section, "max_speed", 1, DTOR(45.0));
  this->vx_min = cf->ReadTupleLength(section, "min_speed", 0, 0.05);
  this->va_min = cf->ReadTupleAngle(section, "min_speed", 1, DTOR(10.0));

  this->avoid_dist = cf->ReadLength(section, "avoid_dist", 0.5);
  this->safety_dist = cf->ReadLength(section, "safety_dist", 0.0);
  this->rotate_stuck_time = cf->ReadFloat(section, "rotate_stuck_time", 2.0);

  this->translate_stuck_time = cf->ReadFloat(section, "translate_stuck_time", 
                                             2.0);
  this->translate_stuck_dist = cf->ReadLength(section, "translate_stuck_dist", 
                                              0.25);
  this->translate_stuck_angle = cf->ReadAngle(section, "translate_stuck_angle", 
                                               DTOR(20.0));
  this->wait_on_stall = 
          cf->ReadInt(section, "wait_on_stall", 0) ?  true : false;

  this->odom = NULL;
  if (cf->ReadDeviceAddr(&this->odom_addr, section, "requires",
                         PLAYER_POSITION2D_CODE, -1, "output") != 0)
  {
    this->SetError(-1);    
    return;
  }

  this->localize = NULL;
  if (cf->ReadDeviceAddr(&this->localize_addr, section, "requires",
                         PLAYER_POSITION2D_CODE, -1, "input") != 0)
  {
    this->SetError(-1);    
    return;
  }

  this->laser = NULL;
  memset(&this->laser_addr,0,sizeof(player_devaddr_t));
  cf->ReadDeviceAddr(&this->laser_addr, section, "requires",
                     PLAYER_LASER_CODE, -1, NULL);
  if(this->laser_addr.interf)
  {
    this->laser_buffer = cf->ReadInt(section, "laser_buffer", 10);
  }

  this->sonar = NULL;
  memset(&this->sonar_addr,0,sizeof(player_devaddr_t));
  cf->ReadDeviceAddr(&this->sonar_addr, section, "requires",
                     PLAYER_SONAR_CODE, -1, NULL);
  if(this->sonar_addr.interf)
  {
    if((this->bad_sonar_count = 
        cf->GetTupleCount(section, "sonar_bad_transducers")))
    {
      this->bad_sonars = new int[bad_sonar_count];
      for(int i=0;i<this->bad_sonar_count;i++)
        this->bad_sonars[i] = cf->ReadTupleInt(section, 
                                               "sonar_bad_transducers",
                                               i, -1);
    }
    this->sonar_buffer = cf->ReadInt(section, "sonar_buffer", 20);
  }

  if(!this->laser_addr.interf && !this->sonar_addr.interf)
  {
    PLAYER_ERROR("ND needs at least one sonar or one laser");
    this->SetError(-1);
    return;
  }
  
  return;
}


ND::~ND() 
{
  delete [] bad_sonars;
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int ND::Setup() 
{
  // Initialise the underlying position device.
  if (this->SetupOdom() != 0)
    return -1;
  
  this->active_goal = false;
  
  // Initialise the laser.
  if (this->laser_addr.interf && this->SetupLaser() != 0)
    return -1;
  if (this->sonar_addr.interf && this->SetupSonar() != 0)
    return -1;

  this->obstacles.longitud = 0;
  this->stall = false;
  this->turning_in_place = false;
  this->last_odom_pose.px = 
          this->last_odom_pose.py = 
          this->last_odom_pose.pa = FLT_MAX;

  this->waiting = false;

  // Start the driver thread.
  this->StartThread();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int ND::Shutdown() 
{
  // Stop the driver thread.
  this->StopThread();

  // Stop the laser
  if(this->laser)
    this->ShutdownLaser();

  // Stop the sonar
  if(this->sonar)
    this->ShutdownSonar();

  // Stop the odom device.
  this->ShutdownOdom();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the underlying odom device.
int ND::SetupOdom() 
{
  // Setup the output position device
  if(!(this->odom = deviceTable->GetDevice(this->odom_addr)))
  {
    PLAYER_ERROR("unable to locate suitable output position device");
    return -1;
  }
  if(this->odom->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to output position device");
    return -1;
  }

  // Setup the input position device
  if(!(this->localize = deviceTable->GetDevice(this->localize_addr)))
  {
    PLAYER_ERROR("unable to locate suitable input position device");
    return -1;
  }
  if(this->localize->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to input position device");
    return -1;
  }

  // Get the odometry geometry
  Message* msg;
  if(!(msg = this->odom->Request(this->InQueue,
                                 PLAYER_MSGTYPE_REQ,
                                 PLAYER_POSITION2D_REQ_GET_GEOM,
                                 NULL, 0, NULL,false)) ||
     (msg->GetHeader()->size != sizeof(player_position2d_geom_t)))
  {
    PLAYER_ERROR("failed to get geometry of underlying position device");
    if(msg)
      delete msg;
    return(-1);
  }
  player_position2d_geom_t* geom = (player_position2d_geom_t*)msg->GetPayload();

  this->robot_geom = *geom;
  printf("robot geom: %.3f %.3f %.3f %.3f %.3f\n",
         this->robot_geom.size.sl,
         this->robot_geom.size.sw,
         this->robot_geom.pose.px,
         this->robot_geom.pose.py,
         RTOD(this->robot_geom.pose.pyaw));
         

  delete msg;

  memset(&this->odom_pose, 0, sizeof(this->odom_pose));
  memset(&this->odom_vel, 0, sizeof(this->odom_vel));
  this->odom_stall = false;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the underlying odom device.
int ND::ShutdownOdom() 
{
  // Stop the robot before unsubscribing
  this->PutPositionCmd(0.0,0.0);
  this->odom->Unsubscribe(this->InQueue);
  this->localize->Unsubscribe(this->InQueue);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the laser
int ND::SetupLaser() 
{
  if(!(this->laser = deviceTable->GetDevice(this->laser_addr)))
  {
    PLAYER_ERROR("unable to locate suitable laser device");
    return -1;
  }
  if (this->laser->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to laser device");
    return -1;
  }

  player_laser_geom_t* cfg;
  Message* msg;

  // Get the laser pose
  if(!(msg = this->laser->Request(this->InQueue, 
                                  PLAYER_MSGTYPE_REQ,
                                  PLAYER_LASER_REQ_GET_GEOM,
                                  NULL, 0, NULL,false)))
  {
    PLAYER_ERROR("failed to get laser geometry");
    return(-1);
  }
  
  // Store the laser pose
  cfg = (player_laser_geom_t*)msg->GetPayload();
  this->laser_pose = cfg->pose;
  delete msg;

  // Allocate space for laser scans that we'll buffer
  this->laser_obstacles = (TInfoEntorno*)malloc(this->laser_buffer *
                                                sizeof(TInfoEntorno));
  assert(this->laser_obstacles);
  this->num_laser_scans = 0;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the sonar
int ND::SetupSonar() 
{
  if(!(this->sonar = deviceTable->GetDevice(this->sonar_addr)))
  {
    PLAYER_ERROR("unable to locate suitable sonar device");
    return -1;
  }
  if (this->sonar->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to sonar device");
    return -1;
  }

  player_sonar_geom_t* cfg;
  Message* msg;

  // Get the sonar poses
  if(!(msg = this->sonar->Request(this->InQueue, 
                                  PLAYER_MSGTYPE_REQ,
                                  PLAYER_SONAR_REQ_GET_GEOM,
                                  NULL, 0, NULL,false)))
  {
    PLAYER_ERROR("failed to get sonar geometry");
    return(-1);
  }
  
  // Store the sonar poses
  cfg = (player_sonar_geom_t*)msg->GetPayload();
  this->num_sonars = cfg->poses_count;
  this->sonar_poses = new player_pose3d_t[num_sonars];
  for(int i=0;i<this->num_sonars;i++)
  {
    this->sonar_poses[i] = cfg->poses[i];
  }
  delete msg;

  // Allocate space for sonar scans that we'll buffer
  this->sonar_obstacles = (TInfoEntorno*)malloc(this->sonar_buffer *
                                                sizeof(TInfoEntorno));
  assert(this->sonar_obstacles);
  this->num_sonar_scans = 0;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shut down the laser
int ND::ShutdownLaser() 
{
  this->laser->Unsubscribe(this->InQueue);
  free(this->laser_obstacles);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shut down the sonar
int ND::ShutdownSonar() 
{
  this->sonar->Unsubscribe(this->InQueue);
  delete [] sonar_poses;
  free(this->sonar_obstacles);
  return 0;
}

// Send a command to the motors
void
ND::PutPositionCmd(double vx, double va)
{
  player_position2d_cmd_vel_t cmd;

  memset(&cmd,0,sizeof(player_position2d_cmd_vel_t));

  cmd.vel.px = vx;
  cmd.vel.pa = va;
  cmd.state = 1;
  // cmd.type = 0;
  //printf("sending: %.3f %.3f\n", vx, RTOD(va));

  this->odom->PutMsg(this->InQueue,
                     PLAYER_MSGTYPE_CMD,
                     PLAYER_POSITION2D_CMD_VEL,
                     (void*)&cmd,sizeof(cmd),NULL);
}

void
ND::ProcessInputOdom(player_msghdr_t* hdr, player_position2d_data_t* data)
{
  this->odom_pose = data->pos;

  player_msghdr_t newhdr = *hdr;
  newhdr.addr = this->device_addr;
  player_position2d_data_t newdata;

  newdata.pos = data->pos;
  newdata.vel = this->odom_vel;
  if(data->stall)
  {
    if(this->wait_on_stall)
    {
      // We'll stop the robot and wait for the stall flag to clear itself,
      // but not report the stall
      this->PutPositionCmd(0.0,0.0);
      this->waiting = true;
      newdata.stall = 0;
    }
    else
      newdata.stall = 1;
  }
  else
  {
    newdata.stall = 0;
    this->waiting = false;
  }

  // this->stall indicates that we're stuck (either ND threw an emergency 
  // stop or it was failing to make progress).  Set the stall flag to let
  // whoever's listening that we've given up.
  if(this->stall)
    newdata.stall = 1;

  this->Publish(&newhdr, &newdata);
}

void
ND::ProcessOutputOdom(player_msghdr_t* hdr, player_position2d_data_t* data)
{
  this->odom_vel = data->vel;
  // Stage's stall flag seems to be broken
  //this->odom_stall = data->stall;
  this->odom_stall = false;
}

void
ND::ProcessLaser(player_msghdr_t* hdr, player_laser_data_t* scan)
{
  double x, y, rx, ry, r, b, db;
  int idx;

  db = scan->resolution;

  // Is the scan buffer full?
  if(this->num_laser_scans == this->laser_buffer)
  {
    // pop the oldest one off
    memmove(this->laser_obstacles,
            this->laser_obstacles+1,
            (this->num_laser_scans-1) * sizeof(TInfoEntorno));
    // add this one to the end
    idx = this->num_laser_scans - 1;
  }
  else
  {
    // we're still filling the buffer; add this one to the end
    idx = this->num_laser_scans++;
  }

  for(unsigned int i=0;i<scan->ranges_count;i++)
  {
    b = scan->min_angle + (i * db);
    r = scan->ranges[i];

    // convert to cartesian coords, in the laser's frame
    x = r * cos(b);
    y = r * sin(b);

    // convert to the robot's frame
    rx = (this->laser_pose.px + 
          x * cos(this->laser_pose.pyaw) -
          y * sin(this->laser_pose.pyaw));
    ry = (this->laser_pose.py + 
          x * sin(this->laser_pose.pyaw) +
          y * cos(this->laser_pose.pyaw));

    // convert to the odometric frame and add to the obstacle list
    this->laser_obstacles[idx].punto[i].x = (this->odom_pose.px + 
                                             rx * cos(this->odom_pose.pa) -
                                             ry * sin(this->odom_pose.pa));
    this->laser_obstacles[idx].punto[i].y = (this->odom_pose.py + 
                                             rx * sin(this->odom_pose.pa) +
                                             ry * cos(this->odom_pose.pa));
  }
  this->laser_obstacles[idx].longitud = scan->ranges_count;
}

void
ND::ProcessSonar(player_msghdr_t* hdr, player_sonar_data_t* scan)
{
  double x, y, rx, ry;
  double r;
  int j;
  int count = 0;
  int idx;

  // Is the scan buffer full?
  if(this->num_sonar_scans == this->sonar_buffer)
  {
    // pop the oldest one off
    memmove(this->sonar_obstacles,
            this->sonar_obstacles+1,
            (this->num_sonar_scans-1) * sizeof(TInfoEntorno));
    // add this one to the end
    idx = this->num_sonar_scans - 1;
  }
  else
  {
    // we're still filling the buffer; add this one to the end
    idx = this->num_sonar_scans++;
  }

  for(unsigned int i=0;i<scan->ranges_count;i++)
  {
    r = scan->ranges[i];

    // Is this a bad transducer?
    for(j=0;j<this->bad_sonar_count;j++)
    {
      if(this->bad_sonars[j] == (int)i)
        break;
    }
    if(j<this->bad_sonar_count)
      continue;

    // convert to cartesian coords, in the sonar's frame
    x = r;
    y = 0.0;

    // convert to the robot's frame
    rx = (this->sonar_poses[i].px + 
          x * cos(this->sonar_poses[i].pyaw) -
          y * sin(this->sonar_poses[i].pyaw));
    ry = (this->sonar_poses[i].py + 
          x * sin(this->sonar_poses[i].pyaw) +
          y * cos(this->sonar_poses[i].pyaw));

    // convert to the odometric frame and add to the obstacle list
    this->sonar_obstacles[idx].punto[count].x = (this->odom_pose.px + 
                                                 rx * cos(this->odom_pose.pa) -
                                                 ry * sin(this->odom_pose.pa));
    this->sonar_obstacles[idx].punto[count].y = (this->odom_pose.py + 
                                                 rx * sin(this->odom_pose.pa) +
                                                 ry * cos(this->odom_pose.pa));
    count++;
  }
  this->sonar_obstacles[idx].longitud = count;
}

void 
ND::ProcessCommand(player_msghdr_t* hdr, player_position2d_cmd_vel_t* cmd)
{
  if(!cmd->state)
  {
    this->PutPositionCmd(0.0,0.0);
    this->active_goal = false;
  }
 
  else 
  {
    PLAYER_MSG2(2, "Stopped by velocity command (%.3f %.3f)",
                cmd->vel.px, RTOD(cmd->vel.pa));
    this->PutPositionCmd(cmd->vel.px, cmd->vel.pa);
    this->active_goal = false;
  }
 if(cmd->vel.px < 0)
      this->dir = -1;
    else
      this->dir = 1;
}

void 
ND::ProcessCommand(player_msghdr_t* hdr, player_position2d_cmd_pos_t* cmd)
{
  PLAYER_MSG3(2, "New goal: (%.3f %.3f %.3f)",
              cmd->pos.px, 
              cmd->pos.py, 
              RTOD(cmd->pos.pa));
  // position control;  cache the goal and we'll process it in the main
  // loop.  also cache vel.px, which tells us whether to go forward or
  // backward to get to the goal.
  this->goal = cmd->pos;

  this->active_goal = true;
  this->turning_in_place = false;
  this->stall = false;
  GlobalTime->GetTimeDouble(&this->translate_start_time);
  this->last_odom_pose = this->odom_pose;
}

void
ND::SetDirection(int dir)
{
  if(dir == this->current_dir)
    return;

  if(dir > 0)
  {
    this->NDparametros.front = this->robot_geom.size.sl/2.0 + this->robot_geom.pose.px + this->safety_dist; // Distance to the front
    this->NDparametros.back = this->robot_geom.size.sl/2.0 - this->robot_geom.pose.px + this->safety_dist;  // Distance to the back
    InicializarND(&this->NDparametros);
  }
  else
  {
    this->NDparametros.front = this->robot_geom.size.sl/2.0 - this->robot_geom.pose.px + this->safety_dist; // Distance to the front
    this->NDparametros.back = this->robot_geom.size.sl/2.0 + this->robot_geom.pose.px + this->safety_dist;  // Distance to the back
    InicializarND(&this->NDparametros);
  }
  this->current_dir = dir;
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int ND::ProcessMessage(QueuePointer &resp_queue, 
                       player_msghdr * hdr, 
                       void * data)
{
  // Is it new odometry data?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, 
                           PLAYER_POSITION2D_DATA_STATE, 
                           this->odom_addr))
  {
    this->ProcessOutputOdom(hdr, (player_position2d_data_t*)data);

    // In case the input and output are the same device
    if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, 
                             PLAYER_POSITION2D_DATA_STATE, 
                             this->localize_addr))
      this->ProcessInputOdom(hdr, (player_position2d_data_t*)data);

    return(0);
  }
  // Is it new localization data?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, 
                           PLAYER_POSITION2D_DATA_STATE, 
                           this->localize_addr))
  {
    this->ProcessInputOdom(hdr, (player_position2d_data_t*)data);
    return(0);
  }
  // Is it a new laser scan?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, 
                                PLAYER_LASER_DATA_SCAN, 
                                this->laser_addr))
  {
    this->ProcessLaser(hdr, (player_laser_data_t*)data);
    return(0);
  }
  // Is it a new sonar scan?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, 
                                PLAYER_SONAR_DATA_RANGES, 
                                this->sonar_addr))
  {
    this->ProcessSonar(hdr, (player_sonar_data_t*)data);
    return(0);
  }
  // Is it a new goal?
 else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
			       PLAYER_POSITION2D_CMD_POS,
                                this->device_addr))
  {
    this->ProcessCommand(hdr, (player_position2d_cmd_pos_t*)data);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                                PLAYER_POSITION2D_CMD_VEL, 
                                this->device_addr))
  {
    this->ProcessCommand(hdr, (player_position2d_cmd_vel_t*)data);
    return 0;
  }
  // Is it a request for the underlying device?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, -1, this->device_addr))
  {
    // Pass the request on to the underlying position device and wait for
    // the reply.
    Message* msg;

    if(!(msg = this->odom->Request(this->InQueue,
                                   hdr->type,
                                   hdr->subtype,
                                   (void*)data, 
                                   hdr->size, 
                                   &hdr->timestamp)))
    {
      PLAYER_WARN1("failed to forward config request with subtype: %d\n",
                   hdr->subtype);
      return(-1);
    }

    player_msghdr_t* rephdr = msg->GetHeader();
    void* repdata = msg->GetPayload();
    // Copy in our address and forward the response
    rephdr->addr = this->device_addr;
    this->Publish(resp_queue, rephdr, repdata);
    delete msg;
    return(0);
  }
  else
    return -1;
}

double
ND::Threshold(double v, double v_min, double v_max)
{
  if(v == 0.0)
    return(v);
  else if(v > 0.0)
  {
    v = MIN(v, v_max);
    v = MAX(v, v_min);
    return(v);
  }
  else
  {
    v = MAX(v, -v_max);
    v = MIN(v, -v_min);
    return(v);
  }
}

void
ND::Main()
{
  TVelocities *cmd_vel;
  TCoordenadas goal;
  TInfoMovimiento pose;
  double g_dx, g_da;
  double vx, va;

  // Fill in the ND's parameter structure

  // Rectangular geometry
  this->NDparametros.geometryRect = 1;		      
  // Distance to the front
  this->NDparametros.front = this->robot_geom.size.sl/2.0 + this->robot_geom.pose.px + this->safety_dist; 
  // Distance to the back
  this->NDparametros.back = this->robot_geom.size.sl/2.0 - this->robot_geom.pose.px + this->safety_dist;  
  // Distance to the left side 
  this->NDparametros.left = this->robot_geom.size.sw/2.0 + this->safety_dist;  

  this->NDparametros.R = 0.2F;			// Not used 

  this->NDparametros.holonomic = 0;		// Non holonomic vehicle

  this->NDparametros.vlmax = this->vx_max;
  this->NDparametros.vamax = this->va_max;

  this->NDparametros.almax = 0.75F;		// More or less it will be like this
  this->NDparametros.aamax = 0.75F;

  this->NDparametros.dsmax = this->avoid_dist; // Security distance
  this->NDparametros.dsmin = this->NDparametros.dsmax/4.0F;
  //NDparametros.enlarge = NDparametros.dsmin/2.0F;
  this->NDparametros.enlarge = this->NDparametros.dsmin*0.2F;
  
  this->NDparametros.discontinuity = this->NDparametros.left;  // Discontinuity (check whether it fits)

  this->NDparametros.T = 0.1F;  // Sample rate of the SICK

  // Pass the structure to ND for initialization
  InicializarND(&this->NDparametros);

  this->current_dir = 1;

  for(;;)
  {
    usleep(20000);

    pthread_testcancel();

    // this->laser_obstacles and this->sonar_obstacles get updated by this
    // call
    this->ProcessMessages();

    // are we waiting for a stall to clear?
    if(this->waiting)
      continue;

    // do we have a goal?
    if(!this->active_goal)
      continue;

    // The robot's current odometric pose
    pose.SR1.posicion.x = this->odom_pose.px;
    pose.SR1.posicion.y = this->odom_pose.py;
    pose.SR1.orientacion = this->odom_pose.pa;

    // Merge the (possibly buffered) laser and sonar obstacle lists
    this->obstacles.longitud = 0;
    for(int i=0;i<this->num_laser_scans;i++) {
      memcpy(this->obstacles.punto + this->obstacles.longitud,
             this->laser_obstacles[i].punto,
             this->laser_obstacles[i].longitud * sizeof(TCoordenadas));
      this->obstacles.longitud += this->laser_obstacles[i].longitud;
    }
    for(int i=0;i<this->num_sonar_scans;i++)
    {
      memcpy(this->obstacles.punto + this->obstacles.longitud,
             this->sonar_obstacles[i].punto,
             this->sonar_obstacles[i].longitud * sizeof(TCoordenadas));
      this->obstacles.longitud += this->sonar_obstacles[i].longitud;
    }
    // TODO: put a smarter check earlier
    assert(this->obstacles.longitud <= MAX_POINTS_SCENARIO);

    // are we at the goal?
    g_dx = hypot(this->goal.px-this->odom_pose.px,
                 this->goal.py-this->odom_pose.py);
    g_da = this->angle_diff(this->goal.pa, this->odom_pose.pa);

    // Are we there?
    if((g_dx < this->dist_eps) && (fabs(g_da) < this->ang_eps))
    {
      this->active_goal = false;
      this->PutPositionCmd(0.0,0.0);
      PLAYER_MSG0(1, "At goal");
      continue;
    }
    else
    {
      // are we close enough in distance?
      if((g_dx < this->dist_eps) || (this->turning_in_place))
      {
        PLAYER_MSG0(3, "Turning in place");
        // To make the robot turn (safely) to the goal orientation, we'll
        // give it a fake goal that is in the right direction, and just
        // ignore the translational velocity.
        goal.x = this->odom_pose.px + 10.0 * cos(this->goal.pa);
        goal.y = this->odom_pose.py + 10.0 * sin(this->goal.pa);

        // In case we went backward to get here, reverse direction so that we
        // can attain the goal heading
        this->SetDirection(1);

        cmd_vel = IterarND(goal,
                           this->dist_eps,
                           &pose,
                           &this->obstacles,
                           NULL);
        if(!cmd_vel)
        {
          // Emergency stop
          this->PutPositionCmd(0.0, 0.0);
          this->stall = true;
          this->active_goal = false;
          PLAYER_MSG0(1, "Emergency stop");
          continue;
        }
        else
          this->stall = false;

        vx = 0.0;
        va = cmd_vel->w;

        if(!this->turning_in_place)
        {
          // first time; cache the time and current heading error
          GlobalTime->GetTimeDouble(&this->rotate_start_time);
          this->rotate_min_error = fabs(g_da);
          this->turning_in_place = true;
        }
        else
        {
          // Are we making progress?
          if(fabs(g_da) < this->rotate_min_error)
          {
            // yes; reset the time
            GlobalTime->GetTimeDouble(&this->rotate_start_time);
            this->rotate_min_error = fabs(g_da);
          }
          else
          {
            // no; have we run out of time?
            double t; 
            GlobalTime->GetTimeDouble(&t);
            if((t - this->rotate_start_time) > this->rotate_stuck_time)
            {
              PLAYER_WARN("Ran out of time trying to attain goal heading");
              this->PutPositionCmd(0.0, 0.0);
              this->stall = true;
              this->active_goal = false;
              continue;
            }
          }
        }
      }
      // we're far away; execute the normal ND loop
      else
      {
        // Have we moved far enough?
        double o_dx = hypot(this->odom_pose.px - this->last_odom_pose.px,
                            this->odom_pose.py - this->last_odom_pose.py);
        double o_da = this->angle_diff(this->odom_pose.pa,
                                       this->last_odom_pose.pa);
        if((o_dx > this->translate_stuck_dist) ||
           (fabs(o_da) > this->translate_stuck_angle))
        {
          this->last_odom_pose = this->odom_pose;
          GlobalTime->GetTimeDouble(&this->translate_start_time);
        }
        else
        {
          // Has it been long enough?
          double t; 
          GlobalTime->GetTimeDouble(&t);
          if((t - this->translate_start_time) > this->translate_stuck_time)
          {
            PLAYER_WARN("ran out of time trying to get to goal");
            this->PutPositionCmd(0.0, 0.0);
            this->stall = true;
            this->active_goal = false;
            continue;
          }
        }

        // The current odometric goal
        goal.x = this->goal.px;
        goal.y = this->goal.py;

        // Were we asked to go backward?
        if(this->dir < 0)
        {
          // Trick the ND by telling it that the robot is pointing the 
          // opposite direction
          pose.SR1.orientacion = NORMALIZE(pose.SR1.orientacion + M_PI);
          // Also reverse the robot's geometry (it may be asymmetric
          // front-to-back)
          this->SetDirection(-1);
        }
        else
          this->SetDirection(1);

        cmd_vel = IterarND(goal,
                           this->dist_eps,
                           &pose,
                           &this->obstacles,
                           NULL);
        if(!cmd_vel)
        {
          // Emergency stop
          this->PutPositionCmd(0.0, 0.0);
          this->stall = true;
          this->active_goal = false;
          PLAYER_MSG0(1, "Emergency stop");
          continue;
        }
        else
          this->stall = false;

        vx = cmd_vel->v;
        va = cmd_vel->w;
      }

      if(!vx && !va)
      {
        // ND is done, yet we didn't detect that we reached the goal.  How
        // odd.
        this->PutPositionCmd(0.0, 0.0);
        this->stall = true;
        this->active_goal = false;
        PLAYER_MSG0(1, "ND is done");
        continue;
      }
      else
      {
        vx = this->Threshold(vx, this->vx_min, this->vx_max);
        if(!vx)
          va = this->Threshold(va, this->va_min, this->va_max);

        // Were we asked to go backward?
        if(this->dir < 0)
        {
          // reverse the commanded x velocity
          vx = -vx;
        }
        this->PutPositionCmd(vx, va);
      }
    }
  }
}

// computes the signed minimum difference between the two angles.
double
ND::angle_diff(double a, double b)
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

////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.
#if 0
/* need the extern to avoid C++ name-mangling  */
extern "C" {
  int player_driver_init(DriverTable* table)
  {
    puts("ND driver initializing");
    ND_Register(table);
    puts("ND initialization done");
    return(0);
  }
}
#endif
