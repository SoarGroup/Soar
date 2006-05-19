#include "Sorts.h"
#include "MineFSM.h"
#include "general.h"
#include "MineManager.h"

MineFSM::MineFSM(GameObj* go) 
         : FSM(go) {
  name = OA_MINE;
  sgo = NULL;
  moveFSM = NULL;
  precision = 1;
  timer = -1;
}

int MineFSM::update() {
  assert (sgo != NULL);
  Vector<sint4> tempVec;
  int moveStatus;
  int temp;
  MiningRoute* newRoute;
  if (gob->is_pending_action()) {
    cout << "MOVEFSM: action has not taken affect!\n";
    return FSM_RUNNING;
  }
  switch (state) {
    case IDLE:
      cout << "MINEFSM: starting\n";
      if (gob->get_int("minerals") > 1) {
        // go to cc first
        tempVec.push_back(route->dropoffLoc.x);
        tempVec.push_back(route->dropoffLoc.y);
        moveFSM->init(tempVec);
        moveFSM->update();
        state = MOVING_TO_DROPOFF;
      }
      else {
        tempVec.push_back(route->miningLoc.x);
        tempVec.push_back(route->miningLoc.y);
        tempVec.push_back(precision);
        moveFSM->init(tempVec);
        moveFSM->update();
        state = MOVING_TO_MINERAL;
      }
      break;
    case MOVING_TO_MINERAL:
      moveStatus = moveFSM->update();
      if (moveStatus == FSM_RUNNING) {
        // do nothing
      }
      else if (moveStatus == FSM_FAILURE) {
        // ??
        tempVec.push_back(route->miningLoc.x);
        tempVec.push_back(route->miningLoc.y);
        tempVec.push_back(precision);
        moveFSM->init(tempVec);
      }
      else if (moveStatus == FSM_SUCCESS) {
        temp = route->mineralInfo->mineral->getID();
        assert(Sorts::OrtsIO->isAlive(temp));
        cout << "orts dist: " << Sorts::OrtsIO->getOrtsDistance(
                                      route->mineralInfo->mineral->gob, gob) << endl;
        cout << "from " << *route->mineralInfo->mineral->gob->sod.x << 
            "," << *route->mineralInfo->mineral->gob->sod.y << 
            " to " << *gob->sod.x << 
            "," << *gob->sod.y << endl;
        assert(Sorts::OrtsIO->getOrtsDistance(route->mineralInfo->mineral->gob, gob) <= 2);
        // otherwise, we need to work on the MoveFSM precision
        
        tempVec.push_back(temp);
        gob->component("pickaxe")->set_action("mine", tempVec); 
        cout << "MINEFSM: mining commencing!\n";
        state = MINING;
        Sorts::mineManager
                ->reportMiningResults((Sorts::OrtsIO->getViewFrame()-timer), 
                                      route, false, this);
      }
      else {
        assert(false);
      }
      break;
    case MINING:
      if (gob->component("pickaxe")->get_int("active") == 0 &&
          gob->get_int("is_mobile") == 1) {
          // finished mining
        if (gob->get_int("minerals") == 0) {
          cout << "MINEFSM: Mining failed for some reason! Trying again..\n";
          temp = route->mineralInfo->mineral->getID();
          tempVec.push_back(temp);
          gob->component("pickaxe")->set_action("mine", tempVec); 
          //assert(false);
        }
        else {
          tempVec.push_back(route->dropoffLoc.x);
          tempVec.push_back(route->dropoffLoc.y);
          moveFSM->init(tempVec);
          moveFSM->update();
          timer = Sorts::OrtsIO->getViewFrame();
          state = MOVING_TO_DROPOFF;
        }
      } // else still mining
      break;
    case MOVING_TO_DROPOFF:
      moveStatus = moveFSM->update();
      if (moveStatus == FSM_RUNNING) {
        // do nothing
      }
      else if (moveStatus == FSM_FAILURE) {
        // ??
        tempVec.push_back(route->dropoffLoc.x);
        tempVec.push_back(route->dropoffLoc.y);
        tempVec.push_back(precision);
        moveFSM->init(tempVec);
      }
      else if (moveStatus == FSM_SUCCESS) {
        temp = route->mineralInfo->mineral->getID();
        assert(Sorts::OrtsIO->isAlive(temp));
        cout << "orts dist: " << Sorts::OrtsIO->getOrtsDistance(
                                      route->cCenterInfo->cCenter->gob, gob) << endl;
        cout << "from " << *route->cCenterInfo->cCenter->gob->sod.x << 
            "," << *route->cCenterInfo->cCenter->gob->sod.y << 
            " to " << *gob->sod.x << 
            "," << *gob->sod.y << endl;
        assert(Sorts::OrtsIO->getOrtsDistance(route->cCenterInfo->cCenter->gob, gob) <= 3);
        // otherwise, we need to work on the MoveFSM precision
        
        tempVec.clear();
        tempVec.push_back(route->cCenterInfo->cCenter->getID());
        gob->set_action("return_resources", tempVec);
        state = SEND_MOVE_TO_MINE_COMMAND;
        if (timer != -1) {
          newRoute = Sorts::mineManager
              ->reportMiningResults((Sorts::OrtsIO->getViewFrame()-timer), 
                                    route, true, this);
          if (newRoute != NULL) { 
            route = newRoute;
            timer = -1; // don't clock this leg, we're not on the new route
          }
        }
      }
      else {
        assert(false);
      }
      break;
    case SEND_MOVE_TO_MINE_COMMAND:
      if (timer != -1) {
        timer = Sorts::OrtsIO->getViewFrame();
      }
      tempVec.push_back(route->miningLoc.x);
      tempVec.push_back(route->miningLoc.y);
      tempVec.push_back(precision);
      moveFSM->init(tempVec);
      moveFSM->update();
      state = MOVING_TO_MINERAL;
      break;
  }
  return FSM_RUNNING;
}
         
          
void MineFSM::init(vector<sint4> p) {
  FSM::init(p);

  assert(sgo != NULL);
  route = Sorts::mineManager->getMiningRoute(this);
  // returns null if manager wasn't prepared
  assert(route != NULL);
  
  state = IDLE;

  assert (p.size() == 0);
  moveFSM = new MoveFSM(gob);
}

MineFSM::~MineFSM() {
  if (moveFSM != NULL) {
    delete moveFSM;
  }
  if (route != NULL) {
    Sorts::mineManager->removeWorker(route, this);
  }
}

void MineFSM::abortMining() {
  route = Sorts::mineManager->getMiningRoute(this);
  assert(route != NULL);
  if (state == MOVING_TO_MINERAL) {
    //moveFSM->stop();
  }
  state = IDLE;
}
