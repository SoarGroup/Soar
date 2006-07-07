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
#include "GridMapTileGrouper.h"
#include <iostream>
#include <assert.h>

#include "Rectangle.h"

using namespace std;

GridMapTileGrouper::GridMapTileGrouper
( const Map<GameTile>& _gameMap,
  int                  _tile_points,
  int                  _gridSizeX, 
  int                  _gridSizeY )

: gameMap(_gameMap), 
  tile_points(_tile_points), 
  gridSizeX(_gridSizeX), 
  gridSizeY(_gridSizeY)
{
  //the grid height and width better be integral divisions of the map height and width
  assert(gameMap.get_width() % gridSizeX == 0 && 
         gameMap.get_height() % gridSizeY == 0);
}

GridMapTileGrouper::~GridMapTileGrouper() {
  for( map<int, MapRegion*>::iterator 
       i = currRegions.begin(); 
       i != currRegions.end();
       i++ )
  {
    delete i->second;
  }
}

MapRegion* GridMapTileGrouper::groupTile(int tile) {
  int x = gameMap.ind2x(tile), y = gameMap.ind2y(tile);
  int gridX = x / gridSizeX, gridY = y / gridSizeY;
  int gridIndex = gridY * gridSizeX + gridX;

  if (currRegions.find(gridIndex) == currRegions.end()) {
    MapRegion* newRegion = new MapRegion(gameMap, tile_points, tile);
    currRegions[gridIndex] = newRegion;

   // cout << "NEW REGION " << (*newRegion).getBoundingBox() << " CREATED" << endl;

    return newRegion;
  }
  else {
    MapRegion *r = (*currRegions.find(gridIndex)).second;
    r->addTile(tile);
   // cout << "APPENDED TO REGION " << r->getId() << ": " 
   //      << r->getBoundingBox() << std::endl;
    return r;
  }
}
