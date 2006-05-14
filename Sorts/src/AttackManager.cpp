#include "AttackManager.h"
#include "Circle.h"
#include "ScriptObj.H"

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

bool attackArcPos(GameObj* atk, GameObj* tgt, Circle& pos) {
  int range;
  int atkRadius = *atk->sod.radius;
  if (*tgt->sod.zcat == GameObj::ON_LAND) {
    range = atk->component("weapon")->get_int("max_ground_range") + atkRadius;
  }
  else {
    range = atk->component("weapon")->get_int("max_air_range") + atkRadius;
  }
  
  double circumference = 2 * PI * range;
  double angleInc = PI * (*atk->sod.radius * 2) / circumference;

  double dx = *tgt->sod.x - *atk->sod.x;
  double dy = *tgt->sod.y - *atk->sod.y;
  double mag = sqrt(dx * dx + dy * dy);
  double ndx = dx / mag;

  double startAng;
  if (dy >= 0) {
    startAng = acos(ndx);
  }
  else {
    startAng = 2 * PI - acos(ndx);
  }

  for(double currInc = 0; currInc < PI; currInc += angleInc) {
    Circle c(0, 0, atkRadius);
    // first counter-clockwise (increase angle)
    c.x = range * cos(startAng + currInc);
    c.y = range * sin(startAng + currInc);
    if ( /* there's no collision */ true) {
      pos = c;
      return true;
    }
    
    // now clockwise (decrease angle)
    c.x = range * cos(startAng - currInc);
    c.y = range * sin(startAng - currInc);
    if ( /* there's no collision */ true) {
      pos = c;
      return true;
    }
  }
  return false;
}

AttackManager::AttackManager
( const set<SoarGameObject*>& _units, 
  const vector<SoarGameObject*>& _targets)
: targets(_targets), currTarget(NULL)
{
  units.insert(units.begin(), _units.begin(), _units.end());
}


// the current strategy is basically to focus fire on one enemy
// at a time until they're all dead, starting with the one that
// will deal the most damage.

// In the future, also implement running weak units away and selecting
// the most urgent unit to kill based on more sophisticated measures
// i.e. some balance between ease of killing and damage rate
int AttackManager::direct(AttackFSM* fsm) {
  if (currTarget == NULL || !Sorts::OrtsIO->isAlive(currTarget->getID())) {
    selectTarget();
    if (currTarget == NULL) {
      return 1;
    }
  }

  GameObj* gob = fsm->getGameObject();
  if (!canHit(gob, currTarget->gob)) {
    // find someone he can hit, don't waste time by not shooting
    if (fsm->getTarget() == NULL ||
        !canHit(gob, fsm->getTarget()->gob)) {
      for(vector<SoarGameObject*>::iterator
          i =  targets.begin();
          i != targets.end();
          i++)
      {
        if (canHit(gob, (*i)->gob)) {
          fsm->attack(*i);
          break;
        }
      }
    }
    // if he's not already moving toward currTarget, tell him to
    int dest_x, dest_y;
    fsm->getDestination(&dest_x, &dest_y);
    Circle dest(dest_x, dest_y, *gob->sod.radius);
    if (!fsm->isMoving() || !canHit(gob, dest, currTarget->gob)) {
      Circle pos;
      if(attackArcPos(gob, currTarget->gob, pos)) {
        fsm->move((int) pos.x, (int) pos.y);
      }
      else {
        // the entire circle around the guy is surrounded
        // what to do here?
      }
    }
  }
  else if (fsm->getTarget() != currTarget || !fsm->isFiring()) {
    fsm->attack(currTarget);
  }

  return 0;
}

void AttackManager::selectTarget() {
  // for now just select the biggest threat
  double best_dmg = -1.0;
  currTarget = NULL;
  
  for(vector<SoarGameObject*>::iterator
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


