#include "AttackTargetInfo.h"

#define CLASS_TOKEN "ATKTAR"
#define DEBUG_OUTPUT false 
#include "OutputDefinitions.h"

AttackTargetInfo::AttackTargetInfo(SoarGameObject* _target)
: target(_target)
{ 
  gob = target->getGob();
  volleyDamage = 0;
}

void AttackTargetInfo::assignAttacker(AttackFSM* fsm) {
  attackers.insert(fsm);
 // ScriptObj* weapon = fsm->getGob()->component("weapon");
 // assert(weapon != NULL);
//  volleyDamage += fsm->getAvgDamage();
//  avgDmg[fsm] = fsm->getAvgDamage();
}

void AttackTargetInfo::unassignAttacker(AttackFSM* fsm) {
  attackers.erase(fsm);
//  volleyDamage -= avgDmg[fsm];
//  avgDmg.erase(fsm);
}

double AttackTargetInfo::avgAttackerDistance() {
  double avgDist = 0;
  if (attackers.size() == 0) {
    assert(false);
  }
  for(set<AttackFSM*>::iterator
      i  = attackers.begin();
      i != attackers.end();
      ++i)
  {
    if (not Sorts::OrtsIO->isAlive((*i)->getSGO()->getID())) {
      msg << "ERROR: stale attacker! SGO: " << (*i)->getSGO() << endl;
      assert(false);
    }
    GameObj* aGob = (*i)->getGob();
    avgDist += squaredDistance(gobX(aGob), gobY(aGob), gobX(gob), gobY(gob));
  }
  avgDist /= attackers.size();
  return avgDist;

}
