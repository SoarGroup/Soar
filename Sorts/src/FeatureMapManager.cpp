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
#include "FeatureMapManager.h"
#include "FeatureMap.h"
#include "Sorts.h"

/* 
  FeatureMapManager.cpp

  Sam Wintermute, 2006

*/

#define CLASS_TOKEN "FMMAN"
#define DEBUG_OUTPUT false
#include "OutputDefinitionsUnique.h"

FeatureMapManager::FeatureMapManager() {

  string FMName;
  FeatureMap* FMPointer;

  FMName = "friendly";
  FMPointer = new FeatureMap();
  stringToFeatureMap[FMName] =FMPointer;
  fmList.push_back(FMPointer);
  
  FMName = "enemy";
  FMPointer = new FeatureMap();
  stringToFeatureMap[FMName] =FMPointer;
  fmList.push_back(FMPointer);
  
/*  FMName = "terrain";
  FMPointer = new FeatureMap();
  stringToFeatureMap[FMName] =FMPointer;
  fmList.push_back(FMPointer);
*/  
  FMName = "minerals";
  FMPointer = new FeatureMap();
  stringToFeatureMap[FMName] =FMPointer;
  fmList.push_back(FMPointer);
/*
  FMName = "air-units";
  FMPointer = new FeatureMap();
  stringToFeatureMap[FMName] =FMPointer;
  fmList.push_back(FMPointer);
  
  FMName = "land-units";
  FMPointer = new FeatureMap();
  stringToFeatureMap[FMName] =FMPointer;
  fmList.push_back(FMPointer);
  */
  FMName = "moving-units";
  FMPointer = new FeatureMap();
  stringToFeatureMap[FMName] =FMPointer;
  fmList.push_back(FMPointer);

  FMName = "friendly-workers";
  FMPointer = new FeatureMap();
  stringToFeatureMap[FMName] =FMPointer;
  fmList.push_back(FMPointer);
  
  changeViewWindow(0,0,0);
}

list<FeatureMap*> FeatureMapManager::identifyFeatures(PerceptualGroup* group) {
  // return a list of all the feature maps the group belongs in
  // this is where the feature maps are really determined

  list<FeatureMap*> relevantMaps;
  
  ASSERT(fmList.size() == 5);

  // see the constructor above for where the maps are named and put in the list
  
  // friend
  if (group->isFriendly()) {
    relevantMaps.push_back(fmList[0]);
  }
  // enemy
  if (not group->isFriendly() and not group->isWorld()) {
    relevantMaps.push_back(fmList[1]);
  }
  /*// terrain
  if (group->isWorld()) {
    relevantMaps.push_back(fmList[2]);
  }*/
  // minerals
  if (group->isMinerals()) {
    relevantMaps.push_back(fmList[2]);
  }/*
  // air_units
  if (group->isAirUnits()) {
    relevantMaps.push_back(fmList[4]);
  }
  // land_units
  if (group->isLandUnits()) {
    relevantMaps.push_back(fmList[5]);
  }*/
  // moving_units
  if (group->isMoving()) {
    relevantMaps.push_back(fmList[3]);
  }
  if (group->isFriendlyWorker()) {
    relevantMaps.push_back(fmList[4]);
  }
  
  return relevantMaps;
}

void FeatureMapManager::refreshGroup(PerceptualGroup* group) {
  // if the group is in not in any feature maps, add it (if in view)
  // otherwise, update it
  
 //* comment out this block to not inhibit things
  if (group->getInSoar()) {
    // inhibit this group from the feature maps- remove it if present
    removeGroup(group);
    return;
  }
  //*/

  int oldSectorNumber = group->getFMSector();
  int newSectorNumber = classifyCenterPointInGrid(group);
  int oldStrength =  group->getFMFeatureStrength();
  int newStrength = group->getSize();

 // if (oldSectorNumber == newSectorNumber and
 //     oldStrength == newStrength) return;
 // we must still add/remove it if the properties changed (ie. stopped moving)
  
  list<FeatureMap*> newFeatureMaps = identifyFeatures(group);
  list<FeatureMap*> oldFeatureMaps;
  list<FeatureMap*>::iterator it;
  
  if (oldSectorNumber != -1) {
    oldFeatureMaps = group->getFMaps();
    //msg << "removing grp " << (int)group << " from " << oldFeatureMaps.size() << " fms\n";
    for (it=oldFeatureMaps.begin(); it != oldFeatureMaps.end(); it++ ) {
      (*it)->removeGroup(group, oldSectorNumber, oldStrength);
    }
  }

  if (newSectorNumber != -1) {
    //msg << "adding grp " << (int)group << " to " << newFeatureMaps.size() << " fms\n";
    for (it=newFeatureMaps.begin(); it != newFeatureMaps.end(); it++ ) {
      (*it)->addGroup(group, newSectorNumber, newStrength);
    }
  }
  group->setFMaps(newFeatureMaps);
  group->setFMSector(newSectorNumber);
  group->setFMFeatureStrength(newStrength);
    
}
/* refresh can handle this fine..
void FeatureMapManager::addGroup(PerceptualGroup* group) {
  // add this group to all feature maps it belongs in
  int sectorNumber;
  int strength = group->getSize();

  sectorNumber = classifyCenterPointInGrid(group);
  group->setFMSector(sectorNumber);
  
  // if the group is not in view, return
  if (sectorNumber == -1) return;
  
  group->setFMFeatureStrength(strength);
  
  list<FeatureMap*> featureMaps = identifyFeatures(group);
  list<FeatureMap*>::iterator it;
  for (it=featureMaps.begin(); it != featureMaps.end(); it++ ) {
    (*it)->addGroup(group, sectorNumber, strength);
  }
}
*/
void FeatureMapManager::removeGroup(PerceptualGroup* group) {
  // remove this group from every feature map it is in
  int sectorNumber;
  int strength = group->getFMFeatureStrength();

  sectorNumber = group->getFMSector();
  
  // if the group is not in view, return
  if (sectorNumber == -1) return;

  list<FeatureMap*> featureMaps = group->getFMaps();
  list<FeatureMap*>::iterator it;
  for (it=featureMaps.begin(); it != featureMaps.end(); it++ ) {
    (*it)->removeGroup(group, sectorNumber, strength);
  }

  group->setFMSector(-1);
}

void FeatureMapManager::updateSoar() {
  map<string, FeatureMap*>::iterator it = stringToFeatureMap.begin();
  msg << "feature map info:\n";
  while (it != stringToFeatureMap.end()) {
    if (it->second->getIsStale()) {
      it->second->setIsStale(false);
      if (it->second->getIsPresent() == false) {
        Sorts::SoarIO->addFeatureMap(it->second, it->first);
        it->second->setIsPresent(true);
      }

      Sorts::SoarIO->refreshFeatureMap(it->second, it->first);
      
    ///*
      msg << "map " << it->first << ":\n"
       << "\t" << it->second->getCount(0)
       << "\t" << it->second->getCount(1)
       << "\t" << it->second->getCount(2)
       << endl
       << "\t" << it->second->getCount(3)
       << "\t" << it->second->getCount(4)
       << "\t" << it->second->getCount(5)
       << endl
       << "\t" << it->second->getCount(6)
       << "\t" << it->second->getCount(7)
       << "\t" << it->second->getCount(8)
       << endl;
    //*/
    }
    it++;
  }
}


PerceptualGroup* FeatureMapManager::getGroup(string mapName, int sector) {
  // return a group from the map given in the sector given
  // return NULL if the sector is empty in that map
  
  map<string, FeatureMap*>::iterator it = stringToFeatureMap.find(mapName);
  ASSERT(it != stringToFeatureMap.end());

  return it->second->getGroup(sector);
}

void FeatureMapManager::changeViewWindow(int x, int y, int width) {
  // the PerceptualGroupManager is responsible for re-populating the maps when this changes!
  // (FMM does not know anything about groups not in view)
  int half = (int) (width / 2);
  xMin = x - half;
  xMax = x + half;
  yMin = y - half;
  yMax = y + half;

  sectorDim = (double)width / 3.0;

  clearAll();
}

int FeatureMapManager::classifyCenterPointInGrid(PerceptualGroup* group) {
  // determine which sector a given point is in
  
  int x,y;
  group->getCenterLoc(x,y);
  
  x -= xMin;

  y -= yMin;

  if (sectorDim == 0) {
    return -1;
  }
  
  int xInterval = (int)(x / sectorDim);
  int yInterval = (int)(y / sectorDim);

  // check if point is out of bounds
  if (xInterval > 2) return -1;
  if (yInterval > 2) return -1;
  if (xInterval < 0) return -1;
  if (yInterval < 0) return -1;

  if (xInterval == 0 and yInterval == 0) {
    return 0;
  }
  else if (xInterval == 1 and yInterval == 0) {
    return 1;
  }
  else if (xInterval == 2 and yInterval == 0) {
    return 2;
  }
  else if (xInterval == 0 and yInterval == 1) {
    return 3;
  }
  else if (xInterval == 1 and yInterval == 1) {
    return 4;
  }
  else if (xInterval == 2 and yInterval == 1) {
    return 5;
  }
  else if (xInterval == 0 and yInterval == 2) {
    return 6;
  }
  else if (xInterval == 1 and yInterval == 2) {
    return 7;
  }
  else if (xInterval == 2 and yInterval == 2) {
    return 8;
  }
  else {
    ASSERT(false);
  }

  return -1;

}
  
void FeatureMapManager::clearAll() {
 
  // the group flags must also be cleared, do that in group manager!
  vector<FeatureMap*>::iterator it = fmList.begin();
  while (it != fmList.end()) {
    (*it)->clear();
    it++;
  }

  return;
}

FeatureMapManager::~FeatureMapManager() {
  // delete the feature maps
  vector<FeatureMap*>::iterator it = fmList.begin();
  while (it != fmList.end()) {
    delete (*it);
    it++;
  }
}

