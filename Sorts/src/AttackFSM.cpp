#include "AttackFSM.h"
#include "AttackManager.h"

using namespace std;

AttackFSM::AttackFSM(GameObj* gob, AttackManager* _manager)
: FSM(gob), manager(_manager)
{
  name = OA_ATTACK;
}

AttackFSM::~AttackFSM() {
  if (moveFSM != NULL) {
    delete moveFSM;
  }
}

int AttackFSM::update() {
  if (manager->direct(this) == 1) {
    return FSM_SUCCESS;
  }
  if (moveFSM != NULL) {
    switch(moveFSM->update()) {
      case FSM_SUCCESS:
      case FSM_FAILURE:
        delete moveFSM;
        moveFSM = NULL;
    }
  }
  return FSM_RUNNING;
}

void AttackFSM::attack(SoarGameObject* t) {
  target = t;
  Vector<sint4> attackParams;
  attackParams.push_back(target->getID());
  gob->component("weapon")->set_action("attack", attackParams);
}

bool AttackFSM::isFiring() {
  return false;
}

void AttackFSM::move(int x, int y) {
  if (moveFSM != NULL) {
    delete moveFSM;
  }
  vector<sint4> moveParams;
  moveParams.push_back(x);
  moveParams.push_back(y);
  moveFSM = new MoveFSM(gob);
  moveFSM->init(moveParams);
  dest_x = x, dest_y = y;
}

void AttackFSM::stopMoving() {
  if (moveFSM != NULL) {
    delete moveFSM;
    moveFSM = NULL;
  }
}

void AttackFSM::getDestination(int* x, int* y) {
  if (moveFSM != NULL) {
    *x = dest_x;
    *y = dest_y;
  }
  else {
    *x = *gob->sod.x;
    *y = *gob->sod.y;
  }
}
