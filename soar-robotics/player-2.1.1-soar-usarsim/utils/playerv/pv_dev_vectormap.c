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
 * CVS: $Id: pv_dev_vectormap.c 4363 2008-02-17 23:05:05Z thjc $
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "playerv.h"

// Update the map configuration
void vectormap_update_config(vectormap_t *map);

// Draw the map 
void vectormap_draw(vectormap_t *map);

// Draw a single map feature
void vectormap_draw_feature(vectormap_t *map, GEOSGeom geom);


// Create a map device
vectormap_t *vectormap_create(mainwnd_t *mainwnd, opt_t *opt, playerc_client_t *client,
                      int index, const char *drivername, int subscribe)
{
  char label[64];
  char section[64];
  vectormap_t *map;
  
  map = malloc(sizeof(vectormap_t));
  map->proxy = playerc_vectormap_create(client, index);
  map->drivername = strdup(drivername);
  map->datatime = 0;

  snprintf(section, sizeof(section), "vectormap:%d", index);

  // Construct the menu
  snprintf(label, sizeof(label), "vectormap:%d (%s)", index, map->drivername);
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
void vectormap_destroy(vectormap_t *map)
{
  if (map->proxy->info.subscribed)
    playerc_vectormap_unsubscribe(map->proxy);

  playerc_vectormap_destroy(map->proxy);
  rtk_fig_destroy(map->fig);
  free(map->drivername);
  free(map);

  return;
}


// Update a map device
void vectormap_update(vectormap_t *map)
{
  unsigned ii;

  // Update the device subscription
  if (rtk_menuitem_ischecked(map->subscribe_item))
  {
    if (!map->proxy->info.subscribed)
    {
      if (playerc_vectormap_subscribe(map->proxy, PLAYER_OPEN_MODE) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());

      // get the map info
      playerc_vectormap_get_map_info( map->proxy );

      // download intial map data
      for (ii = 0;  ii < map->proxy->layers_count; ++ii)
      {
        if (playerc_vectormap_get_layer_data( map->proxy, ii ))
        {
          PRINT_ERR1("libplayerc error: %s", playerc_error_str());
          return;
        }
      }
      vectormap_draw( map );
    }
  }
  else
  {
    if (map->proxy->info.subscribed)
      if (playerc_vectormap_unsubscribe(map->proxy) != 0)
        PRINT_ERR1("libplayerc error: %s", playerc_error_str());
  }
  rtk_menuitem_check(map->subscribe_item, map->proxy->info.subscribed);

  // Dont draw the map.
  if(map->proxy->info.subscribed)
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
			if (time_diff > VECTORMAP_UPDATE_TIME)
			{
				// get the map info
    				playerc_vectormap_get_map_info( map->proxy );

    				// download intial map data
    				for (ii = 0;  ii < map->proxy->layers_count; ++ii)
    				{
    				    if (playerc_vectormap_get_layer_data( map->proxy, ii ))
    				    {
        				PRINT_ERR1("libplayerc error: %s", playerc_error_str());
        				return;
    				    }
    				}
    				vectormap_draw( map );
				old_time = time;
			}
    }
    rtk_fig_show(map->fig, 1);
  } else
    rtk_fig_show(map->fig, 0);
}

// Draw the map scan
void vectormap_draw(vectormap_t *map)
{
  unsigned ii, jj;
  GEOSGeom feature;

  rtk_fig_show(map->fig, 1);
  rtk_fig_clear(map->fig);

  // draw map data
  uint32_t colour = 0xFF0000;
  for (ii = 0;  ii < map->proxy->layers_count; ++ii)
  {
    // get a different colour for each layer the quick way, will duplicate after 6 layers
    colour = colour >> 4 & colour << 24;
    rtk_fig_color_rgb32(map->fig, colour);

    // render the features
    for (jj = 0; jj < map->proxy->layers_data[ii]->features_count; ++jj)
    {
      feature = playerc_vectormap_get_feature_data( map->proxy, ii, jj );
      if (feature)
        vectormap_draw_feature(map, feature);
    }
  }

  // draw map extent
  rtk_fig_color_rgb32( map->fig, 0xFF0000 );
  double xCenter = map->proxy->extent.x1 - (map->proxy->extent.x1 - map->proxy->extent.x0)/2;
  double yCenter = map->proxy->extent.y1 - (map->proxy->extent.y1 - map->proxy->extent.y0)/2;

  rtk_fig_rectangle(
                    map->fig,
                    xCenter,
                    yCenter,
                    0,
                    map->proxy->extent.x1 - map->proxy->extent.x0,
                    map->proxy->extent.y1 - map->proxy->extent.y0,
                    0
                   );

  return;
}


void vectormap_draw_feature(vectormap_t *map, GEOSGeom geom)
{
#ifdef HAVE_GEOS
  GEOSCoordSeq seq;
  unsigned numcoords;
  unsigned ii;
  double x,y,x2,y2;
  switch (GEOSGeomTypeId(geom)) 
  {
    case GEOS_POINT:
      seq = GEOSGeom_getCoordSeq(geom);
      GEOSCoordSeq_getX(seq, 00, &x);
      GEOSCoordSeq_getY(seq, 00, &y);
      rtk_fig_line( map->fig, x-0.1, y ,x+0.1, y);
      rtk_fig_line( map->fig, x, y-0.1 ,x, y+0.1);
      break;
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
      seq = GEOSGeom_getCoordSeq(geom);
      if(GEOSCoordSeq_getSize(seq, &numcoords))
      {
        GEOSCoordSeq_getX(seq, 0, &x2);
        GEOSCoordSeq_getY(seq, 0, &y2);
        if (numcoords < 2)
        {
          rtk_fig_point( map->fig, x2, y2 );
        }
        else
        {
          for (ii = 1; ii < numcoords; ++ii)
          {
            x = x2;
            y = y2;
            GEOSCoordSeq_getX(seq, ii, &x2);
            GEOSCoordSeq_getY(seq, ii, &y2);
            rtk_fig_line( map->fig, x, y ,x2, y2);
          }
        }
      }
      break;

    case GEOS_POLYGON:
      vectormap_draw_feature(map,GEOSGetExteriorRing(geom));
      for (ii = 0; ii < GEOSGetNumInteriorRings(geom); ++ii)
      {
        vectormap_draw_feature(map,GEOSGetInteriorRingN(geom,ii));
      }
      break;

    case GEOS_MULTIPOINT:
    case GEOS_MULTILINESTRING:
    case GEOS_MULTIPOLYGON:
    case GEOS_GEOMETRYCOLLECTION:
      for (ii = 0; ii < GEOSGetNumGeometries(geom); ++ii)
      {
        vectormap_draw_feature(map,GEOSGetGeometryN(geom,ii));
      }
      break;

    default:
      PRINT_ERR1("unknown feature type: %d", GEOSGeomTypeId(geom));
  }
#else
  printf("lib GEOS was not available at build time so features will not be rendered\n");
#endif
}
