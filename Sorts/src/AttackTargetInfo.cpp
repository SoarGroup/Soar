#include "AttackTargetInfo.h"

AttackTargetInfo::AttackTargetInfo(SoarGameObject* _target)
: target(_target)
{ 
  gob = target->getGob();
  volleyDamage = 0;
}

void AttackTargetInfo::assignAttacker(AttackFSM* fsm) {
  attackers.insert(fsm);
  ScriptObj* weapon = fsm->getGob()->component("weapon");
  assert(weapon != NULL);
  volleyDamage += fsm->getAvgDamage();
}

void AttackTargetInfo::unassignAttacker(AttackFSM* fsm) {
  attackers.erase(fsm);
  volleyDamage -= fsm->getAvgDamage();
}

double AttackTargetInfo::avgAttackerDistance() {
  double avgDist = 0;
  GameObj* tGob = target->getGob();
  for(set<AttackFSM*>::iterator
      i  = attackers.begin();
      i != attackers.end();
      ++i)
  {
    GameObj* aGob = (*i)->getGob();
    avgDist += squaredDistance(*aGob->sod.x, *aGob->sod.y, *tGob->sod.x, *tGob->sod.y);
  }
  avgDist /= attackers.size();
  cout << "AVERAGEDIST: " << avgDist << endl;
  return avgDist;

}
