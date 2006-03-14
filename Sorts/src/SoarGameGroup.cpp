#include "include/SoarGameGroup.h"
#include "include/general.h"
#include "include/OrtsInterface.h"
#include <assert.h>
#include <vector>
#include <iostream>

SoarGameGroup::SoarGameGroup( SoarGameObject* unit, 
                              OrtsInterface*  _ORTSIO,
                              MapManager*     _mapManager )
: ORTSIO(_ORTSIO), mapManager(_mapManager)
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

  centerX = 0;
  centerY = 0;
}

SoarGameGroup::~SoarGameGroup() {
  for(list<MapRegion*>::iterator
      i  = regionsOccupied.begin();
      i != regionsOccupied.end();
      i++)
  {
    (*i)->groupExit(this);
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

void SoarGameGroup::updateBoundingBox() {
  bbox = (*members.begin())->getBoundingBox();
  for(set<SoarGameObject*>::iterator 
      i  = members.begin(); 
      i != members.end(); 
      i++)
  {
    bbox.accomodate((*i)->getBoundingBox());
  }
}

void SoarGameGroup::updateStats(bool saveProps) {
  int x = 0;
  int y = 0;
  set<SoarGameObject*>::iterator currentObject;
  if (saveProps) {
    soarData.stringIntPairs.clear();
    soarData.stringStringPairs.clear();
    soarData.regionsOccupied.clear();

    currentObject = members.begin();

    int health = 0;
    int speed = 0;
    int size = members.size();
    int minerals = 0;
    int running = 0;
    int success = 0;
    int failure = 0;
    int idle = 0;
    int stuck = 0;
    
    int objStatus;

    // this is not a huge problem (just a redundant update), but should not happen
  //  assert(stale == true);
    
    while (currentObject != members.end()) {
      // be careful is some numbers are very big for each object and the double could overflow
      
      if (canMine) {
        minerals += (*currentObject)->gob->get_int("minerals");
      }
      
      // not everything has health
      // if no hp, just set it to 0
      // get_int asserts if not valid, this is what it calls internally
      if ((*currentObject)->gob->get_int_ptr("hp") != 0) {
        health += (*currentObject)->gob->get_int("hp");
      }
      else {
        health += 0;
      }
      speed += *(*currentObject)->gob->sod.speed;
      x += *(*currentObject)->gob->sod.x;
      y += *(*currentObject)->gob->sod.y;

      objStatus = (*currentObject)->getStatus();
      if (objStatus == OBJ_RUNNING) {
        running++;
      }
      else if (objStatus == OBJ_SUCCESS) {
        success++;
      }
      else if (objStatus == OBJ_FAILURE) {
        failure++;
      }
      else if (objStatus == OBJ_IDLE) {
        idle++;
      }
      else if (objStatus == OBJ_STUCK) {
        stuck++;
      }
       
      currentObject++;
    } // while (currentObject != members.end())
    
    health /= size;
    speed /= size;
    x /= size;
    y /= size;

    centerX = x;
    centerY = y;

    updateBoundingBox();
    updateRegionsOccupied();

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

    if (friendly) {
      stringStringWme.first = "command";
      stringStringWme.second = currentCommand;
      soarData.stringStringPairs.push_back(stringStringWme);

      stringIntWme.first = "command_running";
      stringIntWme.second = running;
      soarData.stringIntPairs.push_back(stringIntWme);

      stringIntWme.first = "command_success";
      stringIntWme.second = success;
      soarData.stringIntPairs.push_back(stringIntWme);
      
      stringIntWme.first = "command_failure";
      stringIntWme.second = failure;
      soarData.stringIntPairs.push_back(stringIntWme);
      
      stringIntWme.first = "command_stuck";
      stringIntWme.second = stuck;
      soarData.stringIntPairs.push_back(stringIntWme);
      
    }
    if (canMine) {
      stringIntWme.first = "minerals";
      stringIntWme.second = minerals;
      soarData.stringIntPairs.push_back(stringIntWme);
    }
    
    staleInSoar = true;
    stale = false;
   
  } // if(saveProps)
  
  // center member calculation- is there a way to do a running calc above?
  double shortestDistance 
        = squaredDistance(x, y, *centerMember->gob->sod.x, *centerMember->gob->sod.y);

  double currentDistance;
  currentObject = members.begin();
  while (currentObject != members.end()) {
    currentDistance = squaredDistance(x, y, 
                      *(*currentObject)->gob->sod.x, *(*currentObject)->gob->sod.y);
    if (currentDistance < shortestDistance) {
      shortestDistance = currentDistance;
      centerMember = *currentObject;
    }
    currentObject++;
  }
}

/**************************************************
 *                                                *
 * Update the regions the group occupies          *
 *                                                *
 **************************************************/
void SoarGameGroup::updateRegionsOccupied() {
  // for now, just leave all regions first and then recompute
  // which ones it enters, even if this may mean exiting and
  // entering the same region redundently
  for( list<MapRegion*>::iterator 
       i  = regionsOccupied.begin();
       i != regionsOccupied.end();
       i++ )
  {
    cout << "&&& Group " << (int) this << " has exited from region " << (*i)->getId() << endl;
    (*i)->groupExit(this);
  }
  regionsOccupied.clear();
  mapManager->getRegionsIntersecting(getBoundingBox(), regionsOccupied);

  for( list<MapRegion*>::iterator
       i  = regionsOccupied.begin();
       i != regionsOccupied.end();
       i++ )
  {
    list<MapRegion*>::iterator j = i;
    j++;
    for( ; j != regionsOccupied.end(); j++) {
      if (*i == *j) {
        assert(false);
      }
    }
  }

  for( list<MapRegion*>::iterator 
       i  = regionsOccupied.begin();
       i != regionsOccupied.end();
       i++ )
  {
    cout << "&&& Group " << (int) this << " has entered region " << (*i)->getId() << endl;
    (*i)->groupEnter(this);
    soarData.regionsOccupied.push_back((*i)->getId());
  }
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
  bool result = true;

 /* cout << "##################" << endl;
  for(list<int>::iterator i = params.begin(); i != params.end(); i++) {
    cout << *i << " ";
  }
  cout << endl;
*/
  set<SoarGameObject*>::iterator currentObject;
  
  list<int>::iterator intIt;  
  list<SoarGameGroup*>::iterator targetGroupIt;  
  Vector<sint4> tempVec;
  
    
  if (type == SA_MOVE) {
    // the third param is speed, always use 3 (the max)
    assert(params.size() == 2);
    intIt = params.begin();  
    tempVec.push_back(*intIt);
    intIt++;
    tempVec.push_back(*intIt);
    tempVec.push_back(3);

    currentCommand = "move";
    sticky = true;
    // this group is stuck together from now on,
    // until Soar issues an unstick action
    for (currentObject = members.begin();
         currentObject != members.end();
         currentObject++) {
      (*currentObject)->issueCommand(type, tempVec);
    } 
  }
  else if (type == SA_MINE) {
    // targets are the mineral patch x, y, and command center
    // in that order

    // FSM parameters:
    // x,y of mineral-patch edge relative to command-center
    // x,y of command-center edge relative to mineral-patch
    assert(targets.size() == 2);
    targetGroupIt = targets.begin();
    SoarGameGroup* mpGroup;
    SoarGameGroup* ccGroup;
    mpGroup = *targetGroupIt;
    targetGroupIt++;
    ccGroup = *targetGroupIt;
   
    // get the centers of the mineral patch and command center
    int mpX, mpY;
    mpGroup->getCenterLoc(mpX, mpY);
    
    int ccX, ccY;
    ccGroup->getCenterLoc(ccX, ccY);
    
    int mpEdgeX, mpEdgeY;
    int ccEdgeX, ccEdgeY;
    
    // get the location on the edge of the mineral patch the worker
    // should walk to
    mpGroup->getLocNear(ccX, ccY, mpEdgeX, mpEdgeY);
    // likewise for the edge of the command center
    ccGroup->getLocNear(mpX, mpY, ccEdgeX, ccEdgeY);

    // fill in FSM parameters
    tempVec.push_back(mpEdgeX);
    tempVec.push_back(mpEdgeY);
    tempVec.push_back(ccEdgeX);
    tempVec.push_back(ccEdgeY);

    currentCommand = "mine";
    sticky = true;
    // this group is stuck together from now on,
    // until Soar issues an unstick action
    
    for (currentObject = members.begin();
         currentObject != members.end();
         currentObject++) {
      (*currentObject)->issueCommand(type, tempVec);
    } 
  }
  else if (type == SA_UNSTICK) {
    sticky = false;
  }
  else {
    assert(false);  
  }
  
  return result;
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
  // checking if everything is in the bounding box, because I'm not sure
  // if the bounding box is getting updated often enough
  for(set<SoarGameObject*>::iterator
      i  = members.begin();
      i != members.end();
      i++)
  {
    int x = *(*i)->gob->sod.x;
    int y = *(*i)->gob->sod.y;
    assert(bbox.xmin <= x && x <= bbox.xmax &&
           bbox.ymin <= y && y <= bbox.ymax);
  }
  return bbox;
}

bool SoarGameGroup::getSticky() {
  return sticky;
}

void SoarGameGroup::setSticky(bool in) {
  sticky = in;
}
void SoarGameGroup::getCenterLoc(int& x, int& y) {
  x = centerX;
  y = centerY;
}

void SoarGameGroup::getLocNear(int x, int y, int& locX, int &locY) {
  // return a location on the bounding box of this group,
  // close to the given point (x,y)
  
  // if point is inside bounding box, just return it

  // better way to do this?
  int adjust = rand() % 50;
  int adjust2 = rand() % 50;

  if (x < bbox.xmin) {
    locX = bbox.xmin - adjust;
    if (y < bbox.ymin) {
      locY = bbox.ymin - adjust2;
    }
    else if (y > bbox.ymax) {
      locY = bbox.ymax + adjust2;
    }
    else {
      locY = y;
    }
  }
  else if (x > bbox.xmax) {
    locX = bbox.xmax + adjust;
    if (y < bbox.ymin) {
      locY = bbox.ymin - adjust2;
    }
    else if (y > bbox.ymax) {
      locY = bbox.ymax + adjust2;
    }
    else {
      locY = y;
    }
  }
  else {
    locX = x;
    if (y < bbox.ymin) {
      locY = bbox.ymin - adjust;
    }
    else if (y > bbox.ymax) {
      locY = bbox.ymax + adjust;
    }
    else {
      locY = y;
    }
  }

  return;
}
