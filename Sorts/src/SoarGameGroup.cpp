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
}

void SoarGameGroup::addUnit(SoarGameObject* unit) {
  //capabilities &= unit->capabilities;

  
  assert(members.find(unit) == members.end());
  members.insert(unit); 
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
  //  x += *currentObject.getX();
  //  y += *currentObject.getY();
//#ifdef DEBUG_GROUPS 
    x += *(*currentObject)->gameObj->sod.x;
    y += *(*currentObject)->gameObj->sod.y;
//#endif
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
    
    staleInSoar = true;
    stale = false;
  }
  
  // center member calculation- is there a way to do a running calc above?
  double shortestDistance 
        = squaredDistance(x, y, *centerMember->gameObj->sod.x, *centerMember->gameObj->sod.y);

  double currentDistance;
  currentObject = members.begin();
  while (currentObject != members.end()) {
    currentDistance = squaredDistance(x, y, 
                      *(*currentObject)->gameObj->sod.x, *(*currentObject)->gameObj->sod.y);
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

bool SoarGameGroup::assignAction(SoarActionType type, list<int> params){ 
  bool result = true;
  
  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  while (currentObject != members.end()) {
    //result &= (*currentObject)->issueCommand(type, params);
    currentObject++;
  }
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
