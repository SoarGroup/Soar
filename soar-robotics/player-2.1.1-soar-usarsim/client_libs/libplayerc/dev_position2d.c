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
 * Desc: Position2d device proxy
 * Author: Andrew Howard
 * Date: 13 May 2002
 * CVS: $Id: dev_position2d.c 4227 2007-10-24 22:32:04Z thjc $
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_position2d_putmsg(playerc_position2d_t *device,
                               player_msghdr_t *header,
                               player_position2d_data_t *data, size_t len);

// Create a new position2d proxy
playerc_position2d_t *playerc_position2d_create(playerc_client_t *client, int index)
{
  playerc_position2d_t *device;

  device = malloc(sizeof(playerc_position2d_t));
  memset(device, 0, sizeof(playerc_position2d_t));
  playerc_device_init(&device->info, client, PLAYER_POSITION2D_CODE, index,
                      (playerc_putmsg_fn_t) playerc_position2d_putmsg);


  return device;
}


// Destroy a position2d proxy
void playerc_position2d_destroy(playerc_position2d_t *device)
{
  playerc_device_term(&device->info);
  free(device);

  return;
}


// Subscribe to the position2d device
int playerc_position2d_subscribe(playerc_position2d_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the position2d device
int playerc_position2d_unsubscribe(playerc_position2d_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_position2d_putmsg(playerc_position2d_t *device,
                               player_msghdr_t *header,
                               player_position2d_data_t *data, size_t len)
{
  if((header->type == PLAYER_MSGTYPE_DATA) &&
     (header->subtype == PLAYER_POSITION2D_DATA_STATE))
  {
    device->px = data->pos.px;
    device->py = data->pos.py;
    device->pa = data->pos.pa;

    device->vx = data->vel.px;
    device->vy = data->vel.py;
    device->va = data->vel.pa;

    device->stall = data->stall;
  }
  else
    PLAYERC_WARN2("skipping position2d message with unknown type/subtype: %s/%d\n",
                 msgtype_to_str(header->type), header->subtype);
}

// Enable/disable the motors
int
playerc_position2d_enable(playerc_position2d_t *device, int enable)
{
  player_position2d_power_config_t config;

  config.state = enable;

  return(playerc_client_request(device->info.client,
                                &device->info,
                                PLAYER_POSITION2D_REQ_MOTOR_POWER,
                                &config, NULL));
}

int
playerc_position2d_position_control(playerc_position2d_t *device, int type)
{
  player_position2d_position_mode_req_t config;

  config.state = type;

  return(playerc_client_request(device->info.client, &device->info,
                                PLAYER_POSITION2D_REQ_VELOCITY_MODE,
                                &config, NULL));
}

// Get the position2d geometry.  The writes the result into the proxy
// rather than returning it to the caller.
int
playerc_position2d_get_geom(playerc_position2d_t *device)
{
  player_position2d_geom_t *geom;

  if(playerc_client_request(device->info.client, &device->info,
                            PLAYER_POSITION2D_REQ_GET_GEOM,
                            NULL, (void**)&geom) < 0)
    return(-1);

  device->pose[0] = geom->pose.px;
  device->pose[1] = geom->pose.py;
  device->pose[2] = geom->pose.pyaw;
  device->size[0] = geom->size.sl;
  device->size[1] = geom->size.sw;
  player_position2d_geom_t_free(geom);
  return(0);
}


// Set the robot speed
int
playerc_position2d_set_cmd_vel(playerc_position2d_t *device,
                               double vx, double vy, double va, int state)
{
  player_position2d_cmd_vel_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.vel.px = vx;
  cmd.vel.py = vy;
  cmd.vel.pa = va;
  cmd.state = state;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_POSITION2D_CMD_VEL,
                              &cmd, NULL);
}

// Set the target pose
int
playerc_position2d_set_cmd_pose_with_vel(playerc_position2d_t *device,
                                         player_pose2d_t pos, player_pose2d_t vel,
                                         int state)
{
  player_position2d_cmd_pos_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.pos = pos;
  cmd.vel = vel;
  cmd.state = state;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_POSITION2D_CMD_POS,
                              &cmd, NULL);
}


// Set the target pose
int
playerc_position2d_set_cmd_pose(playerc_position2d_t *device,
                                double gx, double gy, double ga, int state)
{
  player_position2d_cmd_pos_t cmd;

  memset(&cmd, 0, sizeof(cmd));
  cmd.pos.px = gx;
  cmd.pos.py = gy;
  cmd.pos.pa = ga;
  cmd.state = state;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_POSITION2D_CMD_POS,
                              &cmd, NULL);
}

// Set the target pose
int
playerc_position2d_set_cmd_car(playerc_position2d_t *device,
                                double vx, double a)
{
  player_position2d_cmd_car_t cmd;
  cmd.velocity = vx;
  cmd.angle = a;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_POSITION2D_CMD_CAR,
                              &cmd, NULL);
}

// Set the odometry offset
int
playerc_position2d_set_odom(playerc_position2d_t *device,
                            double ox, double oy, double oa)
{
  player_position2d_set_odom_req_t req;

  req.pose.px = ox;
  req.pose.py = oy;
  req.pose.pa = oa;

  return(playerc_client_request(device->info.client,
                                &device->info,
                                PLAYER_POSITION2D_REQ_SET_ODOM,
                                &req, NULL));
}

void playerc_position2d_print( playerc_position2d_t * device,
             const char* prefix )
{
  if( prefix )
    printf( "%s: ", prefix );

  printf( "#time\t\tpx\tpy\tpa\tvx\txvy\tva\tstall\n"
    "%14.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.d\n",
    device->info.datatime,
    device->px,
    device->py,
    device->pa,
    device->vx,
    device->vy,
    device->va,
    device->stall );
}
