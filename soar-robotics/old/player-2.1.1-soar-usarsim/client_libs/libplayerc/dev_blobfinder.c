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
 * Desc: Visual blob finder proxy
 * Author: Andrew Howard
 * Date: 24 May 2002
 * CVS: $Id: dev_blobfinder.c 4232 2007-11-01 22:16:23Z gerkey $
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"


// Local declarations
void playerc_blobfinder_putmsg( playerc_blobfinder_t *device, 
			        player_msghdr_t *header,
			        void *datap );

// Create a new blobfinder proxy
playerc_blobfinder_t *playerc_blobfinder_create(playerc_client_t *client, int index)
{
  playerc_blobfinder_t *device;

  device = malloc(sizeof(playerc_blobfinder_t));
  memset(device, 0, sizeof(playerc_blobfinder_t));

  playerc_device_init(&device->info, client, PLAYER_BLOBFINDER_CODE, index,
                      (playerc_putmsg_fn_t) playerc_blobfinder_putmsg);

  return device;
}


// Destroy a blobfinder proxy
void playerc_blobfinder_destroy(playerc_blobfinder_t *device)
{
  playerc_device_term(&device->info);
  free(device->blobs);
  free(device);
}


// Subscribe to the blobfinder device
int playerc_blobfinder_subscribe(playerc_blobfinder_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the blobfinder device
int playerc_blobfinder_unsubscribe(playerc_blobfinder_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_blobfinder_putmsg(playerc_blobfinder_t *device, 
			       player_msghdr_t *header,
			       void* datap )
{
  if( header->type == PLAYER_MSGTYPE_DATA &&
      header->subtype == PLAYER_BLOBFINDER_DATA_BLOBS)
    {
      player_blobfinder_data_t *data = (player_blobfinder_data_t*)datap;
      
      device->width  = data->width;
      device->height = data->height;
      
      // threshold the number of blobs to avoid overunning the array
      device->blobs_count =data->blobs_count;
      device->blobs = realloc(device->blobs, device->blobs_count * sizeof(device->blobs[0]));
      memcpy(device->blobs, data->blobs, sizeof (player_blobfinder_blob_t)*device->blobs_count);

    }

  return;
}

