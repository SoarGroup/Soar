#ifndef Sorts_H
#define Sorts_H

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
    Sorts
    ( SoarInterface*      _SoarIO,
      OrtsInterface*      _OrtsIO,
      GroupManager*       _groupManager, 
      MapManager*         _mapManager, 
      FeatureMapManager*  _featureMapManager)
    {
      SoarIO = _SoarIO;
      OrtsIO = _OrtsIO;
      groupManager = _groupManager;
      mapManager = _mapManager;
      featureMapManager = _featureMapManager;
    }

    SoarInterface* SoarIO;
    OrtsInterface* OrtsIO;
    GroupManager* groupManager;
    MapManager* mapManager;
    FeatureMapManager* featureMapManager;
};


#endif
