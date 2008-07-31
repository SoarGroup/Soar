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

#include <string.h>
#include <stdlib.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_limb_putmsg(playerc_limb_t *device,
                         player_msghdr_t *header,
                         player_limb_data_t *data, size_t len);

// Create an actarray proxy
playerc_limb_t *playerc_limb_create(playerc_client_t *client, int index)
{
  playerc_limb_t *device;

  device = malloc(sizeof(playerc_limb_t));
  memset(device, 0, sizeof(playerc_limb_t));
  playerc_device_init(&device->info, client, PLAYER_LIMB_CODE, index,
                       (playerc_putmsg_fn_t) playerc_limb_putmsg);

  return device;
}

// Destroy an actarray proxy
void playerc_limb_destroy(playerc_limb_t *device)
{
  playerc_device_term(&device->info);
  free(device);
}

// Subscribe to the actarray device
int playerc_limb_subscribe(playerc_limb_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}

// Un-subscribe from the actarray device
int playerc_limb_unsubscribe(playerc_limb_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}

void playerc_limb_putmsg(playerc_limb_t *device,
                         player_msghdr_t *header,
                         player_limb_data_t *data, size_t len)
{
  if((header->type == PLAYER_MSGTYPE_DATA) && (header->subtype == PLAYER_LIMB_DATA_STATE))
  {
    device->data.position.px = data->position.px;
    device->data.position.py = data->position.py;
    device->data.position.pz = data->position.pz;

    device->data.approach.px = data->approach.px;
    device->data.approach.py = data->approach.py;
    device->data.approach.pz = data->approach.pz;

    device->data.orientation.px = data->orientation.px;
    device->data.orientation.py = data->orientation.py;
    device->data.orientation.pz = data->orientation.pz;

    device->data.state = data->state;
  }
  else
    PLAYERC_WARN2("skipping limb message with unknown type/subtype: %s/%d\n",
                  msgtype_to_str(header->type), header->subtype);
}

// Get the limb geometry
int playerc_limb_get_geom(playerc_limb_t *device)
{
  player_limb_geom_req_t *geom;

  if(playerc_client_request(device->info.client, &device->info,
     PLAYER_LIMB_REQ_GEOM,
     NULL, (void**)&geom) < 0)
    return -1;

  device->geom.basePos.px = geom->basePos.px;
  device->geom.basePos.py = geom->basePos.py;
  device->geom.basePos.pz = geom->basePos.pz;
  player_limb_geom_req_t_free(geom);
  return 0;
}

// Command the end effector to move home
int playerc_limb_home_cmd(playerc_limb_t *device)
{
  player_null_t cmd;

  memset(&cmd, 0, sizeof(cmd));

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_LIMB_CMD_HOME,
                              &cmd, NULL);
}

// Command the end effector to stop immediatly
int playerc_limb_stop_cmd(playerc_limb_t *device)
{
  player_null_t cmd;

  memset(&cmd, 0, sizeof(cmd));

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_LIMB_CMD_STOP,
                              &cmd, NULL);
}

// Command the end effector to move to a specified pose
int playerc_limb_setpose_cmd(playerc_limb_t *device, float pX, float pY, float pZ, float aX, float aY, float aZ, float oX, float oY, float oZ)
{
  player_limb_setpose_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.position.px = pX;  cmd.position.py = pY;  cmd.position.pz = pZ;
  cmd.approach.px = aX;  cmd.approach.py = aY;  cmd.approach.pz = aZ;
  cmd.orientation.px = oX;  cmd.orientation.py = oY;  cmd.orientation.pz = oZ;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_LIMB_CMD_SETPOSE,
                              &cmd, NULL);
}

// Command the end effector to move to a specified position
int playerc_limb_setposition_cmd(playerc_limb_t *device, float pX, float pY, float pZ)
{
  player_limb_setposition_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.position.px = pX;
  cmd.position.py = pY;
  cmd.position.pz = pZ;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_LIMB_CMD_SETPOSITION,
                              &cmd, NULL);
}

// Command the end effector to move along the provided vector from
int playerc_limb_vecmove_cmd(playerc_limb_t *device, float x, float y, float z, float length)
{
  player_limb_vecmove_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.direction.px = x;
  cmd.direction.py = y;
  cmd.direction.pz = z;
  cmd.length = length;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_LIMB_CMD_VECMOVE,
                              &cmd, NULL);
}

// Turn the power to the limb on or off
int playerc_limb_power(playerc_limb_t *device, uint32_t enable)
{
  player_limb_power_req_t config;

  config.value = enable;

  return playerc_client_request(device->info.client, &device->info,
                                PLAYER_LIMB_REQ_POWER,
                                &config, NULL);
}

// Turn the brakes of all actuators in the limb that have them on or off
int playerc_limb_brakes(playerc_limb_t *device, uint32_t enable)
{
  player_limb_brakes_req_t config;

  config.value = enable;

  return playerc_client_request(device->info.client, &device->info,
                                PLAYER_LIMB_REQ_BRAKES,
                                &config, NULL);
}

// Set the speed of the end effector for all subsequent movement commands
int playerc_limb_speed_config(playerc_limb_t *device, float speed)
{
  player_limb_speed_req_t config;

  config.speed = speed;

  return playerc_client_request(device->info.client, &device->info,
                                PLAYER_LIMB_REQ_SPEED,
                                &config, NULL);
}
