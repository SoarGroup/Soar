#include "FeatureMap.h"

/*
  FeatureMap.cpp

  Sam Wintermute, 2006
*/

FeatureMap::FeatureMap() {
  // create the set for each sector
  // initialize the sector iterators
  
  set<SoarGameGroup*> tempSector;
  for (int i=0; i<9; i++) {
    fmSectors.push_back(tempSector);
    fmSectorIterators.push_back(NULL);
    fmCounts.push_back(0);
  }
  
  isPresent = false;
}


SoarGameGroup* FeatureMap::getGroup(int sector) {
  // return a group from the given sector
  // cycle through the qualifying groups
  
  if (fmSectors[sector].size() == 0) {
    return NULL;
  }

  SoarGameGroup* group = *(fmSectorIterators[sector]);

  fmSectorIterators[sector]++;

  if (fmSectorIterators[sector] == fmSectors[sector].end()) {
    fmSectorIterators[sector] = fmSectors[sector].begin();
  }
  return group;
}

void FeatureMap::addGroup(SoarGameGroup* group, int sector, int strength) {
  // add the group to the map
  // note that we don't care about looking for the features
  // since the map itself doesn't know what the features are
  
  // FeatureMapManager handles identifying the sector, too

  //assert(fmSectors[sector].find(group) == fmSectors[sector].end());
  fmSectors[sector].insert(group); 
  fmCounts[sector] += strength;

  if (fmSectors[sector].size() == 1) {
    // if this is the only object in the sector, initialize the iterator
    fmSectorIterators[sector] = fmSectors[sector].begin();
  }
}

void FeatureMap::removeGroup(SoarGameGroup* group, int sector, int strength) {  
  // remove the group (if it exists)

  set<SoarGameGroup*>::iterator it;
  it = fmSectors[sector].find(group);
  assert(it != fmSectors[sector].end());
  
  // if the iterator points to this group, change it.
  if (fmSectorIterators[sector] == it) {
    fmSectorIterators[sector] = fmSectors[sector].begin();
  }
  
  fmSectors[sector].erase(it);
  fmCounts[sector] -= strength;
  assert(fmCounts[sector] >= 0);
}

int FeatureMap::getCount(int sector) {
  return fmCounts[sector];
}

void FeatureMap::clear() {
  // remove all objects, reset the iterators
  for (unsigned int i=0; i<fmSectors.size(); i++) {
    fmSectors[i].clear();
    fmSectorIterators[i] = fmSectors[i].begin();
    fmCounts[i] = 0;
  }

  return;
}

void FeatureMap::setIsPresent(bool in) {
  isPresent = in;
}

bool FeatureMap::getIsPresent() {
  return isPresent;
}
