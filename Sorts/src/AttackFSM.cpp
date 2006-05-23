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
  assert(manager != NULL);
  manager->registerFSM(this);
  target = NULL;
  dest.x = *gob->sod.x; dest.y = *gob->sod.y;
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
        moving = false;
        break;
      default:
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
  moveParams[0] = x;
  moveParams[1] = y;
  moveFSM->init(moveParams);
  dest.x = x;
  dest.y = y;
  moving = true;
}

void AttackFSM::stopMoving() {
  moveFSM->stop();
  moving = false;
  dest.x = *gob->sod.x;
  dest.y = *gob->sod.y;
}

void AttackFSM::disown(int lastStatus) {
  disownedStatus = lastStatus;
  manager = NULL;
}
