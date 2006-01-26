#ifndef SoarGameGroup_h
#define SoarGameGroup_h

#include <list>
#include <set>
#include "constants.h"
#include "SoarGameObject.h"
//#include "SoarInterface.h"

using namespace std;
// inside SoarInterface.h too
typedef list<pair<string, int> > groupPropertyList;

//class SoarGameObject{}; // TEMPORARY

class SoarGameGroup {
  public:
    SoarGameGroup(SoarGameObject* unit);
    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    groupPropertyList updateStats();
//    bool assignAction(SoarAction);
  // need to rethink SoarAction
  void getUnits(list<SoarGameObject*> unitList);
    void mergeTo(SoarGameGroup* target);
    bool getStale();
    void setStale(bool val);
  private:
    set <SoarGameObject*> members;
    // int capabilities; // get from unit capabilities
    double statistics[GP_NUM_STATS]; 
    bool stale;
};
#endif
