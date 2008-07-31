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
 * CVS: $Id: pv_dev_laser.c 4334 2008-01-28 02:08:36Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "playerv.h"


// Update the laser configuration
void laser_update_config(laser_t *laser);

// Draw the laser scan
void laser_draw(laser_t *laser);


// Create a laser device
laser_t *laser_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                      int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  laser_t *laser;
  
  laser = malloc(sizeof(laser_t));

  laser->proxy = playerc_laser_create(client, index);
  laser->drivername = strdup(drivername);
  laser->datatime = 0;

  snprintf(section, sizeof(section), "laser:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "laser:%d (%s)", index, laser->drivername);
  laser->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  laser->subscribe_item = rtk_menuitem_create(laser->menu, "Subscribe", 1);
  laser->style_item = rtk_menuitem_create(laser->menu, "Filled", 1);

#if 0
  laser->res025_item = rtk_menuitem_create(laser->menu, "0.25 deg resolution", 1);
  laser->res050_item = rtk_menuitem_create(laser->menu, "0.50 deg resolution", 1);
  laser->res100_item = rtk_menuitem_create(laser->menu, "1.00 deg resolution", 1);
#endif
  laser->range_mm_item = rtk_menuitem_create(laser->menu, "mm Range Resolution",1);
  laser->range_cm_item = rtk_menuitem_create(laser->menu, "cm Range Resolution",1);
  laser->range_dm_item = rtk_menuitem_create(laser->menu, "dm Range Resolution",1);

  // Set the initial menu state
  rtk_menuitem_check(laser->subscribe_item, subscribe);
  rtk_menuitem_check(laser->style_item, 1);

  // Construct figures
  laser->scan_fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 0);
  
  return laser;
}


// Destroy a laser device
void laser_destroy(laser_t *laser)
{
  if (laser->proxy->info.subscribed)
    playerc_laser_unsubscribe(laser->proxy);
  playerc_laser_destroy(laser->proxy);

  rtk_fig_destroy(laser->scan_fig);

#if 0
  rtk_menuitem_destroy(laser->res025_item);
  rtk_menuitem_destroy(laser->res050_item);
  rtk_menuitem_destroy(laser->res100_item);
#endif
  rtk_menuitem_destroy(laser->range_mm_item);
  rtk_menuitem_destroy(laser->range_cm_item);
  rtk_menuitem_destroy(laser->range_dm_item);
  rtk_menuitem_destroy(laser->subscribe_item);
  rtk_menuitem_destroy(laser->style_item);
  rtk_menu_destroy(laser->menu);

  free(laser->drivername);
  
  free(laser);
}


// Update a laser device
void laser_update(laser_t *laser)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(laser->subscribe_item))
  {
    if (!laser->proxy->info.subscribed)
    {
      if (playerc_laser_subscribe(laser->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());

      // Get the laser geometry
      if (playerc_laser_get_geom(laser->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());

      rtk_fig_origin(laser->scan_fig,
                     laser->proxy->pose[0],
                     laser->proxy->pose[1],
                     laser->proxy->pose[2]);

    }
  }
  else
  {
    if (laser->proxy->info.subscribed)
      if (playerc_laser_unsubscribe(laser->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(laser->subscribe_item, laser->proxy->info.subscribed);

  // Making config changes here causes X to go nuts.  Don't know why - BPG
  // Update the configuration stuff
  if (laser->proxy->info.subscribed)
    laser_update_config(laser);
  
  if (laser->proxy->info.subscribed)
  {
    // Draw in the laser scan if it has been changed.
    if (laser->proxy->info.datatime != laser->datatime)
    {
      laser_draw(laser);
      laser->datatime = laser->proxy->info.datatime;
    }
  }
  else
  {
    // Dont draw the laser.
    rtk_fig_show(laser->scan_fig, 0);
  }
}


// Update the laser configuration
void laser_update_config(laser_t *laser)
{
  int update;
  double min, max, range_res, res, scanning_frequency;
  unsigned char  intensity;
  
  min = laser->proxy->scan_start;
  max = laser->proxy->scan_start + laser->proxy->scan_count*laser->proxy->scan_res;
  range_res = laser->proxy->range_res;
  res = laser->proxy->scan_res;
  scanning_frequency = laser->proxy->scanning_frequency;
  intensity = laser->proxy->intensity_on;

  update = 0;
#if 0
  if (rtk_menuitem_isactivated(laser->res025_item))
  {
    res = 25; min = -50*M_PI/180; max = +50*M_PI/180; update = 1;
  }
  if (rtk_menuitem_isactivated(laser->res050_item))
  {
    res = 50; min = -M_PI/2; max = +M_PI/2; update = 1;
  }
  if (rtk_menuitem_isactivated(laser->res100_item))
  {
    res = 100; min = -M_PI/2; max = +M_PI/2; update = 1;
  }
#endif

  if (rtk_menuitem_isactivated(laser->range_mm_item)) {
    range_res = .001;
    update = 1;
  }
  if (rtk_menuitem_isactivated(laser->range_cm_item)) {
    range_res = .010;
    update = 1;
  }
  if (rtk_menuitem_isactivated(laser->range_dm_item)) {
    range_res = .100;
    update = 1;
  }

  // Set the laser configuration.
  if (update)
  {
    if (playerc_laser_set_config(laser->proxy, min, max, res, range_res,
				 intensity, scanning_frequency) != 0)
      PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }

#if 0
  res = (int) (laser->proxy->scan_res * 180 / M_PI * 100);
  rtk_menuitem_check(laser->res025_item, (res == 25));
  rtk_menuitem_check(laser->res050_item, (res == 50));
  rtk_menuitem_check(laser->res100_item, (res == 100));
#endif

  range_res = laser->proxy->range_res;
  rtk_menuitem_check(laser->range_mm_item, (range_res < .0011));
  rtk_menuitem_check(laser->range_cm_item, (.009 < range_res && range_res < .011));
  rtk_menuitem_check(laser->range_dm_item, (range_res > .090));


  return;
}  


// Draw the laser scan
void laser_draw(laser_t *laser)
{
  int i;
  int style;
  double ax, ay, bx, by;
  double r, b, res;
  int point_count;
  double (*points)[2];
  rtk_fig_show(laser->scan_fig, 1);      
  rtk_fig_clear(laser->scan_fig);

  if (!rtk_menuitem_ischecked(laser->style_item))
  {
    rtk_fig_color_rgb32(laser->scan_fig, COLOR_LASER_OCC);
      
    // Draw in the range scan
    for (i = 0; i < laser->proxy->scan_count; i++)
    {
      bx = laser->proxy->point[i].px;
      by = laser->proxy->point[i].py;
      if (i == 0)
      {
        ax = bx;
        ay = by;
      }
      rtk_fig_line(laser->scan_fig, ax, ay, bx, by);
      ax = bx;
      ay = by;
    }
  }
  else
  {
    res = laser->proxy->scan_res / 2;
          
    // Draw in the range scan (empty space)
    points = calloc(laser->proxy->scan_count,sizeof(double)*2);
    for (i = 0; i < laser->proxy->scan_count; i++)
    {      
      r = laser->proxy->scan[i][0];
      b = laser->proxy->scan[i][1];
      points[i][0] = r * cos(b - res);
      points[i][1] = r * sin(b - res);
    }
    rtk_fig_color_rgb32(laser->scan_fig, COLOR_LASER_EMP);
    rtk_fig_polygon(laser->scan_fig, 0, 0, 0, laser->proxy->scan_count, points, 1);
    free(points);
    points = NULL;
              
    // Draw in the range scan (occupied space)
    rtk_fig_color_rgb32(laser->scan_fig, COLOR_LASER_OCC);
    for (i = 0; i < laser->proxy->scan_count; i++)
    {
      r = laser->proxy->scan[i][0];
      b = laser->proxy->scan[i][1];
      ax = r * cos(b - res);
      ay = r * sin(b - res);
      bx = r * cos(b + res);
      by = r * sin(b + res);
      rtk_fig_line(laser->scan_fig, ax, ay, bx, by);
    }
  } 

  // Draw in the intensity scan
  for (i = 0; i < laser->proxy->scan_count; i++)
  {
    if (laser->proxy->intensity[i] == 0)
      continue;
    ax = laser->proxy->point[i].px;
    ay = laser->proxy->point[i].py;
    rtk_fig_rectangle(laser->scan_fig, ax, ay, 0, 0.05, 0.05, 1);
  }

  // Draw in the laser itself
  rtk_fig_color_rgb32(laser->scan_fig, COLOR_LASER);
  rtk_fig_rectangle(laser->scan_fig, 0, 0, 0,
                    laser->proxy->size[0], laser->proxy->size[1], 0);

  return;
}




