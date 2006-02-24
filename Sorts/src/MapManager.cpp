#include "MapManager.h"

#include "SoarInterface.h"
#include "SoarGameGroup.h"
#include "Rectangle.h"

using namespace std;

MapManager::MapManager
( const Map<GameTile>& _gameMap, 
  int                  _tilePoints, 
  MapTileGrouper&      _tileGrouper,
  SoarInterface*       _soarInterface )

:gameMap(_gameMap), 
 tilePoints(_tilePoints), 
 tileGrouper(_tileGrouper),
 soarInterface(_soarInterface)
{ }


void MapManager::getRegionsOccupied
( SoarGameGroup* group, 
  list<MapRegion*>& regionsOccupied ) 
{
  Rectangle groupBBox = group->getBoundingBox();
  if (groupBBox.area() == 0) {
    // group with single unit
    MapRegion* r = getRegionUnder(groupBBox.xmin, groupBBox.ymin);
    // there had better be a region under a group you can see
    assert(r != NULL);
    regionsOccupied.push_back(r);
  }
  else {
    for(set<MapRegion*>::iterator 
        i  = regions.begin(); 
        i != regions.end(); 
        i++) 
    {
      if((*i)->intersects(groupBBox)) {
        regionsOccupied.push_back(*i);
      }
    }
  }
}

MapRegion* MapManager::getRegionUnder(int x, int y) {
  int tileIndex = gameMap.xy2ind(x / tilePoints, y / tilePoints);
  if (tileMembership.find(tileIndex) == tileMembership.end()) {
    return (*tileMembership.begin()).second;
    //return NULL;
  }
  // should fix this so it doesn't search twice..
  return (*tileMembership.find(tileIndex)).second;
}

void MapManager::addExploredTiles(Vector<sint4> newTiles) {
  for(Vector<sint4>::iterator i = newTiles.begin(); i != newTiles.end(); i++) {
    // if this tile was just encountered, we shouldn't know about it yet
    assert(tileMembership.find(*i) == tileMembership.end());

    MapRegion* assignedRegion = tileGrouper.groupTile(*i);
    tileMembership[*i] = assignedRegion;

    if (regions.find(assignedRegion) == regions.end()) {
      regions.insert(tileMembership[*i]);
      soarInterface->addMapRegion(assignedRegion);
    }
    else {
      soarInterface->refreshMapRegion(assignedRegion);
    }
  }
}

void MapManager::addBoundaries(Vector<ScriptObj*> boundaries) {
  for(Vector<ScriptObj*>::iterator i = boundaries.begin(); i != boundaries.end(); i++) 
  {
    GameObj* gob = (GameObj*) *i;
    int tileMinX = *gob->sod.x1 / tilePoints;
    int tileMinY = *gob->sod.y1 / tilePoints;
    int tileMaxX = *gob->sod.x2 / tilePoints;
    int tileMaxY = *gob->sod.y2 / tilePoints;

    set<MapRegion*> processedRegions;
    for(int x = tileMinX; x < tileMaxX; x++) {
      for(int y = tileMinY; y < tileMaxY; y++) {
        int tileIndex = gameMap.xy2ind(x, y);
        map<int, MapRegion*>::iterator i = tileMembership.find(tileIndex);
        if (i != tileMembership.end() && 
            processedRegions.find((*i).second) == processedRegions.end()) 
        {
          (*i).second->addBoundary(gob);
        }
      }
    }
  }
}
