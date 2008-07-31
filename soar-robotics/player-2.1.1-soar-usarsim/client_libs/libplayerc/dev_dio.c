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
/*  dio Proxy for libplayerc library.
 *  Structure based on the rest of libplayerc.
 */
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"


// Local declarations
void playerc_dio_putmsg(playerc_dio_t *device,
                           player_msghdr_t *header,
                           player_dio_data_t *data,
                           size_t len);


// Create a new dio proxy
playerc_dio_t *playerc_dio_create(playerc_client_t *client, int index)
{
  playerc_dio_t *device;

  device = malloc(sizeof(playerc_dio_t));
  memset(device, 0, sizeof(playerc_dio_t));
  playerc_device_init(&device->info, client, PLAYER_DIO_CODE, index,
                      (playerc_putmsg_fn_t) playerc_dio_putmsg);

  return device;
}

// Destroy a dio proxy
void playerc_dio_destroy(playerc_dio_t *device)
{
  playerc_device_term(&device->info);
  free(device);
}

// Subscribe to the dio device
int playerc_dio_subscribe(playerc_dio_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}

// Un-subscribe from the dio device
int playerc_dio_unsubscribe(playerc_dio_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_dio_putmsg(playerc_dio_t *device, player_msghdr_t *header,
                            player_dio_data_t *data, size_t len)
{
  if((header->type == PLAYER_MSGTYPE_DATA) &&
     (header->subtype == PLAYER_DIO_DATA_VALUES))
  {
    device->count = data->count;
    device->digin = data->bits;
  }
}

/* Set the output for the dio device. */
int playerc_dio_set_output(playerc_dio_t *device, uint8_t output_count, uint32_t bits)
{
  player_dio_data_t cmd;

  memset(&cmd, 0, sizeof(cmd));

  cmd.count = output_count;
  cmd.bits = bits;

  return playerc_client_write(device->info.client,
    &device->info, PLAYER_DIO_CMD_VALUES,&cmd,NULL);
}

