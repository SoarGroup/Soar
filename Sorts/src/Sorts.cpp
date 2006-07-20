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
#include "Sorts.h"

SoarInterface*          Sorts::SoarIO = NULL;
OrtsInterface*          Sorts::OrtsIO = NULL;
PerceptualGroupManager* Sorts::pGroupManager = NULL;
MineManager*            Sorts::mineManager = NULL;
AttackManagerRegistry*  Sorts::amr = NULL;
FeatureMapManager*      Sorts::featureMapManager = NULL;
TerrainModule*          Sorts::terrainModule = NULL;
SpatialDB*              Sorts::spatialDB = NULL;
GameActionManager*      Sorts::gameActionManager = NULL;
//TerrainManager          Sorts::terrainManager;
pthread_mutex_t*        Sorts::mutex = NULL;
int                     Sorts::cyclesSoarAhead = 0;
bool                    Sorts::catchup = false;
int                     Sorts::frame = -1;
#ifdef USE_CANVAS 
SortsCanvas             Sorts::canvas;
#endif
