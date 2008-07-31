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
 * Desc: Bumper interface
 * Author: Toby Collett (Based on sonar dev by Andrew Howard)
 * Date: 13 Feb 2004
 * CVS: $Id: pv_dev_bumper.c 4390 2008-02-29 04:02:37Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Update the bumper configuration
void bumper_update_config(bumper_t *bumper);

// Draw the bumper scan
void bumper_draw(bumper_t *bumper);

// Dont draw the bumper scan
void bumper_nodraw(bumper_t *bumper);


// Create a bumper device
bumper_t *bumper_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                      int index, const char *drivername, int subscribe)
{
  int i;
  char label[64];
  char section[64];
  bumper_t *bumper;
  
  bumper = malloc(sizeof(bumper_t));
  bumper->proxy = playerc_bumper_create(client, index);
  bumper->drivername = strdup(drivername);
  bumper->datatime = 0;
  bumper->mainwnd = mainwnd;

  snprintf(section, sizeof(section), "bumper:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "bumper:%d (%s)", index, bumper->drivername);
  bumper->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  bumper->subscribe_item = rtk_menuitem_create(bumper->menu, "Subscribe", 1);

  // Set the initial menu state
  // Set initial device state
  rtk_menuitem_check(bumper->subscribe_item, subscribe);

  bumper->fig_count = 0;
  bumper->scan_fig = NULL;
  return bumper;
}

void bumper_allocate_figures(bumper_t * bumper, int fig_count)
{
  int i;
  if (fig_count <= bumper->fig_count)
    return;
  bumper->scan_fig = realloc(bumper->scan_fig,fig_count*sizeof(bumper->scan_fig[0]));
  
  // Construct figures
  for (i = bumper->fig_count; i < fig_count; i++)
    bumper->scan_fig[i] = rtk_fig_create(bumper->mainwnd->canvas, bumper->mainwnd->robot_fig, 1);
  bumper->fig_count = fig_count;
}


// Destroy a bumper device
void bumper_destroy(bumper_t *bumper)
{
  int i;
  
  if (bumper->proxy->info.subscribed)
    playerc_bumper_unsubscribe(bumper->proxy);
  playerc_bumper_destroy(bumper->proxy);

  for (i = 0; i < bumper->fig_count; i++)
    rtk_fig_destroy(bumper->scan_fig[i]);
  free(bumper->scan_fig);

  rtk_menuitem_destroy(bumper->subscribe_item);
  rtk_menu_destroy(bumper->menu);

  free(bumper->drivername);
  free(bumper);
}


// Update a bumper device
void bumper_update(bumper_t *bumper)
{
  int i;
  
  // Update the device subscription
  if (rtk_menuitem_ischecked(bumper->subscribe_item))
  {
    if (!bumper->proxy->info.subscribed)
    {
      if (playerc_bumper_subscribe(bumper->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("subscribe failed : %s", playerc_error_str());

      // Get the bumper geometry
      if (playerc_bumper_get_geom(bumper->proxy) != 0)
        PRINT_ERR1("get_geom failed : %s", playerc_error_str());    

      bumper_allocate_figures(bumper, bumper->proxy->pose_count);
      for (i = 0; i < bumper->proxy->pose_count; i++){
	  //fprintf(stderr, "bumper poses %02d: %f %f %f %f %f\n",i,bumper->proxy->poses[i][0],bumper->proxy->poses[i][1],bumper->proxy->poses[i][2],bumper->proxy->poses[i][3],bumper->proxy->poses[i][4]);
        rtk_fig_origin(bumper->scan_fig[i],
                       bumper->proxy->poses[i].pose.px, // convert mm to m
                       bumper->proxy->poses[i].pose.py,
                       bumper->proxy->poses[i].pose.pyaw); // convert deg to rad
					   
      }
    }
  }
  else
  {
    if (bumper->proxy->info.subscribed)
      if (playerc_bumper_unsubscribe(bumper->proxy) != 0)
        PRINT_ERR1("unsubscribe failed : %s", playerc_error_str());
  }
  rtk_menuitem_check(bumper->subscribe_item, bumper->proxy->info.subscribed);

  if (bumper->proxy->info.subscribed)
  {
    // Draw in the bumper scan if it has been changed.
    if (bumper->proxy->info.datatime != bumper->datatime)
      bumper_draw(bumper);
    bumper->datatime = bumper->proxy->info.datatime;
  }
  else
  {
    // Dont draw the bumper.
    bumper_nodraw(bumper);
  }
}


// Draw the bumper scan
void bumper_draw(bumper_t *bumper)
{
  int i;
  double radius, half_angle;

  for (i = 0; i< bumper->proxy->bumper_count; i++)
  {
    rtk_fig_show(bumper->scan_fig[i], 1);      
    rtk_fig_clear(bumper->scan_fig[i]);

    // Draw in the bumper, diff colour if its active
    if (bumper->proxy->bumpers[i] == 0)
      rtk_fig_color_rgb32(bumper->scan_fig[i], COLOR_BUMPER);
    else
      rtk_fig_color_rgb32(bumper->scan_fig[i], COLOR_BUMPER_ACTIVE);

    // thicker line for the bumper
    rtk_fig_linewidth(bumper->scan_fig[i],4);

    // calc the geometry of the bumper in rtk terms
    radius = bumper->proxy->poses[i].radius;

    // check for straight line
    if(radius == 0)
    {
      rtk_fig_line(bumper->scan_fig[i],
                   0.0,-bumper->proxy->poses[i].length/2.0,
                   0.0,bumper->proxy->poses[i].length/2.0);
    }
    else
    {
      half_angle = (bumper->proxy->poses[i].length)/radius/2.0 - 0.04;
      rtk_fig_ellipse_arc(bumper->scan_fig[i],-radius,0,0,radius*2,radius*2,-half_angle,half_angle);	
    }
  }
}


// Dont draw the bumper scan
void bumper_nodraw(bumper_t *bumper)
{
  int i;
  for (i = 0; i < bumper->proxy->bumper_count; i++)
    rtk_fig_show(bumper->scan_fig[i], 0);
}


