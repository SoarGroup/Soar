/*  Player - One Hell of a Robot Server
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

/* notes:
 * the Main thread continues running when no-one is connected
 * this we retain our odometry data, whether anyone is connected or not
 */

/* Modified by Toby Collett, University of Auckland 2003-02-25
 * Added support for bump sensors for RWI b21r robot, uses DIO
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_rflex rflex
 * @brief RWI mobile robots

The rflex driver is used to control RWI robots by directly communicating
with RFLEX onboard the robot (i.e., Mobility is bypassed).  To date,
these drivers have been tested on an ATRV-Jr, but they should
work with other RFLEX-controlled robots: you will have to determine some
parameters to set in the config file, however.

As of March 2003 these drivers have been modified to support the
b21r robot, Currently additional support has been added for the @ref
interface_power interface and @ref interface_bumper
interface. For the pan tilt unit on the b21r please refer to
the @ref driver_ptu46 driver.

@par Compile-time dependencies

- none

@par Provides

The rflex driver provides the following device interfaces, some of them named:

- @ref interface_position2d : This interface returns odometry data,
  and accepts velocity commands.
- "sonar" @ref interface_sonar : Range data from the sonar array
- "sonar2" @ref interface_sonar : Range data from the second sonar array
- @ref interface_ir
- @ref interface_bumper
- @ref interface_power
- @ref interface_aio
- @ref interface_dio

@par Supported configuration requests

- The @ref interface_position2d interface supports:
  - PLAYER_POSITION2D_REQ_SET_ODOM
  - PLAYER_POSITION2D_REQ_MOTOR_POWER
  - PLAYER_POSITION2D_REQ_VELOCITY_MODE
  - PLAYER_POSITION2D_REQ_RESET_ODOM
  - PLAYER_POSITION2D_REQ_GET_GEOM
- The @ref interface_ir interface supports:
  - PLAYER_IR_REQ_POWER
  - PLAYER_IR_REQ_POSE
- The "sonar" @ref interface_sonar interface supports:
  - PLAYER_SONAR_REQ_POWER
  - PLAYER_SONAR_REQ_GET_GEOM
- The "sonar2" @ref interface_sonar interface supports:
  - PLAYER_SONAR_REQ_POWER
  - PLAYER_SONAR_REQ_GET_GEOM
- The @ref interface_bumper interface supports:
  - PLAYER_BUMPER_REQ_GET_GEOM

@par Configuration file options

- port (string)
  - Default: "/dev/ttyR0"
  - Serial port used to communicate with the robot.
- m_length (float)
  - Default: 0.5
  - Length of the robot in meters
- m_width (float)
  - Default: 0.5
  - Width of the robot in meters
- odo_distance_conversion (float)
  - Default: 0
  - Odometry conversion. See Note 1.
- odo_angle_conversion (float)
  - Default: 0
  - Odometry conversion. See Note 2.
- default_trans_acceleration (float)
  - Default: 0.1
  - Set translational acceleration, in m.
- default_rot_acceleration (float)
  - Default: 0.1
  - Set rotational acceleration, in radians.
- rflex_joystick (integer)
  - Default: 0
  - Enables joystick control via the rflex controller
- rflex_joy_pos_ratio (float)
  - Default: 0
  - Joystick to movement conversion ratio
- rflex_joy_ang_ratio (float)
  - Default: 0
  - Joystick to movement conversion ratio
- range_distance_conversion (float)
  - Default: 1476
  - Sonar range conversion factor. See Note 7.
- max_num_sonars (integer)
  - Default: 64
  - See Note 4
- num_sonars (integer)
  - Default: 24
  - See Note 4
- sonar_age (integer)
  - Default: 1
  - Prefiltering parameter. See Note 3.
- num_sonar_banks (integer)
  - Default: 8
  - See Note 4
- num_sonars_possible_per_bank (integer)
  - Default: 16
  - See Note 4
- num_sonars_in_bank (integer tuple)
  - Default: [ 8 8 8 ... ]
  - See Note 4
- sonar_echo_delay (integer)
  - Default: 3000
  - Sonar configuration parameters
- sonar_ping_delay (integer)
  - Default: 0
  - Sonar configuration parameters
- sonar_set_delay (integer)
  - Default: 0
  - Sonar configuration parameters
- mrad_sonar_poses (tuple float)
  - Default: [ 0 0 0 ... ]
  - Sonar positions and directions.  See Note 6.
- sonar_2nd_bank_start (integer)
  - Default: 0
  - Address of the second sonar bank (lower bank on the b21r)
- pose_count (integer)
  - Default: 8
  - Total Number of IR sensors
- rflex_base_bank (integer)
  - Default: 0
  - Base IR Bank
- rflex_bank_count (integer)
  - Default: 0
  - Number of banks in use
- ir_min_range (float)
  - Default: 0.100
  - Min range of ir sensors (m) (Any range below this is returned as 0)
- ir_max_range (float)
  - Default: 0.800
  - Max range of ir sensors (m) (Any range above this is returned as max)
- rflex_banks (float tuple)
  - Default: [ 0 0 0 ... ]
  - Number of IR sensors in each bank
- poses (float tuple)
  - Default: [ 0 0 0 ... ]
  - x,y,theta of IR sensors (m, m, deg)
- rflex_ir_calib (float tuple)
  - Default: [ 1 1 ... ]
  - IR Calibration data (see Note 8)
- bumper_count (integer)
  - Default: 0
  - Number of bumper panels
- bumper_def (float tuple)
  - Default: [ 0 0 0 0 0 ... ]
  - x,y,theta,length,radius (m,m,deg,m,m) for each bumper
- rflex_bumper_address (integer)
  - Default: 0x40
  - The base address of first bumper in the DIO address range
- rflex_bumper_style (string)
  - Default: "addr"
  - Bumpers can be defined either by addresses or a bit mask
- rflex_power_offset (float)
  - Default: 0
  - The calibration constant for the power calculation in volts

@par Notes
-# Since the units used by the Rflex for odometry appear to be completely
   arbitrary, this coefficient is needed to convert to meters: m =
   (rflex units) / (odo_distance_conversion).  These arbitrary units
   also seem to be different on each robot model. I'm afraid you'll
   have to determine your robot's conversion factor by driving a known
   distance and observing the output of the RFlex.
-# Conversion coefficient
   for rotation odometry: see odo_distance_conversion. Note that
   heading is re-calculated by the Player driver since the RFlex is not
   very accurate in this respect. See also Note 1.
-# Used for prefiltering:
   the standard Polaroid sensors never return values that are closer
   than the closest obstacle, thus we can buffer locally looking for the
   closest reading in the last "sonar_age" readings. Since the servo
   tick here is quite small, you can still get pretty recent data in
   the client.
-# These values are all used for remapping the sonars from Rflex indexing
   to player indexing. Individual sonars are enumerated 0-15 on each
   board, but at least on my robots each only has between 5 and 8 sonar
   actually attached.  Thus we need to remap all of these indexes to
   get a contiguous array of N sonars for Player.
     - max_num_sonars is the maximum enumeration value+1 of
       all sonar meaning if we have 4 sonar boards this number is 64.
     - num_sonars is the number of physical sonar sensors -
       meaning the number of ranges that will be returned by Player.  -
       num_sonar_banks is the number of sonar boards you have.
     - num_sonars_possible_per_bank is probobly 16 for all
       robots, but I included it here just in case. this is the number of
       sonar that can be attached to each sonar board, meaning the maximum
       enumeration value mapped to each board.  - num_sonars_in_bank
       is the nubmer of physical sonar attached to each board in order -
       you'll notice on each the sonar board a set of dip switches, these
       switches configure the enumeration of the boards (ours are 0-3)
-# The first RFlex device (position, sonar or power) in the config file
   must include this option, and only the first device's value will be used.
-# This is about the ugliest way possible of telling Player where each
   sonar is mounted.  Include in the string groups of three values: "x1
   y1 th1 x2 y2 th2 x3 y3 th3 ...".  x and y are m and theta is radians,
in Player's robot coordinate system.
-# Used to convert between arbitrary sonar units to millimeters: m =
   sonar units / range_distance_conversion.
-# Calibration is in the form Range = a*Voltage^b and stored in the
   tuple as [a1 b1 a2 b2 ...] etc for each ir sensor.

@par Example

@verbatim
driver
(
  name "rflex"
  provides ["position2d:1" "bumper:0" "sonar::sonar:0" "sonar2::sonar:1" "power:0" "ir:0"]

  rflex_serial_port     "/dev/ttyR0"
  m_length      0.5
  m_width       0.5
  odo_distance_conversion   103000
  odo_angle_conversion    35000
  default_trans_acceleration  0.5
  default_rot_acceleration    0.017
  rflex_joystick      1
  rflex_joy_pos_ratio   6
  rflex_joy_ang_ratio   -0.01


  bumper_count    14
  bumper_def    [   -216.506351 125.000000 -210.000000 261.799388 250.000000 -0.000000 250.000000 -270.000000 261.799388 250.000000 216.506351 125.000000 -330.000000 261.799388 250.000000 216.506351 -125.000000 -390.000000 261.799388 250.000000 0.000000 -250.000000 -450.000000 261.799388 250.000000 -216.506351 -125.000000 -510.000000 261.799388 250.000000 -240.208678 -99.497692 -157.500000 204.203522 260.000000 -240.208678 99.497692 -202.500000 204.203522 260.000000 -99.497692 240.208678 -247.500000 204.203522 260.000000 99.497692 240.208678 -292.500000 204.203522 260.000000 240.208678 99.497692 -337.500000 204.203522 260.000000 240.208678 -99.497692 -382.500000 204.203522 260.000000 99.497692 -240.208678 -427.500000 204.203522 260.000000 -99.497692 -240.208678 -472.500000 204.203522 260.000000 ]
  rflex_bumper_address  64 # 0x40

  range_distance_conversion   1.476
  sonar_age       1
  sonar_echo_delay    30000
  sonar_ping_delay    0
  sonar_set_delay     0
  max_num_sonars      224
  num_sonars        48
  num_sonar_banks     14
  num_sonars_possible_per_bank  16
  num_sonars_in_bank    [4 4 4 4 4 4 3 3 3 3 3 3 3 3]
  # theta (rads), x, y (m) in robot coordinates (x is forward)
  mrad_sonar_poses  [     3.01069  -0.24786122    0.03263155     2.74889  -0.23096988    0.09567086     2.48709  -0.19833834   0.15219036     2.22529  -0.15219036   0.19833834     1.96350   -0.09567086   0.23096988     1.70170   -0.03263155   0.24786122     1.43990    0.03263155   0.24786122     1.17810    0.09567086   0.23096988     0.91630   0.15219036   0.19833834     0.65450   0.19833834   0.15219036     0.39270   0.23096988    0.09567086     0.13090   0.24786122    0.03263155    -0.13090   0.24786122   -0.03263155    -0.39270   0.23096988   -0.09567086    -0.65450   0.19833834  -0.15219036    -0.91630   0.15219036  -0.19833834    -1.17810    0.09567086  -0.23096988    -1.43990    0.03263155  -0.24786122    -1.70170   -0.03263155  -0.24786122    -1.96350   -0.09567086  -0.23096988    -2.22529  -0.15219036  -0.19833834    -2.48709  -0.19833834  -0.15219036    -2.74889  -0.23096988   -0.09567086    -3.01069  -0.24786122   -0.03263155       4.18879  -0.13000000  -0.22516660     3.92699  -0.18384776  -0.18384776     3.66519  -0.22516660  -0.13000000     3.40339  -0.25114071   -0.06729295     3.14159  -0.26000000     0.00000     2.87979  -0.25114071    0.06729295     2.61799  -0.22516660   0.13000000     2.35619  -0.18384776   0.18384776     2.09440  -0.13000000   0.22516660     1.83260   -0.06729295   0.25114071     1.57080     0.00000   0.26000000     1.30900    0.06729295   0.25114071     1.04720   0.13000000   0.22516660     0.78540   0.18384776   0.18384776     0.52360   0.22516660   0.13000000     0.26180   0.25114071    0.06729295     0.00000   0.26000000     0.00000    -0.26180   0.25114071   -0.06729295    -0.52360   0.22516660  -0.13000000    -0.78540   0.18384776  -0.18384776    -1.04720   0.13000000  -0.22516660    -1.30900    0.06729295  -0.25114071    -1.57080     0.00000  -0.26000000    -1.83260   -0.06729295  -0.25114071    -2.09440  -0.13000000  -0.22516660    -2.35619  -0.18384776  -0.18384776]
  sonar_2nd_bank_start  24

  rflex_power_offset    1.2 # volts

  rflex_base_bank 0
  rflex_bank_count 6
  rflex_banks [4 4 4 4 4 4]
  pose_count  24
  ir_min_range  0.100
  ir_max_range  0.800
  rflex_ir_calib  [ 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 4000 -2 ]
  poses [-0.247861 0.032632 -3.272492 -0.230970 0.095671 -3.534292 -0.198338 0.152190 -3.796091 -0.152190 0.198338 -4.057890 -0.095671 0.230970 -4.319690 -0.032632 0.247861 -4.581489 0.032632 0.247861 -4.843289 0.095671 0.230970 -5.105088 0.152190 0.198338 -5.366888 0.198338 0.152190 -5.628687 0.230970 0.095671 -5.890486 0.247861 0.032632 -6.152286 0.247861 -0.032632 -6.414085 0.230970 -0.095671 -6.675884 0.198338 -0.152190 -6.937684 0.152190 -0.198338 -7.199483 0.095671 -0.230970 -7.461283 0.032632 -0.247861 -7.723082 -0.032632 -0.247861 -7.984881 -0.095671 -0.230970 -8.246680 -0.152190 -0.198338 -8.508480 -0.198338 -0.152190 -8.770280 -0.230970 -0.095671 -9.032079 -0.247861 -0.032631 -9.293879 ]
)
@endverbatim

@author Matthew Brewer, Toby Collett
*/
/** @} */



#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>  /* for abs() */
#include <netinet/in.h>
#include <termios.h>
#include <assert.h>

#include "rflex.h"
#include "rflex_configs.h"

//rflex communications stuff
#include "rflex_commands.h"
#include "rflex-io.h"

#include <libplayercore/playercore.h>
#include <libplayerxdr/playerxdr.h>
extern PlayerTime* GlobalTime;

extern int               RFLEX::joy_control;

// help function to rotate sonar positions
void SonarRotate(double heading, double x1, double y1, double t1, double *x2, double *y2, double *t2)
{
  *t2 = t1 - heading;
  *x2 = x1*cos(heading) + y1*sin(heading);
  *y2 = -x1*sin(heading) + y1*cos(heading);
}


//NOTE - this is accessed as an extern variable by the other RFLEX objects
rflex_config_t rflex_configs;

/* initialize the driver.
 *
 * returns: pointer to new REBIR object
 */
Driver*
RFLEX_Init(ConfigFile *cf, int section)
{
  return (Driver *) new RFLEX( cf, section);
}

/* register the rflex driver in the drivertable
 *
 * returns:
 */
void
RFLEX_Register(DriverTable *table)
{
  table->AddDriver("rflex", RFLEX_Init);
}

///////////////////////////////
// Message handler functions
///////////////////////////////
void PrintHeader(player_msghdr_t hdr);
int RFLEX::ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr,
                           void * data)
{
  assert(hdr);

  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_SONAR_REQ_POWER,
                        sonar_id))
  {
    Lock();
    if(reinterpret_cast<player_sonar_power_config_t *> (data)->state==0)
      rflex_sonars_off(rflex_fd);
    else
      rflex_sonars_on(rflex_fd);
    Unlock();
    Publish(sonar_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_SONAR_REQ_POWER,
                        sonar_id_2))
  {
    Lock();
    if(reinterpret_cast<player_sonar_power_config_t *> (data)->state==0)
      rflex_sonars_off(rflex_fd);
    else
      rflex_sonars_on(rflex_fd);
    Unlock();
    Publish(sonar_id_2, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_SONAR_REQ_GET_GEOM,
                        sonar_id))
  {
    player_sonar_geom_t geom;
    Lock();
    geom.poses_count = rflex_configs.sonar_1st_bank_end;
    geom.poses = new player_pose3d_t[geom.poses_count];
    for (int i = 0; i < rflex_configs.sonar_1st_bank_end; i++)
    {
      geom.poses[i] = rflex_configs.mrad_sonar_poses[i];
    }
    Unlock();
    Publish(sonar_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype, &geom,sizeof(geom));
    delete [] geom.poses;
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_SONAR_REQ_GET_GEOM,
                        sonar_id_2))
  {
    player_sonar_geom_t geom;
    Lock();
    geom.poses_count = (rflex_configs.num_sonars - rflex_configs.sonar_2nd_bank_start);
    geom.poses = new player_pose3d_t[geom.poses_count];
    for (int i = 0; i < rflex_configs.num_sonars - rflex_configs.sonar_2nd_bank_start; i++)
    {
      geom.poses[i] = rflex_configs.mrad_sonar_poses[i+rflex_configs.sonar_2nd_bank_start];
    }
    Unlock();
    Publish(sonar_id_2, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype, &geom,sizeof(geom));
    delete [] geom.poses;
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_BUMPER_REQ_GET_GEOM,
                        bumper_id))
  {
    player_bumper_geom_t geom;
    Lock();
    geom.bumper_def_count = rflex_configs.bumper_count;
    geom.bumper_def = new player_bumper_define_t[geom.bumper_def_count];
    for (int i = 0; i < rflex_configs.bumper_count; i++)
    {
      geom.bumper_def[i] = rflex_configs.bumper_def[i];
    }
    Unlock();
    Publish(bumper_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype, &geom,sizeof(geom));
    delete [] geom.bumper_def;
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_IR_REQ_POSE, ir_id))
  {
    Lock();
    Publish(ir_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype, &rflex_configs.ir_poses,sizeof(rflex_configs.ir_poses));
    Unlock();
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_IR_REQ_POWER, ir_id))
  {
    player_ir_power_req_t * req = reinterpret_cast<player_ir_power_req_t*> (data);
    Lock();
    if (req->state == 0)
      rflex_ir_off(rflex_fd);
    else
      rflex_ir_on(rflex_fd);
    Unlock();
    Publish(ir_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_SET_ODOM, position_id))
  {
    player_position2d_set_odom_req * set_odom_req = reinterpret_cast<player_position2d_set_odom_req*> (data);
    Lock();
    set_odometry(set_odom_req->pose.px,set_odom_req->pose.py,set_odom_req->pose.pa);
    Unlock();
    Publish(position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_MOTOR_POWER, position_id))
  {
    Lock();
    if(((player_position2d_power_config_t*)data)->state==0)
      rflex_brake_on(rflex_fd);
    else
      rflex_brake_off(rflex_fd);
    Unlock();
    Publish(position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_VELOCITY_MODE, position_id))
  {
    // Does nothing, needs to be implemented
    Publish(position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_RESET_ODOM, position_id))
  {
    Lock();
    reset_odometry();
    Unlock();
    Publish(position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype);
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_GET_GEOM, position_id))
  {
    player_position2d_geom_t geom={{0}};
    Lock();
    geom.size.sl = rflex_configs.m_length;
    geom.size.sw = rflex_configs.m_width;
    Unlock();
    Publish(position_id, resp_queue, PLAYER_MSGTYPE_RESP_ACK, hdr->subtype, &geom,sizeof(geom));
    return 0;
  }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, position_id))
  {
    Lock();
    command = *reinterpret_cast<player_position2d_cmd_vel_t *> (data);
    Unlock();
 
    // reset command_type since we have a new 
    // velocity command so we can get into the 
    // velocity control section in RFLEX::Main()
    
    command_type = 0;
    
    return 0;
  }

  return -1;
}

RFLEX::RFLEX(ConfigFile* cf, int section)
        : Driver(cf,section)
{
  // zero ids, so that we'll know later which interfaces were requested
  memset(&this->position_id, 0, sizeof(player_devaddr_t));
  memset(&this->sonar_id, 0, sizeof(player_devaddr_t));
  memset(&this->sonar_id_2, 0, sizeof(player_devaddr_t));
  memset(&this->ir_id, 0, sizeof(player_devaddr_t));
  memset(&this->bumper_id, 0, sizeof(player_devaddr_t));
  memset(&this->power_id, 0, sizeof(player_devaddr_t));
  memset(&this->aio_id, 0, sizeof(player_devaddr_t));
  memset(&this->dio_id, 0, sizeof(player_devaddr_t));
  
  command_type = 0;

  this->position_subscriptions = 0;
  this->sonar_subscriptions = 0;
  this->ir_subscriptions = 0;
  this->bumper_subscriptions = 0;

  // Do we create a robot position interface?
  if(cf->ReadDeviceAddr(&(this->position_id), section, "provides",
                      PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->position_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  else
  {
      PLAYER_WARN("Position2d interface not created for rflex driver");
  }

  // Do we create a sonar interface?
  if(cf->ReadDeviceAddr(&(this->sonar_id), section, "provides",
                      PLAYER_SONAR_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->sonar_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  else
  {
      PLAYER_WARN("sonar bank 1 interface not created for rflex driver");
  }
  // Do we create a second sonar interface?
  if(cf->ReadDeviceAddr(&(this->sonar_id_2), section, "provides",
                      PLAYER_SONAR_CODE, -1, "bank2") == 0)
  {
    if(this->AddInterface(this->sonar_id_2) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  else
  {
      PLAYER_WARN("sonar bank 2 interface not created for rflex driver");
  }

  // Do we create an ir interface?
  if(cf->ReadDeviceAddr(&(this->ir_id), section, "provides",
                      PLAYER_IR_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->ir_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  else
  {
      PLAYER_WARN("ir interface not created for rflex driver");
  }

  // Do we create a bumper interface?
  if(cf->ReadDeviceAddr(&(this->bumper_id), section, "provides",
                      PLAYER_BUMPER_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->bumper_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  else
  {
      PLAYER_WARN("bumper interface not created for rflex driver");
  }

  // Do we create a power interface?
  if(cf->ReadDeviceAddr(&(this->power_id), section, "provides",
                      PLAYER_POWER_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->power_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  else
  {
      PLAYER_WARN("power interface not created for rflex driver");
  }

  // Do we create an aio interface?
  if(cf->ReadDeviceAddr(&(this->aio_id), section, "provides",
                      PLAYER_AIO_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->aio_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  else
  {
      PLAYER_WARN("aio interface not created for rflex driver");
  }

  // Do we create a dio interface?
  if(cf->ReadDeviceAddr(&(this->dio_id), section, "provides",
                      PLAYER_DIO_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(this->dio_id) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
  else
  {
      PLAYER_WARN("dio interface not created for rflex driver");
  }

  //just sets stuff to zero
  set_config_defaults();

  // joystick override
  joy_control = 0;

  //get serial port: everyone needs it (and we dont' want them fighting)
  strncpy(rflex_configs.serial_port,
          cf->ReadString(section, "rflex_serial_port",
                         rflex_configs.serial_port),
          sizeof(rflex_configs.serial_port));

  ////////////////////////////////////////////////////////////////////////
  // Position-related options

  //length
  rflex_configs.m_length=
    cf->ReadFloat(section, "m_length",0.5);
  //width
  rflex_configs.m_width=
    cf->ReadFloat(section, "m_width",0.5);
  //distance conversion
  rflex_configs.odo_distance_conversion=
    cf->ReadFloat(section, "odo_distance_conversion", 0.0);
  //angle conversion
  rflex_configs.odo_angle_conversion=
    cf->ReadFloat(section, "odo_angle_conversion", 0.0);
  //default trans accel
  rflex_configs.mPsec2_trans_acceleration=
    cf->ReadFloat(section, "default_trans_acceleration",0.1);
  //default rot accel
  rflex_configs.radPsec2_rot_acceleration=
    cf->ReadFloat(section, "default_rot_acceleration",0.1);

  // absolute heading options
  rflex_configs.heading_home_address =
    cf->ReadInt(section, "rflex_heading_home_address",0);
  rflex_configs.home_on_start =
    cf->ReadInt(section, "rflex_home_on_start",0);

  // use rflex joystick for position
  rflex_configs.use_joystick |= cf->ReadInt(section, "rflex_joystick",0);
  rflex_configs.joy_pos_ratio = cf->ReadFloat(section, "rflex_joy_pos_ratio",0);
  rflex_configs.joy_ang_ratio = cf->ReadFloat(section, "rflex_joy_ang_ratio",0);

  ////////////////////////////////////////////////////////////////////////
  // Sonar-related options
  int x;

  rflex_configs.range_distance_conversion=
          cf->ReadFloat(section, "range_distance_conversion",1476);
  rflex_configs.max_num_sonars=
          cf->ReadInt(section, "max_num_sonars",64);
  rflex_configs.num_sonars=
          cf->ReadInt(section, "num_sonars",24);
  rflex_configs.sonar_age=
          cf->ReadInt(section, "sonar_age",1);
  rflex_configs.sonar_max_range=
          cf->ReadInt(section, "sonar_max_range",3000);
  rflex_configs.num_sonar_banks=
          cf->ReadInt(section, "num_sonar_banks",8);
  rflex_configs.num_sonars_possible_per_bank=
          cf->ReadInt(section, "num_sonars_possible_per_bank",16);
  rflex_configs.num_sonars_in_bank=(int *) malloc(rflex_configs.num_sonar_banks*sizeof(double));
  for(x=0;x<rflex_configs.num_sonar_banks;x++)
    rflex_configs.num_sonars_in_bank[x]=
            (int) cf->ReadTupleFloat(section, "num_sonars_in_bank",x,8);
  rflex_configs.sonar_echo_delay=
          cf->ReadInt(section, "sonar_echo_delay",3000);
  rflex_configs.sonar_ping_delay=
          cf->ReadInt(section, "sonar_ping_delay",0);
  rflex_configs.sonar_set_delay=
          cf->ReadInt(section, "sonar_set_delay", 0);
  rflex_configs.mrad_sonar_poses=(player_pose3d_t *) malloc(rflex_configs.num_sonars*sizeof(player_pose3d_t));
  for(x=0;x<rflex_configs.num_sonars;x++)
  {
    rflex_configs.mrad_sonar_poses[x].px=
            cf->ReadTupleFloat(section, "mrad_sonar_poses",3*x+1,0.0);
    rflex_configs.mrad_sonar_poses[x].py=
            cf->ReadTupleFloat(section, "mrad_sonar_poses",3*x+2,0.0);
    rflex_configs.mrad_sonar_poses[x].pyaw=
            cf->ReadTupleFloat(section, "mrad_sonar_poses",3*x,0.0);
  }
  rflex_configs.sonar_2nd_bank_start=cf->ReadInt(section, "sonar_2nd_bank_start", 0);
  rflex_configs.sonar_1st_bank_end=rflex_configs.sonar_2nd_bank_start>0?rflex_configs.sonar_2nd_bank_start:rflex_configs.num_sonars;
  ////////////////////////////////////////////////////////////////////////
  // IR-related options

  int pose_count=cf->ReadInt(section, "pose_count",8);
  rflex_configs.ir_base_bank=cf->ReadInt(section, "rflex_base_bank",0);
  rflex_configs.ir_bank_count=cf->ReadInt(section, "rflex_bank_count",0);
  rflex_configs.ir_min_range=cf->ReadFloat(section,"ir_min_range",0.1);
  rflex_configs.ir_max_range=cf->ReadFloat(section,"ir_max_range",0.8);
  rflex_configs.ir_count=new int[rflex_configs.ir_bank_count];
  rflex_configs.ir_a=new double[pose_count];
  rflex_configs.ir_b=new double[pose_count];
  rflex_configs.ir_poses.poses_count=pose_count;
  rflex_configs.ir_poses.poses=(player_pose3d_t *)
  malloc(rflex_configs.ir_poses.poses_count*sizeof(player_pose3d_t));
  unsigned int RunningTotal = 0;
  for(int i=0; i < rflex_configs.ir_bank_count; ++i)
    RunningTotal += (rflex_configs.ir_count[i]=(int) cf->ReadTupleFloat(section, "rflex_banks",i,0));
  rflex_configs.ir_total_count = RunningTotal;
  // posecount is actually unnecasary, but for consistancy will juse use it for error checking :)
  if (RunningTotal != rflex_configs.ir_poses.poses_count)
  {
    PLAYER_WARN("Error in config file, pose_count not equal to total poses in bank description\n");
    rflex_configs.ir_poses.poses_count = RunningTotal;
  }

  //  rflex_configs.ir_poses.poses=new int16_t[rflex_configs.ir_poses.pose_count];
  for(unsigned int i=0;i<rflex_configs.ir_poses.poses_count;i++)
  {
    rflex_configs.ir_poses.poses[i].px= cf->ReadTupleFloat(section, "poses",i*3,0);
    rflex_configs.ir_poses.poses[i].py= cf->ReadTupleFloat(section, "poses",i*3+1,0);
    rflex_configs.ir_poses.poses[i].pyaw= cf->ReadTupleFloat(section, "poses",i*3+2,0);

    // Calibration parameters for ir in form range=(a*voltage)^b
    rflex_configs.ir_a[i] = cf->ReadTupleFloat(section, "rflex_ir_calib",i*2,1);
    rflex_configs.ir_b[i] = cf->ReadTupleFloat(section, "rflex_ir_calib",i*2+1,1);
  }

  ////////////////////////////////////////////////////////////////////////
  // Bumper-related options
  rflex_configs.bumper_count = cf->ReadInt(section, "bumper_count",0);
  rflex_configs.bumper_def = new player_bumper_define_t[rflex_configs.bumper_count];
  for(x=0;x<rflex_configs.bumper_count;++x)
  {
    rflex_configs.bumper_def[x].pose.px =  (cf->ReadTupleFloat(section, "bumper_def",5*x,0)); //m
    rflex_configs.bumper_def[x].pose.py =  (cf->ReadTupleFloat(section, "bumper_def",5*x+1,0)); //m
    rflex_configs.bumper_def[x].pose.pyaw =  (cf->ReadTupleFloat(section, "bumper_def",5*x+2,0)); //rad
    rflex_configs.bumper_def[x].length =  (cf->ReadTupleFloat(section, "bumper_def",5*x+3,0)); //m
    rflex_configs.bumper_def[x].radius =  (cf->ReadTupleFloat(section, "bumper_def",5*x+4,0));  //m
  }
  rflex_configs.bumper_address = cf->ReadInt(section, "rflex_bumper_address",DEFAULT_RFLEX_BUMPER_ADDRESS);


  const char *bumperStyleStr = cf->ReadString(section, "rflex_bumper_style",DEFAULT_RFLEX_BUMPER_STYLE);

  if(strcmp(bumperStyleStr,RFLEX_BUMPER_STYLE_BIT) == 0)
  {
    rflex_configs.bumper_style = BUMPER_BIT;
  }
  else if(strcmp(bumperStyleStr,RFLEX_BUMPER_STYLE_ADDR) == 0)
  {
    rflex_configs.bumper_style = BUMPER_ADDR;
  }
  else
  {
    //Invalid value
    rflex_configs.bumper_style = BUMPER_ADDR;
  }

  ////////////////////////////////////////////////////////////////////////
  // Power-related options
  rflex_configs.power_offset = cf->ReadFloat(section, "rflex_power_offset",DEFAULT_RFLEX_POWER_OFFSET);

  rflex_fd = -1;

}

RFLEX::~RFLEX()
{
}

int RFLEX::Setup(){
  /* now spawn reading thread */
  StartThread();
  return(0);
}

int RFLEX::Shutdown()
{
  if(rflex_fd == -1)
  {
    return 0;
  }
  StopThread();
  //make sure it doesn't go anywhere
  rflex_stop_robot(rflex_fd,(int) M2ARB_ODO_CONV(rflex_configs.mPsec2_trans_acceleration));
  //kill that infernal clicking
  rflex_sonars_off(rflex_fd);
  // release the port
  rflex_close_connection(&rflex_fd);
  return 0;
}

/* start a thread that will invoke Main() */
void
RFLEX::StartThread(void)
{
  ThreadAlive = true;
  pthread_create(&driverthread, NULL, &DummyMain, this);
}

/* wait for termination of the thread */
// this is called with the lock held
void
RFLEX::StopThread(void)
{
  void* dummy;
  ThreadAlive = false;
  Unlock();

  //pthread_cancel(driverthread);
  if(pthread_join(driverthread,&dummy))
    perror("Driver::StopThread:pthread_join()");
  Lock();
}

int
RFLEX::Subscribe(player_devaddr_t id)
{
  int setupResult;

  // do the subscription
  if((setupResult = Driver::Subscribe(id)) == 0)
  {
    // also increment the appropriate subscription counter
    switch(id.interf)
    {
      case PLAYER_POSITION2D_CODE:
        this->position_subscriptions++;
        break;
      case PLAYER_SONAR_CODE:
        this->sonar_subscriptions++;
        break;
      case PLAYER_BUMPER_CODE:
        this->bumper_subscriptions++;
        break;
      case PLAYER_IR_CODE:
        this->ir_subscriptions++;
        break;
    }
  }

  return(setupResult);
}

int
RFLEX::Unsubscribe(player_devaddr_t id)
{
  int shutdownResult;

  // do the unsubscription
  if((shutdownResult = Driver::Unsubscribe(id)) == 0)
  {
    // also decrement the appropriate subscription counter
    switch(id.interf)
    {
      case PLAYER_POSITION2D_CODE:
        --this->position_subscriptions;
        assert(this->position_subscriptions >= 0);
        break;
      case PLAYER_SONAR_CODE:
        sonar_subscriptions--;
        assert(sonar_subscriptions >= 0);
        break;
      case PLAYER_BUMPER_CODE:
        --this->bumper_subscriptions;
        assert(this->bumper_subscriptions >= 0);
        break;
      case PLAYER_IR_CODE:
        --this->ir_subscriptions;
        assert(this->ir_subscriptions >= 0);
        break;
    }
  }

  return(shutdownResult);
}

void
RFLEX::Main()
{
  PLAYER_MSG1(1,"%s","Rflex Thread Started");

  //sets up connection, and sets defaults
  //configures sonar, motor acceleration, etc.
  if(initialize_robot()<0){
    PLAYER_ERROR("ERROR, no connection to RFLEX established\n");
    return;
  }
  Lock();
  reset_odometry();
  Unlock();



  static double mPsec_speedDemand=0.0, radPsec_turnRateDemand=0.0;
  bool newmotorspeed, newmotorturn;

  int i;
  int last_sonar_subscrcount = 0;
  int last_position_subscrcount = 0;
  int last_ir_subscrcount = 0;

  while(1)
  {
    int oldstate;
    int ret;
    ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&oldstate);

    // we want to turn on the sonars if someone just subscribed, and turn
    // them off if the last subscriber just unsubscribed.
    if(!last_sonar_subscrcount && this->sonar_subscriptions)
    {
      Lock();
        rflex_sonars_on(rflex_fd);
        Unlock();
    }
    else if(last_sonar_subscrcount && !(this->sonar_subscriptions))
    {
      Lock();
    rflex_sonars_off(rflex_fd);
    Unlock();
    }
    last_sonar_subscrcount = this->sonar_subscriptions;

    // we want to turn on the ir if someone just subscribed, and turn
    // it off if the last subscriber just unsubscribed.
    if(!last_ir_subscrcount && this->ir_subscriptions)
    {
      Lock();
    rflex_ir_on(rflex_fd);
    Unlock();
    }
    else if(last_ir_subscrcount && !(this->ir_subscriptions))
    {
      Lock();
    rflex_ir_off(rflex_fd);
    Unlock();
    }
    last_ir_subscrcount = this->ir_subscriptions;

    // we want to reset the odometry and enable the motors if the first
    // client just subscribed to the position device, and we want to stop
    // and disable the motors if the last client unsubscribed.

    //first user logged in
    if(!last_position_subscrcount && this->position_subscriptions)
    {
      Lock();
      //set drive defaults
      rflex_motion_set_defaults(rflex_fd);

      //make sure robot doesn't go anywhere
      rflex_stop_robot(rflex_fd,(int) (M2ARB_ODO_CONV(rflex_configs.mPsec2_trans_acceleration)));

    Unlock();
    }
    //last user logged out
    else if(last_position_subscrcount && !(this->position_subscriptions))
    {
      Lock();
      //make sure robot doesn't go anywhere
      rflex_stop_robot(rflex_fd,(int) (M2ARB_ODO_CONV(rflex_configs.mPsec2_trans_acceleration)));
      // disable motor power
      rflex_brake_on(rflex_fd);
      Unlock();
    }
    last_position_subscrcount = this->position_subscriptions;

  ProcessMessages();

    if(this->position_subscriptions || rflex_configs.use_joystick || rflex_configs.home_on_start)
    {
    Lock();
    newmotorspeed = false;
    newmotorturn = false;

    if (rflex_configs.home_on_start)
    {
      command.vel.pa = M_PI/18;
      newmotorturn=true;
    }

    if(mPsec_speedDemand != command.vel.px)
    {
          newmotorspeed = true;
          mPsec_speedDemand = command.vel.px;
        }
        if(radPsec_turnRateDemand != command.vel.pa)
        {
          newmotorturn = true;
          radPsec_turnRateDemand = command.vel.pa;
        }
        /* NEXT, write commands */
        // rflex has a built in failsafe mode where if no move command is recieved in a
        // certain interval the robot stops.
        // this is a good thing given teh size of the robot...
        // if network goes down or some such and the user looses control then the robot stops
        // if the robot is running in an autonomous mdoe it is easy enough to simply
        // resend the command repeatedly

        // allow rflex joystick to overide the player command
        if (joy_control > 0)
          --joy_control;
        // only set new command if type is valid and their is a new command
        else if (command_type == 0)
        {
          rflex_set_velocity(rflex_fd,(long) M2ARB_ODO_CONV(mPsec_speedDemand),(long) RAD2ARB_ODO_CONV(radPsec_turnRateDemand),(long) M2ARB_ODO_CONV(rflex_configs.mPsec2_trans_acceleration));
          command_type = 255;
        }
        Unlock();
    }
    else
    {
      Lock();
    rflex_stop_robot(rflex_fd,(long) M2ARB_ODO_CONV(rflex_configs.mPsec2_trans_acceleration));
    Unlock();
    }

    /* Get data from robot */
  static float LastYaw = 0;
    player_rflex_data_t rflex_data = {{{0}}};

  Lock();
    update_everything(&rflex_data);
    Unlock();

  if (position_id.interf != 0)
  {
    Publish(this->position_id,PLAYER_MSGTYPE_DATA,PLAYER_POSITION2D_DATA_STATE,
            (unsigned char*)&rflex_data.position,
            sizeof(player_position2d_data_t),
            NULL);
  }
  if (sonar_id.interf != 0)
  {
      Publish(this->sonar_id,PLAYER_MSGTYPE_DATA,PLAYER_SONAR_DATA_RANGES,
            (unsigned char*)&rflex_data.sonar,
            sizeof(player_sonar_data_t),
            NULL);
  }
  // Here we check if the robot has changed Yaw...
  // If it has we need to update the geometry as well
  if (rflex_data.position.pos.pa != LastYaw)
  {
    // Transmit new sonar geometry
    double NewGeom[3];

        player_sonar_geom_t geom;

    Lock();
        geom.poses_count = rflex_configs.num_sonars - rflex_configs.sonar_2nd_bank_start;
        geom.poses = new player_pose3d_t[geom.poses_count];
        for (i = 0; i < rflex_configs.num_sonars - rflex_configs.sonar_2nd_bank_start; i++)
        {
      SonarRotate(rad_odo_theta, rflex_configs.mrad_sonar_poses[i+rflex_configs.sonar_2nd_bank_start].px,rflex_configs.mrad_sonar_poses[i+rflex_configs.sonar_2nd_bank_start].py,rflex_configs.mrad_sonar_poses[i+rflex_configs.sonar_2nd_bank_start].pyaw,NewGeom,&NewGeom[1],&NewGeom[2]);
          geom.poses[i].px = NewGeom[0];
          geom.poses[i].py = NewGeom[1];
          geom.poses[i].pyaw = NewGeom[2];
        }
        Unlock();
        if (sonar_id_2.interf)
        Publish(this->sonar_id_2,  PLAYER_MSGTYPE_DATA, PLAYER_SONAR_DATA_GEOM,
        (unsigned char*)&geom, sizeof(player_sonar_geom_t), NULL);
        delete [] geom.poses;
  }
  LastYaw = rflex_data.position.pos.pa;

  if (sonar_id_2.interf)
      Publish(this->sonar_id_2,PLAYER_MSGTYPE_DATA,PLAYER_SONAR_DATA_RANGES,
            (unsigned char*)&rflex_data.sonar2,
            sizeof(player_sonar_data_t),
            NULL);
  if (ir_id.interf)
      Publish(this->ir_id,PLAYER_MSGTYPE_DATA,PLAYER_IR_DATA_RANGES,
            (unsigned char*)&rflex_data.ir,
            sizeof(player_ir_data_t),
            NULL);
  if (bumper_id.interf)
      Publish(this->bumper_id,PLAYER_MSGTYPE_DATA,PLAYER_BUMPER_DATA_STATE,
            (unsigned char*)&rflex_data.bumper,
            sizeof(player_bumper_data_t),
            NULL);
  if (power_id.interf)
    Publish(this->power_id,PLAYER_MSGTYPE_DATA,PLAYER_POWER_DATA_STATE,
            (unsigned char*)&rflex_data.power,
            sizeof(player_power_data_t),
            NULL);
  if (aio_id.interf != 0)
  {
      Publish(this->aio_id,PLAYER_MSGTYPE_DATA,PLAYER_AIO_DATA_STATE,
            (unsigned char*)&rflex_data.aio,
            sizeof(player_aio_data_t),
            NULL);
  }
  if (dio_id.interf != 0)
  {
      Publish(this->dio_id,PLAYER_MSGTYPE_DATA,PLAYER_DIO_DATA_VALUES,
            (unsigned char*)&rflex_data.dio,
            sizeof(player_dio_data_t),
            NULL);
  }

  ret=pthread_setcancelstate(oldstate,NULL);
  player_position2d_data_t_cleanup(&rflex_data.position);
  player_sonar_data_t_cleanup(&rflex_data.sonar);
  player_sonar_data_t_cleanup(&rflex_data.sonar2);
  player_gripper_data_t_cleanup(&rflex_data.gripper);
  player_power_data_t_cleanup(&rflex_data.power);
  player_bumper_data_t_cleanup(&rflex_data.bumper);
  player_dio_data_t_cleanup(&rflex_data.dio);
  player_aio_data_t_cleanup(&rflex_data.aio);
  player_ir_data_t_cleanup(&rflex_data.ir);
  Lock();
  if (!ThreadAlive)
  {
    Unlock();
    break;
  }
  Unlock();

  // release cpu somewhat so other threads can run.
  usleep(1000);
  
  }
  pthread_exit(NULL);
}

int RFLEX::initialize_robot(){
  // Neither g++ nor I can find a definition for thread_is_running - BPG
#if 0
#ifdef _REENTRANT
  if (thread_is_running)
    {
      fprintf(stderr,"Tried to connect to the robot while the command loop "
                  "thread is running.\n");
      fprintf(stderr,"This is a bug in the code, and must be fixed.\n");
      return -1;
    }
#endif
#endif

  if (rflex_open_connection(rflex_configs.serial_port, &rflex_fd) < 0)
    return -1;

  printf("Rflex initialisation called\n");
  rflex_initialize(rflex_fd, (int) M2ARB_ODO_CONV(rflex_configs.mPsec2_trans_acceleration),(int) RAD2ARB_ODO_CONV(rflex_configs.radPsec2_rot_acceleration), 0, 0);
  printf("RFlex init done\n");

  return 0;
}

void RFLEX::reset_odometry() {
  m_odo_x=0;
  m_odo_y=0;
  rad_odo_theta= 0.0;
}

void RFLEX::set_odometry(float m_x, float m_y, float rad_theta) {
  m_odo_x=m_x;
  m_odo_y=m_y;
  rad_odo_theta=rad_theta;
}

void RFLEX::update_everything(player_rflex_data_t* d)
{

  int *arb_ranges = new int[rflex_configs.num_sonars];
  char *abumper_ranges = new char[rflex_configs.bumper_count];
  uint8_t *air_ranges = new uint8_t[rflex_configs.ir_total_count];

  static int initialized = 0;

  double m_new_range_position; double rad_new_bearing_position;
  double mPsec_t_vel;
  double radPsec_r_vel;

  int arb_t_vel, arb_r_vel;
  static int arb_last_range_position;
  static int arb_last_bearing_position;
  int arb_new_range_position;
  int arb_new_bearing_position;
  double m_displacement;
  short a_num_sonars, a_num_bumpers, a_num_ir;

  int batt,brake;

  int i;

  //update status
  rflex_update_status(rflex_fd, &arb_new_range_position,
                      &arb_new_bearing_position, &arb_t_vel,
                      &arb_r_vel);
  mPsec_t_vel=ARB2M_ODO_CONV(arb_t_vel);
  radPsec_r_vel=ARB2RAD_ODO_CONV(arb_r_vel);
  m_new_range_position=ARB2M_ODO_CONV(arb_new_range_position);
  rad_new_bearing_position=ARB2RAD_ODO_CONV(arb_new_bearing_position);

  if (!initialized) {
    initialized = 1;
  } else {
    rad_odo_theta += ARB2RAD_ODO_CONV(arb_new_bearing_position - arb_last_bearing_position);
    rad_odo_theta = normalize_theta(rad_odo_theta);
    m_displacement = ARB2M_ODO_CONV(arb_new_range_position - arb_last_range_position);

    //integrate latest motion into odometry
    m_odo_x += m_displacement * cos(rad_odo_theta);
    m_odo_y += m_displacement * sin(rad_odo_theta);
    d->position.pos.px = m_odo_x;
    d->position.pos.py = m_odo_y;
    while(rad_odo_theta<0)
      rad_odo_theta+=2*M_PI;
    while(rad_odo_theta>2*M_PI)
      rad_odo_theta-=2*M_PI;
    d->position.pos.pa = rad_odo_theta;

    d->position.vel.px = mPsec_t_vel;
    d->position.vel.pa = radPsec_r_vel;
    //TODO - get better stall information (battery draw?)
  }
  d->position.stall = false;

  arb_last_range_position = arb_new_range_position;
  arb_last_bearing_position = arb_new_bearing_position;

   //note - sonar mappings are strange - look in rflex_commands.c
  if(this->sonar_subscriptions)
  {
    // TODO - currently bad sonar data is sent back to clients
    // (not enough data buffered, so sonar sent in wrong order - missing intermittent sonar values - fix this
    a_num_sonars=rflex_configs.num_sonars;

//    pthread_testcancel();
    rflex_update_sonar(rflex_fd, a_num_sonars,
           arb_ranges);
//    pthread_testcancel();
    if (d->sonar.ranges_count!=rflex_configs.sonar_1st_bank_end)
    {
      d->sonar.ranges_count=rflex_configs.sonar_1st_bank_end;
      delete [] d->sonar.ranges;
      d->sonar.ranges = new float[d->sonar.ranges_count];
    }
    for (i = 0; i < rflex_configs.sonar_1st_bank_end; i++){
      d->sonar.ranges[i] = ARB2M_RANGE_CONV(arb_ranges[i]);
    }
    if (d->sonar2.ranges_count!=(rflex_configs.num_sonars - rflex_configs.sonar_2nd_bank_start))
    {
      d->sonar2.ranges_count=(rflex_configs.num_sonars - rflex_configs.sonar_2nd_bank_start);
      delete [] d->sonar2.ranges;
      d->sonar2.ranges = new float[d->sonar2.ranges_count];
    }
    for (i = 0; i < rflex_configs.num_sonars - rflex_configs.sonar_2nd_bank_start; i++){
      d->sonar2.ranges[i] = ARB2M_RANGE_CONV(arb_ranges[rflex_configs.sonar_2nd_bank_start+i]);
    }
  }

  // if someone is subscribed to bumpers copy internal data to device
  if(this->bumper_subscriptions)
  {
    a_num_bumpers=rflex_configs.bumper_count;

//    pthread_testcancel();
    // first make sure our internal state is up to date
    rflex_update_bumpers(rflex_fd, a_num_bumpers, abumper_ranges);
 //   pthread_testcancel();

    d->bumper.bumpers_count=(a_num_bumpers);
    d->bumper.bumpers = (uint8_t*)abumper_ranges;
  }

  // if someone is subscribed to irs copy internal data to device
  if(this->ir_subscriptions)
  {
    a_num_ir=rflex_configs.ir_poses.poses_count;

//    pthread_testcancel();
    // first make sure our internal state is up to date
    rflex_update_ir(rflex_fd, a_num_ir, air_ranges);
//    pthread_testcancel();

    if (d->ir.ranges_count != (unsigned int) a_num_ir)
    {
      d->ir.ranges_count = a_num_ir;
      d->ir.voltages_count = a_num_ir;
      delete [] d->ir.ranges;
      delete [] d->ir.voltages;
      d->ir.ranges = new float [a_num_ir];
      d->ir.voltages = new float [a_num_ir];
    }
    for (int i = 0; i < a_num_ir; ++i)
    {
      d->ir.voltages[i] = air_ranges[i];
      // using power law mapping of form range = a*voltage^b
      float range = pow(air_ranges[i],rflex_configs.ir_b[i]) * rflex_configs.ir_a[i];
      // check for min and max ranges, < min = 0 > max = max
      range = range < rflex_configs.ir_min_range ? 0 : range;
      range = range > rflex_configs.ir_max_range ? rflex_configs.ir_max_range : range;
      d->ir.ranges[i] = (range);

    }
  }

  //this would get the battery,time, and brake state (if we cared)
  //update system (battery,time, and brake also request joystick data)
  rflex_update_system(rflex_fd,&batt,&brake);
  
  // set the bits for the fields we're using
  d->power.valid = PLAYER_POWER_MASK_VOLTS | PLAYER_POWER_MASK_PERCENT;
  
  d->power.volts = static_cast<float>(batt)/100.0 + rflex_configs.power_offset;
  if (d->power.volts > 24)
    d->power.percent = 100;
  else if (d->power.volts < 20)
    d->power.percent = 0;
  else
    d->power.percent = 100.0*(d->power.volts-20.0)/4.0;
}

//this is so things don't crash if we don't load a device
//(and thus it's settings)
void RFLEX::set_config_defaults(){
  memset(&rflex_configs, 0, sizeof(rflex_configs));
  strcpy(rflex_configs.serial_port,"/dev/ttyR0");
  rflex_configs.mPsec2_trans_acceleration=0.500;
  rflex_configs.radPsec2_rot_acceleration=0.500;
}
