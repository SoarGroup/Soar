#include "AttackFSM.h"
#include "AttackManager.h"
#include "AttackManagerRegistry.h"
#include "general.h"
#include "Sorts.h"

#define msg cout << "AttackFSM.cpp "

using namespace std;

AttackFSM::AttackFSM(SoarGameObject* _sgob)
: FSM(_sgob->getGob()), sgob(_sgob), attackParams(1)
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
  AttackManager* newManager = Sorts::amr->getManager(params[0]);
  assert(newManager != NULL);
  if (manager != NULL && newManager != manager) {
    msg << "HIJACKED!" << endl;
    manager->unregisterFSM(this);
  }
  if (manager == newManager) {
    msg << "Reassigned to identical attack manager" << endl;
  }
  else {
    manager = newManager;
    manager->registerFSM(this);
  }
  dest.set(0, *gob->sod.x);
  dest.set(1, *gob->sod.y);
  moving = false;
  disownedStatus = 0;

  target = NULL;
  reassign = true;
  failCount = 0;
}

int AttackFSM::update() {
  msg << "updating.\n";
  vector<sint4> moveParams(2);
  if (manager == NULL) {
    if (disownedStatus == 0) {
      return FSM_FAILURE;
    }
    else {
      return FSM_SUCCESS;
    }
  }
  int status = manager->direct(this);
  if (status > 0) {
    return FSM_SUCCESS;
  }
  if (status < 0) {
    return FSM_FAILURE;
  }

  if (moving) {
    switch(moveFSM->update()) {
      case FSM_SUCCESS:
        moving = false;
        break;
      case FSM_FAILURE:
        // repath
        msg << "MOVE FAILED" << endl;
        moveParams[0] = dest(0);
        moveParams[1] = dest(1);
        moveFSM->init(moveParams);
        break;
      case FSM_UNREACHABLE:
        // unreachable dest, this should never happen
        msg << "(" << dest(0) << "," << dest(1) << ") UNREACHABLE" << endl;
        moving = false;
        break;
      default:
        break;
    }
  }
  return FSM_RUNNING;
}

void AttackFSM::attack(SoarGameObject* t) {
  if (t != NULL) {
    attackParams[0] = target->getID();
    weapon->set_action("attack", attackParams);
    sgob->setLastAttacked(attackParams[0]);
  }
}

bool AttackFSM::isFiring() {
  return weapon->get_int("shooting") == 1;
}

int AttackFSM::move(int x, int y) {
  if (moveFSM == NULL) {
    moveFSM = new MoveFSM(gob);
  }
  if (moving) {
    moveFSM->stop();
  }
  vector<sint4> moveParams(2);
  moveParams[0] = x;
  moveParams[1] = y;
  msg << "initting move\n";
  moveFSM->init(moveParams);
  int status = moveFSM->update();
  switch (status) {
    case FSM_UNREACHABLE:
      msg << "(" << x << "," << y << endl;
      return 1;
    default:
      dest.set(0, x);
      dest.set(1, y);
      moving = true;
      return 0;
  }
}

void AttackFSM::stopMoving() {
  moveFSM->stop();
  moving = false;
  dest.set(0, *gob->sod.x);
  dest.set(1, *gob->sod.y);
}

void AttackFSM::disown(int lastStatus) {
  disownedStatus = lastStatus;
  manager = NULL;
}
