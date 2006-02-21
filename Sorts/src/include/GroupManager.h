#ifndef GroupManager_h
#define GroupManager_h

#include "SoarGameObject.h"
#include "MapManager.h"

#ifdef DEBUG_GROUPS
#include "FakeSoarInterface.h"
#else
class SoarInterface;
#endif
class OrtsInterface;

class GroupManager {
  public:
    GroupManager(SoarInterface* si, MapManager* _mapManager) 
    : SoarIO(si), mapManager(_mapManager) 
    { }

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
    OrtsInterface* ORTSIO;

    MapManager *mapManager;
};

struct objectGroupingStruct {
  SoarGameObject* object;
  SoarGameGroup* group;
  bool assigned;
  int x,y;
};

#endif // GroupManager_h
