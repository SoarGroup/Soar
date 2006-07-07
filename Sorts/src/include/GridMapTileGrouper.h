/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
#ifndef GridMapTileGrouper_H
#define GridMapTileGrouper_H

#include "Map.H"
#include "GameTile.H"

#include "MapTileGrouper.h"
#include "MapRegion.h"

#include <map>

using namespace std;

class GridMapTileGrouper : public MapTileGrouper {
public:

  GridMapTileGrouper(const Map<GameTile>& _gameMap, 
                     int                  _tile_points,
                     int                  _gridSizeX,
                     int                  _gridSizeY);

  ~GridMapTileGrouper();

  MapRegion* groupTile(int tile);

private:
  // needed only to pass into constructor of MapRegion, probably bad design
  const Map<GameTile>& gameMap;
  int tile_points;

  map<int, MapRegion*> currRegions;
  int gridSizeX, gridSizeY;
};

#endif
