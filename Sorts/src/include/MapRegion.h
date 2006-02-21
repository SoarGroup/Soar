#ifndef MapRegion_H_
#define MapRegion_H_

#include <set>

#include "GameTile.H"
#include "Map.H"
#include "GameObj.H"

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
  void addBoundary(GameObj* gob);

  void groupEnter(SoarGameGroup* g);
  void groupExit(SoarGameGroup* g);

  // some queries on the region
  bool isOccupied();
  bool isFriendly();

  int getId();

private:
  // counter that increments region ids
  static int idCounter;

  int id;

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
