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
#ifndef MapManager_H_
#define MapManager_H_

#include<set>

#include "Map.H"
#include "ScriptObj.H"
#include "GameTile.H"
#include "Global.H"
#include "Vector.H"

#include "MapTileGrouper.h"
class Sorts;

using namespace std;

class MapManager {
public:
  MapManager( const Map<GameTile>& _gameMap, 
              int                  _tilePoints, 
              MapTileGrouper&      _tileGrouper);

  // these will be automatically associated with regions
  // the association is dependent on the policy we use
  void addExploredTiles(Vector<sint4> newTiles);

  void addBoundaries(Vector<ScriptObj*> boundaries);

  // querying functions
  MapRegion* getRegionUnder(int x, int y);
  void getRegionsIntersecting(const Rectangle& r, list<MapRegion*>& regions);

  // attention shifting functions
  // these will modify the Soar input link (via SoarInterface)
  void focus(int region);
  void defocus();

private:
  const Map<GameTile>& gameMap;
  int tilePoints;
  MapTileGrouper& tileGrouper;

  set<MapRegion*> regions;
  map<int, MapRegion*> tileMembership;
};

#endif
