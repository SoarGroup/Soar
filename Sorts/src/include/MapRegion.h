#ifndef MapRegion_H_
#define MapRegion_H_

#include <set>

#include "GameTile.H"
#include "Map.H"
#include "GameObj.H"

#include "Rectangle.h"

class PerceptualGroup;

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

  void groupEnter(PerceptualGroup* g);
  void groupExit(PerceptualGroup* g);

  // some queries on the region
  bool isOccupied();
  bool isFriendly();

  int getId();

private: // functions
  void recalcPointBBox();

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
  set<PerceptualGroup*> groupsHere;

  list<Rectangle> boundaries;

};

#endif
