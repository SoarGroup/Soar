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
