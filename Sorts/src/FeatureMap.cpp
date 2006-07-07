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
#include "FeatureMap.h"

/*
  FeatureMap.cpp

  Sam Wintermute, 2006
*/

FeatureMap::FeatureMap() {
  // create the set for each sector
  // initialize the sector iterators
  
  set<PerceptualGroup*> tempSector;
  for (int i=0; i<9; i++) {
    fmSectors.push_back(tempSector);
    fmSectorIterators.push_back(NULL);
    fmCounts.push_back(0);
  }
  
  isPresent = false;
  isStale = true;
}


PerceptualGroup* FeatureMap::getGroup(int sector) {
  // return a group from the given sector
  // cycle through the qualifying groups
  
  if (fmSectors[sector].size() == 0) {
    return NULL;
  }

  PerceptualGroup* group = *(fmSectorIterators[sector]);

  fmSectorIterators[sector]++;

  if (fmSectorIterators[sector] == fmSectors[sector].end()) {
    fmSectorIterators[sector] = fmSectors[sector].begin();
  }

  return group;
}

void FeatureMap::addGroup(PerceptualGroup* group, int sector, int strength) {
  // add the group to the map
  // note that we don't care about looking for the features
  // since the map itself doesn't know what the features are
  
  // FeatureMapManager handles identifying the sector, too

  //assert(fmSectors[sector].find(group) == fmSectors[sector].end());
  isStale = true;
  fmSectors[sector].insert(group); 
  fmCounts[sector] += strength;

  if (fmSectors[sector].size() == 1) {
    // if this is the only object in the sector, initialize the iterator
    fmSectorIterators[sector] = fmSectors[sector].begin();
  }
}

void FeatureMap::removeGroup(PerceptualGroup* group, int sector, int strength) {  
  // remove the group (if it exists)

  isStale = true;
  set<PerceptualGroup*>::iterator it;
  it = fmSectors[sector].find(group);
  assert(it != fmSectors[sector].end());
  
  fmSectors[sector].erase(it);
  
  // if the iterator points to this group, change it.
  if (fmSectorIterators[sector] == it) {
    fmSectorIterators[sector] = fmSectors[sector].begin();
  }
  
  fmCounts[sector] -= strength;
  assert(fmCounts[sector] >= 0);
}

int FeatureMap::getCount(int sector) {
  return fmCounts[sector];
}

void FeatureMap::clear() {
  isStale = true;
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

void FeatureMap::setIsStale(bool val) {
  isStale = val;
}

bool FeatureMap::getIsStale() {
  return isStale;
}
