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
 * Desc: Scanning range-finder (FRF) interface
 * Author: Andrew Howard
 * Date: 14 May 2002
 * CVS: $Id: pv_dev_wifi.c 4152 2007-09-17 02:18:59Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Update the wifi configuration
void wifi_update_config(wifi_t *wifi);

// Draw the wifi scan
void wifi_draw(wifi_t *wifi);


// Create a wifi device
wifi_t *wifi_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                      int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  wifi_t *wifi;
  
  wifi = malloc(sizeof(wifi_t));
  wifi->proxy = playerc_wifi_create(client, index);
  wifi->drivername = strdup(drivername);
  wifi->datatime = 0;

  snprintf(section, sizeof(section), "wifi:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "wifi:%d (%s)", index, wifi->drivername);
  wifi->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  wifi->subscribe_item = rtk_menuitem_create(wifi->menu, "Subscribe", 1);

  // Set the initial menu state
  rtk_menuitem_check(wifi->subscribe_item, subscribe);
  
  // Construct figures
  wifi->fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 50);
  
  return wifi;
}


// Destroy a wifi device
void wifi_destroy(wifi_t *wifi)
{
  if (wifi->proxy->info.subscribed)
    playerc_wifi_unsubscribe(wifi->proxy);

  playerc_wifi_destroy(wifi->proxy);
  rtk_fig_destroy(wifi->fig);
  free(wifi->drivername);
  free(wifi);
  
  return;
}


// Update a wifi device
void wifi_update(wifi_t *wifi)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(wifi->subscribe_item))
  {
    if (!wifi->proxy->info.subscribed)
      if (playerc_wifi_subscribe(wifi->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  else
  {
    if (wifi->proxy->info.subscribed)
      if (playerc_wifi_unsubscribe(wifi->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(wifi->subscribe_item, wifi->proxy->info.subscribed);

  if (wifi->proxy->info.subscribed)
  {
    // Draw in the wifi scan if it has been changed.
    if (wifi->proxy->info.datatime != wifi->datatime)
    {
      wifi_draw(wifi);
      wifi->datatime = wifi->proxy->info.datatime;
    }
  }
  else
  {
    // Dont draw the wifi.
    rtk_fig_show(wifi->fig, 0);
  }
}


// Draw the wifi scan
void wifi_draw(wifi_t *wifi)
{
  int i;
  char ntext[64], text[1024];

  rtk_fig_show(wifi->fig, 1);      
  rtk_fig_clear(wifi->fig);
  
  text[0] = 0;
  for (i = 0; i < wifi->proxy->link_count; i++)
  {
    snprintf(ntext, sizeof(ntext), "%s %02d %02d %02d\n",
             wifi->proxy->links[i].ip, wifi->proxy->links[i].qual,
             wifi->proxy->links[i].level, wifi->proxy->links[i].noise);
    strcat(text, ntext);
  }

  // Draw in the wifi reading
  // TODO: get text origin from somewhere
  rtk_fig_color_rgb32(wifi->fig, COLOR_WIFI);
  rtk_fig_text(wifi->fig, +1, +1, 0, text);

  return;
}




