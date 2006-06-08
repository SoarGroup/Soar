#ifndef mapquery_h
#define mapquery_h
#include "general.h"

class MapQuery {
  public:
    MapQuery();
    void processMapCommands();
    coordinate getLastResult();
  private:
    coordinate lastResult;
    void findBuildingLoc(BuildingType, coordinate, int);
};


#endif
