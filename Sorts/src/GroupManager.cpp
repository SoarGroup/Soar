#include "include/GroupManager.h"
#include "include/general.h"
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
    /* The way you're using the list of groups here, the action is just
     * assigned to each group. But I thought the point of specifying
     * multiple groups in an action was so that you can say "group 1
     * go attack group 2," not "I want groups 1 and 2 to both attack."
     */

    groupIter = (**actionIter).groups.begin();
    while (groupIter != (**actionIter).groups.end()) {
      success &= (*groupIter)->assignAction((**actionIter).type, (**actionIter).params, (*groupIter)->getCenterMember());
      groupIter++;
    }
    actionIter++;
  }

  return success;
}

void GroupManager::reGroup() {
#ifdef DEBUG_GROUPS
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

  // this should really come from the attention code
  double groupingDistanceSquared = 10000;
  
  set<int>::iterator typeIter = staleGroupTypes.begin();
  objectGroupingStruct objectData;
  list<SoarGameObject*> groupMembers;
  list<SoarGameGroup*>::iterator groupIter;
  list<SoarGameObject*>::iterator objectIter;
  
  list<objectGroupingStruct> groupingList;
  list<objectGroupingStruct> centerGroupingList;

  // save all the to-merge pairs in a list
  // do all the merges at the end
  // this prevents invalid groups in the list (groups are deleted after a merge)
  list<pair<SoarGameGroup*, SoarGameGroup*> > toMergeList;
  
  SoarGameObject* centerObject;

  while (typeIter != staleGroupTypes.end()) {
    groupingList.clear();
    groupIter = groupsInFocus.begin();
    if (groupIter == groupsInFocus.end()) {
      groupIter = groupsNotInFocus.begin();
    }
    while (groupIter != groupsNotInFocus.end()){
      // not a typo- loop will jump between lists
      if ((*groupIter)->getType() == *typeIter) {
        // group is of the type we are re-grouping
        
        objectData.group = *groupIter;
        objectData.assigned = false;
        centerObject = (*groupIter)->getCenterMember();
    
        // centers are stored in a separate list
        objectData.object = centerObject;
        objectData.x = centerObject->x;
        objectData.y = centerObject->y;
        
        centerGroupingList.push_back(objectData);
        groupMembers = (*groupIter)->getMembers();
        objectIter = groupMembers.begin();
        while (objectIter != groupMembers.end()) {
          if ((*objectIter) != centerObject){
            // don't add the center object to this list
            objectData.object = *objectIter;
            objectData.x = (*objectIter)->x;
            objectData.y = (*objectIter)->y;
            groupingList.push_back(objectData);
          }
          objectIter++;
        }
      }
      // else it was a group of a different type
      
      // switch over from in-focus to not- we don't care about the
      // distinction here
      groupIter++;
      if (groupIter == groupsInFocus.end()) {
        groupIter = groupsNotInFocus.begin();
      }
    }
    // the lists are now built, centers in a separate list we will
    // treat as being "before" the other list
    
    // now follow the grouping procedure as outlined above
    bool obj1IsACenter = true;
    list<objectGroupingStruct>::iterator obj1StructIter, obj2StructIter;
    objectGroupingStruct obj1Struct;
    
    obj1StructIter = centerGroupingList.begin();
    if (obj1StructIter == centerGroupingList.end()) {
      // this really should not happen
      obj1StructIter = groupingList.begin();
      obj1IsACenter = false;
    }
    while (obj1StructIter != groupingList.end()) {
      obj1Struct = *obj1StructIter;
      
      if (not obj1IsACenter and not obj1Struct.assigned) {
        // make a new group for this object- no existing 
        // group has claimed it yet
        obj1Struct.group->removeUnit(obj1Struct.object);
        addGroup(obj1Struct.object);
        obj1Struct.group = obj1Struct.object->getGroup();
        obj1Struct.assigned = true;
      }
     
      // iterate through all lower objects to see if they should join the group
      obj2StructIter = obj1StructIter;
      obj2StructIter++;
      if (obj2StructIter == centerGroupingList.end()) {
        obj2StructIter = groupingList.begin();
      }
      while (obj2StructIter != groupingList.end()) {
        //obj2Struct = *obj2StructIter;
        if (squaredDistance(obj1Struct.x, obj1Struct.y, 
                            (*obj2StructIter).x, (*obj2StructIter).y)
            <= groupingDistanceSquared) {
          if ((*obj2StructIter).assigned) {
            // obj2 already has been grouped- obj1's group 
            // should join that group
            pair<SoarGameGroup*, SoarGameGroup*> groups;
            groups.first = obj1Struct.group;
            groups.second = (*obj2StructIter).group;
            toMergeList.push_back(groups);
          }
          else {
            // obj2 has not been assigned. Assign it to obj1's group.
            (*obj2StructIter).assigned = true;
            (*obj2StructIter).group->removeUnit((*obj2StructIter).object);
            (*obj2StructIter).group = obj1Struct.group;
            (*obj2StructIter).group->addUnit((*obj2StructIter).object);
            
          }
          //cout << "grouped!" << endl;
        }
        else {
          //cout << "not grouped!" << endl;
        }
        obj2StructIter++; 
        if (obj2StructIter == centerGroupingList.end()) {
          obj2StructIter = groupingList.begin();
        }
      }
      // jump the iterator between the two lists
      obj1StructIter++;
      if (obj1StructIter == centerGroupingList.end()) {
        obj1StructIter = groupingList.begin();
        obj1IsACenter = false;
      }
    }
    typeIter++;
  } // end iterating through all the types that need re-grouping
  
  // do merges- always merge the smaller group to the bigger group
  
  list<pair<SoarGameGroup*, SoarGameGroup*> >::iterator toMergeIter;
  list<pair<SoarGameGroup*, SoarGameGroup*> >::iterator toMergeIter2;

  // if two groups merge, we need to ensure that the subsumed group
  // does not have any outstanding merges
  toMergeIter = toMergeList.begin();
  while (toMergeIter != toMergeList.end()) {
    if ((*toMergeIter).first == (*toMergeIter).second) {
      // do nothing- the groups were already merged
    }
    else if ((*toMergeIter).first->getSize() >
        (*toMergeIter).second->getSize() ) {
      // merge second into first
      
      toMergeIter2 = toMergeIter;
      toMergeIter2++;
      while (toMergeIter2 != toMergeList.end()) {
        // replace all occurrences of the squashed group with the
        // new combined group
        if ((*toMergeIter2).first == (*toMergeIter).second) {
          (*toMergeIter2).first = (*toMergeIter).first;
        }
        else if ((*toMergeIter2).second == (*toMergeIter).second) {
          (*toMergeIter2).second = (*toMergeIter).first;
        }
        toMergeIter2++;
      }

      (*toMergeIter).second->mergeTo((*toMergeIter).first);
    }
    else {
      // merge first into second
      
      toMergeIter2 = toMergeIter;
      toMergeIter2++;
      while (toMergeIter2 != toMergeList.end()) {
        // replace all occurrences of the squashed group with the
        // new combined group
        if ((*toMergeIter2).first == (*toMergeIter).first) {
          (*toMergeIter2).first = (*toMergeIter).second;
        }
        else if ((*toMergeIter2).second == (*toMergeIter).first) {
          (*toMergeIter2).second = (*toMergeIter).second;
        }
        toMergeIter2++;
      }

      (*toMergeIter).first->mergeTo((*toMergeIter).second);
    }
    
    toMergeIter++;
  }
  
#endif
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
