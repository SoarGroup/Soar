#ifndef MapManager_H_
#define MapManager_H_

#include<set>

#include "Map.H"
#include "ScriptObj.H"
#include "GameTile.H"
#include "Global.H"
#include "Vector.H"

#include "MapTileGrouper.h"

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

  // attention shifting functions
  // these will modify the Soar input link (via SoarInterface)
  void focus(int region);
  void defocus();

  void getRegionsOccupied(SoarGameGroup* group, list<MapRegion*> &regions);
  
private:
  const Map<GameTile>& gameMap;
  int tilePoints;
  MapTileGrouper& tileGrouper;

  list<MapRegion*> regions;
  map<int, MapRegion*> tileMembership;
};

#endif
