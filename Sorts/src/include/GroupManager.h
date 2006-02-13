#ifndef GroupManager_h
#define GroupManager_h

#ifdef DEBUG_GROUPS
#include "FakeSoarInterface.h"
#else
#include "SoarInterface.h"
#endif
#include "OrtsInterface.h"

class GroupManager {
  public:
    GroupManager(SoarInterface* si) : SoarIO(si) { };
    ~GroupManager();

    void updateWorld();
    bool assignActions();

    void addGroup(SoarGameObject* object);
    // used by ORTSInterface when it sees a new object- create a group for it
    void setORTSIO(OrtsInterface* oio);
    
  private:
    void reGroup();
    void refreshGroups(bool);
    void adjustAttention();

    SoarInterface* SoarIO;

    set <pair<string, int> > staleGroupCategories;
    
    list <SoarGameGroup*> groupsInFocus;
    list <SoarGameGroup*> groupsNotInFocus;
    OrtsInterface ORTSIO;
};

struct objectGroupingStruct {
  SoarGameObject* object;
  SoarGameGroup* group;
  bool assigned;
  int x,y;
};

#endif // GroupManager_h
