/** @ingroup drivers */
/** @{ */
/** @defgroup driver_bumpersafe bumpersafe
 * @brief Bumper monitor

This is a low level safety 'driver' that temporarily disables
velocity commands if a bumper is pressed. It sits on top of @ref
interface_bumper and @ref interface_position2d devices.

The general concept of this device is to not do much, but to provide
a last line of defense in the case that higher level drivers or client
software fails in its object avoidance.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d

@par Requires

- @ref interface_position2d : the underlying robot to be controlled
- @ref interface_bumper : the bumper to read from

@par Configuration requests

- PLAYER_POSITION2D_REQ_MOTOR_POWER : if motor is switched on then we
  reset the 'safe state' so robot can move with a bump panel active
- all other requests are just passed on to the underlying @ref
  interface_position2d device

@par Configuration file options

- none

@par Example

@verbatim
driver
(
  name "bumpersafe"
  provides ["position:0"]
  requires ["position:1" "bumper:0"]
)
@endverbatim

@author Toby Collett

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

class BumperSafe : public Driver
{
  public:
    // Constructor
    BumperSafe( ConfigFile* cf, int section);

    // Destructor
    virtual ~BumperSafe() {};

    // Setup/shutdown routines.
    virtual int Setup();
    virtual int Shutdown();

    // Underlying devices
    int SetupPosition();
    int ShutdownPosition();

    int SetupBumper();
    int ShutdownBumper();

    // Message Handler
  int ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr,
                               void * data);

  private:

    // state info
    bool Blocked;
    player_bumper_data_t CurrentState;
    player_bumper_data_t SafeState;

    // Position device info
    Device *position;
    player_devaddr_t position_id;
    int speed,turnrate;
    double position_time;
    bool position_subscribed;

    // Bumper device info
    Device *bumper;
    player_devaddr_t bumper_id;
    double bumper_time;
    player_bumper_geom_t bumper_geom;
    bool bumper_subscribed;
};

// Initialization function
Driver* BumperSafe_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new BumperSafe( cf, section)));
}

// a driver registration function
void BumperSafe_Register(DriverTable* table)
{
  table->AddDriver("bumper_safe",  BumperSafe_Init);
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int BumperSafe::Setup()
{
  Blocked = true;

  // Initialise the underlying devices.
  if (this->SetupPosition() != 0)
  {
    PLAYER_ERROR2("Bumber safe failed to connect to undelying position device %d:%d\n",position_id.interf, position_id.index);
    return -1;
  }
  if (this->SetupBumper() != 0)
  {
    PLAYER_ERROR2("Bumber safe failed to connect to undelying bumper device %d:%d\n",bumper_id.interf, bumper_id.index);
    return -1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int BumperSafe::Shutdown() {
  // Stop the laser
  this->ShutdownPosition();

  // Stop the odom device.
  this->ShutdownBumper();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int BumperSafe::ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr,
                               void * data)
{
  assert(hdr);
  assert(data);

  if (hdr->type==PLAYER_MSGTYPE_SYNCH)
  {
    return 0;
  }

  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_DATA, PLAYER_BUMPER_DATA_STATE, bumper_id))
  {
    // we got bumper data, we need to deal with this
    double time = hdr->timestamp;

    Lock();
    // Dont do anything if this is old data.
    if (time - bumper_time < 0.001)
      return 0;
    bumper_time = time;

    CurrentState = *reinterpret_cast<player_bumper_data *> (data);

    unsigned char hash = 0;
    for (unsigned int i = 0; i < CurrentState.bumpers_count; ++i)
      hash |= CurrentState.bumpers[i] & ~SafeState.bumpers[i];

    if (hash)
    {
      Blocked = true;
      Unlock();
      player_position2d_cmd_vel_t NullCmd = {{0}};

        position->PutMsg(InQueue,PLAYER_MSGTYPE_CMD,PLAYER_POSITION2D_CMD_VEL,&NullCmd,sizeof(NullCmd),NULL);
    }
    else
    {
      Blocked = false;
      SafeState = CurrentState;
      Unlock();
    }
    return 0;
  }

  if (Blocked && Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_MOTOR_POWER, device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_power_config_t));
    // if motor is switched on then we reset the 'safe state' so robot can move with a bump panel active
      if (((player_position2d_power_config_t *) data)->state == 1)
    {
      Lock();
      SafeState = CurrentState;
      Blocked = false;
      /*cmd.xspeed = 0;
      cmd.yspeed = 0;
      cmd.yawspeed = 0;*/
      Unlock();
    }
    Publish(device_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_MOTOR_POWER);
    return 0;
  }

  // set reply to value so the reply for this message goes straight to the given client
  if(Device::MatchDeviceAddress(hdr->addr,device_addr) && hdr->type == PLAYER_MSGTYPE_REQ)
  {
      // Forward the message
      position->PutMsg(this->InQueue, hdr, data);
      // Store the return address for later use
      this->ret_queue = resp_queue;
      // Set the message filter to look for the response
      this->InQueue->SetFilter(this->position_id.host,
                             this->position_id.robot,
                             this->position_id.interf,
                             this->position_id.index,
                             -1,
                             hdr->subtype);
      // No response now; it will come later after we hear back from the
      // laser
      return(0);
  }

  // Forward responses (success or failure) from the position device
  if(Device::MatchDeviceAddress(hdr->addr,position_id) &&
    (hdr->type == PLAYER_MSGTYPE_RESP_ACK || hdr->type == PLAYER_MSGTYPE_RESP_NACK))
  {
      // Copy in our address and forward the response
      hdr->addr = this->device_addr;
      this->Publish(this->ret_queue, hdr, data);
      // Clear the filter
      this->InQueue->ClearFilter();
      // No response to send; we just sent it ourselves
      return(0);
    }

  // Forward data from the position device
  if(Device::MatchDeviceAddress(hdr->addr,position_id) && hdr->type == PLAYER_MSGTYPE_DATA)
  {
      // Copy in our address and forward the response
      hdr->addr = this->device_addr;
      this->Publish(this->ret_queue, hdr, data);
      // No response to send; we just sent it ourselves
      return(0);
    }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
    Lock();
    if (!Blocked)
    {
      Unlock();
        position->PutMsg(InQueue,PLAYER_MSGTYPE_CMD,PLAYER_POSITION2D_CMD_VEL,data,hdr->size,&hdr->timestamp);
    }
    Unlock();
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_POS, device_addr))
  {
    assert(hdr->size == sizeof(player_position2d_cmd_pos_t));
    Lock();
    if (!Blocked)
    {
      Unlock();
        position->PutMsg(InQueue,PLAYER_MSGTYPE_CMD,PLAYER_POSITION2D_CMD_POS,data,hdr->size,&hdr->timestamp);
    }
    Unlock();
    return 0;
  }

  return -1;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the underlying position device.
int BumperSafe::SetupPosition()
{
  // Subscribe to the laser.
  if(Device::MatchDeviceAddress(this->position_id, this->device_addr))
  {
    PLAYER_ERROR("attempt to subscribe to self");
    return(-1);
  }
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

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the underlying position device.
int BumperSafe::ShutdownPosition()
{
  position->Unsubscribe(InQueue);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Set up the bumper
int BumperSafe::SetupBumper()
{
  if(!(this->bumper = deviceTable->GetDevice(this->bumper_id)))
  {
    PLAYER_ERROR("unable to locate suitable bumper device");
    return(-1);
  }
  if(this->bumper->Subscribe(this->InQueue) != 0)
  {
    PLAYER_ERROR("unable to subscribe to bumper device");
    return(-1);
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shut down the bumper
int BumperSafe::ShutdownBumper() {
  bumper->Unsubscribe(InQueue);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
BumperSafe::BumperSafe( ConfigFile* cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_POSITION2D_CODE)
{
  Blocked = false;

  this->position = NULL;
  // Must have a position device
  if (cf->ReadDeviceAddr(&this->position_id, section, "requires",
                       PLAYER_POSITION2D_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  this->position_time = 0.0;

  this->bumper = NULL;
  // Must have a bumper device
  if (cf->ReadDeviceAddr(&this->bumper_id, section, "requires",
                       PLAYER_BUMPER_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);
    return;
  }
  this->bumper_time = 0.0;

  return;
}
