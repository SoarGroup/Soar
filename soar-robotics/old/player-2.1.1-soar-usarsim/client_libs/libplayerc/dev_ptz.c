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
 * Desc: PTZ device proxy
 * Author: Andrew Howard
 * Date: 26 May 2002
 * CVS: $Id: dev_ptz.c 4227 2007-10-24 22:32:04Z thjc $
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_ptz_putmsg(playerc_ptz_t *device, player_msghdr_t *header,
                         player_ptz_data_t *data, size_t len);

// Create a new ptz proxy
playerc_ptz_t *playerc_ptz_create(playerc_client_t *client, int index)
{
  playerc_ptz_t *device;

  device = malloc(sizeof(playerc_ptz_t));
  memset(device, 0, sizeof(playerc_ptz_t));
  playerc_device_init(&device->info, client, PLAYER_PTZ_CODE, index,
                      (playerc_putmsg_fn_t) playerc_ptz_putmsg);

  return device;
}


// Destroy a ptz proxy
void playerc_ptz_destroy(playerc_ptz_t *device)
{
  playerc_device_term(&device->info);
  free(device);
}


// Subscribe to the ptz device
int playerc_ptz_subscribe(playerc_ptz_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the ptz device
int playerc_ptz_unsubscribe(playerc_ptz_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_ptz_putmsg(playerc_ptz_t *device, player_msghdr_t *header,
                         player_ptz_data_t *data, size_t len)
{
  device->pan  = data->pan;
  device->tilt = data->tilt;
  device->zoom = data->zoom;
  return;
}

// Set the pan, tilt and zoom values.
int playerc_ptz_set(playerc_ptz_t *device, double pan,
                    double tilt, double zoom)
{
  player_ptz_cmd_t cmd;

  cmd.pan  = pan;
  cmd.tilt = tilt;
  cmd.zoom = zoom;

  // set speed = 0 by default
  cmd.panspeed  = 0;
  cmd.tiltspeed = 0;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_PTZ_CMD_STATE, &cmd, NULL);

}

// Query the pan and tilt status.
int playerc_ptz_query_status(playerc_ptz_t *device)
{
  player_ptz_req_status_t *cmd;

  if(playerc_client_request(device->info.client, &device->info,
                            PLAYER_PTZ_REQ_STATUS,
                            NULL,
                            (void**)&cmd) < 0)

    return -1;

  device->status = cmd->status;
  player_ptz_req_status_t_free(cmd);
  return 0;
}

// Set the pan, tilt and zoom values with speed as well.
int playerc_ptz_set_ws(playerc_ptz_t *device, double pan, double tilt,
                       double zoom, double panspeed, double tiltspeed)
{
  player_ptz_cmd_t cmd;

  cmd.pan  = pan;
  cmd.tilt = tilt;
  cmd.zoom = zoom;
  cmd.panspeed  = panspeed;
  cmd.tiltspeed = tiltspeed;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_PTZ_CMD_STATE, &cmd, NULL);
}

// Change control mode
int
playerc_ptz_set_control_mode(playerc_ptz_t *device, int mode)
{
  player_ptz_req_control_mode_t config;

  config.mode = mode;

  return(playerc_client_request(device->info.client,
                                &device->info,
                                PLAYER_PTZ_REQ_CONTROL_MODE,
                                &config, NULL));
}
