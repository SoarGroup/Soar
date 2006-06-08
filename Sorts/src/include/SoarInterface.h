#ifndef soarinterface_h
#define soarinterface_h

#include<utility>
#include<list>
#include<map>
#include<pthread.h>

#include "sml_Client.h"

#include "MapRegion.h"
#include "SoarAction.h"
#include "general.h"
#include "FeatureMap.h"
#include "PerceptualGroupManager.h"
class Sorts;

class PerceptualGroup;

using namespace std;

typedef struct {
  int groupId;
  bool added;
  sml::Identifier* WMEptr;
  // change this later into hash_map<string, IntElement*> and write a hash
  // function for strings
  map<string, sml::IntElement*> intProperties;
  map<string, sml::FloatElement*> floatProperties;
  map<string, sml::StringElement*> stringProperties;
  // a list of integers for the IDs of the regions its in
  list<sml::IntElement*> regionWMEs;
} InputLinkGroupRep;

typedef struct {
  sml::Identifier* identifierWME;
  sml::IntElement* idWME;
  sml::IntElement* xminWME;
  sml::IntElement* xmaxWME; 
  sml::IntElement* yminWME; 
  sml::IntElement* ymaxWME; // bounding box
  sml::IntElement* sizeWME;
} InputLinkMapRegionRep;

typedef struct {
  sml::Identifier* identifierWME;
  sml::IntElement* sector0WME;
  sml::IntElement* sector1WME;
  sml::IntElement* sector2WME;
  sml::IntElement* sector3WME;
  sml::IntElement* sector4WME;
  sml::IntElement* sector5WME;
  sml::IntElement* sector6WME;
  sml::IntElement* sector7WME;
  sml::IntElement* sector8WME;
} InputLinkFeatureMapRep;


typedef struct {
  sml::Identifier* identifierWME;
  sml::IntElement* centerXWME;
  sml::IntElement* centerYWME;
  sml::IntElement* viewWidthWME;
  sml::IntElement* focusXWME;
  sml::IntElement* focusYWME;
  sml::IntElement* ownerGroupingWME;
  sml::IntElement* numObjectsWME;
  sml::IntElement* groupingRadiusWME;
} VisionParameterRep;

typedef struct {
  sml::Identifier* identifierWME;
  sml::StringElement* queryNameWME;
  sml::IntElement* param0WME;
  sml::IntElement* param1WME;
} QueryResultRep;

/* 
The PerceptualGroupManager will have a pointer to this structure, and can call
the public functions to get new actions and change what groups Soar can
"see". Whatever needs to be done to initialize Soar or do higher level
things like pause for debugging will be done by the main loop, which can
call other (not yet defined) functions inside the SoarInterface.

SoarInterface will be responsible for creating and organizing the WMEs
for the groups, but not for determining what information is in the
structure (for example, we don't want a "health" WME for a tree, but
SoarInterface shouldn't need to figure that out). There will be a list
of visible properties inside the PerceptualGroups that will be set by the
PerceptualGroupManager and read by the SoarInterface to determine that.

Note: addGroup should not actually add the group to the input_link until
that group is refreshed! Initially, the stats will not be set.
*/

class SoarInterface {
  public:
    SoarInterface
    ( sml::Agent*      _agent,
      pthread_mutex_t* _objectActionQueueMutex,
      pthread_mutex_t* _attentionActionQueueMutex,
      pthread_mutex_t* _soarMutex);

    ~SoarInterface();

    void getNewObjectActions(list<ObjectAction>& newActions);
    void getNewAttentionActions(list<AttentionAction>& newActions);
    void getNewMapActions(list<MapAction>& newActions);

    void updateViewFrame(int frame);

    // grouping commands for Group Manager to call
    void addGroup(PerceptualGroup* group);
    void removeGroup(PerceptualGroup* group);
    void refreshGroup(PerceptualGroup* group);
    int  groupId(PerceptualGroup* group);

    // map commands
    void addMapRegion(MapRegion* r);
    void removeMapRegion(MapRegion* r);
    void refreshMapRegion(MapRegion* r);
    int  mapRegionId(MapRegion* r);

    // feature map commands
    void addFeatureMap(FeatureMap* m, string name);
    //void removeFeatureMap(FeatureMap* m);
    void refreshFeatureMap(FeatureMap*, string name);
    
    // commit all changes to Soar Input link
    void commitInputLinkChanges();

    // update player info
    void updatePlayerGold(int amount);

    void updateQueryResult(string name, int param0, int param1);
    /* this is the function for the Soar interrupt handler.
     * Don't try to call this
     */
    void getNewSoarOutput();

    void initSoarInputLink();
    void initVisionState(VisionParameterStruct vps);
    void updateVisionState(VisionParameterStruct& vps);
    bool getStale();
    void setStale(bool);
    void lockSoarMutex();
    void unlockSoarMutex();

    void stopSoar();
    void startSoar();
    bool isSoarRunning() { return soarRunning; }

  private:

    void lockObjectActionMutex();
    void unlockObjectActionMutex();
    void lockAttentionActionMutex();
    void unlockAttentionActionMutex();

    void processObjectAction(ObjectActionType, sml::Identifier*);
    void processAttentionAction(AttentionActionType, sml::Identifier*);
    void processMapAction(MapActionType, sml::Identifier*);
    void improperCommandError();

    // SML pointers
    sml::Agent *agent;

    sml::Identifier* inputLink;
    sml::Identifier* groupsIdWME;
    sml::Identifier* gameInfoIdWME;
    sml::IntElement* playerGoldWME;
    sml::IntElement* viewFrameWME;

  /**************************************************
   *                                                *
   * Member variables for group management          *
   *                                                *
   **************************************************/

    // SoarInterface numbers each group based on its own convention
    int groupIdCounter;

    // these are the maps that keep track of input link <-> middleware objects
    /* Change these later to hash maps */
    map<PerceptualGroup*, InputLinkGroupRep> groupTable;
    map<int, PerceptualGroup*>               groupIdLookup;
   
  
  /**************************************************
   *                                                *
   * Member variables for map maintanence           *
   *                                                *
   **************************************************/
    sml::Identifier* mapIdWME;
    map<MapRegion*, InputLinkMapRegionRep> mapRegionTable;
    map<int, MapRegion*>                   mapRegionIdLookup;

  /**************************************************
   *                                                *
   * Member variables for feature maps              *
   *                                                *
   **************************************************/
    sml::Identifier* featureMapIdWME;
    map<string, InputLinkFeatureMapRep> featureMapTable;

    // for vision state

    VisionParameterStruct initialVisionParams;
    VisionParameterRep visionParamRep;

    QueryResultRep queryResultRep;

  /**************************************************
   *                                                *
   * Member variables for actions                   *
   *                                                *
   **************************************************/

    list<ObjectAction> objectActionQueue;
    list<AttentionAction> attentionActionQueue;
    list<MapAction> mapActionQueue;

    // associated mutexes that protect them (no longer used)
    pthread_mutex_t* objectActionQueueMutex;
    pthread_mutex_t* attentionActionQueueMutex;
    pthread_mutex_t* soarMutex;

    bool stale;
    bool soarRunning;
};

#endif
