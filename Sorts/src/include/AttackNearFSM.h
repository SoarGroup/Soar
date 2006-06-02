#ifndef AttackNearFSM_h
#define AttackNearFSM_h

#include "FSM.h"

class AttackNearFSM : public FSM {
public:
  AttackNearFSM(GameObj* gob);

  void init() { }
  int update();

private:
  list<GameObj*> nearby;
  ScriptObj* weapon;
  int maxAttackRange;
  int count;
  Vector<sint4> attackParams;
};

#endif
