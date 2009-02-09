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

/*
 * $Id: nomad_position.cc 4126 2007-08-20 06:37:30Z thjc $
 * 
 * Driver for the Nomadics Nomad 200 robot. Should be easily adapted for other Nomads.
 * Authors: Richard Vaughan (vaughan@sfu.ca)
 * Based on Brian Gerkey et al's P2OS driver.
 * 
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_nomad_position nomad_position
 * @brief Nomadics NOMAD200 motors, odometry

The nomad_position driver controls the wheelbase of the Nomadics
NOMAD200 robot.  This driver is a thin wrapper that exchanges data and
commands with the @ref driver_nomad driver; look there for more
information and an example.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_position2d

@par Requires

- player_interface_nomad

@par Configuration requests

- PLAYER_POSITION2D_REQ_GET_GEOM
  
@par Configuration file options

- none


@author Richard Vaughan, Pawel Zebrowski

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


// so we can access the deviceTable and extract pointers to the sonar
// and position objects
#include <drivertable.h>
#include <devicetable.h>
#include "error.h"
#include "nomad.h"

class NomadPosition:public Driver 
{
  public:

  NomadPosition( ConfigFile* cf, int section);
  virtual ~NomadPosition();
  
  // MessageHandler
  int ProcessMessage(ClientData * client, player_msghdr * hdr, uint8_t * data, uint8_t * resp_data, size_t * resp_len);
  
  virtual int Setup();
  virtual int Shutdown();
  
protected:
  Driver* nomad;
  player_device_id_t nomad_id;
  
};

// a factory creation function
Driver* NomadPosition_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new NomadPosition( cf, section)));
}

// a driver registration function
void NomadPosition_Register(DriverTable* table)
{
  table->AddDriver( "nomad_position",  NomadPosition_Init);
}




NomadPosition::NomadPosition( ConfigFile* cf, int section)
        : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_POSITION2D_CODE, PLAYER_ALL_MODE)
{
  // Must have a nomad
  if (cf->ReadDeviceId(&this->nomad_id, section, "requires",
                       PLAYER_NOMAD_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }
}

NomadPosition::~NomadPosition()
{
  puts( "Destroying NomadPosition driver" );
}

int NomadPosition::Setup()
{
  printf("NomadPosition Setup.. ");
  fflush(stdout);
  
  // if we didn't specify a port for the nomad, use the same port as
  // this device
  if( this->nomad_id.port == 0 )
    this->nomad_id.port = device_id.port;
  
  printf( "finding Nomad (%d:%d:%d).. ",
	  this->nomad_id.port,
	  this->nomad_id.code,
	  this->nomad_id.index ); fflush(stdout);

  // get the pointer to the Nomad
  this->nomad = SubscribeInternal(nomad_id);

  if(!this->nomad)
  {
    PLAYER_ERROR("unable to find nomad device");
    return(-1);
  }
  
  else printf( " OK.\n" );
 
  puts( "NomadPosition setup done" );
  return(0);
}

int NomadPosition::Shutdown()
{
  // Unsubscribe from the laser device
  UnsubscribeInternal(this->nomad_id);

  puts("NomadPosition has been shutdown");
  return(0);
}

////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int NomadPosition::ProcessMessage(ClientData * client, player_msghdr * hdr, uint8_t * data, uint8_t * resp_data, size_t * resp_len)
{
  assert(hdr);
  assert(data);
  assert(resp_data);
  assert(resp_len);
	
  if (MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_GET_GEOM, device_id))
  {
  	assert(*resp_len >= sizeof(player_position_geom_t));
  	player_position_geom_t & geom = *reinterpret_cast<player_position_geom_t *> (resp_data);
    *resp_len = sizeof(player_position_geom_t);

    geom.pose[0] = htons((short) (0)); // x offset
    geom.pose[1] = htons((short) (0)); // y offset
    geom.pose[2] = htons((short) (0)); // a offset
    geom.size[0] = htons((short) (2 * NOMAD_RADIUS_MM )); // x size
    geom.size[1] = htons((short) (2 * NOMAD_RADIUS_MM )); // y size   

    return PLAYER_MSGTYPE_RESP_ACK;
  }

  if (MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 0, device_id))
  {
  	assert(hdr->size == sizeof(player_position_cmd_t));
  	player_position_cmd_t & command = *reinterpret_cast<player_position_cmd_t *> (data);

    // convert from the generic position interface to the
    // Nomad-specific command
    player_nomad_cmd_t cmd;
    memset( &cmd, 0, sizeof(cmd) );
    cmd.vel_trans = (command.xspeed);
    cmd.vel_steer = (command.yawspeed);
    cmd.vel_turret = (command.yspeed);
      
    // command the Nomad device
    nomad->ProcessMessage(PLAYER_MSGTYPE_CMD,0,device_id,sizeof(cmd),(uint8_t*)&cmd);
    *resp_len =0;
    return 0;
  }

  if (MatchMessage(hdr, PLAYER_MSGTYPE_DATA, 0, nomad_id))
  {
  	assert(hdr->size == sizeof(player_nomad_data_t));
  	player_nomad_data_t & nomad_data = *reinterpret_cast<player_nomad_data_t *> (data);
  
    // extract the position data from the Nomad packet
    player_position_data_t pos;
    memset(&pos,0,sizeof(pos));
      
    pos.xpos = nomad_data.x;
    pos.ypos = nomad_data.y;
    pos.yaw = nomad_data.a;
    pos.xspeed = nomad_data.vel_trans;
    pos.yawspeed = nomad_data.vel_steer;
      
    PutMsg(device_id,NULL,PLAYER_MSGTYPE_DATA,0,(void*)&pos, sizeof(pos), NULL);
  }
      
  *resp_len = 0;
  return -1;
}



