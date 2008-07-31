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
#include <assert.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_actarray_putmsg(playerc_actarray_t *device,
                             player_msghdr_t *header,
                             player_actarray_data_t *data, size_t len);

// Create an actarray proxy
playerc_actarray_t *playerc_actarray_create(playerc_client_t *client, int index)
{
  playerc_actarray_t *device;

  device = malloc(sizeof(playerc_actarray_t));
  memset(device, 0, sizeof(playerc_actarray_t));
  playerc_device_init(&device->info, client, PLAYER_ACTARRAY_CODE, index,
                       (playerc_putmsg_fn_t) playerc_actarray_putmsg);

  return device;
}

// Destroy an actarray proxy
void playerc_actarray_destroy(playerc_actarray_t *device)
{
  playerc_device_term(&device->info);
  free(device->actuators_data);
  free(device->actuators_geom);
  free(device);
}

// Subscribe to the actarray device
int playerc_actarray_subscribe(playerc_actarray_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}

// Un-subscribe from the actarray device
int playerc_actarray_unsubscribe(playerc_actarray_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}

void playerc_actarray_putmsg(playerc_actarray_t *device,
                             player_msghdr_t *header,
                             player_actarray_data_t *data, size_t len)
{
  int i = 0;

  if((header->type == PLAYER_MSGTYPE_DATA) && (header->subtype == PLAYER_ACTARRAY_DATA_STATE))
  {
    device->actuators_count = data->actuators_count;
    device->actuators_data = realloc(device->actuators_data,device->actuators_count*sizeof(device->actuators_data[0]));
    for (i = 0; i < device->actuators_count; i++)
    {
      device->actuators_data[i] = data->actuators[i];
    }
    device->motor_state = data->motor_state;
  }
  else
    PLAYERC_WARN2("skipping actarray message with unknown type/subtype: %s/%d\n",
                  msgtype_to_str(header->type), header->subtype);
}

/** Accessor method for the actuator data */
player_actarray_actuator_t playerc_actarray_get_actuator_data(playerc_actarray_t *device, int index)
{
	assert(index < device->actuators_count);
	return device->actuators_data[index];
}

/** Accessor method for the actuator geom */
player_actarray_actuatorgeom_t playerc_actarray_get_actuator_geom(playerc_actarray_t *device, int index)
{
	assert(index < device->actuators_geom_count);
	return device->actuators_geom[index];
}

// Get the actarray geometry
int playerc_actarray_get_geom(playerc_actarray_t *device)
{
  player_actarray_geom_t *geom;
  int ii = 0, result = 0;

  if((result = playerc_client_request(device->info.client, &device->info,
      PLAYER_ACTARRAY_REQ_GET_GEOM, NULL, (void*)&geom)) < 0)
    return result;
  device->actuators_geom_count = geom->actuators_count;
  device->actuators_geom = realloc(device->actuators_geom,device->actuators_geom_count*sizeof(device->actuators_geom[0]));
  for (ii = 0; ii < device->actuators_geom_count; ii++)
  {
    device->actuators_geom[ii] = geom->actuators[ii];
  }
  device->base_pos = geom->base_pos;
  device->base_orientation = geom->base_orientation;
  player_actarray_geom_t_free(geom);
  return 0;
}

// Command a joint in the array to move to a specified position
int playerc_actarray_position_cmd(playerc_actarray_t *device, int joint, float position)
{
  player_actarray_position_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.joint = joint;
  cmd.position = position;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_ACTARRAY_CMD_POS,
                              &cmd, NULL);
}

// Command all joints in the array to move with a specified current
int playerc_actarray_multi_position_cmd(playerc_actarray_t *device, float *positions, int positions_count)
{
  player_actarray_multi_position_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.positions=positions;
  cmd.positions_count = positions_count;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_ACTARRAY_CMD_MULTI_POS,
                              &cmd, NULL);
}


// Command a joint in the array to move at a specified speed
int playerc_actarray_speed_cmd(playerc_actarray_t *device, int joint, float speed)
{
  player_actarray_speed_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.joint = joint;
  cmd.speed = speed;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_ACTARRAY_CMD_SPEED,
                              &cmd, NULL);
}


// Command all joints in the array to move with a specified current
int playerc_actarray_multi_speed_cmd(playerc_actarray_t *device, float *speeds, int speeds_count)
{
  player_actarray_multi_speed_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.speeds = speeds;
  cmd.speeds_count = speeds_count;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_ACTARRAY_CMD_MULTI_SPEED,
                              &cmd, NULL);
}


// Command a joint (or, if joint is -1, the whole array) to go to its home position
int playerc_actarray_home_cmd(playerc_actarray_t *device, int joint)
{
  player_actarray_home_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.joint = joint;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_ACTARRAY_CMD_HOME,
                              &cmd, NULL);
}

// Command a joint in the array to move with a specified current
int playerc_actarray_current_cmd(playerc_actarray_t *device, int joint, float current)
{
  player_actarray_current_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.joint = joint;
  cmd.current = current;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_ACTARRAY_CMD_CURRENT,
                              &cmd, NULL);
}

// Command all joints in the array to move with a specified current
int playerc_actarray_multi_current_cmd(playerc_actarray_t *device, float *currents, int currents_count)
{
  player_actarray_multi_current_cmd_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.currents = currents;
  cmd.currents_count = currents_count;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_ACTARRAY_CMD_MULTI_CURRENT,
                              &cmd, NULL);
}

// Turn the power to the array on or off
int playerc_actarray_power(playerc_actarray_t *device, uint8_t enable)
{
  player_actarray_power_config_t config;

  config.value = enable;

  return playerc_client_request(device->info.client, &device->info,
                                PLAYER_ACTARRAY_REQ_POWER,
                                &config, NULL);
}

// Turn the brakes of all actuators in the array that have them on or off
int playerc_actarray_brakes(playerc_actarray_t *device, uint8_t enable)
{
  player_actarray_brakes_config_t config;

  config.value = enable;

  return playerc_client_request(device->info.client, &device->info,
                                PLAYER_ACTARRAY_REQ_BRAKES,
                                &config, NULL);
}

// Set the speed of a joint (-1 for all joints) for all subsequent movement commands
int playerc_actarray_speed_config(playerc_actarray_t *device, int joint, float speed)
{
  player_actarray_speed_config_t config;

  config.joint = joint;
  config.speed = speed;

  return playerc_client_request(device->info.client, &device->info,
                                PLAYER_ACTARRAY_REQ_SPEED,
                                &config, NULL);
}

// Set the speed of a joint (-1 for all joints) for all subsequent movement commands
int playerc_actarray_accel_config(playerc_actarray_t *device, int joint, float accel)
{
  player_actarray_accel_config_t config;

  config.joint = joint;
  config.accel = accel;

  return playerc_client_request(device->info.client, &device->info,
                                PLAYER_ACTARRAY_REQ_ACCEL,
                                &config, NULL);
}



