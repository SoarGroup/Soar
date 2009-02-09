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
 * Desc: GPS device proxy
 * Author: Andrew Howard
 * Date: 26 May 2002
 * CVS: $Id: dev_gps.c 4232 2007-11-01 22:16:23Z gerkey $
 **************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "playerc.h"
#include "error.h"


// Local declarations
void playerc_gps_putmsg(playerc_gps_t *device,
                               player_msghdr_t *header,
                               void *data, size_t len);

// Create a new gps proxy
playerc_gps_t *playerc_gps_create(playerc_client_t *client, int index)
{
  playerc_gps_t *device;

  device = malloc(sizeof(playerc_gps_t));
  memset(device, 0, sizeof(playerc_gps_t));
  playerc_device_init(&device->info, client, PLAYER_GPS_CODE, index,
                      (playerc_putmsg_fn_t) playerc_gps_putmsg);

  return device;
}


// Destroy a gps proxy
void playerc_gps_destroy(playerc_gps_t *device)
{
  playerc_device_term(&device->info);
  free(device);
}


// Subscribe to the gps device
int playerc_gps_subscribe(playerc_gps_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the gps device
int playerc_gps_unsubscribe(playerc_gps_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_gps_putmsg(playerc_gps_t *device,
                               player_msghdr_t *header,
                               void *data, size_t len)
{
  if((header->type == PLAYER_MSGTYPE_DATA) &&
     (header->subtype == PLAYER_GPS_DATA_STATE))
  {
    player_gps_data_t * gps_data = (player_gps_data_t *) data;
    device->utc_time = gps_data->time_sec + ((double) gps_data->time_usec)/1e6;

    device->lat = gps_data->latitude / 1e7;
    device->lon = gps_data->longitude / 1e7;
    device->alt = gps_data->altitude / 1e3;

    device->utm_e = gps_data->utm_e / 100.0;
    device->utm_n = gps_data->utm_n / 100.0;

    device->hdop = gps_data->hdop / 10.0;
    device->vdop = gps_data->vdop / 10.0;

    device->err_horz = gps_data->err_horz;
    device->err_vert = gps_data->err_vert;

    device->quality = gps_data->quality;
    device->sat_count = gps_data->num_sats;
  }
  else
    PLAYERC_WARN2("skipping gps message with unknown type/subtype: %s/%d\n",
                 msgtype_to_str(header->type), header->subtype);
  return;
}

