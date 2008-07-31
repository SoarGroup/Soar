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
 * Desc: Sonar (fixed range finder) interface
 * Author: Andrew Howard
 * Date: 14 May 2002
 * CVS: $Id: pv_dev_sonar.c 4390 2008-02-29 04:02:37Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "playerv.h"


// Update the sonar configuration
void sonar_update_config(sonar_t *sonar);

// Draw the sonar scan
void sonar_draw(sonar_t *sonar);

// Dont draw the sonar scan
void sonar_nodraw(sonar_t *sonar);

// Update the geometry
void sonar_update_geom(sonar_t *sonar);

// Create a sonar device
sonar_t *sonar_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                      int index, const char *drivername, int subscribe)
{
  int i;
  char label[64];
  char section[64];
  sonar_t *sonar;
  
  sonar = malloc(sizeof(sonar_t));
  sonar->proxy = playerc_sonar_create(client, index);
  sonar->drivername = strdup(drivername);
  sonar->datatime = 0;
  sonar->mainwnd = mainwnd;

  snprintf(section, sizeof(section), "sonar:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "sonar:%d (%s)", index, sonar->drivername);
  sonar->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  sonar->subscribe_item = rtk_menuitem_create(sonar->menu, "Subscribe", 1);

  // Set the initial menu state
  // Set initial device state
  rtk_menuitem_check(sonar->subscribe_item, subscribe);

  sonar->fig_count = 0;
  sonar->scan_fig = NULL;

  return sonar;
}

void sonar_allocate_figures(sonar_t * sonar, int fig_count)
{
  int i;
  if (fig_count <= sonar->fig_count)
    return;
  sonar->scan_fig = realloc(sonar->scan_fig,fig_count*sizeof(sonar->scan_fig[0]));
  
  // Construct figures
  for (i = sonar->fig_count; i < fig_count; i++)
	  sonar->scan_fig[i] = rtk_fig_create(sonar->mainwnd->canvas, sonar->mainwnd->robot_fig, 1);
  sonar->fig_count = fig_count;
}


// Destroy a sonar device
void sonar_destroy(sonar_t *sonar)
{
  int i;
  
  if (sonar->proxy->info.subscribed)
    playerc_sonar_unsubscribe(sonar->proxy);
  playerc_sonar_destroy(sonar->proxy);

  for (i = 0; i < sonar->fig_count; i++)
    rtk_fig_destroy(sonar->scan_fig[i]);

  rtk_menuitem_destroy(sonar->subscribe_item);
  rtk_menu_destroy(sonar->menu);

  free(sonar->drivername);
  free(sonar);
}


// Update a sonar device
void sonar_update(sonar_t *sonar)
{
  int i;
  
  // Update the device subscription
  if (rtk_menuitem_ischecked(sonar->subscribe_item))
  {
    if (!sonar->proxy->info.subscribed)
    {
      if (playerc_sonar_subscribe(sonar->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("subscribe failed : %s", playerc_error_str());

      // Get the sonar geometry
      if (playerc_sonar_get_geom(sonar->proxy) != 0)
        PRINT_ERR1("get_geom failed : %s", playerc_error_str());    

      sonar_update_geom(sonar);
    }
  }
  else
  {
    if (sonar->proxy->info.subscribed)
      if (playerc_sonar_unsubscribe(sonar->proxy) != 0)
        PRINT_ERR1("unsubscribe failed : %s", playerc_error_str());
  }
  rtk_menuitem_check(sonar->subscribe_item, sonar->proxy->info.subscribed);

  if (sonar->proxy->info.subscribed)
  {
    if (sonar->proxy->info.freshgeom)
    {
    	sonar->proxy->info.freshgeom = 0;
	sonar_update_geom(sonar);
    }
  
    // Draw in the sonar scan if it has been changed.
    if (sonar->proxy->info.datatime != sonar->datatime)
      sonar_draw(sonar);
    sonar->datatime = sonar->proxy->info.datatime;
  }
  else
  {
    // Dont draw the sonar.
    sonar_nodraw(sonar);
  }
}

// update sonar geometry
void sonar_update_geom(sonar_t *sonar)
{
  int i;
  sonar_allocate_figures(sonar, sonar->proxy->pose_count);
  for (i = 0; i < sonar->proxy->pose_count; i++)
    rtk_fig_origin(sonar->scan_fig[i],
      sonar->proxy->poses[i].px,
      sonar->proxy->poses[i].py,
      sonar->proxy->poses[i].pyaw
    );
}


// Draw the sonar scan
void sonar_draw(sonar_t *sonar)
{
  int i;
  double dr, da;
  double points[3][2];

  for (i = 0; i < sonar->proxy->scan_count; i++)
  {
    rtk_fig_show(sonar->scan_fig[i], 1);      
    rtk_fig_clear(sonar->scan_fig[i]);

    // Draw in the sonar itself
    rtk_fig_color_rgb32(sonar->scan_fig[i], COLOR_SONAR);
    rtk_fig_rectangle(sonar->scan_fig[i], 0, 0, 0, 0.01, 0.05, 0);

    // Draw in the range scan
    rtk_fig_color_rgb32(sonar->scan_fig[i], COLOR_SONAR_SCAN);
    dr = sonar->proxy->scan[i];
    da = 20 * M_PI / 180 / 2;
  
    //rtk_fig_line(sonar->scan_fig[i], 0, 0, dr, 0);
    //rtk_fig_line(sonar->scan_fig[i], dr, -dr * da/2, dr, +dr * da/2);
    points[0][0] = 0;
    points[0][1] = 0;
    points[1][0] = dr * cos(-da);
    points[1][1] = dr * sin(-da);
    points[2][0] = dr * cos(+da);
    points[2][1] = dr * sin(+da);
    rtk_fig_polygon(sonar->scan_fig[i], 0, 0, 0, 3, points, 1);
  }
}


// Dont draw the sonar scan
void sonar_nodraw(sonar_t *sonar)
{
  int i;

  for (i = 0; i < sonar->proxy->scan_count; i++)
    rtk_fig_show(sonar->scan_fig[i], 0);
}


