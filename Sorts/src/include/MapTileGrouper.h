#ifndef MapTileGrouper_H
#define MapTileGrouper_H

#include "MapRegion.h"

class MapTileGrouper {
public:
  // for some reason if you don't have this it generates a warning
  virtual ~MapTileGrouper() {} 
  virtual MapRegion* groupTile(int tile) = 0;
};

#endif
