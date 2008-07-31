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
 * Desc: Power device proxy
 * Author: Andrew Howard
 * Date: 13 May 2002
 * CVS: $Id: dev_power.c 4227 2007-10-24 22:32:04Z thjc $
 **************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_power_putmsg(playerc_power_t *device, player_msghdr_t *header,
                              player_power_data_t *data, size_t len);


// Create a new power proxy
playerc_power_t *playerc_power_create(playerc_client_t *client, int index)
{
  playerc_power_t *device;

  device = malloc(sizeof(playerc_power_t));
  memset(device, 0, sizeof(playerc_power_t));
  playerc_device_init(&device->info, client, PLAYER_POWER_CODE, index,
                      (playerc_putmsg_fn_t) playerc_power_putmsg);

  
  return device;
}


// Destroy a power proxy
void playerc_power_destroy(playerc_power_t *device)
{
  playerc_device_term(&device->info);
  free(device);

  return;
}


// Subscribe to the power device
int playerc_power_subscribe(playerc_power_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}


// Un-subscribe from the power device
int playerc_power_unsubscribe(playerc_power_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}


// Process incoming data
void playerc_power_putmsg(playerc_power_t *device, player_msghdr_t *header,
                              player_power_data_t *data, size_t len)
{
  device->valid = data->valid;
  
  if( device->valid & PLAYER_POWER_MASK_VOLTS )
    device->charge = data->volts;

  if( device->valid & PLAYER_POWER_MASK_PERCENT )
    device->percent = data->percent;

  if( device->valid & PLAYER_POWER_MASK_JOULES )
    device->joules = data->joules;

  if( device->valid & PLAYER_POWER_MASK_WATTS )
    device->watts = data->watts;

  if( device->valid & PLAYER_POWER_MASK_CHARGING )
    device->charging = data->charging;

  return;
}

