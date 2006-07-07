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
#ifndef FEATUREMAP_H
#define FEATUREMAP_H
/*
  FeatureMap.h

  Sam Wintermute, 2006
*/

#include <list>
#include "PerceptualGroup.h"
using namespace std;


//class PerceptualGroup;

class FeatureMap {
  public:
    FeatureMap();
    ~FeatureMap() { }
    PerceptualGroup* getGroup(int sector);
    // return a group from the given sector
    // cycle through the qualifying groups

    void addGroup(PerceptualGroup* group, int sector, int strength);
    // add the group to the map
    // note that we don't care about looking for the features
    // since the map itself doesn't know what the features are
  
    void removeGroup(PerceptualGroup* group, int sector, int strength);  
    // remove the group (if it exists)

    int getCount(int);

    void clear();
    // remove all objects

    void setIsPresent(bool);
    bool getIsPresent();

    void setIsStale(bool);
    bool getIsStale();
  private:
    vector<set<PerceptualGroup*> > fmSectors;
    vector<set<PerceptualGroup*>::iterator> fmSectorIterators;
    vector<int> fmCounts;
    bool isPresent; // true if FM is in Soar
    bool isStale;
};
#endif
