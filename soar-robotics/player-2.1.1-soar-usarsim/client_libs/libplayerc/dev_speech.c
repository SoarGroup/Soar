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
/*  Speech Proxy for libplayerc library. 
 *  Structure based on the rest of libplayerc. 
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_speech_putmsg (playerc_speech_t *device, player_msghdr_t *header,
			     char *data, size_t len);

// Create a new speech proxy
playerc_speech_t *playerc_speech_create(playerc_client_t *client, int index)
{
  playerc_speech_t *device;

  device = malloc(sizeof(playerc_speech_t));
  memset(device, 0, sizeof(playerc_speech_t));
  playerc_device_init(&device->info, client, PLAYER_SPEECH_CODE, index,
                      (playerc_putmsg_fn_t) playerc_speech_putmsg);

  return device;
}

// Destroy a speech proxy
void playerc_speech_destroy(playerc_speech_t *device)
{
  playerc_device_term(&device->info);
  free(device);
}

// Subscribe to the speech device
int playerc_speech_subscribe(playerc_speech_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}

// Un-subscribe from the speech device
int playerc_speech_unsubscribe(playerc_speech_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}

// Process incoming data
void playerc_speech_putmsg(playerc_speech_t *device, player_msghdr_t *header,
			    char *data, size_t len)
{
  /* there's no much to do */
}

/* Set the output for the speech device. */
int playerc_speech_say(playerc_speech_t *device, char *str)
{
  player_speech_cmd_t cmd;
  
  memset(&cmd, 0, sizeof(cmd));
  cmd.string = str;
  cmd.string_count = strlen(str) + 1; 
	
  return playerc_client_write(device->info.client, 
			      &device->info, PLAYER_SPEECH_CMD_SAY, &cmd, NULL);
}

