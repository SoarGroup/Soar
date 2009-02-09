/* 
 *  PlayerViewer
 *  Copyright (C) Andrew Howard 2002
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
/***************************************************************************
 * Desc: 
 * Author: 
 * Date: 
 * CVS: $Id: pv_dev_power.c 4152 2007-09-17 02:18:59Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Update the power configuration
void power_update_config(power_t *power);

// Draw the power scan
void power_draw(power_t *power);


// Create a power device
power_t *power_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                      int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  power_t *power;
  
  power = malloc(sizeof(power_t));
  power->proxy = playerc_power_create(client, index);
  power->drivername = strdup(drivername);
  power->datatime = 0;

  snprintf(section, sizeof(section), "power:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "power:%d (%s)", index, power->drivername);
  power->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  power->subscribe_item = rtk_menuitem_create(power->menu, "Subscribe", 1);

  // Set the initial menu state
  rtk_menuitem_check(power->subscribe_item, subscribe);
  
  // Construct figures
  power->fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 50);
  
  return power;
}


// Destroy a power device
void power_destroy(power_t *power)
{
  if (power->proxy->info.subscribed)
    playerc_power_unsubscribe(power->proxy);

  playerc_power_destroy(power->proxy);
  rtk_fig_destroy(power->fig);
  free(power->drivername);
  free(power);
  
  return;
}


// Update a power device
void power_update(power_t *power)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(power->subscribe_item))
  {
    if (!power->proxy->info.subscribed)
      if (playerc_power_subscribe(power->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  else
  {
    if (power->proxy->info.subscribed)
      if (playerc_power_unsubscribe(power->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(power->subscribe_item, power->proxy->info.subscribed);

  if (power->proxy->info.subscribed)
  {
    // Draw in the power scan if it has been changed.
    if (power->proxy->info.datatime != power->datatime)
    {
      power_draw(power);
      power->datatime = power->proxy->info.datatime;
    }
  }
  else
  {
    // Dont draw the power.
    rtk_fig_show(power->fig, 0);
  }
}


// Draw the power scan
void power_draw(power_t *power)
{
  char text[256];
  char buf[64];

  rtk_fig_show(power->fig, 1);      
  rtk_fig_clear(power->fig);

  // TODO: get text origin from somewhere
  
  // Draw in the power reading
  rtk_fig_color_rgb32(power->fig, COLOR_POWER);
  
  text[0] = 0;
  
  if( power->proxy->valid & PLAYER_POWER_MASK_VOLTS )
    {
      snprintf(buf, sizeof(buf), "Voltage: %4.1fV", 
	       power->proxy->charge );
      
      strncat( text, buf, sizeof(text) );
    }

  if( power->proxy->valid & PLAYER_POWER_MASK_PERCENT )
    {
      snprintf(buf, sizeof(buf), "(%5.1f%%)", 
	       power->proxy->percent);
      strncat( text, buf, sizeof(text) );
    }
  
  if( power->proxy->valid & PLAYER_POWER_MASK_JOULES )
    {
      snprintf(buf, sizeof(buf), " Joules: %4f", 
	       power->proxy->joules);
      strncat( text, buf, sizeof(text) );
    }
  
  if( power->proxy->valid & PLAYER_POWER_MASK_WATTS )
    {
      snprintf(buf, sizeof(buf), " Watts: %4.1f", 
	       power->proxy->watts);
      strncat( text, buf, sizeof(text) );
    }
  

  if( power->proxy->valid & PLAYER_POWER_MASK_CHARGING )
    strncat( text, 
	     power->proxy->charging ? " CHARGING" : "",
	     sizeof(text) );

  rtk_fig_text(power->fig, -1, +1, 0, text);

  return;
}




