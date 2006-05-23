#include "AttackManager.h"
#include "AttackManagerRegistry.h"
#include "Circle.h"
#include "Sorts.h"

#include "ScriptObj.H"

#define msg cout << "AttackManager.cpp: "

// some useful functions
double calcWeaponDamageRate(GameObj* gob) {
  ScriptObj* weapon = gob->component("weapon");
  if (weapon == NULL) { 
    return 0; 
    // this might not actually be right, since spells are threats too
  }

  double minDmg = weapon->get_int("min_damage");
  double maxDmg = weapon->get_int("max_damage");
  double cooldown = weapon->get_int("cooldown");

  return (minDmg + maxDmg) / (2 * cooldown); // hp / time
}

bool canHit(GameObj *atk, GameObj *tgt) {
  ScriptObj* weapon = atk->component("weapon");
  if (weapon == NULL) {
    return false;
  }
  double d = 
    squaredDistance(*atk->sod.x, *atk->sod.y, *tgt->sod.x, *tgt->sod.y);
  double r;
  if (*tgt->sod.zcat == GameObj::ON_LAND) {
    r = weapon->get_int("max_ground_range") + *atk->sod.radius + *tgt->sod.radius;
  }
  else {
    r = weapon->get_int("max_air_range") + *atk->sod.radius + *tgt->sod.radius;
  }
  return r * r >= d;
}

bool canHit(GameObj *gob, const Circle& c, bool isGround) {
  ScriptObj* weapon = gob->component("weapon");
  if (weapon == NULL) {
    return false;
  }
  double d = squaredDistance(*gob->sod.x, *gob->sod.y, (int) c.x, (int) c.y);
  double r;
  if (isGround) {
    r = weapon->get_int("max_ground_range") + *gob->sod.radius + c.r;
  }
  else {
    r = weapon->get_int("max_air_range") + *gob->sod.radius + c.r;
  }
  return r * r >= d;
}

bool canHit(GameObj* atk, const Circle& loc, GameObj *tgt) {
  ScriptObj* weapon = atk->component("weapon");
  if (weapon == NULL) {
    return false;
  }
  double d = squaredDistance((int) loc.x, (int) loc.y, *tgt->sod.x, *tgt->sod.y);
  double r;
  if (*tgt->sod.zcat == GameObj::ON_LAND) {
    r = weapon->get_int("max_ground_range") + *atk->sod.radius + *tgt->sod.radius;
  }
  else {
    r = weapon->get_int("max_air_range") + *atk->sod.radius + *tgt->sod.radius;
  }
  return r * r >= d;
 
}

bool canHit(const Circle& c1, const Circle& c2, double range) {
  double d = squaredDistance((int) c1.x, (int) c1.y, (int) c2.x, (int) c2.y);
  double r = range + c1.r + c2.r;
  return r * r >= d;
}

// fake version
/*
bool attackArcPos(GameObj*atk, GameObj* tgt, Circle& pos) {
  pos.x = *tgt->sod.x;
  pos.y = *tgt->sod.y;
  return true;
}
*/


Point AttackManager::attackArcPos(GameObj* atk, GameObj* tgt) {
  int range;
  int atkRadius = *atk->sod.radius;
  int tgtRadius = *tgt->sod.radius;
  Point aPos(*atk->sod.x, *atk->sod.y);
  Point tPos(*tgt->sod.x, *tgt->sod.y);
  msg << "ax: " << aPos.x << endl;
  msg << "ay: " << aPos.y << endl;
  msg << "tx: " << tPos.x << endl;
  msg << "ty: " << tPos.y << endl;
  if (*tgt->sod.zcat == GameObj::ON_LAND) {
    range = atk->component("weapon")->get_int("max_ground_range") 
      + atkRadius + tgtRadius;
  }
  else {
    range = atk->component("weapon")->get_int("max_air_range") 
      + atkRadius + tgtRadius;
  }
  
  double circumference = 2 * PI * range;
  double angleInc = PI * (*atk->sod.radius * 2) / circumference;

  double dx   = aPos.x - tPos.x;
  double dy   = aPos.y - tPos.y;
  double dist = dx * dx + dy * dy;
  double ndx  = dx / dist;

  double startAng;
  if (dy >= 0) {
    startAng = acos(ndx);
    msg << "ang: " << startAng << endl;
  }
  else {
    startAng = PI + acos(ndx);
    msg << "ang(m): " << startAng << endl;
  }

  for(double currInc = 0; currInc < PI; currInc += angleInc) {
    list<GameObj*> collisions;
    Point fPos, ifPos;
    // first counter-clockwise (increase angle)
    fPos.x = tPos.x + range * cos(startAng + currInc);
    fPos.y = tPos.y + range * sin(startAng + currInc);
    if (fPos.x >= 0 && fPos.y >= 0) {
      ifPos = fPos.roundToward(tPos);

      bool slotTaken = false;
      for(list<AttackFSM*>::iterator
          i =  team.begin();
          i != team.end();
          i++) 
      {
        if ((*i)->isMoving()) {
          double d = ifPos.distSqTo((*i)->getDestination());
          double r = *(*i)->getGameObject()->sod.radius + atkRadius;
          if (d < r * r) {
            slotTaken = true;
            break;
          }
        }
      }
      if (!slotTaken) {
        Sorts::satellite->getCollisions(
          ifPos.intx(), ifPos.inty(), atkRadius, NULL, collisions);
        if (collisions.size() == 0) { // there's no collision
          return ifPos;
        }
      }
    }
    
    // now clockwise (decrease angle)
    fPos.x = tPos.x + range * cos(startAng - currInc);
    fPos.y = tPos.y + range * sin(startAng - currInc);
    if (fPos.x >= 0 && fPos.y >= 0) {
      ifPos = fPos.roundToward(tPos);

      bool slotTaken = false;
      for(list<AttackFSM*>::iterator
          i =  team.begin();
          i != team.end();
          i++) 
      {
        if ((*i)->isMoving()) {
          double d = ifPos.distSqTo((*i)->getDestination());
          double r = *(*i)->getGameObject()->sod.radius + atkRadius;
          if (d < r * r) {
            slotTaken = true;
            break;
          }
        }
      }
      if (!slotTaken) {
        Sorts::satellite->getCollisions(
          ifPos.intx(), ifPos.inty(), atkRadius, NULL, collisions);
        if (collisions.size() == 0) { 
          return ifPos;
        }
      }
    }
  }
  Point errPos(-1, -1);
  return errPos;
}

AttackManager::AttackManager(const set<SoarGameObject*>& _targets)
: targets(_targets), currTarget(NULL), currAttackParams(1)
{
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
      i++)
  {
    (*i)->disown(status);
  }
}

void AttackManager::registerFSM(AttackFSM* fsm) {
  team.push_back(fsm);
}

void AttackManager::unregisterFSM(AttackFSM* fsm) {
  assert(find(team.begin(), team.end(), fsm) != team.end());
  team.erase(find(team.begin(), team.end(), fsm));
  if (team.size() == 0) {
    Sorts::amr->removeManager(this);
    delete this;
  }
}

// the current strategy is basically to focus fire on one enemy
// at a time until they're all dead, starting with the one that
// will deal the most damage.

// In the future, also implement running weak units away and selecting
// the most urgent unit to kill based on more sophisticated measures
// i.e. some balance between ease of killing and damage rate
int AttackManager::direct(AttackFSM* fsm) {
  updateTargetList();
  if (currTarget == NULL) {
    cout << "ATTACK MANAGER: SELECT NEW TARGET" << endl;
    selectTarget();
    if (currTarget == NULL) {
      // no more targets left
      Sorts::amr->removeManager(this);
      delete this;
      return 1;
    }
  }

  GameObj* gob = fsm->getGameObject();
  if (!canHit(gob, currTarget->gob)) {
    // find someone he can hit, don't waste time by not shooting
    if (fsm->getTarget() == NULL ||
        !Sorts::OrtsIO->isAlive(fsm->getTarget()->getID()) ||
        !canHit(gob, fsm->getTarget()->gob)) 
    {
      SoarGameObject* tempTarget = selectCloseTarget(gob);
      if (tempTarget != NULL) {
        fsm->attack(tempTarget);
      }
    }
    // if he's not already moving toward currTarget, tell him to
    Point dest = fsm->getDestination();
    Circle cdest(dest.x, dest.y, *gob->sod.radius);
    if (!fsm->isMoving() || !canHit(gob, cdest, currTarget->gob)) {
      Point pos = attackArcPos(gob, currTarget->gob);
      if(pos.x >= 0) {
        msg << "Moving to Position: " << pos.x << ", " << pos.y << endl;
        fsm->move(pos.intx(), pos.inty());
      }
      else {
        // the entire circle around the guy is surrounded
        // should find another target to shoot at
      }
    }
  }
  else if (fsm->getTarget() != currTarget || !fsm->isFiring()) {
    fsm->attack(currTarget);
  }

  return 0;
}

void AttackManager::updateTargetList() {
  for(set<SoarGameObject*>::iterator
      i =  targets.begin(); 
      i != targets.end();
      i++)
  {
    if (!Sorts::OrtsIO->isAlive((*i)->getID())) {
      msg << "Unit is no longer alive or moved out of view" << endl;
      targets.erase(i);
      continue;
    }
  }
  // finally check the current target
  if (currTarget != NULL && !Sorts::OrtsIO->isAlive(currTarget->getID())) {
    currTarget = NULL;
  }
}

void AttackManager::selectTarget() {
  // for now just select the biggest threat
  double best_dmg = -1.0;
  currTarget = NULL;
  
  for(set<SoarGameObject*>::iterator
      i =  targets.begin(); 
      i != targets.end();
      i++)
  {
    double dmg = calcWeaponDamageRate((*i)->gob);
    if (dmg > best_dmg) {
      currTarget = *i;
      best_dmg = dmg;
    }
  }
  if (currTarget != NULL) {
    currAttackParams[0] = currTarget->getID();
  }
}

SoarGameObject* AttackManager::selectCloseTarget(GameObj* gob) {
  SoarGameObject* target = NULL;
  for(set<SoarGameObject*>::iterator
      i =  targets.begin();
      i != targets.end();
      i++)
  {
    if (canHit(gob, (*i)->gob)) {
      target = *i;
      break;
    }
  }
  return target;
}
