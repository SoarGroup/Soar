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
