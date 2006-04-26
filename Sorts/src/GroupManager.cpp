#include "include/GroupManager.h"
#include "include/general.h"
#include <iostream>

#include "PerceptualGroup.h"
#include "InternalGroup.h"
#include "Sorts.h"
#include "SoarInterface.h"

using namespace std;

/*
  GroupManager.cpp
  SORTS project
  Sam Wintermute, 2006

*/


GroupManager::GroupManager() {
  // sorts ptr is NOT valid here, use the intialize() function! 
    
  // this default should be reflected in the agent's assumptions
  // (1024 = 32^2)
  perceptualGroupingRadius = 1024;
  
  // this does not change
  internalGroupingRadius = 1024;

  // the number of objects near the focus point to add
  // agent can change this, if it wishes to cheat
  numObjects = 9991;

  ownerGrouping = false;
}

void GroupManager::initialize() {
  // called right after sorts ptr set up
  focusX = (int) (sorts->OrtsIO->getMapXDim() / 2.0);
  focusY = (int) (sorts->OrtsIO->getMapYDim() / 2.0);

  centerX = focusX;
  centerY = focusY;

  viewWidth = 2*focusX;
  cout << "init: " << focusX << " " << focusY << " " << viewWidth << endl;
  sorts->featureMapManager->changeViewWindow(focusX, focusY, viewWidth);

}
void GroupManager::updateVision() {
  prepareForReGroup();
    // prune empty groups (if units died)
    // prepare list of group categories that need to be reGrouped
    // recalculate the center member for groups that changed
  
  reGroupInternal();
  reGroupPerceptual();
    // re-calculate the groups
    
  generateInternalGroupData();
  generatePerceptualGroupData();
    // prune groups emptied during reGrouping
    // aggregate data about the groups
  
  adjustAttention(false);
    // determine which groups are attended to,
    // and send them to Soar
  return;
}

bool GroupManager::assignActions() {
  // through the sorts->getSoarInterface(), look for any new actions, and assign them to groups
  // actions have a list of params and a list of groups,
  // the first group (must exist) is the group the action will be applied to
    
  list <ObjectAction> newActions;
  
  sorts->SoarIO->getNewObjectActions(newActions);
  list <ObjectAction>::iterator actionIter = newActions.begin();
 
  list <PerceptualGroup*>::iterator groupIter;
  bool success = true;
  list<PerceptualGroup*> targetGroups;
  PerceptualGroup* sourceGroup;
  
  while (actionIter != newActions.end()){
    targetGroups.clear();
    list<PerceptualGroup*>& groups = (*actionIter).groups;
    groupIter = groups.begin();
    
    assert(groupIter != groups.end());
    // the first group is the group the action is applied to, it must exist
    
    sourceGroup = *groupIter;
    groupIter++;
    
    while (groupIter != groups.end()) {
      targetGroups.push_back(*groupIter);
      groupIter++;
    }
    
    success &= sourceGroup->assignAction(
            (*actionIter).type, (*actionIter).params, targetGroups);
    
    actionIter++;
  }

  return success;
}

void GroupManager::processVisionCommands() {
  // called when Soar changes the view window, wants to attend to an item
  // in a feature map, or changes a grouping parameter.

  // view window change:
  // call sorts->getFeatureMapManager()->changeViewWindow()
  // updateFeatureMaps(true)

  // attention shift (w/o view window shift):
  // call adjustAttention() to select the new groups
  // call updateFeatureMaps(false) to inhibit any newly-attended to groups

  // attention shift w/ view window shift:
  // sorts->getFeatureMapManager()->changeViewWindow()
  // adjustAttention()
  // updateFeatureMaps(true)

  // grouping change:
  // this is the same as updateVision, except we don't need prepareForReGroup,
  // since none of the objects in the world actually changed:
  // reGroupPerceptual()
  // generatePerceptualGroupData()
  // adjustAttention()
  // updateFeatureMaps(false)
  
  list<AttentionAction> actions;
  sorts->SoarIO->getNewAttentionActions(actions);

  int radius;
  PerceptualGroup* centerGroup;
        list<int>::iterator it;

  for (list<AttentionAction>::iterator i = actions.begin();
                                       i != actions.end();
                                       i++) {
    switch (i->type) {
      case AA_LOOK_LOCATION:
        assert(i->params.size() == 2); // x,y
        it = i->params.begin();
        focusX = *it;
        it++;
        focusY = *it;
        
        // recalc all center distances and rebuild the order of the groups
        remakeGroupSet();
        adjustAttention(false); 

        break;
      case AA_LOOK_FEATURE:
        // attention shift (w/o view window shift)

        // first (and only) param is the sector number
        assert(i->params.size() == 1);
        centerGroup = sorts->featureMapManager->getGroup(i->fmName, 
                                                        *(i->params.begin()));
        if (centerGroup == NULL) {
          cout << "ERROR: sector " << *(i->params.begin()) << " of map " <<
            i->fmName << " is empty! Ignoring command\n";
          return;
        }
        
        // set focus point the center of the group
        centerGroup->getCenterLoc(focusX, focusY);

        // recalc all center distances and rebuild the order of the groups
        remakeGroupSet();
        adjustAttention(false); 
        break;
      case AA_RESIZE:
        assert(i->params.size() == 1);
        viewWidth = *(i->params.begin());
        sorts->featureMapManager->changeViewWindow(centerX, centerY, viewWidth);
        adjustAttention(true);   // re-update all the feature maps-
                                 // changeViewWindow clears them out
        break;
      case AA_MOVE_LOCATION:
        break;
      case AA_MOVE_FEATURE:
        assert(i->params.size() == 1);
        centerGroup = sorts->featureMapManager->getGroup(i->fmName, 
                                                        *(i->params.begin()));
        if (centerGroup == NULL) {
          cout << "ERROR: sector " << *(i->params.begin()) << " of map " <<
            i->fmName << " is empty! Ignoring command\n";
          return;
        }
        
        centerGroup->getCenterLoc(focusX, focusY);
        centerX = focusX;
        centerY = focusY;
        sorts->featureMapManager->changeViewWindow(centerX, centerY, viewWidth);

        // recalc all center distances and rebuild the order of the groups
        remakeGroupSet();
        sorts->featureMapManager->changeViewWindow(centerX, centerY, viewWidth);
        adjustAttention(true);   // re-update all the feature maps-
                                 // changeViewWindow clears them out
        break;
      case AA_GROUPING_RADIUS:
      // grouping change:
      // this is the same as updateVision, except we don't need prepareForReGroup,
      // since none of the objects in the world actually changed:
        assert(i->params.size() == 1);
        radius = *(i->params.begin());
        radius *= radius;
        if (radius != perceptualGroupingRadius) {
          perceptualGroupingRadius = radius;
          setAllPerceptualCategoriesStale();
          reGroupPerceptual();
          generatePerceptualGroupData();
          adjustAttention(false);
        }
        break;
      case AA_NUM_OBJECTS:
        assert(i->params.size() == 1);
        numObjects = *(i->params.begin());
        adjustAttention(false);
        break;
      case AA_OWNER_GROUPING_ON:
        // handle similar to grouping radius change-
        // rebuild all the groups
        if (ownerGrouping) {
          cout << "WARNING: turning on ownerGrouping when it is already on.\n";
          break;
        }
        ownerGrouping = true; 
        // this essentially changes the definition of category
        
        setAllPerceptualCategoriesStale();
        reGroupPerceptual();
        generatePerceptualGroupData();
        adjustAttention(false);
        break;
      case AA_OWNER_GROUPING_OFF:
        if (not ownerGrouping) {
          cout << "WARNING: turning off ownerGrouping when it is already off.\n";
          break;
        }
        ownerGrouping = false; 
        
        setAllPerceptualCategoriesStale();
        reGroupPerceptual();
        generatePerceptualGroupData();
        adjustAttention(false);
        break;
      case AA_NO_SUCH_ACTION:
        break;
      default:
        break;
    }
  }
}

void GroupManager::prepareForReGroup() {
  // iterate through all the groups, if they are stale,
  // recalculate the center member
 
  // prune empty groups

  // add the group type of stale groups to
  // the staleGroupCategories sets, so reGroupX will run on them
 
  set<PerceptualGroup*>::iterator groupIter;

  list<set<PerceptualGroup*>::iterator> toErase;
  
  for (groupIter = perceptualGroups.begin();
       groupIter != perceptualGroups.end(); 
       groupIter++) {
    if ((*groupIter)->getHasStaleMembers()) {
      if ((*groupIter)->isEmpty()) {
        toErase.push_back(groupIter);
        // can't delete from sets like this- the iterator gets screwed up
        // keep a list, delete everything at once
      }
      else {
        (*groupIter)->updateCenterMember();
        stalePGroupCategories.insert((*groupIter)->getCategory(ownerGrouping));
      }
    }
  }
  for (list<set<PerceptualGroup*>::iterator>::iterator it= toErase.begin();
      it != toErase.end();
      it++) {
    removePerceptualGroup(**it);
    perceptualGroups.erase(*it);
  }

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
        staleIGroupCategories.insert((*igroupIter)->getCategory());
      }
    }
  }
  //cout << "end ref" << endl;
  return;
}


void GroupManager::reGroupPerceptual() {
  // iterate through stalePGroupCategories set
  //  find all the groups of each type
  //  add the members to a big list, centers first
  //  keep a struct for each object:
  //    ptr to the obj, flag for if it has been assigned a group, ptr to group
  //  go through each obj1 in the list
  //    if not a group-center and not flagged, rm from old group and make a new group
  //    check each object (obj2) below in list:
  //      if objs are close and obj2 not flagged, flag obj2 and bring to same group
  //      if objs are close and obj2 flagged, check oldGroup flag
  //        merge groups, preferring to keep groups w/ oldgroup flag
  //        if neither is set, choice is arbitrary
  //        if both are set, prefer the larger group

  set<pair<string, int> >::iterator catIter = stalePGroupCategories.begin();
  perceptualGroupingStruct objectData;
  list<SoarGameObject*> groupMembers;
  set<PerceptualGroup*>::iterator groupIter;
  list<SoarGameObject*>::iterator objectIter;
  
  list<perceptualGroupingStruct> groupingList;
  list<perceptualGroupingStruct> centerGroupingList;

  // save all the to-merge pairs in a list
  // do all the merges at the end
  // this prevents invalid groups in the list (groups are deleted after a merge)
  list<pair<PerceptualGroup*, PerceptualGroup*> > toMergeList;
  
  SoarGameObject* centerObject;

  while (catIter != stalePGroupCategories.end()) {
    //cout << "doing type " << catIter->first << endl;
    groupingList.clear();
    centerGroupingList.clear();
    
    for (groupIter = perceptualGroups.begin(); groupIter != perceptualGroups.end(); groupIter++) {
      if (not (*groupIter)->getSticky() and
         ((*groupIter)->getCategory(ownerGrouping) == *catIter)) {
        //cout << "group " << (int) (*groupIter) << endl;
        // group is of the type we are re-grouping
       
        if ((*groupIter)->isOld()) {
          // oldGroup means the group has been around for at least one cycle
          // centers of old groups have priority for retaining their
          // group ID
          objectData.group = *groupIter;
          objectData.assigned = false;
          centerObject = (*groupIter)->getCenterMember();
      
          // centers are stored in a separate list
          objectData.object = centerObject;
          objectData.x = *centerObject->gob->sod.x;
          objectData.y = *centerObject->gob->sod.y;
          objectData.oldGroup = true;
          
          centerGroupingList.push_back(objectData);
          objectData.oldGroup = false;
          groupMembers = (*groupIter)->getMembers();
          objectIter = groupMembers.begin();
          while (objectIter != groupMembers.end()) {
            if ((*objectIter) != centerObject){
              // don't add the center object to this list
              objectData.object = *objectIter;
              
              objectData.x = *(*objectIter)->gob->sod.x;
              objectData.y = *(*objectIter)->gob->sod.y;
              groupingList.push_back(objectData);
            }
            objectIter++;
          }
        }
        else {
          // not an old group- don't care about center distinction
          objectData.group = *groupIter;
          objectData.assigned = false;
          objectData.oldGroup = false;
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
      }
      // else it was a group of a different type
    }
    // the lists are now built, centers in a separate list we will
    // treat as being "before" the other list
    
    // now follow the grouping procedure as outlined above
    bool obj1IsACenter = true;
    list<perceptualGroupingStruct>::iterator obj1StructIter, obj2StructIter;
    perceptualGroupingStruct obj1Struct;
    
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
        perceptualGroups.insert(new PerceptualGroup(obj1Struct.object, sorts));
        obj1Struct.group = obj1Struct.object->getPerceptualGroup();
        obj1Struct.assigned = true;
        //cout << "XXX making new group " << (int) obj1Struct.group << endl; 
      }
     
      // iterate through all lower objects to see if they should join the group
      obj2StructIter = obj1StructIter;
      obj2StructIter++;
      if (obj2StructIter == centerGroupingList.end()) {
        obj2StructIter = groupingList.begin();
      }
      while (obj2StructIter != groupingList.end()) {
        if (squaredDistance(obj1Struct.x, obj1Struct.y, 
                            (*obj2StructIter).x, (*obj2StructIter).y)
            <= perceptualGroupingRadius) {
          if ((*obj2StructIter).assigned) {
            // obj2 already has been grouped- groups should merge
            pair<PerceptualGroup*, PerceptualGroup*> groups;

            if (obj1Struct.oldGroup and not (*obj2StructIter).oldGroup) {
              // obj1's group isn't new, and 2's is, keep 1's group
              groups.second = obj1Struct.group;
              groups.first = (*obj2StructIter).group;
            }
            else if (not obj1Struct.oldGroup and (*obj2StructIter).oldGroup) {
              // vice versa
              groups.first = obj1Struct.group;
              groups.second = (*obj2StructIter).group;
            }
            else if (not obj1Struct.oldGroup and not (*obj2StructIter).oldGroup) {
              // arbitrary
              groups.first = obj1Struct.group;
              groups.second = (*obj2StructIter).group;
            }
            else {
              // both old- keep the bigger
              if (obj1Struct.group->getSize() > (*obj2StructIter).group->getSize()) {
                groups.second = obj1Struct.group;
                groups.first = (*obj2StructIter).group;
              }
              else {
                groups.first = obj1Struct.group;
                groups.second = (*obj2StructIter).group;
              }
            }
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
            (*obj2StructIter).oldGroup = obj1Struct.oldGroup;
            
          }
        //  cout << "grouped!" << endl;
        }
        else {
       //   cout << "not grouped!" << endl;
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
    catIter++;
  } // end iterating through all the types that need re-grouping
  
  // do merges- always merge the first group to the second
  
  list<pair<PerceptualGroup*, PerceptualGroup*> >::iterator toMergeIter;
  list<pair<PerceptualGroup*, PerceptualGroup*> >::iterator toMergeIter2;

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
 
  stalePGroupCategories.clear();
  //cout << "XXX regroup done" << endl;
  return;
}

void GroupManager::reGroupInternal() {
  // this is simpler than reGroupPerceptual because we don't care about
  // groups maintaining their identities across cycles
  
  // iterate through stalePGroupCategories set
  //  find all the groups of each type
  //  add the members to a big list
  //  keep a struct for each object:
  //    ptr to the obj, flag for if it has been assigned a group, ptr to group
  //  go through each obj1 in the list
  //    if not flagged, rm from old group and make a new group
  //    check each object (obj2) below in list:
  //      if objs are close and obj2 not flagged, flag obj2 and bring to same group
  //      if objs are close and obj2 flagged, merge groups
  
  set<pair<string, int> >::iterator catIter = staleIGroupCategories.begin();
  internalGroupingStruct objectData;
  list<SoarGameObject*> groupMembers;
  list<InternalGroup*>::iterator groupIter;
  list<SoarGameObject*>::iterator objectIter;
  
  list<internalGroupingStruct> groupingList;

  // save all the to-merge pairs in a list
  // do all the merges at the end
  // this prevents invalid groups in the list (groups are deleted after a merge)
  list<pair<InternalGroup*, InternalGroup*> > toMergeList;
  
  while (catIter != staleIGroupCategories.end()) {
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
 
  staleIGroupCategories.clear();
  //cout << "XXX regroup done" << endl;
  return;
}

void GroupManager::removePerceptualGroup(PerceptualGroup* group) {
  sorts->SoarIO->removeGroup(group);
  sorts->featureMapManager->removeGroup(group);
  delete group;
}

void GroupManager::generatePerceptualGroupData() {
  // iterate through all the groups, if they are stale,
  // refresh them (re-calc stats)
 
  set<PerceptualGroup*>::iterator groupIter;
  list<set<PerceptualGroup*>::iterator> toErase;
  list<set<PerceptualGroup*>::iterator> toReinsert;

  for (groupIter = perceptualGroups.begin(); 
       groupIter != perceptualGroups.end(); 
       groupIter++) {
    if ((*groupIter)->getHasStaleMembers()) {
      if ((*groupIter)->isEmpty()) {
        toErase.push_back(groupIter);
        removePerceptualGroup(*groupIter);
      }
      else {
        (*groupIter)->generateData();
        // groups that have stale members need to be removed and reinserted
        // this is because the set is maintained in order of distance from
        // the focus center, and a stale-membered group could have had this 
        // distance changed
        toReinsert.push_back(groupIter);
      }
    }
  }
  
  for (list<set<PerceptualGroup*>::iterator>::iterator it= toErase.begin();
      it != toErase.end();
      it++) {
    perceptualGroups.erase(*it);
  }
  
  PerceptualGroup* grp;
  for (list<set<PerceptualGroup*>::iterator>::iterator it= toReinsert.begin();
      it != toReinsert.end();
      it++) {
    grp = **it;
    perceptualGroups.erase(*it);
    perceptualGroups.insert(grp);
    // groups have no knowledge of the focus point, only their distance to it.
    grp->calcDistToFocus(focusX, focusY);
  }

  return;
}

void GroupManager::generateInternalGroupData() {
  // iterate through all the groups, if they are stale,
  // refresh them (re-calc stats)
 
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

void GroupManager::makeNewGroup(SoarGameObject* object) {
  // make a new perceptual and internal group for the object
  int size1 = perceptualGroups.size();
  perceptualGroups.insert(new PerceptualGroup(object, sorts));

  // make sure the insertion takes, the compare function could
  // make the elements seem identical, which would not let them both
  // in the set.
  assert(perceptualGroups.size() == (unsigned int)(size1 + 1));
 
  internalGroups.push_back(new InternalGroup(object));
  return;
}

void GroupManager::adjustAttention(bool rebuildFeatureMaps) {
  // iterate through all staleProperties groups, if in attn. range,
  // send params to Soar
  
  set<PerceptualGroup*>::iterator groupIter;
  int i=0;
  for (groupIter = perceptualGroups.begin(); groupIter != perceptualGroups.end(); groupIter++) {
    if (i < numObjects) {
      if (not (*groupIter)->getInSoar()) {
        sorts->SoarIO->addGroup(*groupIter);
        sorts->SoarIO->refreshGroup(*groupIter);
        (*groupIter)->setInSoar(true);
       // cout << "AAA adding group " << (int)*groupIter << ", dist " <<
       //   (*groupIter)->getDistToFocus() << endl;
        
        // recently added / removed from Soar is a stale property, 
        // as far as feature maps are concerned- the inhibition of
        // the group must change
        (*groupIter)->setHasStaleProperties(true);
      }
      else if ((*groupIter)->getHasStaleProperties()) {
        sorts->SoarIO->refreshGroup(*groupIter);
        //cout << "AAA refreshing group " << (int)*groupIter << ", dist " <<
        //  (*groupIter)->getDistToFocus() << endl;
      }
    }
    else { 
      //cout << "AAA not adding group " << (int)*groupIter << ", dist " <<
      //  (*groupIter)->getDistToFocus() << endl;
      if ((*groupIter)->getInSoar() == true) {
        (*groupIter)->setInSoar(false);
        sorts->SoarIO->removeGroup(*groupIter);
        (*groupIter)->setHasStaleProperties(true);
      }
    }
    i++;
  }
  if (rebuildFeatureMaps) {
    // do this after view window changes-
    // the change results in empty feature maps
    // re-add all groups (via refresh) to the feature maps

    // FeatureMapManager can't do this because it doesn't know what all the 
    // groups are.
    
    list <FeatureMap*> emptyList;
    
    for (groupIter = perceptualGroups.begin(); groupIter != perceptualGroups.end(); groupIter++) {
      // sector == -1 means that the group is not in any fmaps
      (*groupIter)->setFMSector(-1);
      sorts->featureMapManager->refreshGroup(*groupIter);
      (*groupIter)->setHasStaleProperties(false);
    }
  }
  else {
    // just refresh groups that changed in the feature maps
    for (groupIter = perceptualGroups.begin(); groupIter != perceptualGroups.end(); groupIter++) {
      if ((*groupIter)->getHasStaleProperties()) {
        sorts->featureMapManager->refreshGroup(*groupIter);
        (*groupIter)->setHasStaleProperties(false);
      }
    }
  }

  sorts->featureMapManager->updateSoar();
  return;
}

GroupManager::~GroupManager() {
  set<PerceptualGroup*>::iterator groupIter;
  for (groupIter = perceptualGroups.begin(); groupIter != perceptualGroups.end(); groupIter++) {
    delete (*groupIter);
  }
}


InternalGroup* GroupManager::getGroupNear(string type, int owner, int x, int y) {
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

void GroupManager::setAllPerceptualCategoriesStale() {
  // add all categories to the stale list
  // (used to force all groups to refresh after grouping params change)
  stalePGroupCategories.clear();
  
  set<PerceptualGroup*>::iterator groupIter;
  // jump iterator between lists

  for (groupIter = perceptualGroups.begin(); groupIter != perceptualGroups.end(); groupIter++) {
    stalePGroupCategories.insert((*groupIter)->getCategory(ownerGrouping));
  }
}

void GroupManager::remakeGroupSet() {
  // if the focus point changes, all groups need to be reinserted in the
  // group set, since it is maintained in order of distance from the center

  set<PerceptualGroup*>::iterator groupIter;
 
  set<PerceptualGroup*, ltGroupPtr> newSet;
  
  for (groupIter = perceptualGroups.begin(); groupIter != perceptualGroups.end(); groupIter++) {
    (*groupIter)->calcDistToFocus(focusX, focusY);
    newSet.insert(*groupIter);
  }

  perceptualGroups = newSet;
}
