/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000 Brian Gerkey and others
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
///////////////////////////////////////////////////////////////////////////
//
// Desc: Adaptive Monte-Carlo localization
// Author: Andrew Howard
// Date: 6 Feb 2003
// CVS: $Id: amcl.cc 4293 2007-12-07 02:43:10Z gerkey $
//
// Theory of operation:
//  TODO
//
// Requires: position (odometry)
// Provides: localization
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_amcl amcl
 * @brief Adaptive Monte Carlo localization

The @p amcl driver implements the Adaptive Monte-Carlo
Localization algorithm described by Dieter Fox.

At the conceptual level, the @p amcl driver maintains a probability
distribution over the set of all possible robot poses, and updates this
distribution using data from odometry, sonar and/or laser range-finders.
The driver also requires a pre-defined map of the environment against
which to compare observed sensor values.

At the implementation level, the @p amcl driver represents the probability
distribution using a particle filter.  The filter is "adaptive" because
it dynamically adjusts the number of particles in the filter: when the
robot's pose is highly uncertain, the number of particles is increased;
when the robot's pose is well determined, the number of particles
is decreased.  The driver is therefore able make a trade-off between
processing speed and localization accuracy.

As an example, consider the sequence of images shown below.  This sequence
shows the filter converging from an initial configuration in which the
pose of the robot is entirely unknown to a final configuration in which
the pose of the robot is well determined.  At the same time, the number
of particles in the filter decreases from 100,000 to less than 100.
Convergence in this case is relatively slow.

@image html amcl-phe200-0010.jpg "t = 1 sec, approx 100,000 particles"
@image html amcl-phe200-0400.jpg "t = 40 sec, approx 1,000 particles"
@image html amcl-phe200-0800.jpg "t = 80 sec, approx 100 particles"
@image html amcl-phe200-1200.jpg "t = 120 sec, approx 100 particles"


The @p amcl driver has the the usual features --
and failures -- associated with simple Monte-Carlo Localization
techniques:
  - If the robot's initial pose is specified as being completely
  unknown, the driver's estimate will usually converge to correct
  pose.  This assumes that the particle filter starts with a large
  number of particles (to cover the space of possible poses), and that
  the robot is driven some distance through the environment (to
  collect observations).
  - If the robot's initial pose is specified accurately, but
  incorrectly, or if the robot becomes lost (e.g., by picking it up
  and replacing it elsewhere) the driver's estimate will not converge
  on the correct pose.  Such situations require the use of more
  advanced techniques that have not yet been implemented.

The @p amcl driver also has some slightly unusual temporal
behavior:
  - When the number of particles in the filter is large, data may
  arrive from the sensors faster than it can be processed.  When this
  happens, data is queued up for later processing, but the driver
  continues to generate an up-to-date estimate for the robot pose.
  Thus, for example, at time t = 10 sec, the driver may have only
  processed sensor readings up until time t = 5 sec, but will
  nevertheless generate an estimate (prediction) of where the robot is
  at t = 10 sec.  The adaptive nature of the algorithm more-or-less
  guarantees that the driver will eventual "catch up": as more
  sensor readings are processed, the number of particles will
  generally decrease, and the sensor update step of the algorithm will
  run faster.

@par Caveats

At the time of writing, this driver is still evolving.  The sensor
models, in particular, are currently over-simplified and
under-parameterized (there are lots of magic numbers lurking about the
place).  Consequently, while this driver is known to work for certain
hardware configurations (think Pioneer2DX with a SICKLMS200 laser
range-finder), other configurations may require some refinement of the
sensor models.

@par Provides

- @ref interface_localize : this interface provides a (sort of)
  representative sample of the current pose hypotheses, weighted by likelihood.
- @ref interface_position2d : this interface provides just the
  most-likely hypothesis, formatted as position data, which you can
  (at your peril) pretend came from a perfect odometry system

@par Requires

The @p amcl driver requires the following interfaces, some of them named:

- "odometry" @ref interface_position2d : source of odometry information
- @ref interface_laser : source of laser scans
- "laser" @ref interface_map : a map in which to localize the
   robot, by fusing odometry and laser/sonar data.
- In principle supported, but currently disabled are:
    - @ref interface_fiducial
    - "imu" @ref interface_position2d
    - @ref interface_sonar
    - @ref interface_gps
    - @ref interface_wifi

@par Configuration requests

- TODO

@par Configuration file options

- Particle filter settings:
  - odom_init (integer)
    - Default: 1
    - Use the odometry device as the "action" sensor
  - pf_min_samples (integer)
    - Default: 100
    - Lower bound on the number of samples to maintain in the particle filter.
  - pf_max_samples (integer)
    - Default: 10000
    - Upper bound on the number of samples to maintain in the particle filter.
  - pf_err (float)
    - Default: 0.01
    - Control parameter for the particle set size.  See notes below.
  - pf_z (float)
    - Default: 3
    - Control parameter for the particle set size.  See notes below.
  - init_pose (tuple: [length length angle])
    - Default: [0 0 0] (m m rad)
    - Initial pose estimate (mean value) for the robot.
  - init_pose_var (tuple: [length length angle])
    - Default: [1 1 2pi] (m m rad)
    - Uncertainty in the initial pose estimate.
  - update_thresh (tuple: [length angle])
    - Default: [0.2 pi/6] (m rad)
    - Minimum change required in action sensor to force update in
      particle filter.
  - odom_drift[0-2] (float tuples)
    - Default:
      - odom_drift[0] [0.2 0.0 0.0]
      - odom_drift[1] [0.0 0.2 0.0]
      - odom_drift[2] [0.2 0.0 0.2]
    - Set the 3 rows of the covariance matrix used for odometric drift.
- Laser settings:
  - laser_pose (length tuple)
    - Default: [0 0 0]
    - Pose of the laser sensor in the robot's coordinate system
  - laser_max_beams (integer)
    - Default: 6
    - Maximum number of range readings being used
  - laser_range_max (length)
    - Default: 8.192 m
    - Maximum range returned by laser
  - laser_range_var (length)
    - Default: 0.1 m
    - Variance in range data returned by laser
  - laser_range_bad (float)
    - Default 0.1
    - ???
- Debugging:
  - enable_gui (integer)
    - Default: 0
    - Set this to 1 to enable the built-in driver GUI (useful
      for debugging).  Player must also be built with @p configure
      @p --enable-rtkgui for this option to have any effect.

@par Notes

- Coordinate System:
The origin of the global coordinate system corresponds to the center
of occupancy grid map.  Standard coordinate orientation is used; i.e.,
positive x is towards the right of the map, positive y towards the top
of the map.

- Number of particles:
The number of particles in the filter can be controlled using the
configuration file parameters @p pf_err and @p pf_z.  Specifically, @p
pf_err is the maximum allowed error between the true distribution and
the estimated distribution, while @p pf_z is the upper standard normal
quantile for (1 - p), where p is the probability that the error on the
estimated distribution will be less than @p pf_err.  If you dont know
what that means, dont worry, I'm not exactly sure either.  See Fox's
paper for a more meaningful explanation.

- Speed:
Many factors affect the speed at which the @p amcl driver
runs, but the following tips might be helpful:
  - Reducing the number of laser range readings being used (@p
    laser_max_beams in the configuration file) will significantly increase
    driver speed, but may also lead to slower convergence and/or less
    accurate localization.
  - Increasing the allowed error @p pf_err and reducing the quantile
    @p pf_z will lead to smaller particle sets and will hence increase
    driver speed.  This may also lead, however, to over-convergence.
As a benchmark, this driver has been successfully deployed on a
Pioneer2DX equipped with a SICK LMS200 and a 266MHz Mobile Pentium
with 32Mb of RAM.

- Memory:
The two key factors affecting memory usage are:
  - The size and resolution of the map.
  - The maximum number of particles.
As currently configured, the @p amcl driver will typically use 10 to
20Mb of memory.  On embedded systems, where memory is at a premium,
users may have to decrease the map resolution or the maximum number of
particles to achieve acceptable preformance.

@par Example: Using the amcl driver with a Pioneer robot

The following configuration file illustrates the use of the @p amcl
driver on a Pioneer robot equipped with a SICK LMS200 scanning laser
range finder:
@verbatim
driver
(
  name "p2os_position"
  provides ["odometry:::position2d:0"]
  port "/dev/ttyS0"
)
driver
(
  name "sicklms200"
  provides ["laser:0"]
  port "/dev/ttyS2"
)
driver
(
  name "mapfile"
  provides ["map:0"]
  resolution 0.05
  filename "mymap.pgm"
)
driver
(
  name "amcl"
  provides ["localize:0"]
  requires ["odometry:::position2d:0" "laser:0" "laser:::map:0"]
)
@endverbatim
Naturally, the @p port, @p filename and @p resolution values should be
changed to match your particular configuration.

@author Andrew Howard

@todo
- Implement / update other sensor models

*/

/** @} */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>       // for atoi(3)
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>

#define PLAYER_ENABLE_TRACE 1
#define PLAYER_ENABLE_MSG 1

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

#include "amcl.h"
// Sensors
#include "amcl_odom.h"
#include "amcl_laser.h"
//#include "amcl_fiducial.h"
//#include "amcl_gps.h"
//#include "amcl_imu.h"

#define MAX_HYPS 8

////////////////////////////////////////////////////////////////////////////////
// Create an instance of the driver
Driver* AdaptiveMCL_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new AdaptiveMCL(cf, section)));
}


////////////////////////////////////////////////////////////////////////////////
// Register the driver
void AdaptiveMCL_Register(DriverTable* table)
{
  table->AddDriver("amcl", AdaptiveMCL_Init);
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Useful macros
#define AMCL_DATASIZE MAX(sizeof(player_localize_data_t), sizeof(player_position_data_t))


////////////////////////////////////////////////////////////////////////////////
// Constructor
AdaptiveMCL::AdaptiveMCL( ConfigFile* cf, int section)
    : Driver(cf, section)
{
  int i;
  double u[3];
  AMCLSensor *sensor;

  memset(&this->localize_addr, 0, sizeof(player_devaddr_t));
  memset(&this->position_addr, 0, sizeof(player_devaddr_t));

  // Do we create a localize interface?
  if(cf->ReadDeviceAddr(&(this->localize_addr), section, "provides",
                        PLAYER_LOCALIZE_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->localize_addr))
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a position interface?
  if(cf->ReadDeviceAddr(&(this->position_addr), section, "provides",
                        PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->position_addr))
    {
      this->SetError(-1);
      return;
    }
  }

  this->init_sensor = -1;
  this->action_sensor = -1;
  this->sensor_count = 0;

  player_devaddr_t odom_addr;
  player_devaddr_t laser_addr;
  //player_devaddr_t fiducial_addr;

  // Create odometry sensor
  if(cf->ReadDeviceAddr(&odom_addr, section, "requires",
                        PLAYER_POSITION2D_CODE, -1, "odometry") == 0)
  {
    this->action_sensor = this->sensor_count;
    if (cf->ReadInt(section, "odom_init", 1))
      this->init_sensor = this->sensor_count;
    sensor = new AMCLOdom(*this,odom_addr);
    sensor->is_action = 1;
    this->sensors[this->sensor_count++] = sensor;
  }

  // Create laser sensor
  if(cf->ReadDeviceAddr(&laser_addr, section, "requires",
                        PLAYER_LASER_CODE, -1, NULL) == 0)
  {
    sensor = new AMCLLaser(*this,laser_addr);
    sensor->is_action = 0;
    this->sensors[this->sensor_count++] = sensor;
  }

#if 0
  // Create fiducial sensor
  if(cf->ReadDeviceAddr(&fiducial_addr, section, "requires",
                        PLAYER_FIDUCIAL_CODE, -1, NULL) == 0)
  {
    sensor = new AMCLFiducial(fiducial_addr);
    sensor->is_action = 0;
    this->sensors[this->sensor_count++] = sensor;
  }
#endif

  /*
  // Create GPS sensor
  if (cf->ReadInt(section, "gps_index", -1) >= 0)
  {
    if (cf->ReadInt(section, "gps_init", 1))
      this->init_sensor = this->sensor_count;
    this->sensors[this->sensor_count++] = new AMCLGps();
  }

  // Create IMU sensor
  if (cf->ReadInt(section, "imu_index", -1) >= 0)
    this->sensors[this->sensor_count++] = new AMCLImu();
  */

  // We need an "action" sensor
  if(this->action_sensor < 0)
  {
    PLAYER_ERROR("No action sensor");
    this->SetError(-1);
    return;
  }

  // Load sensor settings
  for (i = 0; i < this->sensor_count; i++)
    this->sensors[i]->Load(cf, section);

  // Create the sensor data queue
  this->q_len = 0;
  this->q_start = 0;
  this->q_size = 20000;
  this->q_data = new AMCLSensorData*[this->q_size];
  memset(this->q_data,0,sizeof(AMCLSensorData*)*this->q_size);

  // Particle filter settings
  this->pf = NULL;
  this->pf_min_samples = cf->ReadInt(section, "pf_min_samples", 100);
  this->pf_max_samples = cf->ReadInt(section, "pf_max_samples", 10000);

  // Adaptive filter parameters
  this->pf_err = cf->ReadFloat(section, "pf_err", 0.01);
  this->pf_z = cf->ReadFloat(section, "pf_z", 3);

  // Initial pose estimate
  this->pf_init_pose_mean = pf_vector_zero();
  this->pf_init_pose_mean.v[0] = cf->ReadTupleLength(section, "init_pose", 0, 0);
  this->pf_init_pose_mean.v[1] = cf->ReadTupleLength(section, "init_pose", 1, 0);
  this->pf_init_pose_mean.v[2] = cf->ReadTupleAngle(section, "init_pose", 2, 0);

  // Initial pose covariance
  u[0] = cf->ReadTupleLength(section, "init_pose_var", 0, 1);
  u[1] = cf->ReadTupleLength(section, "init_pose_var", 1, 1);
  u[2] = cf->ReadTupleAngle(section, "init_pose_var", 2, 2*M_PI);
  this->pf_init_pose_cov = pf_matrix_zero();
  this->pf_init_pose_cov.m[0][0] = u[0] * u[0];
  this->pf_init_pose_cov.m[1][1] = u[1] * u[1];
  this->pf_init_pose_cov.m[2][2] = u[2] * u[2];

  // Update distances
  this->min_dr = cf->ReadTupleLength(section, "update_thresh", 0, 0.20);
  this->min_da = cf->ReadTupleAngle(section, "update_thresh", 1, M_PI/6);

  // Initial hypothesis list
  this->hyp_count = 0;
  this->hyp_alloc = 0;
  this->hyps = NULL;
  pthread_mutex_init(&this->best_hyp_lock,NULL);

#ifdef INCLUDE_RTKGUI
  // Enable debug gui
  this->enable_gui = cf->ReadInt(section, "enable_gui", 0);
#endif


  return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor
AdaptiveMCL::~AdaptiveMCL(void)
{
  int i;

  // Delete sensor data queue
  delete[] this->q_data;
  free(hyps);

  // Delete sensors
  for (i = 0; i < this->sensor_count; i++)
  {
    this->sensors[i]->Unload();
    delete this->sensors[i];
  }
  this->sensor_count = 0;
  pthread_mutex_destroy(&this->best_hyp_lock);

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int AdaptiveMCL::Setup(void)
{
  PLAYER_MSG0(2, "setup");

  // Create the particle filter
  assert(this->pf == NULL);
  this->pf = pf_alloc(this->pf_min_samples, this->pf_max_samples);
  this->pf->pop_err = this->pf_err;
  this->pf->pop_z = this->pf_z;

  // Start sensors
  for (int i = 0; i < this->sensor_count; i++)
    if (this->sensors[i]->Setup() < 0)
    {
      PLAYER_ERROR1 ("failed to setup sensor %d", i);
      return -1;
    }


  // Start the driver thread.
  PLAYER_MSG0(2, "running");
  this->StartThread();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int AdaptiveMCL::Shutdown(void)
{
  int i;

  PLAYER_MSG0(2, "shutting down");

  // Stop the driver thread.
  this->StopThread();

  // Stop sensors
  for (i = 0; i < this->sensor_count; i++)
    this->sensors[i]->Shutdown();

  // Delete the particle filter
  pf_free(this->pf);
  this->pf = NULL;

  PLAYER_MSG0(2, "shutdown done");
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Check for updated sensor data
void AdaptiveMCL::UpdateSensorData(void)
{
/*
  int i;
  AMCLSensorData *data;
  pf_vector_t pose, delta;

  assert(this->action_sensor >= 0 && this->action_sensor < this->sensor_count);

  // Check the sensors for new data
  for (i = 0; i < this->sensor_count; i++)
  {
    data = this->sensors[i]->GetData();
    if (data != NULL)
    {
      this->Push(data);
      if(this->sensors[i]->is_action)
      {
        if (this->pf_init)
        {
          // HACK: Assume that the action sensor is odometry
          pose = ((AMCLOdomData*)data)->pose;

          // Compute change in pose
          delta = pf_vector_coord_sub(pose, this->pf_odom_pose);

          // Publish new pose estimate
          this->PutDataPosition(delta);
        }
      }
    }
  }
  return;*/
}


////////////////////////////////////////////////////////////////////////////////
// Push data onto the filter queue.
void AdaptiveMCL::Push(AMCLSensorData *data)
{
  int i;

  this->Lock();

  if (this->q_len >= this->q_size)
  {
    this->Unlock();
    PLAYER_ERROR("queue overflow");
    return;
  }
  i = (this->q_start + this->q_len++) % this->q_size;

  this->q_data[i] = data;

  this->Unlock();
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Take a peek at the queue
AMCLSensorData *AdaptiveMCL::Peek(void)
{
  int i;

  this->Lock();

  if (this->q_len == 0)
  {
    this->Unlock();
    return NULL;
  }
  i = this->q_start % this->q_size;

  this->Unlock();
  return this->q_data[i];
}


////////////////////////////////////////////////////////////////////////////////
// Pop data from the filter queue
AMCLSensorData *AdaptiveMCL::Pop(void)
{
  int i;

  this->Lock();

  if (this->q_len == 0)
  {
    this->Unlock();
    return NULL;
  }
  i = this->q_start++ % this->q_size;
  this->q_len--;

  this->Unlock();
  return this->q_data[i];
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void AdaptiveMCL::Main(void)
{
  this->q_len = 0;

  // No data has yet been pushed, and the
  // filter has not yet been initialized
  this->pf_init = false;

  // Initial hypothesis list
  this->hyp_count = 0;

  // WARNING: this only works for Linux
  // Run at a lower priority
  nice(10);

#ifdef INCLUDE_RTKGUI
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  // Start the GUI
  if (this->enable_gui)
    this->SetupGUI();
#endif

#ifdef INCLUDE_OUTFILE
  // Open file for logging results
  this->outfile = fopen("amcl.out", "w+");
#endif

  while (true)
  {
#ifdef INCLUDE_RTKGUI
    if (this->enable_gui)
    {
      rtk_canvas_render(this->canvas);
      rtk_app_main_loop(this->app);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
#endif

    // Sleep for 1ms (will actually take longer than this).
    const struct timespec sleeptime = {0, 1000000L};
    nanosleep(&sleeptime, NULL);

    // Test if we are supposed to cancel this thread.  This is the
    // only place we can cancel from if we are running the GUI.
    pthread_testcancel();

    // Process any pending messages
    this->ProcessMessages();
    //UpdateSensorData();

#ifdef INCLUDE_RTKGUI
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
#endif

    // Initialize the filter if we havent already done so
    if (!this->pf_init)
      this->InitFilter();

    // Update the filter
    if (this->UpdateFilter())
    {
#ifdef INCLUDE_OUTFILE
      // Save the error values
      pf_vector_t mean;
      double var;

      pf_get_cep_stats(pf, &mean, &var);

      printf("amcl %.3f %.3f %f\n", mean.v[0], mean.v[1], mean.v[2] * 180 / M_PI);

      fprintf(this->outfile, "%d.%03d unknown 6665 localize 01 %d.%03d ",
              0, 0, 0, 0);
      fprintf(this->outfile, "1.0 %e %e %e %e 0 0 0 0 0 0 0 0 \n",
              mean.v[0], mean.v[1], mean.v[2], var);
      fflush(this->outfile);
#endif
    }
  }
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Thread finalization
void AdaptiveMCL::MainQuit()
{
#ifdef INCLUDE_RTKGUI
  // Stop the GUI
  if (this->enable_gui)
    this->ShutdownGUI();
#endif

#ifdef INCLUDE_OUTFILE
  // Close the log file
  fclose(this->outfile);
#endif

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Initialize the filter
void AdaptiveMCL::InitFilter(void)
{
  ::pf_init(this->pf, this->pf_init_pose_mean, this->pf_init_pose_cov);

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Update the filter with new sensor data
bool AdaptiveMCL::UpdateFilter(void)
{
  int i;
  double ts;
  double weight;
  pf_vector_t pose, delta;
  pf_vector_t pose_mean;
  pf_matrix_t pose_cov;
  amcl_hyp_t *hyp;
  AMCLSensorData *data;
  bool update;

  //printf("q len %d\n", this->q_len);

  // Get the action data
  data = this->Pop();
  if (data == NULL)
    return false;
  if (!data->sensor->is_action)
  {
    delete data;
    return false;
  }

  // Use the action timestamp
  ts = data->time;

  // HACK
  pose = ((AMCLOdomData*) data)->pose;
  delta = pf_vector_zero();
  update = false;

  //printf("odom pose\n");
  //pf_vector_fprintf(pose, stdout, "%.3f");

  // Compute change in pose
  if (this->pf_init)
  {
    // Compute change in pose
    delta = pf_vector_coord_sub(pose, this->pf_odom_pose);

    // See if we should update the filter
    update = fabs(delta.v[0]) > this->min_dr ||
      fabs(delta.v[1]) > this->min_dr ||
      fabs(delta.v[2]) > this->min_da;
  }

  // If the filter has not been initialized
  if (!this->pf_init)
  {
    // Discard this action data
    delete data; data = NULL;

    // Pose at last filter update
    this->pf_odom_pose = pose;

    // Filter is now initialized
    this->pf_init = true;

    // Should update sensor data
    update = true;

    //printf("init\n");
    //pf_vector_fprintf(pose, stdout, "%.3f");
  }

  // If the robot has moved, update the filter
  else if (this->pf_init && update)
  {
    //printf("pose\n");
    //pf_vector_fprintf(pose, stdout, "%.3f");

    // HACK
    // Modify the delta in the action data so the filter gets
    // updated correctly
    ((AMCLOdomData*) data)->delta = delta;

    // Use the action data to update the filter
    data->sensor->UpdateAction(this->pf, data);
    delete data; data = NULL;

    // Pose at last filter update
    //this->pf_odom_pose = pose;
  }
  else
  {
    // Discard this action data
    delete data; data = NULL;
  }

  bool processed_first_sensor = false;
  // If the robot has moved, update the filter
  if (update)
  {
    // Process the remaining sensor data up to the next action data
    while (1)
    {
      data = this->Peek();
      if ((data == NULL ) || (data->sensor->is_action))
      {
        // HACK: Discard action data until we've processed at least one
        // sensor reading
        if(!processed_first_sensor)
        {
          if(data)
          {
            data = this->Pop();
            assert(data);
            delete data;
          }
          // avoid a busy loop while waiting for a sensor reading to
          // process
          //return false;
          usleep(1000);
          ProcessMessages();
          //UpdateSensorData();
          continue;
        }
        else
          break;
      }
      data = this->Pop();
      assert(data);

      // Use the data to update the filter
      // HACK: only use the first sensor reading
      if(!processed_first_sensor)
      {
        data->sensor->UpdateSensor(this->pf, data);
        processed_first_sensor = true;
        this->pf_odom_pose = pose;
      }

#ifdef INCLUDE_RTKGUI
      // Draw the current sensor data
      if (this->enable_gui)
        data->sensor->UpdateGUI(this->canvas, this->robot_fig, data);
#endif

      // Discard data
      delete data; data = NULL;
    }

    // Resample the particles
    pf_update_resample(this->pf);

    // Read out the current hypotheses
    double max_weight = 0.0;
    pf_vector_t max_weight_pose={{0.0,0.0,0.0}};
    this->hyp_count = 0;
    //for (i = 0; (size_t) i < sizeof(this->hyps) / sizeof(this->hyps[0]); i++)
    for (i = 0; MAX_HYPS; i++)
    {
      if (!pf_get_cluster_stats(this->pf, i, &weight, &pose_mean, &pose_cov))
        break;

      //pf_vector_fprintf(pose_mean, stdout, "%.3f");

      if (this->hyp_count +1 > this->hyp_alloc)
      {
        this->hyp_alloc = this->hyp_count+1;
        this->hyps = (amcl_hyp_t*)realloc(this->hyps, sizeof(amcl_hyp_t)*this->hyp_alloc);
      }
      hyp = this->hyps + this->hyp_count++;
      hyp->weight = weight;
      hyp->pf_pose_mean = pose_mean;
      hyp->pf_pose_cov = pose_cov;

      if(hyp->weight > max_weight)
      {
        max_weight = hyp->weight;
        max_weight_pose = hyp->pf_pose_mean;
      }
    }

    if(max_weight > 0.0)
    {
      pthread_mutex_lock(&this->best_hyp_lock);
      this->best_hyp = max_weight_pose;
      pthread_mutex_unlock(&this->best_hyp_lock);
    }

#ifdef INCLUDE_RTKGUI
    // Update the GUI
    if (this->enable_gui)
      this->UpdateGUI();
#endif

    // Encode data to send to server
    this->PutDataLocalize(ts);
    delta = pf_vector_zero();
    this->PutDataPosition(delta,ts);

    return true;
  }
  else
  {
    // Process the remaining sensor data up to the next action data
    while (1)
    {
      data = this->Peek();
      if (data == NULL)
        break;
      if (data->sensor->is_action)
        break;
      data = this->Pop();
      assert(data);
      delete data; data = NULL;
    }

    // Encode data to send to server; only the position interface
    // gets updated every cycle
    this->PutDataPosition(delta,ts);

    return false;
  }
}

// this function will be passed to qsort(3) to sort the hypoths before
// sending them out.  the idea is to sort them in descending order of
// weight.
int
hypoth_compare(const void* h1, const void* h2)
{
  const player_localize_hypoth_t* hyp1 = (const player_localize_hypoth_t*)h1;
  const player_localize_hypoth_t* hyp2 = (const player_localize_hypoth_t*)h2;
  if(hyp1->alpha < hyp2->alpha)
    return(1);
  else if(hyp1->alpha == hyp2->alpha)
    return(0);
  else
    return(-1);
}


////////////////////////////////////////////////////////////////////////////////
// Output data on the localize interface
void AdaptiveMCL::PutDataLocalize(double time)
{
  int i;
  amcl_hyp_t *hyp;
  pf_vector_t pose;
  pf_matrix_t pose_cov;
  //size_t datalen;
  player_localize_data_t data;

  // Record the number of pending observations
  data.pending_count = 0;
  data.pending_time = 0.0;

  // Encode the hypotheses
  data.hypoths_count = this->hyp_count;
  data.hypoths = new player_localize_hypoth_t[data.hypoths_count];
  for (i = 0; i < this->hyp_count; i++)
  {
    hyp = this->hyps + i;

    // Get the current estimate
    pose = hyp->pf_pose_mean;
    pose_cov = hyp->pf_pose_cov;

    // Check for bad values
    if (!pf_vector_finite(pose))
    {
      pf_vector_fprintf(pose, stderr, "%e");
      assert(0);
    }
    if (!pf_matrix_finite(pose_cov))
    {
      pf_matrix_fprintf(pose_cov, stderr, "%e");
      assert(0);
    }

    data.hypoths[i].alpha = hyp->weight;

    data.hypoths[i].mean.px = pose.v[0];
    data.hypoths[i].mean.py = pose.v[1];
    data.hypoths[i].mean.pa = pose.v[2];

    data.hypoths[i].cov[0] = pose_cov.m[0][0];
    data.hypoths[i].cov[1] = pose_cov.m[1][1];
    data.hypoths[i].cov[2] = pose_cov.m[2][2];
  }

  // sort according to weight
  qsort((void*)data.hypoths,data.hypoths_count,
        sizeof(player_localize_hypoth_t),&hypoth_compare);

  // Push data out
  this->Publish(this->localize_addr, 
                PLAYER_MSGTYPE_DATA,
                PLAYER_LOCALIZE_DATA_HYPOTHS,
                (void*)&data);
  delete [] data.hypoths;
}


////////////////////////////////////////////////////////////////////////////////
// Output data on the position interface
void AdaptiveMCL::PutDataPosition(pf_vector_t delta, double time)
{
  pf_vector_t pose;
  player_position2d_data_t data;

  memset(&data,0,sizeof(data));

  // Get the current estimate
  pthread_mutex_lock(&this->best_hyp_lock);
  pose = this->best_hyp;
  pthread_mutex_unlock(&this->best_hyp_lock);

  // Add the accumulated odometric change
  pose = pf_vector_coord_add(delta, pose);

  data.pos.px = pose.v[0];
  data.pos.py = pose.v[1];
  data.pos.pa = pose.v[2];

  // Push data out
  this->Publish(this->position_addr, 
                PLAYER_MSGTYPE_DATA,
                PLAYER_POSITION2D_DATA_STATE,
                (void*)&data,sizeof(data),&time);
}

// MessageHandler
int
AdaptiveMCL::ProcessMessage(QueuePointer & resp_queue,
                            player_msghdr * hdr,
                            void * data)
{
  player_localize_set_pose_t* setposereq;

  // Is it a request to set the filter's pose?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                           PLAYER_LOCALIZE_REQ_SET_POSE,
                           this->localize_addr))
  {
    setposereq = (player_localize_set_pose_t*)data;

    pf_vector_t pose;
    pf_matrix_t cov;

    pose.v[0] = setposereq->mean.px;
    pose.v[1] = setposereq->mean.py;
    pose.v[2] = setposereq->mean.pa;

    cov = pf_matrix_zero();
    cov.m[0][0] = setposereq->cov[0];
    cov.m[1][1] = setposereq->cov[1];
    cov.m[2][2] = setposereq->cov[2];

    // Re-initialize the filter
    this->pf_init_pose_mean = pose;
    this->pf_init_pose_cov = cov;
    this->pf_init = false;

    // Send an ACK
    this->Publish(this->localize_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_LOCALIZE_REQ_SET_POSE);
    return(0);
  }
  // Is it a request for the current particle set?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                PLAYER_LOCALIZE_REQ_GET_PARTICLES,
                                this->localize_addr))
  {
    pf_vector_t mean;
    double var;
    player_localize_get_particles_t resp;
    pf_sample_set_t *set;
    pf_sample_t *sample;
    size_t i;

    pf_get_cep_stats(this->pf, &mean, &var);

    resp.mean.px = mean.v[0];
    resp.mean.py = mean.v[1];
    resp.mean.pa = mean.v[2];
    resp.variance = var;

    set = this->pf->sets + this->pf->current_set;

    resp.particles_count = set->sample_count;
    resp.particles = new player_localize_particle_t [resp.particles_count];

    // TODO: pick representative particles
    for(i=0;i<resp.particles_count;i++)
    {
      sample = set->samples + i;
      resp.particles[i].pose.px = sample->pose.v[0];
      resp.particles[i].pose.py = sample->pose.v[1];
      resp.particles[i].pose.pa = sample->pose.v[2];
      resp.particles[i].alpha = sample->weight;
    }

    this->Publish(this->localize_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_LOCALIZE_REQ_GET_PARTICLES,
                  (void*)&resp);
    delete [] resp.particles;
    return(0);
  } else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                  PLAYER_POSITION2D_REQ_GET_GEOM, device_addr))
  {
    assert(hdr->size == 0);
    ProcessGeom(resp_queue, hdr);
    return(0);
  }

  // pass on the rest of the messages to the sensors
  for (int i = 0; i < this->sensor_count; i++)
  {
    int ret = this->sensors[i]->ProcessMessage(resp_queue, hdr, data);
    if (ret >= 0)
    	return ret;
  }

  return(-1);
}

void
AdaptiveMCL::ProcessGeom(QueuePointer &resp_queue, player_msghdr_t* hdr)
{
  player_position2d_geom_t geom={{0}};
  // just return a point so we don't get errors from playerv
  geom.size.sl = 0.01;
  geom.size.sw = 0.01;

  Publish(this->position_addr, resp_queue,
          PLAYER_MSGTYPE_RESP_ACK,
          PLAYER_POSITION2D_REQ_GET_GEOM,
          &geom, sizeof(geom), NULL);
}

#ifdef INCLUDE_RTKGUI

////////////////////////////////////////////////////////////////////////////////
// Set up the GUI
int AdaptiveMCL::SetupGUI(void)
{
  int i;

  // Initialize RTK
  rtk_init(NULL, NULL);

  this->app = rtk_app_create();

  this->canvas = rtk_canvas_create(this->app);
  rtk_canvas_title(this->canvas, "AdaptiveMCL");

  /* TODO: fix
  if (this->map != NULL)
  {
    rtk_canvas_size(this->canvas, this->map->size_x, this->map->size_y);
    rtk_canvas_scale(this->canvas, this->map->scale, this->map->scale);
  }
  else
  */
  {
    rtk_canvas_size(this->canvas, 400, 400);
    rtk_canvas_scale(this->canvas, 0.1, 0.1);
  }

  this->map_fig = rtk_fig_create(this->canvas, NULL, -1);
  this->pf_fig = rtk_fig_create(this->canvas, this->map_fig, 5);

  /* FIX
  // Draw the map
  if (this->map != NULL)
  {
    map_draw_occ(this->map, this->map_fig);
    //map_draw_cspace(this->map, this->map_fig);

    // HACK; testing
    map_draw_wifi(this->map, this->map_fig, 0);
  }
  */

  rtk_fig_line(this->map_fig, 0, -100, 0, +100);
  rtk_fig_line(this->map_fig, -100, 0, +100, 0);

  // Best guess figure
  this->robot_fig = rtk_fig_create(this->canvas, NULL, 9);

  // Draw the robot
  rtk_fig_color(this->robot_fig, 0.7, 0, 0);
  rtk_fig_rectangle(this->robot_fig, 0, 0, 0, 0.40, 0.20, 0);

  // TESTING
  //rtk_fig_movemask(this->robot_fig, RTK_MOVE_TRANS | RTK_MOVE_ROT);

  // Initialize the sensor GUI's
  for (i = 0; i < this->sensor_count; i++)
    this->sensors[i]->SetupGUI(this->canvas, this->robot_fig);

  rtk_app_main_init(this->app);

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shut down the GUI
int AdaptiveMCL::ShutdownGUI(void)
{
  int i;

  // Finalize the sensor GUI's
  for (i = 0; i < this->sensor_count; i++)
    this->sensors[i]->ShutdownGUI(this->canvas, this->robot_fig);

  rtk_fig_destroy(this->robot_fig);
  rtk_fig_destroy(this->pf_fig);
  rtk_fig_destroy(this->map_fig);

  rtk_canvas_destroy(this->canvas);
  rtk_app_destroy(this->app);

  rtk_app_main_term(this->app);

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Update the GUI
void AdaptiveMCL::UpdateGUI(void)
{
  rtk_fig_clear(this->pf_fig);
  rtk_fig_color(this->pf_fig, 0, 0, 1);
  pf_draw_samples(this->pf, this->pf_fig, 1000);
  pf_draw_cep_stats(this->pf, this->pf_fig);
  //rtk_fig_color(this->pf_fig, 0, 1, 0);
  //pf_draw_stats(this->pf, this->pf_fig);
  //pf_draw_hist(this->pf, this->pf_fig);

  // Draw the best pose estimate
  this->DrawPoseEst();

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Draw the current best pose estimate
void AdaptiveMCL::DrawPoseEst()
{
  int i;
  double max_weight;
  amcl_hyp_t *hyp;
  pf_vector_t pose;

  this->Lock();

  max_weight = -1;
  for (i = 0; i < this->hyp_count; i++)
  {
    hyp = this->hyps + i;

    if (hyp->weight > max_weight)
    {
      max_weight = hyp->weight;
      pose = hyp->pf_pose_mean;
    }
  }

  this->Unlock();

  if (max_weight < 0.0)
    return;

  // Shift the robot figure
  rtk_fig_origin(this->robot_fig, pose.v[0], pose.v[1], pose.v[2]);

  return;
}

#endif


