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
#include "FeatureMapManager.h"
#include "GameActionManager.h"
class AttackManagerRegistry;
class MineManager;

#include "SpatialDB.h"
#include "TerrainModule.H"

#include "SortsCanvas.h"

class Sorts {
  public:
    Sorts
    ( SoarInterface*          _SoarIO,
      OrtsInterface*          _OrtsIO,
      PerceptualGroupManager* _pGroupManager, 
      FeatureMapManager*      _featureMapManager,
      TerrainModule*          _tm,
      SpatialDB *             _spatialDB,
      AttackManagerRegistry*  _amr,
      MineManager*            _mineMan,
      GameActionManager*      _gam,
      pthread_mutex_t*        _mutex )
    {
      SoarIO = _SoarIO;
      OrtsIO = _OrtsIO;
      pGroupManager = _pGroupManager;
      featureMapManager = _featureMapManager;
      terrainModule = _tm;
      spatialDB = _spatialDB;
      amr = _amr;
      mineManager = _mineMan;
      gameActionManager = _gam;
      mutex = _mutex;
      catchup = false;
      frame = -1;
    }

    static SoarInterface*           SoarIO;
    static OrtsInterface*           OrtsIO;
    static PerceptualGroupManager*  pGroupManager;
    static FeatureMapManager*       featureMapManager;
    static TerrainModule*           terrainModule;
    static SpatialDB*               spatialDB;
    static AttackManagerRegistry*   amr;
    static MineManager*             mineManager;
    static GameActionManager*       gameActionManager;
 //   static TerrainManager           terrainManager;
    static pthread_mutex_t*         mutex;
    static bool                     catchup;
    static int                      cyclesSoarAhead;
    static int frame;
    static SortsCanvas              canvas;
};

#endif
