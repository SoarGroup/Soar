#include "Sorts.h"

SoarInterface* Sorts::SoarIO = NULL;
OrtsInterface* Sorts::OrtsIO = NULL;
GroupManager* Sorts::groupManager = NULL;
MapManager* Sorts::mapManager = NULL;
FeatureMapManager* Sorts::featureMapManager = NULL;
TerrainModule* Sorts::terrainModule = NULL;
Satellite *Sorts::satellite = NULL;
pthread_mutex_t* Sorts::mutex = NULL;
bool Sorts::catchup;


