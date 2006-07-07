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
#include "MapRegion.h"

#include <assert.h>

#include "Map.H"
#include "GameTile.H"

#include "PerceptualGroup.h"

int MapRegion::idCounter = 0;

// every region must have at least one tile in it
MapRegion::MapRegion(const Map<GameTile>& _map, int _tile_points, int tile)
: map(_map), tile_points(_tile_points)
{
  id = idCounter++;
  tiles.insert(tile);
  int ind_x = map.ind2x(tile);
  int ind_y = map.ind2y(tile);
  indexBBox.collapse(ind_x, ind_y);
  recalcPointBBox();
}

bool MapRegion::containsPoint(int x, int y) {
  int ind_x = x % tile_points;
  int ind_y = y % tile_points;
  return tiles.find(map.xy2ind(ind_x, ind_y)) != tiles.end();
}

Rectangle MapRegion::getBoundingBox() {
  return pointBBox;
}

Rectangle MapRegion::getIndexBoundingBox() {
  return indexBBox;
}

// as in area, not extension
int MapRegion::size() {
  return tiles.size();
}

void MapRegion::groupEnter(PerceptualGroup* g) {
  assert(groupsHere.find(g) == groupsHere.end());

  if (groupsHere.find(g) != groupsHere.end()) {
    return;
  }
  groupsHere.insert(g);
}

void MapRegion::groupExit(PerceptualGroup* g) {
  assert(groupsHere.find(g) != groupsHere.end());

  groupsHere.erase(g);
}

bool MapRegion::isOccupied() {
  return !groupsHere.empty();
}

bool MapRegion::isFriendly() {
  for( set<PerceptualGroup*>::iterator 
       i  = groupsHere.begin();
       i != groupsHere.end();
       i++ )
  {
    if (!(*i)->isFriendly()) {
      return false;
    }
  }
  return true;
}

// there's probably a better way to do this
bool MapRegion::intersects(const Rectangle& r) {
  if (!pointBBox.intersects(r)) {
    return false;
  }
  Rectangle tile_rect;
  for(set<int>::iterator i = tiles.begin(); i != tiles.end(); i++) {
    int xmin = map.ind2x(*i) * tile_points;
    int ymin = map.ind2y(*i) * tile_points;
    tile_rect.set(xmin, xmin + tile_points, ymin, ymin + tile_points);
    if (tile_rect.intersects(r)) {
      return true;
    }
  }
  return false;
}

void MapRegion::addTile(int tile) {
  int ind_x = map.ind2x(tile);
  int ind_y = map.ind2y(tile);

  tiles.insert(tile);
  indexBBox.accomodate(ind_x, ind_y);
  recalcPointBBox();
}

void MapRegion::addBoundary(GameObj* gob) {
  Rectangle r;
  r.set(*gob->sod.x1, *gob->sod.x2, *gob->sod.y1, *gob->sod.y2);
  boundaries.push_back(r);
}

void MapRegion::recalcPointBBox() {
  pointBBox.xmin = indexBBox.xmin * tile_points;
  pointBBox.ymin = indexBBox.ymin * tile_points;
  // + 1 to include lower right corner
  pointBBox.xmax = (indexBBox.xmax + 1) * tile_points;
  pointBBox.ymax = (indexBBox.ymax + 1) * tile_points;
}

int MapRegion::getId() { 
  return id;
}
