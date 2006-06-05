#ifndef AttackNearFSM_h
#define AttackNearFSM_h

#include "FSM.h"
#include "SoarGameObject.h"

class AttackNearFSM : public FSM {
public:
  AttackNearFSM(SoarGameObject* sgob);

  void init() { }
  int update();

private:
  SoarGameObject* sgob;
  list<GameObj*> nearby;
  ScriptObj* weapon;
  int maxAttackRange;
  int count;
  Vector<sint4> attackParams;
};

#endif
