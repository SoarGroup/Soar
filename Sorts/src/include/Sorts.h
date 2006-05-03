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

#include "TerrainModule.H"
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
      FeatureMapManager*  _featureMapManager,
      TerrainModule*            _tm,
      pthread_mutex_t*    _mutex)
    {
      SoarIO = _SoarIO;
      OrtsIO = _OrtsIO;
      groupManager = _groupManager;
      mapManager = _mapManager;
      featureMapManager = _featureMapManager;
      terrainModule = _tm;
      mutex = _mutex;
      catchup = false;
    }

    SoarInterface* SoarIO;
    OrtsInterface* OrtsIO;
    GroupManager* groupManager;
    MapManager* mapManager;
    FeatureMapManager* featureMapManager;
    TerrainModule* terrainModule;
    pthread_mutex_t* mutex;
    bool catchup;
};

/* The mutex in the Sorts object is now the mutex for everything. Previously, 
   we used mutexes for the shared data structures and Soar IO, but this 
   simplifies things by putting a single mutex that only allows either
   the Soar-event code or the ORTS-event code to run at the same time. I'm
   leaving the old mutexes in SoarInterface.cpp, but commented out- if we
   ever can use actual multithreading, they will allow that (where this scheme
   will not). If that is the case, a mutex needs to be added around the 
   functions in GroupManager, since both Soar and ORTS events call them.

   -sw, 4/25/06
*/
   

#endif
