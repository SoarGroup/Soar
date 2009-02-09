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
 * Desc: localize device proxy
 * Author: Boyoon Jung, Andrew Howard
 * Date: 20 Jun 2002
 * CVS: $Id: dev_localize.c 6398 2008-05-04 02:12:02Z thjc $
 **************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"


// Local declarations
void playerc_localize_putmsg(playerc_localize_t *device, player_msghdr_t *header,
                           player_localize_data_t *data, size_t len);

// Create a new localize proxy
playerc_localize_t *playerc_localize_create(playerc_client_t *client, int index)
{
  playerc_localize_t *device;

  device = malloc(sizeof(playerc_localize_t));
  memset(device, 0, sizeof(playerc_localize_t));
  playerc_device_init(&device->info, client, PLAYER_LOCALIZE_CODE, index,
                      (playerc_putmsg_fn_t) playerc_localize_putmsg);
    
  return device;
}


// Destroy a localize proxy
void playerc_localize_destroy(playerc_localize_t *device)
{
  playerc_device_term(&device->info);
  free(device->map_cells);
  free(device->hypoths);
  free(device->particles);
  free(device);
  return;
}


// Subscribe to the localize device
int playerc_localize_subscribe(playerc_localize_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the localize device
int playerc_localize_unsubscribe(playerc_localize_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_localize_putmsg(playerc_localize_t *device, player_msghdr_t *header,
                              player_localize_data_t *data, size_t len)
{
  int i;//, k;
  
  device->pending_count = data->pending_count;
  device->pending_time = data->pending_time;
  device->hypoth_count = data->hypoths_count;
  device->hypoths = realloc(device->hypoths,device->hypoth_count*sizeof(device->hypoths[0]));
  for (i = 0; i < data->hypoths_count; i++)
  {
    device->hypoths[i] = data->hypoths[i];
  }
  
  return;
}


// Set the robot pose (mean and covariance)
int playerc_localize_set_pose(playerc_localize_t *device, double pose[3], double cov[3])
{
  player_localize_set_pose_t req;

  req.mean.px = pose[0];
  req.mean.py = pose[1];
  req.mean.pa = pose[2];
  
  req.cov[0] = cov[0];
  req.cov[1] = cov[1];
  req.cov[2] = cov[2];

  if(playerc_client_request(device->info.client, 
                            &device->info,
                            PLAYER_LOCALIZE_REQ_SET_POSE,
                            &req, NULL) < 0)
  {
    PLAYERC_WARN1("%s\n", playerc_error_str());
    return -1;
  }

  return 0;
}


// Get the particle set
int playerc_localize_get_particles(playerc_localize_t *device)
{
  int i;
  player_localize_get_particles_t *req;


  if(playerc_client_request(device->info.client, &device->info,
                            PLAYER_LOCALIZE_REQ_GET_PARTICLES,
                            NULL, (void**) &req) < 0)

    return -1;

  device->mean[0] = req->mean.px;
  device->mean[1] = req->mean.py;
  device->mean[2] = req->mean.pa;

  device->variance = req->variance;

  device->num_particles = req->particles_count;
  device->particles = realloc(device->particles,req->particles_count*sizeof(device->particles[0]));

  for(i=0;i<device->num_particles;i++)
  {
    device->particles[i].pose[0] = req->particles[i].pose.px;
    device->particles[i].pose[1] = req->particles[i].pose.py;
    device->particles[i].pose[2] = req->particles[i].pose.pa;
    device->particles[i].weight = req->particles[i].alpha;
  }
  player_localize_get_particles_t_free(req);
  return 0;
}
