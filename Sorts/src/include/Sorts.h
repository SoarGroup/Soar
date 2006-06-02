#ifndef Sorts_H
#define Sorts_H

/* Sorts class

   highest-level class for the middleware
   has pointers to the various interfaces and managers

   most lower-level objects should have a pointer to this
*/

#include "SoarInterface.h"
#include "OrtsInterface.h"
#include "PerceptualGroupManager.h"
//#include "InternalGroupManager.h"
#include "MapManager.h"
#include "FeatureMapManager.h"
//#include "AttackManagerRegistry.h"
//#include "MineManager.h"
class AttackManagerRegistry;
class MineManager;

#include "Satellite.h"
#include "TerrainModule.H"

#include "SortsCanvas.h"

class Sorts {
  public:
    Sorts
    ( SoarInterface*          _SoarIO,
      OrtsInterface*          _OrtsIO,
      PerceptualGroupManager* _pGroupManager, 
  //    InternalGroupManager*   _iGroupManager,
      MapManager*             _mapManager, 
      FeatureMapManager*      _featureMapManager,
      TerrainModule*          _tm,
      Satellite *             _satellite,
      AttackManagerRegistry*  _amr,
      MineManager*            _mineMan,
      pthread_mutex_t*        _mutex )
    {
      SoarIO = _SoarIO;
      OrtsIO = _OrtsIO;
      pGroupManager = _pGroupManager;
      //iGroupManager = _iGroupManager;
      mapManager = _mapManager;
      featureMapManager = _featureMapManager;
      terrainModule = _tm;
      satellite = _satellite;
      amr = _amr;
      mineManager = _mineMan;
      mutex = _mutex;
      catchup = false;
    }

    static SoarInterface*           SoarIO;
    static OrtsInterface*           OrtsIO;
    static PerceptualGroupManager*  pGroupManager;
  //  static InternalGroupManager*    iGroupManager;
    static MapManager*              mapManager;
    static FeatureMapManager*       featureMapManager;
    static TerrainModule*           terrainModule;
    static Satellite*               satellite;
    static AttackManagerRegistry*   amr;
    static MineManager*             mineManager;
    static pthread_mutex_t*         mutex;
    static bool                     catchup;
    static SortsCanvas              canvas;
};



/* The mutex in the Sorts object is now the mutex for everything. Previously, 
   we used mutexes for the shared data structures and Soar IO, but this 
   simplifies things by putting a single mutex that only allows either
   the Soar-event code or the ORTS-event code to run at the same time. I'm
   leaving the old mutexes in SoarInterface.cpp, but commented out- if we
   ever can use actual multithreading, they will allow that (where this scheme
   will not). If that is the case, a mutex needs to be added around the 
   functions in PerceptualGroupManager, since both Soar and ORTS events call them.

   -sw, 4/25/06
*/
   

#endif
