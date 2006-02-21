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
