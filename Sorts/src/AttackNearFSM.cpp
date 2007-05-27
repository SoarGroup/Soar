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
#include "AttackNearFSM.h"
#include "Sorts.h"

#define CLASS_TOKEN "ATKNEARFSM"
#define DEBUG_OUTPUT false 
#include "OutputDefinitions.h"

AttackNearFSM::AttackNearFSM(SoarGameObject* _sgob)
: FSM(_sgob->getGob()), sgob(_sgob), attackParams(1)
{
  name = OA_ATTACK_NEAR;
  count = 100;
  weapon = gob->component("weapon");
  ASSERT(weapon != NULL);
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
  msg << "updating.\n";
  if (weapon->get_int("shooting") == 1) {
    return FSM_RUNNING;
  }

  if (count > 3) {
    Sorts::spatialDB->getObjectCollisions(
      *gob->sod.x, 
      *gob->sod.y, 
      maxAttackRange,
      NULL,
      nearby);
    //sort(nearby.begin(), nearby.end());
    count = 0;
  }
  else {
    count++;
    msg << "waiting..\n";
  }
  
  GameObj* lowest = NULL;
  list<GameObj*>::iterator i = nearby.begin();
  while (i != nearby.end()) {
    if (Sorts::OrtsIO->isAlive(Sorts::OrtsIO->getGobId(*i)) &&
        *(*i)->sod.owner != Sorts::OrtsIO->getMyId() && 
        *(*i)->sod.owner != Sorts::OrtsIO->getWorldId())
    {
      if (canHit(gob, *i)) {
        if (lowest == NULL) {
          lowest = *i;
        }
        else if (*i < lowest) {
          lowest = *i;
        }
      }
      ++i;
    }
    else {
      i = nearby.erase(i);
    }
  }
  if (lowest != NULL) {
    //Sorts::canvas.flashColor(sgob, 255, 128, 0, 1); // orange
    attackParams[0] = Sorts::OrtsIO->getGobId(lowest);
    msg << "opportunistic attack!\n";
    weapon->set_action("attack", attackParams);
    sgob->setLastAttacked(attackParams[0]);
    sgob->setLastAttackOpportunistic(true);
  }
  return FSM_RUNNING;
}
