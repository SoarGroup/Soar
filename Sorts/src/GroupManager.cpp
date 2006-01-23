#include "GroupManager.h"

/* TODO:

recent changes:
-ORTSIO now directly removes dead objects from groups,
  regrouper (or update stats?) must prune empty groups

-ORTSIO makes the new groups too, and calls a GroupMgr function to add them to the list

-GameObj*'s have ptrs to the groups, and set the stale bit (need a group touch function) when changes occur
  So the updateStats func iterates thru all groups and updates the stale ones

-adjustGroups must take data from the attn module:
  determine on/off link based on map region
  determine sticky radius based on zoom level
*/


void GroupManager::updateWorld() {
  createNewGroups();
  adjustGroups();
  updateStats();

  return;
}

bool GroupManager::assignActions() {
  // through the SoarIO, look for any new actions, and assign them to groups
    
  vector <SoarAction*> newActions = SoarIO->getNewActions();
  Action a;
  bool success = true;
  
  for (unsigned int i=0; i<newActions.size(); i++) {
    a = newActions.at(i);
    success &= a.group->assignAction(a.action);
  }

  return success;
}

void GroupManager::createNewGroups() {
  vector <SoarGameObject*> newObjs = ORTSIO.getNewObjects();

  for (unsigned int i=0; i<newObjs.size(); i++) {
    SoarGameGroup* grp = new SoarGameGroup(newObjs.at(i));

    newGroups.push_back(grp);
  }

  return;
}

void GroupManager::adjustGroups() {
  // do stuff here!
  // otherwise, all groups are singletons

  // lump the new groups w/ each other or with existing groups

  for (unsigned int i=0; i<newGroups.size(); i++) { 
    // do smart stuff here

    // if it is actually a new group..
    groupsInFocus.push_back(newGroups.at(i));
    SoarIO->addGroup(newGroups.at(i));

    // else remove it from the newGroups list, so the stats don't get updated
  }
  return;
}

void GroupManager::updateStats() {
  vector <SoarGameGroup*> changedGroups = ORTSIO.getChangedGroups();
  groupPropertyList changedProperties; 
  SoarGameGroup* grp;
  
  for (unsigned int i=0; i < newGroups.size(); i++) { 
    grp = newGroups.at(i);
    grp->updateStats();
    SoarIO->refreshGroup(grp);
  }
  for (unsigned int i=0; i < changedGroups.size(); i++) {
    grp = changedGroups.at(i);
    changedProperties = grp->updateStats();
    SoarIO->refreshGroup(grp, changedProperties);
  }

  return;
}

