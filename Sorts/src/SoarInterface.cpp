#include<iostream>
#include <assert.h>
#include <pthread.h>

#include "SoarInterface.h"
#include "Utils.h"

#include "Game.H"

using namespace std;

// lookup table that translates string names to action codes
SoarActionType actionTypeLookup(const char* actionName);

SoarInterface::SoarInterface(sml::Agent*      _agent,
                             GameStateModule* _gsm,
                             pthread_mutex_t* _objectActionQueueMutex,
                             pthread_mutex_t* _attentionActionQueueMutex,
                             pthread_mutex_t* _groupActionQueueMutex
                            )
: agent(_agent),
  gsm(_gsm),
  objectActionQueueMutex(_objectActionQueueMutex),
  attentionActionQueueMutex(_attentionActionQueueMutex),
  groupActionQueueMutex(_groupActionQueueMutex)
{
  inputLink = agent->GetInputLink();
  groupIdCounter = 0;
}

SoarInterface::~SoarInterface() {
}

/* Note: We don't need to create the property list associated with
 * the group at this time, wait until a refresh
 */
void SoarInterface::addGroup(SoarGameGroup* group) {
  // make sure the group does not exist already
  assert(mwToSoarGroups.find(group) == mwToSoarGroups.end());
  
  SoarIOGroupRep newGroup;
  newGroup.groupId = groupIdCounter++;
  newGroup.added = false;
  
  mwToSoarGroups[group] = newGroup;
  gIdToMwGroups[newGroup.groupId] = group;
}

void SoarInterface::removeGroup(SoarGameGroup* group) {
  // make sure the group exists
  //assert(mwToSoarGroups.find(group) != mwToSoarGroups.end());
  
  // false removes can come in (fairly frequently)
  // if a group is merged into another the same cycle it appears, a false remove
  // is generated
  if (mwToSoarGroups.find(group) == mwToSoarGroups.end()) {
    return;
  }
  
  SoarIOGroupRep &g = mwToSoarGroups[group];
  if (g.groupId >= 0) {
    agent->DestroyWME(g.WMEptr);
  }
  mwToSoarGroups.erase(group);
  gIdToMwGroups.erase(g.groupId);
}

void SoarInterface::refreshGroup(SoarGameGroup* group, groupPropertyList gpl) {
  // make sure the group exists
  assert(mwToSoarGroups.find(group) != mwToSoarGroups.end());
  
  SoarIOGroupRep &g = mwToSoarGroups[group];
  
  if (!g.added) {
    // add the group to the soar input link if it hasn't been already
    g.added = true;

    if (group->getOwner() == gsm->get_game().get_client_player()) {
      g.WMEptr = agent->CreateIdWME(playerGroupsId, "group");
    }
    else {
      g.WMEptr = agent->CreateIdWME(otherPlayers[group->getOwner()].groupsId, "group");
    }

    // label the group with its id
    agent->CreateIntWME(g.WMEptr, "id", g.groupId);
    cout << "WME (r): group" << endl;
    cout << "WME: \tid " << g.groupId << endl;

    // add properties
    for(groupPropertyList::iterator i = gpl.begin(); i != gpl.end(); i++) {
      // create a new WME object for the property
      g.properties[(*i).first] = agent->CreateIntWME(g.WMEptr, (*i).first.c_str(), (*i).second);
      cout << "WME: \t" << (*i).first << " " << (*i).second << endl;
    }
  }
  else {
    // group already added, just update values.
    // Note that I'm assuming no new values are introduced
    cout << "WME: updated group" << endl;
    cout << "WME: \tid " << g.groupId << endl;
    for(groupPropertyList::iterator i = gpl.begin(); i != gpl.end(); i++) {
      agent->Update(g.properties[(*i).first], (*i).second);
      cout << "WME: \t" << (*i).first << " " << (*i).second << endl;
    }
  }
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
    /*int groupCounter = 0;
    while(true) {
      string groupParam = "group";
      groupParam += ('0' + groupCounter++);
      const char* paramValue = cmdPtr->GetParameterValue(groupParam.c_str());
      if (paramValue == NULL) {
        break;
      }
      int groupId = atoi(paramValue);
      assert(gIdToMwGroups.find(groupId) != gIdToMwGroups.end());
      newAction.groups.push_back(gIdToMwGroups[groupId]);
    }*/
    
    /* There's really no need for a list of groups, all actions should
       be group to group, or group with int parameters.
    */
    string groupParam = "source_group";
    const char* paramValue = cmdPtr->GetParameterValue(groupParam.c_str());
    if (paramValue == NULL) {
      newAction.source = NULL;
    }
    else {
      int groupId = atoi(paramValue);
      assert(gIdToMwGroups.find(groupId) != gIdToMwGroups.end());
      newAction.source = gIdToMwGroups[groupId];
    }
    groupParam = "target_group";
    paramValue = cmdPtr->GetParameterValue(groupParam.c_str());
    if (paramValue == NULL) {
      newAction.target = NULL;
    }
    else {
      int groupId = atoi(paramValue);
      assert(gIdToMwGroups.find(groupId) != gIdToMwGroups.end());
      newAction.target = gIdToMwGroups[groupId];
    }
    

    // append all the integer parameters
    int paramCounter = 0;
    while (true) {
      string intParam = "param";
      intParam += ('0' + paramCounter++);
      const char* paramValue = cmdPtr->GetParameterValue(intParam.c_str());
      if (paramValue == NULL) {
        break;
      }
      newAction.params.push_back(atoi(paramValue));
    }

    // add the new action to the action queue
    objectActionQueue.push_back(&newAction);
  } // for over commands

  pthread_mutex_unlock(objectActionQueueMutex);
}

// called by middleware to get queued Soar actions
void SoarInterface::getNewActions(list<SoarAction*>& newActions) {
  pthread_mutex_lock(objectActionQueueMutex);
  for(list<SoarAction*>::iterator i = objectActionQueue.begin(); i != objectActionQueue.end(); i++) {
    newActions.push_back(*i);
    cout << "adding action.." << endl;
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
  agent->Update(playerGoldWME, amount);
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
  mapIdentifier = agent->CreateIdWME(inputLink, "map");

  playerGoldWME = agent->CreateIntWME(playerId, "gold", 0);
  playerGroupsId = agent->CreateIdWME(playerId, "groups");

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
//  agent->Commit();
//  cout << "### COMMIT FINISHED ###" << endl;
}
