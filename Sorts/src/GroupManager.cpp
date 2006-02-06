#include "include/GroupManager.h"
#include <iostream>
using namespace std;

/*
  GroupManager.cpp
  SORTS project
  Sam Wintermute, 2006

*/

void GroupManager::updateWorld() {
  
  refreshGroups(false);
  reGroup();
  refreshGroups(true);
  adjustAttention();

  return;
}

bool GroupManager::assignActions() {
  // through the SoarIO, look for any new actions, and assign them to groups
    
  list <SoarAction*> newActions;
  SoarIO->getNewActions(newActions);
  list <SoarAction*>::iterator actionIter = newActions.begin();
 
  list <SoarGameGroup*>::iterator groupIter;
  bool success = true;
  
  while (actionIter != newActions.end()){
    groupIter = (**actionIter).groups.begin();
    while (groupIter != (**actionIter).groups.end()) {
      success &= (*groupIter)->assignAction((**actionIter).type, (**actionIter).params);
      groupIter++;
    }
    actionIter++;
  }

  return success;
}

void GroupManager::reGroup() {
  // iterate through staleGroupTypes set
  //  find all the groups of each type
  //  add the members to a big list, centers first
  //  keep a struct for each object:
  //    ptr to the obj, flag for if it has been assigned a group, ptr to group
  //  go through each obj1 in the list
  //    if not a group-center and not flagged, rm from old group and make a new group
  //    check each object (obj2) below in list:
  //      if objs are close and obj2 not flagged, flag obj2 and bring to same group
  //      if objs are close and obj2 flagged, merge obj1's group -> obj2's group 

  

  return;
}

void GroupManager::refreshGroups(bool final) {
  // iterate through all the groups, if they are stale,
  // refresh them (re-calc stats)
  // pass the final flag to updateStats,
  // it will save a list of WMEs for Soar if the flag is set
 
  // also, if not final, add the group type of stale groups to
  // the staleGroupTypes set, so reGroup will run on them
 
  list<SoarGameGroup*>::iterator groupIter;

  groupIter = groupsInFocus.begin();
  while (groupIter != groupsInFocus.end()) {
    if ((*groupIter)->getStale()) {
      if ((*groupIter)->isEmpty()) {
        SoarIO->removeGroup(*groupIter);
        delete (*groupIter);
        groupsInFocus.erase(groupIter);
      }
      else {
        (*groupIter)->updateStats(final);
        staleGroupTypes.insert((*groupIter)->getType());
      }
    }
    groupIter++;
  }

  groupIter = groupsNotInFocus.begin();
  while (groupIter != groupsNotInFocus.end()) {
    if ((*groupIter)->getStale()) {
      if ((*groupIter)->isEmpty()) {
        SoarIO->removeGroup(*groupIter);
        delete (*groupIter);
        groupsInFocus.erase(groupIter);
      }
      else {
        (*groupIter)->updateStats(final);
        staleGroupTypes.insert((*groupIter)->getType());
      }
    }
    groupIter++;
  }

  return;
}

void GroupManager::addGroup(SoarGameObject* object) {
  groupsNotInFocus.push_back(new SoarGameGroup(object));
  return;
}

void GroupManager::adjustAttention() {
  // iterate through all staleInSoar groups, if in attn. range,
  // send params to Soar
  
  list<SoarGameGroup*>::iterator groupIter = groupsNotInFocus.begin();
  while (groupIter != groupsNotInFocus.end()) {
    // for now, move everything into focus
    groupsInFocus.push_back(*groupIter);
    SoarIO->addGroup(*groupIter);
    groupsNotInFocus.erase(groupIter);
    groupIter++;
  }

  groupIter = groupsInFocus.begin();
  while (groupIter != groupsInFocus.end()) {
    if ((*groupIter)->getStaleInSoar()) {
      SoarIO->refreshGroup((*groupIter), (*groupIter)->getProps());
      (*groupIter)->setStaleInSoar(false);
    }
    groupIter++;
  }
  return;
}

GroupManager::~GroupManager() {
  list<SoarGameGroup*>::iterator groupIter = groupsNotInFocus.begin();
  while (groupIter != groupsNotInFocus.end()) {
    delete (*groupIter);
    groupIter++;
  }
  groupIter = groupsInFocus.begin();
  while (groupIter != groupsInFocus.end()) {
    delete (*groupIter);
    groupIter++;
  }
}
