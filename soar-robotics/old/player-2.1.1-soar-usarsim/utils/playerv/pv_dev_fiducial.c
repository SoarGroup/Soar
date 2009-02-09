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
 * Desc: LBD (laser beacon detector) device interface
 * Author: Andrew Howard
 * Date: 14 May 2002
 * CVS: $Id: pv_dev_fiducial.c 4152 2007-09-17 02:18:59Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Draw the detected beacons
void fiducial_draw(fiducial_t *fiducial);


// Create a fiducial device
fiducial_t *fiducial_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                            int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  fiducial_t *fiducial;
  
  fiducial = malloc(sizeof(fiducial_t));
  memset(fiducial, 0, sizeof(fiducial_t));

  // Create a proxy
  fiducial->proxy = playerc_fiducial_create(client, index);
  fiducial->drivername = strdup(drivername);
  fiducial->datatime = 0;

  snprintf(section, sizeof(section), "fiducial:%d", index);
    
  // Construct the menu
  snprintf(label, sizeof(label), "fiducial:%d (%s)", index, fiducial->drivername);
  fiducial->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  fiducial->subscribe_item = rtk_menuitem_create(fiducial->menu, "Subscribe", 1);

  // Set the initial menu state
  rtk_menuitem_check(fiducial->subscribe_item, subscribe);
  
  // Create a figure
  fiducial->fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 1);

  return fiducial;
}


// Destroy a fiducial device
void fiducial_destroy(fiducial_t *fiducial)
{
  if (fiducial->proxy->info.subscribed)
    playerc_fiducial_unsubscribe(fiducial->proxy);
  playerc_fiducial_destroy(fiducial->proxy);

  rtk_fig_destroy(fiducial->fig);
  rtk_menuitem_destroy(fiducial->subscribe_item);
  rtk_menu_destroy(fiducial->menu);

  free(fiducial->drivername);
  free(fiducial);
}


// Update a fiducial device
void fiducial_update(fiducial_t *fiducial)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(fiducial->subscribe_item))
  {
    if (!fiducial->proxy->info.subscribed)
    {
      if (playerc_fiducial_subscribe(fiducial->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());

      // Get the geometry
      if (playerc_fiducial_get_geom(fiducial->proxy) != 0)
	PRINT_ERR1("libplayerc error: %s", playerc_error_str());
    }
  }
  else
  {
    if (fiducial->proxy->info.subscribed)
      if (playerc_fiducial_unsubscribe(fiducial->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(fiducial->subscribe_item, fiducial->proxy->info.subscribed);

  if (fiducial->proxy->info.subscribed)
  {
    // Draw in the beacon data if it has changed.
    if (fiducial->proxy->info.datatime != fiducial->datatime)
    {
      fiducial_draw(fiducial);
      fiducial->datatime = fiducial->proxy->info.datatime;
    }
  }
  else
  {
    // Dont draw the beacon data
    rtk_fig_show(fiducial->fig, 0);
  }
}


// Draw the fiducial scan
void fiducial_draw(fiducial_t *fiducial)
{
  int i;
  double ox, oy, oa;
  double wx, wy;
  char text[64];
  player_fiducial_item_t *fdata;

  rtk_fig_show(fiducial->fig, 1);      
  rtk_fig_clear(fiducial->fig);
  rtk_fig_color_rgb32(fiducial->fig, COLOR_FIDUCIAL);

  rtk_fig_origin(fiducial->fig,
                 fiducial->proxy->fiducial_geom.pose.px,
                 fiducial->proxy->fiducial_geom.pose.py,
                 fiducial->proxy->fiducial_geom.pose.pyaw);
  rtk_fig_rectangle(fiducial->fig, 0, 0, 0,
                    fiducial->proxy->fiducial_geom.size.sw,                    
                    fiducial->proxy->fiducial_geom.size.sl, 0);

  wx = fiducial->proxy->fiducial_geom.fiducial_size.sw;
  wy = fiducial->proxy->fiducial_geom.fiducial_size.sl;
  
  for (i = 0; i < fiducial->proxy->fiducials_count; i++)
  {
    fdata = fiducial->proxy->fiducials + i;

    /*
    ox = fdata->range * cos(fdata->bearing);
    oy = fdata->range * sin(fdata->bearing);
    oa = fdata->orient;
    */
    ox = fdata->pose.px;
    oy = fdata->pose.py;
    oa = fdata->pose.pyaw;
    
    rtk_fig_rectangle(fiducial->fig, ox, oy, oa, wx, wy, 0);
    rtk_fig_arrow(fiducial->fig, ox, oy, oa, 0.20, 0.10);
    snprintf(text, sizeof(text), "  %d", fdata->id);
    rtk_fig_text(fiducial->fig, ox, oy, oa, text);
  }
}




