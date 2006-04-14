#ifndef GroupManager_h
#define GroupManager_h

#include "SoarGameObject.h"

class Sorts;

class GroupManager {
  public:
    GroupManager();
    ~GroupManager();

    void updateVision();
    bool assignActions();
    void processVisionCommands();

    void addGroup(SoarGameObject* object);
    // used by ORTSInterface when it sees a new object- create a group for it
    
    SoarGameGroup* getGroupNear(string type, int owner, int x, int y);
    
    void setSorts(const Sorts* s) {sorts = s;}
    
  private:
    void prepareForReGroup();
    void reGroup();
    void generateGroupData();
    void adjustAttention();
    void updateFeatureMaps(bool refreshAll);

    void removeGroup(SoarGameGroup*);

    set <pair<string, int> > staleGroupCategories;
    
    list <SoarGameGroup*> groupsInFocus;
    list <SoarGameGroup*> groupsNotInFocus;
    
    const Sorts* sorts;
};

struct objectGroupingStruct {
  SoarGameObject* object;
  SoarGameGroup* group;
  bool assigned;
  bool oldGroup;
  int x,y;
};

#endif // GroupManager_h
