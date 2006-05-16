#include "AttackFSM.h"
#include "AttackManager.h"
#include "AttackManagerRegistry.h"

#include "Sorts.h"

using namespace std;

AttackFSM::AttackFSM(GameObj* gob)
: FSM(gob)
{
  name = OA_ATTACK;
  weapon = gob->component("weapon");
  assert(weapon != NULL);
  moveFSM = NULL;
  manager = NULL;
}

AttackFSM::~AttackFSM() {
  if (manager != NULL) {
    manager->unregisterFSM(this);
  }
  if (moveFSM != NULL) {
    delete moveFSM;
  }
}

void AttackFSM::init(vector<sint4> params) {
  FSM::init(params);

  // get the attack manager from the registry
  manager = Sorts::amr->getManager(params[0]);
  manager->registerFSM(this);
  target = NULL;
  dest_x = 0; dest_y = 0;
  moving = false;
  disownedStatus = 0;
}

int AttackFSM::update() {
  if (manager == NULL) {
    if (disownedStatus == 0) {
      return FSM_FAILURE;
    }
    else {
      return FSM_SUCCESS;
    }
  }
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
        std::cout << "##JZXU## MOVE FAILED" << std::endl;
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
  if (moving) {
    moveFSM->stop();
  }
  vector<sint4> moveParams(2);
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
  if (moving) {
    *x = dest_x;
    *y = dest_y;
  }
  else {
    *x = *gob->sod.x;
    *y = *gob->sod.y;
  }
}

void AttackFSM::disown(int lastStatus) {
  disownedStatus = lastStatus;
  manager = NULL;
}
