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
 * Desc: Main window with sensor data
 * Author: Andrew Howard
 * Date: 14 May 2002
 * CVS: $Id: mainwnd.c 3962 2007-01-31 22:09:57Z gerkey $
 ***************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "playerv.h"


// Process export options
void mainwnd_update_export(mainwnd_t *wnd);


// Create the main window
mainwnd_t *mainwnd_create(rtk_app_t *app, const char *host, int port)
{
  char title[128];
  mainwnd_t *wnd;

  wnd = malloc(sizeof(mainwnd_t));
  wnd->canvas = rtk_canvas_create(app);

  wnd->host = host;
  wnd->port = port;

  // Set up the canvas
  rtk_canvas_movemask(wnd->canvas, RTK_MOVE_PAN | RTK_MOVE_ZOOM);
  rtk_canvas_size(wnd->canvas, 320, 240);
  rtk_canvas_scale(wnd->canvas, 0.02, 0.02);
  rtk_canvas_origin(wnd->canvas, 0, 0);

  snprintf(title, sizeof(title), "PlayerViewer %s:%d", host, port);
  rtk_canvas_title(wnd->canvas, title);

  // Create file menu
  wnd->file_menu = rtk_menu_create(wnd->canvas, "File");
  wnd->stills_menu = rtk_menu_create_sub(wnd->file_menu, "Capture stills");
  wnd->movie_menu = rtk_menu_create_sub(wnd->file_menu, "Capture movie");
  wnd->exit_item = rtk_menuitem_create(wnd->file_menu, "Exit", 0);

  // Stills sub-menu
  wnd->stills_jpeg_menuitem = rtk_menuitem_create(wnd->stills_menu, "JPEG format", 1);
  wnd->stills_ppm_menuitem = rtk_menuitem_create(wnd->stills_menu, "PPM format", 1);
  wnd->stills_series = 0;
  wnd->stills_count = 0;

  // Movie sub-menu
  wnd->movie_x1_menuitem = rtk_menuitem_create(wnd->movie_menu, "Speed x1", 1);
  wnd->movie_x2_menuitem = rtk_menuitem_create(wnd->movie_menu, "Speed x2", 1);
  wnd->movie_count = 0;
  
  // Create view menu
  wnd->view_menu = rtk_menu_create(wnd->canvas, "View");
  wnd->view_item_rotate = rtk_menuitem_create(wnd->view_menu, "Rotate", 1);
  wnd->view_item_1m = rtk_menuitem_create(wnd->view_menu, "Grid 1 m", 1);
  wnd->view_item_10m = rtk_menuitem_create(wnd->view_menu, "Grid 10 m", 1);
  wnd->view_item_2f = rtk_menuitem_create(wnd->view_menu, "Grid 2 feet", 1);
  wnd->view_item_ego = rtk_menuitem_create(wnd->view_menu, "Frame egocentric", 1); 
  // Create device menu
  wnd->device_menu = rtk_menu_create(wnd->canvas, "Devices");

  // Create figure to draw the grid on
  wnd->grid_fig = rtk_fig_create(wnd->canvas, NULL, -90);
  
  // Create a figure to attach everything else to
  wnd->robot_fig = rtk_fig_create(wnd->canvas, NULL, 0);

  // Set the initial grid state (this is a bit of a hack, since
  // it duplicated the code in update()).
  rtk_menuitem_check(wnd->view_item_rotate, 0);
  rtk_menuitem_check(wnd->view_item_1m, 1);
  rtk_menuitem_check(wnd->view_item_10m, 0);
  rtk_menuitem_check(wnd->view_item_2f, 0);
  rtk_menuitem_check(wnd->view_item_ego, 1);
  
  rtk_fig_color_rgb32(wnd->grid_fig, COLOR_GRID_MINOR);
  rtk_fig_grid(wnd->grid_fig, 0, 0, 500, 500, 1);
  rtk_fig_color_rgb32(wnd->grid_fig, COLOR_GRID_MAJOR);
  rtk_fig_grid(wnd->grid_fig, 0, 0, 500, 500, 10);
      
  return wnd;
}


// Destroy the main window
void mainwnd_destroy(mainwnd_t *wnd)
{
  // Destroy the grid menu
  rtk_menuitem_destroy(wnd->view_item_rotate);
  rtk_menuitem_destroy(wnd->view_item_1m);
  rtk_menuitem_destroy(wnd->view_item_2f);
  rtk_menu_destroy(wnd->view_menu);
  
  // Destroy device menu
  rtk_menu_destroy(wnd->device_menu);

  // Destroy file menu
  rtk_menuitem_destroy(wnd->exit_item);
  rtk_menu_destroy(wnd->file_menu);

  // Destroy canvas
  rtk_fig_destroy(wnd->robot_fig);
  rtk_fig_destroy(wnd->grid_fig);
  rtk_canvas_destroy(wnd->canvas);
  
  free(wnd);
}


// Update the window.
// Returns 1 if the program should quit.
int mainwnd_update(mainwnd_t *wnd)
{
  //char filename[256];
  
  // See if we should quit
  if (rtk_canvas_isclosed(wnd->canvas))
    return 1;
  if (rtk_menuitem_isactivated(wnd->exit_item))
    return 1;

  // Handle export stuff
  mainwnd_update_export(wnd);

  // Center the robot
  if (rtk_menuitem_ischecked(wnd->view_item_ego))
    rtk_fig_origin(wnd->robot_fig, 0, 0, 0);
      
  // Rotate the display
  if (rtk_menuitem_isactivated(wnd->view_item_rotate))
  {
    if (rtk_menuitem_ischecked(wnd->view_item_rotate))
      rtk_fig_origin(wnd->robot_fig, 0, 0, M_PI / 2);
    else
      rtk_fig_origin(wnd->robot_fig, 0, 0, 0);
  }
  
  // Draw in the grid, perhaps
  if (rtk_menuitem_isactivated(wnd->view_item_1m))
  {
    rtk_fig_clear(wnd->grid_fig);
    if (rtk_menuitem_ischecked(wnd->view_item_1m))
    {
      rtk_fig_color_rgb32(wnd->grid_fig, COLOR_GRID_MINOR);
      rtk_fig_grid(wnd->grid_fig, 0, 0, 500, 500, 1);
      rtk_fig_color_rgb32(wnd->grid_fig, COLOR_GRID_MAJOR);
      rtk_fig_grid(wnd->grid_fig, 0, 0, 500, 500, 10);
      rtk_menuitem_check(wnd->view_item_10m, 0);
      rtk_menuitem_check(wnd->view_item_2f, 0);
    }
  }

  // Draw in the grid, perhaps
  if (rtk_menuitem_isactivated(wnd->view_item_10m))
  {
    rtk_fig_clear(wnd->grid_fig);
    if (rtk_menuitem_ischecked(wnd->view_item_10m))
    {
      rtk_fig_color_rgb32(wnd->grid_fig, COLOR_GRID_MINOR);
      rtk_fig_grid(wnd->grid_fig, 0, 0, 500, 500, 10);
      rtk_fig_color_rgb32(wnd->grid_fig, COLOR_GRID_MAJOR);
      rtk_fig_grid(wnd->grid_fig, 0, 0, 500, 500, 100);
      rtk_menuitem_check(wnd->view_item_1m, 0);
      rtk_menuitem_check(wnd->view_item_2f, 0);
    }
  }

  // Draw in the grid, perhaps
  if (rtk_menuitem_isactivated(wnd->view_item_2f))
  {
    rtk_fig_clear(wnd->grid_fig);
    if (rtk_menuitem_ischecked(wnd->view_item_2f))
    {
      rtk_fig_color_rgb32(wnd->grid_fig, COLOR_GRID_MINOR);
      rtk_fig_grid(wnd->grid_fig, 0, 0, 500, 500, 4 * 0.0254);
      rtk_fig_color_rgb32(wnd->grid_fig, COLOR_GRID_MAJOR);
      rtk_fig_grid(wnd->grid_fig, 0, 0, 500, 500, 2 * 12 * 0.0254);
      rtk_menuitem_check(wnd->view_item_10m, 0);
      rtk_menuitem_check(wnd->view_item_1m, 0);
    }
  }
  // Render the canvas
  rtk_canvas_render(wnd->canvas);
        
  return 0;
}


// Process export options
void mainwnd_update_export(mainwnd_t *wnd)
{
  char filename[1024];
  
  // Start/stop export (jpeg)
  if (rtk_menuitem_isactivated(wnd->stills_jpeg_menuitem))
  {
    if (rtk_menuitem_ischecked(wnd->stills_jpeg_menuitem))
    {
      wnd->stills_series++;
      rtk_menuitem_enable(wnd->stills_ppm_menuitem, 0);
    }
    else
      rtk_menuitem_enable(wnd->stills_ppm_menuitem, 1);      
  }
  if (rtk_menuitem_ischecked(wnd->stills_jpeg_menuitem))
  {
    snprintf(filename, sizeof(filename), "stage-%03d-%04d.jpg",
             wnd->stills_series, wnd->stills_count++);
    printf("saving [%s]\n", filename);
    rtk_canvas_export_image(wnd->canvas, filename, RTK_IMAGE_FORMAT_JPEG);
  }

  // Start/stop export (ppm)
  if (rtk_menuitem_isactivated(wnd->stills_ppm_menuitem))
  {
    if (rtk_menuitem_ischecked(wnd->stills_ppm_menuitem))
    {
      wnd->stills_series++;
      rtk_menuitem_enable(wnd->stills_jpeg_menuitem, 0);
    }
    else
      rtk_menuitem_enable(wnd->stills_jpeg_menuitem, 1);      
  }
  if (rtk_menuitem_ischecked(wnd->stills_ppm_menuitem))
  {
    snprintf(filename, sizeof(filename), "playerv-%03d-%04d.ppm",
             wnd->stills_series, wnd->stills_count++);
    printf("saving [%s]\n", filename);
    rtk_canvas_export_image(wnd->canvas, filename, RTK_IMAGE_FORMAT_PPM);
  }

  // Start/stop movie (x1)
  if (rtk_menuitem_isactivated(wnd->movie_x1_menuitem))
  {
    if (rtk_menuitem_ischecked(wnd->movie_x1_menuitem))
    {
      snprintf(filename, sizeof(filename), "playerv-%03d.mpg", wnd->movie_count++);
      rtk_canvas_movie_start(wnd->canvas, filename, 10, 1);
      rtk_menuitem_enable(wnd->movie_x2_menuitem, 0);
    }
    else
    {
      rtk_canvas_movie_stop(wnd->canvas);
      rtk_menuitem_enable(wnd->movie_x2_menuitem, 1);
    }
  }
  if (rtk_menuitem_ischecked(wnd->movie_x1_menuitem))
    rtk_canvas_movie_frame(wnd->canvas);

  // Start/stop movie (x2)
  if (rtk_menuitem_isactivated(wnd->movie_x2_menuitem))
  {
    if (rtk_menuitem_ischecked(wnd->movie_x2_menuitem))
    {
      snprintf(filename, sizeof(filename), "playerv-%03d.mpg", wnd->movie_count++);
      rtk_canvas_movie_start(wnd->canvas, filename, 10, 2);
      rtk_menuitem_enable(wnd->movie_x1_menuitem, 0);
    }
    else
    {
      rtk_canvas_movie_stop(wnd->canvas);
      rtk_menuitem_enable(wnd->movie_x1_menuitem, 1);
    }
  }
  if (rtk_menuitem_ischecked(wnd->movie_x2_menuitem))
    rtk_canvas_movie_frame(wnd->canvas);

  return;
}



