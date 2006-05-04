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
