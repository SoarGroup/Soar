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
  AttackManager
  ( const set<SoarGameObject*>& _units, 
    const vector<SoarGameObject*>& _targets );

  int direct(AttackFSM* fsm);

private: // functions
  void selectTarget();
  void positionAndAttack();

private:
  enum AttackState { IDLE, ATTACKING };

  AttackState state;

  vector<SoarGameObject*> units;
  vector<SoarGameObject*> targets;

  SoarGameObject* currTarget;
  Vector<sint4> currAttackParams;
};

#endif
