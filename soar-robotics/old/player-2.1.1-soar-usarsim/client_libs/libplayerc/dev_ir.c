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
 * Desc: ir proxy
 * Author: Toby Collett (based on ir proxy by Andrew Howard), Richard Vaughan
 * Date: 13 Feb 2004
 * CVS: $Id: dev_ir.c 4346 2008-02-07 02:23:00Z rtv $
 **************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"


// Local declarations

// Process incoming data
void playerc_ir_putmsg(playerc_ir_t *device, 
                          player_msghdr_t *header,
                          void *data);

// Create a new ir proxy
playerc_ir_t *playerc_ir_create(playerc_client_t *client, int index)
{
  playerc_ir_t *device;

  device = malloc(sizeof(playerc_ir_t));
  memset(device, 0, sizeof(playerc_ir_t));

  playerc_device_init(&device->info, client, PLAYER_IR_CODE, index,
                      (playerc_putmsg_fn_t) playerc_ir_putmsg);
    
  return device;
}


// Destroy a ir proxy
void playerc_ir_destroy(playerc_ir_t *device)
{
  playerc_device_term(&device->info);
  free(device);
}


// Subscribe to the ir device
int playerc_ir_subscribe(playerc_ir_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the ir device
int playerc_ir_unsubscribe(playerc_ir_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_ir_putmsg(playerc_ir_t *device, 
                          player_msghdr_t *header,
                          void *data)
{
  if((header->type == PLAYER_MSGTYPE_DATA) &&
     (header->subtype == PLAYER_IR_DATA_RANGES))
    {
      player_ir_data_t_copy( &device->data,(player_ir_data_t *)data ); 
    }
}


// Get the ir geometry.  The writes the result into the proxy
// rather than returning it to the caller.
int playerc_ir_get_geom(playerc_ir_t *device)
{
  player_ir_pose_t *geom;
  int ret;
  ret = playerc_client_request(device->info.client, &device->info,PLAYER_IR_REQ_POSE, NULL, (void**)&geom);
  if (ret < 0)
    return ret;
  player_ir_pose_t_copy(&device->poses, geom);
  player_ir_pose_t_free(geom);
  return 0;
  
}
