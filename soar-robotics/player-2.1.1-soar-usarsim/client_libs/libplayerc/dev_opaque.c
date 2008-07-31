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
 * Desc: Opaque proxy
 * Author: Brad Kratochvil
 * Date: 10 April 2006
 * CVS: $Id: dev_opaque.c 4232 2007-11-01 22:16:23Z gerkey $
 **************************************************************************/
#if HAVE_CONFIG_H
  #include "config.h"
#endif

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_opaque_putmsg(playerc_opaque_t *device,
                           player_msghdr_t *header,
                           player_opaque_data_t *data,
                           size_t len);

// Create a new opaque proxy
playerc_opaque_t *playerc_opaque_create(playerc_client_t *client, int index)
{
  playerc_opaque_t *device;

  device = malloc(sizeof(playerc_opaque_t));
  memset(device, 0, sizeof(playerc_opaque_t));
  playerc_device_init(&device->info, client, PLAYER_OPAQUE_CODE, index,
                      (playerc_putmsg_fn_t) playerc_opaque_putmsg);
  return device;
}


// Destroy a opaque proxy
void playerc_opaque_destroy(playerc_opaque_t *device)
{
  playerc_device_term(&device->info);
  free(device->data);
  free(device);
}


// Subscribe to the opaque device
int playerc_opaque_subscribe(playerc_opaque_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the opaque device
int playerc_opaque_unsubscribe(playerc_opaque_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_opaque_putmsg(playerc_opaque_t *device, player_msghdr_t *header,
                            player_opaque_data_t *data, size_t len)
{
  if((header->type == PLAYER_MSGTYPE_DATA) &&
     (header->subtype == PLAYER_OPAQUE_DATA_STATE))
  {
    device->data_count   = data->data_count;
    device->data = realloc(device->data, sizeof(*device->data)*device->data_count);

    memcpy(device->data, data->data, device->data_count);
  }
  else
    PLAYERC_WARN2("skipping opaque message with unknown type/subtype: %s/%d\n",
                 msgtype_to_str(header->type), header->subtype);
  return;
}

int playerc_opaque_cmd(playerc_opaque_t *device, player_opaque_data_t *data)
{
  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_OPAQUE_CMD,
                              data, NULL);
}

// Send a generic request
int playerc_opaque_req(playerc_opaque_t *device, player_opaque_data_t *request, player_opaque_data_t **reply)
{
  return playerc_client_request(device->info.client, &device->info,
                            PLAYER_OPAQUE_REQ,
                            (void*)request, (void**)reply);
}
