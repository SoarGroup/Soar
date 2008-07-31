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
 * CVS: $Id: pv_dev_actarray.c 4325 2008-01-09 11:45:37Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"

void actarray_draw(actarray_t *actarray);
void actarray_move(actarray_t *actarray);
void actarray_allocate(actarray_t *actarray, int size);
void actarray_deallocate(actarray_t *actarray);

#define ARRAY_SPACING 1
#define ARRAY_X_OFFSET 2


// Create a actarray device
actarray_t *actarray_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                  int index, const char *drivername, int subscribe)
{
  char section[64];
  char label[64];
  actarray_t *actarray;
  
  actarray = malloc(sizeof(actarray_t));
  actarray->datatime = 0;
  actarray->drivername = strdup(drivername);
  actarray->proxy = playerc_actarray_create(client, index);

  // Set initial device state
  snprintf(section, sizeof(section), "actarray:%d", index);
  if (subscribe)
  {
    if (playerc_actarray_subscribe(actarray->proxy, PLAYER_OPEN_MODE) != 0)
      PRINT_ERR1("libplayerc error: %s", playerc_error_str());
    playerc_actarray_get_geom(actarray->proxy);
  }

  // Construct the menu
  snprintf(label, sizeof(label), "actarray:%d (%s)", index, actarray->drivername);
  actarray->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  actarray->subscribe_item = rtk_menuitem_create(actarray->menu, "Subscribe", 1);
  actarray->command_item = rtk_menuitem_create(actarray->menu, "Command", 1);
  
  // Set the initial menu state
  rtk_menuitem_check(actarray->subscribe_item, actarray->proxy->info.subscribed);

  // Construct figures
  actarray->actuator_fig = NULL;
  actarray->actuator_fig_cmd = NULL;
  actarray->lastvalue = NULL;
  actarray->mainwnd = mainwnd;
  
  return actarray;
}


// Destroy a actarray device
void actarray_destroy(actarray_t *actarray)
{
  // Destroy figures
  actarray_allocate(actarray,0);
  free(actarray->actuator_fig);
  free(actarray->actuator_fig_cmd);

  // Destroy menu items
  rtk_menuitem_destroy(actarray->command_item);
  rtk_menuitem_destroy(actarray->subscribe_item);
  rtk_menu_destroy(actarray->menu);

  // Unsubscribe/destroy the proxy
  if (actarray->proxy->info.subscribed)
    playerc_actarray_unsubscribe(actarray->proxy);
  playerc_actarray_destroy(actarray->proxy);

  free(actarray->drivername);
  free(actarray);
}

void actarray_allocate(actarray_t *actarray, int size)
{
  int ii;
  if (size == actarray->fig_count)
    return;
  if (size < actarray->fig_count)
  {
    for (ii=size; ii < actarray->fig_count; ++ii)
    {
      rtk_fig_destroy(actarray->actuator_fig[ii]);
      rtk_fig_destroy(actarray->actuator_fig_cmd[ii]);
    }
  }
  actarray->actuator_fig = realloc(actarray->actuator_fig, size * sizeof(rtk_fig_t*));
  actarray->actuator_fig_cmd = realloc(actarray->actuator_fig_cmd, size * sizeof(rtk_fig_t*));
  actarray->lastvalue = realloc(actarray->lastvalue, size*sizeof(double));
  if (size > actarray->fig_count)
  {
    for (ii=actarray->fig_count; ii < size; ++ii)
    {
      actarray->lastvalue[ii] = 1e10;
      actarray->actuator_fig[ii] = rtk_fig_create(actarray->mainwnd->canvas, actarray->mainwnd->robot_fig, 10);
      actarray->actuator_fig_cmd[ii] = rtk_fig_create(actarray->mainwnd->canvas, actarray->mainwnd->robot_fig, 11);
      rtk_fig_movemask(actarray->actuator_fig_cmd[ii], RTK_MOVE_TRANS);
      rtk_fig_origin(actarray->actuator_fig_cmd[ii], ARRAY_SPACING*ii+ARRAY_X_OFFSET, 0, 0);
      rtk_fig_color_rgb32(actarray->actuator_fig_cmd[ii], COLOR_ACTARRAY_CMD);
      rtk_fig_ellipse(actarray->actuator_fig_cmd[ii], 0, 0, 0, 0.2, 0.2, 0);

    }
  }
  actarray->fig_count = size;
}


// Update a actarray device
void actarray_update(actarray_t *actarray)
{
  int ii;
  // Update the device subscription
  if (rtk_menuitem_ischecked(actarray->subscribe_item))
  {
    if (!actarray->proxy->info.subscribed)
    {
      if (playerc_actarray_subscribe(actarray->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
      playerc_actarray_get_geom(actarray->proxy);
    }
  }
  else
  {
    if (actarray->proxy->info.subscribed)
      if (playerc_actarray_unsubscribe(actarray->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(actarray->subscribe_item, actarray->proxy->info.subscribed);

  // Draw in the actarray scan if it has been changed.
  if (actarray->proxy->info.subscribed)
  {
    if (actarray->proxy->info.datatime != actarray->datatime)
      actarray_draw(actarray);
    actarray->datatime = actarray->proxy->info.datatime;
  }
  else
  {
    for (ii = 0; ii < actarray->fig_count; ++ii)
    {
      rtk_fig_show(actarray->actuator_fig[ii], 0);
    }
  }

  // Move the actarray
  if (actarray->proxy->info.subscribed && rtk_menuitem_ischecked(actarray->command_item))
  {
    for (ii = 0; ii < actarray->fig_count; ++ii)
    {
      rtk_fig_show(actarray->actuator_fig_cmd[ii], 1);
    }
    actarray_move(actarray);
  }
  else
  {
    for (ii = 0; ii < actarray->fig_count; ++ii)
    {
      rtk_fig_show(actarray->actuator_fig_cmd[ii], 0);
    }
  }
}

// Draw the actarray scan
void actarray_draw(actarray_t *actarray)
{
  double value;
  double min, max;
  double ax, ay, bx, by;
  double fx, fd;

  int ii;

  actarray_allocate(actarray, actarray->proxy->actuators_count);

  for(ii = 0; ii < actarray->proxy->actuators_count; ++ii)
  {
    value = actarray->proxy->actuators_data[ii].position;
    min = -1;
    max = 1;
    if (actarray->proxy->actuators_geom && actarray->proxy->actuators_geom_count == actarray->proxy->actuators_count)
    {
      min = actarray->proxy->actuators_geom[ii].min;
      max = actarray->proxy->actuators_geom[ii].max;
    }
    // now limit and scale the value to the actuator bar
    if (value > max) value = max;
    if (value < min) value = min;
    value = 2*(value-min)/(max-min) -1;
    rtk_fig_t * fig = actarray->actuator_fig[ii];
    rtk_fig_show(fig, 1);
    rtk_fig_clear(fig);
    rtk_fig_origin(actarray->actuator_fig[ii], ARRAY_SPACING*ii +ARRAY_X_OFFSET, 0, 0);
    rtk_fig_color_rgb32(fig, COLOR_ACTARRAY_DATA);
    rtk_fig_line(fig, 0, -1, 0, 1);
    rtk_fig_ellipse(actarray->actuator_fig[ii], 0, value, 0, 0.2, 0.2, 1);
  }

}


// Move the actarray
void actarray_move(actarray_t *actarray)
{
  double ox, oy, oa, min, max;
  double value;

  int ii;
  for(ii = 0; ii < actarray->fig_count; ++ii)
  {
    rtk_fig_get_origin(actarray->actuator_fig_cmd[ii], &ox, &oy, &oa);
    value = oy;

    min = -1;
    max = 1;
    if (actarray->proxy->actuators_geom && actarray->proxy->actuators_geom_count == actarray->proxy->actuators_count)
    {
      min = actarray->proxy->actuators_geom[ii].min;
      max = actarray->proxy->actuators_geom[ii].max;
    }
    // now limit and scale the value to the actuator bar
    value = ((oy+1)/2)*(max-min)+min;
    if (value > max) value = max;
    if (value < min) value = min;

    if (actarray->lastvalue[ii] != value)
    {
      if (playerc_actarray_position_cmd(actarray->proxy, ii, value) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
      actarray->lastvalue[ii] = value;
    }
  }
}




