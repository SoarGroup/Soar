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

/* Desc: Driver for getting signal strengths for AODV ah-hoc network software.
 * Author: Andrew Howard ahoward@usc.edu
 * Date: 26 Nov 2002
 * $Id: aodv.cc 6566 2008-06-14 01:00:19Z thjc $
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_aodv aodv
 * @brief Linux AODV ad hoc networking

The aodv driver reports signal strengths for AODV ah-hoc network software.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_wifi

@par Requires

- None

@par Configuration requests

- none

@par Configuration file options

- none
 
@par Example 

@verbatim
driver
(
  name "aodv"
  provides ["wifi:0"]
)
@endverbatim

@author Andrew Howard

*/
/** @} */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <netinet/in.h>

#include <libplayercore/playercore.h>

extern PlayerTime *GlobalTime;

#define AODV_INFO_FILE "/proc/aodv/route_table"

class Aodv : public Driver
{
  public: Aodv( ConfigFile *cf, int section);

  // Initialize driver
  public: virtual int Setup();

  // Finalize driver
  public: virtual int Shutdown();

  // Get the current readings
  public: virtual void Update();

  // File handle for the /proc file system entry
  protected: FILE *file;
  player_wifi_data_t data;
};


// Instantiate driver for given interface
Driver * Aodv_Init( ConfigFile *cf, int section)
{ 
  return ((Driver*)(new Aodv(cf, section)));
}


// Register driver type
void Aodv_Register(DriverTable *table)
{
  table->AddDriver("aodv", Aodv_Init);
  return;
}


// Constructor
Aodv::Aodv( ConfigFile *cf, int section)
  : Driver(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_WIFI_CODE)
{
  return;
}


// Initialize driver
int Aodv::Setup()
{
  // Just open the file
  this->file = fopen(AODV_INFO_FILE, "r");
  if (this->file == NULL)
  {
    PLAYER_ERROR2("unable to open [%s]; error [%s]",
                  AODV_INFO_FILE, strerror(errno));
    return -1;
  }
  memset(&data, 0, sizeof(data));
  return 0;
}


// Finalize driver
int Aodv::Shutdown()
{
  fclose(this->file);
  free(data.links);
  return 0;
}


// Get new data
void Aodv::Update()
{
  int n;
  unsigned int link_count;
  player_wifi_link_t *link;
  char ip[16], next_ip[16];
  int seq, hop;
  int qual, level, noise;

  //struct timeval curr;

  // Rewind to start of file
  rewind(this->file);

  // Skip header
  for (n = 0; n < 5;)
    if (fgetc(this->file) == '\n')
      n++;

  link_count = 0;
  while (TRUE)
  {
    n = fscanf(this->file, " %16s %d %d %16s ( %d ) ", ip, &seq, &hop, next_ip, &level);
    if (n == EOF)
      break;
    if (n < 5)
      continue;

    qual = 0;
    noise = 0;
    
    printf("aodv %s : %d\n", ip, level);

    if (link_count > data.links_count)
    {
      data.links = (player_wifi_link_t*)realloc(data.links, sizeof(data.links[0])*link_count);
      data.links_count++;
    }
    assert(data.links);
    link = &data.links[link_count];
    link_count++;

    strncpy((char*)link->ip, ip, sizeof(link->ip));
    //link->qual_type = PLAYER_WIFI_QUAL_UNKNOWN;
    link->qual = (qual);
    link->noise = (noise);
  }

  // Copy data to the server's buffer
  Publish(device_addr, PLAYER_MSGTYPE_DATA, PLAYER_WIFI_DATA_STATE, &data);


  return;
}
