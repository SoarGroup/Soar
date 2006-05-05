#include "InternalGroupManager.h"
#include "general.h"
#include <iostream>

#include "InternalGroup.h"
#include "Sorts.h"
#include "SoarInterface.h"

using namespace std;

/*
  InternalGroupManager.cpp
  SORTS project
  Sam Wintermute, 2006

*/


InternalGroupManager::InternalGroupManager() {
  // this does not change
  internalGroupingRadius = 1024;
}

void InternalGroupManager::updateGroups() {
  prepareForReGroup();
    // prune empty groups (if units died)
    // prepare list of group categories that need to be reGrouped
    // recalculate the center member for groups that changed
  
  reGroup();
    
  generateGroupData();
    // prune groups emptied during reGrouping
    // aggregate data about the groups
  
  return;
}

void InternalGroupManager::prepareForReGroup() {
  list<InternalGroup*>::iterator igroupIter; 
  for (igroupIter = internalGroups.begin();
       igroupIter != internalGroups.end(); 
       igroupIter++) {
    if ((*igroupIter)->getHasStaleMembers()) {
      if ((*igroupIter)->isEmpty()) {
        delete (*igroupIter);
        internalGroups.erase(igroupIter);
      }
      else {
        staleGroupCategories.insert((*igroupIter)->getCategory());
      }
    }
  }
  //cout << "end ref" << endl;
  return;
}

void InternalGroupManager::reGroup() {
  // this is simpler than reGroupPerceptual because we don't care about
  // groups maintaining their identities across cycles
  
  // iterate through staleGroupCategories set
  //  find all the groups of each type
  //  add the members to a big list
  //  keep a struct for each object:
  //    ptr to the obj, flag for if it has been assigned a group, ptr to group
  //  go through each obj1 in the list
  //    if not flagged, rm from old group and make a new group
  //    check each object (obj2) below in list:
  //      if objs are close and obj2 not flagged, flag obj2 and bring to same group
  //      if objs are close and obj2 flagged, merge groups
  
  set<pair<string, int> >::iterator catIter = staleGroupCategories.begin();
  internalGroupingStruct objectData;
  list<SoarGameObject*> groupMembers;
  list<InternalGroup*>::iterator groupIter;
  list<SoarGameObject*>::iterator objectIter;
  
  list<internalGroupingStruct> groupingList;

  // save all the to-merge pairs in a list
  // do all the merges at the end
  // this prevents invalid groups in the list (groups are deleted after a merge)
  list<pair<InternalGroup*, InternalGroup*> > toMergeList;
  
  while (catIter != staleGroupCategories.end()) {
    //cout << "doing type " << catIter->first << endl;
    groupingList.clear();
    
    for (groupIter = internalGroups.begin(); 
        groupIter != internalGroups.end(); 
        groupIter++) {
      if ((*groupIter)->getCategory() == *catIter) {
        //cout << "group " << (int) (*groupIter) << endl;
        // group is of the type we are re-grouping
       
        // not an old group- don't care about center distinction
        objectData.group = *groupIter;
        objectData.assigned = false;
        groupMembers = (*groupIter)->getMembers();
        objectIter = groupMembers.begin();
        while (objectIter != groupMembers.end()) {
          objectData.object = *objectIter;
          objectData.x = *(*objectIter)->gob->sod.x;
          objectData.y = *(*objectIter)->gob->sod.y;
          groupingList.push_back(objectData);
          objectIter++;
        }
      }
      // else it was a group of a different type
    }
    list<internalGroupingStruct>::iterator obj1StructIter, obj2StructIter;
    internalGroupingStruct obj1Struct;
    
    obj1StructIter = groupingList.begin();
    
    while (obj1StructIter != groupingList.end()) {
      obj1Struct = *obj1StructIter;
      
      if (not obj1Struct.assigned) {
        // make a new group for this object- no existing 
        // group has claimed it yet
        obj1Struct.group->removeUnit(obj1Struct.object);
        internalGroups.push_back(new InternalGroup(obj1Struct.object));
        obj1Struct.group = obj1Struct.object->getInternalGroup();
        obj1Struct.assigned = true;
        //cout << "XXX making new group " << (int) obj1Struct.group << endl; 
      }
     
      // iterate through all lower objects to see if they should join the group
      obj2StructIter = obj1StructIter;
      obj2StructIter++;
      
      while (obj2StructIter != groupingList.end()) {
        if (squaredDistance(obj1Struct.x, obj1Struct.y, 
                            (*obj2StructIter).x, (*obj2StructIter).y)
            <= internalGroupingRadius) {
          if ((*obj2StructIter).assigned) {
            // obj2 already has been grouped- groups should merge
            pair<InternalGroup*, InternalGroup*> groups;

            groups.second = obj1Struct.group;
            groups.first = (*obj2StructIter).group;
            
            toMergeList.push_back(groups);
            //cout << "XXX will merge " << (int) groups.first << " -> " << (int) groups.second << endl;
          }
          else {
            // obj2 has not been assigned. Assign it to obj1's group.
            //cout << "XXX obj from group " << (int) (*obj2StructIter).group <<
            //        " joining " << (int) obj1Struct.group << endl;
            (*obj2StructIter).assigned = true;
            (*obj2StructIter).group->removeUnit((*obj2StructIter).object);
            (*obj2StructIter).group = obj1Struct.group;
            (*obj2StructIter).group->addUnit((*obj2StructIter).object);
            
          }
        //  cout << "grouped!" << endl;
        }
        else {
       //   cout << "not grouped!" << endl;
        }
        obj2StructIter++; 
      }
      // jump the iterator between the two lists
      obj1StructIter++;
    }
    catIter++;
  } // end iterating through all the types that need re-grouping
  
  // do merges- always merge the first group to the second
  
  list<pair<InternalGroup*, InternalGroup*> >::iterator toMergeIter;
  list<pair<InternalGroup*, InternalGroup*> >::iterator toMergeIter2;

  // if two groups merge, we need to ensure that the subsumed group
  // does not have any outstanding merges
  toMergeIter = toMergeList.begin();
  while (toMergeIter != toMergeList.end()) {
    //cout << "groups " << (int)  (*toMergeIter).first << " and " << (int) (*toMergeIter).second << " will merge\n";
    if ((*toMergeIter).first == (*toMergeIter).second) {
      // do nothing- the groups were already merged
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
        if ((*toMergeIter2).second == (*toMergeIter).first) {
          (*toMergeIter2).second = (*toMergeIter).second;
        }
        toMergeIter2++;
      }

      (*toMergeIter).first->mergeTo((*toMergeIter).second);
    }
    
    toMergeIter++;
  }
 
  staleGroupCategories.clear();
  //cout << "XXX regroup done" << endl;
  return;
}

void InternalGroupManager::generateGroupData() {
  // prune empty groups, update centers
  
  list<InternalGroup*>::iterator groupIter;

  for (groupIter = internalGroups.begin(); 
       groupIter != internalGroups.end(); 
       groupIter++) {
    if ((*groupIter)->getHasStaleMembers()) {
      if ((*groupIter)->isEmpty()) {
        delete (*groupIter);
        internalGroups.erase(groupIter);
      }
      else {
        (*groupIter)->updateCenterLoc();
        (*groupIter)->setHasStaleMembers(false);
      }
    }
  }
  
  return;
}

void InternalGroupManager::makeNewGroup(SoarGameObject* object) {
  if (internallyGrouped(object)) {
    internalGroups.push_back(new InternalGroup(object));
  }
  return;
}

InternalGroupManager::~InternalGroupManager() {
  list<InternalGroup*>::iterator groupIter;
  for (groupIter = internalGroups.begin(); 
       groupIter != internalGroups.end(); 
       groupIter++) {
    delete (*groupIter);
  }
}


InternalGroup* InternalGroupManager::getGroupNear(string type, int owner, int x, int y) {
  // return NULL if no groups of that type are known
  
  //cout << "search for " << type << " owned by " << owner << endl;
  pair<string, int> targetCategory;
  targetCategory.first = type;
  targetCategory.second = owner;
  
  int currentX, currentY;
  double currentDistance;
  double closestDistance = 99999999;
  InternalGroup* closestGroup = (InternalGroup*) NULL;
  
  list<InternalGroup*>::iterator groupIter;

  for (groupIter = internalGroups.begin(); 
      groupIter != internalGroups.end(); 
      groupIter++) {
    if ((*groupIter)->getCategory() == targetCategory) {
      (*groupIter)->getCenterLoc(currentX, currentY);
      currentDistance = squaredDistance(x, y, currentX, currentY);
      if (currentDistance < closestDistance) {
        closestDistance = currentDistance;
        closestGroup = *groupIter;
      }
    }
  }
  
  return closestGroup;
}

bool InternalGroupManager::internallyGrouped(SoarGameObject* sgo) {
  // return true if the object is something FSMs might need to look for
  string name = sgo->gob->bp_name();

  if (name == "mineral" or name == "commandCenter") {
    return true;
  }
  return false;
}
