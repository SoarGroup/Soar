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
#include "AttackTargetInfo.h"

#define CLASS_TOKEN "ATKTAR"
#define DEBUG_OUTPUT true 
#include "OutputDefinitions.h"

AttackTargetInfo::AttackTargetInfo(SoarGameObject* _target)
: target(_target)
{ 
  gob = target->getGob();
  volleyDamage = 0;
  dbg << "created.\n";
}

void AttackTargetInfo::assignAttacker(AttackFSM* fsm) {
  attackers.insert(fsm);
 // ScriptObj* weapon = fsm->getGob()->component("weapon");
 // ASSERT(weapon != NULL);
//  volleyDamage += fsm->getAvgDamage();
//  avgDmg[fsm] = fsm->getAvgDamage();
}

void AttackTargetInfo::unassignAttacker(AttackFSM* fsm) {
  attackers.erase(fsm);
  ASSERT(attackers.find(fsm) == attackers.end());
  dbg << "unassigned fsm " << fsm << endl;
//  volleyDamage -= avgDmg[fsm];
//  avgDmg.erase(fsm);
}

double AttackTargetInfo::avgAttackerDistance() {
  double avgDist = 0;
  if (attackers.size() == 0) {
    ASSERT(false);
  }
  for(set<AttackFSM*>::iterator
      i  = attackers.begin();
      i != attackers.end();
      ++i)
  {
    if (not Sorts::OrtsIO->isAlive((*i)->getSGO()->getID())) {
      msg << "ERROR: stale attacker! SGO: " << (*i)->getSGO() << endl;
      ASSERT(false);
    }
    GameObj* aGob = (*i)->getGob();
    avgDist += squaredDistance(gobX(aGob), gobY(aGob), gobX(gob), gobY(gob));
  }
  avgDist /= attackers.size();
  return avgDist;

}
