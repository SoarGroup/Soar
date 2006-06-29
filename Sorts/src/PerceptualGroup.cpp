
#include <assert.h>
#include <vector>
#include <iostream>

// ORTS includes
#include "Object.H"

// our includes
#include "PerceptualGroup.h"
#include "general.h"
#include "Sorts.h"
#include "AttackManager.h"
#include "AttackManagerRegistry.h"
#include "AttackFSM.h"
#include "MineManager.h"
#include "InfluenceERF.h"
#include "Vec2d.h"

#ifdef USE_CANVAS
#include "SortsCanvas.h"
#define USE_CANVAS_GROUPS
#endif

#define CLASS_TOKEN "GROUP"
#define DEBUG_OUTPUT true 
#include "OutputDefinitions.h"

PerceptualGroup::PerceptualGroup (SoarGameObject* unit) {
  members.insert(unit);
  unit->setPerceptualGroup(this);
  setHasStaleMembers();
  hasStaleProperties= true;
  centerMember = unit;
  typeName = unit->getGob()->bp_name();
  owner = unit->getOwner();
  friendly = unit->isFriendly();
  world = unit->isWorld();
  mixedType = false;
  inSoar = false;
  old = false;
  hasCommand = false;

  fmSector = -1;
  soarID = -1;

  minerals = (typeName == "mineral");
  friendlyWorker = (typeName == "worker");
  airUnits = (*(unit->getGob()->sod.zcat) == 1);
  landUnits = (*(unit->getGob()->sod.zcat) == 3);
  bbox.collapse(*unit->getGob()->sod.x, *unit->getGob()->sod.y);

  sticky = false;
  currentCommand = "none";
  if (typeName == "worker") {
    canMine = true;
  }
  else {
    canMine = false;
  }

  centerX = 0;
  centerY = 0;

  lastQueryResult.x = -1;
  lastQueryResult.y = -1;
  lastQueryDist = 0;

  distToFocus = 0;
  dbg << "created" << endl;
}

PerceptualGroup::~PerceptualGroup() {
  for(list<MapRegion*>::iterator
      i  = regionsOccupied.begin();
      i != regionsOccupied.end();
      i++)
  {
    (*i)->groupExit(this);
  }
  dbg << "destroyed" << endl;
}

void PerceptualGroup::addUnit(SoarGameObject* unit) {

  assert(members.find(unit) == members.end());
  // don't group units from different teams together
  assert(unit->getGob()->get_int("owner") == owner);
  
  if (not mixedType and 
     (unit->getGob()->bp_name() != typeName)) {
    mixedType = true;
    minerals = false;
    friendlyWorker = false;
    airUnits = false;
    landUnits = false;
  } 

  members.insert(unit); 
  unit->setPerceptualGroup(this);
  setHasStaleMembers();
}

bool PerceptualGroup::removeUnit(SoarGameObject* unit) {
  assert(members.find(unit) != members.end());

  members.erase(unit);
  setHasStaleMembers();
  
  if (centerMember == unit) {
    // make sure center is a valid unit
    // it should be refreshed before use, though
    centerMember = *(members.begin());
  }
  return true;
}

void PerceptualGroup::updateBoundingBox() {
  dbg << "updating bbox for " << members.size() << " units.\n";
  if (members.size() == 0) {
    return;
  }
  set<SoarGameObject*>::iterator i = members.begin();
  bbox = (*i)->getBoundingBox();
  ++i;
  while (i != members.end()) {
    bbox.accomodate((*i)->getBoundingBox());
    ++i;
  }
}

// This function calculates all the data that will go on the input link
void PerceptualGroup::generateData() {
  int x = 0;
  int y = 0;
 
  attribs.clear();

  int health = 0;
  int speed = 0;
  int size = members.size();
  int mineralCount = 0;
  int running = 0;
  int success = 0;
  int failure = 0;
  int idle = 0;
  int stuck = 0;

  int shooting = 0;
  int damaged = 0;

  Vec2d avg_heading;

  moving = false;
  
  int objStatus;

  for( set<SoarGameObject*>::iterator 
       currentObject =  members.begin();
       currentObject != members.end();
       currentObject++ )
  {
    GameObj* gob = (*currentObject)->getGob();

    if (canMine) {
      mineralCount += gob->get_int("minerals");
    }
    
    // not everything has health
    // if no hp, just set it to 0
    // get_int asserts if not valid, this is what it calls internally
    if (gob->get_int_ptr("hp") != 0) {
      health += gob->get_int("hp");
    }
    else {
      health += 0;
    }
    speed += *gob->sod.speed;
    x += *gob->sod.x;
    y += *gob->sod.y;

    // average heading over those units that could move
    if (gob->has_attr("heading")) {
      avg_heading += getHeadingVector(gob->get_int("heading"));
    }

    ScriptObj* weapon = gob->component("weapon");
    if (weapon != NULL and weapon->get_int("shooting") == 1) {
      ++shooting;
    }

    if (gob->dir_dmg > 0) {
      ++damaged;
    }

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
  } 
 
  // average all attributes
  x /= size;
  y /= size;

  centerX = x;
  centerY = y;

  updateBoundingBox();
  updateRegionsOccupied();
  
  health /= size;
  attribs.add("health", health);
  attribs.add("speed", speed / size);
  attribs.add("num_members", size);

  attribs.add("enemy", ((not world) and (not friendly)));

  attribs.add("x-pos", x);
  attribs.add("y-pos", y);
  attribs.add("x-min", bbox.xmin);
  attribs.add("x-max", bbox.xmax);
  attribs.add("y-min", bbox.ymin);
  attribs.add("y-max", bbox.ymax);

  attribs.add("dist_to_focus", (float)distToFocus);

  coordinate lastQ = Sorts::gameActionManager->getLastResult();
  if (lastQ.x != lastQueryResult.x ||
      lastQ.y != lastQueryResult.y) {
    lastQueryDist = sqrt(squaredDistance(centerX, centerY, lastQ.x, lastQ.y));
  }
  attribs.add("dist_to_query", (float)lastQueryDist);

  if (mixedType) {
    attribs.add("type", "mixed");
  }
  else {
    attribs.add("type", typeName);
  }

  attribs.add("shooting", shooting);

  attribs.add("taking-damage", damaged);

#if 0
//// Heading info
  Vec2d normHeading = avg_heading.norm();
  attribs.add("heading-i", (float) normHeading(0));
  attribs.add("heading-j", (float) normHeading(1));

//// Threats and support
  int worldId = Sorts::OrtsIO->getWorldId();
  if (owner != worldId) {
    Vec2d avgThreatVec;
    Vec2d avgSupportVec;
    Circle approx = bbox.getCircumscribingCircle();
    int numPlayers = Sorts::OrtsIO->getNumPlayers();
    // this probably isn't right
    bool isGround = (*(*members.begin())->gob->sod.zcat == GameObj::ON_LAND);
    list<GameObj*> collisions;
    int threats = 0;
    int support = 0;
    msg << "-------------" << endl;
    msg << "Group at " << x << "," << y << " under owner " << owner << endl;

    InfluenceERF erf(10, isGround); // look ten viewframe ahead
    Sorts::spatialDB->getCollisions
      ((int) x, (int) y, (int) approx.r, &erf, collisions);

    for(list<GameObj*>::iterator
        i  = collisions.begin();
        i != collisions.end();
        i++) 
    {
      if (*(*i)->sod.owner == worldId) {
        continue;
      }
      else if (*(*i)->sod.owner != owner) {
        threats++;
        avgThreatVec += Vec2d(*(*i)->sod.x - x, *(*i)->sod.y - y);
      }
      else {
        support++;
        avgSupportVec += Vec2d(*(*i)->sod.x - x, *(*i)->sod.y - y);
      }
    }
    // obviously you intersect yourself
    support -= members.size();

    avgThreatVec = avgThreatVec.norm();
    avgSupportVec = avgSupportVec.norm();
    
    attribs.add("num-threats", threats);
    attribs.add("threat-vector-i", (float) avgThreatVec(0));
    attribs.add("threat-vector-j", (float) avgThreatVec(1));
*/ 
    attribs.add("num-supports", support);
    attribs.add("support-vector-i", (float) avgSupportVec(0));
    attribs.add("support-vector-j", (float) avgSupportVec(1));
  }
#endif
  
  // command info:
  // show last command, and as many status attributes as are applicable
  // if a group has one member succeed, fail, or still running, that 
  // status is there

  if (friendly) {

    if (sticky) {
      attribs.add("sticky", 1);
    }
    else {
      attribs.add("sticky", 0);
    }
    
    attribs.add("command", currentCommand);
    attribs.add("command_running", running);
    attribs.add("command_success", success);
    attribs.add("command_failure", failure);
    attribs.add("command_stuck", stuck);
    if (running > 0
        or success > 0
        or failure > 0) {
      hasCommand = true;
    }
  }
  if (canMine) {
    attribs.add("minerals", mineralCount);
  }
  
  hasStaleProperties = true;
  hasStaleMembers = false;

  if (speed > 0) {
    moving = true;
  }

  old = true;
}

void PerceptualGroup::updateCenterLoc() {
  // used in lieu of generateData for internal groups
  // since the only data we actually need is the location of the center
  
  int x = 0;
  int y = 0;
  set<SoarGameObject*>::iterator currentObject;
  
  currentObject = members.begin();

  int size = members.size();

  while (currentObject != members.end()) {
    x += *(*currentObject)->getGob()->sod.x;
    y += *(*currentObject)->getGob()->sod.y;
    currentObject++;
  }
  
  x /= size;
  y /= size;

  centerX = x;
  centerY = y;

  hasStaleProperties = true;
  hasStaleMembers = false;

  return;
}

void PerceptualGroup::updateCenterMember() {
  double shortestDistance 
        = squaredDistance(centerX, centerY, 
          *centerMember->getGob()->sod.x, *centerMember->getGob()->sod.y);

  double currentDistance;
  
  set<SoarGameObject*>::iterator currentObject;
  currentObject = members.begin();
  while (currentObject != members.end()) {
    currentDistance = squaredDistance(centerX, centerY, 
                      *(*currentObject)->getGob()->sod.x, *(*currentObject)->getGob()->sod.y);
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
void PerceptualGroup::updateRegionsOccupied() {
  // for now, just leave all regions first and then recompute
  // which ones it enters, even if this may mean exiting and
  // entering the same region redundently
  for( list<MapRegion*>::iterator 
       i  = regionsOccupied.begin();
       i != regionsOccupied.end();
       i++ )
  {
    (*i)->groupExit(this);
  }
  regionsOccupied.clear();
  Sorts::mapManager->getRegionsIntersecting(getBoundingBox(), regionsOccupied);

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
    (*i)->groupEnter(this);
  }
}

void PerceptualGroup::getRegionsOccupied(list<int>& regions) {
  regions.clear();
  for( list<MapRegion*>::iterator 
       i  = regionsOccupied.begin();
       i != regionsOccupied.end();
       i++ )
  {
    regions.push_back((*i)->getId());
  }
}

void PerceptualGroup::mergeTo(PerceptualGroup* target) {
  // move all members into other group

  // the group should be destructed after this is called.

  if (target == this) {
    msg << "WARNING: merge of group to self requested. Ignoring." << endl;
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

bool PerceptualGroup::assignAction(ObjectActionType type, list<int> params,
                                 list<PerceptualGroup*> targets) { 
  bool result = true;

  set<SoarGameObject*>::iterator currentObject;
  
  list<int>::iterator intIt;  
  list<PerceptualGroup*>::iterator targetGroupIt;  
  Vector<sint4> tempVec;
  list<SoarGameObject*> sgoList;
  
  switch (type) {
    case OA_MOVE:
      // the third param is precision
      assert(params.size() >= 2);
      for (intIt = params.begin();
          intIt != params.end();
          intIt++) {
        tempVec.push_back(*intIt);
      }

      currentCommand = "move";
      sticky = true;
      // this group is stuck together from now on,
      // until Soar issues an unstick action
      for (currentObject = members.begin();
           currentObject != members.end();
           currentObject++) {
        (*currentObject)->assignAction(type, tempVec);
      } 
      break;

    case OA_MINE:
      assert(targets.size() == 0);
      currentCommand = "mine";
      tempVec.clear();
      sticky = true;
      // this group is stuck together from now on,
      // until Soar issues an unstick action
      
      getMembers(sgoList);
      Sorts::mineManager->prepareRoutes(sgoList);
      
      for (currentObject = members.begin();
           currentObject != members.end();
           currentObject++) {
        (*currentObject)->assignAction(type, tempVec);
      }
      break;

    case OA_FREE:
      sticky = false;
      currentCommand = "none";
      hasStaleMembers = true;
      for (currentObject = members.begin();
           currentObject != members.end();
           currentObject++) {
        (*currentObject)->endCommand();
      }
      break;
    case OA_STICK:
      sticky = true;
      msg << "STUCK!\n";
      hasStaleMembers = true;
      break;
    case OA_SEVER: {
      // remove n closest members near x,y, add to new sticky group
      assert(params.size() == 3); // n,x,y
      intIt = params.begin();
      
      int numMembers = *intIt;
      coordinate c;
      intIt++;
      c.x = *intIt;
      intIt++;
      c.y = *intIt;
      SoarGameObject* member;
     
      if (numMembers == 0) return false;
      member = getMemberNear(c);
      if (member == NULL) return false;
      removeUnit(member);
      Sorts::pGroupManager->makeNewGroup(member);
      PerceptualGroup* newGroup = member->getPerceptualGroup();
      member->assignAction(OA_IDLE, tempVec);
      // make the new group idle
      
      newGroup->setSticky(true);
      for (int i=1; i<numMembers; i++) {
        member = getMemberNear(c);
        if (member == NULL) return false;
        removeUnit(member);
        newGroup->addUnit(member);
        member->assignAction(OA_IDLE, tempVec);
      }
      newGroup->setCommandString("severed");
      newGroup->setSticky(true);
      }
      break;
      
    case OA_JOIN: {
      // join two groups with the same command
      // mixed types will be handled, though
      assert(targets.size() == 1);
      PerceptualGroup* tgt = *targets.begin();
      assert(currentCommand == tgt->getCommandString());
      this->mergeTo(tgt);
      tgt->setHasStaleMembers();
      break;
    }

    case OA_ATTACK: {
      currentCommand = "attack";
      int managerId = Sorts::amr->assignManager(targets, members.size());
      tempVec.clear();
      tempVec.push_back(managerId);

      sticky = true;

      for(set<SoarGameObject*>::iterator
          i =  members.begin();
          i != members.end();
          i++)
      {
        (*i)->assignAction(type, tempVec);
      }
      break;
    }
    case OA_BUILD:
      assert(params.size() == 4);
      currentCommand = "build";
      // building type, x, y, usebuffer
      intIt = params.begin();
      tempVec.clear();
      tempVec.push_back(*intIt);
      ++intIt;
      tempVec.push_back(*intIt);
      ++intIt;
      tempVec.push_back(*intIt);
      ++intIt;
      tempVec.push_back(*intIt);
      sticky = true;
      
      for (currentObject = members.begin();
           currentObject != members.end();
           currentObject++) {
        (*currentObject)->assignAction(type, tempVec);
      }
      break;

    case OA_TRAIN:
      assert(params.size() == 3);
      currentCommand = "train";
      // type to train, number, usebuffer
      intIt = params.begin();
      tempVec.clear();
      tempVec.push_back(*intIt);
      ++intIt;
      tempVec.push_back(*intIt);
      ++intIt;
      tempVec.push_back(*intIt);
      sticky = true;
      for (currentObject = members.begin();
           currentObject != members.end();
           currentObject++) {
        (*currentObject)->assignAction(type, tempVec);
      }
      break;

    case OA_STOP:
      sticky = false;
      tempVec.clear();
      for (currentObject  = members.begin();
           currentObject != members.end();
           currentObject++)
      {
        (*currentObject)->assignAction(type, tempVec);
      }
      hasStaleMembers = true;
      break;

    default:
      assert(false);  
  }
  
  return result;
}

bool PerceptualGroup::isEmpty() {
  return (members.empty());
}

bool PerceptualGroup::getHasStaleMembers() {
  return hasStaleMembers;
}
void PerceptualGroup::setHasStaleMembers() {
  hasStaleMembers = true;
}

const AttributeSet& PerceptualGroup::getAttributes() {
  return attribs;
}

bool PerceptualGroup::getHasStaleProperties() {
  return hasStaleProperties;
}

void PerceptualGroup::setHasStaleProperties(bool val) {
  hasStaleProperties = val;
}

void PerceptualGroup::getMembers(list<SoarGameObject*>& memberList) {
  memberList.clear();
  memberList.insert(memberList.begin(), members.begin(), members.end());
}

SoarGameObject* PerceptualGroup::getCenterMember() {
  return centerMember;
}

int PerceptualGroup::getSize() {
  return members.size();
}

int PerceptualGroup::getOwner() {
  return owner;
}

bool PerceptualGroup::isFriendly() {
  return friendly;
}

bool PerceptualGroup::isWorld() {
  return world;
}

bool PerceptualGroup::isMinerals() {
  return minerals;
}
bool PerceptualGroup::isAirUnits() {
  return airUnits;
}
bool PerceptualGroup::isLandUnits() {
  return landUnits;
}

bool PerceptualGroup::isMoving() {
  return moving;
}

pair<string, int> PerceptualGroup::getCategory(bool ownerGrouping) {
  // get the group's category-
  // category is the type and owner, if ownerGrouping is not used,
  // otherwise just the owner alone
  
  pair<string, int> cat;
  if (not ownerGrouping) {
    cat.first = typeName;
  }
  else {
    cat.first = "";
  }
  cat.second = owner;

  return cat;
}

Rectangle& PerceptualGroup::getBoundingBox() {
  return bbox;
}

bool PerceptualGroup::getSticky() {
  return sticky;
}

void PerceptualGroup::setSticky(bool in) {
  sticky = in;
}
void PerceptualGroup::getCenterLoc(int& x, int& y) {
  x = centerX;
  y = centerY;
}

void PerceptualGroup::getLocNear(int x, int y, int& locX, int &locY) {
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

void PerceptualGroup::setFMSector(int num) {
  fmSector = num;
}

int PerceptualGroup::getFMSector() {
  return fmSector;
}

void PerceptualGroup::setFMaps(list <FeatureMap*> _fMaps) {
  fMaps = _fMaps;
  return;
}

list <FeatureMap*> PerceptualGroup::getFMaps() {
  return fMaps;
}

void PerceptualGroup::setFMFeatureStrength(int num) {
  fmFeatureStrength = num;
}

int PerceptualGroup::getFMFeatureStrength() {
  return fmFeatureStrength;
}

void PerceptualGroup::calcDistToFocus(int focusX, int focusY) {
  distToFocus = squaredDistance(focusX, focusY, centerX, centerY);
}

double PerceptualGroup::getDistToFocus() {
  return distToFocus;
}
  
bool PerceptualGroup::getInSoar() {
  return inSoar;
}

void PerceptualGroup::setInSoar(bool val) {
  inSoar = val;
  return;
}

bool PerceptualGroup::isOld() {
  return old;
}


bool PerceptualGroup::isFriendlyWorker() {
  return friendlyWorker;
}

SoarGameObject* PerceptualGroup::getMemberNear(coordinate c) {
  double currentDistance;
  double closestDistance = 999999999;
  SoarGameObject* closestMember = NULL;
  
  for (set<SoarGameObject*>::iterator it = members.begin();
       it != members.end();
       it++) {
    currentDistance = coordDistanceSq((*it)->getLocation(), c);
    if (currentDistance < closestDistance) {
      closestDistance = currentDistance;
      closestMember = *it;
    }
  }

  return closestMember;
}

int PerceptualGroup::getSoarID() {
  return soarID;
}

void PerceptualGroup::setSoarID(int id) {
  soarID = id;
}
