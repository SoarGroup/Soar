
#include <assert.h>
#include <vector>
#include <iostream>

// ORTS includes
#include "Object.H"

// our includes
#include "InternalGroup.h"
#include "general.h"
#include "SoarGameObject.h"

InternalGroup::InternalGroup
( SoarGameObject* unit) 
{
  members.insert(unit);
  unit->setInternalGroup(this);
  setHasStaleMembers();
  currentMember = unit;
  typeName = unit->gob->bp_name();
  owner = unit->getOwner();

  centerX = 0;
  centerY = 0;
}

InternalGroup::~InternalGroup() {
}

void InternalGroup::addUnit(SoarGameObject* unit) {

  assert(members.find(unit) == members.end());
  // don't group units from different teams together
  assert(unit->gob->get_int("owner") == owner);
  
  members.insert(unit); 
  unit->setInternalGroup(this);
  setHasStaleMembers();
}

bool InternalGroup::removeUnit(SoarGameObject* unit) {
  assert(members.find(unit) != members.end());

  if (currentMember == unit) {
    getNextMember();
    // throw out the result
    // currentMember may still be invalid (empty group)
    // be sure to refresh before using!
  }
  
  members.erase(unit);
  setHasStaleMembers();
  
  return true;
}

void InternalGroup::updateCenterLoc() {
  // used in lieu of generateData for internal groups
  // since the only data we actually need is the location of the center
  
  int x = 0;
  int y = 0;
  set<SoarGameObject*>::iterator currentObject;
  
  currentObject = members.begin();

  int size = members.size();

  while (currentObject != members.end()) {
    x += *(*currentObject)->gob->sod.x;
    y += *(*currentObject)->gob->sod.y;
    currentObject++;
  }
  
  x /= size;
  y /= size;

  centerX = x;
  centerY = y;

  hasStaleMembers = false;

  return;
}

void InternalGroup::mergeTo(InternalGroup* target) {
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

  setHasStaleMembers();

  return;
}

bool InternalGroup::isEmpty() {
  return (members.empty());
}

bool InternalGroup::getHasStaleMembers() {
  return hasStaleMembers;
}
void InternalGroup::setHasStaleMembers(bool val) {
  hasStaleMembers = val;
}

void InternalGroup::setHasStaleMembers() {
  //cout << "stale: set so" << endl;
  hasStaleMembers = true;
}

void InternalGroup::getMembers(list<SoarGameObject*> memberList) {
  memberList.clear();
  memberList.insert(memberList.begin(), members.begin(), members.end());
}

int InternalGroup::getSize() {
  return members.size();
}
SoarGameObject* InternalGroup::getNextMember() {
  set<SoarGameObject*>::iterator objIt;
  objIt = members.find(currentMember);
  assert(objIt != members.end());
  
  objIt++;
  if (objIt == members.end()) {
    objIt = members.begin();
  }
  currentMember = *objIt;
  return currentMember;
}

pair<string, int> InternalGroup::getCategory() {
  pair<string, int> cat;
  cat.first = typeName;
  cat.second = owner;

  return cat;
}

void InternalGroup::getCenterLoc(int& x, int& y) {
  x = centerX;
  y = centerY;
}

