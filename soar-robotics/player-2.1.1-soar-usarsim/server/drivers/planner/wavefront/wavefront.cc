/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003  Brian Gerkey   gerkey@robotics.stanford.edu
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
/** @defgroup driver_wavefront wavefront
 * @brief Wavefront-propagation path-planner

The wavefront driver implements a global path planner for a planar
mobile robot.

This driver works in the following way: upon receiving a new @ref
interface_planner target, a path is planned from the robot's
current pose, as reported by the underlying @ref interface_localize
device.  The waypoints in this path are handed down, in sequence,
to the underlying @ref interface_position2d device, which should
be capable of local navigation (the @ref driver_vfh driver is a
great candidate). By tying everything together in this way, this driver
offers the mythical "global goto" for your robot.

The planner first creates a configuration space of grid cells from the
map that is given, treating both occupied and unknown cells as occupied.
The planner assigns a cost to each of the free cells based on their
distance to the nearest obstacle. The nearer the obstacle, the higher
the cost. Beyond the max_radius given by the user, the cost in the
c-space cells is zero.

When the planner is given a new goal, it finds a path by working its
way outwards from the goal cell, assigning plan costs to the cells as
it expands (like a wavefront expanding outwards in water). The plan
cost in each cell is dependant on its distance from the goal, as well
as the obstacle cost assigned in the configuration space step. Once the
plan costs for all the cells have been evaluated, the robot can simply
follow the gradient of each lowest adjacent cell all the way to the goal.

In order to function effectively with an underlying obstacle avoidance
algorithm, such as Vector Field Histogram (the @ref driver_vfh
driver), the planner only hands off waypoints, not the entire path. The
wavefront planner finds the longest straight-line distances that don't
cross obstacles between cells that are on the path. The endpoints of
these straight lines become sequential goal locations for the underlying
device driving the robot.

For help in using this driver, try the @ref util_playernav utility.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_planner

@par Requires

This driver controls two named position2d devices: one for input and one
for output.  That way you can read poses from a localization or SLAM system
and send commands directly to the robot.  The input and output devices may
be the same.

- "input" @ref interface_position2d : source of current pose information
  (usually you would use the @ref driver_amcl driver)
- "output" @ref interface_position2d : robot to be controlled;
  this device must be capable of position control (usually you would
  use the @ref driver_vfh driver)
- @ref interface_map : the map to plan paths in

@par Configuration requests

- PLAYER_PLANNER_REQ_GET_WAYPOINTS

@par Configuration file options

Note that the various thresholds should be set to GREATER than the
underlying position device; otherwise the planner could wait indefinitely
for the position device to achieve a target, when the position device
thinks it has already achieved it.

- safety_dist (length)
  - Default: 0.25 m
  - Don't plan a path any closer than this distance to any obstacle.
    Set this to be GREATER than the corresponding threshold of
    the underlying position device!
- max_radius (length)
  - Default: 1.0 m
  - For planning purposes, all cells that are at least this far from
    any obstacle are equally good (save CPU cycles).
- dist_penalty (float)
  - Default: 1.0
  - Extra cost to discourage cutting corners
- distance_epsilon (length)
  - Default: 0.5 m
  - Planar distance from the target position that will be considered
    acceptable.
    Set this to be GREATER than the corresponding threshold of
    the underlying position device!
- angle_epsilon (angle)
  - Default: 10 deg
  - Angular difference from target angle that will considered acceptable.
    Set this to be GREATER than the corresponding threshold of the
    underlying position device!
- replan_dist_thresh (length)
  - Default: 2.0 m
  - Change in robot's position (in localization space) that will
    trigger replanning.  Set to -1 for no replanning (i.e, make
    a plan one time and then stick with it until the goal is reached).
    Replanning is pretty cheap computationally and can really help in
    dynamic environments.  Note that no changes are made to the map in
    order to replan; support is forthcoming for explicitly replanning
    around obstacles that were not in the map.  See also replan_min_time.
- replan_min_time (float)
  - Default: 2.0
  - Minimum time in seconds between replanning.  Set to -1 for no
    replanning.  See also replan_dist_thresh;
- cspace_file (filename)
  - Default: "player.cspace"
  - Use this file to cache the configuration space (c-space) data.
    At startup, if this file can be read and if the metadata (e.g., size,
    scale) in it matches the current map, then the c-space data is
    read from the file.  Otherwise, the c-space data is computed.
    In either case, the c-space data will be cached to this file for
    use next time.  C-space computation can be expensive and so caching
    can save a lot of time, especially when the planner is frequently
    stopped and started.  This feature requires md5 hashing functions
    in libcrypto.
- add_rotational_waypoints (integer)
  - Default: 1
  - If non-zero, add an in-place rotational waypoint before the next
    waypoint if the difference between the robot's current heading and the
    heading to the next waypoint is greater than 45 degrees.  Generally
    helps the low-level position controller, but sacrifices speed.
- force_map_refresh (integer)
  - Default: 0
  - If non-zero, map is updated from subscribed map device whenever
    new goal is set

@par Example

This example shows how to use the wavefront driver to plan and execute paths
on a laser-equipped Pioneer.

@verbatim
driver
(
  name "p2os"
  provides ["odometry:::position2d:0"]
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
  name "mapfile"
  provides ["map:0"]
  filename "mymap.pgm"
  resolution 0.1
)
driver
(
  name "amcl"
  provides ["position2d:2"]
  requires ["odometry:::position2d:1" "laser:0" "laser:::map:0"]
)
driver
(
  name "vfh"
  provides ["position2d:1"]
  requires ["position2d:0" "laser:0"]
  safety_dist 0.1
  distance_epsilon 0.3
  angle_epsilon 5
)
driver
(
  name "wavefront"
  provides ["planner:0"]
  requires ["output:::position2d:1" "input:::position2d:2" "map:0"]
  safety_dist 0.15
  distance_epsilon 0.5
  angle_epsilon 10
)
@endverbatim

@author Brian Gerkey, Andrew Howard
*/
/** @} */

// TODO:
//
//  - allow for computing a path, without actually executing it.
//
//  - compute and return path length

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

#include <libplayercore/playercore.h>
#include "plan.h"

// TODO: monitor localize timestamps, and slow or stop robot accordingly

// time to sleep between loops (us)
#define CYCLE_TIME_US 100000

class Wavefront : public Driver
{
  private:
    // Main function for device thread.
    virtual void Main();

    // bookkeeping
    player_devaddr_t position_id;
    player_devaddr_t localize_id;
    player_devaddr_t map_id;
    double map_res;
    double robot_radius;
    double safety_dist;
    double max_radius;
    double dist_penalty;
    double dist_eps;
    double ang_eps;
    const char* cspace_fname;

    // the plan object
    plan_t* plan;

    // pointers to the underlying devices
    Device* position;
    Device* localize;
    Device* mapdevice;

    // are we disabled?
    bool enable;
    // current target (m,m,rad)
    double target_x, target_y, target_a;
    // index of current waypoint;
    int curr_waypoint;
    // current waypoint (m,m,rad)
    double waypoint_x, waypoint_y, waypoint_a;
    // current waypoint, in odometric coords (m,m,rad)
    double waypoint_odom_x, waypoint_odom_y, waypoint_odom_a;
    // are we pursuing a new goal?
    bool new_goal;
    // current odom pose
    double position_x, position_y, position_a;
    // current list of waypoints
    double (*waypoints)[2];
    int waypoint_count;
    int waypoints_allocated;
    // current localize pose
    double localize_x, localize_y, localize_a;
    // have we told the underlying position device to stop?
    bool stopped;
    // have we reached the goal (used to decide whether or not to replan)?
    bool atgoal;
    // replan each time the robot's localization position changes by at
    // least this much (meters)
    double replan_dist_thresh;
    // leave at least this much time (seconds) between replanning cycles
    double replan_min_time;
    // should we request the map at startup? (or wait for it to be pushed
    // to us as data?)
    bool request_map;
    // Do we have a map yet?
    bool have_map;
    // Has the map changed since last time we planned?
    bool new_map;
    // Is there a new map available (which we haven't retrieved yet)?
    bool new_map_available;
    // Do we consider inserting a rotational waypoint between every pair of
    // waypoints, or just before the first one?
    bool always_insert_rotational_waypoints;
    // Should map be updated on every new goal?
    int force_map_refresh;
    // Should we do velocity control, or position control?
    bool velocity_control;

    // methods for internal use
    int SetupLocalize();
    int SetupPosition();
    int SetupMap();
    int GetMap(bool threaded);
    int GetMapInfo(bool threaded);
    int ShutdownPosition();
    int ShutdownLocalize();
    int ShutdownMap();
    double angle_diff(double a, double b);

    void ProcessCommand(player_planner_cmd_t* cmd);
    void ProcessLocalizeData(player_position2d_data_t* data);
    void ProcessPositionData(player_position2d_data_t* data);
    void ProcessMapInfo(player_map_info_t* info);
    void PutPositionCommand(double x, double y, double a, unsigned char type);
    void PutPlannerData();
    void StopPosition();
    void LocalizeToPosition(double* px, double* py, double* pa,
                            double lx, double ly, double la);
    void SetWaypoint(double wx, double wy, double wa);

  public:
    // Constructor
    Wavefront( ConfigFile* cf, int section);

    // Setup/shutdown routines.
    virtual int Setup();
    virtual int Shutdown();

    // Process incoming messages from clients
    virtual int ProcessMessage(QueuePointer & resp_queue,
                               player_msghdr * hdr,
                               void * data);
};


// Initialization function
Driver* Wavefront_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new Wavefront( cf, section)));
}


// a driver registration function
void Wavefront_Register(DriverTable* table)
{
  table->AddDriver("wavefront",  Wavefront_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
Wavefront::Wavefront( ConfigFile* cf, int section)
  : Driver(cf, section, true,
           PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_PLANNER_CODE)
{
  // Must have a position device to control
  if (cf->ReadDeviceAddr(&this->position_id, section, "requires",
                         PLAYER_POSITION2D_CODE, -1, "output") != 0)
  {
    this->SetError(-1);
    return;
  }
  // Must have a position device from which to read global poses
  if (cf->ReadDeviceAddr(&this->localize_id, section, "requires",
                         PLAYER_POSITION2D_CODE, -1, "input") != 0)
  {
    this->SetError(-1);
    return;
  }
  // Must have a map device
  if (cf->ReadDeviceAddr(&this->map_id, section, "requires",
                         PLAYER_MAP_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  this->safety_dist = cf->ReadLength(section,"safety_dist", 0.25);
  this->max_radius = cf->ReadLength(section,"max_radius",1.0);
  this->dist_penalty = cf->ReadFloat(section,"dist_penalty",1.0);
  this->dist_eps = cf->ReadLength(section,"distance_epsilon", 0.5);
  this->ang_eps = cf->ReadAngle(section,"angle_epsilon",DTOR(10));
  this->replan_dist_thresh = cf->ReadLength(section,"replan_dist_thresh",2.0);
  this->replan_min_time = cf->ReadFloat(section,"replan_min_time",2.0);
  this->request_map = cf->ReadInt(section,"request_map",1);
  this->cspace_fname = cf->ReadFilename(section,"cspace_file","player.cspace");
  this->always_insert_rotational_waypoints =
          cf->ReadInt(section, "add_rotational_waypoints", 1);
  this->force_map_refresh = cf->ReadInt(section, "force_map_refresh", 0);
  this->velocity_control = cf->ReadInt(section, "velocity_control", 0);
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int
Wavefront::Setup()
{
  this->have_map = false;
  this->new_map = false;
  this->new_map_available = false;
  this->stopped = true;
  this->atgoal = true;
  this->enable = true;
  this->target_x = this->target_y = this->target_a = 0.0;
  this->position_x = this->position_y = this->position_a = 0.0;
  this->localize_x = this->localize_y = this->localize_a = 0.0;
  this->waypoint_x = this->waypoint_y = this->waypoint_a = 0.0;
  this->waypoint_odom_x = this->waypoint_odom_y = this->waypoint_odom_a = 0.0;
  this->curr_waypoint = -1;

  this->new_goal = false;

  this->waypoint_count = 0;
  this->waypoints_allocated = 8;
  this->waypoints = (double (*)[2])malloc(this->waypoints_allocated * 
                                          sizeof(this->waypoints[0]));

  if(SetupPosition() < 0)
    return(-1);

  if(!(this->plan = plan_alloc(this->robot_radius+this->safety_dist,
                               this->robot_radius+this->safety_dist,
                               this->max_radius,
                               this->dist_penalty)))
  {
    PLAYER_ERROR("failed to allocate plan");
    return(-1);
  }

  if(SetupMap() < 0)
    return(-1);
  if(SetupLocalize() < 0)
    return(-1);

  // Start the driver thread.
  this->StartThread();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int
Wavefront::Shutdown()
{
  // Stop the driver thread.
  this->StopThread();

  if(this->plan)
    plan_free(this->plan);
  free(this->waypoints);
  this->waypoints = NULL;

  ShutdownPosition();
  ShutdownLocalize();
  ShutdownMap();

  return 0;
}

void
Wavefront::ProcessCommand(player_planner_cmd_t* cmd)
{
  double new_x, new_y, new_a;
  //double eps = 1e-3;

  new_x = cmd->goal.px;
  new_y = cmd->goal.py;
  new_a = cmd->goal.pa;

#if 0
  if((fabs(new_x - this->target_x) > eps) ||
     (fabs(new_y - this->target_y) > eps) ||
     (fabs(this->angle_diff(new_a,this->target_a)) > eps))
  {
#endif
    this->target_x = new_x;
    this->target_y = new_y;
    this->target_a = new_a;
    printf("new goal: %f, %f, %f\n", target_x, target_y, target_a);
    this->new_goal = true;
    this->atgoal = false;
#if 0
  }
#endif
}

void
Wavefront::ProcessLocalizeData(player_position2d_data_t* data)
{
  this->localize_x = data->pos.px;
  this->localize_y = data->pos.py;
  this->localize_a = data->pos.pa;
}

void
Wavefront::ProcessPositionData(player_position2d_data_t* data)
{
  this->position_x = data->pos.px;
  this->position_y = data->pos.py;
  this->position_a = data->pos.pa;
}

void
Wavefront::ProcessMapInfo(player_map_info_t* info)
{
  // Got new map info pushed to us.  We'll save this info and get the new
  // map.
  this->plan->scale = info->scale;
  this->plan->size_x = info->width;
  this->plan->size_y = info->height;
  this->plan->origin_x = info->origin.px;
  this->plan->origin_y = info->origin.py;

  // Now get the map data, possibly in separate tiles.
  if(this->GetMap(true) < 0)
  {
    this->have_map = false;
    this->StopPosition();
  }
  else
  {
    this->have_map = true;
    this->new_map = true;
    // force replanning
    if(this->curr_waypoint >= 0)
      this->new_goal = true;
  }
}

void
Wavefront::PutPlannerData()
{
  player_planner_data_t data;

  memset(&data,0,sizeof(data));

  if(this->waypoint_count > 0)
    data.valid = 1;
  else
    data.valid = 0;

  if((this->waypoint_count > 0) && (this->curr_waypoint < 0))
    data.done = 1;
  else
    data.done = 0;

  // put the current localize pose
  data.pos.px = this->localize_x;
  data.pos.py = this->localize_y;
  data.pos.pa = this->localize_a;

  data.goal.px = this->target_x;
  data.goal.py = this->target_y;
  data.goal.pa = this->target_a;

  if(data.valid && !data.done)
  {
    data.waypoint.px = this->waypoint_x;
    data.waypoint.py = this->waypoint_y;
    data.waypoint.pa = this->waypoint_a;

    data.waypoint_idx = this->curr_waypoint;
    data.waypoints_count = this->waypoint_count;
  }

  this->Publish(this->device_addr, 
                PLAYER_MSGTYPE_DATA,
                PLAYER_PLANNER_DATA_STATE,
                (void*)&data,sizeof(data),NULL);
}

void
Wavefront::PutPositionCommand(double x, double y, double a, unsigned char type)
{
  player_position2d_cmd_vel_t vel_cmd;
  player_position2d_cmd_pos_t pos_cmd;

  memset(&vel_cmd,0,sizeof(vel_cmd));
  memset(&pos_cmd,0,sizeof(pos_cmd));

  if(type)
  {
    // position control
    pos_cmd.pos.px = x;
    pos_cmd.pos.py = y;
    pos_cmd.pos.pa = a;
    pos_cmd.state=1;
    this->position->PutMsg(this->InQueue,
                         PLAYER_MSGTYPE_CMD,
                         PLAYER_POSITION2D_CMD_POS,
                         (void*)&pos_cmd,sizeof(pos_cmd),NULL);
  }
  else
  {
    // velocity control (used to stop the robot)
    vel_cmd.vel.px = x;
    vel_cmd.vel.py = y;
    vel_cmd.vel.pa = a;
    vel_cmd.state=1;
    this->position->PutMsg(this->InQueue,
                         PLAYER_MSGTYPE_CMD,
                         PLAYER_POSITION2D_CMD_VEL,
                         (void*)&vel_cmd,sizeof(vel_cmd),NULL);
  }

  this->stopped = false;
}

void
Wavefront::LocalizeToPosition(double* px, double* py, double* pa,
                              double lx, double ly, double la)
{
  double offset_x, offset_y, offset_a;
  double lx_rot, ly_rot;

  offset_a = this->angle_diff(this->position_a,this->localize_a);
  lx_rot = this->localize_x * cos(offset_a) - this->localize_y * sin(offset_a);
  ly_rot = this->localize_x * sin(offset_a) + this->localize_y * cos(offset_a);

  offset_x = this->position_x - lx_rot;
  offset_y = this->position_y - ly_rot;

  //printf("offset: %f, %f, %f\n", offset_x, offset_y, RTOD(offset_a));

  *px = lx * cos(offset_a) - ly * sin(offset_a) + offset_x;
  *py = lx * sin(offset_a) + ly * cos(offset_a) + offset_y;
  *pa = la + offset_a;
}

void
Wavefront::StopPosition()
{
  if(!this->stopped)
  {
    //puts("stopping robot");
    PutPositionCommand(0.0,0.0,0.0,0);
    this->stopped = true;
  }
}

void
Wavefront::SetWaypoint(double wx, double wy, double wa)
{
  double wx_odom, wy_odom, wa_odom;

  // transform to odometric frame
  LocalizeToPosition(&wx_odom, &wy_odom, &wa_odom, wx, wy, wa);

  // hand down waypoint
  //printf("sending waypoint: %.3f %.3f %.3f\n",
         //wx_odom, wy_odom, RTOD(wa_odom));
  PutPositionCommand(wx_odom, wy_odom, wa_odom,1);

  // cache this waypoint, odometric coords
  this->waypoint_odom_x = wx_odom;
  this->waypoint_odom_y = wy_odom;
  this->waypoint_odom_a = wa_odom;
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Wavefront::Main()
{
  double dist, angle;
  double t;
  double last_replan_lx=0.0, last_replan_ly=0.0;
  double last_replan_time = 0.0;
  double last_publish_time = 0.0;
  double replan_timediff, replan_dist;
  bool rotate_waypoint=false;
  bool replan;
  int rotate_dir=0;

  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

  // block until we get initial data from underlying devices
  // TODO
  //this->position->Wait();
  this->StopPosition();

  for(;;)
  {
    pthread_testcancel();

    ProcessMessages();

    if(!this->have_map && !this->new_map_available)
    {
      usleep(CYCLE_TIME_US);
      continue;
    }

    GlobalTime->GetTimeDouble(&t);

    if((t - last_publish_time) > 0.25)
    {
      last_publish_time = t;
      this->PutPlannerData();
    }

    // Is it time to replan?
    replan_timediff = t - last_replan_time;
    replan_dist = sqrt(((this->localize_x - last_replan_lx) *
                        (this->localize_x - last_replan_lx)) +
                       ((this->localize_y - last_replan_ly) *
                        (this->localize_y - last_replan_ly)));
    replan = (this->replan_dist_thresh >= 0.0) &&
            (replan_dist > this->replan_dist_thresh) &&
            (this->replan_min_time >= 0.0) &&
            (replan_timediff > this->replan_min_time) &&
            !this->atgoal;

    // Did we get a new goal, or is it time to replan?
    if(this->new_goal || replan || (this->velocity_control && !this->atgoal))
    {
#if 0
      // Should we get a new map?
      if(this->new_map_available)
      {
        this->new_map_available = false;

        if(this->GetMapInfo(true) < 0)
          PLAYER_WARN("failed to get new map info");
        else
        {
          if(this->GetMap(true) < 0)
            PLAYER_WARN("failed to get new map data");
          else
          {
            this->new_map = true;
            this->have_map = true;
          }
        }
      }

      // We need to recompute the C-space if the map changed, or if the
      // goal or robot pose lie outside the bounds of the area we last
      // searched.
      if(this->new_map ||
         !plan_check_inbounds(plan,this->localize_x,this->localize_y) ||
          !plan_check_inbounds(plan,this->target_x,this->target_y))
      {
        // Unfortunately, this computation can take a while (e.g., 1-2
        // seconds).  So we'll stop the robot while it thinks.
        this->StopPosition();

        // Set the bounds to search only an axis-aligned bounding box
        // around the robot and the goal.
        plan_set_bbox(this->plan, 1.0, 3.0,
                      this->localize_x, this->localize_y,
                      this->target_x, this->target_y);

        struct timeval t0, t1;
        gettimeofday(&t0, NULL);
        plan_update_cspace(this->plan,this->cspace_fname);
        gettimeofday(&t1, NULL);
        printf("time to update: %f\n",
               (t1.tv_sec + t1.tv_usec/1e6) -
               (t0.tv_sec + t0.tv_usec/1e6));
        this->new_map = false;
      }
#endif

      // compute costs to the new goal.  Try local plan first
      if(new_goal ||
         (this->plan->path_count == 0) ||
         (plan_do_local(this->plan, this->localize_x, 
                         this->localize_y, 5.0) < 0))
      {
        if(!new_goal && (this->plan->path_count != 0))
           puts("Wavefront: local plan failed");

        // Create a global plan
        if(plan_do_global(this->plan, this->localize_x, this->localize_y, 
                          this->target_x, this->target_y) < 0)
          puts("Wavefront: global plan failed");
        else
          this->new_goal = false;
      }

      if(!this->velocity_control)
      {
        // extract waypoints along the path to the goal from the current position
        plan_update_waypoints(this->plan, this->localize_x, this->localize_y);

        if(this->plan->waypoint_count == 0)
        {
          fprintf(stderr, "Wavefront (port %d):\n  "
                  "No path from (%.3lf,%.3lf,%.3lf) to (%.3lf,%.3lf,%.3lf)\n",
                  this->device_addr.robot,
                  this->localize_x,
                  this->localize_y,
                  RTOD(this->localize_a),
                  this->target_x,
                  this->target_y,
                  RTOD(this->target_a));
          // Only fail here if this is our first try at making a plan;
          // if we're replanning and don't find a path then we'll just stick
          // with the old plan.
          if(this->curr_waypoint < 0)
          {
            //this->curr_waypoint = -1;
            this->new_goal=false;
            this->waypoint_count = 0;
          }
        }
        else
        {
          if (this->plan->waypoint_count > this->waypoints_allocated)
          {
            this->waypoints = (double (*)[2])realloc(this->waypoints, sizeof(this->waypoints[0])*this->plan->waypoint_count);
            this->waypoints_allocated = this->plan->waypoint_count;
          }
          this->waypoint_count = this->plan->waypoint_count;
        }

        if(this->plan->waypoint_count > 0)
        {
          for(int i=0;i<this->plan->waypoint_count;i++)
          {
            double wx, wy;
            plan_convert_waypoint(this->plan,
                                  this->plan->waypoints[i],
                                  &wx, &wy);
            this->waypoints[i][0] = wx;
            this->waypoints[i][1] = wy;
          }

          this->curr_waypoint = 0;
          // Why is this here?
          this->new_goal = true;
        }
        last_replan_time = t;
        last_replan_lx = this->localize_x;
        last_replan_ly = this->localize_y;
      }
    }

    if(!this->enable)
    {
      this->StopPosition();
      usleep(CYCLE_TIME_US);
      continue;
    }


    if(this->velocity_control)
    {
      if(this->plan->path_count && !this->atgoal)
      {
        // Check doneness
        dist = sqrt(((this->localize_x - this->target_x) *
                     (this->localize_x - this->target_x)) +
                    ((this->localize_y - this->target_y) *
                     (this->localize_y - this->target_y)));
        angle = fabs(this->angle_diff(this->target_a,this->localize_a));
        if((dist < this->dist_eps) && (angle < this->ang_eps))
        {
          this->StopPosition();
          this->new_goal = false;
          this->curr_waypoint = -1;
          this->atgoal = true;
        }
        else
        {
          // Compute velocities
          double wx, wy;
          double maxd=2.0;
          double distweight=10.0;

          if(plan_get_carrot(this->plan, &wx, &wy,
                             this->localize_x, this->localize_y,
                             maxd, distweight) < 0)
          {
            puts("Failed to find a carrot");
            this->StopPosition();
          }
          else
          {
            // Establish fake waypoints, for client-side visualization
            this->curr_waypoint = 0;
            this->waypoint_count = 2;
            this->waypoints[0][0] = this->localize_x;
            this->waypoints[0][1] = this->localize_y;
            this->waypoint_x = this->waypoints[1][0] = wx;
            this->waypoint_y = this->waypoints[1][1] = wy;
            this->waypoint_a = 0.0;

            // TODO: expose these control params in the .cfg file
            double tvmin = 0.1;
            double tvmax = 0.5;
            double avmin = DTOR(10.0);
            double avmax = DTOR(45.0);
            double amin = DTOR(5.0);
            double amax = DTOR(20.0);

            double d = sqrt((this->localize_x-wx)*(this->localize_x-wx) +
                            (this->localize_y-wy)*(this->localize_y-wy));
            double b = atan2(wy - this->localize_y, wx - this->localize_x);

            double av,tv;
            double a = amin + (d / maxd) * (amax-amin);
            double ad = angle_diff(b, this->localize_a);

            // Are we on top of the goal?
            if(d < this->dist_eps)
            {
              if(!rotate_dir)
              {
                if(ad < 0)
                  rotate_dir = -1;
                else
                  rotate_dir = 1;
              }

              tv = 0.0;
              av = rotate_dir * (avmin + (fabs(ad)/M_PI) * (avmax-avmin));
            }
            else
            {
              rotate_dir = 0;

              if(fabs(ad) > a)
                tv = 0.0;
              else
                tv = tvmin + (d / (M_SQRT2 * maxd)) * (tvmax-tvmin);

              av = avmin + (fabs(ad)/M_PI) * (avmax-avmin);
              if(ad < 0)
                av = -av;
            }

            this->PutPositionCommand(tv,0.0,av,0);
          }
        }
      }
      else
        this->StopPosition();
    }
    else // !velocity_control
    {
      bool going_for_target = (this->curr_waypoint == this->plan->waypoint_count);
      dist = sqrt(((this->localize_x - this->target_x) *
                   (this->localize_x - this->target_x)) +
                  ((this->localize_y - this->target_y) *
                   (this->localize_y - this->target_y)));
      // Note that we compare the current heading and waypoint heading in the
      // *odometric* frame.   We do this because comparing the current
      // heading and waypoint heading in the localization frame is unreliable
      // when making small adjustments to achieve a desired heading (i.e., the
      // robot gets there and VFH stops, but here we don't realize we're done
      // because the localization heading hasn't changed sufficiently).
      angle = fabs(this->angle_diff(this->waypoint_odom_a,this->position_a));
      if(going_for_target && dist < this->dist_eps && angle < this->ang_eps)
      {
        // we're at the final target, so stop
        StopPosition();
        this->curr_waypoint = -1;
        this->new_goal = false;
        this->atgoal = true;
      }
      else if(this->curr_waypoint < 0)
      {
        // no more waypoints, so stop
        StopPosition();
      }
      else
      {
        // are we there yet?  ignore angle, cause this is just a waypoint
        dist = sqrt(((this->localize_x - this->waypoint_x) *
                     (this->localize_x - this->waypoint_x)) +
                    ((this->localize_y - this->waypoint_y) *
                     (this->localize_y - this->waypoint_y)));
        // Note that we compare the current heading and waypoint heading in the
        // *odometric* frame.   We do this because comparing the current
        // heading and waypoint heading in the localization frame is unreliable
        // when making small adjustments to achieve a desired heading (i.e., the
        // robot gets there and VFH stops, but here we don't realize we're done
        // because the localization heading hasn't changed sufficiently).
        if(this->new_goal ||
           (rotate_waypoint &&
            (fabs(this->angle_diff(this->waypoint_odom_a,this->position_a))
             < M_PI/4.0)) ||
           (!rotate_waypoint && (dist < this->dist_eps)))
        {
          if(this->curr_waypoint == this->waypoint_count)
          {
            // no more waypoints, so wait for target achievement

            //puts("waiting for goal achievement");
            usleep(CYCLE_TIME_US);
            continue;
          }
          // get next waypoint
          this->waypoint_x = this->waypoints[this->curr_waypoint][0];
          this->waypoint_y = this->waypoints[this->curr_waypoint][1];
          this->curr_waypoint++;

          this->waypoint_a = this->target_a;
          if(this->always_insert_rotational_waypoints ||
             (this->curr_waypoint == 2))
          {
            dist = sqrt((this->waypoint_x - this->localize_x) *
                        (this->waypoint_x - this->localize_x) +
                        (this->waypoint_y - this->localize_y) *
                        (this->waypoint_y - this->localize_y));
            angle = atan2(this->waypoint_y - this->localize_y,
                          this->waypoint_x - this->localize_x);
            if((dist > this->dist_eps) &&
               fabs(this->angle_diff(angle,this->localize_a)) > M_PI/4.0)
            {
              this->waypoint_x = this->localize_x;
              this->waypoint_y = this->localize_y;
              this->waypoint_a = angle;
              this->curr_waypoint--;
              rotate_waypoint=true;
            }
            else
              rotate_waypoint=false;
          }
          else
            rotate_waypoint=false;

          this->new_goal = false;
        }

        SetWaypoint(this->waypoint_x, this->waypoint_y, this->waypoint_a);
      }
    }

    usleep(CYCLE_TIME_US);
  }
}


////////////////////////////////////////////////////////////////////////////////
// Set up the underlying position device.
int
Wavefront::SetupPosition()
{
  player_position2d_geom_t* geom;
  player_position2d_power_config_t motorconfig;

  // Subscribe to the position device.
  if(!(this->position = deviceTable->GetDevice(this->position_id)))
  {
    PLAYER_ERROR("unable to locate suitable position device");
    return(-1);
  }
  if(this->position->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to position device");
    return(-1);
  }

  Message* msg;
  // Enable the motors
  motorconfig.state = 1;
  if(!(msg = this->position->Request(this->InQueue,
                                     PLAYER_MSGTYPE_REQ,
                                     PLAYER_POSITION2D_REQ_MOTOR_POWER,
                                     (void*)&motorconfig,
                                     sizeof(motorconfig), NULL, false)))
  {
    PLAYER_WARN("failed to enable motors");
  }
  else
    delete msg;

  // Get the robot's geometry
  if(!(msg = this->position->Request(this->InQueue,
                                     PLAYER_MSGTYPE_REQ,
                                     PLAYER_POSITION2D_REQ_GET_GEOM,
                                     NULL, 0, NULL, false)) ||
     (msg->GetHeader()->size != sizeof(player_position2d_geom_t)))
  {
    PLAYER_ERROR("failed to get geometry of underlying position device");
    if(msg)
      delete msg;
    return(-1);
  }

  geom = (player_position2d_geom_t*)msg->GetPayload();

  // take the bigger of the two dimensions, convert to meters, and halve
  // to get a radius
  this->robot_radius = MAX(geom->size.sl, geom->size.sw);
  this->robot_radius /= 2.0;

  delete msg;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the underlying localize device.
int
Wavefront::SetupLocalize()
{
  // Subscribe to the localize device.
  if(!(this->localize = deviceTable->GetDevice(this->localize_id)))
  {
    PLAYER_ERROR("unable to locate suitable localize device");
    return(-1);
  }
  if(this->localize->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to localize device");
    return(-1);
  }

  return(0);
}

// Retrieve the map data in tiles, assuming that the map info is already
// stored in this->plan.
int
Wavefront::GetMap(bool threaded)
{
  // allocate space for map cells
  this->plan->cells = (plan_cell_t*)realloc(this->plan->cells,
                                            (this->plan->size_x *
                                             this->plan->size_y *
                                             sizeof(plan_cell_t)));
  assert(this->plan->cells);

  // Reset the grid
  plan_reset(this->plan);

  // now, get the map data
  player_map_data_t data_req;
  memset(&data_req,0,sizeof(player_map_data_t));
  int i,j;
  int oi,oj;
  int sx,sy;
  int si,sj;

  // Grab 640x640 tiles
  sy = sx = 640;
  oi=oj=0;
  while((oi < this->plan->size_x) && (oj < this->plan->size_y))
  {
    si = MIN(sx, this->plan->size_x - oi);
    sj = MIN(sy, this->plan->size_y - oj);

    data_req.col = oi;
    data_req.row = oj;
    data_req.width = si;
    data_req.height = sj;

    Message* msg;
    if(!(msg = this->mapdevice->Request(this->InQueue,
                                        PLAYER_MSGTYPE_REQ,
                                        PLAYER_MAP_REQ_GET_DATA,
                                        (void*)&data_req,0,NULL,
                                        threaded)))
    {
      PLAYER_ERROR("failed to get map data");
      // dont free plan->cells this here as it is realloced above and free'd on shutdown
      //free(this->plan->cells);
      return(-1);
    }

    player_map_data_t* mapdata = (player_map_data_t*)msg->GetPayload();
    plan_cell_t* cell;

    // copy the map data
    for(j=0;j<sj;j++)
    {
      for(i=0;i<si;i++)
      {
        cell = this->plan->cells + PLAN_INDEX(this->plan,oi+i,oj+j);
        cell->occ_dist = this->plan->max_radius;
        if((cell->occ_state = mapdata->data[j*si + i]) >= 0)
          cell->occ_dist = 0;
      }
    }

    delete msg;

    oi += si;
    if(oi >= this->plan->size_x)
    {
      oi = 0;
      oj += sj;
    }
  }

  plan_init(this->plan);
  plan_compute_cspace(this->plan);

  return(0);
}

int
Wavefront::GetMapInfo(bool threaded)
{
  Message* msg;
  if(!(msg = this->mapdevice->Request(this->InQueue,
                                      PLAYER_MSGTYPE_REQ,
                                      PLAYER_MAP_REQ_GET_INFO,
                                      NULL, 0, NULL, threaded)))
  {
    PLAYER_WARN("failed to get map info");
    this->plan->scale = 0.1;
    this->plan->size_x = 0;
    this->plan->size_y = 0;
    this->plan->origin_x = 0.0;
    this->plan->origin_y = 0.0;
    return(-1);
  }

  player_map_info_t* info = (player_map_info_t*)msg->GetPayload();

  // copy in the map info
  this->plan->scale = info->scale;
  this->plan->size_x = info->width;
  this->plan->size_y = info->height;
  this->plan->origin_x = info->origin.px;
  this->plan->origin_y = info->origin.py;

  delete msg;
  return(0);
}

// setup the underlying map device (i.e., get the map)
int
Wavefront::SetupMap()
{
  // Subscribe to the map device
  if(!(this->mapdevice = deviceTable->GetDevice(this->map_id)))
  {
    PLAYER_ERROR("unable to locate suitable map device");
    return(-1);
  }
  if(mapdevice->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to map device");
    return(-1);
  }

  // should we get the map now?  if not, we'll wait for it to be pushed to
  // us as data later.
  if(!this->request_map)
    return(0);

  printf("Wavefront: Loading map from map:%d...\n", this->map_id.index);
  fflush(stdout);

  // Fill in the map structure

  // first, get the map info
  if(this->GetMapInfo(false) < 0)
    return(-1);
  // Now get the map data, possibly in separate tiles.
  if(this->GetMap(false) < 0)
    return(-1);

  this->have_map = true;
  this->new_map = true;

  puts("Done.");

  return(0);
}

int
Wavefront::ShutdownPosition()
{
  return(this->position->Unsubscribe(this->InQueue));
}

int
Wavefront::ShutdownLocalize()
{
  return(this->localize->Unsubscribe(this->InQueue));
}

int
Wavefront::ShutdownMap()
{
  return(this->mapdevice->Unsubscribe(this->InQueue));
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int
Wavefront::ProcessMessage(QueuePointer & resp_queue,
                          player_msghdr * hdr,
                          void * data)
{
  // Is it new odometry data?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                           PLAYER_POSITION2D_DATA_STATE,
                           this->position_id))
  {
    this->ProcessPositionData((player_position2d_data_t*)data);

    // In case localize_id and position_id are the same
    if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                             PLAYER_POSITION2D_DATA_STATE,
                             this->localize_id))
      this->ProcessLocalizeData((player_position2d_data_t*)data);
    return(0);
  }
  // Is it new localization data?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                                PLAYER_POSITION2D_DATA_STATE,
                                this->localize_id))
  {
    this->ProcessLocalizeData((player_position2d_data_t*)data);
    return(0);
  }
  // Is it a new goal for the planner?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                                PLAYER_PLANNER_CMD_GOAL,
                                this->device_addr))
  {
    if (this->force_map_refresh)
    {
      PLAYER_WARN("requesting new map");

      if (this->plan) plan_free(this->plan);
      this->plan = plan_alloc(this->robot_radius+this->safety_dist,
                              this->robot_radius+this->safety_dist,
                              this->max_radius,
                              this->dist_penalty);
      assert(this->plan);

      // Fill in the map structure

      // first, get the map info
      if(this->GetMapInfo(true) < 0) return -1;
      // Now get the map data, possibly in separate tiles.
      if(this->GetMap(true) < 0) return -1;

      this->have_map = true;
      this->new_map = true;
    }
    assert(data);
    this->ProcessCommand((player_planner_cmd_t*)data);
    return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                PLAYER_PLANNER_REQ_GET_WAYPOINTS,
                                this->device_addr))
  {
    player_planner_waypoints_req_t reply;

    reply.waypoints_count = this->waypoint_count;
    reply.waypoints = (player_pose2d_t*)calloc(sizeof(reply.waypoints[0]),this->waypoint_count);
    for(int i=0;i<(int)reply.waypoints_count;i++)
    {
      reply.waypoints[i].px = this->waypoints[i][0];
      reply.waypoints[i].py = this->waypoints[i][1];
      reply.waypoints[i].pa = 0.0;
    }

    this->Publish(this->device_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_PLANNER_REQ_GET_WAYPOINTS,
                  (void*)&reply);
    free(reply.waypoints);
    return(0);
  }
  // Is it a request to enable or disable the planner?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                PLAYER_PLANNER_REQ_ENABLE,
                                this->device_addr))
  {
    if(hdr->size != sizeof(player_planner_enable_req_t))
    {
      PLAYER_ERROR("incorrect size for planner enable request");
      return(-1);
    }
    player_planner_enable_req_t* enable_req =
            (player_planner_enable_req_t*)data;

    if(enable_req->state)
    {
      this->enable = true;
      PLAYER_MSG0(2,"Robot enabled");
    }
    else
    {
      this->enable = false;
      PLAYER_MSG0(2,"Robot disabled");
    }
    this->Publish(this->device_addr,
                  resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_PLANNER_REQ_ENABLE);
    return(0);
  }
  // Is it new map metadata?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                                PLAYER_MAP_DATA_INFO,
                                this->map_id))
  {
    if(hdr->size != sizeof(player_map_info_t))
    {
      PLAYER_ERROR("incorrect size for map info");
      return(-1);
    }
    //this->ProcessMapInfo((player_map_info_t*)data);
    this->new_map_available = true;
    return(0);
  }
  else
    return(-1);
}

// computes the signed minimum difference between the two angles.
double
Wavefront::angle_diff(double a, double b)
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
