#ifndef gameactionmanager_h
#define gameactionmanager_h
#include "general.h"

class GameActionManager {
  public:
    GameActionManager();
    void processGameCommands();
    coordinate getLastResult();
    int getMineralBuffer() { return mineralBuffer; }
    void setMineralBuffer(int val);
  private:
    coordinate lastResult;
    void findBuildingLoc(BuildingType, coordinate, int);
    int mineralBuffer;
};


#endif
