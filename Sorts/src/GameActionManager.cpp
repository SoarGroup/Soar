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
#include "GameActionManager.h"
#include "Sorts.h"
#include "Rectangle.h"

#define MAX_BUILDING_PLACE_TRIES 70

#define CLASS_TOKEN "GAMAN"
#define DEBUG_OUTPUT true 
#include "OutputDefinitionsUnique.h"

GameActionManager::GameActionManager() 
  : lastResult(0,0) {
    mineralBuffer = 0;
}

void GameActionManager::processGameCommands() {
#ifndef GAME_ONE
  list<GameAction> actions;
  Sorts::SoarIO->getNewGameActions(actions);
  for (list<GameAction>::iterator i = actions.begin();
                                 i != actions.end();
                                 i++) {
    if (i->type == GA_FIND_BUILDING_LOC) {
      findBuildingLoc(i->building, i->nearLocation, i->intValue);
    }
    else if (i->type == GA_SET_MINERAL_BUFFER) {
      setMineralBuffer(mineralBuffer + i->intValue);
    }
    else if (i->type == GA_CLEAR_MINERAL_BUFFER) {
      mineralBuffer = 0;
    }
    else {
      ASSERT(false);
    }
  }
#endif
}

void GameActionManager::findBuildingLoc(BuildingType building, coordinate nearLocation, 
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
  else {
    buildingRadius = buildingWidth = buildingHeight = 0;
    ASSERT(false);
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
        && not Sorts::spatialDB->hasTerrainCollision(&rect)) {
      found = true;
      msg << "found location: " << newLoc << endl;
      break;
    }
    else {
      dbg << "bad location: " << newLoc << endl;
      dbg << "rect: " << rect << endl;
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
 
coordinate GameActionManager::getLastResult() {
  return lastResult;
}

void GameActionManager::setMineralBuffer(int val) {
  mineralBuffer = val;
  msg << "mineral buffer: " << val << endl;
  Sorts::SoarIO->updateMineralBuffer(mineralBuffer);
}

