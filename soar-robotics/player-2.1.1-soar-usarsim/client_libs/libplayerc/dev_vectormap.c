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
 * Desc: Map device proxy
 * Author: Benjamin Morelli
 * Date: July 2007
 * CVS: $Id: dev_vectormap.c 6398 2008-05-04 02:12:02Z thjc $
 **************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

/*#if HAVE_ZLIB_H
#include <zlib.h>
#endif*/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "playerc.h"
#include "error.h"
#include <libplayerxdr/playerxdr.h>

/** Dummy function passed as a function pointer GEOS when it is initialised. GEOS uses this for logging. */
inline void geosprint(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	fprintf(stderr,"GEOSError: ");
	vfprintf(stderr,format, ap);
	fflush(stderr);
	va_end(ap);
};

// Create a new vectormap proxy
playerc_vectormap_t *playerc_vectormap_create(playerc_client_t *client, int index)
{
  playerc_vectormap_t *device = NULL;
  device = malloc(sizeof(playerc_vectormap_t));
  memset(device, 0, sizeof(playerc_vectormap_t));

  playerc_device_init(&device->info, client, PLAYER_VECTORMAP_CODE, index,
                       (playerc_putmsg_fn_t) NULL);

  return device;
}

// Destroy a vectormap proxy
void playerc_vectormap_destroy(playerc_vectormap_t *device)
{
  playerc_device_term(&device->info);
  playerc_vectormap_cleanup(device);
  free(device);
}

// Subscribe to the vectormap device
int playerc_vectormap_subscribe(playerc_vectormap_t *device, int access)
{
#ifdef HAVE_GEOS
  initGEOS(geosprint,geosprint);
#endif
  device->geom = NULL;
  return playerc_device_subscribe(&device->info, access);
}

// Un-subscribe from the vectormap device
int playerc_vectormap_unsubscribe(playerc_vectormap_t *device)
{
  if (device->geom)
  {
#ifdef HAVE_GEOS
    GEOSGeom_destroy(device->geom);
#endif
    device->geom = NULL;
  }
#ifdef HAVE_GEOS
  finishGEOS();
#endif
  return playerc_device_unsubscribe(&device->info);
}

// Get vectormap meta-data
int playerc_vectormap_get_map_info(playerc_vectormap_t* device)
{
  int ii;
  player_vectormap_info_t *info_req;

  // try to get map info
  if (playerc_client_request(
      device->info.client,
      &device->info,
      PLAYER_VECTORMAP_REQ_GET_MAP_INFO,
      NULL,
      (void**)&info_req) < 0)
  {
    PLAYERC_ERR("failed to get vectormap info");
    return -1;
  }

  playerc_vectormap_cleanup(device);
  device->srid = info_req->srid;
  device->extent = info_req->extent;
  device->layers_count = info_req->layers_count;

  device->layers_data = calloc(device->layers_count, sizeof(player_vectormap_layer_data_t*));
  device->layers_info = calloc(device->layers_count, sizeof(player_vectormap_layer_info_t*));
  if (!device->layers_data || !device->layers_info)
  {
    PLAYERC_ERR("calloc failed. failed to get vectormap info");
    return -1;
  }

  for (ii=0; ii<device->layers_count; ++ii)
  {
    device->layers_info[ii] = player_vectormap_layer_info_t_clone(&info_req->layers[ii]);
  }
  player_vectormap_info_t_free(info_req);
  return 0;
}

// Get layer data
int playerc_vectormap_get_layer_data(playerc_vectormap_t *device, unsigned layer_index)
{
  player_vectormap_layer_data_t data_req, *data_resp;
  memset(&data_req, 0, sizeof(data_req));

  data_req.name = strdup(device->layers_info[layer_index]->name);
  data_req.name_count = device->layers_info[layer_index]->name_count;

  if (playerc_client_request(
      device->info.client,
      &device->info,
      PLAYER_VECTORMAP_REQ_GET_LAYER_DATA,
      &data_req,
      (void**)&data_resp) < 0)
  {
    PLAYERC_ERR("failed to get layer data");
    player_vectormap_layer_data_t_cleanup(&data_req);
    return -1;
  }
  player_vectormap_layer_data_t_cleanup(&data_req);
  player_vectormap_layer_data_t_free(device->layers_data[layer_index]);
  device->layers_data[layer_index] = data_resp;

  return 0;
}

// Write layer data
int playerc_vectormap_write_layer(playerc_vectormap_t *device, const player_vectormap_layer_data_t * data)
{
  if (playerc_client_request(
      device->info.client,
      &device->info,
      PLAYER_VECTORMAP_REQ_WRITE_LAYER,
      data,
      NULL) < 0)
  {
    PLAYERC_ERR("failed to write layer data");
    return -1;
  }
  return 0;
}


GEOSGeom playerc_vectormap_get_feature_data(playerc_vectormap_t *device, unsigned layer_index, unsigned feature_index)
{
#ifdef HAVE_GEOS
  if (device->geom)
  {
    GEOSGeom_destroy(device->geom);
    device->geom = NULL;
  }
  device->geom = GEOSGeomFromWKB_buf(
    device->layers_data[layer_index]->features[feature_index].wkb,
    device->layers_data[layer_index]->features[feature_index].wkb_count
  );
  return device->geom;
#else
  return NULL;
#endif
}

void playerc_vectormap_cleanup(playerc_vectormap_t *device)
{
  unsigned ii;
  if (device->layers_data)
  {
    for (ii=0; ii<device->layers_count; ++ii)
    {
      player_vectormap_layer_data_t_free(device->layers_data[ii]);
      player_vectormap_layer_info_t_free(device->layers_info[ii]);
    }
    free(device->layers_data);
    device->layers_data = NULL;
    free(device->layers_info);
    device->layers_info = NULL;
  }
  device->srid = -1;
  device->layers_count = 0;
  memset(&device->extent, 0, sizeof(player_extent2d_t));
}
