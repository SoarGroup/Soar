#include<iostream>
#include <assert.h>
#include <pthread.h>

#include "SoarInterface.h"
#include "SoarGameGroup.h"
#include "general.h"

#include "Game.H"

using namespace std;

// lookup table that translates string names to action codes
SoarActionType actionTypeLookup(const char* actionName);

SoarInterface::SoarInterface(GameStateModule* _gsm,
                             sml::Agent*      _agent,
                             pthread_mutex_t* _objectActionQueueMutex,
                             pthread_mutex_t* _attentionActionQueueMutex,
                             pthread_mutex_t* _groupActionQueueMutex,
                             pthread_mutex_t* _soarMutex
                            )
: gsm(_gsm),
  agent(_agent),
  objectActionQueueMutex(_objectActionQueueMutex),
  attentionActionQueueMutex(_attentionActionQueueMutex),
  groupActionQueueMutex(_groupActionQueueMutex),
  soarMutex(_soarMutex)
{
  inputLink = agent->GetInputLink();
  groupIdCounter = 0;
  stale = true;
}

SoarInterface::~SoarInterface() {
}

/* Note: We don't need to create the property list associated with
 * the group at this time, wait until a refresh
 */
void SoarInterface::addGroup(SoarGameGroup* group) {
  // make sure the group does not exist already
  assert(groupTable.find(group) == groupTable.end());
  
  InputLinkGroupRep newGroup;
  newGroup.groupId = groupIdCounter++;
  newGroup.added = false;
  
  groupTable[group] = newGroup;
  groupIdLookup[newGroup.groupId] = group;
}

void SoarInterface::removeGroup(SoarGameGroup* group) {
  // make sure the group exists
  //assert(groupTable.find(group) != groupTable.end());
  
  // false removes can come in (fairly frequently)
  // if a group is merged into another the same cycle it appears, a false remove
  // is generated
  if (groupTable.find(group) == groupTable.end()) {
    return;
  }
  
  InputLinkGroupRep &g = groupTable[group];
  if (g.groupId >= 0) {
    agent->DestroyWME(g.WMEptr);
  }
  groupTable.erase(group);
  groupIdLookup.erase(g.groupId);
}

void SoarInterface::refreshGroup(SoarGameGroup* group, groupPropertyStruct gps) {
  lockSoarMutex();
  stale = true;
  
  // make sure the group exists
  assert(groupTable.find(group) != groupTable.end());
  
  InputLinkGroupRep &g = groupTable[group];
 
  cout << "REF GRP\n";
  
  if (!g.added) {
    // add the group to the soar input link if it hasn't been already
    g.added = true;

    if (group->isFriendly()) {
      g.WMEptr = agent->CreateIdWME(playerGroupsId, "group");
    }
    else if (group->isWorld() ) {
      g.WMEptr = agent->CreateIdWME(worldGroupsId, "group");
    }
    else {
      g.WMEptr = agent->CreateIdWME( otherPlayers[group->getOwner()].groupsId, 
                                     "group" );
    }

    // label the group with its id
    agent->CreateIntWME(g.WMEptr, "id", g.groupId);

    // add properties
    for(list<pair<string,int> >::iterator 
        i  = gps.stringIntPairs.begin(); 
        i != gps.stringIntPairs.end(); 
        i++) 
    {
      // create a new WME object for the property
      g.intProperties[(*i).first] = 
        agent->CreateIntWME(g.WMEptr, (*i).first.c_str(), (*i).second);
      cout << "\tadd: " << (*i).first << " " << (*i).second << endl;
    }
    for(list<pair<string,string> >::iterator 
        i = gps.stringStringPairs.begin(); 
        i != gps.stringStringPairs.end(); 
        i++) 
    {
      // create a new WME object for the property
      g.stringProperties[(*i).first] = 
        agent->CreateStringWME( g.WMEptr, 
                                (*i).first.c_str(), 
                                (*i).second.c_str() );
      cout << "\tadd: " << (*i).first << " " << (*i).second << endl;
    }
  }
  else {
    // group already added, just update values.
    // Note that I'm assuming no new values are introduced
    for(list<pair<string, int> >::iterator
        i = gps.stringIntPairs.begin(); 
        i != gps.stringIntPairs.end(); 
        i++)
    {
      // (added assertions to check this.. -sw)
      assert(g.intProperties.find((*i).first) 
             != g.intProperties.end());
      agent->Update(g.intProperties[(*i).first], (*i).second);
      cout << "\tupd: " << (*i).first << " " << (*i).second << endl;
    }
    for(list<pair<string, string> >::iterator 
        j = gps.stringStringPairs.begin();
        j != gps.stringStringPairs.end(); 
        j++) 
    {
      assert(g.stringProperties.find((*j).first) 
             != g.stringProperties.end());
      agent->Update(g.stringProperties[(*j).first], (*j).second.c_str());
      cout << "\tupd: " << (*j).first << " " << (*j).second << endl;
    }
  }

  // in any case, remove all the region ids and put in new ones
  for(list<sml::IntElement*>::iterator
      i  = g.regionWMEs.begin();
      i != g.regionWMEs.end();
      i++)
  {
    agent->DestroyWME(*i);
  }
  g.regionWMEs.clear();
  for(list<int>::iterator 
      i  = gps.regionsOccupied.begin();
      i != gps.regionsOccupied.end();
      i++)
  {
    g.regionWMEs.push_back(agent->CreateIntWME(g.WMEptr, "in-region", *i));
  }

  unlockSoarMutex();
}

int SoarInterface::groupId(SoarGameGroup* group) {
  assert(groupTable.find(group) != groupTable.end());
  return groupTable[group].groupId;
}

void SoarInterface::addMapRegion(MapRegion *r) {
  assert(mapRegionTable.find(r) == mapRegionTable.end());

  InputLinkMapRegionRep rep;
  rep.identifierWME = agent->CreateIdWME(mapIdWME, "region");
  Rectangle box = r->getBoundingBox();
  rep.idWME   = agent->CreateIntWME(rep.identifierWME, "id", r->getId());
  rep.xminWME = agent->CreateIntWME(rep.identifierWME, "xmin", box.xmin);
  rep.xmaxWME = agent->CreateIntWME(rep.identifierWME, "xmax", box.xmax);
  rep.yminWME = agent->CreateIntWME(rep.identifierWME, "ymin", box.ymin);
  rep.ymaxWME = agent->CreateIntWME(rep.identifierWME, "ymax", box.ymax);

  rep.sizeWME = agent->CreateIntWME(rep.identifierWME, "size", r->size());

  mapRegionTable[r] = rep;
  mapRegionIdLookup[r->getId()] = r;
}

void SoarInterface::removeMapRegion(MapRegion *r) {
  assert(mapRegionTable.find(r) != mapRegionTable.end());
  
  InputLinkMapRegionRep& rep = mapRegionTable[r];
  agent->DestroyWME(rep.identifierWME);
  mapRegionIdLookup.erase(r->getId());
  mapRegionTable.erase(r);
}

void SoarInterface::refreshMapRegion(MapRegion *r) {
  lockSoarMutex();
  stale = true;
  
  assert(mapRegionTable.find(r) != mapRegionTable.end());
  
  InputLinkMapRegionRep& rep = mapRegionTable[r];
  Rectangle box = r->getBoundingBox();
  agent->Update(rep.idWME, r->getId());
  agent->Update(rep.xminWME, box.xmin);
  agent->Update(rep.xmaxWME, box.xmax);
  agent->Update(rep.yminWME, box.ymin);
  agent->Update(rep.ymaxWME, box.ymax);
  agent->Update(rep.sizeWME, r->size());

  unlockSoarMutex();
}


// called in soar event handler to take everything off the output
// link and put onto the action queue each time soar generates output
void SoarInterface::getNewSoarOutput() {
  int numberCommands = agent->GetNumberCommands() ;
  
  // try to lock the object action queue over the entire loop for now
  // might have to change later
  pthread_mutex_lock(objectActionQueueMutex);

  for (int i = 0 ; i < numberCommands ; i++) {
    sml::Identifier* cmdPtr = agent->GetCommand(i) ;

    // check if this command has already been encountered
    if (soarActions.find(cmdPtr) != soarActions.end()) {
      continue;
    }

    string name = cmdPtr->GetCommandName() ;
    cout << "command name: " << name << endl;
    SoarActionType type = actionTypeLookup(name.c_str());
    assert(type != SA_NO_SUCH_ACTION);

    SoarAction& newAction = soarActions[cmdPtr];
    newAction.type = type;
    
    // append all the group parameters
    int groupCounter = 0;
    while(true) {
      const char* paramValue = cmdPtr->GetParameterValue(catStrInt("group", groupCounter++).c_str());
      if (paramValue == NULL) {
        break;
      }
      int groupId = atoi(paramValue);
      assert(groupIdLookup.find(groupId) != groupIdLookup.end());
      newAction.groups.push_back(groupIdLookup[groupId]);
    }
    
    /* There's really no need for a list of groups, all actions should
       be group to group, or group with int parameters.
    */
    /* Update: We're going back to group lists, for stuff like
     * "I want this worker to harvest this mineral and deposit to this base"
     */
    /*
    string groupParam = "source_group";
    const char* paramValue = cmdPtr->GetParameterValue(groupParam.c_str());
    if (paramValue == NULL) {
      newAction.source = NULL;
    }
    else {
      int groupId = atoi(paramValue);
      assert(groupIdLookup.find(groupId) != groupIdLookup.end());
      newAction.source = groupIdLookup[groupId];
    }
    groupParam = "target_group";
    paramValue = cmdPtr->GetParameterValue(groupParam.c_str());
    if (paramValue == NULL) {
      newAction.target = NULL;
    }
    else {
      int groupId = atoi(paramValue);
      assert(groupIdLookup.find(groupId) != groupIdLookup.end());
      newAction.target = groupIdLookup[groupId];
    }
    */
    
    // append all the integer parameters
    int paramCounter = 0;
    while (true) {
      const char* paramValue = cmdPtr->GetParameterValue(catStrInt("param", paramCounter++).c_str());
      if (paramValue == NULL) {
        break;
      }
      newAction.params.push_back(atoi(paramValue));
    }

    // add the new action to the action queue
    objectActionQueue.push_back(&newAction);

    cmdPtr->AddStatusComplete();
  } // for over commands

  pthread_mutex_unlock(objectActionQueueMutex);
}

// called by middleware to get queued Soar actions
void SoarInterface::getNewActions(list<SoarAction*>& newActions) {
  pthread_mutex_lock(objectActionQueueMutex);
  for(list<SoarAction*>::iterator i = objectActionQueue.begin(); 
                                  i != objectActionQueue.end(); 
                                  i++)
  {
    newActions.push_back(*i);
    objectActionQueue.erase(i);
  }
  pthread_mutex_unlock(objectActionQueueMutex);
}

/* This is slow. In the future make some kind of hashtable
 */
SoarActionType actionTypeLookup(const char* actionName) {
  if      (!strcmp(actionName, "move"))      return SA_MOVE;
  else if (!strcmp(actionName, "mine"))      return SA_MINE;
  else                                       return SA_NO_SUCH_ACTION;
}


void SoarInterface::updatePlayerGold(int amount) {
  lockSoarMutex();
  stale = true;
  agent->Update(playerGoldWME, amount);
  unlockSoarMutex();
}

/*
 * inputLink ----> me ----> gold
 *             |       \--> groups
 *             |
 *             \-> p1 ---->groups ---->
 *             |                    \->
 *             \-> p2 ----> ...
 *             |
 *             \-> map ---> ...
 */
void SoarInterface::initSoarInputLink() {
  playerId= agent->CreateIdWME(inputLink, "me");
  mapIdWME = agent->CreateIdWME(inputLink, "map");

  playerGoldWME = agent->CreateIntWME(playerId, "gold", 0);
  playerGroupsId = agent->CreateIdWME(playerId, "groups");

  worldId = agent->CreateIdWME(inputLink, "world");
  worldGroupsId = agent->CreateIdWME(worldId, "groups");

  for(int p = 0; p < gsm->get_game().get_player_num(); p++) {
    if (p != gsm->get_game().get_client_player()) {
      otherPlayers[p].id = agent->CreateIdWME(inputLink, catStrInt("p", p).c_str());
      otherPlayers[p].groupsId = agent->CreateIdWME(otherPlayers[p].id, "groups");
    }
  }

  agent->Commit();
}


void SoarInterface::commitInputLinkChanges() {
//  cout << "### COMMIT ABOUT TO BE CALLED ###" << endl;
  //agent->Commit();
//  cout << "### COMMIT FINISHED ###" << endl;
}

void SoarInterface::lockSoarMutex() { 
  cout << "lock req" << endl;
  pthread_mutex_lock(soarMutex);
  cout << "lock got" << endl;
}

void SoarInterface::unlockSoarMutex() { 
  pthread_mutex_unlock(soarMutex);
}

bool SoarInterface::getStale() {
  return stale;
}

void SoarInterface::setStale(bool _st) {
  stale = _st;
}
