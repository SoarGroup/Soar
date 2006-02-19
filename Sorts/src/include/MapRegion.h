#ifndef MapRegion_H_
#define MapRegion_H_

#include <set>

#include "GameTile.H"
#include "Map.H"

#include "Rectangle.h"

class SoarGameGroup;

using namespace std;

class MapRegion {
public:
  MapRegion(const Map<GameTile>& _map, int _tile_points, int tile);

  bool containsPoint(int x, int y);

  Rectangle getBoundingBox();
  Rectangle getIndexBoundingBox();
  bool intersects(const Rectangle& r);

  int size();

  void addTile(int tile);
  void addBoundary(const Rectangle& b);

  void groupEnter(SoarGameGroup* g);
  void groupExit(SoarGameGroup* g);

  // some queries on the region
  bool isOccupied();
  bool isFriendly();

private:

  const Map<GameTile>& map; 

  int tile_points;

  // bounding boxes
  Rectangle indexBBox;
  Rectangle pointBBox;

  set<int> tiles;

  // groups that are currently in this region
  set<SoarGameGroup*> groupsHere;

  list<Rectangle> boundaries;

  void recalcPointBBox();
};

#endif
