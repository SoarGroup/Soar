#include "include/SoarGameGroup.h"
#include "include/general.h"
#include "include/OrtsInterface.h"
#include <assert.h>
#include <vector>
#include <iostream>

SoarGameGroup::SoarGameGroup(SoarGameObject* unit, OrtsInterface* _ORTSIO)
: ORTSIO(_ORTSIO)
{
  members.insert(unit);
  // capabilities = unit->capabilities;
  unit->setGroup(this);
  setStale();
  staleInSoar= true;
  //type = 0;
  centerMember = unit;
  currentMember = unit;
#ifndef DEBUG_GROUPS
  typeName = unit->gob->bp_name();
#else
  typeName = "unknown";
#endif
  owner = unit->getOwner();
  friendly = unit->isFriendly();
  world = unit->isWorld();
}

void SoarGameGroup::addUnit(SoarGameObject* unit) {
  //capabilities &= unit->capabilities;

  assert(members.find(unit) == members.end());
  // don't group units from different teams together
  assert(unit->gob->get_int("owner") == owner);

  members.insert(unit); 
  unit->setGroup(this);
  //cout << " au! " << endl;
  setStale();
}

bool SoarGameGroup::removeUnit(SoarGameObject* unit) {
  //assert(members.find(unit));
  assert(members.find(unit) != members.end());

  if (centerMember == unit) {
    // make sure center is a valid unit
    // it should be refreshed before use, though
    centerMember = *(members.begin());
  }
  if (currentMember == unit) {
    getNextMember();
    // throw out the result
    // currentMember may still be invalid (empty group)
    // be sure to refresh before using!
  }
  
  members.erase(unit);
  //cout << " ru! " << endl;
  setStale();
  return true;
}

void SoarGameGroup::updateStats(bool saveProps) {
  if (saveProps) {
    soarData.stringIntPairs.clear();
    soarData.stringStringPairs.clear();
  }
  
  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  double health = 0;
  double x = 0;
  double y = 0;
  double speed = 0;
  double size = members.size();

  // this is not a huge problem (just a redundant update), but should not happen
//  assert(stale == true);
  
  while (currentObject != members.end()) {
    // be careful is some numbers are very big for each object and the double could overflow
    
  //  health += *currentObject.getHealth();
    health += (*currentObject)->gob->get_int("hp");
    speed += *(*currentObject)->gob->sod.speed;
    #ifdef DEBUG_GROUPS
    x += (*currentObject)->x;
    y += (*currentObject)->y;
    #else
    x += *(*currentObject)->gob->sod.x;
    y += *(*currentObject)->gob->sod.y;
    #endif
    currentObject++;
  }
  
  health /= size;
  speed /= size;
  x /= size;
  y /= size;

  statistics[GP_NUM_MEMBERS] = size;
  statistics[GP_X_POS] = x;
  statistics[GP_Y_POS] = y;
  statistics[GP_HEALTH] = health;
  statistics[GP_SPEED] = speed;

  if (saveProps) {
    pair<string, int> stringIntWme;
    pair<string, string> stringStringWme;

    stringIntWme.first = "health";
    stringIntWme.second = (int)(health);
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "speed";
    stringIntWme.second = (int)(speed);
    soarData.stringIntPairs.push_back(stringIntWme);

    // how do we want to represent position?
    stringIntWme.first = "x-pos";
    stringIntWme.second = (int)(x);
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "y-pos";
    stringIntWme.second = (int)(y);
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "num_members";
    stringIntWme.second = (int)size;
    soarData.stringIntPairs.push_back(stringIntWme);

    
    stringStringWme.first = "type";
    stringStringWme.second = typeName;
    soarData.stringStringPairs.push_back(stringStringWme);
    
    staleInSoar = true;
    stale = false;
    //cout << "cleared stale" << endl;
  }
  
  // center member calculation- is there a way to do a running calc above?
  #ifdef DEBUG_GROUPS
  double shortestDistance 
        = squaredDistance(x, y, centerMember->x, centerMember->y);
  #else
  double shortestDistance 
        = squaredDistance(x, y, *centerMember->gob->sod.x, *centerMember->gob->sod.y);
  #endif

  double currentDistance;
  currentObject = members.begin();
  while (currentObject != members.end()) {
    #ifdef DEBUG_GROUPS
    currentDistance = squaredDistance(x, y, 
                      (*currentObject)->x, (*currentObject)->y);
    #else
    currentDistance = squaredDistance(x, y, 
                      *(*currentObject)->gob->sod.x, *(*currentObject)->gob->sod.y);
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

  setStale();

  return;
}

bool SoarGameGroup::assignAction(SoarActionType type, list<int> params,
                                 list<SoarGameObject*> targets) { 
  #ifndef DEBUG_GROUPS
  bool result = true;

  cout << "##################" << endl;
  for(list<int>::iterator i = params.begin(); i != params.end(); i++) {
    cout << *i << " ";
  }
  cout << endl;

  list<int>::iterator intIt = params.begin();  
  list<SoarGameObject*>::iterator objectIt = targets.begin();  
  Vector<sint4> tempVec;
  
  
  if (type == SA_MOVE) {
    // the third param is speed, always use 3 (the max)
    assert(params.size() == 2);
    tempVec.push_back(*intIt);
    intIt++;
    tempVec.push_back(*intIt);
    tempVec.push_back(3);
  }
  else if (type == SA_MINE) {
    // get the id of the mineral patch and command center
    // order: x, y, ID, x, y, ID
    assert(targets.size() == 2);
    tempVec.push_back(*(*objectIt)->gob->sod.x);
    tempVec.push_back(*(*objectIt)->gob->sod.y);
    tempVec.push_back(ORTSIO->getID(*objectIt));
    objectIt++;
    tempVec.push_back(*(*objectIt)->gob->sod.x);
    tempVec.push_back(*(*objectIt)->gob->sod.y);
    tempVec.push_back(ORTSIO->getID(*objectIt));
  }
  else {
    assert(false);  
  }

  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  while (currentObject != members.end()) {
    (*currentObject)->issueCommand(type, tempVec);
    currentObject++;
  }
  return result;
  #endif
}

bool SoarGameGroup::isEmpty() {
  return (members.empty());
}

bool SoarGameGroup::getStale() {
  return stale;
}

groupPropertyStruct SoarGameGroup::getSoarData() {
  return soarData;
}

/*void SoarGameGroup::setType(int inType) {
  type = inType;
  return;
}

int SoarGameGroup::getType() {
  return type;
}*/

bool SoarGameGroup::getStaleInSoar() {
  return staleInSoar;
}

void SoarGameGroup::setStaleInSoar(bool val) {
  staleInSoar = val;
}
void SoarGameGroup::setStale() {
  //cout << "stale: set so" << endl;
  stale = true;
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
SoarGameObject* SoarGameGroup::getNextMember() {
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

int SoarGameGroup::getOwner() {
  return owner;
}

bool SoarGameGroup::isFriendly() {
  return friendly;
}

bool SoarGameGroup::isWorld() {
  return world;
}

pair<string, int> SoarGameGroup::getCategory() {
  pair<string, int> cat;
  cat.first = typeName;
  cat.second = owner;

  return cat;
}
