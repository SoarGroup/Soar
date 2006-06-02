#ifndef AttackManager_H
#define AttackManager_H

#include <vector>
#include <set>
#include <map>

#include "Point.h"
#include "Circle.h"
#include "SoarGameObject.h"
#include "AttackFSM.h"

#include "Vector.H"
#include "GameObj.H"

#include "SortsCanvas.h"

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
  void            selectTarget();
  SoarGameObject* selectCloseTarget(GameObj* gob);
  void            positionAndAttack();
  int             updateTargetList();
  void            reprioritize();
  void            attackArcPos(GameObj* atk, GameObj* tgt, list<Vec2d>& positions);

  void            assignTarget(AttackFSM* fsm, SoarGameObject* target);
  void            unassignTarget(SoarGameObject* target);

private: // variables
  
  list<AttackFSM*> team;
  set<SoarGameObject*> targets;
  vector<SoarGameObject*> sortedTargets;

  map<SoarGameObject*, list<AttackFSM*>*> targetAssignments;
};

#endif
