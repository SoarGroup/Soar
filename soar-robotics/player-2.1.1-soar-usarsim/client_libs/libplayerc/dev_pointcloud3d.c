/*
 *  libplayerc : a Player client library
 *  Copyright (C) Andrew Howard 2002-2003
 * *
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
 * Desc: PointCloud3D device proxy
 * Author: Radu Bogdan Rusu
 * Date: 6 September 2006
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Process incoming data
void playerc_pointcloud3d_putmsg (playerc_pointcloud3d_t *device,
                                  player_msghdr_t *header,
	                          void *data);

// Create a new pointcloud3d proxy
playerc_pointcloud3d_t *playerc_pointcloud3d_create (playerc_client_t *client, int index)
{
  playerc_pointcloud3d_t *device;

  device = malloc (sizeof (playerc_pointcloud3d_t));
  memset (device, 0, sizeof (playerc_pointcloud3d_t));

  playerc_device_init (&device->info, client, PLAYER_POINTCLOUD3D_CODE, index,
      (playerc_putmsg_fn_t) playerc_pointcloud3d_putmsg);

  return device;
}

// Destroy a pointcloud3d proxy
void playerc_pointcloud3d_destroy (playerc_pointcloud3d_t *device)
{
  playerc_device_term (&device->info);
  free (device->points);
  free (device);

  return;
}

// Subscribe to the pointcloud3d device
int playerc_pointcloud3d_subscribe (playerc_pointcloud3d_t *device, int access)
{
  return playerc_device_subscribe (&device->info, access);
}


// Un-subscribe from the pointcloud3d device
int playerc_pointcloud3d_unsubscribe (playerc_pointcloud3d_t *device)
{
  return playerc_device_unsubscribe (&device->info);
}

// Process incoming data
void playerc_pointcloud3d_putmsg (playerc_pointcloud3d_t *device,
			          player_msghdr_t *header,
		                  void *data)
{
    if((header->type == PLAYER_MSGTYPE_DATA) &&
       (header->subtype == PLAYER_POINTCLOUD3D_DATA_STATE))
    {
	player_pointcloud3d_data_t* pc3_data = (player_pointcloud3d_data_t*)data;
        device->points_count = pc3_data->points_count;
        device->points = realloc(device->points, device->points_count*sizeof(device->points[0]));
        memcpy (device->points, pc3_data->points,
        	sizeof (player_pointcloud3d_element_t)*device->points_count);
    }
    else
	PLAYERC_WARN2("skipping pointcloud3d message with unknown type/subtype: %s/%d\n",
	    msgtype_to_str(header->type), header->subtype);
}
