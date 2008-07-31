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

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_urglaser urglaser
 * @brief Hokuyo URG laser range-finder

The urglaser driver controls the Hokuyo URG scanning laser range-finder.
Communication with the laser can be either via USB or RS232. The driver
supports both Hokuyo's SCIP1 and 2 protocols.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_laser

@par Requires

- none

@par Configuration requests

- PLAYER_LASER_REQ_GET_GEOM
- PLAYER_LASER_REQ_GET_CONFIG
- PLAYER_LASER_REQ_SET_CONFIG
- PLAYER_LASER_REQ_GET_ID

@par Configuration file options

- port (string)
  - Default: "/dev/ttyACM0"
  - Port to which the laser is connected.  Can be either a serial port or
    the port associated with USB acm device.  See use_serial.

- pose (float tuple m m rad)
  - Default: [0.0 0.0 0.0]
  - Pose (x,y,theta) of the laser, relative to its parent object (e.g.,
    the robot to which the laser is attached).

- min_angle, max_angle (angle float)
  - Default: [-2.094 2.094] (or [-120.0 120.0] in degrees)
  - Minimum and maximum scan angles to return

- use_serial (integer)
  - Default: 0
  - If non-zero, communicate via RS232 instead of USB.

- baud (integer)
  - Default: 115200
  - Baud rate to use when communicating with the laser over RS232.  Valid
    rates are: 19200, 57600, and 115200.  The driver will auto-detect the
    current rate then change to the desired rate.

@par Example

@verbatim
driver
(
  name "urglaser"
  provides ["laser:0"]
  port "/dev/ttyACM0"
)
@endverbatim

@author Toby Collett, Nico Blodow

*/
/** @} */

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <math.h>
#include <termios.h>

#include <replace/replace.h>
using namespace std;

#include "urg_laser.h"

#define URG04_MIN_STEP 44
#define URG04_MAX_STEP 725

#include <libplayercore/playercore.h>

class URGLaserDriver : public Driver
{
  public:

	// Constructor;
	URGLaserDriver(ConfigFile* cf, int section);
	// Destructor
	~URGLaserDriver();

	// Implementations of virtual functions
	int Setup();
	int Shutdown();

	// This method will be invoked on each incoming message
	virtual int ProcessMessage(QueuePointer & resp_queue, 
                               player_msghdr * hdr,
                               void * data);

  private:
	// Main function for device thread.
	virtual void Main();

	urg_laser_readings_t * Readings;
	urg_laser Laser;

	player_laser_data_t Data;
	player_laser_geom_t Geom;
	player_laser_config_t Conf;

	bool UseSerial;
	int BaudRate, min_i, max_i;
	float user_min_angle, user_max_angle;
	char * Port;
};


////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
URGLaserDriver::URGLaserDriver (ConfigFile* cf, int section)
: Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_LASER_CODE)
{
  //init vars
  memset (&Data, 0, sizeof (Data));
  memset (&Geom, 0, sizeof (Geom));
  memset (&Conf, 0, sizeof (Conf));
  Geom.size.sw = (0.050);
  Geom.size.sl = (0.050);

  Readings = new urg_laser_readings_t;
  assert (Readings);

  // read options from config file
  Geom.pose.px = (cf->ReadTupleLength (section, "pose", 0, 0));
  Geom.pose.py = (cf->ReadTupleLength (section, "pose", 1, 0));
  Geom.pose.pyaw = (cf->ReadTupleAngle  (section, "pose", 2, 0));

  // set up config structure
  Conf.min_angle = cf->ReadAngle (section, "min_angle", DTOR (-120));
  Conf.max_angle = cf->ReadAngle (section, "max_angle", DTOR (120));
  user_min_angle = Conf.min_angle;
  user_max_angle = Conf.max_angle;
  
//  Conf.resolution = DTOR (270.0/769.0);
  Conf.resolution = DTOR (360.0/1024.0);
  Conf.max_range = 4.0;
  Conf.range_res = 0.001;
  Conf.intensity = 0;
	
  int b = cf->ReadInt (section, "baud", 115200);
  switch (b)
  {
    case 115200:
      BaudRate = B115200;
      break;
    case 57600:
      BaudRate = B57600;
      break;
    case 19200:
      BaudRate = B19200;
      break;
    default:
      PLAYER_WARN1 ("ignoring invalid baud rate %d", b);
      BaudRate = B115200;
      break;
  }

  Port = strdup (cf->ReadString (section, "port", "/dev/ttyACM0"));
  UseSerial = (cf->ReadInt (section, "use_serial", 0) == 1);
}

////////////////////////////////////////////////////////////////////////////////
URGLaserDriver::~URGLaserDriver ()
{
  delete Readings;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int
  URGLaserDriver::Setup ()
{
  //config data
  if (Laser.Open (Port, UseSerial, BaudRate) < 0)
  {
    this->SetError(1);
    return -1;
  }
  Laser.GetSensorConfig (&Conf);

  int half_idx = Laser.GetNumRanges() / 2; 
  
  // Solve the min/max angle problem
  min_i = static_cast<int> (round (half_idx + Conf.min_angle/Conf.resolution));
  max_i = static_cast<int> (round (half_idx + Conf.max_angle/Conf.resolution));
  
  // For ancient firmware versions, set some hard limits on the min/max angle capabilities
  if(Laser.GetSCIPVersion() < 3)
  {
    if (min_i < URG04_MIN_STEP)
      min_i = URG04_MIN_STEP;
    if (max_i > URG04_MAX_STEP)
      max_i = URG04_MAX_STEP;
  }

  int user_min_i = static_cast<int> (round (half_idx + user_min_angle/Conf.resolution));
  int user_max_i = static_cast<int> (round (half_idx + user_max_angle/Conf.resolution));
  
  if (user_min_i > user_max_i)
    user_min_i = user_max_i;
    
  // We restrict the URG 04-LX to its capabilities
  if (user_min_i < min_i)
  {
    PLAYER_WARN2 ("restricting the lower index bound to %d from %d", min_i, user_min_i);
    user_min_i = min_i;
  }
  min_i = user_min_i;

  if (user_max_i > max_i)
  {
    PLAYER_WARN2 ("restricting the upper index bound to %d from %d", max_i, user_max_i);
    user_max_i = max_i;
  }
  max_i = user_max_i;
  
  Conf.min_angle = (min_i - half_idx) * Conf.resolution;
  Conf.max_angle = (max_i - half_idx) * Conf.resolution;
  
  // Start the device thread; spawns a new thread and executes
  // ExampleDriver::Main(), which contains the main loop for the driver.
  StartThread ();
  
  return (0);
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int
  URGLaserDriver::Shutdown ()
{
  // Stop and join the driver thread
  StopThread ();

  Laser.Close ();

  return (0);
}


////////////////////////////////////////////////////////////////////////////////
int
  URGLaserDriver::ProcessMessage (QueuePointer &resp_queue,
                                  player_msghdr * hdr,
                                  void * data)
{
  if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ,
                             PLAYER_LASER_REQ_GET_GEOM, this->device_addr))
    Publish (device_addr,resp_queue, PLAYER_MSGTYPE_RESP_ACK,
             hdr->subtype, &Geom, sizeof (Geom), NULL);
    
  else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ,
		                  PLAYER_LASER_REQ_GET_CONFIG, this->device_addr))
  {
    Publish (device_addr,resp_queue, PLAYER_MSGTYPE_RESP_ACK,
             hdr->subtype, &Conf, sizeof (Conf), NULL);
  }
  else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_REQ,
		                  PLAYER_LASER_REQ_GET_ID, this->device_addr))
  {
    player_laser_get_id_config_t player_ID_conf;
    // Get laser identification information
    player_ID_conf.serial_number = Laser.GetIDInfo ();
		
    Publish (device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype, 
             &player_ID_conf, sizeof (player_ID_conf), NULL);
  }
  else
    return (-1);
  
  return (0);
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void
  URGLaserDriver::Main ()
{
  // The main loop; interact with the device here
  for (;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel ();

    // Process any pending messages
    ProcessMessages ();

    // update device data
    Laser.GetReadings (Readings, min_i, max_i);
    Data.min_angle = Conf.min_angle;
    Data.max_angle = Conf.max_angle;
    
    // TODO: check this value
    Data.max_range    = Conf.max_range;
    Data.resolution   = Conf.resolution;
    Data.ranges_count = (max_i - min_i);
    Data.ranges = new float [Data.ranges_count];
    // TODO: look into getting intensity data
    Data.intensity_count = 0;
    
    for (unsigned int i = 0; i < Data.ranges_count; ++i)
    {
        
      Data.ranges[i]  = Readings->Readings[i+min_i] < 20 ? (Data.max_range*1000) : (Readings->Readings[i+min_i]);
      Data.ranges[i] /= 1000;
    }
    Publish (device_addr, PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN,
             &Data);
    delete [] Data.ranges;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Things for building shared object, and functions for registering and creating
//  new instances of the driver
////////////////////////////////////////////////////////////////////////////////

// Factory creation function. This functions is given as an argument when
// the driver is added to the driver table
Driver*
  URGLaserDriver_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return ((Driver*)(new URGLaserDriver (cf, section)));
}

// Registers the driver in the driver table. Called from the
// player_driver_init function that the loader looks for
int 
  URGLaserDriver_Register (DriverTable* table)
{
  table->AddDriver ("urglaser", URGLaserDriver_Init);
  return (0);
}
