#ifndef SoarGameGroup_h
#define SoarGameGroup_h

#include <list>
#include <set>
#include "constants.h"

class SoarGameGroup {
  public:
    SoarGameGroup(SoarGameObject* unit);
    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    groupPropertyList updateStats();
    bool assignAction(Action);
    void getUnits(list<SoarGameObject*> unitList);
    void mergeTo(SoarGameGroup* target);
    bool getStale();
    void setStale(bool val);
  private:
    set <SoarGameObject*> members;
    // int capabilities; // get from unit capabilities
    double[GP_NUM_STATS] statistics; 
    bool stale;
};
#endif
