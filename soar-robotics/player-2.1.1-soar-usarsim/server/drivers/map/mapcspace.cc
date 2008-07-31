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
 * $Id: mapcspace.cc 6566 2008-06-14 01:00:19Z thjc $
 *
 * A driver to read an occupancy grid map from another map device and
 * convolve it with a robot to create the C-space.
 */

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_mapcspace mapcspace
 * @brief Grow obstacles in grid maps

The mapcspace driver reads a occupancy grid map from another @ref
interface_map device and convolves it with a robot of a particular
shape and size to create the configuration space (C-space) map.  That is,
this driver "grows" obstacles in the map to produce a new map in which,
for path-planning purposes, you can consider the robot to be a point.

Both occupied and unknown cells are grown.

Note that @ref interface_map devices produce no data; the map is
delivered via a sequence of configuration requests.

@par Compile-time dependencies

- none

@par Provides

- @ref interface_map : the resulting C-space map

@par Requires

- @ref interface_map : the raw map, from which to make the C-space map

@par Configuration requests

- PLAYER_MAP_REQ_GET_INFO
- PLAYER_MAP_REQ_GET_DATA

@par Configuration file options

- robot_radius (length)
  - Default: -1.0
  - The radius of the robot to convolve with the map
- robot_shape (string)
  - Default: "circle"
  - The shape of the robot to convolve with the map.  Should be one of:
    "circle".
 
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
  name "mapcspace"
  requires ["map:0"]  # read from map:0
  provides ["map:1"]  # output C-space map on map:1
  robot_shape "circle"
  robot_radius 0.5 m
)
@endverbatim

@author Brian Gerkey

*/
/** @} */

#include "maptransform.h"

typedef enum
{
  CIRCLE,
} robot_shape_t;
         
class MapCspace : public MapTransform
{
  private:
    robot_shape_t robot_shape;
    double robot_radius;

    // convolve the map with a circular robot to produce the cspace
    int Transform();
    

  public:
    MapCspace(ConfigFile* cf, int section);
    ~MapCspace();
};

Driver*
MapCspace_Init(ConfigFile* cf, int section)
{
  return((Driver*)(new MapCspace(cf, section)));
}

// a driver registration function
void 
MapCspace_Register(DriverTable* table)
{
  table->AddDriver("mapcspace", MapCspace_Init);
}


// this one has no data or commands, just configs
MapCspace::MapCspace(ConfigFile* cf, int section)
  : MapTransform(cf, section)
{
  const char* shapestring;

  if((robot_radius = cf->ReadLength(section,"robot_radius",-1.0)) < 0)
  {
    PLAYER_ERROR("must specify positive robot radius");
    return;
  }
  if(!(shapestring = cf->ReadString(section,"robot_shape",NULL)))
  {
    PLAYER_ERROR("must specify robot shape");
    return;
  }
  if(!strcmp(shapestring,"circle"))
  {
    robot_shape = CIRCLE;
  }
  else
  {
    PLAYER_ERROR1("unknown robot shape \"%s\"", shapestring);
    return;
  }

}

MapCspace::~MapCspace()
{
}


// convolve the map with a circular robot to produce the cspace
int
MapCspace::Transform()
{
  unsigned int i,j;
  int di,dj;
  int r;
  char state;

  // allocate the transformed map
  new_map = source_map;
  new_data = new char[new_map.width * new_map.height];
  memcpy(new_data,source_data,new_map.width * new_map.height);

  PLAYER_MSG1(5,"MapCspace creating C-space for circular robot with radius %.3fm:",
         this->robot_radius);
  fflush(NULL);

  // compute robot radius in map cells
  r = (int)rint(this->robot_radius / source_map.scale);
  PLAYER_MSG1(5,"Robot Radius in map Cells: %d",r);

  for(j=0; j < this->source_map.height; j++)
  {
    for(i=0; i < this->source_map.width; i++)
    {
      state = this->source_data[MAP_IDX(source_map,i,j)];

      // grow both occupied and unknown regions
      if(state >= 0)
      {
        for(dj = -r; dj <= r; dj++)
        {
          for(di = -r; di <= r; di++)
          {
            // stay within the radius
            if((int)rint(sqrt(static_cast<double>(di*di + dj*dj))) > r)
              continue;

            // make sure we stay on the map
            if(!MAP_VALID(new_map,i+di,j+dj))
              continue;

            // don't change occupied to uknown
            if(this->new_data[MAP_IDX(new_map,i+di,j+dj)] < state)
            {
              this->new_data[MAP_IDX(new_map,i+di,j+dj)] = state;
            }
          }
        }
      }
    }
  }
  return(0);
}

