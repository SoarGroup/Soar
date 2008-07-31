/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004  Brian Gerkey gerkey@stanford.edu    
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * $Id: mapscale.cc 4126 2007-08-20 06:37:30Z thjc $
 *
 * A driver to read an occupancy grid map from another map device and
 * scale it to produce a map with a different given resolution.
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_mapscale mapscale
 * @brief Scale grid maps

The mapscale driver reads a occupancy grid map from another @ref
interface_map device and scales it to produce a new map
with a different resolution.  The scaling is accomplished with the
gdk_pixbuf_scale_simple() function, using the GDK_INTERP_HYPER algorithm.

@par Compile-time dependencies

- gdk-pixbuf-2.0 (usually installed as part of GTK)

@par Provides

- @ref interface_map : the resulting scaled map

@par Requires

- @ref interface_map : the raw map, to be scaled

@par Configuration requests

- PLAYER_MAP_REQ_GET_INFO
- PLAYER_MAP_REQ_GET_DATA

@par Configuration file options

- resolution (length)
  - Default: -1.0
  - The new scale (length / pixel).
 
@par Example 

@verbatim
driver
(
  name "mapfile"
  provides ["map:0"]
  filename "mymap.pgm"
  resolution 0.1  # 10cm per pixel
)
driver
(
  name "mapscale"
  requires ["map:0"]  # read from map:0
  provides ["map:1"]  # output scaled map on map:1
  resolution 0.5 # scale to 50cm per pixel
)
@endverbatim

@author Brian Gerkey

*/
/** @} */


#include "maptransform.h"

// use gdk to interpolate
#include <gdk-pixbuf/gdk-pixbuf.h>

class MapScale : public MapTransform
{
  private:
    // interpolate the map
    int Transform();
    
  public:
    MapScale(ConfigFile* cf, int section);
    ~MapScale();
};

////////////////////////////////////////////////////////////////////////////////
// Create an instance of the driver
Driver* MapScale_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new MapScale(cf, section)));
}


////////////////////////////////////////////////////////////////////////////////
// Register the driver
void MapScale_Register(DriverTable* table)
{
  table->AddDriver("mapscale", MapScale_Init);
  return;
}



// this one has no data or commands, just configs
MapScale::MapScale(ConfigFile* cf, int section)
  : MapTransform(cf, section)
{
  if((new_map.scale = cf->ReadLength(section,"resolution",-1.0)) < 0)
  {
    PLAYER_ERROR("must specify positive map resolution");
    return;
  }
}

MapScale::~MapScale()
{
}

// interpolate the map
int
MapScale::Transform()
{
  unsigned int i,j;
  double scale_factor;
  GdkPixbuf* pixbuf;
  GdkPixbuf* new_pixbuf;
  int rowstride;
  int bits_per_sample;
  int n_channels;
  gboolean has_alpha;
  guchar* map_pixels;
  guchar* new_map_pixels;
  guchar* p;
  int new_rowstride;

  has_alpha = FALSE;
  
  bits_per_sample = 8;
  n_channels = 3;
  rowstride = source_map.width * sizeof(guchar) * n_channels;
  map_pixels = (guchar*)calloc(this->source_map.width * this->source_map.height,
                                      sizeof(guchar)*n_channels);
  assert(map_pixels);                                      

  // fill in the image from the map
  for(j=0; j<this->source_map.height; j++)
  {
    for(i=0; i<this->source_map.width; i++)
    {
      // fill the corresponding image pixel
      p = map_pixels + (this->source_map.height - j - 1)*rowstride + i*n_channels;

      if(this->source_data[MAP_IDX(source_map,i,j)] == -1)
        *p = 255;
      else if(this->source_data[MAP_IDX(source_map,i,j)] == 0)
        *p = 127;
      else
        *p = 0;
    }
  }

  g_assert((pixbuf = gdk_pixbuf_new_from_data((const guchar*)map_pixels,
                                              GDK_COLORSPACE_RGB,
                                              has_alpha,
                                              bits_per_sample,
                                              this->source_map.width,
                                              this->source_map.height,
                                              rowstride,
                                              NULL,NULL)));

  scale_factor = this->source_map.scale / this->new_map.scale;
  this->new_map.width = static_cast<unsigned int> (rint(this->source_map.width * scale_factor));
  this->new_map.height = static_cast<unsigned int> (rint(this->source_map.height * scale_factor));
  
  PLAYER_MSG3(4,"MapScale: New map is %dx%d scale %f",new_map.width,new_map.height,new_map.scale);

  new_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 
                                                 this->new_map.width,
                                                 this->new_map.height,
                                                 GDK_INTERP_HYPER);
  g_assert(new_pixbuf);                                                 
                                                 
  new_map_pixels = gdk_pixbuf_get_pixels(new_pixbuf);
  new_rowstride = gdk_pixbuf_get_rowstride(new_pixbuf);

  this->new_data = new char[this->new_map.width * this->new_map.height];
  assert(new_data);                                           
  // fill in the map from the scaled image
  for(j=0; j<this->new_map.height; j++)
  {
    for(i=0; i<this->new_map.width; i++)
    {
      // fill the corresponding map cell
      p = new_map_pixels + (this->new_map.height-j-1)*new_rowstride + i*n_channels;

      if(*p > 0.66 * 255)
        this->new_data[MAP_IDX(new_map,i,j)] = -1;
      else if(*p < 0.33 * 255)
        this->new_data[MAP_IDX(new_map,i,j)] = 1;
      else
        this->new_data[MAP_IDX(new_map,i,j)] = 0;
//      printf("%d %d %d %d \n",i,j,*p, this->new_mapdata[MAP_IDX(scaled_map,i,j)]);
    }
  }

  g_object_unref((GObject*)pixbuf);
  // TODO: create a GdkPixbufDestroyNotify function, and pass it to
  // gdk_pixbuf_new_from_data() above, so that this buffer is free()d
  // automatically.
  free(map_pixels);
  map_pixels = NULL;
  g_object_unref((GObject*)new_pixbuf);

  return(0);
}

