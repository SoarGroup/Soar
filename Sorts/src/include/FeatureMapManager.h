#ifndef FMAPMANAGER_H
#define FMAPMANAGER_H

/* 
  FeatureMapManager.h
  SORTS Project
  Sam Wintermute, 2006
*/

#include "FeatureMap.h"
#include "SoarInterface.h"

#include <string>
using namespace std;
class SoarGameGroup;
//class SoarInterface;

class FeatureMapManager {
  public:
    FeatureMapManager(SoarInterface* _sio);
    ~FeatureMapManager();
    void refreshGroup(SoarGameGroup* group);
    void updateSoar();
    SoarGameGroup* getGroup(string mapName, int sector);
    void changeViewWindow(int x, int y, int width);
    void addGroup(SoarGameGroup* group);
    void removeGroup(SoarGameGroup* group);
  private:
    int classifyCenterPointInGrid(SoarGameGroup* group);
    list<FeatureMap* > identifyFeatures(SoarGameGroup* group);
    void clearAll();
    vector<FeatureMap* > fmList;
    map<string, FeatureMap* > stringToFeatureMap;
    int xMin, xMax, yMin, yMax;
    double sectorDim;

    SoarInterface* SoarIO;
};

#endif
