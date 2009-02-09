/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey   &  Kasper Stoy
 *                      gerkey@usc.edu    kaspers@robotics.usc.edu
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
// Desc: Driver ISense InertiaCube2 orientation sensor.
// Author: Andrew Howard
// Date: 20 Aug 2002
// CVS: $Id: inertiacube2.cc 4135 2007-08-23 19:58:48Z gerkey $
//
// Theory of operation:
//  Uses an inertial orientation sensor to correct the odometry coming
//  from a robot.  The assumption is that the position device we
//  subscribe to has good position information but poor orientation
//  information.
//
// Requires: position
// Provides: position
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_inertiacube2 inertiacube2
 * @brief iSense InertiaCube2 IMU

@todo This driver is currently disabled because it needs to be updated to
the Player 2.0 API.

Uses an iSense InertiaCube2 inertial orientation sensor to correct
the odometry coming from a robot.  The assumption is that the position
device we subscribe to has good position information but poor orientation
information.

Neither configuration requests nor commands are passed through to the
underlying @ref interface_position2d device.

@par Compile-time dependencies

- &lt;isense/isense.h&gt;

@par Provides

- @ref interface_position2d : corrected pose information

@par Requires

- @ref interface_position2d : source of raw odometry

@par Configuration requests

- none

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS3"
  - The serial port where the InertiaCube2 is connected

- compass (integer)
  - Default: 2
  - Compass setting (0 = off, 1 = partial, 2 = full).

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
  name "inertiacube2"
  requires ["position:1"]  # get odometry from position:1
  provides ["position:0"]  # produce corrected pose on position:0
  port "/dev/ttyS1"
)
@endverbatim

@author Andrew Howard

*/
/** @} */

#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>       // for atoi(3)
#include <netinet/in.h>   // for htons(3)
#include <unistd.h>

#include <isense/isense.h>

#include <libplayercore/playercore.h>


// Driver for detecting laser retro-reflectors.
class InertiaCube2 : public Driver
{
  // Constructor
  public: InertiaCube2( ConfigFile* cf, int section);

  // Setup/shutdown routines.
  public: virtual int Setup();
  public: virtual int Shutdown();

  // Set up the underlying position device.
  private: int SetupPosition();
  private: int ShutdownPosition();

  // Initialize the IMU.
  private: int SetupImu();
  private: int ShutdownImu();

  // Get the tracker type.
  private: const char *ImuType(int type);

  // Get the tracker model.
  private: const char *ImuModel(int model);

  // Main function for device thread.
  private: virtual void Main();

  // Process incoming messages from clients 
  int ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data);

  // Update the InertiaCube.
  private: void UpdateImu();

  // Generate a new pose estimate.
  private: void UpdatePose();

  // Update the device data (the data going back to the client).
  private: void UpdateData();

  // Geometry of underlying position device.
  private: player_position2d_geom_t geom;

  // Compass setting (0 = off, 1 = partial, 2 = full).
  private: int compass;

  // Serial port.
  private: const char *port;

  // Position device info (the one we are subscribed to).
  private: player_devaddr_t position_id;
  private: Device *position;
  private: double position_time;
  private: player_pose_t position_old_pose;
  private: player_pose_t position_new_pose;

  // Handle to the imu tracker.
  private: ISD_TRACKER_HANDLE imu;
  private: double imu_old_orient;
  private: double imu_new_orient;

  // Combined pose estimate.
  private: player_pose_t pose;
};


// Initialization function
Driver* InertiaCube2_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new InertiaCube2( cf, section)));
}


// a driver registration function
void InertiaCube2_Register(DriverTable* table)
{
  table->AddDriver("inertiacube2", InertiaCube2_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
InertiaCube2::InertiaCube2( ConfigFile* cf, int section)
        : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_POSITION2D_CODE)
{
  this->port = cf->ReadString(section, "port", "/dev/ttyS3");

  // Must have a position device
  if (cf->ReadDeviceAddr(&this->position_id, section, "requires",
                       PLAYER_POSITION2D_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  this->position = NULL;
  this->position_time = 0;

  this->compass = cf->ReadInt(section, "compass", 2);

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int InertiaCube2::Setup()
{
  // Initialise the underlying position device.
  if (this->SetupPosition() != 0)
    return -1;

  // Initialise the cube.
  if (this->SetupImu() != 0)
    return -1;

  // Start the driver thread.
  this->StartThread();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int InertiaCube2::Shutdown()
{
  // Stop the driver thread.
  this->StopThread();

  // Stop the imu.
  this->ShutdownImu();

  // Stop the position device.
  this->ShutdownPosition();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the underlying position device.
int InertiaCube2::SetupPosition()
{
  // Subscribe to the positino device.
  if (Device::MatchDeviceAddress (position_id, device_addr))
  {
    PLAYER_ERROR ("attempt to subscribe to self");
    return -1;
  }
  if (!(position = deviceTable->GetDevice (position_id)))
  {
    PLAYER_ERROR ("unable to locate suitable camera device");
    return -1;
  }
  if (position->Subscribe (InQueue) != 0)
  {
    PLAYER_ERROR ("unable to subscribe to camera device");
    return -1;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
int InertiaCube2::ShutdownPosition()
{
  // Unsubscribe from devices.
  this->position->Unsubscribe(InQueue);

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Initialize the imu.
int InertiaCube2::SetupImu()
{
  int i;
  int port;
  int verbose;
  ISD_TRACKER_INFO_TYPE info;
  ISD_STATION_INFO_TYPE sinfo;
  ISD_TRACKER_DATA_TYPE data;

  verbose = 0;

  // Open the tracker.  Takes a port number, so we strip it from the
  // port string.
  port = atoi(this->port + strlen(this->port) - 1);
  this->imu = ISD_OpenTracker((Hwnd) NULL, port + 1, FALSE, verbose);
  if (this->imu < 1)
  {
    PLAYER_ERROR("failed to detect InterSense tracking device");
    return -1;
  }

  // Get tracker configuration info
  if (!ISD_GetTrackerConfig(this->imu, &info, verbose))
  {
    PLAYER_ERROR("failed to get configuration info");
    return -1;
  }

  printf("InterSense Tracker type [%s] model [%s]\n",
         this->ImuType(info.TrackerType), this->ImuModel(info.TrackerModel));

  // Get some more configuration info.
  if (!ISD_GetStationConfig(this->imu, &sinfo, 1, verbose))
  {
    PLAYER_ERROR("failed to get station info");
    return -1;
  }

  // Set compass value (0 = off, 1 = partial, 2 = full).
  sinfo.Compass = this->compass;

  printf("compass %d enhancement %d sensitivity %d prediction %d format %d\n",
         sinfo.Compass, sinfo.Enhancement, sinfo.Sensitivity,
         sinfo.Prediction, sinfo.AngleFormat);

  // Change the configuration.
  if (!ISD_SetStationConfig(this->imu, &sinfo, 1, verbose))
  {
    PLAYER_ERROR("failed to get station info");
    return -1;
  }

  // Wait a while for the unit to settle.
  for (i = 0; i < 100;)
  {
    if (!ISD_GetData(this->imu, &data))
    {
      PLAYER_ERROR("failed to get data");
      return -1;
    }
    if (data.Station[0].NewData)
      i++;
    usleep(10000);
  }

  // Reset the heading component.
  if (!ISD_ResetHeading(this->imu, 1))
  {
    PLAYER_ERROR("failed to reset heading");
    return -1;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Finalize the imu.
int InertiaCube2::ShutdownImu()
{
  ISD_CloseTracker(this->imu);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Get the tracker type.
const char *InertiaCube2::ImuType(int type)
{
  switch (type)
  {
    case ISD_NONE:
      return "Unknown";
    case ISD_PRECISION_SERIES:
      return "IS Precision Series";
    case ISD_INTERTRAX_SERIES:
      return "InterTrax Series";
  }
  return "Unknown";
}


////////////////////////////////////////////////////////////////////////////////
// Get the tracker model.
const char *InertiaCube2::ImuModel(int model)
{
  switch (model)
  {
    case ISD_IS300:
      return "IS-300 Series";
    case ISD_IS600:
      return "IS-600 Series";
    case ISD_IS900:
      return "IS-900 Series";
    case ISD_INTERTRAX:
      return "InterTrax 30";
    case ISD_INTERTRAX_2:
      return "InterTrax2";
    case ISD_INTERTRAX_LS:
      return "InterTraxLS";
    case ISD_INTERTRAX_LC:
      return "InterTraxLC";
    case ISD_ICUBE2:
      return "InertiaCube2";
    case ISD_ICUBE2_PRO:
      return "InertiaCube2 Pro";
    case ISD_IS1200:
      return "IS-1200 Series";
  }
  return "Unknown";
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void InertiaCube2::Main()
{
  struct timespec sleeptime;

  // Sleep for 1ms (will actually take longer than this).
  sleeptime.tv_sec = 0;
  sleeptime.tv_nsec = 1000000L;

  while (true)
  {
    // Go to sleep for a while (this is a polling loop).
    nanosleep(&sleeptime, NULL);

    // Test if we are supposed to cancel this thread.
    pthread_testcancel();

    // Process any pending requests.
    ProcessMessages();

    // Update the InertiaCube
    UpdateImu();
  }
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int InertiaCube2::ProcessMessage (QueuePointer &resp_queue, player_msghdr * hdr, void * data)
{
  assert(hdr);
  assert(data);

  if(Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA , PLAYER_POSITION2D_DATA_STATE, position_id))
  {
    player_position2d_data_t & pos_data = *reinterpret_cast<player_position2d_data_t *> (data);
    position_new_pose = pos_data.pos;
    UpdatePose();
    UpdateData();
    return 0;
  }
  return -1;
}



////////////////////////////////////////////////////////////////////////////////
// Update the InertiaCube.
void InertiaCube2::UpdateImu()
{
  ISD_TRACKER_DATA_TYPE data;

  // Update the tracker data.
  if (ISD_GetData(this->imu, &data) == 0)
  {
    PLAYER_ERROR("error getting data");
    return;
  }

  // Pick out the yaw value.
  this->imu_new_orient = -data.Station[0].Orientation[0] * M_PI / 180;

  /*
  printf("orientation %f %f %f\r",
         data.Station[0].Orientation[0],
         data.Station[0].Orientation[1],
         data.Station[0].Orientation[2]);
  */

  return;
}



////////////////////////////////////////////////////////////////////////////////
// Generate a new pose estimate.
// This algorithm assumes straight line segments.  We can probably to better
// than this.
void InertiaCube2::UpdatePose()
{
  double dx, dy, da;
  double tx, ty;

  // Compute change in pose relative to previous pose.
  dx = this->position_new_pose.px - this->position_old_pose.px;
  dy = this->position_new_pose.py - this->position_old_pose.py;
  da = this->position_old_pose.pa;
  tx =  dx * cos(da) + dy * sin(da);
  ty = -dx * sin(da) + dy * cos(da);

  // Add this to the previous pose esimate.
  this->pose.px += tx * cos(this->imu_old_orient) - ty * sin(this->imu_old_orient);
  this->pose.py += tx * sin(this->imu_old_orient) + ty * cos(this->imu_old_orient);
  this->pose.pa = this->imu_new_orient;

  this->position_old_pose = this->position_new_pose;
  this->imu_old_orient = this->imu_new_orient;

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Update the device data (the data going back to the client).
void InertiaCube2::UpdateData()
{
  uint32_t timesec, timeusec;
  player_position2d_data_t data;

  data.pos = pose;

  // Copy data to server.
  Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, (unsigned char*) &data, sizeof(data), &position_time);

  return;
}
