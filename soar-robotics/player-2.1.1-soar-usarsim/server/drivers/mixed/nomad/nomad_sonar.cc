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
 * $Id: nomad_sonar.cc 4126 2007-08-20 06:37:30Z thjc $
 * 
 * Driver for the Nomadics Nomad 200 robot. Should be easily adapted for other Nomads.
 * Authors: Richard Vaughan (vaughan@sfu.ca)
 * Based on Brian Gerkey et al's P2OS driver.
 * 
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_nomad_sonar nomad_sonar
 * @brief Nomadics NOMAD200 sonar array

The nomad_sonar driver controls the sonars of the Nomadics
NOMAD200 robot.  This driver is a thin wrapper that exchanges data and
commands with the @ref driver_nomad driver; look there for more
information and an example.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_sonar

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
// and sonar objects
#include <drivertable.h>
#include <devicetable.h>
#include "error.h"
#include "nomad.h"

class NomadSonar:public Driver 
{
  public:

  NomadSonar( ConfigFile* cf, int section);
  virtual ~NomadSonar();
  
  // MessageHandler
  int ProcessMessage(ClientData * client, player_msghdr * hdr, uint8_t * data, uint8_t * resp_data, size_t * resp_len);
  
  virtual int Setup();
  virtual int Shutdown();
  
protected:
  Driver* nomad;
  player_device_id_t nomad_id;
  
};

// a factory creation function
Driver* NomadSonar_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new NomadSonar( cf, section)));
}

// a driver registration function
void NomadSonar_Register(DriverTable* table)
{
  table->AddDriver( "nomad_sonar",  NomadSonar_Init);
}




NomadSonar::NomadSonar( ConfigFile* cf, int section)
        : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_SONAR_CODE, PLAYER_READ_MODE)
{
  // Must have a nomad
  if (cf->ReadDeviceId(&this->nomad_id, section, "requires",
                       PLAYER_NOMAD_CODE, -1, NULL) != 0)
  {
    this->SetError(-1);    
    return;
  }
}

NomadSonar::~NomadSonar()
{
  puts( "Destroying NomadSonar driver" );
}

int NomadSonar::Setup()
{
  printf("NomadSonar Setup.. ");
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

  puts( "NomadSonar setup done" );
  return(0);
}

int NomadSonar::Shutdown()
{
  // Unsubscribe from the laser device
  UnsubscribeInternal(this->nomad_id);

  puts("NomadSonar has been shutdown");
  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Process an incoming message
int NomadSonar::ProcessMessage(ClientData * client, player_msghdr * hdr, uint8_t * data, uint8_t * resp_data, size_t * resp_len)
{
  assert(hdr);
  assert(data);
  assert(resp_data);
  assert(resp_len);
	
  if (MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_SONAR_REQ_GET_GEOM, device_id))
  {
  	assert(*resp_len >= sizeof(player_sonar_geom_t));
  	player_sonar_geom_t & geom = *reinterpret_cast<player_sonar_geom_t *> (resp_data);
    *resp_len = sizeof(player_sonar_geom_t);

    double interval = (M_PI*2.0)/PLAYER_NOMAD_SONAR_COUNT;
    double radius = NOMAD_RADIUS_MM;
	    
    geom.pose_count = htons((uint16_t)PLAYER_NOMAD_SONAR_COUNT);
    for (int i = 0; i < PLAYER_NOMAD_SONAR_COUNT; i++)
    {
      double angle = interval * i;
	  geom.poses[i][0] = htons((int16_t)rint(radius*cos(angle)));
	  geom.poses[i][1] = htons((int16_t)rint(radius*sin(angle)));
	  geom.poses[i][2] = htons((int16_t)RTOD(angle));
    }
    return PLAYER_MSGTYPE_RESP_ACK;
  }

  if (MatchMessage(hdr, PLAYER_MSGTYPE_DATA, 0, nomad_id))
  {
  	assert(hdr->size == sizeof(player_nomad_data_t));
  	player_nomad_data_t & nomad_data = *reinterpret_cast<player_nomad_data_t *> (data);
  
    // extract the sonar data from the Nomad packet
    player_sonar_data_t player_data;
    memset(&player_data,0,sizeof(player_data));
      
    player_data.range_count = ntohs((uint16_t)PLAYER_NOMAD_SONAR_COUNT);
  
    memcpy( &player_data.ranges, 
     &nomad_data.sonar, 
     PLAYER_NOMAD_SONAR_COUNT * sizeof(uint16_t) );
      
    PutMsg(device_id,NULL,PLAYER_MSGTYPE_DATA,0,(void*)&player_data, sizeof(player_data), NULL);
  }
      
  *resp_len = 0;
  return -1;
}


