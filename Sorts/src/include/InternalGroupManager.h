#ifndef InternalGroupManager_h
#define InternalGroupManager_h

#include "SoarGameObject.h"
#include "PerceptualGroup.h"
#include "SoarInterface.h"

class Sorts;


class InternalGroupManager {
  public:
    InternalGroupManager();
    ~InternalGroupManager();

    void updateGroups();

    void makeNewGroup(SoarGameObject* object);
    // used by ORTSInterface when it sees a new object- create a group for it
    
    InternalGroup* getGroupNear(string type, int owner, int x, int y);
    
  private:
    int internalGroupingRadius;
    
    void prepareForReGroup();
    void reGroup();
    void generateGroupData();

    // don't care about order of internal groups
    list <InternalGroup*> internalGroups;

    set <pair<string, int> > staleGroupCategories;
    

    void setAllPerceptualCategoriesStale();
    bool internallyGrouped(SoarGameObject* sgo);
    
};

struct internalGroupingStruct {
  SoarGameObject* object;
  InternalGroup* group;
  bool assigned;
  int x,y;
};

#endif // InternalGroupManager_h
