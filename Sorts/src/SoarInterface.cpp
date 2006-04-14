#include<iostream>
#include <assert.h>
#include <pthread.h>

#include "SoarInterface.h"
#include "SoarGameGroup.h"
#include "general.h"
#include "SoarAction.h"
#include "Sorts.h"

#include "Game.H"

using namespace std;

SoarInterface::SoarInterface
( sml::Agent*      _agent,
  pthread_mutex_t* _objectActionQueueMutex,
  pthread_mutex_t* _attentionActionQueueMutex,
  pthread_mutex_t* _soarMutex
)
: agent(_agent),
  objectActionQueueMutex(_objectActionQueueMutex),
  attentionActionQueueMutex(_attentionActionQueueMutex),
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
    cout << "XXX remove id " << g.groupId << " ptr " << (int)group << endl;
    agent->DestroyWME(g.WMEptr);
  }
  groupTable.erase(group);
  groupIdLookup.erase(g.groupId);
}

void SoarInterface::refreshGroup(SoarGameGroup* group) {
  lockSoarMutex();
  stale = true;

  groupPropertyStruct gps = group->getSoarData();
  
  // make sure the group exists
  assert(groupTable.find(group) != groupTable.end());
  
  InputLinkGroupRep &g = groupTable[group];
 
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
    cout << "XXX adding id " << g.groupId << " ptr " << (int) group << endl;

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
    cout << "XXX updating id " << g.groupId << endl;
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
  lockSoarMutex();
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
  unlockSoarMutex();
}

void SoarInterface::removeMapRegion(MapRegion *r) {
  lockSoarMutex();
  assert(mapRegionTable.find(r) != mapRegionTable.end());
  
  InputLinkMapRegionRep& rep = mapRegionTable[r];
  agent->DestroyWME(rep.identifierWME);
  mapRegionIdLookup.erase(r->getId());
  mapRegionTable.erase(r);
  unlockSoarMutex();
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

void SoarInterface::addFeatureMap(FeatureMap *m, string name) {
  lockSoarMutex();
  assert(featureMapTable.find(name) == featureMapTable.end());

  InputLinkFeatureMapRep rep;
  rep.identifierWME = agent->CreateIdWME(featureMapIdWME, name.c_str());
  rep.sector0WME = agent->CreateIntWME(rep.identifierWME, 
                                       "sector0", m->getCount(0));
  rep.sector1WME = agent->CreateIntWME(rep.identifierWME, 
                                       "sector1", m->getCount(1));
  rep.sector2WME = agent->CreateIntWME(rep.identifierWME, 
                                       "sector2", m->getCount(2));
  rep.sector3WME = agent->CreateIntWME(rep.identifierWME, 
                                       "sector3", m->getCount(3));
  rep.sector4WME = agent->CreateIntWME(rep.identifierWME, 
                                       "sector4", m->getCount(4));
  rep.sector5WME = agent->CreateIntWME(rep.identifierWME, 
                                       "sector5", m->getCount(5));
  rep.sector6WME = agent->CreateIntWME(rep.identifierWME, 
                                       "sector6", m->getCount(6));
  rep.sector7WME = agent->CreateIntWME(rep.identifierWME, 
                                       "sector7", m->getCount(7));
  rep.sector8WME = agent->CreateIntWME(rep.identifierWME, 
                                       "sector8", m->getCount(8));

  featureMapTable[name] = rep;
  unlockSoarMutex();
}
/* save for dynamic feature maps, if needed
void SoarInterface::removeFeatureMap(FeatureMap *r) {
  lockSoarMutex();
  assert(featureMapTable.find(r) != featureMapTable.end());
  
  InputLinkFeatureMapRep& rep = featureMapTable[r];
  agent->DestroyWME(rep.identifierWME);
  featureMapIdLookup.erase(r->getId());
  featureMapTable.erase(r);
  unlockSoarMutex();
}
*/

void SoarInterface::refreshFeatureMap(FeatureMap *m, string name) {
  lockSoarMutex();
  stale = true;
  
  assert(featureMapTable.find(name) != featureMapTable.end());
  
  InputLinkFeatureMapRep& rep = featureMapTable[name];
  agent->Update(rep.sector0WME, m->getCount(0));
  agent->Update(rep.sector1WME, m->getCount(1));
  agent->Update(rep.sector2WME, m->getCount(2));
  agent->Update(rep.sector3WME, m->getCount(3));
  agent->Update(rep.sector4WME, m->getCount(4));
  agent->Update(rep.sector5WME, m->getCount(5));
  agent->Update(rep.sector6WME, m->getCount(6));
  agent->Update(rep.sector7WME, m->getCount(7));
  agent->Update(rep.sector8WME, m->getCount(8));

  unlockSoarMutex();
}


// called in soar event handler to take everything off the output
// link and put onto the action queue each time soar generates output
void SoarInterface::getNewSoarOutput() {
  lockSoarMutex();
  
  int numberCommands = agent->GetNumberCommands();
  
  for (int i = 0 ; i < numberCommands ; i++) {
    sml::Identifier* cmdPtr = agent->GetCommand(i);

    // check if this command has already been encountered
    if (cmdPtr->GetParameterValue("status") != NULL) {
      continue;
    }
    
    string name = cmdPtr->GetCommandName() ;
    cout << "command name: " << name << endl;
    ObjectActionType OType = objectActionTypeLookup(name);

    if (OType != OA_NO_SUCH_ACTION) {
      processObjectAction(OType, cmdPtr);
    }
    else {
      AttentionActionType AType = attentionActionTypeLookup(name);
      if (AType != AA_NO_SUCH_ACTION) {
        processAttentionAction(AType, cmdPtr);
      }
      else {
        assert(false);
      }
    }
  }

  unlockSoarMutex();
}

void SoarInterface::processObjectAction(ObjectActionType type, 
                                        sml::Identifier* cmdPtr) {
  ObjectAction newAction;
  newAction.type = type;
  
  lockObjectActionMutex();
  // append all the group parameters
  int groupCounter = 0;
  while(true) {
    const char* paramValue 
    = cmdPtr->GetParameterValue(catStrInt("group", groupCounter++).c_str());
    if  (paramValue == NULL) {
      break;
    }
    int groupId = atoi(paramValue);
    assert(groupIdLookup.find(groupId) != groupIdLookup.end());
    newAction.groups.push_back(groupIdLookup[groupId]);
  }
  
  // append all the integer parameters
  int paramCounter = 0;
  while (true) {
    const char* paramValue 
    = cmdPtr->GetParameterValue(catStrInt("param", paramCounter++).c_str());
    if (paramValue == NULL) {
      break;
    }
    newAction.params.push_back(atoi(paramValue));
  }

  // add the new action to the action queue
  objectActionQueue.push_back(newAction);

  cmdPtr->AddStatusComplete();

  unlockObjectActionMutex();
}

void SoarInterface::processAttentionAction(AttentionActionType type, 
                                        sml::Identifier* cmdPtr) {
  AttentionAction newAction;
  newAction.type = type;
  
  lockAttentionActionMutex();
  const char* paramValue;
  
  switch (type) {
    case AA_LOOK_LOCATION:
    case AA_MOVE_LOCATION:  
      paramValue = cmdPtr->GetParameterValue("x");
      if (paramValue == NULL) {
        improperCommandError();
        return;
      }
      newAction.params.push_back(atoi(paramValue));
      paramValue = cmdPtr->GetParameterValue("y");
      if (paramValue == NULL) {
        improperCommandError();
        return;
      }
      newAction.params.push_back(atoi(paramValue));
      break;
    case AA_LOOK_FEATURE:
    case AA_MOVE_FEATURE:
      paramValue = cmdPtr->GetParameterValue("sector");
      if (paramValue == NULL) {
        improperCommandError();
        return;
      }
      newAction.params.push_back(atoi(paramValue));
      paramValue = cmdPtr->GetParameterValue("feature");
      if (paramValue == NULL) {
        improperCommandError();
        return;
      }
      newAction.fmName = paramValue;
      break;
    case AA_RESIZE:
    case AA_GROUPING_RADIUS:
      paramValue = cmdPtr->GetParameterValue("value");
      if (paramValue == NULL) {
        improperCommandError();
        return;
      }
      newAction.params.push_back(atoi(paramValue));
      break;
    default:
      assert(false);
      break;
  }

  attentionActionQueue.push_back(newAction);

  cmdPtr->AddStatusComplete();

  unlockAttentionActionMutex();
}

// called by middleware to get queued Soar actions
void SoarInterface::getNewObjectActions(list<ObjectAction>& newActions) {
  lockObjectActionMutex();
  for(list<ObjectAction>::iterator i = objectActionQueue.begin(); 
                                  i != objectActionQueue.end(); 
                                  i++)
  {
    newActions.push_back(*i);
    objectActionQueue.erase(i);
  }
  unlockObjectActionMutex();
}

void SoarInterface::getNewAttentionActions(list<AttentionAction>& newActions) {
  lockAttentionActionMutex();
  for(list<AttentionAction>::iterator i = attentionActionQueue.begin(); 
                                  i != attentionActionQueue.end(); 
                                  i++)
  {
    newActions.push_back(*i);
    attentionActionQueue.erase(i);
  }
  unlockAttentionActionMutex();
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
  featureMapIdWME = agent->CreateIdWME(inputLink, "feature-maps");

  playerGoldWME = agent->CreateIntWME(playerId, "gold", 0);
  playerGroupsId = agent->CreateIdWME(playerId, "groups");

  worldId = agent->CreateIdWME(inputLink, "world");
  worldGroupsId = agent->CreateIdWME(worldId, "groups");
/*
  for(int p = 0; p < gsm->get_game().get_player_num(); p++) {
    if (p != gsm->get_game().get_client_player()) {
      otherPlayers[p].id = agent->CreateIdWME(inputLink, catStrInt("p", p).c_str());
      otherPlayers[p].groupsId = agent->CreateIdWME(otherPlayers[p].id, "groups");
    }
  }
*/
  int numPlayers = sorts->OrtsIO->getNumPlayers();
  int myId = sorts->OrtsIO->getMyId();
  
  for (int p=0; p < numPlayers; p++) {
    if (p != myId) {
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
  pthread_mutex_lock(soarMutex);
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

void SoarInterface::lockObjectActionMutex() { 
  pthread_mutex_lock(objectActionQueueMutex);
}
 
void SoarInterface::unlockObjectActionMutex() { 
  pthread_mutex_unlock(objectActionQueueMutex);
}
void SoarInterface::lockAttentionActionMutex() { 
  pthread_mutex_lock(attentionActionQueueMutex);
}
 
void SoarInterface::unlockAttentionActionMutex() { 
  pthread_mutex_unlock(attentionActionQueueMutex);
}

void SoarInterface::improperCommandError() {
  cout << "ERROR: Improperly formatted command!\n";
}
