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
 * Desc: Map interface
 * Author: Richard Vaughan, based on similar code by Andrew Howard
 * Date: 5 March 2005
 * CVS: $Id: pv_dev_map.c 4232 2007-11-01 22:16:23Z gerkey $
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"


// Update the map configuration
void map_update_config(map_t *map);

// Draw the map scan
void map_draw(map_t *map);


// Create a map device
map_t *map_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                      int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  map_t *map;
  
  map = malloc(sizeof(map_t));
  map->proxy = playerc_map_create(client, index);
  map->drivername = strdup(drivername);
  map->datatime = 0;

  snprintf(section, sizeof(section), "map:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "map:%d (%s)", index, map->drivername);
  map->menu = rtk_menu_create_sub(mainwnd->device_menu, label);
  map->subscribe_item = rtk_menuitem_create(map->menu, "Subscribe", 1);
  map->continuous_item = rtk_menuitem_create(map->menu, "continuous update", 1);
  // Set the initial menu state
  rtk_menuitem_check(map->subscribe_item, subscribe);
  
  // Construct figures
  map->fig = rtk_fig_create(mainwnd->canvas, NULL, 1);
  
  return map;
}


// Destroy a map device
void map_destroy(map_t *map)
{
  if (map->proxy->info.subscribed)
    playerc_map_unsubscribe(map->proxy);

  playerc_map_destroy(map->proxy);
  rtk_fig_destroy(map->fig);
  free(map->drivername);
  free(map);
  
  return;
}


// Update a map device
void map_update(map_t *map)
{
  // Update the device subscription
  if (rtk_menuitem_ischecked(map->subscribe_item))
  {
    if (!map->proxy->info.subscribed)
    {
      if (playerc_map_subscribe(map->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
      // download a map
      if (playerc_map_get_map( map->proxy ) >= 0)	
        map_draw( map );
    }
  }
  else
  {
    if (map->proxy->info.subscribed)
      if (playerc_map_unsubscribe(map->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(map->subscribe_item, map->proxy->info.subscribed);

  if (map->proxy->info.subscribed)
  {
    if(rtk_menuitem_ischecked(map->continuous_item))
    {
			static struct timeval old_time = {0,0};
			struct timeval time;
			double time_diff = 0.0;
			gettimeofday(&time, NULL);
			// i don't use (map->proxy->info.datatime != map->datatime))
			// because some drivers return strange datatimes
			// and the map may change to often for the current map format
			// in playerv.h you can adjust MAP_UPDATE_TIME (default 1 sec)
			time_diff = (double)(time.tv_sec - old_time.tv_sec) +
				(double)(time.tv_usec - old_time.tv_usec)/1000000;
			if (time_diff > MAP_UPDATE_TIME)
			{
				playerc_map_get_map(map->proxy);
				map_draw(map);
				old_time = time;
			}
    }
  }
  else
  {
    // Dont draw the map.
    rtk_fig_show(map->fig, 0);
  }
}


// Draw the map scan
void map_draw(map_t *map)
{
  int x,y;
  char ntext[64], text[1024];
  double scale = map->proxy->resolution;

  rtk_fig_show(map->fig, 1);      
  rtk_fig_clear(map->fig);
  
  puts( "map draw" );

  rtk_fig_color_rgb32(map->fig, 0xFF0000 ); 
  rtk_fig_rectangle(map->fig, 
		    0,0,0,		   
		    map->proxy->width * scale,
		    map->proxy->height * scale,
		    0 );

  // TODO - combine contiguous cells to minimize the number of
  // rectangles we have to draw - performance is pretty nasty right
  // now on big maps.

  for( x=0; x<map->proxy->width; x++ )
    for( y=0; y<map->proxy->height; y++ )
      {
	switch( map->proxy->cells[ x + y * map->proxy->width ] )
	  {
	  case -1:
	    // empty: draw nothing
	    break;
	    
	  case 0:
	    // unknown: draw grey square
	    rtk_fig_color_rgb32(map->fig, 0x808080 );
	    rtk_fig_rectangle(map->fig,
			      (x - map->proxy->width/2.0) * scale + scale/2.0,
			      (y - map->proxy->height/2.0) * scale + scale/2.0,
			      0,
			      scale, scale, 1);
	    break;

	  case +1:  
	    // occupied: draw black square
	    rtk_fig_color_rgb32(map->fig, 0x0 ); 
	    rtk_fig_rectangle(map->fig, 
			      (x - map->proxy->width/2.0) * scale + scale/2.0, 
			      (y - map->proxy->height/2.0) * scale + scale/2.0, 
			      0, 
			      scale, scale, 1);	    
	    break;

	  default:
	    puts( "Warning: invalid occupancy value." );
	    break;
	  }
      }

  return;
}




