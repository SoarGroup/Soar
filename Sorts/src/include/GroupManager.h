#ifndef GroupManager_h
#define GroupManager_h

#include "FakeSoarInterface.h"

class GroupManager {
  public:
    GroupManager(SoarInterface* si) : SoarIO(si) { };
    ~GroupManager();

    void updateWorld();
    bool assignActions();

    void addGroup(SoarGameObject* object);
    // used by ORTSInterface when it sees a new object- create a group for it
    
  private:
    void reGroup();
    void refreshGroups(bool);
    void adjustAttention();

    SoarInterface* SoarIO;

    set <int> staleGroupTypes;
    
    list <SoarGameGroup*> groupsInFocus;
    list <SoarGameGroup*> groupsNotInFocus;
};

#endif // GroupManager_h
