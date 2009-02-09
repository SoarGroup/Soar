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
/***************************************************************************
 * Desc: RFID reader proxy
 * Author: Radu Bogdan Rusu
 * Date: 31 January 2006
 **************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Process incoming data
void playerc_rfid_putmsg (playerc_rfid_t *device,
                          player_msghdr_t *header,
			  void *data);

// Create a new rfid proxy
playerc_rfid_t *playerc_rfid_create(playerc_client_t *client, int index)
{
    playerc_rfid_t *device;
    device = malloc(sizeof(playerc_rfid_t));
    memset(device, 0, sizeof(playerc_rfid_t));
    playerc_device_init(&device->info, client, PLAYER_RFID_CODE, index,
       (playerc_putmsg_fn_t) playerc_rfid_putmsg);

    return device;
}


// Destroy a rfid proxy
void playerc_rfid_destroy(playerc_rfid_t *device)
{
  int i;
  playerc_device_term(&device->info);
  if (device->tags) 
  {
    for (i = 0; i < device->tags_count; i++)
      free(device->tags[i].guid);
  }
  free(device);
}


// Subscribe to the rfid device
int playerc_rfid_subscribe(playerc_rfid_t *device, int access)
{
    return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the rfid device
int playerc_rfid_unsubscribe(playerc_rfid_t *device)
{
    return playerc_device_unsubscribe(&device->info);
}

// Process incoming data
void playerc_rfid_putmsg (playerc_rfid_t *device,
			  player_msghdr_t *header,
			  void *data)
{
  int i;

  if((header->type == PLAYER_MSGTYPE_DATA) &&
	(header->subtype == PLAYER_RFID_DATA_TAGS))
  {
    player_rfid_data_t* rfid_data = (player_rfid_data_t*)data;
    // clean up our existing tags
    if (device->tags) 
    {
      for (i = 0; i < device->tags_count; i++)
        free(device->tags[i].guid);
    }
    device->tags_count = rfid_data->tags_count;
    device->tags = realloc(device->tags,device->tags_count * sizeof(device->tags[0]));
    memset(device->tags,0,device->tags_count * sizeof(device->tags[0]));
    for (i = 0; i < device->tags_count; i++)
    {
      device->tags[i].type = rfid_data->tags[i].type;
      device->tags[i].guid_count = rfid_data->tags[i].guid_count;
      device->tags[i].guid = malloc(device->tags[i].guid_count);
      memcpy(device->tags[i].guid,rfid_data->tags[i].guid,device->tags[i].guid_count);
    }
  }
  else
	PLAYERC_WARN2("skipping rfid message with unknown type/subtype: %s/%d\n",
	    msgtype_to_str(header->type), header->subtype);
}

