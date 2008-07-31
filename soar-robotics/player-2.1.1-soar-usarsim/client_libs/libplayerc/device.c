/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) Andrew Howard 2002-2003
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
 * Desc: Common device functions
 * Author: Andrew Howard
 * Date: 13 May 2002
 * CVS: $Id: device.c 4160 2007-09-20 23:15:47Z thjc $
 **************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"


void playerc_device_init(playerc_device_t *device, playerc_client_t *client,
                         int code, int index, playerc_putmsg_fn_t putmsg)
{
  device->id = device;
  device->client = client;
  device->addr.host = 0;
  device->addr.robot = client->port;
  device->addr.interf = code;
  device->addr.index = index;
  device->subscribed = 0;
  device->callback_count = 0;
  device->putmsg = putmsg;

  if (device->client)
    playerc_client_adddevice(device->client, device);
  return;
}


// Finalize the device
void playerc_device_term(playerc_device_t *device)
{
  if (device->client)
    playerc_client_deldevice(device->client, device);
  return;
}


// Subscribe/unsubscribe the device
int playerc_device_subscribe(playerc_device_t *device, int access)
{
  if (playerc_client_subscribe(device->client, device->addr.interf,
                               device->addr.index, access,
                               device->drivername, sizeof(device->drivername)) != 0)
    return -1;
  device->subscribed = 1;
  return 0;
}



// Subscribe/unsubscribe the device
int playerc_device_unsubscribe(playerc_device_t *device)
{
  device->subscribed = 0;
  return playerc_client_unsubscribe(device->client,
                                    device->addr.interf,
                                    device->addr.index);
}

// Query the capabilities of a device
int playerc_device_hascapability(playerc_device_t *device, uint32_t type, uint32_t subtype)
{
  player_capabilities_req_t capreq;
  capreq.type = type;
  capreq.subtype = subtype;

  return playerc_client_request(device->client, device, PLAYER_CAPABILTIES_REQ,
		  &capreq, NULL) >= 0 ? 1 : 0;
}

int playerc_device_get_intprop(playerc_device_t *device, char *property, int32_t *value)
{
  int result = 0;

  player_intprop_req_t req, *resp;
  req.key = property;
  req.key_count = strlen (property) + 1;
  req.value = 0;

  if((result = playerc_client_request(device->client, device,
                            PLAYER_GET_INTPROP_REQ, &req, (void**)&resp)) < 0)
    return result;

  *value = resp->value;
  player_intprop_req_t_free(resp);
  return 0;
}

int playerc_device_set_intprop(playerc_device_t *device, char *property, int32_t value)
{
  int result = 0;

  player_intprop_req_t req;
  req.key = property;
  req.key_count = strlen (property) + 1;
  req.value = value;

  if((result = playerc_client_request(device->client, device,
                            PLAYER_SET_INTPROP_REQ, &req, NULL)) < 0)
    return result;

  return 0;
}

int playerc_device_get_dblprop(playerc_device_t *device, char *property, double *value)
{
  int result = 0;

  player_dblprop_req_t req, *resp;
  req.key = property;
  req.key_count = strlen (property) + 1;
  req.value = 0;

  if((result = playerc_client_request(device->client, device,
                            PLAYER_GET_DBLPROP_REQ, &req, (void**)&resp)) < 0)
    return result;

  *value = resp->value;
  player_dblprop_req_t_free(resp);
  return 0;
}

int playerc_device_set_dblprop(playerc_device_t *device, char *property, double value)
{
  int result = 0;

  player_dblprop_req_t req;
  req.key = property;
  req.key_count = strlen (property) + 1;
  req.value = value;

  if((result = playerc_client_request(device->client, device,
                            PLAYER_SET_DBLPROP_REQ, &req, NULL)) < 0)
    return result;

  return 0;
}

int playerc_device_get_strprop(playerc_device_t *device, char *property, char **value)
{
  int result = 0;

  player_strprop_req_t req, *resp;
  req.key = property;
  req.key_count = strlen (property) + 1;
  req.value = NULL;
  req.value_count = 0;

  if((result = playerc_client_request(device->client, device,
                            PLAYER_GET_STRPROP_REQ, &req, (void**)&resp)) < 0)
    return result;

  if (((*value) = strdup (resp->value)) == NULL)
  {
    player_strprop_req_t_free(resp);
    PLAYER_ERROR ("Failed to allocate memory to store property value");
    return -1;
  }
  player_strprop_req_t_free(resp);
  return 0;
}

int playerc_device_set_strprop(playerc_device_t *device, char *property, char *value)
{
  int result = 0;

  player_strprop_req_t req;
  req.key = property;
  req.key_count = strlen (property) + 1;
  req.value = value;
  req.value_count = strlen (value) + 1;

  if((result = playerc_client_request(device->client, device,
                            PLAYER_SET_STRPROP_REQ, &req, NULL)) < 0)
    return result;

  return 0;
}
