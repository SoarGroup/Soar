#ifndef soarinterface_h
#define soarinterface_h

#include<utility>
#include<list>
#include<map>
#include<pthread.h>

#include "GameStateModule.H"

#include "sml_Client.h"

#include "SoarAction.h"
#include "general.h"

class SoarGameGroup;

using namespace std;

//typedef list<pair<string, int> > groupPropertyList;

typedef struct {
  int groupId;
  bool added;
  sml::Identifier* WMEptr;
  // change this later into hash_map<string, IntElement*> and write a hash
  // function for strings
  map<string, sml::IntElement*> intProperties;
  map<string, sml::StringElement*> stringProperties;
} SoarIOGroupRep;

typedef struct {
  sml::Identifier* id;
  sml::Identifier* groupsId;
  // some other things in the future
} OtherPlayerRep;

/* 
The GroupManager will have a pointer to this structure, and can call
the public functions to get new actions and change what groups Soar can
"see". Whatever needs to be done to initialize Soar or do higher level
things like pause for debugging will be done by the main loop, which can
call other (not yet defined) functions inside the SoarInterface.

SoarInterface will be responsible for creating and organizing the WMEs
for the groups, but not for determining what information is in the
structure (for example, we don't want a "health" WME for a tree, but
SoarInterface shouldn't need to figure that out). There will be a list
of visible properties inside the SoarGameGroups that will be set by the
GroupManager and read by the SoarInterface to determine that.

Note: addGroup should not actually add the group to the input_link until
that group is refreshed! Initially, the stats will not be set.
*/

class SoarInterface {
  public:
    SoarInterface(sml::Agent*           _agent,
                  GameStateModule*      _gsm,
                  pthread_mutex_t* _objectActionQueueMutex,
                  pthread_mutex_t* _attentionActionQueueMutex,
                  pthread_mutex_t* _groupActionQueueMutex
                 );

    ~SoarInterface();

    void getNewActions(list<SoarAction*>& newActions);

    // grouping commands for Group Manager to call
    void addGroup(SoarGameGroup* group);
    void removeGroup(SoarGameGroup* group);
    void refreshGroup(SoarGameGroup* group, groupPropertyStruct gps);

    // commit all changes to Soar Input link
    void commitInputLinkChanges();

    // update player info
    void updatePlayerGold(int amount);

    /* this is the function for the Soar interrupt handler.
     * Don't try to call this
     */
    void getNewSoarOutput();

    void initSoarInputLink();

  private:

    int groupIdCounter;

    // SML pointers
    sml::Agent *agent;

    // input link stuff
    sml::Identifier* inputLink;
    sml::Identifier* playerId;
    sml::Identifier* playerGroupsId;

    sml::Identifier* worldId;
    sml::Identifier* worldGroupsId;

    sml::IntElement* playerGoldWME;
    sml::Identifier* mapIdentifier;

    GameStateModule* gsm;

    map<int, OtherPlayerRep> otherPlayers;

    // these are the maps that keep track of input link <-> middleware objects
    /* Change these later to hash maps */
    map<SoarGameGroup*, SoarIOGroupRep> mwToSoarGroups;
    map<int, SoarGameGroup*>            gIdToMwGroups;
   
    // keep track of actions on the input link and middleware
    map<sml::Identifier*, SoarAction>  soarActions;

    // lists of actions that are currently unprocessed
    // there is a race condition on accessing this list
    list<SoarAction*> objectActionQueue;
    // need to add two more, once we get the SoarAction class modified

    // ... and the associated mutexes that protect them
    pthread_mutex_t* objectActionQueueMutex;
    pthread_mutex_t* attentionActionQueueMutex;
    pthread_mutex_t* groupActionQueueMutex;
};

#endif
