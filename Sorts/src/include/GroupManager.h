#ifndef GroupManager_h
#define GroupManager_h

#include "SoarGameObject.h"
//#include "MapManager.h"
//#include "FeatureMapManager.h"
//#include "FeatureMap.h"
//#include "SoarInterface.h"

//class SoarInterface;
//class OrtsInterface;
//class FeatureMapManager;

//#include "Sorts.h"
class Sorts;

class GroupManager {
  public:
//    GroupManager(SoarInterface* si, MapManager* _mapManager,
//                 FeatureMapManager* fmm);
    GroupManager();
    ~GroupManager();

    void updateVision();
    bool assignActions();
    void processVisionCommands();

    void addGroup(SoarGameObject* object);
    // used by ORTSInterface when it sees a new object- create a group for it
    
    //void setORTSIO(OrtsInterface* oio);

    SoarGameGroup* getGroupNear(string type, int owner, int x, int y);
    
    void setSorts(Sorts* s) {sorts = s;}
    
  private:
    void prepareForReGroup();
    void reGroup();
    void generateGroupData();
    void adjustAttention();
    void updateFeatureMaps(bool refreshAll);

    void removeGroup(SoarGameGroup*);

//    SoarInterface* SoarIO;

    set <pair<string, int> > staleGroupCategories;
    
    list <SoarGameGroup*> groupsInFocus;
    list <SoarGameGroup*> groupsNotInFocus;
    
    Sorts* sorts;
    //OrtsInterface* ORTSIO;

    //MapManager *mapManager;
    //FeatureMapManager *featureMaps;
};

struct objectGroupingStruct {
  SoarGameObject* object;
  SoarGameGroup* group;
  bool assigned;
  bool oldGroup;
  int x,y;
};

#endif // GroupManager_h
