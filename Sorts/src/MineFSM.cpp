#include "Sorts.h"
#include "MineFSM.h"
#include "general.h"
#include "MineManager.h"

#define msg cout << "MINEFSM: "

#define GIVEUPSPEED .66
#define RAW_GIVEUP_THRESHOLD 200 
#define OPPORTUNISTIC_DROPOFF

MineFSM::MineFSM(GameObj* go) 
         : FSM(go) {
  name = OA_MINE;
  sgo = NULL;
  moveFSM = NULL;
  precision = 1;
  timer = 0;
  timed = false;
  giveUpThreshold = -1;
}

int MineFSM::update() {
  assert (sgo != NULL);
  Vector<sint4> tempVec;
  int moveStatus;
  int temp;
  MiningRoute* newRoute;
  if (Sorts::OrtsIO->getViewFrame() == Sorts::OrtsIO->getActionFrame()) {
    timer++;
  }
  // time since timer started
  msg << "timer: " << timer << endl;

  if (timer > RAW_GIVEUP_THRESHOLD) {
    msg << "giving up before starting\n";
    newRoute = Sorts::mineManager->minerGivesUp(route, this);
    
    if (newRoute != NULL) {
      route = newRoute;
      giveUpThreshold = (int)(GIVEUPSPEED*route->pathlength);
      timer = 0;//Sorts::OrtsIO->getViewFrame(); 
      timed = false;// don't clock this leg, we're not on the new route
      state = IDLE;
    }
  }
  
  if (timed and giveUpThreshold != -1
      and (timer > giveUpThreshold)) {
    msg << "giving up on leg\n";
    newRoute = Sorts::mineManager->minerGivesUp(route, this);
    if (newRoute != NULL) {
      route = newRoute;
      giveUpThreshold = GIVEUPSPEED*((int)route->pathlength);
      timer = 0;//Sorts::OrtsIO->getViewFrame(); 
      timed = false;// don't clock this leg, we're not on the new route
      state = IDLE;
    }
  }
  
  
  if (gob->is_pending_action()) {
    cout << "MOVEFSM: action has not taken affect!\n";
    return FSM_RUNNING;
  }
  switch (state) {
    case IDLE:
      cout << "MINEFSM: starting\n";
      timed = false;
      timer = 0;//Sorts::OrtsIO->getViewFrame();
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
      else if (moveStatus == FSM_UNREACHABLE) {
        //assert(false);
        msg << "ERROR: miner has unreachable route!\n";
        // mineManager should not have given this path out
        newRoute = Sorts::mineManager->minerGivesUp(route, this);
        if (newRoute != NULL) {
          route = newRoute;
          giveUpThreshold = (int)(GIVEUPSPEED*route->pathlength);
          timer = 0;//Sorts::OrtsIO->getViewFrame(); 
          timed = false;// don't clock this leg, we're not on the new route
          state = IDLE;
        }
        else { // try again
          tempVec.push_back(route->miningLoc.x);
          tempVec.push_back(route->miningLoc.y);
          tempVec.push_back(precision);
          moveFSM->init(tempVec);
        }
      }
      else if (moveStatus == FSM_SUCCESS) {
        temp = route->mineralInfo->mineral->getID();
        assert(Sorts::OrtsIO->isAlive(temp));
        /*
        cout << "orts dist: " << Sorts::OrtsIO->getOrtsDistance(
                                      route->mineralInfo->mineral->gob, gob) << endl;
        cout << "from " << *route->mineralInfo->mineral->gob->sod.x << 
            "," << *route->mineralInfo->mineral->gob->sod.y << 
            " to " << *gob->sod.x << 
            "," << *gob->sod.y << endl;
        
        */
        //assert(Sorts::OrtsIO->getOrtsDistance(route->mineralInfo->mineral->getGob(),
         //                                     gob) <= 2);
        // otherwise, we need to work on the MoveFSM precision
        
        tempVec.push_back(temp);
        gob->component("pickaxe")->set_action("mine", tempVec); 
        cout << "MINEFSM: mining commencing!\n";
        state = MINING;
        timer = 0;
        /*
        if (timed) { 
          Sorts::mineManager
                  ->reportMiningResults(timer, 
                                        route, false, this);
        }*/
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
          //timer = 0;//Sorts::OrtsIO->getViewFrame();
          timed = true;
          state = MOVING_TO_DROPOFF;
        }
      } // else still mining
      break;
    case MOVING_TO_DROPOFF:
      moveStatus = moveFSM->update();
      if (moveStatus == FSM_RUNNING) {
#ifdef OPPORTUNISTIC_DROPOFF
        double ccDist = Sorts::OrtsIO->getOrtsDistance(
            route->cCenterInfo->cCenter->getGob(), gob);
        if (ccDist <= 3) {
          tempVec.clear();
          tempVec.push_back(route->cCenterInfo->cCenter->getID());
          gob->set_action("return_resources", tempVec);
          state = SEND_MOVE_TO_MINE_COMMAND;
          msg << "opportunistic dropoff!\n";
        }
        else if (ccDist <= 5) {
          tempVec.clear();
          coordinate loc = route->cCenterInfo->cCenter->getLocation();
          tempVec.push_back(loc.x);
          tempVec.push_back(loc.y);
          msg << "opportunistic veer-toward station.\n";
          moveFSM->init(tempVec);
        }
#endif
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
        /*
        cout << "orts dist: " << Sorts::OrtsIO->getOrtsDistance(
                                      route->cCenterInfo->cCenter->gob, gob) << endl;
        cout << "from " << *route->cCenterInfo->cCenter->gob->sod.x << 
            "," << *route->cCenterInfo->cCenter->gob->sod.y << 
            " to " << *gob->sod.x << 
            "," << *gob->sod.y << endl;
        */
        assert(Sorts::OrtsIO->getOrtsDistance(route->cCenterInfo->cCenter->getGob(),
                                              gob) <= 3);
        // otherwise, we need to work on the MoveFSM precision
        
        tempVec.clear();
        tempVec.push_back(route->cCenterInfo->cCenter->getID());
        gob->set_action("return_resources", tempVec);
        state = SEND_MOVE_TO_MINE_COMMAND;
        /*if (timed) {
          newRoute = Sorts::mineManager
              ->reportMiningResults(timer, 
                                    route, true, this);
          if (newRoute != NULL) { 
            route = newRoute;
            giveUpThreshold = GIVEUPSPEED*route->pathlength;
            timer = 0;//Sorts::OrtsIO->getViewFrame(); 
            timed = false;// don't clock this leg, we're not on the new route
          }
        }*/
      }
      else {
        // unreachable or stuck
        state = PANIC_START;
      }
      break;
    case SEND_MOVE_TO_MINE_COMMAND:
      timer = 0;//Sorts::OrtsIO->getViewFrame();
      timed = true;
      tempVec.push_back(route->miningLoc.x);
      tempVec.push_back(route->miningLoc.y);
      tempVec.push_back(precision);
      moveFSM->init(tempVec);
      moveFSM->update();
      state = MOVING_TO_MINERAL;
      break;
    case PANIC_START:
      if (moveFSM != NULL) {
        tempVec.push_back(*gob->sod.x + ((int)rand() % 20) -5);
        tempVec.push_back(*gob->sod.y + ((int)rand() % 20) -5);
        moveFSM->initNoPath(tempVec);
        moveFSM->update();
        msg << "panic! to " << tempVec[0] << "," << tempVec[1] << endl;
      }
      panicCount = 0;
      state = PANIC;
      break;
    case PANIC:
      moveStatus = moveFSM->update();
      if (moveStatus == FSM_SUCCESS) {
        state = IDLE;
      }
      else if (moveStatus == FSM_FAILURE) {
        tempVec.push_back(*gob->sod.x + ((int)rand() % 30) -15);
        tempVec.push_back(*gob->sod.y + ((int)rand() % 30) -15);
        moveFSM->init(tempVec);
        moveFSM->update();
        panicCount++;
        msg << "repanic! count " << panicCount << "  to " 
            << tempVec[0] << "," << tempVec[1] << endl;
      }
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
  
  giveUpThreshold = (int)(GIVEUPSPEED*route->pathlength);
  
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
  if (moveFSM != NULL) {
    moveFSM->stop();
  }
  state = IDLE;
}

void MineFSM::panic() {
  if (state == MINING) {
    msg << "panic averted, I'm mining\n";
  }
  else {
    state = PANIC_START;
  }
}
