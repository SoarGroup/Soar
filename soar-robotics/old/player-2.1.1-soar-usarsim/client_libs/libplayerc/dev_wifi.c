/*
 *  libplayerc : a Player client library
 *  Copyright (C) Andrew Howard 2002-2003
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) Andrew Howard 2003
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/***************************************************************************
 * Desc: WiFi device proxy
 * Author: Andrew Howard
 * Date: 13 May 2002
 * CVS: $Id: dev_wifi.c 4259 2007-11-26 21:50:21Z gerkey $
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_wifi_putmsg(playerc_wifi_t *self, player_msghdr_t *header,
                          player_wifi_data_t *data, size_t len);


// Create a new wifi proxy
playerc_wifi_t *playerc_wifi_create(playerc_client_t *client, int index)
{
  playerc_wifi_t *self;

  self = malloc(sizeof(playerc_wifi_t));
  memset(self, 0, sizeof(playerc_wifi_t));
  playerc_device_init(&self->info, client, PLAYER_WIFI_CODE, index,
                      (playerc_putmsg_fn_t) playerc_wifi_putmsg);

  return self;
}


// Destroy a wifi proxy
void playerc_wifi_destroy(playerc_wifi_t *self)
{
  playerc_device_term(&self->info);
  free(self->links);
  free(self);
}


// Subscribe to the wifi device
int playerc_wifi_subscribe(playerc_wifi_t *self, int access)
{
  return playerc_device_subscribe(&self->info, access);
}


// Un-subscribe from the wifi device
int playerc_wifi_unsubscribe(playerc_wifi_t *self)
{
  return playerc_device_unsubscribe(&self->info);
}


// Process incoming data
void playerc_wifi_putmsg(playerc_wifi_t *device, player_msghdr_t *header,
                          player_wifi_data_t *data, size_t len)
{
  int i;

  if((header->type == PLAYER_MSGTYPE_DATA))
  {
    device->link_count = data->links_count;
    device->links = realloc(device->links, sizeof(*device->links)*device->link_count);

    // copy all available link information
    for (i = 0; i < device->link_count; i++)
    {
      memset(device->links[i].mac,0,sizeof(device->links[i].mac));
      memcpy(device->links[i].mac, data->links[i].mac, data->links[i].mac_count);    
      memset(device->links[i].ip,0,sizeof(device->links[i].ip));
      memcpy(device->links[i].ip, data->links[i].ip, data->links[i].ip_count);
      memset(device->links[i].essid,0,sizeof(device->links[i].essid));
      memcpy(device->links[i].essid, data->links[i].essid, data->links[i].essid_count);

      device->links[i].mode = data->links[i].mode;
      device->links[i].encrypt = data->links[i].encrypt;
      device->links[i].freq = data->links[i].freq;
      device->links[i].qual = data->links[i].qual;
      device->links[i].level = data->links[i].level;
      device->links[i].noise = data->links[i].noise;
    }
  }
  return;
}

// Get link state
playerc_wifi_link_t *playerc_wifi_get_link(playerc_wifi_t *self, int link)
{
  //if (link >= self->link_count)
  //  return NULL;

  return self->links + link;
}

