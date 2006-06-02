#include "Sorts.h"

SoarInterface*          Sorts::SoarIO = NULL;
OrtsInterface*          Sorts::OrtsIO = NULL;
PerceptualGroupManager* Sorts::pGroupManager = NULL;
//InternalGroupManager* Sorts::iGroupManager = NULL;
MapManager*             Sorts::mapManager = NULL;
MineManager*            Sorts::mineManager = NULL;
AttackManagerRegistry*  Sorts::amr = NULL;
FeatureMapManager*      Sorts::featureMapManager = NULL;
TerrainModule*          Sorts::terrainModule = NULL;
Satellite*              Sorts::satellite = NULL;
pthread_mutex_t*        Sorts::mutex = NULL;
bool                    Sorts::catchup;
SortsCanvas             Sorts::canvas;

