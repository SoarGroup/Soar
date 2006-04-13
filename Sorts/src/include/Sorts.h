#ifndef middleware_h
#define middleware_h

/* Sorts class

   highest-level class for the middleware
   has pointers to the various interfaces and managers

   most lower-level objects should have a pointer to this
*/

#include "include/SoarInterface.h"
#include "include/OrtsInterface.h"
#include "include/GroupManager.h"
#include "include/MapManager.h"
#include "include/FeatureMapManager.h"
/*
class SoarInterface;
class OrtsInterface;
class GroupManager;
class MapManager;
class FeatureMapManager;
*/
class Sorts {
  public:
    Sorts(SoarInterface*, OrtsInterface*, 
               GroupManager*, MapManager*, FeatureMapManager*);
    SoarInterface* getSoarInterface();
    OrtsInterface* getOrtsInterface();
    GroupManager* getGroupManager();
    MapManager* getMapManager();
    FeatureMapManager* getFeatureMapManager();
  private:
    SoarInterface* SoarIO;
    OrtsInterface* ORTSIO;
    GroupManager* gm;
    MapManager* mm;
    FeatureMapManager* fmm;
};




#endif
