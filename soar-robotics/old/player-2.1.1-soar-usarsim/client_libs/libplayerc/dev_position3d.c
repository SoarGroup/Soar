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
 * Desc: Position3d device proxy
 * Author: Andrew Howard
 * Date: 13 May 2002
 * CVS: $Id: dev_position3d.c 4227 2007-10-24 22:32:04Z thjc $
 **************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"


// Local declarations
void playerc_position3d_putmsg(playerc_position3d_t *device,
                               player_msghdr_t *header,
                               player_position3d_data_t *data, size_t len);


// Create a new position3d proxy
playerc_position3d_t
*playerc_position3d_create(playerc_client_t *client, int index)
{
  playerc_position3d_t *device;

  device = malloc(sizeof(playerc_position3d_t));
  memset(device, 0, sizeof(playerc_position3d_t));
  playerc_device_init(&device->info, client, PLAYER_POSITION3D_CODE, index,
                      (playerc_putmsg_fn_t) playerc_position3d_putmsg);


  return device;
}


// Destroy a position3d proxy
void
playerc_position3d_destroy(playerc_position3d_t *device)
{
  playerc_device_term(&device->info);
  free(device);

  return;
}


// Subscribe to the position3d device
int
playerc_position3d_subscribe(playerc_position3d_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the position3d device
int
playerc_position3d_unsubscribe(playerc_position3d_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void
playerc_position3d_putmsg(playerc_position3d_t *device,
                          player_msghdr_t *header,
                          player_position3d_data_t *data, size_t len)
{
  if((header->type == PLAYER_MSGTYPE_DATA) &&
     (header->subtype == PLAYER_POSITION3D_DATA_STATE))
  {
    device->pos_x = data->pos.px;
    device->pos_y = data->pos.py;
    device->pos_z = data->pos.pz;

    device->pos_roll = data->pos.proll;
    device->pos_pitch = data->pos.ppitch;
    device->pos_yaw = data->pos.pyaw;

    device->vel_x = data->vel.px;
    device->vel_y = data->vel.py;
    device->vel_z = data->vel.pz;

    device->vel_roll = data->vel.proll;
    device->vel_pitch = data->vel.ppitch;
    device->vel_yaw = data->vel.pyaw;

    device->stall = data->stall;
  }
  else
    PLAYERC_WARN2("skipping position3d message with unknown type/subtype: %s/%d\n",
                  msgtype_to_str(header->type), header->subtype);
}


// TODO
#if 0
// Process incoming data
void playerc_position3d_putgeom(playerc_position3d_t *device, player_msghdr_t *header,
                                player_position3d_geom_t *data, size_t len)
{
  if (len != sizeof(player_position3d_geom_t))
  {
    PLAYERC_ERR2("reply has unexpected length (%d != %d)", len, sizeof(player_position3d_geom_t));
    return;
  }

  // TODO
  device->pose[0] = ((int16_t) ntohs(data->pose[0])) / 1000.0;
  device->pose[1] = ((int16_t) ntohs(data->pose[1])) / 1000.0;
  device->pose[2] = ((int16_t) ntohs(data->pose[2])) / 1000.0;

  device->pose[3] = ((int16_t) ntohs(data->pose[3]))  / 1000.0;
  device->pose[4] = ((int16_t) ntohs(data->pose[4])) / 1000.0;
  device->pose[5] = ((int16_t) ntohs(data->pose[5])) / 1000.0;

  device->size[0] = ((int16_t) ntohs(data->size[0])) / 1000.0;
  device->size[1] = ((int16_t) ntohs(data->size[1])) / 1000.0;
  device->size[2] = ((int16_t) ntohs(data->size[2])) / 1000.0;
}
#endif

// Enable/disable the motors
int
playerc_position3d_enable(playerc_position3d_t *device, int enable)
{
  player_position3d_power_config_t config;

  memset(&config, 0, sizeof(config));
//  config.request = PLAYER_POSITION3D_REQ_MOTOR_POWER;
  config.state = enable;

  return playerc_client_request(device->info.client, &device->info,
                                PLAYER_POSITION3D_REQ_MOTOR_POWER,
                                &config,NULL);
}


// Get the position3d geometry.  The writes the result into the proxy
// rather than returning it to the caller.
int
playerc_position3d_get_geom(playerc_position3d_t *device)
{
  int len;
  player_position3d_geom_t *config;

  len = playerc_client_request(device->info.client, &device->info,
                               PLAYER_POSITION3D_REQ_GET_GEOM,
                               NULL, (void**)&config);
  if (len < 0)
    return -1;

  //TODO: Actually store the geometry
  player_position3d_geom_t_free(config);
  
  return 0;
}


// Set the robot speed
int
playerc_position3d_set_velocity(playerc_position3d_t *device,
                                double vx, double vy, double vz,
                                double vr, double vp, double vt, int state)
{
  player_position3d_cmd_vel_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.vel.px = vx;
  cmd.vel.py = vy;
  cmd.vel.pz = vz;

  cmd.vel.proll = vr;
  cmd.vel.ppitch = vp;
  cmd.vel.pyaw = vt;

  cmd.state = state;

  return playerc_client_write(device->info.client,
                              &device->info,
                              PLAYER_POSITION3D_CMD_SET_VEL,
                              &cmd,
                              NULL);
}

// Set the target pose (pos,vel)
int
playerc_position3d_set_pose_with_vel(playerc_position3d_t *device,
                                     player_pose3d_t pos,
                                     player_pose3d_t vel)
{
  player_position3d_cmd_pos_t cmd;

  memset(&cmd, 0, sizeof(cmd));

  cmd.pos = pos;
  cmd.vel = vel;

  return playerc_client_write(device->info.client,
                              &device->info,
                              PLAYER_POSITION3D_CMD_SET_POS,
                              &cmd,
                              NULL);
}


// Set the target pose (px,py,pz,pr,pp,pt)
int
playerc_position3d_set_pose(playerc_position3d_t *device,
                            double gx, double gy, double gz,
                            double gr, double gp, double gt)
{
  player_position3d_cmd_pos_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.pos.px = gx;
  cmd.pos.py = gy;
  cmd.pos.pz = gz;

  cmd.pos.proll = gr;
  cmd.pos.ppitch = gp;
  cmd.pos.pyaw = gt;

  return playerc_client_write(device->info.client,
                              &device->info,
                              PLAYER_POSITION3D_CMD_SET_POS,
                              &cmd,
                              NULL);
}

/** For compatibility with old position3d interface */
int
playerc_position3d_set_speed(playerc_position3d_t *device,
                             double vx, double vy, double vz, int state)
{
  return playerc_position3d_set_velocity(device,vx,vy,vz,0,0,0, state);
}

/** For compatibility with old position3d interface */
int
playerc_position3d_set_cmd_pose(playerc_position3d_t *device,
                                double gx, double gy, double gz)
{
  return playerc_position3d_set_pose(device,gx,gy,gz,0,0,0);
}

int
playerc_position3d_set_vel_mode(playerc_position3d_t *device, int aMode)
{
  player_position3d_velocity_mode_config_t config;
  memset(&config, 0, sizeof(config));
  config.value = aMode;

  return playerc_client_request(device->info.client,
                                &device->info,
                                PLAYER_POSITION3D_REQ_VELOCITY_MODE,
                                &config,NULL);
}

int
playerc_position3d_set_odom(playerc_position3d_t *device,
                            double ox, double oy, double oz,
                            double oroll, double opitch, double oyaw)
{
  player_position3d_set_odom_req_t config;

  memset(&config, 0, sizeof(config));

  config.pos.px = ox;
  config.pos.py = oy;
  config.pos.pz = oz;

  config.pos.proll = oroll;
  config.pos.ppitch = opitch;
  config.pos.pyaw = oyaw;

  return playerc_client_request(device->info.client,
                              &device->info,
                              PLAYER_POSITION3D_REQ_SET_ODOM,
                              &config,NULL);

}

int playerc_position3d_reset_odom(playerc_position3d_t *device)
{
  player_position3d_reset_odom_config_t config;

  memset(&config, 0, sizeof(config));



  return playerc_client_request(device->info.client,
                              	&device->info,
				PLAYER_POSITION3D_REQ_RESET_ODOM,
				&config,NULL);
}
