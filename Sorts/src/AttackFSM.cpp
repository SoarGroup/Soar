#include "AttackFSM.h"
#include "AttackManager.h"
#include "AttackManagerRegistry.h"
#include "general.h"
#include "Sorts.h"

#define CLASS_TOKEN "ATKFSM"
#define DEBUG_OUTPUT false 
#include "OutputDefinitions.h"

using namespace std;

AttackFSM::AttackFSM(SoarGameObject* _sgob)
: FSM(_sgob->getGob()), sgob(_sgob), attackParams(1)
{
  name = OA_ATTACK;
  weapon = gob->component("weapon");
  assert(weapon != NULL);
  moveFSM = NULL;
  manager = NULL;
  waitingForCatchup = false;
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

  panic = false;

  assert(manager == NULL);
  
  // get the attack manager from the registry
  AttackManager* newManager = Sorts::amr->getManager(params[0]);

  assert(newManager != NULL);
  manager = newManager;
  manager->registerFSM(this);
  dest.set(0, *gob->sod.x);
  dest.set(1, *gob->sod.y);
  moving = false;
  disownedStatus = 0;

  target = NULL;
  reassign = true;
  failCount = 0;
  waitingForCatchup = false;
}

int AttackFSM::update() {
  unsigned long st = gettime();
  msg << "updating.\n";
  vector<sint4> moveParams(2);
  if (manager == NULL) {
    if (disownedStatus == 0) {
      dbg << "TIME " << (gettime() - st) / 1000 << endl;
      stop();
      return FSM_FAILURE;
    }
    else {
      dbg << "TIME " << (gettime() - st) / 1000 << endl;
      stop();
      return FSM_SUCCESS;
    }
  }
  int status = manager->direct(this);
  if (status > 0) {
    dbg << "TIME " << (gettime() - st) / 1000 << endl;
    stop();
    return FSM_SUCCESS;
  }
  if (status < 0) {
    dbg << "TIME " << (gettime() - st) / 1000 << endl;
    stop();
    return FSM_FAILURE;
  }

  if (moving && !waitingForCatchup) {
    if (firstMove) {
      // don't call update again on first move
      firstMove = false;
    }
    else if (panic) {
      int moveStatus = moveFSM->update();
      if (moveStatus == FSM_RUNNING || 
          moveStatus == FSM_SUCCESS) {
        msg << "panic succeeded.\n";
        moveParams[0] = (int)dest(0);
        moveParams[1] = (int)dest(1);
        moveFSM->init(moveParams);
        moveFSM->update();
        panic = false;
      }
      else {
        msg << "panic again..\n";
        moveFSM->panic();
        moveFSM->update();
      }
    }   
    else {
      switch(moveFSM->update()) {
        case FSM_SUCCESS:
          moving = false;
          break;
        case FSM_FAILURE:
//        case FSM_STUCK:
          moveFails++;
          if (moveFails > 5) {
            moving = false;
          }
          else {
            // repath
            msg << "MOVE FAILED" << endl;
            moveParams[0] = (int)dest(0);
            moveParams[1] = (int)dest(1);
            moveFSM->init(moveParams);
          }
          break;
        case FSM_UNREACHABLE:
          // unreachable dest, this should never happen
          msg << "(" << dest(0) << "," << dest(1) << ") UNREACHABLE" << endl;
          moving = false;
          break;
        case FSM_STUCK:
          panic = true;
          msg << "Stuck at " << gobX(gob) << ", " << gobY(gob) << " trying to get to " << dest(0) << "," << dest(1) << endl;
          msg << "panic starting.\n";
          moveFSM->panic();
          moveFSM->update(); 
        default:
          break;
      }
    }
  }
  else {
    dbg << "not updating move, moving=" << moving << " waitingForCatchup=" 
        << waitingForCatchup << endl;
  }
  dbg << "TIME " << (gettime() - st) / 1000 << endl;
  return FSM_RUNNING;
}

void AttackFSM::attack(SoarGameObject* t) {
  if (t != NULL) {
    attackParams[0] = target->getID();
    assert (Sorts::OrtsIO->isAlive(attackParams[0]));
    weapon->set_action("attack", attackParams);
    sgob->setLastAttacked(attackParams[0]);
  }
}

bool AttackFSM::isFiring() {
  return weapon->get_int("shooting") == 1;
}

int AttackFSM::move(int x, int y, bool forcePathfind) {
  moveFails = 0;
  if (moveFSM == NULL) {
    moveFSM = new MoveFSM(gob);
  }
  if (moving) {
    moveFSM->stop();
  }
  vector<sint4> moveParams(2);
  moveParams[0] = x;
  moveParams[1] = y;

  if (forcePathfind) {
    // add a fourth (and third, ignored) parameter, to signal forced pathfind
    moveParams.push_back(-1);
    moveParams.push_back(1);
  }

  msg << "initting move (" << x << ", " << y << ")" << endl;
  moveFSM->init(moveParams);
  int status = moveFSM->update();
  switch (status) {
    case FSM_UNREACHABLE:
    case FSM_STUCK:
      msg << "movefsm is stuck!" << endl;
      return 1;
    default:
      dest.set(0, x);
      dest.set(1, y);
      moving = true;
      firstMove = true;
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


void AttackFSM::stop() {
  if (manager != NULL) {
    manager->unregisterFSM(this);
    manager = NULL;
    disownedStatus = 0;
  }
  if (moving) {
    moveFSM->stop();
    moving = false;
  }
  dest.set(0, *gob->sod.x);
  dest.set(1, *gob->sod.y);

  if (target != NULL and target->getID() == sgob->getLastAttacked()) {
    Vector<sint4> stopParams(0);
    weapon->set_action("stop", stopParams);
    sgob->setLastAttacked(-1);
  }
}

int AttackFSM::getAvgDamage() {
  assert(Sorts::OrtsIO->isAlive(sgob->getID()));
  return (weapon->get_int("min_damage") + weapon->get_int("max_damage")) / 2;
}
