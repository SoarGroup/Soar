#include "GroupManager.h"

/* TODO:

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
  adjustGroups();
  epdateStats();

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

void GroupManager::adjustGroups() {

  // for now, all groups of 1
  // new groups are in the NIF list, just move to IF list

  vector<SoarGameGroup*>::iterator NIFIter = groupsNotInFocus.begin();
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
void addGroup(const SoarGameObject* object) {
  groupsNotInFocus.push_back(new SoarGameGroup(object));
  return;
}
