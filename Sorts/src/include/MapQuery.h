#ifndef mapquery_h
#define mapquery_h
#include "general.h"

class MapQuery {
  public:
    MapQuery();
    void processMapCommands();
  private:
    void findBuildingLoc(BuildingType, coordinate, int);
};


#endif
