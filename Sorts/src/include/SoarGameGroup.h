#ifndef SoarGameGroup_h
#define SoarGameGroup_h

#include <list>
#include <set>
#include <string>
#include "constants.h"
#include "SoarAction.h"
#include "OrtsInterface.h"

#ifdef DEBUG_GROUPS
#include "FakeSoarGameObject.h"
#else
#include "SoarGameObject.h"
#endif

using namespace std;
// inside SoarInterface.h too
typedef list<pair<string, int> > groupPropertyList;
struct groupPropertyStruct {
  list<pair<string, int> > stringIntPairs;
  list<pair<string, string> > stringStringPairs;
};

//class SoarGameObject{}; // TEMPORARY

class SoarGameGroup {
  public:
    SoarGameGroup(SoarGameObject* unit, OrtsInterface* in_ORTSIO);
    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    void updateStats(bool saveProps);
    bool assignAction(SoarActionType type, list<int> params,
                      list<SoarGameObject*> targets);
    bool isEmpty();

    list<SoarGameObject*> getMembers(); 
  
    void mergeTo(SoarGameGroup* target);
    bool getStale();
    void setStale();
    void setStale(bool val);
    bool getStaleInSoar();
    void setStaleInSoar(bool val);
    groupPropertyStruct getSoarData();
    //void setType(int inType);
    pair<string, int> getCategory();
    int getSize();
    SoarGameObject* getCenterMember();
    SoarGameObject* getNextMember();

    // get the player number that owns this group
    int getOwner();
    bool getFriendly();
  private:
    set <SoarGameObject*> members;
    // int capabilities; // get from unit capabilities
    double statistics[GP_NUM_STATS]; 
    groupPropertyStruct soarData;
    bool stale;
    bool staleInSoar;
    // stale means the internal statistics haven't been updated-
    // something moved, or new members were added.

    // staleInSoar means that internal statistics are correct, but Soar
    // needs a refresh-- either refresh it or remove it from Soar ASAP

    int type;
    string typeName;
    
    SoarGameObject* centerMember;
    SoarGameObject* currentMember; // for getNextMember functionality

    int owner;
    bool friendly;
    OrtsInterface ORTSIO;
};
#endif
