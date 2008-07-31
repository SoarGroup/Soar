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
 * Date: 31 March 2004
 * CVS: $Id: pv_dev_ir.c 4347 2008-02-07 19:01:37Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "playerv.h"


// Update the ir configuration
void ir_update_config(ir_t *ir);

// Draw the ir scan
void ir_draw(ir_t *ir);

// Dont draw the ir scan
void ir_nodraw(ir_t *ir);


// Create an ir device
ir_t *ir_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                      int index, const char *drivername, int subscribe)
{
  int i;
  char label[64];
  char section[64];
  ir_t *ir;
  
  ir = malloc(sizeof(ir_t));
  ir->proxy = playerc_ir_create(client, index);
  ir->drivername = strdup(drivername);
  ir->datatime = 0;
  ir->mainwnd = mainwnd;

  snprintf(section, sizeof(section), "ir:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "ir:%d (%s)", index, ir->drivername);
  ir->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  ir->subscribe_item = rtk_menuitem_create(ir->menu, "Subscribe", 1);

  // Set the initial menu state
  // Set initial device state
  rtk_menuitem_check(ir->subscribe_item, subscribe);

  ir->fig_count = 0;

  return ir;
}


void ir_allocate_figures(ir_t * ir, int fig_count)
{
  int i;
  if (fig_count <= ir->fig_count)
    return;
  ir->scan_fig = realloc(ir->scan_fig,fig_count*sizeof(ir->scan_fig[0]));
  
  // Construct figures
  for (i = ir->fig_count; i < fig_count; i++)
	  ir->scan_fig[i] = rtk_fig_create(ir->mainwnd->canvas, ir->mainwnd->robot_fig, 1);
  ir->fig_count = fig_count;
}


// Destroy an ir device
void ir_destroy(ir_t *ir)
{
  int i;
  
  if (ir->proxy->info.subscribed)
    playerc_ir_unsubscribe(ir->proxy);
  playerc_ir_destroy(ir->proxy);

  for (i = 0; i < ir->fig_count; i++)
    rtk_fig_destroy(ir->scan_fig[i]);
  free(ir->scan_fig);

  rtk_menuitem_destroy(ir->subscribe_item);
  rtk_menu_destroy(ir->menu);

  free(ir->drivername);
  free(ir);
}


// Update an ir device
void ir_update(ir_t *ir)
{
  int i;
  
  // Update the device subscription
  if (rtk_menuitem_ischecked(ir->subscribe_item))
  {
    if (!ir->proxy->info.subscribed)
    {
      if (playerc_ir_subscribe(ir->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("subscribe failed : %s", playerc_error_str());

      // Get the ir geometry
      if (playerc_ir_get_geom(ir->proxy) != 0)
        PRINT_ERR1("get_geom failed : %s", playerc_error_str());    

      ir_allocate_figures(ir, ir->proxy->poses.poses_count);
      for (i = 0; i < ir->proxy->poses.poses_count; i++)
	  {
        rtk_fig_origin(ir->scan_fig[i],
                       ir->proxy->poses.poses[i].px,
                       ir->proxy->poses.poses[i].py,
                       ir->proxy->poses.poses[i].pyaw);
	  }
    }
  }
  else
  {
    if (ir->proxy->info.subscribed)
      if (playerc_ir_unsubscribe(ir->proxy) != 0)
        PRINT_ERR1("unsubscribe failed : %s", playerc_error_str());
  }
  rtk_menuitem_check(ir->subscribe_item, ir->proxy->info.subscribed);

  if (ir->proxy->info.subscribed)
  {
    // Draw in the ir scan if it has been changed.
    if (ir->proxy->info.datatime != ir->datatime)
      ir_draw(ir);
    ir->datatime = ir->proxy->info.datatime;
  }
  else
  {
    // Dont draw the ir.
    ir_nodraw(ir);
  }
}


// Draw the ir scan
void ir_draw(ir_t *ir)
{
  int i;
  double dr, da;
  double points[3][2];

  for (i = 0; i < ir->proxy->data.ranges_count; i++)
  {
    rtk_fig_show(ir->scan_fig[i], 1);      
    rtk_fig_clear(ir->scan_fig[i]);

    // Draw in the ir itself
    rtk_fig_color_rgb32(ir->scan_fig[i], COLOR_IR);
    rtk_fig_rectangle(ir->scan_fig[i], 0, 0, 0, 0.01, 0.05, 0);

    // Draw in the range scan
    rtk_fig_color_rgb32(ir->scan_fig[i], COLOR_IR_SCAN);
    dr = ((double)ir->proxy->data.ranges[i]);
    da = 20 * M_PI / 180 / 2;
  
    points[0][0] = 0;
    points[0][1] = 0;
    points[1][0] = dr * cos(-da);
    points[1][1] = dr * sin(-da);
    points[2][0] = dr * cos(+da);
    points[2][1] = dr * sin(+da);
    rtk_fig_polygon(ir->scan_fig[i], 0, 0, 0, 3, points, 1);
  }
}


// Dont draw the ir scan
void ir_nodraw(ir_t *ir)
{
  int i;

  for (i = 0; i < ir->proxy->data.ranges_count; i++)
    rtk_fig_show(ir->scan_fig[i], 0);
}


