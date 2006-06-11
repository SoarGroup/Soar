#include <sys/time.h>

#include "AttackManager.h"
#include "general.h"
#include "AttackManagerRegistry.h"
#include "Circle.h"
#include "Sorts.h"

#include "ScriptObj.H"

#define msg cout << "AttackManager.cpp: "

#ifdef USE_CANVAS
#define USE_CANVAS_ATTACK_MANAGER
#endif

// fake version
/*
bool attackArcPos(GameObj*atk, GameObj* tgt, Circle& pos) {
  pos.x = *tgt->sod.x;
  pos.y = *tgt->sod.y;
  return true;
}
*/

void AttackManager::attackArcPos
( GameObj* atk, 
  GameObj* tgt, 
  list<Vec2d>& positions) 
{
  int range;
  int atkRadius = *atk->sod.radius;
  int tgtRadius = *tgt->sod.radius;
  Vec2d aPos(*atk->sod.x, *atk->sod.y);
  Vec2d tPos(*tgt->sod.x, *tgt->sod.y);
  if (*tgt->sod.zcat == GameObj::ON_LAND) {
    range = atk->component("weapon")->get_int("max_ground_range") 
      + atkRadius + tgtRadius;
  }
  else {
    range = atk->component("weapon")->get_int("max_air_range") 
      + atkRadius + tgtRadius;
  }

  range = range - 3; // for safety

  Vec2d closestPos = tPos - Vec2d(tPos - aPos, range);
  list<Vec2d> atkPos;
  positionsOnCircle(tPos, closestPos, *atk->sod.radius * 2, atkPos);

  for(list<Vec2d>::iterator
      i  = atkPos.begin();
      i != atkPos.end();
      i++) 
  {
    list<GameObj*> collisions;
    Vec2d intPos = i->roundToward(tPos);
    if (0 <= intPos(0) && intPos(0) <= Sorts::OrtsIO->getMapXDim() && 
        0 <= intPos(1) && intPos(1) <= Sorts::OrtsIO->getMapYDim()) 
    {
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
        Sorts::spatialDB->getCollisions(
          intPos(0), intPos(1), atkRadius, NULL, collisions);
        if (collisions.size() == 0) { // there's no collision
          positions.push_back(intPos);
        }
      }
    }
  }
}

AttackManager::AttackManager(const set<SoarGameObject*>& _targets)
: targets(_targets)
{
  reprioritize();

#ifdef USE_CANVAS_ATTACK_MANAGER
  Uint8 r = (Uint8) (((int) this) % 156) + 100;
  for(set<SoarGameObject*>::iterator
      i  = targets.begin();
      i != targets.end();
      ++i)
  {
    if (!Sorts::canvas.gobRegistered((*i)->getGob())) {
      Sorts::canvas.registerGob((*i)->getGob());
      Sorts::canvas.setColor((*i)->getGob(), r, 0, 0);
    }
  }
#endif
}

AttackManager::~AttackManager() {
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
    Sorts::canvas.unregisterGob((*i)->getGob());
#endif
    (*i)->disown(status);
  }

  for(map<SoarGameObject*, list<AttackFSM*>*>::iterator
      i  = targetAssignments.begin();
      i != targetAssignments.end();
      ++i)
  {
    delete i->second;
  }
}

void AttackManager::registerFSM(AttackFSM* fsm) {
  team.push_back(fsm);
#ifdef USE_CANVAS_ATTACK_MANAGER
  Sorts::canvas.registerGob(fsm->getGob());
  Uint8 b = (Uint8) (((int) this) % 156) + 100;
  Sorts::canvas.setColor(fsm->getGob(), 0, 0, b);
#endif
}

void AttackManager::unregisterFSM(AttackFSM* fsm) {
  assert(find(team.begin(), team.end(), fsm) != team.end());
  team.erase(find(team.begin(), team.end(), fsm));

#ifdef USE_CANVAS_ATTACK_MANAGER
  Sorts::canvas.unregisterGob(fsm->getGob());
#endif
  if (team.size() == 0) {
    msg << "I've gone out the window (Nobody cares about me anymore)" << endl;
    Sorts::amr->removeManager(this);
    delete this;
  }
}

void AttackManager::assignTarget(AttackFSM* fsm, SoarGameObject* target) {
  // first unassign old target
  if (fsm->target != NULL) {
    assert(targetAssignments.find(fsm->target) != targetAssignments.end());
    list<AttackFSM*>* attackers = targetAssignments[fsm->target];
    list<AttackFSM*>::iterator i = 
      find(attackers->begin(), attackers->end(), fsm);
    assert(i != attackers->end());
    attackers->erase(i);
  }

  // now assign to new target
  if (targetAssignments.find(target) == targetAssignments.end()) {
    targetAssignments[target] = new list<AttackFSM*>();
  }
  targetAssignments[target]->push_back(fsm);
  fsm->target = target;
}

void AttackManager::unassignTarget(SoarGameObject* target) {
  if (targetAssignments.find(target) != targetAssignments.end()) {
    list<AttackFSM*>* attackers = targetAssignments[target];
    for(list<AttackFSM*>::iterator
        i  = attackers->begin();
        i != attackers->end();
        i++)
    {
      (*i)->target = NULL;
      (*i)->reassign = true;
    }
    delete targetAssignments[target];
    targetAssignments.erase(target);
  }
}

// the current strategy is basically to focus fire on one enemy
// at a time until they're all dead, minimizing damage taken by self

// In the future, also implement running weak units away
int AttackManager::direct(AttackFSM* fsm) {
  timeval st;
  gettimeofday(&st, NULL);
#ifdef USE_CANVAS_ATTACK_MANAGER
  Sorts::canvas.flashColor(fsm->getGob(), 0, 255, 0, 1);
  Sorts::canvas.update();
#endif

  GameObj* gob = fsm->getGob();
  SoarGameObject* sgob = Sorts::OrtsIO->getSoarGameObject(gob);

  if (updateTargetList() > 0) {
    if (targets.size() == 0) {
      Sorts::amr->removeManager(this);
      msg << "I've gone out the window (Finished my job)" << endl;
      delete this;
      return 1;
    }
    reprioritize();

    for(list<AttackFSM*>::iterator
        i  = team.begin();
        i != team.end();
        ++i)
    {
      (*i)->target = NULL;
    }
  }

  if (fsm->failCount == 10) {
    msg << "Failed too many times" << endl;
    ++(fsm->failCount);
  }
  if (fsm->failCount > 10) {
    return -1;
  }

  // try to hit immediately attackable things first
  if (fsm->target == NULL) {
    for(vector<SoarGameObject*>::iterator
        i  = sortedTargets.begin();
        i != sortedTargets.end();
        ++i)
    {
      if (canHit(gob, (*i)->getGob())) {
        fsm->target = *i;
#ifdef USE_CANVAS_ATTACK_MANAGER
        GameObj* gob = (*i)->getGob();
        Sorts::canvas.trackDestination(fsm->getGob(), *gob->sod.x, *gob->sod.y);
#endif
        break;
      }
    }
  }
  if (fsm->target == NULL) {
    for(vector<SoarGameObject*>::iterator
      i  = sortedTargets.begin();
      i != sortedTargets.end();
      ++i)
    {
//        Sorts::canvas.setColor(fsm->getGob(), 0, 0, 255);
      list<Vec2d> positions;
      attackArcPos(fsm->getGob(), (*i)->getGob(), positions);
      for(list<Vec2d>::iterator
          j  = positions.begin();
          j != positions.end();
          ++j)
      {
        if (fsm->move((*j)(0), (*j)(1)) == 0) {
          msg <<"Moving to Position: "<<(*j)(0)<<", "<<(*j)(1)<<endl;
          fsm->target = *i;
#ifdef USE_CANVAS_ATTACK_MANAGER
          GameObj* gob = (*i)->getGob();
          Sorts::canvas.trackDestination(fsm->getGob(),*gob->sod.x,*gob->sod.y);
#endif
          break;
        }
      }
      if (fsm->target != NULL) {
        break;
      }
    }
  }
  if (fsm->target == NULL) {
    // wasn't successfully assigned a target, wait until next time
    ++(fsm->failCount);
    msg << "Assignment Failed" << endl;
    return 0;
  }

  assert(fsm->target != NULL);

  fsm->failCount = 0;
  GameObj* tgob = fsm->target->getGob();
  if (!canHit(gob, tgob)) {
    if (fsm->isMoving()) {
      Vec2d dest = fsm->getDestination();
      if (canHit(gob, dest, tgob)) {
        // on his way like he should be, let him keep going
        return 0;
      }
    }
    // not moving, or should be moving somewhere else
    msg << "CANNOT HIT" << endl;

    list<Vec2d> positions;
    attackArcPos(gob, tgob, positions);
    for(list<Vec2d>::iterator
        i  = positions.begin();
        i != positions.end();
        ++i)
    {
      if (fsm->move((*i)(0), (*i)(1)) == 0) {
        msg << "Moving to Position: " << (*i)(0) << ", " << (*i)(1) << endl;
#ifdef USE_CANVAS_ATTACK_MANAGER
//        Sorts::canvas.trackDestination(fsm->getGob(), (*i)(0), (*i)(1));
#endif
        break;
      }
    }
    if (!fsm->isMoving()) {
      fsm->target = NULL;
    }
  }
  else if (!fsm->isFiring() ||  
           sgob->getLastAttacked() != fsm->target->getID())
  {
    fsm->attack(fsm->target);
  }

  timeval et;
  gettimeofday(&et, NULL);
  msg << "TIME SPENT: " << et.tv_usec - st.tv_usec << endl;
  return 0;
}

int AttackManager::updateTargetList() {
  int numVanished = 0;
  for(set<SoarGameObject*>::iterator
      i =  targets.begin(); 
      i != targets.end();
      ++i)
  {
    if (!Sorts::OrtsIO->isAlive((*i)->getID())) {
      msg << "(" << (int) this << ") Unit " << (*i)->getID() << " is no longer alive or moved out of view" << endl;
      // this target could have been in multiple attack managers

#ifdef USE_CANVAS_ATTACK_MANAGER
      if (Sorts::canvas.gobRegistered((*i)->getGob())) {
        Sorts::canvas.unregisterGob((*i)->getGob());
      }
#endif
      //unassignTarget(*i);
      targets.erase(i);
      ++numVanished;
    }
  }
  return numVanished;
}

struct TargetCompare {
  Vec2d myPos;

  bool operator()(SoarGameObject* t1, SoarGameObject* t2) {
    // this formula was derived by minimizing damage to your own units,
    // and assuming that none of your units die while attacking (your
    // damage rate stays constant)
    double rating1 = weaponDamageRate(t1->getGob()) * t2->getGob()->get_int("hp");
    double rating2 = weaponDamageRate(t2->getGob()) * t1->getGob()->get_int("hp");

    if (rating1 < rating2) {
      return true;
    }
    else if (rating1 == rating2) { // break ties by distance
      Vec2d p1(*t1->getGob()->sod.x, *t1->getGob()->sod.y);
      Vec2d p2(*t2->getGob()->sod.x, *t2->getGob()->sod.y);

      double dist1 = (myPos - p1).magSq();
      double dist2 = (myPos - p2).magSq();

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
  // calculate centroid
  double xsum = 0, ysum = 0;
  for(list<AttackFSM*>::iterator
      i  = team.begin();
      i != team.end();
      i++)
  {
    xsum += *(*i)->getGob()->sod.x;
    ysum += *(*i)->getGob()->sod.y;
  }
  
  TargetCompare comparator;
  comparator.myPos = Vec2d(xsum / team.size(), ysum / team.size());

  sortedTargets.clear();
  sortedTargets.insert(sortedTargets.begin(), targets.begin(), targets.end());
  sort(sortedTargets.begin(), sortedTargets.end(), comparator);
}

SoarGameObject* AttackManager::selectCloseTarget(GameObj* gob) {
  SoarGameObject* target = NULL;
  for(vector<SoarGameObject*>::iterator
      i =  sortedTargets.begin();
      i != sortedTargets.end();
      ++i)
  {
    if (canHit(gob, (*i)->getGob())) {
      target = *i;
      break;
    }
  }
  return target;
}
