/*
 *  libplayerc : a Player client library
 *  Copyright (C) Andrew Howard 2002-2003
 * *
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
 * Desc: Graphics2d device proxy
 * Author: Richard Vaughan
 * Date: 8 February 2006
 * CVS: $Id: dev_graphics2d.c 4232 2007-11-01 22:16:23Z gerkey $
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "playerc.h"
#include "error.h"


// Create a new graphics2d proxy
playerc_graphics2d_t *playerc_graphics2d_create(playerc_client_t *client, int index)
{
  playerc_graphics2d_t *device;

  device = malloc(sizeof(playerc_graphics2d_t));
  memset(device, 0, sizeof(playerc_graphics2d_t));

  /* start with a visible drawing color */
  device->color.red = 0xff;

  playerc_device_init(&device->info, client, PLAYER_GRAPHICS2D_CODE, index, NULL );

  return device;
}


// Destroy a graphics2d proxy
void playerc_graphics2d_destroy(playerc_graphics2d_t *device)
{
  playerc_device_term(&device->info);
  free(device);

  return;
}


// Subscribe to the graphics2d device
int playerc_graphics2d_subscribe(playerc_graphics2d_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the graphics2d device
int playerc_graphics2d_unsubscribe(playerc_graphics2d_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


int playerc_graphics2d_setcolor(playerc_graphics2d_t *device, 
			     player_color_t col )
{
  device->color = col;
  return 0;
}

int playerc_graphics2d_draw_points(playerc_graphics2d_t *device, 
			      player_point_2d_t pts[], int count )
{
  player_graphics2d_cmd_points_t cmd;

  cmd.points_count = count;
  cmd.points= pts;
  cmd.color = device->color;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_GRAPHICS2D_CMD_POINTS,
                              &cmd, NULL);
}


int playerc_graphics2d_draw_polyline(playerc_graphics2d_t *device, 
				     player_point_2d_t pts[], 
				     int count )
{
  player_graphics2d_cmd_polyline_t cmd;
  
  cmd.points_count = count;
  cmd.points = pts;
  cmd.color = device->color;
   
  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_GRAPHICS2D_CMD_POLYLINE,
                              &cmd, NULL);
}

int playerc_graphics2d_draw_polygon(playerc_graphics2d_t *device, 
				    player_point_2d_t pts[], 
				    int count,
				    int filled,
				    player_color_t fill_color )
{
  player_graphics2d_cmd_polygon_t cmd;

  cmd.points_count = count;
  cmd.points = pts;
  cmd.color = device->color;
  cmd.filled = filled;

  if( filled )
    memcpy( &cmd.fill_color, &fill_color, sizeof(cmd.fill_color));
  
  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_GRAPHICS2D_CMD_POLYGON,
                              &cmd, NULL);  
}

int playerc_graphics2d_clear(playerc_graphics2d_t *device )
{
  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_GRAPHICS2D_CMD_CLEAR,
                              NULL, NULL);
}
