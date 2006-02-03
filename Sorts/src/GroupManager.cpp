#include "GroupManager.h"

/*
  GroupManager.cpp
  SORTS project
  Sam Wintermute, 2006
*/

void GroupManager::updateWorld() {
  
  reGroup();
  updateStats();

  /*
    This might not be the optimal way to update-
    if objects move groups don't know until updateStats,
    and they don't change until reGroup the next cycle.

    Ideally, we could do this:
    updateStats_1() update things that affect grouping (don't send to soar)
    reGroup() based on the newly updated info
    updateStats_2() update new groups and send to Soar

  */

  return;
}

bool GroupManager::assignActions() {
  // through the SoarIO, look for any new actions, and assign them to groups
    
  list <SoarAction*> newActions = SoarIO->getNewActions();
  list <SoarAction*>::iterator actionIter = newActions.front();
  
  list <SoarGameGroup*>::iterator groupIter;
  bool success = true;
  
  while (actionIter != newActions.end()){
    groupIter = actionIter->groups.front();
    while (groupIter != actionIter->groups.end()) {
      success &= groupIter->assignAction(actionIter->type, actionIter->params);
      groupIter++;
    }
    actionIter++;
  }

  return success;
}

void GroupManager::reGroup() {

  // for now, all groups of 1
  // new groups are in the NIF list, just move to IF list

  list<SoarGameGroup*>::iterator NIFIter = groupsNotInFocus.begin();
  while (NIFIter != groupsNotInFocus.end()) { 
    // do smart stuff here

    // if it is actually a new group..
    groupsInFocus.push_back(*NIFIter);
    SoarIO->addGroup(*NIFIter);
    groupsNotInFocus.erase(NIFIter);
    NIFIter++;
  }
  return;
}

void GroupManager::updateStats() {
  groupPropertyList changedProperties; 
  SoarGameGroup* grp;
 
  list<SoarGameGroup*>::iterator groupIter;

  // look at each group in focus, if it changed, update Soar
  groupIter = groupsInFocus.begin();
  while (groupIter != groupsInFocus.end()) {
    if ((*groupIter)->getStale()) {
      if ((*groupIter)->isEmpty()) {
        SoarIO->removeGroup(*groupIter);
        delete (*groupIter);
        groupsInFocus.erase(groupIter);
      }
      else {
        changedProperties = (*groupIter)->updateStats();
        SoarIO->refreshGroup(grp, changedProperties);
      }
    }
    groupIter++;
  }

  // same for out of focus, but don't update Soar
  // if they moved into focus, reGroup will get it next cycle
  groupIter = groupsNotInFocus.begin();
  while (groupIter != groupsNotInFocus.end()) {
    if ((*groupIter)->getStale()) {
      if ((*groupIter)->isEmpty()) {
        SoarIO->removeGroup(*groupIter);
        delete (*groupIter);
        groupsInFocus.erase(groupIter);
      }
      else {
        (*groupIter)->updateStats();
      }
    }
    groupIter++;
  }

  return;
}

void GroupManager::addGroup(const SoarGameObject* object) {
  groupsNotInFocus.push_back(new SoarGameGroup(object));
  return;
}

GroupManager::~GroupManager() {
  groupIter = groupsNotInFocus.begin();
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
