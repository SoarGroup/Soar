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
