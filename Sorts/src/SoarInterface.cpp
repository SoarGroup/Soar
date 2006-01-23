#include <assert.h>
#include <pthread.h>

#include "SoarInterface.h"

using namespace sml;

// lookup table that translates string names to action codes
SoarActionType actionTypeLookup(const char* actionName);

SoarInterface::SoarInterface(pthread_mutex_t* _actionQueueMutex) 
: actionQueueMutex(_actionQueueMutex)
{
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
  assert(mwToSoarGroups.find(group) != mwToSoarGroups.end());
  
  SoarIOGroupRep &g = mwToSoarGroups[group];
  if (g.groupId >= 0) {
    agent->DestroyWME(g.WMEptr);
  }
  mwToSoarGroups.erase(group);
  gIdToMwGroups.erase(g.groupId);
}

void SoarInterface::refreshGroup(SoarGameGroup* group, groupPropertyList& gpl) {
  // make sure the group exists
  assert(mwToSoarGroups.find(group) != mwToSoarGroups.end());
  
  SoarIOGroupRep &g = mwToSoarGroups[group];
  
  if (!g.added) {
    // add the group to the soar input link if it hasn't been already
    g.added = true;
    g.WMEptr = agent->CreateIdWME(inputLink, "group");
    for(groupPropertyList::iterator i = gpl.begin(); i != gpl.end(); i++) {
      for(groupPropertyList::iterator i = gpl.begin(); i != gpl.end(); i++) {
        // create a new WME object for the property
        g.properties[(*i).first] = agent->CreateIntWME(g.WMEptr, (*i).first.c_str(), (*i).second);
      }
    }
  }
  else {
    // group already added, just update values.
    // Note that I'm assuming no new values are introduced
    for(groupPropertyList::iterator i = gpl.begin(); i != gpl.end(); i++) {
      agent->Update(g.properties[(*i).first], (*i).second);
    }
  }
}

// called in soar event handler to take everything off the output
// link and put onto the action queue each time soar generates output
void SoarInterface::getNewActions(list<SoarAction*> &actions) {
  int numberCommands = agent->GetNumberCommands() ;
  for (int i = 0 ; i < numberCommands ; i++) {
    Identifier* cmdPtr = agent->GetCommand(i) ;

    // check if this command has already been encountered
    if (soarActions.find(cmdPtr) == soarActions.end()) {
      continue;
    }

    std::string name = cmdPtr->GetCommandName() ;
    std::cout << "command name: " << name << std::endl;
    SoarActionType type = actionTypeLookup(name.c_str());
    assert(type != SA_NO_SUCH_ACTION);
    
    SoarAction& newAction = soarActions[cmdPtr];
    newAction.type = type;
    
    // append all the group parameters
    int groupCounter = 0;
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

    // add the new action to the action queue. Have to lock on mutex here
    pthread_mutex_lock(actionQueueMutex);
    actionQueue.push_back(&newAction);
    pthread_mutex_unlock(actionQueueMutex);

  } // for over commands
}

// called by middleware to get queued Soar actions
void SoarInterface::getNewActions(list<SoarAction*> newActions) {
  pthread_mutex_lock(actionQueueMutex);
  for(list<SoarAction*>::iterator i = actionQueue.begin(); i != actionQueue.end(); i++) {
    newActions.push_back(*i);
    actionQueue.erase(i);
  }
  pthread_mutex_unlock(actionQueueMutex);
}

/* This is slow. In the future make some kind of hashtable
 */
SoarActionType actionTypeLookup(const char* actionName) {
  if      (!strcmp(actionName, "move"))      return SA_MOVE;
  else if (!strcmp(actionName, "mine"))      return SA_MINE;
  else                                       return SA_NO_SUCH_ACTION;
}
