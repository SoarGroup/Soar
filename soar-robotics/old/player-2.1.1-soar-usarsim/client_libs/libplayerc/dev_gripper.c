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
 * Desc: Gripper device proxy
 * Author: Doug Blank
 * Date: 13 April 2005
 * CVS: $Id: dev_gripper.c 4232 2007-11-01 22:16:23Z gerkey $
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_gripper_putmsg (playerc_gripper_t *device,
           player_msghdr_t *header, void* generic);

// Create a new gripper proxy
playerc_gripper_t *playerc_gripper_create (playerc_client_t *client, int index)
{
  playerc_gripper_t *device;

  device = malloc (sizeof (playerc_gripper_t));
  memset (device, 0, sizeof (playerc_gripper_t));
  playerc_device_init (&device->info, client, PLAYER_GRIPPER_CODE, index,
                      (playerc_putmsg_fn_t) playerc_gripper_putmsg);
  return device;
}


// Destroy a gripper proxy
void playerc_gripper_destroy (playerc_gripper_t *device)
{
  playerc_device_term (&device->info);
  free (device);
  return;
}


// Subscribe to the gripper device
int playerc_gripper_subscribe (playerc_gripper_t *device, int access)
{
  return playerc_device_subscribe (&device->info, access);
}


// Un-subscribe from the gripper device
int playerc_gripper_unsubscribe (playerc_gripper_t *device)
{
  return playerc_device_unsubscribe (&device->info);
}


// Process incoming data
void playerc_gripper_putmsg (playerc_gripper_t *device, player_msghdr_t *header, void *generic)
{
  if (header->type == PLAYER_MSGTYPE_DATA &&
      header->subtype == PLAYER_GRIPPER_DATA_STATE)
  {
    player_gripper_data_t * data = (player_gripper_data_t*) generic;

    device->state = data->state;
    device->beams = data->beams;
    device->stored = data->stored;
  }
}

// Command the gripper to open
int playerc_gripper_open_cmd (playerc_gripper_t *device)
{
  player_null_t cmd;

  memset (&cmd, 0, sizeof (cmd));
  return playerc_client_write (device->info.client, &device->info, PLAYER_GRIPPER_CMD_OPEN, &cmd, NULL);
}

// Command the gripper to close
int playerc_gripper_close_cmd (playerc_gripper_t *device)
{
  player_null_t cmd;

  memset (&cmd, 0, sizeof (cmd));
  return playerc_client_write (device->info.client, &device->info, PLAYER_GRIPPER_CMD_CLOSE, &cmd, NULL);
}

// Command the gripper to stop
int playerc_gripper_stop_cmd (playerc_gripper_t *device)
{
  player_null_t cmd;

  memset (&cmd, 0, sizeof (cmd));
  return playerc_client_write (device->info.client, &device->info, PLAYER_GRIPPER_CMD_STOP, &cmd, NULL);
}

// Command the gripper to store
int playerc_gripper_store_cmd (playerc_gripper_t *device)
{
  player_null_t cmd;

  memset (&cmd, 0, sizeof (cmd));
  return playerc_client_write (device->info.client, &device->info, PLAYER_GRIPPER_CMD_STORE, &cmd, NULL);
}

// Command the gripper to retrieve
int playerc_gripper_retrieve_cmd (playerc_gripper_t *device)
{
  player_null_t cmd;

  memset (&cmd, 0, sizeof (cmd));
  return playerc_client_write (device->info.client, &device->info, PLAYER_GRIPPER_CMD_RETRIEVE, &cmd, NULL);
}

// Get the geometry.  The writes the result into the proxy
// rather than returning it to the caller.
int playerc_gripper_get_geom (playerc_gripper_t *device)
{
  player_gripper_geom_t *config;

  if(playerc_client_request(device->info.client,&device->info, PLAYER_GRIPPER_REQ_GET_GEOM,
                            NULL, (void**)&config) < 0)
    return -1;

  device->pose = config->pose;
  device->outer_size = config->outer_size;
  device->inner_size = config->inner_size;
  device->num_beams = config->num_beams;
  device->capacity = config->capacity;
  player_gripper_geom_t_free(config);
  return 0;
}

// print human-readable state
void playerc_gripper_printout (playerc_gripper_t *device, const char* prefix)
{
  if (prefix)
    printf ("%s: ", prefix);

  printf ("[%14.3f] pose[(%.2f,%.2f,%.2f),(%.2f,%.2f,%.2f)] outer_size[%.2f,%.2f,%.2f]"
    " inner_size[%.2f,%.2f,%.2f] state[%s] beams[%32X]\n",
    device->info.datatime,
    device->pose.px, device->pose.py, device->pose.pz,
    device->pose.proll, device->pose.ppitch, device->pose.pyaw,
    device->outer_size.sw, device->outer_size.sl, device->outer_size.sh,
    device->inner_size.sw, device->inner_size.sl, device->inner_size.sh,
    (device->state == PLAYER_GRIPPER_STATE_OPEN ? "open" :
      (device->state == PLAYER_GRIPPER_STATE_CLOSED ? "closed" :
        (device->state == PLAYER_GRIPPER_STATE_MOVING ? "moving" : "error"))),
    device->beams);
}
