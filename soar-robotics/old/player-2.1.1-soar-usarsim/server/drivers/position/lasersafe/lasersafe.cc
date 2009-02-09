/** @ingroup drivers */
/** @{ */
/** @defgroup driver_lasersafe lasersafe
 * @brief Bumper monitor

This is a low level safety 'driver' that temporarily disables
velocity commands if a laser range is detected within a specified
safe distance. It sits on top of @ref interface_laser and
@ref interface_position2d devices.

The general concept of this device is to not do much, but to provide
a last line of defense in the case that higher level drivers or client
software fails in its object avoidance.
When the laser scanner detects an obstacle within the safe distance, it
will prevent the robot from moving either forwards or backwards
(depending on which way the laser scanner is facing). For example, if
the laser scanner is facing forwards and it detects an obstacle, the
robot will only be able to back away, not continue forwards into the
obstacle.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d

@par Requires

- @ref interface_position2d : the underlying robot to be controlled
- @ref interface_laser : the laser to read from

@par Configuration requests

- PLAYER_POSITION2D_REQ_MOTOR_POWER : if motor is switched on then we
  reset the 'safe state' so robot can move with a bump panel active
- all other requests are just passed on to the underlying @ref
  interface_position2d device

@par Configuration file options

- safedistance
  Default: 0.4m
  The distance at which to stop
- step
  Default: 5
  The number of range readings to skip over. For example, the default
  value will check readings 0, 5, 10, and so on. A value of 10 will
  check readings 0, 10, 20, 30, ...
- history
  Default: 3
  The driver will use a moving average of range readings to help
  overcome noise issues. This specifies the number of readings to
  consider in total (so a value of 3 gives the current plus the two
  previous readings).
  If set to 1, only the most recent scan data will be used.
- forward
  Default: 1
  Indicates if the laser scanner is pointing forwards (1) or backwards (0).
- boxmode
  Default: 1
  If 1, the driver uses a box model for the safety area instead of a radius
  distance from the laser scanner. This can allow you to, for example, ensure
  that the robot can pass through narrow passages without driving into an
  object ahead. Set to 0 to use the radius mode.
- boxwidth
  Default: -1m
  The width of the box. If less than zero, the position2d device will be
  queried for the width of the robot and that will be used as the box width.
- boxsafety
  default: 0.1
  A safety margin to use if getting the width of the robot for box mode. Won't
  be used if specifying the width of the box manually in the config file. The
  default of 0.1 is a 10% safety margin, 0.25 would be 25%, and so on.

TODO: Make driver more advanced so that it can find the laser's pose and
allow movement in the opposite direction to the way the laser is pointing.

@par Example

@verbatim
driver
(
  name "lasersafe"
  provides ["position2d:0"]
  requires ["position2d:1" "laser:0"]
  safedistance 0.3
  step 10
  history 1
)
@endverbatim

@author Toby Collett, Geoffrey Biggs

*/
/** @} */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#include <libplayercore/playercore.h>
#include <libplayercore/error.h>

class LaserSafe : public Driver
{
  public:
    // Constructor
    LaserSafe (ConfigFile* cf, int section);

    // Destructor
    virtual ~LaserSafe () {};

    // Setup/shutdown routines.
    virtual int Setup ();
    virtual int Shutdown ();

    // Underlying devices
    int SetupPosition ();
    int ShutdownPosition ();

    int SetupLaser ();
    int ShutdownLaser ();

    // Message Handler
    int ProcessMessage (QueuePointer & resp_queue, player_msghdr * hdr, void * data);

  private:

    bool ScanInRange (double scanDistance, double scanAngle);

    // state info
    bool Blocked;
    player_laser_data_t CurrentState;
    player_laser_data_t *history;
//    player_laser_data_t SafeState;

    // Position device info
    Device *position;
    player_devaddr_t position_id;
    int speed,turnrate;
    double position_time;
    bool position_subscribed;

    // Laser device info
    Device *laser;
    player_devaddr_t laser_id;
    double laser_time;
    bool laser_subscribed;

    // State info
    double safeDistance;
    unsigned int step;
    unsigned int historyLength;
    unsigned int currentHistSlot;
    bool front;
    bool boxMode;
    double boxWidth;
    double boxSafety;
    bool needPoseInfo, gotPoseInfo;
};

// Initialization function
Driver* LaserSafe_Init (ConfigFile* cf, int section)
{
  return ((Driver*) (new LaserSafe (cf, section)));
}

// a driver registration function
void LaserSafe_Register(DriverTable* table)
{
  table->AddDriver ("lasersafe",  LaserSafe_Init);
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int LaserSafe::Setup ()
{
  // Initialise the underlying devices.
  if (SetupPosition () != 0)
  {
    PLAYER_ERROR2 ("Laser safe failed to connect to undelying position2d device %d:%d\n", position_id.interf, position_id.index);
    return -1;
  }
  if (SetupLaser () != 0)
  {
    PLAYER_ERROR2 ("Laser safe failed to connect to undelying laser device %d:%d\n", laser_id.interf, laser_id.index);
    return -1;
  }

  Blocked = true;

  // Create enough laser history
  if (historyLength > 1)
  {
    if (!(history = new player_laser_data_t[historyLength - 1]))
    {
      PLAYER_WARN1 ("Laser safe failed to create history buffer for history length %d, falling back to no history\n", historyLength);
      historyLength = 1;
      history = NULL;
    }
    else
    {
      memset (history, 0, sizeof (player_laser_data_t) * (historyLength - 1));
    }
    currentHistSlot = 0;
  }

  // Send a request for the robot's width from the position driver if need it
  if (needPoseInfo)
  {
    position->PutMsg (InQueue, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_GET_GEOM, NULL, 0, NULL);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int LaserSafe::Shutdown ()
{
  // Stop the laser
  ShutdownPosition ();

  // Stop the odom device.
  ShutdownLaser ();

  // Delete the history (if present)
  if (history)
  {
    delete[] history;
    history = NULL;
  }

  gotPoseInfo = false;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Check if a laser scan distance is within warning distance
bool LaserSafe::ScanInRange (double scanDistance, double scanAngle)
{
  if (boxMode)
  {
    double x, y;

    x = scanDistance * cos (scanAngle);
    y = scanDistance * sin (scanAngle);

    if (x < safeDistance && fabs (y) < boxWidth)    // Box is centered on laser
    {
      return true;
    }
    return false;
  }
  else
  {
    if (scanDistance < safeDistance)
      return true;
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int LaserSafe::ProcessMessage (QueuePointer & resp_queue, player_msghdr * hdr, void * data)
{
  assert(hdr);
  assert(data);

  if (hdr->type==PLAYER_MSGTYPE_SYNCH)
  {
    return 0;
  }

  if(Message::MatchMessage (hdr, PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN, laser_id))
  {
    // we got laser data, we need to deal with this
    double time = hdr->timestamp;
    bool hit = false;

    Lock ();
    // Dont do anything if this is old data.
    if (time - laser_time < 0.001)
      return 0;
    laser_time = time;

    CurrentState = *reinterpret_cast<player_laser_data *> (data);

    if (history)
    {
      double accumulated = 0.0f;
      double scanAngle = CurrentState.min_angle;
      unsigned int ii = 0;
      for (ii = 0; ii < CurrentState.ranges_count; ii += step)
      {
        accumulated = 0.0f;
        for (unsigned int jj = 0; jj < historyLength - 1; jj++)
        {
          accumulated += history[jj].ranges[ii];
        }
        accumulated += CurrentState.ranges[ii];
        if (ScanInRange (accumulated / static_cast<double> (historyLength), scanAngle))
        {
          hit = true;
          break;
        }
        // The history buffer is circular, so the current data needs to go into the correct index
        history[currentHistSlot].ranges[ii] = CurrentState.ranges[ii];
        scanAngle += (CurrentState.resolution * step);
      }
      // Copy any remaining history after encountering a hit
      for (; ii < CurrentState.ranges_count; ii += step)
        history[currentHistSlot].ranges[ii] = CurrentState.ranges[ii];
      // Increment the history slot counter and wrap it
      currentHistSlot = (currentHistSlot + 1) % (historyLength - 1);
    }
    else
    {
      double scanAngle = CurrentState.min_angle;
      // If no history then just check the current data
      for (unsigned int ii = 0; ii < CurrentState.ranges_count; ii += step)
      {
        if (ScanInRange (CurrentState.ranges[ii], scanAngle))
        {
          hit = true;
          break;
        }
        scanAngle += (CurrentState.resolution * step);
      }
    }

    if (hit)
    {
      Blocked = true;
      Unlock ();
      player_position2d_cmd_vel_t NullCmd = {{0}};

      position->PutMsg (InQueue, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, &NullCmd, sizeof (NullCmd), NULL);
    }
    else
    {
      Blocked = false;
      Unlock ();
    }

    return 0;
  }
  // set reply to value so the reply for this message goes straight to the given client
  if (Device::MatchDeviceAddress (hdr->addr, device_addr) && hdr->type == PLAYER_MSGTYPE_REQ)
  {
    // Forward the message
    position->PutMsg (InQueue, hdr, data);
    // Store the return address for later use
    ret_queue = resp_queue;
    // Set the message filter to look for the response
    InQueue->SetFilter (position_id.host,
                        position_id.robot,
                        position_id.interf,
                        position_id.index,
                        -1,
                        hdr->subtype);
    // No response now; it will come later after we hear back from the
    // laser
    return 0;
  }

  // Forward responses (success or failure) from the position device
  if (Device::MatchDeviceAddress (hdr->addr, position_id) &&
     (hdr->type == PLAYER_MSGTYPE_RESP_ACK || hdr->type == PLAYER_MSGTYPE_RESP_NACK))
  {
    if (!gotPoseInfo && hdr->type == PLAYER_MSGTYPE_RESP_ACK && hdr->subtype == PLAYER_POSITION2D_REQ_GET_GEOM)
    {
      boxWidth = reinterpret_cast<player_position2d_geom_t*> (data)->size.sw;
	  boxWidth += boxWidth * boxSafety;     // Add safety margin to the robot's width
      boxWidth /= 2.0f;
      gotPoseInfo = true;
    }
    else
    {
      // Copy in our address and forward the response
      hdr->addr = device_addr;
      Publish (ret_queue, hdr, data);
      // Clear the filter
      InQueue->ClearFilter ();
    }
    // No response to send; we just sent it ourselves
    return 0;
  }

  // Forward data from the position device
  if (Device::MatchDeviceAddress (hdr->addr, position_id) && hdr->type == PLAYER_MSGTYPE_DATA)
  {
    // Copy in our address and forward the response
    hdr->addr = device_addr;
    Publish (ret_queue, hdr, data);
    // No response to send; we just sent it ourselves
    return 0;
  }

  if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, device_addr))
  {
    assert (hdr->size == sizeof (player_position2d_cmd_vel_t));
    bool fwdMove = reinterpret_cast<player_position2d_cmd_vel_t*> (data)->vel.px > 0 ? true : false;
    Lock ();
    if (!Blocked || (Blocked && front && !fwdMove) || (Blocked && !front && fwdMove))
    {
      Unlock ();
      position->PutMsg (InQueue, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, data, hdr->size, &hdr->timestamp);
    }
    else
      Unlock ();
    return 0;
  }

  if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_POS, device_addr))
  {
    assert (hdr->size == sizeof (player_position2d_cmd_pos_t));
    bool fwdMove = reinterpret_cast<player_position2d_cmd_pos_t*> (data)->pos.px > 0 ? true : false;
    Lock ();
    if (!Blocked || (Blocked && front && !fwdMove) || (Blocked && !front && fwdMove))
    {
      Unlock ();
      position->PutMsg (InQueue, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_POS, data, hdr->size, &hdr->timestamp);
    }
    else
      Unlock ();
    return 0;
  }

  return -1;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the underlying position device.
int LaserSafe::SetupPosition ()
{
  // Subscribe to the position.
  if (Device::MatchDeviceAddress (position_id, device_addr))
  {
    PLAYER_ERROR ("attempt to subscribe to self");
    return -1;
  }
  if (!(position = deviceTable->GetDevice (position_id)))
  {
    PLAYER_ERROR ("unable to locate suitable position2d device");
    return -1;
  }
  if (position->Subscribe (InQueue) != 0)
  {
    PLAYER_ERROR ("unable to subscribe to position2d device");
    return -1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the underlying position device.
int LaserSafe::ShutdownPosition ()
{
  position->Unsubscribe (InQueue);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the bumper
int LaserSafe::SetupLaser ()
{
  if (!(laser = deviceTable->GetDevice(laser_id)))
  {
    PLAYER_ERROR ("unable to locate suitable laser device");
    return -1;
  }
  if (laser->Subscribe(InQueue) != 0)
  {
    PLAYER_ERROR ("unable to subscribe to laser device");
    return -1;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shut down the bumper
int LaserSafe::ShutdownLaser () {
  laser->Unsubscribe (InQueue);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
LaserSafe::LaserSafe (ConfigFile* cf, int section)
  : Driver (cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_POSITION2D_CODE)
{
  Blocked = false;
  gotPoseInfo = false;
  needPoseInfo = false;

  position = NULL;
  // Must have a position device
  if (cf->ReadDeviceAddr (&position_id, section, "requires",
                       PLAYER_POSITION2D_CODE, -1, NULL) != 0)
  {
    SetError (-1);
    return;
  }
  position_time = 0.0;

  laser = NULL;
  // Must have a laser device
  if (cf->ReadDeviceAddr (&laser_id, section, "requires",
                       PLAYER_LASER_CODE, -1, NULL) != 0)
  {
    SetError (-1);
    return;
  }
  laser_time = 0.0;
  safeDistance = cf->ReadLength (section, "safedistance", 0.4);
  step = cf->ReadInt (section, "step", 5);
  historyLength = cf->ReadInt (section, "history", 1);
  history = NULL;
  int temp = cf->ReadInt (section, "forward", 1);
  front = temp > 0 ? true : false;
  temp = cf->ReadInt (section, "boxmode", 1);
  boxMode = temp > 0 ? true : false;
  boxWidth = cf->ReadLength (section, "boxwidth", -1.0f);
  boxSafety = cf->ReadFloat (section, "boxsafety", 0.1);

  if (boxWidth < 0.0f && boxMode)
    needPoseInfo = true;  // Don't need the pose info if not in box mode or specified in config
  else
    boxWidth /= 2.0f;   // Box is always centred on laser

  return;
}
