#ifndef MapTileGrouper_H
#define MapTileGrouper_H

#include "MapRegion.h"

class MapTileGrouper {
  virtual MapRegion* groupTile(int tile) = 0;
};

#endif
