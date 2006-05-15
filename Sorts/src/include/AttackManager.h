#ifndef AttackManager_H
#define AttackManager_H

#include <vector>
#include <set>

#include "Circle.h"
#include "SoarGameObject.h"
#include "AttackFSM.h"

#include "Vector.H"
#include "GameObj.H"

using namespace std;

class AttackManager {
public:
  AttackManager(const set<SoarGameObject*>& _targets);
  ~AttackManager();

  void registerFSM(AttackFSM* fsm);
  void unregisterFSM(AttackFSM* fsm);
  int direct(AttackFSM* fsm);

  set<SoarGameObject*>* getTargets() { return &targets; }

private: // functions
  void selectTarget();
  void positionAndAttack();

private:
  list<AttackFSM*> team;
  set<SoarGameObject*> targets;

  SoarGameObject* currTarget;
  Vector<sint4> currAttackParams;
};

#endif
