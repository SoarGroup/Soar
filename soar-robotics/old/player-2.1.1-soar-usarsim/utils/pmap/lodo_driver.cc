/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003
 *     Brian Gerkey, Andrew Howard
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
 * Desc: Player driver for laser-stabilized odometry
 * Author: Andrew Howard
 * Date: 24 Nov 2004
 * CVS: $Id: lodo_driver.cc 4360 2008-02-15 09:20:26Z thjc $
 */

/** @defgroup lodo_driver Laser-stabilized odometry (lodo_driver)

The lodo driver provides laser-stabilized odometric pose estimates:
the inputs are raw odometry and laser scans, the outputs are corrected
pose estimates and synchronized laser scans (i.e., no lag-lead
effects).  The drift in this corrected estimate is much, much, lower
than that seen with odometry alone (e.g., less than 1m drift in
position after 100m of travel).  The lodo driver is a thin wrapper
around a free-standing laser-odometry library (liblodo); see lodo.h
for details of how the library works.

To use the lodo driver, simply point it at a suitable pair of position
and laser devices.  Generally, the position device will be provided by
to a robot driver, such @ref player_driver_p2os, while the laser
device will be provided by the @ref player_driver_sicklms200 driver.
The lodo driver provides a new pair of position and laser interfaces
that provide corrected pose estimates and synchronized laser data,
respectively.  If you have a Pioneer-like robot with a SICK laser, the
lodo driver will be almost entirely transparent to your client
programs: simply get odometry and laser data from the lodo driver
instead of the underlying hardware drivers.

@par Laser configuration

For best performance, set the laser range and bearing resolutions
to 10cm and 1 degree, respectively; for the SICK, this corresponds to:

@verbatim
driver
(
  name "sicklms200"
  resolution 100
  range_res 10
  rate 38400
)
@endverbatim

It is also vitally important that the laser be rigidly mounted to
the robot (that means no velcro or gaffer tape), with a known, fixed
orientation.  Badly aligned lasers introduce systematic errors and high
drift rates (worse than odometry alone).

See the caveats below for additional usage notes.


@par Caveats

While the driver has been extensively tested on certain hardware
configurations (Pioneer2DX robots with SICK LMS200 lasers), mileage
may vary on other platforms. Several points should be noted:

- The incremental pose corrections are applied to orientation @e only; this
works great on robots with good linear odometry (like the Pioneer2DX),
but is likely to fail on a skid-steered Pioneer2AT (don't know,
haven't tried it).

- The driver has been tested with a SICK LMS200 operating at 38400 baud,
1 degree angular resolution and 10cm range resolution (yields 181 samples at
10Hz).  Higher data rates (e.g., 75Hz are not recommended unless you have
a very fast processor).

- The library is fairly compute intensive; on a 2.6 GHz P4, processing
time for each scan is of the order of 10ms.



@par Requires

- @ref player_interface_position2d
- @ref player_interface_laser


@par Provides

- @ref player_interface_position2d
- @ref player_interface_laser


@par Supported configuration requests

None

@todo Pass configuration requests through to underlying drivers.


@par Configuration file options

- plugin (filename)
  - Default: "lodo_driver.so"
  - Loads the driver as a plugin.

- laser_pose (float tuple)
  - Default: [0 0 0]
  - Pose of the laser sensor in the robot's coordinate system (m, m, degrees).

- max_range (float length)
  - Default: 8.0
  - Maximum valid laser range.


@todo Expose library options.

@par Example

@verbatim
device
(
  plugin "lodo_driver.so"
  driver "lodo_driver"
  requires ["position2d:0" "laser:0"]
  provides ["position2d:10" "laser:10"]
)
@endverbatim

@par Authors

Andrew Howard

*/

#include <unistd.h>
#include <string.h>
#include <netinet/in.h>

/*#include <libplayercore/drivertable.h>
#include <libplayercore/devicetable.h>
#include <libplayercore/driver.h>*/
#include <libplayercore/playercore.h>
#include <libplayercore/error.h>
// #include <libplayercore/player.h>

#include <libplayerc/playerc.h>

#include "lodo.h"


// Use this to replace missing declaration in player 1.6
extern "C" void playerc_position2d_putdata(playerc_position2d_t *device, player_msghdr_t *header,
                                         player_position2d_data_t *data, size_t len);


////////////////////////////////////////////////////////////////////////////////
/// @brief Driver class for laser-stabilized odometry (lodo)
class LodoDriver : public Driver
{
  /// @brief Constructor; need that
  public: LodoDriver(ConfigFile* cf, int section);

  /// @brief Initialize the driver
  public: int Setup();

  /// @brief Finalize the driver
  public: int Shutdown();

  /// @brief Message handler
  public: int ProcessMessage (QueuePointer &resp_queue, player_msghdr *hdr, void *data);

  /// Laser offset in robot cs
  private: pose2_t laser_pose;

  /// Max valid laser range
  private: double max_range;

  /// Client proxy
  private: playerc_client_t *client;

  /// Required position2d interface
  private: player_devaddr_t position2d_id;
  private: Device *position2d_driver;
  private: player_position2d_data_t position2d_data;
//  private: playerc_position2d_t *position2d;

  /// Required laser interface
  private: player_devaddr_t laser_id;
  private: Device *laser_driver;
  private: player_laser_data_t laser_data;
//  private: playerc_laser_t *laser;
  private: double laser_time;
//  private: int laser_data_size;
  private: double *rangeData;

  /// Provided position2d interface
  private: player_devaddr_t out_position2d_id;
  private: player_position2d_data_t out_position2d_data;
//  private: player_position2d_cmd_t out_position2d_cmd;

  /// Provided laser interface
  private: player_devaddr_t out_laser_id;

  /// Lodo lib handler
  private: lodo_t *lodo;

  /// Corrected robot pose
  private: pose2_t robot_pose;
};


// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver* LodoDriver_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return ((Driver*) (new LodoDriver(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void LodoDriver_Register(DriverTable* table)
{
  table->AddDriver("lodo_driver", LodoDriver_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

// need the extern to avoid C++ name-mangling
extern "C"
{
  int player_driver_init(DriverTable* table)
  {
    puts("plugin init");
    LodoDriver_Register(table);
    return(0);
  }
}


////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
LodoDriver::LodoDriver(ConfigFile* cf, int section)
    : Driver(cf, section)
{
  lodo = NULL;

  // Get incoming position2d interface
  if (cf->ReadDeviceAddr(&(this->position2d_id), section, "requires",
                       PLAYER_POSITION2D_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }

  // Get incoming laser interface
  if (cf->ReadDeviceAddr(&(this->laser_id), section, "requires",
                       PLAYER_LASER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }

  // Create position2d interface
  if (cf->ReadDeviceAddr(&(this->out_position2d_id), section, "provides",
                       PLAYER_POSITION2D_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  if (this->AddInterface(this->out_position2d_id) != 0)
  {
    this->SetError(-1);
    return;
  }

  // Create laser interface
  if (cf->ReadDeviceAddr(&(this->out_laser_id), section, "provides",
                       PLAYER_LASER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  if (this->AddInterface(this->out_laser_id) != 0)
  {
    this->SetError(-1);
    return;
  }

  // Laser offset from robot origin
  this->laser_pose.pos.x = cf->ReadTupleLength(section, "laser_pose", 0, 0.0);
  this->laser_pose.pos.y = cf->ReadTupleLength(section, "laser_pose", 1, 0.0);
  this->laser_pose.rot = cf->ReadTupleAngle(section, "laser_pose", 2, 0.0);

  // Max valid range
  this->max_range = cf->ReadLength(section, "max_range", 8.00);
  // Make range data NULL until allocation (can't allocate till we know how many we need)
  rangeData = NULL;

  memset (&position2d_data, 0, sizeof (player_position2d_data_t));
  memset (&laser_data, 0, sizeof (player_laser_data_t));

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int LodoDriver::Setup()
{
  if(!(this->position2d_driver = deviceTable->GetDevice(this->position2d_id)))
  {
    PLAYER_ERROR("unable to locate suitable position2d device");
    return(-1);
  }
  if(this->position2d_driver->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to postion2d device");
    return(-1);
  }
  

  // Subscribe to the laser device
  if(!(this->laser_driver = deviceTable->GetDevice(this->laser_id)))
  {
    PLAYER_ERROR("unable to locate suitable laser device");
    return(-1);
  }
  if(this->laser_driver->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to laser device");
    return(-1);
  }

  // Defer creation of lodo object so we can auto-dectect the
  // laser settings
  this->lodo = NULL;

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int LodoDriver::Shutdown()
{
  // Free lodo
  if (this->lodo)
    lodo_free(this->lodo);

  // Release required drivers
  this->laser_driver->Unsubscribe(this->InQueue);
  this->position2d_driver->Unsubscribe(this->InQueue);

  if (rangeData)
  {
    delete[] rangeData;
    rangeData = NULL;
  }

  return(0);
}


int LodoDriver::ProcessMessage (QueuePointer & resp_queue, player_msghdr *hdr, void *data)
{
  pose2_t pose;

  if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, this->position2d_id))
  {
    position2d_data = *reinterpret_cast<player_position2d_data_t*> (data);
    return 0;
  }
  else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN, this->laser_id))
  {
    laser_data = *reinterpret_cast<player_laser_data_t*> (data);
    this->laser_time = hdr->timestamp;
    Publish (out_laser_id, PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN, &laser_data, sizeof (player_laser_data_t), &laser_time);

    // Auto-detect range count and create lodo object
    if (this->lodo == NULL)
    {
      PLAYER_MSG1(1, "autodetected %d range readings", laser_data.ranges_count);
      if (laser_data.ranges_count != 181)
      {
        PLAYER_WARN("range readings != 181; please set your laser to 181 readings");
      }

      this->lodo = lodo_alloc(laser_data.ranges_count, laser_data.max_range, 0.1,
                              laser_data.min_angle, laser_data.resolution);
      this->lodo->laser_pose = this->laser_pose;
    }

    // Check for changes in the scan count
    if (laser_data.ranges_count != static_cast<unsigned int> (this->lodo->num_ranges))
    {
      PLAYER_ERROR2("incorrect range count; expecting %d, got %d",
                    laser_data.ranges_count, this->lodo->num_ranges);
      return -1;
    }

    // Massage odometric pose
    pose.pos = vector2_set(position2d_data.pos.px, position2d_data.pos.py);
    pose.rot = position2d_data.pos.pa;

    if (!rangeData)
    {
      if (!(rangeData = new double[laser_data.ranges_count]))
      {
        PLAYER_ERROR1 ("failed to allocate memory for %d range values", laser_data.ranges_count);
        return -1;
      }
    }
    for (unsigned int ii = 0; ii < laser_data.ranges_count; ii++)
      rangeData[ii] = static_cast<double> (laser_data.ranges[ii]);

    // Update and correct pose
    pose = lodo_add_scan(this->lodo, pose, laser_data.ranges_count, rangeData);

    // Store corrected robot pose
    this->robot_pose = pose;

    // Write position2d data; use the corrected pose and the laser timestamp
    memcpy(&this->out_position2d_data, &this->position2d_data, sizeof(this->out_position2d_data));
    this->out_position2d_data.pos.px = this->robot_pose.pos.x;
    this->out_position2d_data.pos.py = this->robot_pose.pos.y;
    this->out_position2d_data.pos.pa = this->robot_pose.rot;
    Publish (out_position2d_id, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, &out_position2d_data, sizeof (player_position2d_data_t), &laser_time);

    return 0;
  }

  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, -1, this->out_position2d_id))
  {
      // Forward the message
      position2d_driver->PutMsg(this->InQueue, hdr, data);
      // Store the return address for later use
      this->ret_queue = resp_queue;
      // Set the message filter to look for the response
      this->InQueue->SetFilter(this->position2d_id.host,
                             this->position2d_id.robot,
                             this->position2d_id.interf,
                             this->position2d_id.index,
                             -1,
                             hdr->subtype);
      // No response now; it will come later after we hear back from the
      // laser
      return(0);
  }

  // Forward responses (success or failure) from the position device
  else if(Device::MatchDeviceAddress(hdr->addr,position2d_id) &&
    (hdr->type == PLAYER_MSGTYPE_RESP_ACK || hdr->type == PLAYER_MSGTYPE_RESP_NACK))
  {
      // Copy in our address and forward the response
      hdr->addr = this->out_position2d_id;
      this->Publish(this->ret_queue, hdr, data);
      // Clear the filter
      this->InQueue->ClearFilter();
      // No response to send; we just sent it ourselves
      return(0);
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, -1, this->out_position2d_id))
  {
    // Forward the message
    position2d_driver->PutMsg(this->InQueue, hdr, data);
  }
  // set reply to value so the reply for this message goes straight to the given client
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, -1, this->out_laser_id))
  {
      // Forward the message
      laser_driver->PutMsg(this->InQueue, hdr, data);
      // Store the return address for later use
      this->ret_queue = resp_queue;
      // Set the message filter to look for the response
      this->InQueue->SetFilter(this->laser_id.host,
                             this->laser_id.robot,
                             this->laser_id.interf,
                             this->laser_id.index,
                             -1,
                             hdr->subtype);
      // No response now; it will come later after we hear back from the
      // laser
      return(0);
  }

  // Forward responses (success or failure) from the position device
  else if(Device::MatchDeviceAddress(hdr->addr,laser_id) &&
    (hdr->type == PLAYER_MSGTYPE_RESP_ACK || hdr->type == PLAYER_MSGTYPE_RESP_NACK))
  {
      // Copy in our address and forward the response
      hdr->addr = this->out_laser_id;
      this->Publish(this->ret_queue, hdr, data);
      // Clear the filter
      this->InQueue->ClearFilter();
      // No response to send; we just sent it ourselves
      return(0);
  }

  return -1;
}


