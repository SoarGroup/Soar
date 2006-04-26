#ifndef GroupManager_h
#define GroupManager_h

#include "SoarGameObject.h"
#include "SoarGameGroup.h"

class Sorts;

struct ltGroupPtr {
  bool operator()(SoarGameGroup* g1, SoarGameGroup* g2) const {
    int d1 = g1->getDistToFocus();
    int d2 = g2->getDistToFocus();
    if (d1 == d2) {
      // give an arbitrary order if distance is the same
      return ((int)g1 < (int)g2);
    }
    return (d1 < d2);
  }
};

class GroupManager {
  public:
    GroupManager();
    ~GroupManager();

    void initialize();

    void updateVision();
    bool assignActions();
    void processVisionCommands();

    void makeNewGroup(SoarGameObject* object);
    // used by ORTSInterface when it sees a new object- create a group for it
    
    SoarGameGroup* getGroupNear(string type, int owner, int x, int y);
    
    void setSorts(const Sorts* s) {sorts = s;}
 
    
  private:
    int perceptualGroupingRadius;
    int internalGroupingRadius;
    int numObjects;
    int focusX, focusY;
    int centerX, centerY;
    int viewWidth;
    bool ownerGrouping;
    
    void prepareForReGroup();
    void reGroup();
    void generateGroupData();
    void adjustAttention(bool rebuildFeatureMaps);

    void removeGroup(SoarGameGroup*);
    void remakeGroupSet();

    set <pair<string, int> > staleGroupCategories;
    
    // this set is maintained in sorted order, items toward the front
    // are closer to the center of focus, and have priority to go on the
    // input link.
    set <SoarGameGroup*, ltGroupPtr> perceptualGroups;
    set <SoarGameGroup*> internalGroups;

    void setAllCategoriesStale();
    
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
