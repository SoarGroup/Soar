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
 * Desc: Gripper interface
 * Author: Richard Vaughan (based on sonar dev by Andrew Howard)
 * Date: 13 Feb 2004
 * CVS: $Id: pv_dev_gripper.c 4152 2007-09-17 02:18:59Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"

// grey
#define GRIPPER_COLOR_FILL 0xAAAAAA
#define GRIPPER_COLOR_LINE 0x101010

// Update the bumper configuration
void gripper_update_config(gripper_t *gripper);

// Draw the gripper scan
void gripper_draw(gripper_t *gripper);

// Dont draw the gripper scan
void gripper_nodraw(gripper_t *gripper);


// Create a gripper device
gripper_t *gripper_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                      int index, const char *drivername, int subscribe)
{
  int i;
  char label[64];
  char section[64];
  gripper_t *gripper;

  gripper = malloc(sizeof(gripper_t));
  gripper->proxy = playerc_gripper_create(client, index);
  gripper->drivername = strdup(drivername);
  gripper->datatime = 0;

  snprintf(section, sizeof(section), "gripper:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "gripper:%d (%s)", index, gripper->drivername);
  gripper->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  gripper->subscribe_item = rtk_menuitem_create(gripper->menu, "Subscribe", 1);
 gripper->open_item = rtk_menuitem_create(gripper->menu, "Open Gripper", 0);
 gripper->close_item = rtk_menuitem_create(gripper->menu, "Close Gripper", 0);


  // Set the initial menu state
  // Set initial device state
  rtk_menuitem_check(gripper->subscribe_item, subscribe);

  // Construct figures
  gripper->grip_fig = rtk_fig_create(mainwnd->canvas, mainwnd->robot_fig, 1);

  return gripper;
}


// Destroy a gripper device
void gripper_destroy(gripper_t *gripper)
{
  int i;

  if( gripper->grip_fig )
    {
      rtk_fig_clear(gripper->grip_fig);
      rtk_fig_destroy(gripper->grip_fig);
    }

  if (gripper->proxy->info.subscribed)
    playerc_gripper_unsubscribe(gripper->proxy);
  playerc_gripper_destroy(gripper->proxy);

  rtk_menuitem_destroy(gripper->subscribe_item);
  rtk_menu_destroy(gripper->menu);

  free(gripper->drivername);
  free(gripper);
}


// Update a gripper device
void gripper_update(gripper_t *gripper)
{
  int i;

  // Update the device subscription
  if (rtk_menuitem_ischecked(gripper->subscribe_item))
  {
    if (!gripper->proxy->info.subscribed)
    {
      if (playerc_gripper_subscribe(gripper->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("subscribe failed : %s", playerc_error_str());

      // Get the gripper geometry
      if (playerc_gripper_get_geom(gripper->proxy) != 0)
	PRINT_ERR1("get_geom failed : %s", playerc_error_str());

      // draw the gripper body
      rtk_fig_origin(  gripper->grip_fig,
		       gripper->proxy->pose.px,
		       gripper->proxy->pose.py,
		       gripper->proxy->pose.pyaw );

      rtk_fig_rectangle( gripper->grip_fig,
			 0,0,0,
			 gripper->proxy->outer_size.sw,
			 gripper->proxy->outer_size.sl,
			 0 );
    }
  }
  else
  {
    if (gripper->proxy->info.subscribed)
      if (playerc_gripper_unsubscribe(gripper->proxy) != 0)
        PRINT_ERR1("unsubscribe failed : %s", playerc_error_str());
  }
  rtk_menuitem_check(gripper->subscribe_item, gripper->proxy->info.subscribed);

  if (gripper->proxy->info.subscribed)
  {
    // Draw in the gripper scan if it has been changed.
    if (gripper->proxy->info.datatime != gripper->datatime)
      {
	//playerc_gripper_printout( gripper->proxy, NULL );

	rtk_fig_clear( gripper->grip_fig );

	// draw paddles

	double gripper_length = gripper->proxy->outer_size.sw;
	double gripper_width = gripper->proxy->outer_size.sl;

	double paddle_center = gripper_length * (1.0/6.0);
	double paddle_length = gripper_length * (2.0/3.0);
	double paddle_width = gripper_width * 0.15;

	rtk_fig_color_rgb32( gripper->grip_fig, GRIPPER_COLOR_FILL );
	rtk_fig_rectangle( gripper->grip_fig,
			   gripper_length * -2.0/6.0,0,0,
			   gripper_length/3.0,
			   gripper_width,
			   1 );

	rtk_fig_color_rgb32( gripper->grip_fig, GRIPPER_COLOR_LINE );
	rtk_fig_rectangle( gripper->grip_fig,
			   gripper_length * -2.0/6.0,0,0,
			   gripper_length/3.0,
			   gripper_width,
			   0 );

	if( gripper->proxy->state == PLAYER_GRIPPER_STATE_OPEN )
	  {
	    double paddle_pos = gripper_width/2.0 - paddle_width/2.0;

	    rtk_fig_color_rgb32( gripper->grip_fig, GRIPPER_COLOR_FILL );
	    rtk_fig_rectangle(gripper->grip_fig,
			      paddle_center, paddle_pos, 0,
			      paddle_length, paddle_width, 1 );
	    rtk_fig_rectangle(gripper->grip_fig,
			      paddle_center, -paddle_pos, 0,
			      paddle_length, paddle_width, 1 );

	    rtk_fig_color_rgb32( gripper->grip_fig, GRIPPER_COLOR_LINE );
	    rtk_fig_rectangle(gripper->grip_fig,
			      paddle_center, paddle_pos, 0,
			      paddle_length, paddle_width, 0 );
	    rtk_fig_rectangle(gripper->grip_fig,
			      paddle_center, -paddle_pos, 0,
			      paddle_length, paddle_width, 0 );
	  }


	if( gripper->proxy->state == PLAYER_GRIPPER_STATE_CLOSED )
	  {
	    double paddle_pos = paddle_width/2.0;

	    rtk_fig_color_rgb32( gripper->grip_fig, GRIPPER_COLOR_FILL );
	    rtk_fig_rectangle(gripper->grip_fig,
			      paddle_center, paddle_pos, 0,
			      paddle_length, paddle_width, 1 );
	    rtk_fig_rectangle(gripper->grip_fig,
			      paddle_center, -paddle_pos, 0,
			      paddle_length, paddle_width, 1 );

	    rtk_fig_color_rgb32( gripper->grip_fig, GRIPPER_COLOR_LINE );
	    rtk_fig_rectangle(gripper->grip_fig,
			      paddle_center, paddle_pos, 0,
			      paddle_length, paddle_width, 0 );
	    rtk_fig_rectangle(gripper->grip_fig,
			      paddle_center, -paddle_pos, 0,
			      paddle_length, paddle_width, 0 );
	  }

	if( gripper->proxy->state == PLAYER_GRIPPER_STATE_MOVING )
	  {
	    rtk_fig_color_rgb32( gripper->grip_fig, GRIPPER_COLOR_FILL );
	    rtk_fig_rectangle( gripper->grip_fig,
			       paddle_center,0,0,
			       paddle_length,
			       gripper_width,
			       1 );
	  }


	// different x location for each beam
	double ibbx =  paddle_center - 0.3*paddle_length;
	double obbx =  paddle_center + 0.3*paddle_length;

	// common y position
	double bby = (gripper->proxy->state == PLAYER_GRIPPER_STATE_OPEN) ? gripper_width/2.0 - paddle_width: 0;

	// size of the paddle indicator lights
	double led_dx = paddle_width/2.0;

	if( gripper->proxy->beams & 0x00000001 )
	  {
	    rtk_fig_rectangle( gripper->grip_fig, ibbx, bby+led_dx, 0, led_dx, led_dx, 1 );
	    rtk_fig_rectangle( gripper->grip_fig, ibbx, -bby-led_dx, 0, led_dx, led_dx, 1 );
	  }
	else
	  {
	    //rtk_fig_line( gripper->grip_fig, ibbx, bby, ibbx, -bby );
	    rtk_fig_rectangle( gripper->grip_fig, ibbx, bby+led_dx, 0, led_dx, led_dx, 0 );
	    rtk_fig_rectangle( gripper->grip_fig, ibbx, -bby-led_dx, 0, led_dx, led_dx, 0 );
	  }

	if( gripper->proxy->beams & 0x00000002 )
	  {
	    rtk_fig_rectangle( gripper->grip_fig, obbx, bby+led_dx, 0, led_dx, led_dx, 1 );
	    rtk_fig_rectangle( gripper->grip_fig, obbx, -bby-led_dx, 0, led_dx, led_dx, 1 );
	  }
	else
	  {
	    //rtk_fig_line( gripper->grip_fig, obbx, bby, obbx, -bby );
	    rtk_fig_rectangle( gripper->grip_fig, obbx, bby+led_dx, 0, led_dx, led_dx, 0 );
	    rtk_fig_rectangle( gripper->grip_fig, obbx, -bby-led_dx, 0, led_dx, led_dx, 0 );
	  }

      }
    gripper->datatime = gripper->proxy->info.datatime;
  }
  else
  {
    // Dont draw the gripper.
    rtk_fig_clear(gripper->grip_fig);
  }

 if(gripper->proxy->info.subscribed)
 {
   if(rtk_menuitem_isactivated(gripper->open_item))
   {
     puts("opening gripper...");
     if(playerc_gripper_open_cmd(gripper->proxy) != 0)
       PRINT_ERR1("libplayerc error opening gripper: %s", playerc_error_str());
   }

   if(rtk_menuitem_isactivated(gripper->close_item))
   {
     puts("closing gripper...");
     if(playerc_gripper_close_cmd(gripper->proxy) != 0)
       PRINT_ERR1("libplayerc error closing gripper: %s", playerc_error_str());
   }
 }

}

