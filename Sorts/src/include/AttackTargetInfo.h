#ifndef AttackTargetInfo_H_
#define AttackTargetInfo_H_
#include <assert.h>

#include "GameObj.H"
#include "ScriptObj.H"

#include "SoarGameObject.h"
#include "AttackFSM.h"
#include "Circle.h"

#include <map>

class AttackTargetInfo {
public:
  AttackTargetInfo() { assert(false); }
  AttackTargetInfo(SoarGameObject* _target);

  void assignAttacker(AttackFSM* fsm);
  void unassignAttacker(AttackFSM* fsm);

  set<AttackFSM*>::const_iterator attackers_begin() { 
    return attackers.begin();
  }

  set<AttackFSM*>::const_iterator attackers_end() {
    return attackers.end();
  }

  void unassignAll() { attackers.clear(); }

  double avgAttackerDistance();

  bool isSaturated() {
    return volleyDamage > gob->get_int("hp");
  }

private:
  int volleyDamage;
  SoarGameObject* target;
  GameObj*        gob;
  set<AttackFSM*> attackers;
  list<Circle>    attackPos;

  map<AttackFSM*, int> avgDmg;
};

#endif
