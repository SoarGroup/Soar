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
 * Desc: Position device interface
 * Author: Andrew Howard
 * Date: 14 May 2002
 * CVS: $Id: pv_dev_position2d.c 4152 2007-09-17 02:18:59Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Draw the position2d scan
void position2d_draw(position2d_t *self);

// Dont draw the position2d data
void position2d_nodraw(position2d_t *self);

// Servo the robot (position2d control)
void position2d_servo_pos(position2d_t *self);

// Servo the robot (car control)
void position2d_servo_car(position2d_t *self);

// Servo the robot (velocity control)
void position2d_servo_vel(position2d_t *self);


// Create a position2d device
position2d_t *position2d_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                            int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  position2d_t *self;
  
  self = malloc(sizeof(position2d_t));

  self->mainwnd = mainwnd;
  self->proxy = playerc_position2d_create(client, index);
  self->drivername = strdup(drivername);
  self->datatime = 0;
  
  snprintf(section, sizeof(section), "position2d:%d", index);
  
  // Construct the menu
  snprintf(label, sizeof(label), "position2d:%d (%s)", index, self->drivername);
  self->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  self->subscribe_item = rtk_menuitem_create(self->menu, "Subscribe", 1);
  self->command_item = rtk_menuitem_create(self->menu, "Command", 1);
  self->pose_mode_item = rtk_menuitem_create(self->menu, "Position mode", 1);
  self->car_mode_item = rtk_menuitem_create(self->menu, "Car mode", 1); 
  self->enable_item = rtk_menuitem_create(self->menu, "Enable", 0);
  self->disable_item = rtk_menuitem_create(self->menu, "Disable", 0);
  self->stats_item = rtk_menuitem_create(self->menu, "Show values", 1);

  // We can use this device to give us a coordinate system
  snprintf(label, sizeof(label), "Frame position2d:%d (%s)", index, self->drivername);
  self->frame_item = rtk_menuitem_create(mainwnd->view_menu, label, 1);

  // Set the initial menu state
  rtk_menuitem_check(self->subscribe_item, subscribe);
  rtk_menuitem_check(self->frame_item, 0);  
  
  // Create a figure representing the robot
  self->robot_fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 10);

  // Create a figure representing the robot's control speed.
  self->control_fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 11);
  rtk_fig_show(self->control_fig, 0);
  rtk_fig_color_rgb32(self->control_fig, COLOR_POSITION_CONTROL);
  rtk_fig_line(self->control_fig, -0.20, 0, +0.20, 0);
  rtk_fig_line(self->control_fig, 0, -0.20, 0, +0.20);
  rtk_fig_ellipse(self->control_fig, 0, 0, 0, 0.20, 0.20, 0);
  rtk_fig_movemask(self->control_fig, RTK_MOVE_TRANS);
  self->path_fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 2);
  
  // figure for speed & stall state readout

  self->stats_fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig , 2);
  rtk_fig_movemask(self->stats_fig, RTK_MOVE_TRANS);
  rtk_fig_origin(self->stats_fig, -3.0,2.0, 0);

  self->goal_px = self->goal_py = self->goal_pa = 0.0;
  
  return self;
}


// Destroy a position2d device
void position2d_destroy(position2d_t *self)
{
  if (self->proxy->info.subscribed)
    playerc_position2d_unsubscribe(self->proxy);
  playerc_position2d_destroy(self->proxy);

  rtk_fig_destroy(self->path_fig);
  rtk_fig_destroy(self->control_fig);
  rtk_fig_destroy(self->robot_fig);
  rtk_fig_destroy(self->stats_fig);

  rtk_menuitem_destroy(self->subscribe_item);
  rtk_menu_destroy(self->menu);

  free(self->drivername);
  free(self);

  return;
}


// Update a position2d device
void position2d_update(position2d_t *self)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(self->subscribe_item))
  {
    if (!self->proxy->info.subscribed)
    {
      if (playerc_position2d_subscribe(self->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());

      //puts( "getting position2d geom" );

      // Get the robot geometry
      if (playerc_position2d_get_geom(self->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
      
      //puts( "done" );

      rtk_fig_color_rgb32(self->robot_fig, COLOR_POSITION_ROBOT);
      rtk_fig_rectangle(self->robot_fig, self->proxy->pose[0],
                        self->proxy->pose[1], self->proxy->pose[2],
                        self->proxy->size[0], self->proxy->size[1], 0);
    }
  }
  else
  {
    if (self->proxy->info.subscribed)
      if (playerc_position2d_unsubscribe(self->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(self->subscribe_item, self->proxy->info.subscribed);

  // HACK
  // Use this device as our global cs
  if (rtk_menuitem_ischecked(self->frame_item) == 1)
  {
    rtk_fig_origin(self->mainwnd->robot_fig, self->proxy->px, self->proxy->py, self->proxy->pa);
  }
  
  // Check enable flag
  if (rtk_menuitem_isactivated(self->enable_item))
  {
    if (self->proxy->info.subscribed)
      if (playerc_position2d_enable(self->proxy, 1) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  if (rtk_menuitem_isactivated(self->disable_item))
  {
    if (self->proxy->info.subscribed)
      if (playerc_position2d_enable(self->proxy, 0) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }

  // Reset control figure when using position2d mode
  if (rtk_menuitem_isactivated(self->pose_mode_item))
  {
    self->goal_px = self->proxy->px;
    self->goal_py = self->proxy->py;
    self->goal_pa = self->proxy->pa;
    rtk_fig_origin(self->control_fig, 0, 0, 0);
  }

  // Servo to the goal
  if (rtk_menuitem_ischecked(self->pose_mode_item))
    position2d_servo_pos(self);
  else if (rtk_menuitem_ischecked(self->car_mode_item)) {
    position2d_servo_car(self);
  }
  else
    position2d_servo_vel(self);
  
  if (self->proxy->info.subscribed)
  {
    // Draw in the position2d scan if it has been changed.
    if (self->proxy->info.datatime != self->datatime)
    {
      position2d_draw(self);
      self->datatime = self->proxy->info.datatime;
    }
  }
  else
  {
    // Dont draw the position2d.
    position2d_nodraw(self);
  }
}


static char text[256];

// Draw the position2d data
void position2d_draw(position2d_t *self)
{
  rtk_fig_show(self->robot_fig, 1);


  rtk_fig_clear(self->stats_fig);
  
  // optionally show pose, velocity & stall values
  if (rtk_menuitem_ischecked(self->stats_item))
    {
      snprintf(text, sizeof(text), 
	       "pose\n x:%.3f\n y:%.3f\n a:%.3f\nvelocity\n x:%.3f\n y:%.3f\n a:%.3f\n %s",
	       self->proxy->px,
	       self->proxy->py,
	       self->proxy->pa,
	       self->proxy->vx,
	       self->proxy->vy,
	       self->proxy->va,
	       self->proxy->stall ? "STALL" : "" );
      
      rtk_fig_text(self->stats_fig, 0,0,0, text);
    }
  
  // REMOVE
  //rtk_fig_show(self->odo_fig, 1);      
  //rtk_fig_clear(self->odo_fig);
  //rtk_fig_color_rgb32(self->odo_fig, COLOR_POSITION_ODO);
  // snprintf(text, sizeof(text), "[%+07.2f, %+07.2f, %+04.0f]",
  //         self->proxy->px, self->proxy->py, self->proxy->pa * 180/M_PI);
  //rtk_fig_text(self->odo_fig, 0, 0, 0, text);
}


// Dont draw the position2d data
void position2d_nodraw(position2d_t *self)
{
  rtk_fig_show(self->robot_fig, 0);
  rtk_fig_clear(self->stats_fig);
  return;
}


// Servo the robot (position2d control)
void position2d_servo_pos(position2d_t *self)
{
  double rx, ry, ra;
  //double gx, gy, ga;
  
  // Only servo if we are subscribed and have enabled commands.
  if (self->proxy->info.subscribed &&
      rtk_menuitem_ischecked(self->command_item))    
  {
    rtk_fig_show(self->control_fig, 1);
    rtk_fig_show(self->path_fig, 1);
  }
  else
  {
    rtk_fig_show(self->control_fig, 0);
    rtk_fig_show(self->path_fig, 0);
    return;
  }
  
  if (rtk_fig_mouse_selected(self->control_fig))
  {
    // Get goal pose in robot cs
    rtk_fig_get_origin(self->control_fig, &rx, &ry, &ra);

    // Compute goal point in position2d cs
    self->goal_px = self->proxy->px + rx * cos(self->proxy->pa) - ry * sin(self->proxy->pa);
    self->goal_py = self->proxy->py + rx * sin(self->proxy->pa) + ry * cos(self->proxy->pa);
    self->goal_pa = self->proxy->pa + ra;

    printf("goal %.3f %.3f %.0f\n", self->goal_px, self->goal_py, self->goal_pa * 180 / M_PI);

    // Set the new goal pose
    playerc_position2d_set_cmd_pose(self->proxy, self->goal_px, self->goal_py, self->goal_pa, 1);
  }
  else
  { 
    // Get goal pose in robot cs
    rtk_fig_get_origin(self->control_fig, &rx, &ry, &ra);
  }

  // Compute goal point in robot cs
  rx = (self->goal_px - self->proxy->px) * cos(self->proxy->pa)
    + (self->goal_py - self->proxy->py) * sin(self->proxy->pa);
  ry = - (self->goal_px - self->proxy->px) * sin(self->proxy->pa)
    + (self->goal_py - self->proxy->py) * cos(self->proxy->pa);
  ra = self->goal_pa - self->proxy->pa;

  // Move the goal figure
  rtk_fig_origin(self->control_fig, rx, ry, ra);

  // Dont draw the path
  rtk_fig_clear(self->path_fig);
    
  return;
}


// Servo the robot (velocity control)
void position2d_servo_vel(position2d_t *self)
{
  double d;
  double rx, ry, ra;
  double kr, ka;
  double vr, va;
  double min_vr, max_vr;
  double min_va, max_va;

  // Only servo if we are subscribed and have enabled commands.
  if (self->proxy->info.subscribed &&
      rtk_menuitem_ischecked(self->command_item))    
  {
    rtk_fig_show(self->control_fig, 1);
    rtk_fig_show(self->path_fig, 1);
  }
  else
  {
    rtk_fig_show(self->control_fig, 0);
    rtk_fig_show(self->path_fig, 0);
    return;
  }

  // Good for P2DX
  min_vr = -0.10;
  max_vr = 0.50;
  min_va = -M_PI/4;
  max_va = +M_PI/4;
  kr = max_vr / 1.00;
  ka = max_va / 1.00;

  /*
  // Good for P2AT
  min_vr = -2.00;
  max_vr = +2.00;
  min_va = -2 * M_PI;
  max_va = +2 * M_PI;
  kr = max_vr / 2.00;
  ka = max_va / 2.00;
  */

  if (rtk_fig_mouse_selected(self->control_fig))
  {
    // Get goal pose in robot cs
    rtk_fig_get_origin(self->control_fig, &rx, &ry, &ra);
  }
  else
  { 
    // Reset the goal figure
    rx = ry = ra = 0;
    rtk_fig_origin(self->control_fig, rx, ry, ra);
  }

  vr = kr * rx;
  va = ka * ry;

  if (vr < 0)
    va *= -1;
  
  // Bound the speed
  if (vr > max_vr)
    vr = max_vr;
  if (vr < min_vr)
    vr = min_vr;
  if (va > max_va)
    va = max_va;
  if (va < min_va)
    va = min_va;
    
  //printf("%f %f\n", vr, va);
      
  // Set the new speed
  playerc_position2d_set_cmd_vel(self->proxy, vr, 0, va, 1);

  // Draw in the path
  d = 0.30;
  rtk_fig_clear(self->path_fig);
  rtk_fig_color_rgb32(self->path_fig, COLOR_POSITION_CONTROL);
  if (rx >= 0)
  {
    rtk_fig_line(self->path_fig, 0, 0, d, 0);
    rtk_fig_line(self->path_fig, d, 0, rx, ry);
  }
  else
  {
    rtk_fig_line(self->path_fig, 0, 0, -d, 0);
    rtk_fig_line(self->path_fig, -d, 0, rx, ry);
  }
  return;
}

// Servo the robot (car control)
void position2d_servo_car(position2d_t *self)
{
  double d;
  double rx, ry, ra;
  double kr, ka;
  double vr, va;
  double min_vr, max_vr;
  double min_va, max_va;

  // Only servo if we are subscribed and have enabled commands.
  if (self->proxy->info.subscribed &&
      rtk_menuitem_ischecked(self->command_item))    
  {
    rtk_fig_show(self->control_fig, 1);
    rtk_fig_show(self->path_fig, 1);
  }
  else
  {
    rtk_fig_show(self->control_fig, 0);
    rtk_fig_show(self->path_fig, 0);
    return;
  }

  // Good for P2DX
  min_vr = -0.10;
  max_vr = 0.50;
  min_va = -M_PI/4;
  max_va = +M_PI/4;
  kr = max_vr / 1.00;
  ka = max_va / 1.00;

  /*
  // Good for P2AT
  min_vr = -2.00;
  max_vr = +2.00;
  min_va = -2 * M_PI;
  max_va = +2 * M_PI;
  kr = max_vr / 2.00;
  ka = max_va / 2.00;
  */

  if (rtk_fig_mouse_selected(self->control_fig))
  {
    // Get goal pose in robot cs
    rtk_fig_get_origin(self->control_fig, &rx, &ry, &ra);
  }
  else
  { 
    // Reset the goal figure
    rx = ry = ra = 0;
    rtk_fig_origin(self->control_fig, rx, ry, ra);
  }

  vr = kr * rx;
  va = ka * ry;

  if (vr < 0)
    va *= -1;
  
  // Bound the speed
  if (vr > max_vr)
    vr = max_vr;
  if (vr < min_vr)
    vr = min_vr;
  if (va > max_va)
    va = max_va;
  if (va < min_va)
    va = min_va;
    
  //printf("%f %f\n", vr, va);
      
  // Set the new speed
  //playerc_position2d_set_cmd_vel(self->proxy, vr, 0, va, 1);
  playerc_position2d_set_cmd_car(self->proxy,vr,va);
  // Draw in the path
  d = 0.30;
  rtk_fig_clear(self->path_fig);
  rtk_fig_color_rgb32(self->path_fig, COLOR_POSITION_CONTROL);
  if (rx >= 0)
  {
    rtk_fig_line(self->path_fig, 0, 0, d, 0);
    rtk_fig_line(self->path_fig, d, 0, rx, ry);
  }
  else
  {
    rtk_fig_line(self->path_fig, 0, 0, -d, 0);
    rtk_fig_line(self->path_fig, -d, 0, rx, ry);
  }
  return;
}










