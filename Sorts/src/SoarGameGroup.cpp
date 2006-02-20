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

  bbox.collapse(*unit->gob->sod.x, *unit->gob->sod.y);

  sticky = false;
  commandStatus = GRP_STATUS_IDLE;
  currentCommand = "none";
  if (typeName == "worker") {
    canMine = true;
  }
  else {
    canMine = false;
  }
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
  assert(members.find(unit) != members.end());

  if (currentMember == unit) {
    getNextMember();
    // throw out the result
    // currentMember may still be invalid (empty group)
    // be sure to refresh before using!
  }
  
  members.erase(unit);
  setStale();
  
  if (centerMember == unit) {
    // make sure center is a valid unit
    // it should be refreshed before use, though
    centerMember = *(members.begin());
  }
  return true;
}

void SoarGameGroup::updateStats(bool saveProps) {
  int x = 0;
  int y = 0;
  set<SoarGameObject*>::iterator currentObject;
  if (saveProps) {
    soarData.stringIntPairs.clear();
    soarData.stringStringPairs.clear();
  
    currentObject = members.begin();

    int health = 0;
    int speed = 0;
    int size = members.size();
    int minerals = 0;
    bool running = false;
    bool success = false;
    bool failure = false;
    bool idle = false;
    int objStatus;

    // this is not a huge problem (just a redundant update), but should not happen
  //  assert(stale == true);
    
    while (currentObject != members.end()) {
      // be careful is some numbers are very big for each object and the double could overflow
      
      if (canMine) {
        minerals += (*currentObject)->gob->get_int("minerals");
      }
      health += (*currentObject)->gob->get_int("hp");
      speed += *(*currentObject)->gob->sod.speed;
      #ifdef DEBUG_GROUPS
      x += (*currentObject)->x;
      y += (*currentObject)->y;
      #else
      x += *(*currentObject)->gob->sod.x;
      y += *(*currentObject)->gob->sod.y;
      #endif

      objStatus = (*currentObject)->getStatus();
      if (objStatus == OBJ_RUNNING) {
        running = true;
      }
      else if (objStatus == OBJ_SUCCESS) {
        success = true;
      }
      else if (objStatus == OBJ_FAILURE) {
        failure = true;
      }
      else if (objStatus == OBJ_IDLE) {
        idle = true;
      }
       
      currentObject++;
    }
    
    health /= size;
    speed /= size;
    x /= size;
    y /= size;

    // recalculate bounding box
    bbox.collapse(x, y);
    for(set<SoarGameObject*>::iterator i = members.begin(); 
                                       i != members.end(); i++)
    {
      bbox.accomodate(*(*i)->gob->sod.x, *(*i)->gob->sod.y);
    }

    pair<string, int> stringIntWme;
    pair<string, string> stringStringWme;

    stringIntWme.first = "health";
    stringIntWme.second = health;
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "speed";
    stringIntWme.second = speed;
    soarData.stringIntPairs.push_back(stringIntWme);

    // how do we want to represent position?
    stringIntWme.first = "x-pos";
    stringIntWme.second = x;
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "y-pos";
    stringIntWme.second = y;
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "x-min";
    stringIntWme.second = bbox.xmin;
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "x-max";
    stringIntWme.second = bbox.xmax;
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "y-min";
    stringIntWme.second = bbox.ymin;
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "y-max";
    stringIntWme.second = bbox.ymax;
    soarData.stringIntPairs.push_back(stringIntWme);

    stringIntWme.first = "num_members";
    stringIntWme.second = size;
    soarData.stringIntPairs.push_back(stringIntWme);

    
    stringStringWme.first = "type";
    stringStringWme.second = typeName;
    soarData.stringStringPairs.push_back(stringStringWme);
    

    // command info:
    // show last command, and as many status attributes as are applicable
    // if a group has one member succeed, fail, or still running, that 
    // status is there
    
    if (idle and friendly) {
      stringStringWme.first = "command_status";
      stringStringWme.second = "idle";
      soarData.stringStringPairs.push_back(stringStringWme);
    }

    if (friendly) {
      stringStringWme.first = "command";
      stringStringWme.second = currentCommand;
      soarData.stringStringPairs.push_back(stringStringWme);
    }

    if (running) {
      stringStringWme.first = "command_status";
      stringStringWme.second = "running";
      soarData.stringStringPairs.push_back(stringStringWme);
    } 
    if (success) {
      stringStringWme.first = "command_status";
      stringStringWme.second = "done_success";
      soarData.stringStringPairs.push_back(stringStringWme);
    } 
    if (failure) {
      stringStringWme.first = "command_status";
      stringStringWme.second = "done_failure";
      soarData.stringStringPairs.push_back(stringStringWme);
    } 

    if (canMine) {
      stringIntWme.first = "minerals";
      stringIntWme.second = minerals;
      soarData.stringIntPairs.push_back(stringIntWme);
    }
    
    staleInSoar = true;
    stale = false;
   
    
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

  setStale();

  return;
}

bool SoarGameGroup::assignAction(SoarActionType type, list<int> params,
                                 list<SoarGameGroup*> targets) { 
  #ifndef DEBUG_GROUPS
  bool result = true;

  cout << "##################" << endl;
  for(list<int>::iterator i = params.begin(); i != params.end(); i++) {
    cout << *i << " ";
  }
  cout << endl;

  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  SoarGameObject* targetObj;
  list<int>::iterator intIt;  
  list<SoarGameGroup*>::iterator targetGroupIt;  
  Vector<sint4> tempVec;
  
  while (currentObject != members.end()) {
    intIt = params.begin();  
    targetGroupIt = targets.begin();  
    tempVec.clear();
    
    if (type == SA_MOVE) {
      // the third param is speed, always use 3 (the max)
      assert(params.size() == 2);
      tempVec.push_back(*intIt);
      intIt++;
      tempVec.push_back(*intIt);
      tempVec.push_back(3);

      currentCommand = "move";
      sticky = true;
      // this group is stuck together from now on,
      // until Soar issues an unstick action
    }
    else if (type == SA_MINE) {
      // get the id of the mineral patch and command center
      // order: x, y, ID, x, y, ID
      assert(targets.size() == 2);
      
      targetObj = (*targetGroupIt)->getNextMember();
      tempVec.push_back(*targetObj->gob->sod.x);
      tempVec.push_back(*targetObj->gob->sod.y);
      
      // add in the ID of the next member of the first group
      tempVec.push_back(ORTSIO->getID(targetObj));
      targetGroupIt++;
      targetObj = (*targetGroupIt)->getNextMember();
      tempVec.push_back(*targetObj->gob->sod.x);
      tempVec.push_back(*targetObj->gob->sod.y);
      // add in the ID of the next member of the second group
      tempVec.push_back(ORTSIO->getID(targetObj));

      currentCommand = "mine";
      sticky = true;
      // this group is stuck together from now on,
      // until Soar issues an unstick action
    }
    else if (type == SA_UNSTICK) {
      sticky = false;
    }
    else {
      assert(false);  
    }
    
    (*currentObject)->issueCommand(type, tempVec);
    if ((*currentObject)->getUpdateRequired()) {
      // issueCommand fires an update(), so we need to check
      // if that update caused a required next-cycle update
      ORTSIO->updateNextCycle(*currentObject);
    }
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

Rectangle SoarGameGroup::getBoundingBox() {
  return bbox;
}

bool SoarGameGroup::getSticky() {
  return sticky;
}

void SoarGameGroup::setSticky(bool in) {
  sticky = in;
}
