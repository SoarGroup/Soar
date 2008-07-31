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
 * Desc: HEALTH proxy
 * Author: M. Ruoss
 * Date: 18 Juli 2006
 **************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Process incoming data
void playerc_health_putmsg (playerc_health_t *device,
                         player_msghdr_t *header,
			 player_health_data_t *data, size_t len);

// Create a new health proxy
playerc_health_t *playerc_health_create(playerc_client_t *client, int index)
{
    playerc_health_t *device;

    device = malloc(sizeof(playerc_health_t));
    memset(device, 0, sizeof(playerc_health_t));
    playerc_device_init(&device->info, client, PLAYER_HEALTH_CODE, index,
       (playerc_putmsg_fn_t) playerc_health_putmsg);

    return device;
}


// Destroy a health proxy
void playerc_health_destroy(playerc_health_t *device)
{
    playerc_device_term(&device->info);
    free(device);
}


// Subscribe to the health device
int playerc_health_subscribe(playerc_health_t *device, int access)
{
    return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the health device
int playerc_health_unsubscribe(playerc_health_t *device)
{
    return playerc_device_unsubscribe(&device->info);
}

// Process incoming data
void playerc_health_putmsg (playerc_health_t *device,
			 player_msghdr_t *header,
			 player_health_data_t *data, size_t len)
{

    if((header->type == PLAYER_MSGTYPE_DATA) &&
       (header->subtype == PLAYER_HEALTH_DATA_STATE))
    {
	device->cpu_usage.idle    = data->cpu_usage.idle;
	device->cpu_usage.system    = data->cpu_usage.system;
	device->cpu_usage.user    = data->cpu_usage.user;
	device->mem.total	= data->mem.total;
	device->mem.used	= data->mem.used;
 	device->mem.free	= data->mem.free;
 	device->swap.total    = data->swap.total;
	device->swap.used    = data->swap.used;
    	device->swap.free    = data->swap.free;
    }
    else
	PLAYERC_WARN2("skipping health message with unknown type/subtype: %s/%d\n",
	    msgtype_to_str(header->type), header->subtype);
}
