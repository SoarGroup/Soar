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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "playerv.h"

// Draw the ranger scan
void ranger_draw(ranger_t *ranger);

// Create a ranger device
ranger_t* ranger_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                        int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  ranger_t *ranger;

  ranger = malloc(sizeof(ranger_t));

  ranger->proxy = playerc_ranger_create(client, index);
  ranger->drivername = strdup(drivername);
  ranger->datatime = 0;

  snprintf(section, sizeof(section), "ranger:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "ranger:%d (%s)", index, ranger->drivername);
  ranger->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  ranger->subscribe_item = rtk_menuitem_create(ranger->menu, "Subscribe", 1);
  ranger->style_item = rtk_menuitem_create(ranger->menu, "Filled", 1);
  ranger->intns_item = rtk_menuitem_create(ranger->menu, "Draw intensity data", 1);
  ranger->device_item = rtk_menuitem_create(ranger->menu, "Singular", 1);

  // Set the initial menu state
  rtk_menuitem_check(ranger->subscribe_item, subscribe);
  rtk_menuitem_check(ranger->style_item, 1);
  rtk_menuitem_check(ranger->intns_item, 1);

  ranger->mainwnd = mainwnd;
  ranger->scan_fig = NULL;

  return ranger;
}


void ranger_delete_figures(ranger_t *ranger)
{
  int ii;

  if (ranger->scan_fig != NULL)
  {
    for (ii = 0; ii < ranger->proxy->sensor_count; ii++)
      rtk_fig_destroy(ranger->scan_fig[ii]);
    free(ranger->scan_fig);
    ranger->scan_fig = NULL;
  }
}


// Destroy a ranger device
void ranger_destroy(ranger_t *ranger)
{
  ranger_delete_figures(ranger);

  if (ranger->proxy->info.subscribed)
    playerc_ranger_unsubscribe(ranger->proxy);
  playerc_ranger_destroy(ranger->proxy);

  rtk_menuitem_destroy(ranger->subscribe_item);
  rtk_menuitem_destroy(ranger->style_item);
  rtk_menuitem_destroy(ranger->intns_item);
  rtk_menuitem_destroy(ranger->device_item);
  rtk_menu_destroy(ranger->menu);

  free(ranger->drivername);

  free(ranger);
}


// Update a ranger device
void ranger_update(ranger_t *ranger)
{
  int ii;

  // Update the device subscription
  if (rtk_menuitem_ischecked(ranger->subscribe_item))
  {
    if (!ranger->proxy->info.subscribed)
    {
      if (playerc_ranger_subscribe(ranger->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());

      // Get the ranger geometry
      if (playerc_ranger_get_geom(ranger->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());

      // Request the device config for min angle and resolution
      if (playerc_ranger_get_config(ranger->proxy, NULL, NULL, NULL, NULL, NULL, NULL) != 0)
      {
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
        ranger->start_angle = 0.0f;
        ranger->resolution = 0.0f;
      }
      else
      {
        ranger->start_angle = ranger->proxy->min_angle;
        ranger->resolution = ranger->proxy->resolution;
      }

      // Delete any current figures
      ranger_delete_figures(ranger);
      // Create the figures
      if ((ranger->scan_fig = malloc(ranger->proxy->sensor_count * sizeof(rtk_fig_t*))) == NULL )
      {
        PRINT_ERR1("Failed to allocate memory for %d figures to display ranger", ranger->proxy->sensor_count);
        return;
      }
      for (ii = 0; ii < ranger->proxy->sensor_count; ii++)
      {
        ranger->scan_fig[ii] = rtk_fig_create(ranger->mainwnd->canvas, ranger->mainwnd->robot_fig, 1);
        rtk_fig_origin(ranger->scan_fig[ii],
                       ranger->proxy->sensor_poses[ii].px,
                       ranger->proxy->sensor_poses[ii].py,
                       ranger->proxy->sensor_poses[ii].pyaw);
      }
    }
  }
  else
  {
    if (ranger->proxy->info.subscribed)
      if (playerc_ranger_unsubscribe(ranger->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
    ranger_delete_figures(ranger);
  }
  rtk_menuitem_check(ranger->subscribe_item, ranger->proxy->info.subscribed);

  if (ranger->proxy->info.subscribed)
  {
    // Draw in the ranger scan if it has been changed
    if (ranger->proxy->info.datatime != ranger->datatime)
    {
      ranger_draw(ranger);
      ranger->datatime = ranger->proxy->info.datatime;
    }
  }
  else
  {
    // Dont draw the scan
    // This causes a segfault, because the ranger figures were already
    // deleted above.  I don't know whether commenting it out is the right
    // thing to do - BPG.
    /*
    for (ii = 0; ii < ranger->proxy->sensor_count; ii++)
      rtk_fig_show(ranger->scan_fig[ii], 0);
      */
  }
}

// Converts a range reading into a point in the CS of the ranger device
// based on the pose of the sensor the reading is likely to belong to
void range_to_point(ranger_t *ranger, int index, int sensor_num, double angle, double *point)
{
  // Point position = point from range -> rotate by range angle// -> rotate by sensor yaw -> translate by sensor position
  point[0] = ranger->proxy->ranges[index] * cos(angle);// + ranger->proxy->sensor_poses[sensor_num].pyaw) + ranger->proxy->sensor_poses[sensor_num].px;
  point[1] = ranger->proxy->ranges[index] * sin(angle);// + ranger->proxy->sensor_poses[sensor_num].pyaw) + ranger->proxy->sensor_poses[sensor_num].py;
}

// Draw the ranger scan
void ranger_draw(ranger_t *ranger)
{
  int ii = 0, jj = 0;
  int point_count;
  double point1[2], point2[2]; 
  double (*points)[2];
  double current_angle = 0.0f, temp = 0.0f;
  unsigned int ranges_per_sensor = 0;

  // Drawing type depends on the selected sensor type
  // Singular sensors (e.g. sonar sensors):
  //   Draw a cone for the first range scan of each sensor
  // Non-singular sensors (e.g. laser scanner):
  //   Draw the edge of the scan and empty space

  // Calculate the number of ranges per sensor
  if (ranger->proxy->sensor_count == 0)
    ranges_per_sensor = ranger->proxy->ranges_count;
  else
    ranges_per_sensor = ranger->proxy->ranges_count / ranger->proxy->sensor_count;

  if (rtk_menuitem_ischecked(ranger->device_item))
  {
    // Draw sonar-like
    points = calloc(3, sizeof(double)*2);
    temp = 20.0f * M_PI / 180.0f / 2.0f;
    for (ii = 0; ii < ranger->proxy->sensor_count; ii++)
    {
      rtk_fig_show(ranger->scan_fig[ii], 1);
      rtk_fig_clear(ranger->scan_fig[ii]);
      rtk_fig_color_rgb32(ranger->scan_fig[ii], COLOR_SONAR_SCAN);

      // Draw a cone for the first range for each sensor
      // Assume the range is straight ahead (ignore min_angle and resolution properties)
      points[0][0] = 0.0f;
      points[0][1] = 0.0f;
      points[1][0] = ranger->proxy->ranges[ii * ranges_per_sensor] * cos(-temp);
      points[1][1] = ranger->proxy->ranges[ii * ranges_per_sensor] * sin(-temp);
      points[2][0] = ranger->proxy->ranges[ii * ranges_per_sensor] * cos(temp);
      points[2][1] = ranger->proxy->ranges[ii * ranges_per_sensor] * sin(temp);
      rtk_fig_polygon(ranger->scan_fig[ii], 0, 0, 0, 3, points, 1);

      // Draw the sensor itself
      rtk_fig_color_rgb32(ranger->scan_fig[ii], COLOR_LASER);
      rtk_fig_rectangle(ranger->scan_fig[ii], 0, 0, 0, ranger->proxy->sensor_sizes[ii].sw, ranger->proxy->sensor_sizes[ii].sl, 0);
    }
    free(points);
    points=NULL;
  }
  else
  {
    // Draw laser-like
    if (rtk_menuitem_ischecked(ranger->style_item))
    {
      // Draw each sensor in turn
      points = calloc(ranger->proxy->sensor_count, sizeof(double)*2);
      for (ii = 0; ii < ranger->proxy->sensor_count; ii++)
      {
        rtk_fig_show(ranger->scan_fig[ii], 1);
        rtk_fig_clear(ranger->scan_fig[ii]);

        // Draw empty space
        points[0][0] = ranger->proxy->sensor_poses[ii].px;
        points[0][1] = ranger->proxy->sensor_poses[ii].py;
        point_count = 1;
        current_angle = ranger->start_angle;
        // Loop over the ranges
        for (jj = ii * ranges_per_sensor; jj < (ii + 1) * ranges_per_sensor; jj++)
        {
          range_to_point(ranger, jj, ii, current_angle, points[point_count]);
          // Move round to the angle of the next range
          current_angle += ranger->resolution;
          point_count++;
        }
        rtk_fig_color_rgb32(ranger->scan_fig[ii], COLOR_LASER_EMP);
        rtk_fig_polygon(ranger->scan_fig[ii], 0, 0, 0, point_count, points, 1);

        // Draw occupied space
        rtk_fig_color_rgb32(ranger->scan_fig[ii], COLOR_LASER_OCC);
        current_angle = ranger->start_angle;
        for (jj = ii * ranges_per_sensor; jj < (ii + 1) * ranges_per_sensor; jj++)
        {
          range_to_point(ranger, jj, ii, current_angle - ranger->resolution, point1);
          range_to_point(ranger, jj, ii, current_angle + ranger->resolution, point2);
          rtk_fig_line(ranger->scan_fig[ii], point1[0], point1[1], point2[0], point2[1]);
          current_angle += ranger->resolution;
        }
      }
      free(points);
      points = NULL;
    }
    else
    {
      // Draw a range scan for each individual sensor in the device
      for (ii = 0; ii < ranger->proxy->sensor_count; ii++)
      {
        rtk_fig_show(ranger->scan_fig[ii], 1);
        rtk_fig_clear(ranger->scan_fig[ii]);

        rtk_fig_color_rgb32(ranger->scan_fig[ii], COLOR_LASER_OCC);
        current_angle = ranger->start_angle;
        // Get the first point
        range_to_point(ranger, ii * ranges_per_sensor, ii, ranger->start_angle, point1);
        // Loop over the rest of the ranges
        for (jj = ii * ranges_per_sensor + 1; jj < (ii + 1) * ranges_per_sensor; jj++)
        {
          range_to_point(ranger, jj, ii, current_angle, point2);
          // Draw a line from point 1 (previous point) to point 2 (current point)
          rtk_fig_line(ranger->scan_fig[ii], point1[0], point1[1], point2[0], point2[1]);
          point1[0] = point2[0];
          point1[1] = point2[1];
          // Move round to the angle of the next range
          current_angle += ranger->resolution;
        }
      }
    }

    // For each sensor...
    for (ii = 0; ii < ranger->proxy->sensor_count; ii++)
    {
      if (rtk_menuitem_ischecked(ranger->intns_item))
      {
        // Draw an intensity scan
        if (ranger->proxy->intensities_count > 0)
        {
          current_angle = ranger->start_angle;
          for (jj = ii * ranges_per_sensor; jj < (ii + 1) * ranges_per_sensor; jj++)
          {
            if (ranger->proxy->intensities[0] != 0)
            {
              range_to_point(ranger, jj, ii, current_angle, point1);
              rtk_fig_rectangle(ranger->scan_fig[ii], point1[0], point1[1], 0, 0.05, 0.05, 1);
            }
            current_angle += ranger->resolution;
          }
        }
      }

      // Draw the sensor itself
      rtk_fig_color_rgb32(ranger->scan_fig[ii], COLOR_LASER);
      rtk_fig_rectangle(ranger->scan_fig[ii], 0, 0, 0, ranger->proxy->sensor_sizes[ii].sw, ranger->proxy->sensor_sizes[ii].sl, 0);
    }
  }
}
