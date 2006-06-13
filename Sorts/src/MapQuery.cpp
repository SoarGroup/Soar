#include "MapQuery.h"
#include "Sorts.h"
#include "Rectangle.h"

#define MAX_BUILDING_PLACE_TRIES 70
#define msg cout << "MQ: "

MapQuery::MapQuery() 
  : lastResult(0,0) {
}

void MapQuery::processMapCommands() {
  list<MapAction> actions;
  Sorts::SoarIO->getNewMapActions(actions);
  for (list<MapAction>::iterator i = actions.begin();
                                 i != actions.end();
                                 i++) {
    if (i->type == MA_FIND_BUILDING_LOC) {
      findBuildingLoc(i->building, i->nearLocation, i->minDistance);
    }
    else {
      assert(false);
    }
  }
}

void MapQuery::findBuildingLoc(BuildingType building, coordinate nearLocation, 
                          int minDistance) {
  int buildingRadius, buildingWidth, buildingHeight;
  if (building == BARRACKS) {
    buildingWidth = 64;
    buildingHeight = 48;
    buildingRadius = 38;
  }
  else if (building == CONTROL_CENTER) {
    buildingWidth = 64;
    buildingHeight = 64;
    buildingRadius = 44;
  }
  else if (building == FACTORY) {
    buildingWidth = 64;
    buildingHeight = 48;
    buildingRadius = 38;
  }

  minDistance += buildingRadius;

  double angle;
  coordinate newLoc;
  bool found = false;
  int mapXdim = Sorts::OrtsIO->getMapXDim();
  int mapYdim = Sorts::OrtsIO->getMapYDim();
  
  for (int i=0; i<MAX_BUILDING_PLACE_TRIES; i++) {
    angle = rand() % 6283; // approx 1000pi
    angle /= 1000.0;

    newLoc.x = nearLocation.x + (int)(minDistance*cos(angle));
    newLoc.y = nearLocation.y + (int)(minDistance*sin(angle));

    Rectangle rect(newLoc.x, newLoc.y, buildingWidth, buildingHeight, true);
    

    if (rect.xmax < mapXdim
        && rect.xmin > 0
        && rect.ymax < mapYdim 
        && rect.ymin > 0
        && not Sorts::spatialDB->hasObjectCollision(&rect)
        && not Sorts::spatialDB->hasTerrainCollision(rect)) {
      found = true;
      msg << "found location: " << newLoc << endl;
      break;
    }
    else {
      msg << "bad location: " << newLoc << endl;
    }
    //found = true; break;
  }

  if (found) {
    lastResult = newLoc;
    Sorts::SoarIO->updateQueryResult("locate-building", newLoc.x, newLoc.y);
    Sorts::pGroupManager->updateQueryDistances();
  }
  else {
    msg << "no location found! "<< endl;
    Sorts::SoarIO->updateQueryResult("locate-building", -1, -1);
  }
} 
 
coordinate MapQuery::getLastResult() {
  return lastResult;
}
