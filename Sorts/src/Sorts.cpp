#include "Sorts.h"

SoarInterface*          Sorts::SoarIO = NULL;
OrtsInterface*          Sorts::OrtsIO = NULL;
PerceptualGroupManager* Sorts::pGroupManager = NULL;
MapManager*             Sorts::mapManager = NULL;
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
