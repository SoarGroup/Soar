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
 * Desc: PTZ device interface
 * Author: Andrew Howard
 * Date: 26 May 2002
 * CVS: $Id: pv_dev_ptz.c 4325 2008-01-09 11:45:37Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Draw the ptz 
void ptz_draw(ptz_t *ptz);

// Move the ptz
void ptz_move(ptz_t *ptz);


// Create a ptz device
ptz_t *ptz_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                  int index, const char *drivername, int subscribe)
{
  char section[64];
  char label[64];
  ptz_t *ptz;
  
  ptz = malloc(sizeof(ptz_t));
  ptz->datatime = 0;
  ptz->drivername = strdup(drivername);
  ptz->proxy = playerc_ptz_create(client, index);

  // Set initial device state
  snprintf(section, sizeof(section), "ptz:%d", index);
  if (subscribe)
  {
    if (playerc_ptz_subscribe(ptz->proxy, PLAYER_OPEN_MODE) != 0)
      PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }

  // Construct the menu
  snprintf(label, sizeof(label), "ptz:%d (%s)", index, ptz->drivername);
  ptz->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  ptz->subscribe_item = rtk_menuitem_create(ptz->menu, "Subscribe", 1);
  ptz->command_item = rtk_menuitem_create(ptz->menu, "Command", 1);
  
  // Set the initial menu state
  rtk_menuitem_check(ptz->subscribe_item, ptz->proxy->info.subscribed);

  // Construct figures
  ptz->data_fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 10);
  ptz->cmd_fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 11);
  ptz->data_fig_tilt = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 12);
  ptz->cmd_fig_tilt = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 13);
  rtk_fig_movemask(ptz->cmd_fig, RTK_MOVE_TRANS);
  rtk_fig_origin(ptz->cmd_fig, 1, 0, 0);
  rtk_fig_color_rgb32(ptz->cmd_fig, COLOR_PTZ_CMD);
  rtk_fig_ellipse(ptz->cmd_fig, 0, 0, 0, 0.2, 0.2, 0);
  
  rtk_fig_movemask(ptz->cmd_fig_tilt, RTK_MOVE_TRANS);
  rtk_fig_origin(ptz->cmd_fig_tilt, 0.8, 0, 0);
  rtk_fig_color_rgb32(ptz->cmd_fig_tilt, COLOR_PTZ_CMD_TILT);
  rtk_fig_ellipse(ptz->cmd_fig_tilt, 0, 0, 0, 0.2, 0.2, 0);
  
  
  return ptz;
}


// Destroy a ptz device
void ptz_destroy(ptz_t *ptz)
{
  // Destroy figures
  rtk_fig_destroy(ptz->cmd_fig);
  rtk_fig_destroy(ptz->data_fig);
  rtk_fig_destroy(ptz->cmd_fig_tilt);
  rtk_fig_destroy(ptz->data_fig_tilt);

  // Destroy menu items
  rtk_menuitem_destroy(ptz->command_item);
  rtk_menuitem_destroy(ptz->subscribe_item);
  rtk_menu_destroy(ptz->menu);

  // Unsubscribe/destroy the proxy
  if (ptz->proxy->info.subscribed)
    playerc_ptz_unsubscribe(ptz->proxy);
  playerc_ptz_destroy(ptz->proxy);

  free(ptz->drivername);
  free(ptz);
}


// Update a ptz device
void ptz_update(ptz_t *ptz)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(ptz->subscribe_item))
  {
    if (!ptz->proxy->info.subscribed)
      if (playerc_ptz_subscribe(ptz->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  else
  {
    if (ptz->proxy->info.subscribed)
      if (playerc_ptz_unsubscribe(ptz->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(ptz->subscribe_item, ptz->proxy->info.subscribed);

  // Draw in the ptz scan if it has been changed.
  if (ptz->proxy->info.subscribed)
  {
    if (ptz->proxy->info.datatime != ptz->datatime)
      ptz_draw(ptz);
    ptz->datatime = ptz->proxy->info.datatime;
  }
  else
  {
    rtk_fig_show(ptz->data_fig, 0);
    rtk_fig_show(ptz->data_fig_tilt, 0);

  }

  // Move the ptz
  if (ptz->proxy->info.subscribed && rtk_menuitem_ischecked(ptz->command_item))
  {
    rtk_fig_show(ptz->cmd_fig, 1);
    rtk_fig_show(ptz->cmd_fig_tilt, 1);
    ptz_move(ptz);
  }
  else
  {
    rtk_fig_show(ptz->cmd_fig, 0);
    rtk_fig_show(ptz->cmd_fig_tilt, 0);
  }
}


// Draw the ptz scan
void ptz_draw(ptz_t *ptz)
{
  double ox, oy, d;
  double ax, ay, bx, by;
  double fx, fd;

  // Camera field of view in x-direction (radians)
  fx = ptz->proxy->zoom;
  fd = 0.5 / tan(fx/2);
  
  rtk_fig_show(ptz->data_fig, 1);      
  rtk_fig_clear(ptz->data_fig);
  rtk_fig_show(ptz->data_fig_tilt, 1);      
  rtk_fig_clear(ptz->data_fig_tilt);

  rtk_fig_color_rgb32(ptz->data_fig, COLOR_PTZ_DATA);
  ox = 100 * cos(ptz->proxy->pan);
  oy = 100 * sin(ptz->proxy->pan);
  rtk_fig_line(ptz->data_fig, 0, 0, ox, oy);
  ox = 100 * cos(ptz->proxy->pan + fx / 2);
  oy = 100 * sin(ptz->proxy->pan + fx / 2);
  rtk_fig_line(ptz->data_fig, 0, 0, ox, oy);
  ox = 100 * cos(ptz->proxy->pan - fx / 2);
  oy = 100 * sin(ptz->proxy->pan - fx / 2);
  rtk_fig_line(ptz->data_fig, 0, 0, ox, oy);

  // Draw in the zoom bar (2 m in length)
  d = sqrt(fd * fd + 0.5 * 0.5);
  ax = d * cos(ptz->proxy->pan + fx / 2);
  ay = d * sin(ptz->proxy->pan + fx / 2);
  bx = d * cos(ptz->proxy->pan - fx / 2);
  by = d * sin(ptz->proxy->pan - fx / 2);
  rtk_fig_line(ptz->data_fig, ax, ay, bx, by);
  
  rtk_fig_color_rgb32(ptz->data_fig_tilt, COLOR_PTZ_DATA_TILT);
  ox = 100 * cos(ptz->proxy->tilt);
  oy = 100 * sin(ptz->proxy->tilt);
  rtk_fig_line(ptz->data_fig_tilt, 0, 0, ox, oy);
}


// Move the ptz
void ptz_move(ptz_t *ptz)
{
  double ox, oy, oa, oxt, oyt, oat;
  double pan, tilt, zoom, speed;
    
  rtk_fig_get_origin(ptz->cmd_fig, &ox, &oy, &oa);
  rtk_fig_get_origin(ptz->cmd_fig_tilt, &oxt, &oyt, &oat);

  pan = atan2(oy, ox);
  tilt = atan2(oyt,oxt);
  zoom = 2 * atan2(0.5, sqrt(ox * ox + oy * oy));
  speed = sqrt(oy*oy + ox*ox);

  if (playerc_ptz_set_ws(ptz->proxy, pan, tilt, zoom,speed,0) != 0)
    PRINT_ERR1("libplayerc error: %s", playerc_error_str());
}




