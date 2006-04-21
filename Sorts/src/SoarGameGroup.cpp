
#include <assert.h>
#include <vector>
#include <iostream>

// ORTS includes
#include "Object.H"

// our includes
#include "include/SoarGameGroup.h"
#include "include/general.h"
#include "include/Sorts.h"

SoarGameGroup::SoarGameGroup
( SoarGameObject* unit, 
  bool            _mixedType,
  const Sorts*    _sorts )
: sorts(_sorts)
{
  members.insert(unit);
  unit->setGroup(this);
  setHasStaleMembers();
  hasStaleProperties= true;
  centerMember = unit;
  currentMember = unit;
  typeName = unit->gob->bp_name();
  owner = unit->getOwner();
  friendly = unit->isFriendly();
  world = unit->isWorld();
  mixedType = _mixedType;
  inSoar = false;

  fmSector = -1;

  if (not mixedType) {
    minerals = (typeName == "mineral");
    airUnits = (*(unit->gob->sod.zcat) == 1);
    landUnits = (*(unit->gob->sod.zcat) == 3);
  }
  else {
    minerals = false;
    airUnits = false;
    landUnits = false;
  }

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
  ///cout << "XXX created: " << (int) this << endl;
}

SoarGameGroup::~SoarGameGroup() {
  for(list<MapRegion*>::iterator
      i  = regionsOccupied.begin();
      i != regionsOccupied.end();
      i++)
  {
    (*i)->groupExit(this);
  }
  //cout << "XXX destroyed: " << (int) this << endl;
}

void SoarGameGroup::addUnit(SoarGameObject* unit) {

  assert(members.find(unit) == members.end());
  // don't group units from different teams together
  assert(unit->gob->get_int("owner") == owner);

  members.insert(unit); 
  unit->setGroup(this);
  setHasStaleMembers();
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
  setHasStaleMembers();
  
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

void SoarGameGroup::generateData() {
  int x = 0;
  int y = 0;
  set<SoarGameObject*>::iterator currentObject;
  
  soarData.stringIntPairs.clear();
  soarData.stringStringPairs.clear();
  soarData.regionsOccupied.clear();

  currentObject = members.begin();

  int health = 0;
  int speed = 0;
  int size = members.size();
  int mineralCount = 0;
  int running = 0;
  int success = 0;
  int failure = 0;
  int idle = 0;
  int stuck = 0;
    
  moving = false;
  
  int objStatus;

  while (currentObject != members.end()) {
    
    if (canMine) {
      mineralCount += (*currentObject)->gob->get_int("minerals");
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
    stringIntWme.second = mineralCount;
    soarData.stringIntPairs.push_back(stringIntWme);
  }
  
  hasStaleProperties = true;
  hasStaleMembers = false;

  if (speed > 0) {
    moving = true;
  }
   
  return;
}

void SoarGameGroup::updateCenterMember() {
  double shortestDistance 
        = squaredDistance(centerX, centerY, 
          *centerMember->gob->sod.x, *centerMember->gob->sod.y);

  double currentDistance;
  
  set<SoarGameObject*>::iterator currentObject;
  currentObject = members.begin();
  while (currentObject != members.end()) {
    currentDistance = squaredDistance(centerX, centerY, 
                      *(*currentObject)->gob->sod.x, *(*currentObject)->gob->sod.y);
    if (currentDistance < shortestDistance) {
      shortestDistance = currentDistance;
      centerMember = *currentObject;
    }
    
    currentObject++;
  }

  return;
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
 //   cout << "&&& Group " << (int) this << " has exited from region " << (*i)->getId() << endl;
    (*i)->groupExit(this);
  }
  regionsOccupied.clear();
  sorts->mapManager->getRegionsIntersecting(getBoundingBox(), regionsOccupied);

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
 //   cout << "&&& Group " << (int) this << " has entered region " << (*i)->getId() << endl;
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

  setHasStaleMembers();

  return;
}

bool SoarGameGroup::assignAction(ObjectActionType type, list<int> params,
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
  
    
  if (type == OA_MOVE) {
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
  else if (type == OA_MINE) {
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
  else if (type == OA_FREE) {
    sticky = false;
    cout << "UNSTUCK!\n";
  }
  else {
    assert(false);  
  }
  
  return result;
}

bool SoarGameGroup::isEmpty() {
  return (members.empty());
}

bool SoarGameGroup::getHasStaleMembers() {
  return hasStaleMembers;
}
void SoarGameGroup::setHasStaleMembers() {
  //cout << "stale: set so" << endl;
  hasStaleMembers = true;
}

groupPropertyStruct SoarGameGroup::getSoarData() {
  return soarData;
}

bool SoarGameGroup::getHasStaleProperties() {
  return hasStaleProperties;
}

void SoarGameGroup::setHasStaleProperties(bool val) {
  hasStaleProperties = val;
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

bool SoarGameGroup::isMinerals() {
  return minerals;
}
bool SoarGameGroup::isAirUnits() {
  return airUnits;
}
bool SoarGameGroup::isLandUnits() {
  return landUnits;
}

bool SoarGameGroup::isMoving() {
  return moving;
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

void SoarGameGroup::setFMSector(int num) {
  fmSector = num;
}

int SoarGameGroup::getFMSector() {
  return fmSector;
}

void SoarGameGroup::setFMaps(list <FeatureMap*> _fMaps) {
  fMaps = _fMaps;
  return;
}

list <FeatureMap*> SoarGameGroup::getFMaps() {
  return fMaps;
}

void SoarGameGroup::setFMFeatureStrength(int num) {
  fmFeatureStrength = num;
}

int SoarGameGroup::getFMFeatureStrength() {
  return fmFeatureStrength;
}

void SoarGameGroup::calcDistToFocus(int focusX, int focusY) {
  distToFocus = squaredDistance(focusX, focusY, centerX, centerY);
}

int SoarGameGroup::getDistToFocus() {
  return distToFocus;
}
  
bool SoarGameGroup::getInSoar() {
  return inSoar;
}

void SoarGameGroup::setInSoar(bool val) {
  inSoar = val;
  return;
}
