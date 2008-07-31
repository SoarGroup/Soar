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
 * Desc: Digital I/O
 * Author: Brad Kratochvil
 * Date: 30.4.2006
 * CVS: $Id: pv_dev_dio.c 4227 2007-10-24 22:32:04Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Update the dio configuration
void dio_update_config(dio_t *dio);

// Draw the dio scan
void dio_draw(dio_t *dio);


// Create a dio device
dio_t *dio_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                  int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  dio_t *dio;

  dio = malloc(sizeof(dio_t));
  dio->proxy = playerc_dio_create(client, index);
  dio->drivername = strdup(drivername);
  dio->datatime = 0;

  snprintf(section, sizeof(section), "dio:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "dio:%d (%s)", index, dio->drivername);
  dio->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  dio->subscribe_item = rtk_menuitem_create(dio->menu, "Subscribe", 1);

  // Set the initial menu state
  rtk_menuitem_check(dio->subscribe_item, subscribe);

  // Construct figures
  dio->fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 50);

  return dio;
}


// Destroy a dio device
void dio_destroy(dio_t *dio)
{
  if (dio->proxy->info.subscribed)
    playerc_dio_unsubscribe(dio->proxy);

  playerc_dio_destroy(dio->proxy);
  rtk_fig_destroy(dio->fig);
  free(dio->drivername);
  free(dio);

  return;
}


// Update a dio device
void dio_update(dio_t *dio)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(dio->subscribe_item))
  {
    if (!dio->proxy->info.subscribed)
      if (playerc_dio_subscribe(dio->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  else
  {
    if (dio->proxy->info.subscribed)
      if (playerc_dio_unsubscribe(dio->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(dio->subscribe_item, dio->proxy->info.subscribed);

  if (dio->proxy->info.subscribed)
  {
    // Draw in the dio scan if it has been changed.
    if (dio->proxy->info.datatime != dio->datatime)
    {
      dio_draw(dio);
      dio->datatime = dio->proxy->info.datatime;
    }
  }
  else
  {
    // Dont draw the dio.
    rtk_fig_show(dio->fig, 0);
  }
}


// Draw the dio scan
void dio_draw(dio_t *dio)
{
  int i;
  char ntext[64], str[1024];

  uint32_t digin = dio->proxy->digin;
  uint32_t count = dio->proxy->count;

  rtk_fig_show(dio->fig, 1);
  rtk_fig_clear(dio->fig);

  sprintf(str, "", str);
  if (count > 0)
  {
    for (i = count-1; i >= 0 ; i--)
    {
      sprintf(str, "%s%i", str, (digin & (1 << i)) > 0);
      if (3==(count-1-i)%4)
        sprintf(str, "%s ", str);
    }
  }

   rtk_fig_color_rgb32(dio->fig, COLOR_DIO);
   rtk_fig_text(dio->fig, +1, +0.5, 0, str);

  return;
}




