#include "include/SoarGameGroup.h"
#include "include/general.h"
#include <assert.h>
#include <vector>
#include <iostream>

SoarGameGroup::SoarGameGroup(SoarGameObject* unit) {
  members.insert(unit);
  // capabilities = unit->capabilities;
  unit->setGroup(this);
  stale = true;
  staleInSoar = true;
  type = 0;
  centerMember = unit;
#ifndef DEBUG_GROUPS  
  typeName = unit->gameObj->bp_name();
#else
  typeName = "unknown";
#endif

}

void SoarGameGroup::addUnit(SoarGameObject* unit) {
  //capabilities &= unit->capabilities;

  
  assert(members.find(unit) == members.end());
  members.insert(unit); 
  unit->setGroup(this);
  stale = true;
}

bool SoarGameGroup::removeUnit(SoarGameObject* unit) {
  //assert(members.find(unit));
  assert(members.find(unit) != members.end());

  if (centerMember == unit) {
    // make sure center is a valid unit
    // it should be refreshed before use, though
    centerMember = *(members.begin());
  }
  
  members.erase(unit);
  stale = true;
  return true;
}

void SoarGameGroup::updateStats(bool saveProps) {
  if (saveProps) {
    propList.clear();
  }
  
  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  double health = 0;
  double x = 0;
  double y = 0;
  double size = members.size();

  // this is not a huge problem (just a redundant update), but should not happen
  assert(stale == true);
  
  while (currentObject != members.end()) {
    // be careful is some numbers are very big for each object and the double could overflow
    
  //  health += *currentObject.getHealth();
#ifdef DEBUG_GROUPS 
    x += (*currentObject)->x;
    y += (*currentObject)->y;
#else
    x += *(*currentObject)->gameObj->sod.x;
    y += *(*currentObject)->gameObj->sod.y;
#endif
    currentObject++;
  }
  
  health /= size;
  x /= size;
  y /= size;

  statistics[GP_NUM_MEMBERS] = size;
  statistics[GP_X_POS] = x;
  statistics[GP_Y_POS] = y;
  statistics[GP_HEALTH] = health;

  if (saveProps) {
    pair<string, int> wme;
    wme.first = "health";
    wme.second = (int)(100*health);
    propList.push_back(wme);

    // how do we want to represent positon?
    wme.first = "x_position";
    wme.second = (int)(100*x);
    propList.push_back(wme);
    wme.first = "y_position";
    wme.second = (int)(100*y);
    propList.push_back(wme);
    wme.first = "num_members";
    wme.second = (int)size;
    propList.push_back(wme);
    
    wme.first = "type";
    wme.second = 0;
    if (typeName == "worker") {
      wme.second = 1;
    }
    propList.push_back(wme);
    
    staleInSoar = true;
    stale = false;
  }
  
  // center member calculation- is there a way to do a running calc above?
  #ifdef DEBUG_GROUPS
  double shortestDistance 
        = squaredDistance(x, y, centerMember->x, centerMember->y);
  #else
  double shortestDistance 
        = squaredDistance(x, y, *centerMember->gameObj->sod.x, *centerMember->gameObj->sod.y);
  #endif

  double currentDistance;
  currentObject = members.begin();
  while (currentObject != members.end()) {
    #ifdef DEBUG_GROUPS
    currentDistance = squaredDistance(x, y, 
                      (*currentObject)->x, (*currentObject)->y);
    #else
    currentDistance = squaredDistance(x, y, 
                      *(*currentObject)->gameObj->sod.x, *(*currentObject)->gameObj->sod.y);
    #endif
    if (currentDistance < shortestDistance) {
      shortestDistance = currentDistance;
      centerMember = *currentObject;
    }
    currentObject++;
  }
  
  return;
}

void SoarGameGroup::mergeTo(SoarGameGroup* target) {
  // move all members into other group

  // the group should be destructed after this is called.

  if (target == this) {
    cout << "WARNING: merge of group to self requested. Ignoring." << endl;
    return;
  }
  
  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  while (currentObject != members.end()) {
    target->addUnit(*currentObject);
    currentObject++;
  }

  members.clear();
  statistics[GP_NUM_MEMBERS] = 0;
  stale = true;

  return;
}

bool SoarGameGroup::assignAction(SoarActionType type, list<int> params,
                                 SoarGameObject* target) { 
  bool result = true;

  list<int>::iterator listIt = params.begin();  
  Vector<sint4> tempVec;
  string ORTSCommand;
  
  tempVec.push_back(*listIt);
  listIt++;
  tempVec.push_back(*listIt);
  
  if (type == SA_MOVE) {
    // the third param is speed, always use 3 (the max)
    tempVec.push_back(3);
    // SoarGameObject should really take the SA_MOVE directly
    ORTSCommand = "Move";
  }
  else if (type == SA_MINE) {
    
  
  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  while (currentObject != members.end()) {
    result &= (*currentObject)->issueCommand("Move", tempVec);
    currentObject++;
  }
  return result;
}

bool SoarGameGroup::isEmpty() {
  return (members.empty());
}

bool SoarGameGroup::getStale() {
  return stale;
}

groupPropertyList SoarGameGroup::getProps() {
  return propList;
}

void SoarGameGroup::setType(int inType) {
  type = inType;
  return;
}

int SoarGameGroup::getType() {
  return type;
}

bool SoarGameGroup::getStaleInSoar() {
  return staleInSoar;
}

void SoarGameGroup::setStaleInSoar(bool val) {
  staleInSoar = val;
}

list<SoarGameObject*> SoarGameGroup::getMembers() {
  list<SoarGameObject*> lst; 
  set<SoarGameObject*>::iterator memberIter=members.begin();

  while (memberIter != members.end()) {
    lst.push_back(*memberIter);
    memberIter++;
  }

  return lst;
}

SoarGameObject* SoarGameGroup::getCenterMember() {
  return centerMember;
}

int SoarGameGroup::getSize() {
  return members.size();
}
