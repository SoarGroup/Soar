#include "AttackNearFSM.h"
#include "Sorts.h"

#define msg cout << "AttackNearFSM.cpp: "

AttackNearFSM::AttackNearFSM(SoarGameObject* _sgob)
: FSM(_sgob->getGob()), sgob(_sgob), attackParams(1)
{
  name = OA_ATTACK_NEAR;
  count = 100;
  weapon = gob->component("weapon");
  assert(weapon != NULL);
  int groundAttackRange = weapon->get_int("max_ground_range");
  int airAttackRange = weapon->get_int("max_air_range");
  if (groundAttackRange > airAttackRange) {
    maxAttackRange = *gob->sod.radius + groundAttackRange;
  }
  else {
    maxAttackRange = *gob->sod.radius + airAttackRange;
  }
}

int AttackNearFSM::update() {
  if (weapon->get_int("shooting") == 1) {
    return FSM_RUNNING;
  }

  if (count > 10) {
    Sorts::spatialDB->getCollisions(
      *gob->sod.x, 
      *gob->sod.y, 
      maxAttackRange,
      NULL,
      nearby);
    count = 0;
  }
  else {
    count++;
  }
  
  list<GameObj*>::iterator i = nearby.begin();
  while (i != nearby.end()) {
    if (Sorts::OrtsIO->isAlive(Sorts::OrtsIO->getGobId(*i)) &&
        *(*i)->sod.owner != Sorts::OrtsIO->getMyId() && 
        *(*i)->sod.owner != Sorts::OrtsIO->getWorldId())
    {
      if (canHit(gob, *i)) {
        attackParams[0] = Sorts::OrtsIO->getGobId(*i);
        weapon->set_action("attack", attackParams);
        sgob->setLastAttacked(attackParams[0]);
        break;
      }
      ++i;
    }
    else {
      i = nearby.erase(i);
    }
  }
  return FSM_RUNNING;
}
