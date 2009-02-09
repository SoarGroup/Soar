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
 * Desc: WSN proxy
 * Author: Radu Bogdan Rusu
 * Date: 30 March 2006
 **************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Process incoming data
void playerc_wsn_putmsg (playerc_wsn_t *device,
                         player_msghdr_t *header,
			 void *data);

// Create a new wsn proxy
playerc_wsn_t *playerc_wsn_create(playerc_client_t *client, int index)
{
    playerc_wsn_t *device;
    device = malloc(sizeof(playerc_wsn_t));
    memset(device, 0, sizeof(playerc_wsn_t));
    playerc_device_init(&device->info, client, PLAYER_WSN_CODE, index,
       (playerc_putmsg_fn_t) playerc_wsn_putmsg);

    return device;
}


// Destroy a wsn proxy
void playerc_wsn_destroy(playerc_wsn_t *device)
{
    playerc_device_term(&device->info);
    free(device);
}


// Subscribe to the wsn device
int playerc_wsn_subscribe(playerc_wsn_t *device, int access)
{
    return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the wsn device
int playerc_wsn_unsubscribe(playerc_wsn_t *device)
{
    return playerc_device_unsubscribe(&device->info);
}

// Process incoming data
void playerc_wsn_putmsg (playerc_wsn_t *device,
			 player_msghdr_t *header,
			 void *data)
{
//    int i, j;

    if((header->type == PLAYER_MSGTYPE_DATA) &&
       (header->subtype == PLAYER_WSN_DATA_STATE))
    {
	player_wsn_data_t* wsn_data = (player_wsn_data_t*)data;
	device->node_type      = wsn_data->node_type;
	device->node_id        = wsn_data->node_id;
	device->node_parent_id = wsn_data->node_parent_id;
	device->data_packet    = wsn_data->data_packet;
    }
    else
	PLAYERC_WARN2("skipping wsn message with unknown type/subtype: %s/%d\n",
	    msgtype_to_str(header->type), header->subtype);
}

// Set the device state.
int
playerc_wsn_set_devstate(playerc_wsn_t *device, int node_id, int group_id,
                         int devnr, int state)
{
  player_wsn_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.node_id  = node_id;
  cmd.group_id = group_id;
  cmd.device   = devnr;
  cmd.enable   = state;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_WSN_CMD_DEVSTATE,
                              &cmd, NULL);
}

// Put the node in sleep mode (0) or wake it up (1).
int
playerc_wsn_power(playerc_wsn_t *device, int node_id, int group_id, int value)
{
  player_wsn_power_config_t config;

  config.node_id  = node_id;
  config.group_id = group_id;
  config.value    = value;

  return(playerc_client_request(device->info.client,
                                &device->info,
                                PLAYER_WSN_REQ_POWER,
                                &config, NULL));
}

// Change the data type to RAW or converted engineering units.
int
playerc_wsn_datatype(playerc_wsn_t *device, int value)
{
  player_wsn_datatype_config_t config;

  config.value = value;

  return(playerc_client_request(device->info.client,
                                &device->info,
                                PLAYER_WSN_REQ_DATATYPE,
                                &config, NULL));
}

// Change data delivery frequency.
int
playerc_wsn_datafreq(playerc_wsn_t *device, int node_id, int group_id,
                     double frequency)
{
  player_wsn_datafreq_config_t config;

  config.node_id   = node_id;
  config.group_id  = group_id;
  config.frequency = frequency;

  return(playerc_client_request(device->info.client,
                                &device->info,
                                PLAYER_WSN_REQ_DATAFREQ,
                                &config, NULL));
}
