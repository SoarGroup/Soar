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
 * $Id: maptransform.h 4135 2007-08-23 19:58:48Z gerkey $
 *
 * Base class for map transform drivers, simply reimplement the transform method
 * with your trasformation function. See MapScale for example
 */

#ifndef _MAPTRANSFORM_H_
#define _MAPTRANSFORM_H_

#include <sys/types.h> // required by Darwin
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libplayercore/playercore.h>

// compute linear index for given map coords
#define MAP_IDX(mf, i, j) ((mf.width) * (j) + (i))

// check that given coords are valid (i.e., on the map)
#define MAP_VALID(mf, i, j) ((i >= 0) && (i < mf.width) && (j >= 0) && (j < mf.height))

class MapTransform : public Driver
{
  protected:
    player_map_info_t source_map;
    player_devaddr_t source_map_addr;
    char* source_data;

	player_map_info_t new_map;
    char* new_data;

    // get the map from the underlying map device
    int GetMap();
    // interpolate the map
    virtual int Transform() = 0;
    
  public:
    MapTransform(ConfigFile* cf, int section);
    virtual ~MapTransform();

    // MessageHandler
    public: virtual int ProcessMessage(QueuePointer &resp_queue, 
                                     player_msghdr * hdr, 
                                     void * data);   
                                     
    int Setup();
    int Shutdown();
};

#endif
