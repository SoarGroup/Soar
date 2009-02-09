/*
 *  libplayerc : a Player client library
 *  dev_blinkenlight.c - interface to a Player blinkenlight device
 *  Copyright (C) Richard Vaughan 2008, based on examples by Andrew Howard
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

#include <string.h>
#include <stdlib.h>

#include "playerc.h"
#include "error.h"

// Local declarations
void playerc_blinkenlight_putmsg(playerc_blinkenlight_t *device,
                             player_msghdr_t *header,
                             void* data, size_t len);

// Create an blinkenlight proxy
playerc_blinkenlight_t *playerc_blinkenlight_create(playerc_client_t *client, int index)
{
  playerc_blinkenlight_t *device;

  device = malloc(sizeof(playerc_blinkenlight_t));
  memset(device, 0, sizeof(playerc_blinkenlight_t));
  playerc_device_init(&device->info, client, PLAYER_BLINKENLIGHT_CODE, index,
                       (playerc_putmsg_fn_t) playerc_blinkenlight_putmsg);

  return device;
}

// Destroy an blinkenlight proxy
void playerc_blinkenlight_destroy(playerc_blinkenlight_t *device)
{
  playerc_device_term(&device->info);
  free(device);
}

// Subscribe to the blinkenlight device
int playerc_blinkenlight_subscribe(playerc_blinkenlight_t *device, int access)
{
  return playerc_device_subscribe(&device->info, access);
}

// Un-subscribe from the blinkenlight device
int playerc_blinkenlight_unsubscribe(playerc_blinkenlight_t *device)
{
  return playerc_device_unsubscribe(&device->info);
}

void playerc_blinkenlight_putmsg(playerc_blinkenlight_t *device,
				 player_msghdr_t *header,
				 void* data, size_t len)
{
  if((header->type == PLAYER_MSGTYPE_DATA) && 
     (header->subtype == PLAYER_BLINKENLIGHT_DATA_STATE))
    {
      player_blinkenlight_data_t* blink = (player_blinkenlight_data_t*)data;
      
      device->enabled = blink->enable;	    
      device->duty_cycle = blink->dutycycle;
      device->period = blink->period;
      device->red = blink->color.red;
      device->green = blink->color.green;
      device->blue = blink->color.blue;
    }
  else
    PLAYERC_WARN2("skipping blinkenlight message with unknown type/subtype: %s/%d\n",
                  msgtype_to_str(header->type), header->subtype);
}

/** Set the light color. The light should probably also be enabled before you see it.*/
int playerc_blinkenlight_color( playerc_blinkenlight_t *device, 
				uint32_t id,
				uint8_t red,
				uint8_t green,
				uint8_t blue )
{     
  player_blinkenlight_cmd_color_t cmd;
  memset( &cmd, 0, sizeof(cmd));
  cmd.id = id;
  cmd.color.red = red;
  cmd.color.green = green;
  cmd.color.blue = blue;
  
  return playerc_client_write( device->info.client, 
			       &device->info,
			       PLAYER_BLINKENLIGHT_CMD_COLOR,
			       &cmd, NULL);
}

/** Turn the light on and off (may not be visible when on depending on the color).*/
int playerc_blinkenlight_enable( playerc_blinkenlight_t *device, 
				 uint32_t enable )
{     
  player_blinkenlight_cmd_power_t cmd;
  memset( &cmd, 0, sizeof(cmd));
  cmd.enable = enable;
  
  return playerc_client_write( device->info.client, 
			       &device->info,
			       PLAYER_BLINKENLIGHT_CMD_POWER,
			       &cmd, NULL);
}

/** Set the light flashing by specifying the period in seconds and the
    mark/space ratio (0.0 to 1.0) */
int playerc_blinkenlight_blink( playerc_blinkenlight_t *device, 
				uint32_t id,
				float period,
				float duty_cycle )
{     
  player_blinkenlight_cmd_flash_t cmd;
  memset( &cmd, 0, sizeof(cmd));  
  cmd.id = id;
  cmd.period = period;
  cmd.dutycycle = duty_cycle;
  
  return playerc_client_write( device->info.client, 
			       &device->info,
			       PLAYER_BLINKENLIGHT_CMD_FLASH,
			       &cmd, NULL);
}
