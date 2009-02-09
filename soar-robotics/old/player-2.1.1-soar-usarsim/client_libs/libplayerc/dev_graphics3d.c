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
 * CVS: $Id: dev_graphics3d.c 4375 2008-02-22 20:51:50Z rtv $
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "playerc.h"
#include "error.h"


// Create a new graphics3d proxy
playerc_graphics3d_t *playerc_graphics3d_create(playerc_client_t *client, int index)
{
  playerc_graphics3d_t *device;

  device = malloc(sizeof(playerc_graphics3d_t));
  memset(device, 0, sizeof(playerc_graphics3d_t));

  /* start with a visible drawing color */
  device->color.red = 0xff;

  playerc_device_init(&device->info, client, PLAYER_GRAPHICS3D_CODE, index, NULL );

  return device;
}


// Destroy a graphics3d proxy
void playerc_graphics3d_destroy(playerc_graphics3d_t *device)
{
  playerc_device_term(&device->info);
  free(device);

  return;
}


// Subscribe to the graphics3d device
int playerc_graphics3d_subscribe(playerc_graphics3d_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the graphics3d device
int playerc_graphics3d_unsubscribe(playerc_graphics3d_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


int playerc_graphics3d_setcolor(playerc_graphics3d_t *device, 
			     player_color_t col )
{
  device->color = col;
  return 0;
}

int playerc_graphics3d_translate(playerc_graphics3d_t *device, 
				 double x, double y, double z )
{
  player_graphics3d_cmd_translate_t cmd;
  cmd.x = x;
  cmd.y = y;
  cmd.z = z;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_GRAPHICS3D_CMD_TRANSLATE,
                              &cmd, NULL);
}

int playerc_graphics3d_rotate( playerc_graphics3d_t *device, 
			       double a, double x, double y, double z )
{
  player_graphics3d_cmd_rotate_t cmd;
  cmd.a = a;
  cmd.x = x;
  cmd.y = y;
  cmd.z = z;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_GRAPHICS3D_CMD_ROTATE,
                              &cmd, NULL);
}


int  playerc_graphics3d_draw(playerc_graphics3d_t *device,
           player_graphics3d_draw_mode_t mode, 
           player_point_3d_t pts[],
           int count )
{
  player_graphics3d_cmd_draw_t cmd;

  cmd.draw_mode = mode;
  cmd.points_count = count;
  cmd.points = pts;
  cmd.color = device->color;

  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_GRAPHICS3D_CMD_DRAW,
                              &cmd, NULL);
}

int playerc_graphics3d_clear(playerc_graphics3d_t *device )
{
  return playerc_client_write(device->info.client, &device->info,
                              PLAYER_GRAPHICS3D_CMD_CLEAR,
                              NULL, NULL);
}
