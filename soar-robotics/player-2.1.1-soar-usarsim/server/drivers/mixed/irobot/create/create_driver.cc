/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2006 -
 *     Brian Gerkey
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
/** @defgroup driver_create create
 @brief iRobot Create

Newer versions of the iRobot Create vaccum robot can be controlled by an
external computer over a serial line.  This driver supports control of
these robots.  

Note that the serial port on top of the Create operates at 5V, not the
RS232 standard of 12V.  This means that you cannot just plug a plain
old serial cable between the Create and your PC's serial port.  You need
to put a level-shifter in between them.  Or you if have a computer that
exposes serial lines at "logic level," (e.g., the Gumstix), you can use
them directly.  Check out <a href="http://www.irobot.com/hacker">iRobot's
hacker site</a> for more information, including the pinout on the Create's
serial port.  The <a href="http://create.pbwiki.com">Create Wiki</a>
has a howto on building an appropriate serial cable.

@par Compile-time dependencies

- none

@par Provides

The create driver provides the following device interfaces:

- @ref interface_position2d
  - This interface returns odometry data (PLAYER_POSITION2D_DATA_STATE), 
    and accepts velocity commands (PLAYER_POSITION2D_CMD_VEL).

- @ref interface_power
  - This interface returns battery levels (PLAYER_POWER_DATA_STATE).

- @ref interface_bumper
  - This interface returns bumper data (PLAYER_BUMPER_DATA_STATE).

- @ref interface_opaque
  - This driver supports programming song, playing songs, setting the LEDs,
    and running demo scripts. 
  - Play song data format in bytes: [0][song_number]
  - Program song data format in bytes: [1][song_number][length(n)][note_1][length_note_1]...[note_n][length_note_n]. 
  - Set LEDS format in bytes: [2][dirt_dectect(0/1)][max_bool(0/1)][clean(0/1)][spot(0/1)][status(0=off,1=red,2=green,3=amber)][power_color(0-255)][power_intensity(0-255)]
  - Run a demo script in bytes: [3][demo number]

@par Supported configuration requests

- PLAYER_POSITION2D_REQ_GET_GEOM
- PLAYER_BUMPER_REQ_GET_GEOM

@par Configuration file options

- port (string)
  - Default: "/dev/ttyS0"
  - Serial port used to communicate with the robot.
- safe (integer)
  - Default: 1
  - Nonzero to keep the robot in "safe" mode (the robot will stop if
    the wheeldrop or cliff sensors are triggered), zero for "full" mode

@par Example

@verbatim
driver
(
  name "create"
  provides ["position2d:0" "power:0" "bumper:0" "ir:0" "opaque:0"]
  port "/dev/ttyS2"
  safe 1
)
@endverbatim


@todo
- Add support for IRs, vacuum motors, etc.
- Recover from a cliff/wheeldrop sensor being triggered in safe mode;
the robot goes into passive mode when this happens, which right now
requires Player to be restarted

@author Brian Gerkey
*/
/** @} */


#include <unistd.h>
#include <stdlib.h>

#include <libplayercore/playercore.h>

#include "create_comms.h"

#define CYCLE_TIME_US 200000

class Create : public Driver
{
  public:
    Create(ConfigFile* cf, int section);
    ~Create();

    int Setup();
    int Shutdown();

    // MessageHandler
    int ProcessMessage(QueuePointer &resp_queue, 
		       player_msghdr * hdr, 
		       void * data);

  private:
    // Main function for device thread.
    virtual void Main();

    // Serial port where the create is
    const char* serial_port;

    // full control or not
    bool safe;

    player_devaddr_t position_addr;
    player_devaddr_t power_addr;
    player_devaddr_t bumper_addr;
    player_devaddr_t ir_addr;
    player_devaddr_t opaque_addr;
    player_devaddr_t gripper_addr;

    player_opaque_data_t *cpdata;

    // The underlying create object
    create_comm_t* create_dev;
};

// a factory creation function
Driver* Create_Init(ConfigFile* cf, int section)
{
  return((Driver*)(new Create(cf, section)));
}

// a driver registration function
void Create_Register(DriverTable* table)
{
  table->AddDriver("create", Create_Init);
}

Create::Create(ConfigFile* cf, int section)
        : Driver(cf,section,true,PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
  memset(&this->position_addr,0,sizeof(player_devaddr_t));
  memset(&this->power_addr,0,sizeof(player_devaddr_t));
  memset(&this->bumper_addr,0,sizeof(player_devaddr_t));
  memset(&this->ir_addr,0,sizeof(player_devaddr_t));
  memset(&this->opaque_addr,0,sizeof(player_devaddr_t));
  //memset(&this->gripper_addr,0,sizeof(player_devaddr_t));

  this->cpdata = (player_opaque_data_t*)malloc(sizeof(player_opaque_data_t));

  // Do we create a position interface?
  if(cf->ReadDeviceAddr(&(this->position_addr), section, "provides",
                        PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->position_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a power interface?
  if(cf->ReadDeviceAddr(&(this->power_addr), section, "provides",
                        PLAYER_POWER_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->power_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a bumper interface?
  if(cf->ReadDeviceAddr(&(this->bumper_addr), section, "provides",
                        PLAYER_BUMPER_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->bumper_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a IR interface?
  if(cf->ReadDeviceAddr(&(this->ir_addr), section, "provides",
                        PLAYER_IR_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->ir_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a Opaque interface?
  if(cf->ReadDeviceAddr(&(this->opaque_addr), section, "provides",
                        PLAYER_OPAQUE_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->opaque_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a gripper interface?
  if(cf->ReadDeviceAddr(&(this->gripper_addr), section, "provides",
                        PLAYER_GRIPPER_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->gripper_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }


  this->serial_port = cf->ReadString(section, "port", "/dev/ttyS0");
  this->safe = cf->ReadInt(section, "safe", 1);
  this->create_dev = NULL;
}

Create::~Create()
{
  free (this->cpdata);
}

int
Create::Setup()
{
  this->create_dev = create_create(this->serial_port);

  if(create_open(this->create_dev, !this->safe) < 0)
  {
    create_destroy(this->create_dev);
    this->create_dev = NULL;
    PLAYER_ERROR("failed to connect to create");
    return(-1);
  }

  this->StartThread();

  return(0);
}

int
Create::Shutdown()
{
  this->StopThread();

  if(create_close(this->create_dev))
  {
    PLAYER_ERROR("failed to close create connection");
  }
  create_destroy(this->create_dev);
  this->create_dev = NULL;
  return(0);
}

void
Create::Main()
{
  for(;;)
  {
     this->ProcessMessages();

     if(create_get_sensors(this->create_dev, -1) < 0)
     {
       PLAYER_ERROR("failed to get sensor data from create");
       create_close(this->create_dev);
       return;
     }

     ////////////////////////////
     // Update position2d data
     player_position2d_data_t posdata;
     memset(&posdata,0,sizeof(posdata));

     posdata.pos.px = this->create_dev->ox;
     posdata.pos.py = this->create_dev->oy;
     posdata.pos.pa = this->create_dev->oa;

     this->Publish(this->position_addr,
                   PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE,
                   (void*)&posdata, sizeof(posdata), NULL);

     ////////////////////////////
     // Update power data
     player_power_data_t powerdata;
     memset(&powerdata,0,sizeof(powerdata));

     powerdata.volts = this->create_dev->voltage;
     powerdata.watts = this->create_dev->voltage * this->create_dev->current;
     powerdata.joules = this->create_dev->charge;
     powerdata.percent = 100.0 * 
             (this->create_dev->charge / this->create_dev->capacity);
     powerdata.charging = 
             (this->create_dev->charging_state == CREATE_CHARGING_NOT) ? 0 : 1;
     powerdata.valid = (PLAYER_POWER_MASK_VOLTS |
                        PLAYER_POWER_MASK_WATTS | 
                        PLAYER_POWER_MASK_JOULES | 
                        PLAYER_POWER_MASK_PERCENT |
                        PLAYER_POWER_MASK_CHARGING);

     this->Publish(this->power_addr,
                   PLAYER_MSGTYPE_DATA, PLAYER_POWER_DATA_STATE,
                   (void*)&powerdata, sizeof(powerdata), NULL);

     ////////////////////////////
     // Update bumper data
     player_bumper_data_t bumperdata;
     memset(&bumperdata,0,sizeof(bumperdata));

     bumperdata.bumpers_count = 2;
     bumperdata.bumpers = new uint8_t[bumperdata.bumpers_count];
     bumperdata.bumpers[0] = this->create_dev->bumper_left;
     bumperdata.bumpers[1] = this->create_dev->bumper_right;

     this->Publish(this->bumper_addr,
                   PLAYER_MSGTYPE_DATA, PLAYER_BUMPER_DATA_STATE,
                   (void*)&bumperdata);
     delete [] bumperdata.bumpers;

     ////////////////////////////
     // Update IR data
     player_ir_data_t irdata;
     memset(&irdata,0,sizeof(irdata));

     irdata.ranges_count = 11;
     irdata.ranges = new float [irdata.ranges_count];
     irdata.ranges[0] = (float)this->create_dev->wall;
     irdata.ranges[1] = (float)this->create_dev->cliff_left;
     irdata.ranges[2] = (float)this->create_dev->cliff_frontleft;
     irdata.ranges[3] = (float)this->create_dev->cliff_frontright;
     irdata.ranges[4] = (float)this->create_dev->cliff_right;
     irdata.ranges[5] = (float)this->create_dev->virtual_wall;
     irdata.ranges[6] = (float)this->create_dev->dirtdetector_right;
     irdata.ranges[7] = (float)this->create_dev->dirtdetector_left;
     irdata.ranges[8] = (float)this->create_dev->wheeldrop_caster;
     irdata.ranges[9] = (float)this->create_dev->wheeldrop_left;
     irdata.ranges[10] = (float)this->create_dev->wheeldrop_right;

     this->Publish(this->ir_addr,
         PLAYER_MSGTYPE_DATA, PLAYER_IR_DATA_RANGES,
         (void*)&irdata);
     delete [] irdata.ranges;


     ////////////////////////////
     // Update Gripper data
     player_gripper_data_t gripperdata;
     memset(&gripperdata,0,sizeof(gripperdata));

     gripperdata.state=this->create_dev->overcurrent_vacuum;
     gripperdata.beams=this->create_dev->dirtdetector_right+this->create_dev->dirtdetector_left;
     gripperdata.stored=0;

     this->Publish(this->gripper_addr,
         PLAYER_MSGTYPE_DATA,
         PLAYER_GRIPPER_DATA_STATE,
         (void*) &gripperdata, sizeof(gripperdata), NULL);

     ////////////////////////////
     // Update Opaque-Control data
     memset(this->cpdata,0,sizeof(cpdata));

     this->cpdata->data_count=5;
     this->cpdata->data = new uint8_t [this->cpdata->data_count];

     this->cpdata->data[0] = this->create_dev->button_max;
     this->cpdata->data[1] = this->create_dev->button_clean;
     this->cpdata->data[2] = this->create_dev->button_spot;
     this->cpdata->data[3] = this->create_dev->button_power;
     this->cpdata->data[4] = this->create_dev->remote_opcode;

     this->Publish(this->opaque_addr,
         PLAYER_MSGTYPE_DATA,PLAYER_OPAQUE_DATA_STATE,
         (void*)this->cpdata, sizeof(*this->cpdata), NULL);
     delete [] this->cpdata->data;

     usleep(CYCLE_TIME_US);
  }
}

int
Create::ProcessMessage(QueuePointer &resp_queue, 
		       player_msghdr * hdr, 
		       void * data)
{
  if(Message::MatchMessage(hdr,
                           PLAYER_MSGTYPE_CMD,
                           PLAYER_POSITION2D_CMD_VEL,
                           this->position_addr))
  {
    // get and send the latest motor command
    player_position2d_cmd_vel_t position_cmd;
    position_cmd = *(player_position2d_cmd_vel_t*)data;
    PLAYER_MSG2(2,"sending motor commands %f:%f", 
                position_cmd.vel.px,
                position_cmd.vel.pa);
    if(create_set_speeds(this->create_dev, 
                         position_cmd.vel.px, 
                         position_cmd.vel.pa) < 0)
    {
      PLAYER_ERROR("failed to set speeds to create");
    }
    return(0);
  }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                 PLAYER_POSITION2D_REQ_MOTOR_POWER,
                                 this->position_addr))
  {
    this->Publish(this->position_addr, resp_queue,
        PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_MOTOR_POWER);
    return 0;
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_POSITION2D_REQ_GET_GEOM,
                                this->position_addr))
  {
    /* Return the robot geometry. */
    player_position2d_geom_t geom={{0}};
    // Assume that it turns about its geometric center, so leave as zeros

    geom.size.sl = CREATE_DIAMETER;
    geom.size.sw = CREATE_DIAMETER;

    this->Publish(this->position_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_POSITION2D_REQ_GET_GEOM,
                  (void*)&geom, sizeof(geom), NULL);
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                PLAYER_BUMPER_REQ_GET_GEOM,
                                this->bumper_addr))
  {
    player_bumper_geom_t geom;

    geom.bumper_def_count = 2;
    geom.bumper_def = new player_bumper_define_t[geom.bumper_def_count];

    geom.bumper_def[0].pose.px = 0.0;
    geom.bumper_def[0].pose.py = 0.0;
    geom.bumper_def[0].pose.pyaw = 0.0;
    geom.bumper_def[0].length = 0.0;
    geom.bumper_def[0].radius = CREATE_DIAMETER/2.0;

    geom.bumper_def[1].pose.px = 0.0;
    geom.bumper_def[1].pose.py = 0.0;
    geom.bumper_def[1].pose.pyaw = 0.0;
    geom.bumper_def[1].length = 0.0;
    geom.bumper_def[1].radius = CREATE_DIAMETER/2.0;

    this->Publish(this->bumper_addr, resp_queue,
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_BUMPER_REQ_GET_GEOM,
                  (void*)&geom);
    delete [] geom.bumper_def;

    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
                                    PLAYER_IR_REQ_POSE,
                                    this->ir_addr))
  {
    player_ir_pose poses;

    poses.poses_count = 11;
    poses.poses = new player_pose3d_t[poses.poses_count];

    // TODO: Fill in proper values
    for (int i=0; i<11; i++)
    {
      poses.poses[i].px = 0.0;
      poses.poses[i].py = 0.0;
      poses.poses[i].pyaw = 0.0;
    }

    this->Publish(this->ir_addr, resp_queue, 
                  PLAYER_MSGTYPE_RESP_ACK,
                  PLAYER_IR_REQ_POSE,
                  (void*)&poses);
    delete [] poses.poses;
    return(0);
  }
  else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD,
                                    PLAYER_OPAQUE_CMD,
                                    this->opaque_addr))
  {
    player_opaque_data_t opaque_data;

    opaque_data = *(player_opaque_data_t*)data;

    // Play Command
    if (opaque_data.data[0] == 0 )
    {
      uint8_t song_index;

      song_index = opaque_data.data[1];

      create_play_song(this->create_dev, song_index);
    }
    // Program song command
    else if (opaque_data.data[0] == 1)
    {
      uint8_t index = opaque_data.data[1];
      uint8_t length = opaque_data.data[2];
      uint8_t notes[length];
      uint8_t note_lengths[length];

      for (unsigned int i=0; i<length; i++)
      {
        notes[i] = opaque_data.data[3+i*2];
        note_lengths[i] = opaque_data.data[4+i*2];
      }

      create_set_song(this->create_dev, index, length, 
          notes, note_lengths);
    }
    // Set the LEDs
    else if (opaque_data.data[0] == 2)
    {
      uint8_t dirt_detect = opaque_data.data[1] == 0 ? 0 : 1;
      uint8_t max = opaque_data.data[2] == 0 ? 0 : 1;
      uint8_t clean = opaque_data.data[3] == 0 ? 0 : 1;
      uint8_t spot = opaque_data.data[4] == 0 ? 0 : 1;
      uint8_t status = opaque_data.data[5];
      uint8_t power_color = opaque_data.data[6];
      uint8_t power_intensity = opaque_data.data[7];

      if (status > 3)
        status = 3;

      if (create_set_leds(this->create_dev, dirt_detect, max, clean, spot, 
            status, power_color, power_intensity) < 0)
      {
        PLAYER_ERROR("failed to set create leds");
        return -1;
      }
    }
    // Run a demo script
    else if (opaque_data.data[0] == 3)
    {
      uint8_t demo_num = opaque_data.data[1];
      if (create_run_demo(this->create_dev, demo_num)<0)
      {
        PLAYER_ERROR("failed to run create demo");
        return -1;
      }
    }

    return 0;
  }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                                 PLAYER_GRIPPER_CMD_OPEN,
                                 this->gripper_addr))
  {
    create_vacuum(this->create_dev,7);
    return 0;
  }
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                                 PLAYER_GRIPPER_CMD_CLOSE,
                                 this->gripper_addr))
  {
    create_vacuum(this->create_dev,0);
    return 0;
  }
  else
  {
    return(-1);
  }
}
