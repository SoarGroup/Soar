/*
   This file is part of Sorts, an interface between Soar and ORTS.
   (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

   Sorts is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Sorts is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Sorts; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
   */
#include <sys/time.h>

#include "AttackManager.h"
#include "general.h"
#include "AttackManagerRegistry.h"
#include "Circle.h"
#include "Sorts.h"

#include "ScriptObj.H"

#define CLASS_TOKEN "ATKMAN"
#define DEBUG_OUTPUT true 
#include "OutputDefinitions.h"

#define WAIT_RATIO    0.5
#define RESUME_RATIO  0.85

#define USE_CANVAS_ATTACK_MANAGER

#define NO_WAITING


/***
 * Functions used by the old sorts agent. We're leaving them here for now
 * in case they get used later.
 */

inline int min(int a, int b) {
  if (a < b) {
    return a;
  }
  return b;
}

inline int max(int a, int b) {
  if (a > b) {
    return a;
  }
  return b;
}

inline int minRadius(GameObj* gob) {
  switch (*gob->sod.shape) {
    case SHAPE_RECTANGLE:
      return min((*gob->sod.x2-*gob->sod.x1), (*gob->sod.y2-*gob->sod.y1))/2;
    case SHAPE_CIRCLE:
      return *gob->sod.radius;
    default:
      ASSERT(false);
  }
}

inline int maxRadius(GameObj* gob) {
  switch (*gob->sod.shape) {
    case SHAPE_RECTANGLE:
      return max((*gob->sod.x2-*gob->sod.x1), (*gob->sod.y2-*gob->sod.y1))/2;
    case SHAPE_CIRCLE:
      return *gob->sod.radius;
    default:
      ASSERT(false);
      return *gob->sod.radius;
  }
}


Vec2d getClosePos(GameObj* src, GameObj* tgt) {
  int srcRadius = maxRadius(src);
  int tgtRadius = maxRadius(tgt);
  Vec2d srcPos(gobX(src), gobY(src));
  Vec2d tgtPos(gobX(tgt), gobY(tgt));

  return tgtPos - Vec2d(tgtPos - srcPos, srcRadius + tgtRadius);
}

void positionsOnRectangle( int ax, int ay, int tx1, int ty1, int tx2, int ty2, 
  int offset, int distBetween, 
  list<Vec2d>& positions)
{
  ASSERT(distBetween > 0);

  cout << "positions on rectangle\n";
  enum Side { LEFT, RIGHT, TOP, BOTTOM };
  Side s[4];

  int posX1 = tx1 - offset;
  int posY1 = ty1 - offset;
  int posX2 = tx2 + offset;
  int posY2 = ty2 + offset;

  int closeX = -1, closeY = -1;

  if (ax < tx1) {
    if (ay < ty1) {
      // upper left corner
      int diffx = tx1 - ax;
      int diffy = ty1 - ay;
      if (diffx < diffy) {
        s[0] = TOP; s[1] = LEFT; s[2] = BOTTOM; s[3] = RIGHT;
        closeX = tx1;
      }
      else {
        s[0] = LEFT; s[1] = TOP; s[2] = RIGHT; s[3] = BOTTOM;
        closeY = ty1;
      }
    }
    else if (ay > ty2) {
      // lower left corner
      int diffx = tx1 - ax;
      int diffy = ay - ty2;
      if (diffx < diffy) {
        s[0] = BOTTOM; s[1] = LEFT; s[2] = TOP; s[3] = RIGHT;
        closeX = tx1;
      }
      else {
        s[0] = LEFT; s[1] = BOTTOM; s[2] = RIGHT; s[3] = TOP;
        closeY = ty2;
      }
    }
    else {
      // left side
      s[0] = LEFT; s[1] = BOTTOM; s[2] = TOP; s[3] = RIGHT;
      closeY = ay;
    }
  }
  else if (ax > tx2) {
    if (ay < ty1) {
      // upper right corner
      int diffx = ax - tx2;
      int diffy = ty1 - ay;
      if (diffx < diffy) {
        s[0] = TOP; s[1] = RIGHT; s[2] = BOTTOM; s[3] = LEFT;
        closeX = tx2;
      }
      else {
        s[0] = RIGHT; s[1] = TOP; s[2] = LEFT; s[3] = BOTTOM;
        closeY = ty1;
      }
    }
    else if (ay > ty2) {
      // lower right corner
      int diffx = ax - tx2;
      int diffy = ay - ty2;
      if (diffx < diffy) {
        s[0] = BOTTOM; s[1] = RIGHT; s[2] = TOP; s[3] = LEFT;
        closeX = tx2;
      }
      else {
        s[0] = RIGHT; s[1] = BOTTOM; s[2] = LEFT; s[3] = TOP;
        closeY = ty2;
      }
    }
    else {
      // right side
      s[0] = RIGHT; s[1] = TOP; s[2] = BOTTOM; s[3] = LEFT;
      closeY = ay;
    }
  }
  else if (ay < ty1) {
    // top
    s[0] = TOP; s[1] = RIGHT; s[2] = LEFT; s[3] = BOTTOM;
    closeX = ax;
  }
  else {
    // bottom
    s[0] = BOTTOM; s[1] = LEFT; s[2] = RIGHT; s[3] = TOP;
    closeX = ax;
  }

  int d;
  for(int i = 0; i < 4; i++) {
    switch(s[i]) {
      case TOP:
        ASSERT(tx1 <= closeX && closeX <= tx2);
        positions.push_back(Vec2d(closeX, posY1));
        for(d=distBetween; closeX-d>=tx1 || closeX+d<=tx2; d+=distBetween) {
          if (closeX-d >= tx1) {
            positions.push_back(Vec2d(closeX-d, posY1));
          }
          if (closeX+d <= tx2) {
            positions.push_back(Vec2d(closeX+d, posY1));
          }
        }
        closeY = ty1; // for next side
        break;
      case BOTTOM:
        ASSERT(tx1 <= closeX && closeX <= tx2);
        positions.push_back(Vec2d(closeX, posY2));
        for(d=distBetween; closeX-d>=tx1 || closeX+d<=tx2; d+=distBetween) {
          if (closeX-d >= tx1) {
            positions.push_back(Vec2d(closeX-d, posY2));
          }
          if (closeX+d <= tx2) {
            positions.push_back(Vec2d(closeX+d, posY2));
          }
        }
        closeY = ty2;
        break;
      case LEFT:
        ASSERT(ty1 <= closeY && closeY <= ty2);
        positions.push_back(Vec2d(posX1, closeY));
        for(d=distBetween; closeY-d>=ty1 || closeY+d<=ty2; d+=distBetween) {
          if (closeY-d >= ty1) {
            positions.push_back(Vec2d(posX1, closeY-d));
          }
          if (closeY+d <= ty2) {
            positions.push_back(Vec2d(posX1, closeY+d));
          }
        }
        closeX = tx1;
        break;
      case RIGHT:
        ASSERT(ty1 <= closeY && closeY <= ty2);
        positions.push_back(Vec2d(posX2, closeY));
        for(d=distBetween; closeY-d>=ty1 || closeY+d<=ty2; d+=distBetween) {
          if (closeY-d >= ty1) {
            positions.push_back(Vec2d(posX2, closeY-d));
          }
          if (closeY+d <= ty2) {
            positions.push_back(Vec2d(posX2, closeY+d));
          }
        }
        closeX = tx2;
        break;
      default:
        ASSERT(false);
    }
  }
  cout << "end positions on rectangle\n";
}


/*****************************************************************
 * AttackManager::AttackManager                                  *
 *****************************************************************/
AttackManager::AttackManager(const set<SoarGameObject*>& _targets) : targetSet(_targets)
{
  reprioritizeCounter = 0;
  numNewAttackers = 0;
  for(set<SoarGameObject*>::iterator
      i  = targetSet.begin();
      i != targetSet.end();
      ++i)
  {
    targets.insert(pair<SoarGameObject*, AttackTargetInfo>(*i, AttackTargetInfo(*i)));
    idSet.insert((*i)->getID());
    if((*i)->getGob()->bp_name() == "controlCenter") bases.push_back(*i);
  }
  reprioritize();
}

/*****************************************************************
 * AttackManager::~AttackManager                                 *
 *****************************************************************/
AttackManager::~AttackManager() {
  cout << "AttackManagerKilled" << endl;
  int status;
  if (targets.size() == 0) {
    status = 1;
  }
  else {
    status = 0;
  }

  for(list<AttackFSM*>::iterator
      i =  team.begin();
      i != team.end();
      ++i)
  {
#ifdef USE_CANVAS_ATTACK_MANAGER
    //Sorts::canvas.unregisterGob((*i)->getGob());
    Sorts::canvas.resetSGO((*i)->getSGO());
#endif
    (*i)->disown(status);
  }
}

/***
 * AttackManager::registerFSM
 * --------------------------
 * Add a unit to the AttackManager
 */
void AttackManager::registerFSM(AttackFSM* fsm) {
  ASSERT(numNewAttackers > 0);
  numNewAttackers--;
  team.push_back(fsm);
}

/***
 * Method: unregisterFSM
 * ---------------------
 * Removes a unit from the AttackManager.
 */
void AttackManager::unregisterFSM(AttackFSM* fsm) {
  ASSERT(find(team.begin(), team.end(), fsm) != team.end());
  team.erase(find(team.begin(), team.end(), fsm));
  if (fsm->getTarget() != NULL) {
    unassignTarget(fsm);
  }

#ifdef USE_CANVAS_ATTACK_MANAGER
  // in addition to attack reassignments, this is called as a result 
  // of units being killed, in which case the gob is
  // already removed from the canvas
  if (Sorts::canvas.sgoRegistered(fsm->getSGO())) {  
    Sorts::canvas.resetSGO(fsm->getSGO());
  }
#endif
  if (team.size() + numNewAttackers == 0) {
    msg << "team size is now 0, going away.." << endl;
    Sorts::amr->removeManager(this);
    delete this;
  }
}


/***
 * Method: assignTarget
 * --------------------
 *  Given an attacking unit and a target unit, this method
 *  checks that the target being assigned does exist, adds the
 *  attacking unit to the set of units attacking the target object,
 *  and sets the attacking unit's target to be the target unit.
 */
void AttackManager::assignTarget(AttackFSM* fsm, SoarGameObject* target) {
  ASSERT(targets.find(target) != targets.end());
  targets[target].assignAttacker(fsm);
  fsm->setTarget(target);
}

/***
 * Method: unassignTarget
 * ----------------------
 *  Removes an AttackFSM's (attacking finite state machine) target.
 */
void AttackManager::unassignTarget(AttackFSM* fsm) {
  ASSERT(fsm->getTarget() != NULL);
  ASSERT(targets.find(fsm->getTarget()) != targets.end());
  targets[fsm->getTarget()].unassignAttacker(fsm);
  fsm->setTarget(NULL);
}

/***
 * Method: unassignAll
 * -----------------
 *  Given a target, this frees all units that had been
 *  attacking it to do something else. 
 */

void AttackManager::unassignAll(SoarGameObject* target) {
  ASSERT(targets.find(target) != targets.end());

  AttackTargetInfo& info = targets[target];
  for(set<AttackFSM*>::const_iterator
      i  = info.attackers_begin();
      i != info.attackers_end();
      ++i)
  {
    (*i)->setTarget(NULL);
    (*i)->setReassign(true);
  }
  info.unassignAll();
}

/***
 * Method: findTarget
 * ------------------
 *  Given an attack unit, this method attacks nearby enemy
 *  units, making its target selection based on enemy
 *  health and distance.
 *
 *  The segments commented out were used by the old sorts
 *  agent. Satured and unsaturated targetting should be
 *  reintroduced. If attacking formation is reintroduced,
 *  I think it will probably be in the MoveAttackFSM.
 */

bool AttackManager::findTarget(AttackFSM* fsm) {
  msg << "FINDING A TARGET" << endl;
  GameObj* gob = fsm->getGob();
/*  vector<SoarGameObject*> saturated;
  vector<SoarGameObject*> unsaturated;

  for(vector<SoarGameObject*>::iterator
      i  = sortedTargets.begin();
      i != sortedTargets.end();
      ++i)
  {
    if (targets[*i].isSaturated()) {
      saturated.push_back(*i);
    }
    else {
      unsaturated.push_back(*i);
    }
  }
*/
  // try to hit immediately attackable things first
  for(vector<SoarGameObject*>::iterator
      i  = sortedTargets.begin();
      i != sortedTargets.end();
      ++i)
  {
    if (canHit(gob, (*i)->getGob())) {
      assignTarget(fsm, *i);
#ifdef USE_CANVAS_ATTACK_MANAGER
//      Sorts::canvas.trackDestination(fsm->getSGO(), 
//                                     (*i)->getX(), (*i)->getY());
#endif
      msg << "NEARBY TARG" << endl;
      return true;
    }
  }
  return false;
/*
  dbg << "no targets nearby..\n";
  // now try to attack the "best" target
  for(vector<SoarGameObject*>::iterator
      i  = sortedTargets.begin();
      i != sortedTargets.end();
      ++i)
  {
    list<Vec2d> positions;
    attackArcPos(fsm->getGob(), (*i)->getGob(), 0, positions);
    for(list<Vec2d>::iterator
        j  = positions.begin();
        j != positions.end();
        ++j)
    {
      if (fsm->move((int)(*j)(0), (int)(*j)(1), false) == 0) {
        msg <<"Moving to Position: "<<(*j)(0)<<", "<<(*j)(1)<<endl;
        assignTarget(fsm, *i);
#ifdef USE_CANVAS_ATTACK_MANAGER
   //     Sorts::canvas.trackDestination(fsm->getSGO(), 
   //                                    (*i)->getX(), (*i)->getY());
#endif
        dbg << "ARC TARG" << endl;
        return true;
      }
      else {
        msg << "ARC CHECK MOVE FAIL" << endl;
      }
    }
  }

  // finally, just run toward someone, preferrably unsaturated
  //  for(int checkSaturated = 1; checkSaturated >= 0; checkSaturated--) {
  for(vector<SoarGameObject*>::iterator
      i  = sortedTargets.begin();
      i != sortedTargets.end();
      ++i)
  {
    //      if (checkSaturated == 0 || !targets[*i].isSaturated()) {
    GameObj* tgob = (*i)->getGob();
    Vec2d closePos = getClosePos(gob, tgob);
    // force a pathfind, now, since the target must have had lots of
    // failure points nearby, but should be reachable
    msg << "Forcing a pathfind" << endl;
    if (fsm->move((int)closePos(0), (int)closePos(1), true) == 0) {
      assignTarget(fsm, *i);
#ifdef USE_CANVAS_ATTACK_MANAGER
  //    Sorts::canvas.trackDestination(fsm->getSGO(), 
 //                                    (*i)->getX(), (*i)->getY());
#endif
      msg << "LAST RESORT TARG" << endl;
      return true;
    }
    //      }
  }
  //  }
  //  }
  return false;*/
}


/***
 * Method: direct
 * --------------
 * This method is called by each individual AttackFSM every turn,
 * and it determines how these FSM's should behave. From this
 * method, units are ordered to move, fire, or remain still. The
 * return values inform the AttackFSM class about the state of the
 * unit, and decisions on how to move are also based on these
 * return values. Return values are:
 *   > 0    - Success
 *   = 0    - Still running
 *   < 0    - Failure
 */

int AttackManager::direct(AttackFSM* fsm) {

  //Once per frame we should update our target list
  if (reprioritizeCounter != Sorts::frame) {
    dbg << "forced reprioritize!\n";
    reprioritize();
    reprioritizeCounter = Sorts::frame;
  }
  msg << "NUMTARGS: " << targets.size() << endl;
  GameObj* gob = fsm->getGob();
  SoarGameObject* sgob = Sorts::OrtsIO->getSoarGameObject(gob);

  fsm->touch(); // tell the fsm its been updated, so it can mark time

  if (updateTargetList() > 0) {
    msg << "UPDATE TARGET LIST" << endl;
    if (targets.size() == 0) {
      Sorts::amr->removeManager(this);
      delete this;
      return 1;
    }
    reprioritize();
  }

  if (fsm->getTarget() == NULL) findTarget(fsm);

  GameObj* goal = (*sortedTargets.begin())->getGob();
  if (fsm->getTarget() == NULL){
   fsm->move(*goal->sod.x, *goal->sod.y, false);
   return 0;
  } 

  GameObj* tgob = fsm->getTarget()->getGob();
  AttackTargetInfo& info = targets[fsm->getTarget()];

  if (!canHit(gob, tgob) || fsm->badAttack) {
    fsm->move(*goal->sod.x, *goal->sod.y, false);
  }
  else if (!fsm->isFiring() ||  
           sgob->getLastAttacked() != fsm->getTarget()->getID())
  {
    fsm->attack(fsm->getTarget());
  }

  return 0;
}

/***
 * Method: updateTargetList
 * ------------------------
 *  Places the targets in the best order to attack them in.
 */
int AttackManager::updateTargetList() {
  int numVanished = 0;
  map<SoarGameObject*, AttackTargetInfo>::iterator i = targets.begin();
  while (i != targets.end()) {
    if (!Sorts::OrtsIO->isAlive(i->first->getID())) {
      msg << "YYY Unit " << i->first->getID() << " is no longer alive or moved out of view" << endl;

      unassignAll(i->first);
      targetSet.erase(i->first);
      map<SoarGameObject*, AttackTargetInfo>::iterator j = i++;
      targets.erase(j);
      ++numVanished;
    }
    else {
      ++i;
    }
  }

  return numVanished;
}

struct NewTargetCompare {
  Vec2d myPos;

  bool operator()(SoarGameObject* t1, SoarGameObject* t2) {
    // first always prefer those that are armed and shooting over 
    // those that are not
    
    ScriptObj* weapon1 = t1->getGob()->component("weapon");
    ScriptObj* weapon2 = t2->getGob()->component("weapon");
    if (weapon1 == NULL && weapon2 != NULL) {
      return true;
    }
    if (weapon1 != NULL && weapon2 == NULL) {
      return false;
    }

    Vec2d p1(*t1->getGob()->sod.x, *t1->getGob()->sod.y);
    Vec2d p2(*t2->getGob()->sod.x, *t2->getGob()->sod.y);
    
    double dist1 = (myPos - p1).magSq();
    double dist2 = (myPos - p2).magSq();

    if (weapon1 == NULL && weapon2 == NULL){
      if(dist1 < dist2) return true;
      return false;
    }
    
    int range1 = weapon1->get_int("max_ground_range");
    int range2 = weapon2->get_int("max_ground_range");
    
    if (dist1 < dist2 - range2) {
      return true;
    }
    else if (dist2 < dist1 - range1) {
      return false;
    }
    
    double rating1 = weaponDamageRate(t1->getGob()) * t2->getGob()->get_int("hp");
    double rating2 = weaponDamageRate(t2->getGob()) * t1->getGob()->get_int("hp");

    if (rating1 < rating2) {
      return true;
    }
    else if (rating1 == rating2) { // break ties by distance

      if (dist1 < dist2) {
        return true;
      }
      else if (dist1 == dist2) {  // lexicographically break ties
        return (t1->getID() < t2->getID());
      }
    }
    return false;
  }
};

void AttackManager::reprioritize() {
  NewTargetCompare comparator;
  comparator.myPos = findCentroid();

  dbg << "YYY reprioritizing\n";
  sortedTargets.clear();
  sortedTargets.insert(sortedTargets.end(), targetSet.begin(), targetSet.end());
  sort(sortedTargets.begin(), sortedTargets.end(), comparator);
  sort(bases.begin(), bases.end(), comparator);
}


void AttackManager::addNewAttackers(int num) {
  // an action has been assigned to a group, but the FSMs have not been created
  // make sure the manager is not detroyed until this many FSMs have been
  // added-- fix the case where the last attacker is unassigned the same cycle
  // new attackers are added

  ASSERT(numNewAttackers == 0); 

  numNewAttackers = num;
}

// need to be called when we try to add attackers that are already part of this
// manager
void AttackManager::decNewAttackers() {
  numNewAttackers--;
}

int AttackManager::size(){
  return team.size();
}

/***
 * Method: gather
 * --------------
 * Collects all attack units at a point.
 */
void AttackManager::gather(AttackFSM *fsm){
  Vec2d gatherAt = Sorts::amr->mainCentroid();
  msg << "er..." << endl;
  fsm->move((sint4) gatherAt(0), (sint4) gatherAt(1), false);
  msg  << "GATHERING AT " << gatherAt(0) << ", " << gatherAt(1) << endl;
}

Vec2d AttackManager::findCentroid(){
  double xsum = 0, ysum = 0;
  for(list<AttackFSM*>::iterator
      i  = team.begin();
      i != team.end();
      i++)
  {
    xsum += *(*i)->getGob()->sod.x;
    ysum += *(*i)->getGob()->sod.y;
  }
  return Vec2d(xsum / team.size(), ysum / team.size());
}

bool AttackManager:: attackLinePos(int range, list<Vec2d>& positions)
{
  //First, see if the enemy is in a linear formation
  double slope = getSlope();
  //If it's not, then return
  if(slope == (double) NULL){
    dbg<< "SLOPE == NULL" <<endl;
    return false;
  }

  //otherwise, set up a line across from the
  //primary target
  //First, get the midpoint of the line
  Vec2d lineMidpoint = getLineMidpoint(slope, range);
  
  positions.push_back(lineMidpoint);

  //and add 
  for(int i = 1; i < 5; i++)
  {
    Vec2d temp = Vec2d(slope, 8*i);
    positions.push_back(lineMidpoint + temp);
    positions.push_back(lineMidpoint - temp);
  }
  return true;
}

Vec2d AttackManager::getLineMidpoint(double slope, int range){ 
  SoarGameObject* first = sortedTargets[0];
  Vec2d enemyPos = Vec2d(first->getX(), first->getY());

  //Get the distance vector from the targetted attack unit
  Vec2d dist = Vec2d(Vec2d(-slope, 1), range);

  Vec2d loc1 = enemyPos + dist;
  Vec2d loc2 = enemyPos - dist;

  Vec2d centroid = findCentroid();
  
  Vec2d fit1 = loc1 - centroid;
  Vec2d fit2 = loc2 - centroid;

  if(fit1.mag() < fit2.mag())
    return loc1;
  return loc2;
  
}

double AttackManager::getSlope(){
  double ySum, xSum, xxSum, xySum, Sy, Sx,
	 slope, x, y, correlation;
  ySum = xSum = xxSum = xySum = Sy = Sx = 0;
  int count = targetSet.size();

  //calculate the sum of the x's and the sum of the y's,
  //which we'll use to find the slope and intercept
  for(set<SoarGameObject*>::iterator i  = targetSet.begin();
	i != targetSet.end(); ++i)
  {
    x = (*i)->getX();
    y = (*i)->getY();

    ySum += y;
    xSum += x;
    xxSum += x*x;
    xySum += x*y;    
  }
  
  //This is the formula for calculating the slope of a best
  //fit line for a set of points
  slope = (ySum*xxSum - xSum*xySum)/(count*xxSum - xSum*xSum);

  for(set<SoarGameObject*>::iterator i = targetSet.begin();
	i != targetSet.end(); ++i)
  {
    x = (*i)->getX();
    y = (*i)->getY();

    Sy += (y - ySum/count);
    Sx += (x - xSum/count);
  }

  correlation = slope*Sx/Sy;
  if(correlation > 0.8)
    return slope;
  return (double) NULL;
}



void AttackManager::attackArcPos( GameObj* atk, 
                                  GameObj* tgt, 
                                  int layer,
                                  list<Vec2d>& positions) 
{
  dbg << "getting arc positions\n";
  dbg << "atk: " << atk << " tgt: " << tgt << endl;
  int atkRadius = *atk->sod.radius;
  Vec2d aPos(*atk->sod.x, *atk->sod.y);
  Vec2d tPos(*tgt->sod.x, *tgt->sod.y);

  int range;
  if (*tgt->sod.zcat == GameObj::ON_LAND) {
    range = atk->component("weapon")->get_int("max_ground_range") 
      + atkRadius;
  }
  else {
    range = atk->component("weapon")->get_int("max_air_range") 
      + atkRadius;
  }

  range = (int)(range * 0.9); // for safety
  range -= layer * atkRadius * 2;

  list<Vec2d> atkPos;

  /* The attackLinePos method was supposed to force
   * our units into a line formation, but as it isn't
   * working very well, and doesn't appear to be doing much,
   * we're just not going to bother with this for now
   *
  if (attackLinePos(range, atkPos)){
    dbg << "USING THE LINE FORMATION" << endl;
  }
  else*/

  if (*tgt->sod.shape == SHAPE_RECTANGLE) {
    int halfwidth = (*tgt->sod.x2 - *tgt->sod.x1) /  2;
    int halfheight = (*tgt->sod.y2 - *tgt->sod.y1) /  2;

    int minRadius = min(halfwidth, halfheight);
    int maxRadius = max(halfwidth, halfheight);

    dbg << "USING RADIUS " << minRadius << endl;

    if (maxRadius - minRadius < range / 2) {
      // treat this as a circle
      Vec2d closestPos = tPos - Vec2d(tPos - aPos, range + minRadius);
      positionsOnCircle(tPos, closestPos, *atk->sod.radius * 2, atkPos);
    }
    else {
      // treat as a real rectangle
      positionsOnRectangle
        ( *atk->sod.x,
          *atk->sod.y,
          *tgt->sod.x1, 
          *tgt->sod.y1, 
          *tgt->sod.x2, 
          *tgt->sod.y2,
          range,
          atkRadius * 2,
          atkPos );
    }
  }
  else {
    int tgtRadius = *tgt->sod.radius;
    Vec2d closestPos = tPos - Vec2d(tPos - aPos, range + tgtRadius);
    positionsOnCircle(tPos, closestPos, *atk->sod.radius * 4, atkPos);
  }

  for(list<Vec2d>::iterator
      i  = atkPos.begin();
      i != atkPos.end();
      i++) 
  {
    list<GameObj*> collisions;
    Vec2d intPos = i->roundToward(tPos);
    if (0 <= intPos(0) && intPos(0) < Sorts::OrtsIO->getMapXDim() && 
        0 <= intPos(1) && intPos(1) < Sorts::OrtsIO->getMapYDim()) 
    {
      Circle c(intPos(0), intPos(1), atkRadius);
      if (Sorts::spatialDB->hasObjectCollision((int)intPos(0),(int)intPos(1),atkRadius)
          or 
          Sorts::spatialDB->hasTerrainCollision(c)) {
        continue;
      }
      bool slotTaken = false;
      for(list<AttackFSM*>::iterator
          j =  team.begin();
          j != team.end();
          j++) 
      {
        if ((*j)->isMoving()) {
          double d = (intPos - (*j)->getDestination()).magSq();
          double r = *(*j)->getGob()->sod.radius + atkRadius;
          if (d < r * r) {
            slotTaken = true;
            break;
          }
        }
      }
      if (!slotTaken) {
        positions.push_back(intPos);
      }
    }
    //positions.push_back(intPos);
  }
}
