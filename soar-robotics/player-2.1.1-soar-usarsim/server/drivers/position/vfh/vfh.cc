#include <assert.h>
#include <math.h>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <libplayercore/playercore.h>
#include "vfh_algorithm.h"

extern PlayerTime *GlobalTime;

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_vfh vfh
 * @brief Vector Field Histogram local navigation algorithm

@note This driver may take several seconds to start up, especially on
slower machines.  You may want to set the 'alwayson' option for vfh to
'1' in your configuration file in order to front-load this delay.
Otherwise, your client may experience a timeout in trying to subscribe
to this device.

The vfh driver implements the Vector Field Histogram Plus local
navigation method by Ulrich and Borenstein.  VFH+ provides real-time
obstacle avoidance and path following capabilities for mobile robots.
Layered on top of a laser-equipped robot, vfh works great as a local
navigation system (for global navigation, you can layer the @ref
driver_wavefront driver on top of vfh).

The primary parameters to tweak to get reliable performance are
safety_dist and free_space_cutoff.  In general, safety_dist determines how
close the robot will come to an obstacle while turning (around a corner
for instance) and free_space_cutoff determines how close a robot will
get to an obstacle in the direction of motion before turning to avoid.
From experience, it is recommeded that max_turnrate should be at least
15% of max_speed.

To get initiated to VFH, I recommend starting with the default
values for all parameters and experimentally adjusting safety_dist
and free_space_cutoff to get a feeling for how the parameters affect
performance.  Once comfortable, increase max_speed and max_turnrate.
Unless you are familiar with the VFH algorithm, I don't recommend
deviating from the default values for cell_size, window_diameter,
or sector_angle.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d : accepts target poses, to which vfh will
  attempt to drive the robot.  Also passes through the data from the
  underlying @ref interface_position2d device.  All data and commands
  are in the odometric frame of the underlying device.

- @ref interface_planner : similar to above, but uses the planer
  interface which allows extras such as "done" when arrived.

@par Requires

- @ref interface_position2d : the underlying robot that will be
  controlled by vfh.

- Exactly one of:
  - @ref interface_laser : the laser that will be used to avoid
    obstacles
  - @ref interface_sonar : the sonar that will be used to avoid
    obstacles

- @todo : add support for getting the robot's true global pose via the
  @ref interface_simulation interface

@par Configuration requests

- all position2d requests (as long as the underlying position2d device
  supports them)

@par Supported commands

- PLAYER_POSITION2D_CMD_POS : Position control.  This is the normal way to use vfh.  Velocity commands will be sent to the underlying @ref interface_position2d device to drive it toward the given pose.
- PLAYER_POSITION2D_CMD_VEL : Velocity control.  Position control is disabled and the velocities are passed directly through to the underlyin @ref interface_position2d device.

@par Configuration file options

- cell_size (length)
  - Default: 0.1 m
  - Local occupancy map grid size
- window_diameter (integer)
  - Default: 61
  - Dimensions of occupancy map (map consists of window_diameter X
    window_diameter cells).
- sector_angle (integer)
  - Default: 5
  - Histogram angular resolution, in degrees.
- safety_dist_0ms (length)
  - Default: 0.1 m
  - The minimum distance the robot is allowed to get to obstacles when stopped.
- safety_dist_1ms (length)
  - Default: safety_dist_0ms
  - The minimum distance the robot is allowed to get to obstacles when
    travelling at 1 m/s.
- max_speed (length / sec)
  - Default: 0.2 m/sec
  - The maximum allowable speed of the robot.
- max_speed_narrow_opening (length / sec)
  - Default: max_speed
  - The maximum allowable speed of the robot through a narrow opening
- max_speed_wide_opening (length / sec)
  - Default: max_speed
  - The maximum allowable speed of the robot through a wide opening
- max_acceleration (length / sec / sec)
  - Default: 0.2 m/sec/sec
  - The maximum allowable acceleration of the robot.
- min_turnrate (angle / sec)
  - Default: 10 deg/sec
  - The minimum allowable turnrate of the robot.
- max_turnrate_0ms (angle / sec)
  - Default: 40 deg/sec
  - The maximum allowable turnrate of the robot when stopped.
- max_turnrate_1ms (angle / sec)
  - Default: max_turnrate_0ms
  - The maximum allowable turnrate of the robot when travelling 1 m/s.
- min_turn_radius_safety_factor (float)
  - Default: 1.0
  - ?
- free_space_cutoff_0ms (float)
  - Default: 2000000.0
  - Unitless value.  The higher the value, the closer the robot will
    get to obstacles before avoiding (while stopped).
- free_space_cutoff_1ms (float)
  - Default: free_space_cutoff_0ms
  - Unitless value.  The higher the value, the closer the robot will
    get to obstacles before avoiding (while travelling at 1 m/s).
- obs_cutoff_0ms (float)
  - Default: free_space_cutoff_0ms
  - ???
- obs_cutoff_1ms (float)
  - Default: free_space_cutoff_1ms
  - ???
- weight_desired_dir (float)
  - Default: 5.0
  - Bias for the robot to turn to move toward goal position.
- weight_current_dir (float)
  - Default: 3.0
  - Bias for the robot to continue moving in current direction of travel.
- distance_epsilon (length)
  - Default: 0.5 m
  - Planar distance from the target position that will be considered
    acceptable.
- angle_epsilon (angle)
  - Default: 10 deg
  - Angular difference from target angle that will considered acceptable.
- Stall escape options.  If the underlying position2d device reports a
  stall, this driver can attempt a blind escape procedure.  It does so by
  driving forward or backward while turning for a fixed amount of time.  If
  the escape fails (i.e., the stall is still in effect), then it will try again.
  - escape_speed (length / sec)
    - Default: 0.0
    - If non-zero, the translational velocity that will be used while trying
      to escape.
  - escape_time (float)
    - Default: 0.0
    - If non-zero, the time (in seconds) for which an escape attempt will be 
      made.
  - escape_max_turnrate (angle / sec)
    - Default: 0.0
    - If non-zero, the maximum angular velocity that will be used when 
      trying to escape.
  - synchronous (int)
    - default: 0
    -  If zero (the default), VFH runs in its own thread. If non-zero, VFH runs in the main Player thread, which will make the server less responsive, but prevent nasty asynchronous behaviour under high CPU load. This is probably only useful when running demanding simulations. 

@par Example
@verbatim
driver
(
  name "p2os"
  provides ["odometry::position:1"]
  port "/dev/ttyS0"
)
driver
(
  name "sicklms200"
  provides ["laser:0"]
  port "/dev/ttyS1"
)
driver
(
  name "vfh"
  requires ["position:1" "laser:0"]
  provides ["position:0"]
  safety_dist 0.10
  distance_epsilon 0.3
  angle_epsilon 5
)
@endverbatim

@author Chris Jones, Brian Gerkey, Alex Brooks, Richard Vaughan

*/

/** @} */

class VFH_Class : public Driver
{
  public:
    // Constructor
    VFH_Class( ConfigFile* cf, int section);

    // Destructor
    virtual ~VFH_Class();

    // Setup/shutdown routines.
    virtual int Setup();
    virtual int Shutdown();

    // Process incoming messages from clients
    virtual int ProcessMessage(QueuePointer & resp_queue,
                               player_msghdr * hdr,
                               void * data);
    // Main function for device thread.
    virtual void Main();

    // Update method called in Player's main thread
    virtual void Update();

  private:
    bool active_goal;
    bool turninginplace;

    // most of the work is done in this method. It is either called
    // from ::Main() or ::Update(), depending on ::synchronous_mode.
    void DoOneUpdate(); 

    // Set up the odometry device.
    int SetupOdom();
    int ShutdownOdom();
    void ProcessOdom(player_msghdr_t* hdr, player_position2d_data_t &data);

    // Class to handle the internal VFH algorithm
    // (like maintaining histograms etc)
    VFH_Algorithm *vfh_Algorithm;

    // Process requests.  Returns 1 if the configuration has changed.
    //int HandleRequests();
    // Handle motor power requests
    void HandlePower(void *client, void *req, int reqlen);
    // Handle geometry requests.
    void HandleGetGeom(void *client, void *req, int reqlen);

    // Set up the laser device.
    int SetupLaser();
    int ShutdownLaser();
    int SetupSonar();
    int ShutdownSonar();
    void ProcessLaser(player_laser_data_t &);
    void ProcessSonar(player_sonar_data_t &);

    // Send commands to underlying position device
    void PutCommand( int speed, int turnrate );

    // Check for new commands from server
    void ProcessCommand(player_msghdr_t* hdr, player_position2d_cmd_pos_t &);

    // Computes the signed minimum difference between the two angles.  Inputs
    // and return values are in degrees.
    double angle_diff(double a, double b);

    // Devices we provide
    player_devaddr_t position_id;
    player_devaddr_t planner_id;
    bool planner;

    player_planner_data_t planner_data;
    

    // Devices we require
    // Odometry device info
    Device *odom;
    player_devaddr_t odom_addr;

    double dist_eps;
    double ang_eps;

    // how fast and how long to back up to escape from a stall
    double escape_speed;
    double escape_max_turnspeed;
    double escape_time;

    // Pose and velocity of robot in odometric cs (mm,mm,deg)
    double odom_pose[3];
    double odom_vel[3];

    // Stall flag and counter
    int odom_stall;

    // Laser device info
    Device *laser;
    player_devaddr_t laser_addr;

    // Sonar device info
    Device *sonar;
    player_devaddr_t sonar_addr;
    int num_sonars;
    player_pose3d_t * sonar_poses;

    // Laser range and bearing values
    int laser_count;
    //double (*laser_ranges)[2];
    double laser_ranges[361][2];

    // Control velocity
    double con_vel[3];

    int speed, turnrate;
    double reset_odom_x, reset_odom_y, reset_odom_t;
    int32_t goal_x, goal_y, goal_t;
    int cmd_state, cmd_type;


  // iff true, run in the main Player thread instead of a dedicated
  // thread. Useful under heavy CPU load, for example when using a
  // simulator with lots of robots - rtv 
  bool synchronous_mode;

  /* the following vars used to be local to the Main() method. I moved
     them here when implementing the synchronous mode - rtv */
  
  float dist; 
  double angdiff; 
  struct timeval startescape, curr; 
  bool escaping; 
  double timediff; int
  escape_turnrate_deg;
  
  // bookkeeping to implement hysteresis when rotating at the goal
  int rotatedir;

  // bookkeeping to implement smarter escape policy
  int escapedir;

  /* end of moved vars - rtv*/

};

// Initialization function
Driver*
VFH_Init(ConfigFile* cf, int section)
{
  return ((Driver*) (new VFH_Class( cf, section)));
}

// a driver registration function
void VFH_Register(DriverTable* table)
{
  table->AddDriver("vfh",  VFH_Init);
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int VFH_Class::Setup()
{
  this->active_goal = false;
  this->turninginplace = false;
  this->goal_x = this->goal_y = this->goal_t = 0;


  // Initialise the underlying position device.
  if (this->SetupOdom() != 0)
    return -1;

  // Initialise the laser.
  if (this->laser_addr.interf && this->SetupLaser() != 0)
    return -1;
  if (this->sonar_addr.interf && this->SetupSonar() != 0)
    return -1;


/*
<<<<<<< vfh.cc
  // FIXME
  // Allocate and intialize
  vfh_Algorithm->Init();

  // initialize some navigation state
  rotatedir = 1;
  escapedir = 1;
  escaping = 0;

=======
>>>>>>> 1.82
*/
  // Start the driver thread.
  if( ! synchronous_mode )
    this->StartThread();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int VFH_Class::Shutdown()
{
  // Stop the driver thread.
  if( ! this->synchronous_mode )
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


/////////////////////////////////////////////////////////////////////////////////
// Update the driver in the main Player thread
void VFH_Class::Update()
{
  if( synchronous_mode )
    this->DoOneUpdate();
  // otherwise, a dedicated thread is running: see VFH_Class::Main()
}


////////////////////////////////////////////////////////////////////////////////
// Set up the underlying odom device.
int VFH_Class::SetupOdom()
{
  if(!(this->odom = deviceTable->GetDevice(this->odom_addr)))
  {
    PLAYER_ERROR("unable to locate suitable position device");
    return -1;
  }
  if(this->odom->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to position device");
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

  // take the bigger of the two dimensions and halve to get a radius
  float robot_radius = MAX(geom->size.sl,geom->size.sw);
  robot_radius /= 2.0;

  delete msg;

  vfh_Algorithm->SetRobotRadius( robot_radius * 1e3 );

  this->odom_pose[0] = this->odom_pose[1] = this->odom_pose[2] = 0.0;
  this->odom_vel[0] = this->odom_vel[1] = this->odom_vel[2] = 0.0;
  this->cmd_state = 1;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the underlying odom device.
int VFH_Class::ShutdownOdom()
{

  // Stop the robot before unsubscribing
  this->speed = 0;
  this->turnrate = 0;
  this->PutCommand( speed, turnrate );

  this->odom->Unsubscribe(this->InQueue);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the laser
int VFH_Class::SetupLaser()
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

  this->laser_count = 0;
  //this->laser_ranges = NULL;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the sonar
int VFH_Class::SetupSonar()
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

  this->laser_count = 0;
  //this->laser_ranges = NULL;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shut down the laser
int VFH_Class::ShutdownLaser()
{
  this->laser->Unsubscribe(this->InQueue);
  //delete [] laser_ranges;
  //laser_ranges = NULL;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shut down the sonar
int VFH_Class::ShutdownSonar()
{
  this->sonar->Unsubscribe(this->InQueue);
  //delete [] laser_ranges;
  //laser_ranges = NULL;
  delete [] sonar_poses;
  sonar_poses = NULL;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Process new odometry data
void
VFH_Class::ProcessOdom(player_msghdr_t* hdr, player_position2d_data_t &data)
{

  // Cache the new odometric pose, velocity, and stall info
  // NOTE: this->odom_pose is in (mm,mm,deg), as doubles
  this->odom_pose[0] = data.pos.px * 1e3;
  this->odom_pose[1] = data.pos.py * 1e3;
  this->odom_pose[2] = RTOD(data.pos.pa);
  this->odom_vel[0] = data.vel.px * 1e3;
  this->odom_vel[1] = data.vel.py * 1e3;
  this->odom_vel[2] = RTOD(data.vel.pa);
  this->odom_stall = data.stall;

  // Also change this info out for use by others
  player_msghdr_t newhdr = *hdr;
  newhdr.addr = this->position_id;
  this->Publish(&newhdr, (void*)&data);

 if(this->planner)
 {
   this->planner_data.pos.px = data.pos.px;
   this->planner_data.pos.py = data.pos.py;
   this->planner_data.pos.pa = data.pos.pa;

   this->Publish(this->planner_id,
                 PLAYER_MSGTYPE_DATA,
                 PLAYER_PLANNER_DATA_STATE,
                 (void*)&this->planner_data,sizeof(this->planner_data), NULL);
 }

}

////////////////////////////////////////////////////////////////////////////////
// Process new laser data
void
VFH_Class::ProcessLaser(player_laser_data_t &data)
{
  int i;
  double b, db, r;

  b = RTOD(data.min_angle);
  db = RTOD(data.resolution);

  this->laser_count = 361;
  //if (!laser_ranges)
	  //this->laser_ranges = new double[laser_count][2];

  for(i = 0; i < laser_count; i++)
    this->laser_ranges[i][0] = -1;

  // vfh seems to be very oriented around 180 degree scans so interpolate to get 180 degrees
//  b += 90.0;
  for(i = 0; i < 181; i++)
  {
  	unsigned int index = (int)rint(i/db);
  	assert(index >= 0 && index < data.ranges_count);
    this->laser_ranges[i*2][0] = data.ranges[index] * 1e3;
//    this->laser_ranges[i*2][1] = index;
//    b += db;
  }

  r = 1000000.0;
  for (i = 0; i < laser_count; i++)
  {
    if (this->laser_ranges[i][0] != -1) {
      r = this->laser_ranges[i][0];
    } else {
      this->laser_ranges[i][0] = r;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Process new sonar data, in a very crude way.
void
VFH_Class::ProcessSonar(player_sonar_data_t &data)
{
  int i;
  double b, r;
  double cone_width = 30.0;
  int count = 361;
  float sonarDistToCenter = 0.0;

  this->laser_count = count;
  //if (!laser_ranges)
	  //this->laser_ranges = new double[laser_count][2];
  
  for(i = 0; i < laser_count; i++)
    this->laser_ranges[i][0] = -1;

  //b += 90.0;
  for(i = 0; i < (int)data.ranges_count; i++)
  {
    for(b = RTOD(this->sonar_poses[i].pyaw) + 90.0 - cone_width/2.0;
        b < RTOD(this->sonar_poses[i].pyaw) + 90.0 + cone_width/2.0;
        b+=0.5)
    {
      if((b < 0) || (rint(b*2) >= count))
        continue;
      // Sonars give distance readings from the perimeter of the robot while lasers give distance
      // from the laser; hence, typically the distance from a single point, like the center.
      // Since this version of the VFH+ algorithm was written for lasers and we pass the algorithm
      // laser ranges, we must make the sonar ranges appear like laser ranges. To do this, we take
      // into account the offset of a sonar's geometry from the center. Simply add the distance from
      // the center of the robot to a sonar to the sonar's range reading.
      sonarDistToCenter = sqrt(pow(this->sonar_poses[i].px,2) + pow(this->sonar_poses[i].py,2));
      this->laser_ranges[(int)rint(b * 2)][0] = (sonarDistToCenter + data.ranges[i]) * 1e3;
      this->laser_ranges[(int)rint(b * 2)][1] = b;
    }
  }

  r = 1000000.0;
  for (i = 0; i < laser_count; i++)
  {
    if (this->laser_ranges[i][0] != -1) {
      r = this->laser_ranges[i][0];
    } else {
      this->laser_ranges[i][0] = r;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Send commands to the underlying position device
void
VFH_Class::PutCommand( int cmd_speed, int cmd_turnrate )
{
  player_position2d_cmd_vel_t cmd;

//printf("Command: speed: %d turnrate: %d\n", cmd_speed, cmd_turnrate);

  this->con_vel[0] = (double)cmd_speed;
  this->con_vel[1] = 0;
  this->con_vel[2] = (double)cmd_turnrate;

  memset(&cmd, 0, sizeof(cmd));

  // Stop the robot (locks the motors) if the motor state is set to
  // disabled.  The P2OS driver does not respect the motor state.
  if (this->cmd_state == 0)
  {
    cmd.vel.px = 0;
    cmd.vel.py = 0;
    cmd.vel.pa = 0;
  }
  // Position mode
  else
  {
    if(fabs(this->con_vel[2]) >
       (double)vfh_Algorithm->GetMaxTurnrate((int)this->con_vel[0]))
    {
      PLAYER_WARN1("fast turn %d", this->con_vel[2]);
      this->con_vel[2] = 0;
    }

    cmd.vel.px =  this->con_vel[0] / 1e3;
    cmd.vel.py =  this->con_vel[1] / 1e3;
    cmd.vel.pa =  DTOR(this->con_vel[2]);
  }

  this->odom->PutMsg(this->InQueue,
                     PLAYER_MSGTYPE_CMD,
                     PLAYER_POSITION2D_CMD_VEL,
                     (void*)&cmd,sizeof(cmd),NULL);
}


////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int VFH_Class::ProcessMessage(QueuePointer & resp_queue,
                              player_msghdr * hdr,
                              void * data)
{
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                           PLAYER_POSITION2D_DATA_STATE, this->odom_addr))
  {
    assert(hdr->size == sizeof(player_position2d_data_t));
    ProcessOdom(hdr, *reinterpret_cast<player_position2d_data_t *> (data));
    return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                                PLAYER_LASER_DATA_SCAN, this->laser_addr))
  {
    // It's not always that big...
    //assert(hdr->size == sizeof(player_laser_data_t));
    ProcessLaser(*reinterpret_cast<player_laser_data_t *> (data));
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                                PLAYER_SONAR_DATA_RANGES, this->sonar_addr))
  {
    // It's not always that big...
    //assert(hdr->size == sizeof(player_laser_data_t));
    ProcessSonar(*reinterpret_cast<player_sonar_data_t *> (data));
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                                PLAYER_PLANNER_CMD_GOAL,
                                this->planner_id))
  {
    // Message on the planner interface
    // Emulate a message on the position2d interface

    player_position2d_cmd_pos_t cmd_pos;
    player_planner_cmd_t *cmd_planner = (player_planner_cmd_t *) data;

    memset(&cmd_pos, 0, sizeof(cmd_pos));
    cmd_pos.pos.px = cmd_planner->goal.px;
    cmd_pos.pos.py = cmd_planner->goal.py;
    cmd_pos.pos.pa = cmd_planner->goal.pa;
    cmd_pos.state = 1;

    /* Process position2d command */
    ProcessCommand(hdr, cmd_pos);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                                PLAYER_POSITION2D_CMD_POS,
                                this->position_id))
  {
    assert(hdr->size == sizeof(player_position2d_cmd_pos_t));
    ProcessCommand(hdr, *reinterpret_cast<player_position2d_cmd_pos_t *> (data));
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                                PLAYER_POSITION2D_CMD_VEL,
                                this->position_id))
  {
    assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
    // make a copy of the header and change the address
    player_msghdr_t newhdr = *hdr;
    newhdr.addr = this->odom_addr;
    this->odom->PutMsg(this->InQueue, &newhdr, (void*)data);
    this->cmd_type = 0;
    this->active_goal = false;

    return 0;
  }  
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, -1, this->position_id))
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
    rephdr->addr = this->position_id;
    this->Publish(resp_queue, rephdr, repdata);
    delete msg;
    return(0);
  }
  else
  {
    return -1;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void VFH_Class::Main()
{
  // loop over the main VFH activity. If we were in synchronous mode,
  // this would happen in VFH_Class::Update(), in the main Player
  // thread instead.
  while( true )
    {
      // Wait till we get new data
      this->Wait();
      // Test if we are supposed to cancel this thread.
      pthread_testcancel();
      this->DoOneUpdate();
    }
}

////////////////////////////////////////////////////////////////////////////////
//
void VFH_Class::DoOneUpdate()
{  
  if( this->InQueue->Empty() )
    return;

  // Process any pending requests.
    this->ProcessMessages();

    if(!this->active_goal)
      return;//continue;

    // Figure how far, in distance and orientation, we are from the goal
    dist = sqrt(pow((goal_x - this->odom_pose[0]),2) +
                pow((goal_y - this->odom_pose[1]),2));
    angdiff = this->angle_diff((double)this->goal_t,this->odom_pose[2]);

    // If we're currently escaping after a stall, check whether we've done
    // so for long enough.
    if(escaping)
    {
      GlobalTime->GetTime(&curr);
      timediff = (curr.tv_sec + curr.tv_usec/1e6) -
              (startescape.tv_sec + startescape.tv_usec/1e6);
      if(timediff > this->escape_time)
      {
        // if we're still stalled, try escaping the other direction
        if(this->odom_stall)
          escapedir *= -1;
        escaping = false;
      }
    }

    // CASE 1: The robot has stalled, so escape if the user specified
    //         a non-zero escape velocity.
    if(escaping ||
       (this->escape_speed && this->escape_time && this->odom_stall))
    {
      if(!escaping)
      {
        GlobalTime->GetTime(&startescape);
        escaping = true;
      }

      this->speed = (int)rint(this->escape_speed * escapedir * 1e3);
      if(this->escape_max_turnspeed)
      {
        // pick a random turnrate in
        // [-escape_max_turnspeed,escape_max_turnspeed]
        escape_turnrate_deg = (int)rint(RTOD(this->escape_max_turnspeed));
        this->turnrate = (int)(2.0 * escape_turnrate_deg *
                               rand()/(RAND_MAX+1.0)) -
                escape_turnrate_deg/2 + 1;
      }
      else
        this->turnrate = 0;
      PutCommand(this->speed, this->turnrate);

      this->turninginplace = false;
    }
    // CASE 2: The robot is at the goal, within user-specified tolerances, so
    //         stop.
    else if((dist < (this->dist_eps * 1e3)) &&
            (fabs(DTOR(angdiff)) < this->ang_eps))
    {
      this->active_goal = false;
      this->speed = this->turnrate = 0;
      PutCommand( this->speed, this->turnrate );
      this->turninginplace = false;
      this->planner_data.done = 1;
    }
    // CASE 3: The robot is too far from the goal position, so invoke VFH to
    //         get there.
    else if (dist > (this->dist_eps * 1e3))
    {
      float Desired_Angle = (90 + atan2((goal_y - this->odom_pose[1]),
                                        (goal_x - this->odom_pose[0]))
                             * 180 / M_PI - this->odom_pose[2]);

      while (Desired_Angle > 360.0)
        Desired_Angle -= 360.0;
      while (Desired_Angle < 0)
        Desired_Angle += 360.0;

      vfh_Algorithm->Update_VFH( this->laser_ranges,
                                 (int)(this->odom_vel[0]),
                                 Desired_Angle,
                                 dist,
                                 this->dist_eps * 1e3,
                                 this->speed,
                                 this->turnrate );
      
      // HACK: if we're within twice the distance threshold, 
      // and still going fast, slow down.

      if((dist < (this->dist_eps * 1e3 * 2.0)) &&
         (this->speed > (vfh_Algorithm->GetCurrentMaxSpeed() / 2.0)))
      {
//        int foo = this->speed;
        this->speed = 
                (int)rint(vfh_Algorithm->GetCurrentMaxSpeed() / 2.0);
        //printf("slowing down from %d to %d\n",
	//     foo, this->speed);
      }

      PutCommand( this->speed, this->turnrate );
      this->turninginplace = false;
    }
    // CASE 4: The robot is at the goal position, but still needs to turn
    //         in place to reach the desired orientation.
    else
    {
      // At goal, stop
      speed = 0;

      // Turn in place in the appropriate direction, with speed
      // proportional to the angular distance to the goal orientation.
      turnrate = (int)rint(fabs(angdiff/180.0) *
                           vfh_Algorithm->GetMaxTurnrate(speed));

      // If we've just gotten to the goal, pick a direction to turn;
      // otherwise, keep turning the way we started (to prevent
      // oscillation)
      if(!this->turninginplace)
      {
        this->turninginplace = true;
        if(angdiff < 0)
          rotatedir = -1;
        else
          rotatedir = 1;
      }

      turnrate *= rotatedir;

      // Threshold to make sure we don't send arbitrarily small turn speeds
      // (which may not cause the robot to actually move).
      if(turnrate < 0)
        turnrate = MIN(turnrate,-this->vfh_Algorithm->GetMinTurnrate());
      else
        turnrate = MAX(turnrate,this->vfh_Algorithm->GetMinTurnrate());

      this->PutCommand( this->speed, this->turnrate );
    }

}

////////////////////////////////////////////////////////////////////////////////
// Check for new commands from the server
void
VFH_Class::ProcessCommand(player_msghdr_t* hdr, player_position2d_cmd_pos_t &cmd)
{
  int x,y,t;

  x = (int)rint(cmd.pos.px * 1e3);
  y = (int)rint(cmd.pos.py * 1e3);
  t = (int)rint(RTOD(cmd.pos.pa));

  this->cmd_type = 1;
  this->cmd_state = cmd.state;

  if((x != this->goal_x) || (y != this->goal_y) || (t != this->goal_t))
  {
    this->active_goal = true;
    this->turninginplace = false;
    this->goal_x = x;
    this->goal_y = y;
    this->goal_t = t;
    
    if(this->planner)
    {
       this->planner_data.goal.px = cmd.pos.px;
       this->planner_data.goal.py = cmd.pos.py;
       this->planner_data.goal.pa = cmd.pos.pa;
       this->planner_data.done = 0;

       this->planner_data.valid = 1;
            /* Not necessarily. But VFH will try anything once */

       this->planner_data.waypoint_idx = -1; /* Not supported */
       this->planner_data.waypoints_count = -1; /* Not supported */
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Constructor
VFH_Class::VFH_Class( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
  double cell_size;
  int window_diameter;
  int sector_angle;
  double safety_dist_0ms;
  double safety_dist_1ms;
  int max_speed;
  int max_speed_narrow_opening;
  int max_speed_wide_opening;
  int max_acceleration;
  int min_turnrate;
  int max_turnrate_0ms;
  int max_turnrate_1ms;
  double min_turn_radius_safety_factor;
  double free_space_cutoff_0ms;
  double obs_cutoff_0ms;
  double free_space_cutoff_1ms;
  double obs_cutoff_1ms;
  double weight_desired_dir;
  double weight_current_dir;


  this->speed = 0;
  this->turnrate = 0;
  
  // read the synchronous flag from the cfg file: defaults to not synchronous
  this->synchronous_mode = cf->ReadInt(section, "synchronous", 0 );

  cell_size = cf->ReadLength(section, "cell_size", 0.1) * 1e3;
  window_diameter = cf->ReadInt(section, "window_diameter", 61);
  sector_angle = cf->ReadInt(section, "sector_angle", 5);
  safety_dist_0ms = cf->ReadLength(section, "safety_dist_0ms", 0.1) * 1e3;
  safety_dist_1ms = cf->ReadLength(section, "safety_dist_1ms",
                                   safety_dist_0ms/1e3) * 1e3;
  max_speed = (int)rint(1e3 * cf->ReadLength(section, "max_speed", 0.2));
  max_speed_narrow_opening =
          (int)rint(1e3 * cf->ReadLength(section,
                                         "max_speed_narrow_opening",
                                         max_speed/1e3));
  max_speed_wide_opening =
          (int)rint(1e3 * cf->ReadLength(section,
                                         "max_speed_wide_opening",
                                         max_speed/1e3));
  max_acceleration = (int)rint(1e3 * cf->ReadLength(section, "max_acceleration", 0.2));
  min_turnrate = (int)rint(RTOD(cf->ReadAngle(section, "min_turnrate", DTOR(10))));
  max_turnrate_0ms = (int) rint(RTOD(cf->ReadAngle(section, "max_turnrate_0ms", DTOR(40))));
  max_turnrate_1ms =
          (int) rint(RTOD(cf->ReadAngle(section,
                                        "max_turnrate_1ms",
                                        DTOR(max_turnrate_0ms))));
  min_turn_radius_safety_factor =
          cf->ReadFloat(section, "min_turn_radius_safety_factor", 1.0);
  free_space_cutoff_0ms = cf->ReadFloat(section,
                                        "free_space_cutoff_0ms",
                                        2000000.0);
  obs_cutoff_0ms = cf->ReadFloat(section,
                                 "obs_cutoff_0ms",
                                 free_space_cutoff_0ms);
  free_space_cutoff_1ms = cf->ReadFloat(section,
                                        "free_space_cutoff_1ms",
                                        free_space_cutoff_0ms);
  obs_cutoff_1ms = cf->ReadFloat(section,
                                 "obs_cutoff_1ms",
                                 free_space_cutoff_1ms);
  weight_desired_dir = cf->ReadFloat(section, "weight_desired_dir", 5.0);
  weight_current_dir = cf->ReadFloat(section, "weight_current_dir", 3.0);

  this->dist_eps = cf->ReadLength(section, "distance_epsilon", 0.5);
  this->ang_eps = cf->ReadAngle(section, "angle_epsilon", DTOR(10.0));

  this->escape_speed = cf->ReadLength(section, "escape_speed", 0.0);
  this->escape_time = cf->ReadFloat(section, "escape_time", 0.0);
  this->escape_max_turnspeed = cf->ReadAngle(section, "escape_max_turnrate", 0.0);

  // Instantiate the classes that handles histograms
  // and chooses directions
  this->vfh_Algorithm = new VFH_Algorithm(cell_size,
                                          window_diameter,
                                          sector_angle,
                                          safety_dist_0ms,
                                          safety_dist_1ms,
                                          max_speed,
                                          max_speed_narrow_opening,
                                          max_speed_wide_opening,
                                          max_acceleration,
                                          min_turnrate,
                                          max_turnrate_0ms,
                                          max_turnrate_1ms,
                                          min_turn_radius_safety_factor,
                                          free_space_cutoff_0ms,
                                          obs_cutoff_0ms,
                                          free_space_cutoff_1ms,
                                          obs_cutoff_1ms,
                                          weight_desired_dir,
                                          weight_current_dir);

  // Devices we provide
  memset(&this->planner_id, 0, sizeof(player_devaddr_t));
  memset(&this->planner_data, 0, sizeof(player_planner_data_t));
  planner = false;
  if (cf->ReadDeviceAddr(&(this->planner_id), section, "provides",
                        PLAYER_PLANNER_CODE, -1, NULL) == 0)
  {
    planner = true;
    if (this->AddInterface(this->planner_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  memset(&this->position_id, 0, sizeof(player_devaddr_t));
  if (cf->ReadDeviceAddr(&(this->position_id), section, "provides",
                        PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if (this->AddInterface(this->position_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }


  this->odom = NULL;
  if (cf->ReadDeviceAddr(&this->odom_addr, section, "requires",
                         PLAYER_POSITION2D_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }

  this->laser = NULL;
  memset(&this->laser_addr,0,sizeof(player_devaddr_t));
  cf->ReadDeviceAddr(&this->laser_addr, section, "requires",
                     PLAYER_LASER_CODE, -1, NULL);
  this->sonar = NULL;
  memset(&this->sonar_addr,0,sizeof(player_devaddr_t));
  cf->ReadDeviceAddr(&this->sonar_addr, section, "requires",
                     PLAYER_SONAR_CODE, -1, NULL);

  if((!this->laser_addr.interf && !this->sonar_addr.interf) ||
     (this->laser_addr.interf && this->sonar_addr.interf))
  {
    PLAYER_ERROR("vfh needs exactly one sonar or one laser");
    this->SetError(-1);
    return;
  }

  // Laser settings
  //TODO this->laser_max_samples = cf->ReadInt(section, "laser_max_samples", 10);
  
  // FIXME
  // Allocate and intialize
  vfh_Algorithm->Init();




  return;
}


///////////////////////////////////////////////////////////////////////////
// VFH Code


VFH_Class::~VFH_Class()
{
  delete this->vfh_Algorithm;

  return;
}

// computes the signed minimum difference between the two angles.  inputs
// and return values are in degrees.
double
VFH_Class::angle_diff(double a, double b)
{
  double ra, rb;
  double d1, d2;

  ra = NORMALIZE(DTOR(a));
  rb = NORMALIZE(DTOR(b));

  d1 = ra-rb;
  d2 = 2*M_PI - fabs(d1);
  if(d1 > 0)
    d2 *= -1.0;

  if(fabs(d1) < fabs(d2))
    return(RTOD(d1));
  else
    return(RTOD(d2));
}

