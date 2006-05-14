#include "AttackFSM.h"
#include "AttackManager.h"
#include "AttackManagerRegistry.h"

#include "Sorts.h"

using namespace std;

AttackFSM::AttackFSM(GameObj* gob)
: FSM(gob)
{
  name = OA_ATTACK;
}

AttackFSM::~AttackFSM() {
  if (moveFSM != NULL) {
    delete moveFSM;
  }
}

void AttackFSM::init(vector<sint4> params) {
  FSM::init(params);

  // get the attack manager from the registry
  manager = Sorts::amr->get(params[0]);
}

int AttackFSM::update() {
  if (manager->direct(this) == 1) {
    return FSM_SUCCESS;
  }
  if (moving) {
    switch(moveFSM->update()) {
      case FSM_SUCCESS:
        moving = false;
        break;
      case FSM_FAILURE:
        // what to do here?
        moving = false;
        break;
    }
  }
  return FSM_RUNNING;
}

void AttackFSM::attack(SoarGameObject* t) {
  target = t;
  Vector<sint4> attackParams;
  attackParams.push_back(target->getID());
  weapon->set_action("attack", attackParams);
}

bool AttackFSM::isFiring() {
  return weapon->get_int("shooting") == 1;
}

void AttackFSM::move(int x, int y) {
  if (moveFSM == NULL) {
    moveFSM = new MoveFSM(gob);
  }
  vector<sint4> moveParams;
  moveParams.push_back(x);
  moveParams.push_back(y);
  moveFSM->init(moveParams);
  dest_x = x;
  dest_y = y;
  moving = true;
}

void AttackFSM::stopMoving() {
  moveFSM->stop();
  moving = false;
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
