#ifndef GroupManager_h
#define GroupManager_h

#include "SoarGameObject.h"
#include "PerceptualGroup.h"
#include "SoarInterface.h"

class Sorts;


struct ltGroupPtr {
  bool operator()(PerceptualGroup* g1, PerceptualGroup* g2) const {
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
    
    InternalGroup* getGroupNear(string type, int owner, int x, int y);
    
    void setSorts(const Sorts* s) {sorts = s;}
 
    
  private:
    int internalGroupingRadius;
    
    VisionParameterStruct visionParams;
    /* in general.h:
    struct VisionParameterStruct {
      int centerX;
      int centerY;
      int viewWidth;
      int focusX;
      int focusY;
      bool ownerGrouping;
      int numObjects;
      int groupingRadius;
    };*/
    
    void prepareForReGroup();
    void reGroupInternal();
    void reGroupPerceptual();
    void generatePerceptualGroupData();
    void generateInternalGroupData();
    void adjustAttention(bool rebuildFeatureMaps);

    void removePerceptualGroup(PerceptualGroup*);
    void remakeGroupSet();

    // GroupManager must handle two sets of groups- perceptual and internal
    // perceptual groups are input to Soar, directly and in feature maps
    // Soar can control how they are created, adjusting the grouping radius
    // and allowing grouping by owner or not.

    // internal groups are used by the FSMs. the FSMs use groups to find nearby
    // objects of a given type, and to do basic load balancing when, 
    // for example, many workers mine the same mineral patch. 

    // the internal groups work best at a set grouping radius, and would not 
    // be useful with grouping by owner, so they must be separatly maintained
    
    // this set is maintained in sorted order, items toward the front
    // are closer to the center of focus, and have priority to go on the
    // input link.
    set <PerceptualGroup*, ltGroupPtr> perceptualGroups;
    
    // don't care about order of internal groups
    list <InternalGroup*> internalGroups;

    set <pair<string, int> > stalePGroupCategories;
    set <pair<string, int> > staleIGroupCategories;
    

    void setAllPerceptualCategoriesStale();
    
    const Sorts* sorts;

};

struct perceptualGroupingStruct {
  SoarGameObject* object;
  PerceptualGroup* group;
  bool assigned;
  bool oldGroup;
  int x,y;
};

struct internalGroupingStruct {
  SoarGameObject* object;
  InternalGroup* group;
  bool assigned;
  int x,y;
};

#endif // GroupManager_h
