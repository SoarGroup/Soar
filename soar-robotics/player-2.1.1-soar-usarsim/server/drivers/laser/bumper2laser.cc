/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004  Brian Gerkey gerkey@stanford.edu    
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
 * A driver to convert bumper readings to fake laser readings.
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_Bumper2Laser Bumper2Laser
 * @brief A driver to convert bumper readings to fake laser readings.

@par Provides

- @ref interface_laser

@par Requires

- @ref interface_bumper

@par Configuration requests

- None

@par Configuration file options

- pose (float tuple)
  - Default: [0.0 0.0 0.0]
  - Pose of the laser on the robot
- resolution (integer)
  - Default: 50 (which means 0.5 degree)
  - Angular scan resolution
- range_res (integer)
  - Default: 1
  - Range resolution
- occupied_value (float)
  - Default: 0.01
  - Value (in meters) returned by laser whenever bumper detects collision

@par Example 

@verbatim
driver
(
  name "bumper2laser"
  requires ["bumper:0"]
  provides ["laser:0"]
)
@endverbatim

@author Paul Osmialowski

*/

/** @} */

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <libplayercore/playercore.h>

extern PlayerTime * GlobalTime;

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif
#ifndef DTOR
#define DTOR(d) ((d) * M_PI / 180.0)
#endif
#ifndef RTOD
#define RTOD(r) ((r) * 180.0 / M_PI)
#endif

class Bumper2Laser : public Driver
{
  public:    
    // Constructor; need that
    Bumper2Laser(ConfigFile * cf, int section);

    virtual ~Bumper2Laser();

    // Must implement the following methods.
    virtual int Setup();
    virtual int Shutdown();

    // This method will be invoked on each incoming message
    virtual int ProcessMessage(QueuePointer & resp_queue, 
                               player_msghdr * hdr,
                               void * data);

  private:
    // Main function for device thread.
    virtual void Main();

    // The address of the bumper device to which we will
    // subscribe
    player_devaddr_t bumper_addr;

    player_devaddr_t laser_addr;

    // Handle for the device that have the address given above
    Device * bumper_dev;

    // Latest data from the bumper
    player_bumper_data_t bumper_data;
    // Flag to tell that bumper_data is valid
    bool bumper_data_valid;

    player_laser_data_t laser_data;

    // Laser pose in robot cs.
    double pose[3];
    double size[2];
    // Scan width and resolution.
    int scan_width, scan_res;

    // Start and end scan angles (for restricted scan).  These are in
    // units of 0.01 degrees.
    int min_angle, max_angle;
    
    // Start and end scan segments (for restricted scan).  These are
    // the values used by the laser.
    int scan_min_segment, scan_max_segment;

    // Range resolution (1 = 1mm, 10 = 1cm, 100 = 10cm).
    int range_res;

    // Turn intensity data on/off
    bool intensity;
    
    float occupied_value;
    
    int scan_id;
};

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
//
// This driver will support the laser interface, and it will subscribe
// to the bumper interface.
//
Bumper2Laser::Bumper2Laser(ConfigFile * cf, int section)
//  : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_LASER_CODE)
    : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
  memset(&(this->bumper_addr), 0, sizeof(player_devaddr_t));
  memset(&(this->laser_addr), 0, sizeof(player_devaddr_t));
  memset(&(this->bumper_data), 0, sizeof this->bumper_data);
  memset(&(this->laser_data), 0, sizeof this->laser_data);
  if (cf->ReadDeviceAddr(&(this->laser_addr), section, "provides",
                         PLAYER_LASER_CODE, -1, NULL))
  {
    this->SetError(-1);
    return;
  }
  if (this->AddInterface(this->laser_addr))
  {
    this->SetError(-1);
    return;
  }
  if (cf->ReadDeviceAddr(&(this->bumper_addr), section, "requires",
                         PLAYER_BUMPER_CODE, -1, NULL))
  {
    this->SetError(-1);
    return;
  }
  // Laser geometry
  this->pose[0] = cf->ReadTupleLength(section, "pose", 0, 0.0);
  this->pose[1] = cf->ReadTupleLength(section, "pose", 1, 0.0);;
  this->pose[2] = cf->ReadTupleLength(section, "pose", 2, 0.0);;
  this->size[0] = 0.15;
  this->size[1] = 0.15;
  this->scan_width = 180;
  this->scan_res = cf->ReadInt(section, "resolution", 50);
  if((this->scan_res != 25) && 
     (this->scan_res != 50) && 
     (this->scan_res != 100))
  {
    PLAYER_ERROR1("Invalid angular resolution %d. Defaulting to 50 (0.5 degree)", 
                  this->scan_res);
    this->scan_res = 50;
  }
  this->min_angle = -9000;
  this->max_angle = +9000;
  this->scan_min_segment = 0;
  this->scan_max_segment = 360;
  this->intensity = false;
  this->range_res = cf->ReadInt(section, "range_res", 1);
  this->occupied_value = cf->ReadFloat(section, "occupied_value", 0.01);
  this->scan_id = 0;
}

Bumper2Laser::~Bumper2Laser()
{
  if (this->bumper_data.bumpers) delete [](this->bumper_data.bumpers);
  this->bumper_data.bumpers = NULL;
  if (this->laser_data.ranges) delete [](this->laser_data.ranges);
  this->laser_data.ranges = NULL;
  if (this->laser_data.intensity) delete [](this->laser_data.intensity);
  this->laser_data.intensity = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int Bumper2Laser::Setup()
{
  // We have not yet received any data
  this->bumper_data_valid = false;
  
  // Retrieve the handle to the bumper device.
  this->bumper_dev = deviceTable->GetDevice(this->bumper_addr);
  if (!(this->bumper_dev))
  {
    PLAYER_ERROR("unable to locate suitable bumper device");
    return -1;
  }
  // Subscribe my message queue the bumper device.
  if (this->bumper_dev->Subscribe(this->InQueue))
  {
    PLAYER_ERROR("unable to subscribe to bumper device");
    return -1;
  }

  // Start the device thread; spawns a new thread and executes
  // Bumper2Laser::Main(), which contains the main loop for the driver.
  StartThread();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int Bumper2Laser::Shutdown()
{
  // Stop and join the driver thread
  StopThread();
    
  // Unsubscribe from the bumper
  this->bumper_dev->Unsubscribe(this->InQueue);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Bumper2Laser::Main() 
{
  struct timespec tspec;
  double time;
  unsigned int old_count;
  unsigned int half;
  float left, right;

  // The main loop; read-think-act
  for(;;)
  {
    // Wait till we get new messages
    this->InQueue->Wait();

    // Test if we are supposed to cancel
    pthread_testcancel();

    // Process incoming messages
    ProcessMessages();

    // Don't do anything if we don't have valid data yet
    if (!(this->bumper_data_valid)) continue;
    if ((this->bumper_data.bumpers_count) <= 0) continue;
    assert((this->bumper_data.bumpers_count) == 2);
    this->bumper_data_valid = false;

    // Get the time at which we started reading
    // This will be a pretty good estimate of when the phenomena occured
    GlobalTime->GetTimeDouble(&time);

    // Prepare packet
    this->laser_data.min_angle = DTOR((this->scan_min_segment * this->scan_res) / 1e2 - this->scan_width / 2.0);
    this->laser_data.max_angle = DTOR((this->scan_max_segment * 
                                       this->scan_res) / 1e2  - 
                                       this->scan_width / 2.0);
    if(this->range_res == 1) this->laser_data.max_range = 8.0;
    else if(this->range_res == 10) this->laser_data.max_range = 80.0;
    else if(this->range_res == 100) this->laser_data.max_range = 150.0;
    else
    {
      PLAYER_WARN("Invalid range_res!");
      this->laser_data.max_range = 8.0;
    }
    this->laser_data.resolution = DTOR(this->scan_res / 1e2);
    old_count = this->laser_data.ranges_count; 
    this->laser_data.ranges_count = this->scan_max_segment - this->scan_min_segment + 1;
    this->laser_data.intensity_count = this->laser_data.ranges_count;
    if (old_count < (this->laser_data.ranges_count))
    {
      if (this->laser_data.ranges) delete [](this->laser_data.ranges);
      if (this->laser_data.intensity) delete [](this->laser_data.intensity);
      this->laser_data.ranges = new float[this->laser_data.ranges_count];
      assert(this->laser_data.ranges);
      this->laser_data.intensity = new uint8_t[this->laser_data.intensity_count];
      assert(this->laser_data.intensity);
    }
    half = (this->laser_data.ranges_count) / 2;
    left = (this->bumper_data.bumpers[0]) ? this->occupied_value : this->laser_data.max_range;
    right = (this->bumper_data.bumpers[1]) ? this->occupied_value : this->laser_data.max_range;
    left *= ((float)(this->range_res));
    right *= ((float)(this->range_res));
    for (int i = 0; i < (int)(this->laser_data.ranges_count); i++)
    {
      this->laser_data.intensity[i] = 0xff;
      if (i < (int)half) this->laser_data.ranges[i] = right;
      else this->laser_data.ranges[i] = left;
    }
    this->laser_data.id = this->scan_id++;
    // You should also publish some data
    this->Publish(this->laser_addr,
                  PLAYER_MSGTYPE_DATA, PLAYER_LASER_DATA_SCAN,
                  (void *)(&(this->laser_data)), 0, &time);
    // sleep for a while
    tspec.tv_sec = 0;
    tspec.tv_nsec = 5000;
    nanosleep(&tspec, NULL);
  }
}

int Bumper2Laser::ProcessMessage(QueuePointer & resp_queue,
                                 player_msghdr * hdr,
                                 void * data)
{
  // Process messages here.  Send a response if necessary, using Publish().
  // If you handle the message successfully, return 0.  Otherwise,
  // return -1, and a NACK will be sent for you, if a response is required.
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_LASER_REQ_SET_CONFIG, 
                           this->laser_addr))
  {
    player_laser_config_t * config = 
            reinterpret_cast<player_laser_config_t *> (data);
    this->intensity = config->intensity;
    this->scan_res =  (int)rint(RTOD(config->resolution)*100);
    this->min_angle = (int)rint(RTOD(config->min_angle)*100);
    this->max_angle = (int)rint(RTOD(config->max_angle)*100);
    this->range_res = (int)config->range_res*1000;
    // Configuration succeeded; send the new config back in the ACK
    this->Publish(this->laser_addr,
                  resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_LASER_REQ_SET_CONFIG,
                  data, hdr->size, NULL);

    // Return 0 to say that we handled this message
    return 0;
  }
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                            PLAYER_LASER_REQ_GET_CONFIG,
                            this->laser_addr))
  {
    player_laser_config_t config;
    config.intensity = this->intensity;
    config.resolution = DTOR(this->scan_res)/100;
    config.min_angle = DTOR(this->min_angle)/100;
    config.max_angle = DTOR(this->max_angle)/100;
    if(this->range_res == 1)
      config.max_range = 8.0;
    else if(this->range_res == 10)
      config.max_range = 80.0;
    else if(this->range_res == 100)
      config.max_range = 150.0;
    else
    {
      PLAYER_WARN("Invalid range_res!");
      config.max_range = 8.0;
    }
    config.range_res = ((double)this->range_res)/1000.0;

    this->Publish(this->laser_addr,
                  resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_LASER_REQ_GET_CONFIG,
                  (void*)&config, sizeof(config), NULL);
    return 0;
  }
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                            PLAYER_LASER_REQ_GET_GEOM,
                            this->laser_addr))
  {
    player_laser_geom_t geom;
    memset(&geom, 0, sizeof(geom));
    geom.pose.px = this->pose[0];
    geom.pose.py = this->pose[1];
    geom.pose.pyaw = this->pose[2];
    geom.size.sl = this->size[0];
    geom.size.sw = this->size[1];

    this->Publish(this->laser_addr,
                  resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_LASER_REQ_GET_GEOM,
                  (void*)&geom, sizeof(geom), NULL);
    return 0;
  }
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA,
                            -1, // -1 means 'all message subtypes'
                            this->bumper_addr))
  { // Is it new bumper data?
    if (this->bumper_data.bumpers) delete [](this->bumper_data.bumpers);
    // Cast the message to the appropriate type and store it
    this->bumper_data = *((player_bumper_data_t *)data);
    this->bumper_data.bumpers = NULL;
    if ((bumper_data.bumpers_count) != 2)
    {
      this->bumper_data_valid = false;
    } else
    {
      this->bumper_data.bumpers = new unsigned char[bumper_data.bumpers_count];
      assert(this->bumper_data.bumpers);
      memcpy(this->bumper_data.bumpers, ((player_bumper_data_t *)data)->bumpers, bumper_data.bumpers_count);
      this->bumper_data_valid = true;
    }
    return 0;
  }
  return -1;
}

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver * Bumper2Laser_Init(ConfigFile * cf, int section)
{
  // Create and return a new instance of this driver
  return (Driver *)(new Bumper2Laser(cf, section));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void Bumper2Laser_Register(DriverTable * table)
{
  table->AddDriver("bumper2laser", Bumper2Laser_Init);
}
