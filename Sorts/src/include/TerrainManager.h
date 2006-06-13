#ifndef TerrainManager_H
#define TerrainManager_H

#include "TerrainContour.h"
#include "BFS.h"
#include "general.h"

#include <list>
#include <map>

class TerrainManager {
public:
  TerrainManager() { }
  ~TerrainManager();

  void addSegment(int x1, int y1, int x2, int y2);

private:
  list<TerrainContour*> contours;
  list<BFS*>            searches;
  map<Point, BFS*>      openPoints;
};

#endif
