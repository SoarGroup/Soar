#ifndef SoarGameGroup_h
#define SoarGameGroup_h

#include <list>
#include <set>
#include <string>
#include "constants.h"
#include "SoarAction.h"

#ifdef DEBUG_GROUPS
#include "FakeSoarGameObject.h"
#else
#include "SoarGameObject.h"
#endif

using namespace std;
// inside SoarInterface.h too
typedef list<pair<string, int> > groupPropertyList;

//class SoarGameObject{}; // TEMPORARY

class SoarGameGroup {
  public:
    SoarGameGroup(SoarGameObject* unit);
    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    void updateStats(bool saveProps);
    bool assignAction(SoarActionType type, list<int> params,
                      SoarGameObject* target);
    bool isEmpty();

    list<SoarGameObject*> getMembers(); 
  
    void mergeTo(SoarGameGroup* target);
    bool getStale();
    void setStale(bool val);
    bool getStaleInSoar();
    void setStaleInSoar(bool val);
    groupPropertyList getProps();
    void setType(int inType);
    int getType();
    int getSize();
    SoarGameObject* getCenterMember();
  private:
    set <SoarGameObject*> members;
    // int capabilities; // get from unit capabilities
    double statistics[GP_NUM_STATS]; 
    groupPropertyList propList;
    bool stale;
    bool staleInSoar;
    // stale means the internal statistics haven't been updated-
    // something moved, or new members were added.

    // staleInSoar means that internal statistics are correct, but Soar
    // needs a refresh-- either refresh it or remove it from Soar ASAP

    int type;
    string typeName;
    
    SoarGameObject* centerMember;
};
#endif
