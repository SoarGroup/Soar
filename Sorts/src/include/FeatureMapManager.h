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
#ifndef FMAPMANAGER_H
#define FMAPMANAGER_H

/* 
  FeatureMapManager.h
  SORTS Project
  Sam Wintermute, 2006
*/

#include "FeatureMap.h"

#include <string>
using namespace std;
class PerceptualGroup;
class Sorts;

class FeatureMapManager {
  public:
    FeatureMapManager();
    ~FeatureMapManager();
    void refreshGroup(PerceptualGroup* group);
    void updateSoar();
    PerceptualGroup* getGroup(string mapName, int sector);
    void changeViewWindow(int x, int y, int width);
    //void addGroup(PerceptualGroup* group); use refreshGroup()
    void removeGroup(PerceptualGroup* group);
  private:
    int classifyCenterPointInGrid(PerceptualGroup* group);
    list<FeatureMap* > identifyFeatures(PerceptualGroup* group);
    void clearAll();
    vector<FeatureMap* > fmList;
    map<string, FeatureMap* > stringToFeatureMap;
    int xMin, xMax, yMin, yMax;
    double sectorDim;
  
};

#endif
