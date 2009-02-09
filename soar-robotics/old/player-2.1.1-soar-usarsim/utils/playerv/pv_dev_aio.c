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
 * Desc: AIO interface
 * Author: Brad kratochvil
 * Date: 3 May 2006
 * CVS: $Id: pv_dev_aio.c 4152 2007-09-17 02:18:59Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Update the aio configuration
void aio_update_config(aio_t *aio);

// Draw the aio scan
void aio_draw(aio_t *aio);


// Create a aio device
aio_t *aio_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                  int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  aio_t *aio;

  aio = malloc(sizeof(aio_t));
  aio->proxy = playerc_aio_create(client, index);
  aio->drivername = strdup(drivername);
  aio->datatime = 0;

  snprintf(section, sizeof(section), "aio:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "aio:%d (%s)", index, aio->drivername);
  aio->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  aio->subscribe_item = rtk_menuitem_create(aio->menu, "Subscribe", 1);

  // Set the initial menu state
  rtk_menuitem_check(aio->subscribe_item, subscribe);

  // Construct figures
  aio->fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 50);

  return aio;
}


// Destroy a aio device
void aio_destroy(aio_t *aio)
{
  if (aio->proxy->info.subscribed)
    playerc_aio_unsubscribe(aio->proxy);

  playerc_aio_destroy(aio->proxy);
  rtk_fig_destroy(aio->fig);
  free(aio->drivername);
  free(aio);

  return;
}


// Update a aio device
void aio_update(aio_t *aio)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(aio->subscribe_item))
  {
    if (!aio->proxy->info.subscribed)
      if (playerc_aio_subscribe(aio->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  else
  {
    if (aio->proxy->info.subscribed)
      if (playerc_aio_unsubscribe(aio->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(aio->subscribe_item, aio->proxy->info.subscribed);

  if (aio->proxy->info.subscribed)
  {
    // Draw in the aio scan if it has been changed.
    if (aio->proxy->info.datatime != aio->datatime)
    {
      aio_draw(aio);
      aio->datatime = aio->proxy->info.datatime;
    }
  }
  else
  {
    // Dont draw the aio.
    rtk_fig_show(aio->fig, 0);
  }
}


// Draw the aio scan
void aio_draw(aio_t *aio)
{
  int i;
  char ntext[64], text[1024];

  rtk_fig_show(aio->fig, 1);
  rtk_fig_clear(aio->fig);

  text[0] = 0;
  for (i = 0; i < aio->proxy->voltages_count; i++)
  {
    snprintf(ntext, sizeof(ntext), "%i: %0.3f\n",
             i, aio->proxy->voltages[i]);
    strcat(text, ntext);
  }

  // Draw in the aio reading
  // TODO: get text origin from somewhere
  rtk_fig_color_rgb32(aio->fig, COLOR_AIO);
  rtk_fig_text(aio->fig, +1, 0, 0, text);

  return;
}




