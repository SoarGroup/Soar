#include "include/MineFSM.h"
#include "include/SoarGameGroup.h"
#include "general.h"

#define WARMUP_TIME 5

// this is squared
#define DISTANCE_EPSILON 25

MineFSM::MineFSM(OrtsInterface* oi, GroupManager* gm, GameObj* go) 
         : FSM(oi,gm,go) {
  name = SA_MINE;
}

int MineFSM::update(bool& updateRequiredNextCycle) {
  SoarGameGroup* sgg;
  
  switch (state) {
    case IDLE:
      runTime = 0;
      cout << "MINEFSM: start!\n";
      if (gob->get_int("minerals") > 1) {
        state = MOVING_TO_BASE_ZONE_WARMUP;
        gob->set_action("move", moveToBaseZoneParams);
      }
      else {
        state = MOVING_TO_MINE_ZONE_WARMUP;
        gob->set_action("move", moveToMineZoneParams);
      }
      break;
    case MOVING_TO_MINE_ZONE_WARMUP:
      if (runTime < WARMUP_TIME) {
        runTime++;
      }
      else {
        runTime = 0;
        state = MOVING_TO_MINE_ZONE;
      }
      break;
    case MOVING_TO_MINE_ZONE:
      // once we get in range, look for a mineral
      cout << "MINEFSM: mmz\n";
      if ((squaredDistance(*gob->sod.x, *gob->sod.y, mine_x, mine_y) 
          < DISTANCE_EPSILON) or *gob->sod.speed == 0) { 
        sgg = groupMan->getGroupNear("mineral", worldId, 
                                     *gob->sod.x, *gob->sod.y);
        if (sgg != NULL) {
          // there is a mineral patch nearby, head right to a mineral
          SoarGameObject* mineral = sgg->getNextMember();
          target_x = *mineral->gob->sod.x;
          target_y = *mineral->gob->sod.y;
          target_id = mineral->getID();

          tempParams.clear();
          tempParams.push_back(target_x);
          tempParams.push_back(target_y);
          gob->set_action("move", tempParams);
          state = MOVING_TO_MINERAL; // do we need a warmup? 
                                     // we should still be in motion
          cout << "MINEFSM: moving to mineral\n";
        }  
        else if (*gob->sod.speed == 0) {
          // move ended, no mineral found, fail.
          return FSM_FAILURE;
        }
      } 
      // else keep moving
      break;
          
    case MOVING_TO_MINERAL:
      if (not ORTSIO->isAlive(target_id)) {
        // minerals gone!
        cout << "MINEFSM: minerals disappeared! looking for more" << endl;
        state = MOVING_TO_MINE_ZONE_WARMUP;
        gob->set_action("move", moveToMineZoneParams);
      }
      else if (*gob->sod.speed == 0) {
        // need to find out if we can mine while in motion
        tempParams.clear();
        tempParams.push_back(target_id);
        if (gob->component("pickaxe")->set_action("mine", tempParams)) {
          cout << "MINEFSM: mining commencing!\n";
          state = MINING;
        }
        else {
          cout << "Mine command failed. Trying again next time." << endl;
          // if we see this a lot, maybe we need a warmup in here
          // (or just disable the message and let it keep trying)
        }
      }
      /*else { 
        tempParams.clear();
        tempParams.push_back(target_id);
        if (gob->component("pickaxe")->set_action("mine", tempParams)) {
          cout << "MINEFSM: mining commencing!\n";
          state = MINING;
          if (*gob->sod.speed == 0) {
            cout << "MINEFSM: MINING BEFORE STOPPED!!" << endl;
          }
        }
      }
      break;*/
    case MINING:
      if (gob->component("pickaxe")->get_int("active") == 0 &&
          gob->get_int("is_mobile") == 1) 
      {
        // finished mining
        cout << "MINEFSM: FINISHED MINING" << endl;
        state = MOVING_TO_BASE_ZONE_WARMUP;

        gob->set_action("move", moveToBaseZoneParams);
      }
      
      break;
    case MOVING_TO_BASE_ZONE_WARMUP: 
      if (runTime < WARMUP_TIME) {
        runTime++;
      }
      else {
        runTime = 0;
        state = MOVING_TO_BASE_ZONE;
      }
      break;
    case MOVING_TO_BASE_ZONE:
      // once we get in range, look for a command center
      if ((squaredDistance(*gob->sod.x, *gob->sod.y, base_x, base_y) 
          < DISTANCE_EPSILON) or *gob->sod.speed == 0) { 
        sgg = groupMan->getGroupNear("commandCenter", myId, 
                                     *gob->sod.x, *gob->sod.y);
        if (sgg != NULL) {
          // there is a mineral patch nearby, head right to a mineral
          SoarGameObject* base = sgg->getNextMember();
          target_x = *base->gob->sod.x;
          target_y = *base->gob->sod.y;
          target_id = base->getID();

          tempParams.clear();
          tempParams.push_back(target_x);
          tempParams.push_back(target_y);
          gob->set_action("move", tempParams);
          state = MOVING_TO_BASE; // do we need a warmup? 
                                     // we should still be in motion
        }  
        else if (*gob->sod.speed == 0) {
          // move ended, no mineral found, fail.
          return FSM_FAILURE;
        }
      } 
      // else keep moving
      break;
          
    case MOVING_TO_BASE:
      if (not ORTSIO->isAlive(target_id)) {
        // base gone!
        cout << "MINEFSM: base disappeared! looking for more" << endl;
        state = MOVING_TO_BASE_ZONE_WARMUP;
        gob->set_action("move", moveToBaseZoneParams);
      }/*
      else if (*gob->sod.speed == 0) {
        // need to find out if we can mine while in motion
        tempParams.clear();
        tempParams.push_back(target_id);
        if (gob->set_action("return_resources", tempParams)) {
          state = SEND_MOVE_TO_MINE_COMMAND;
          runTime = 0;
        }
        else {
          cout << "Return command failed. Trying again next cycle." << endl;
          runTime++;
          if (runTime > WARMUP_TIME) {
            return FSM_FAILURE;  
          }
        }
      }*/
      else { 
        tempParams.clear();
        tempParams.push_back(target_id);
        if (gob->set_action("return_resources", tempParams)) {
          state = SEND_MOVE_TO_MINE_COMMAND;
          if (*gob->sod.speed == 0) {
            cout << "MINEFSM: DROPOFF BEFORE STOPPED!!" << endl;
          }
        }
      }
      break;
    case SEND_MOVE_TO_MINE_COMMAND:
      // need an extra state here, since the action on the 
      // return is used up by return_resources
      gob->set_action("move", moveToMineZoneParams);
      state = MOVING_TO_MINE_ZONE_WARMUP;
      break;
  } // switch (state)

  updateRequiredNextCycle = true;
  return FSM_RUNNING;
}
          
void MineFSM::init(vector<signed long> p) {
  FSM::init(p);

  state = IDLE;

  assert (p.size() == 4);
  
  mine_x  = p[0];
  mine_y  = p[1];
  base_x  = p[2];
  base_y  = p[3];

  // set up the parameter vectors for later use
  moveToMineZoneParams.push_back(mine_x);
  moveToMineZoneParams.push_back(mine_y);
  moveToBaseZoneParams.push_back(base_x);
  moveToBaseZoneParams.push_back(base_y);

  worldId = ORTSIO->getWorldId();
  myId = ORTSIO->getMyId();
}
/*
private:
enum MineState { IDLE, MINING, MOVING_TO_MINE, MOVING_TO_BASE,
                  MOVING_TO_MINE_WARMUP, MOVING_TO_BASE_WARMUP,
                  SEND_MOVE_TO_MINE_COMMAND};
MineState state;

int mine_id, mine_x, mine_y;
int base_id, base_x, base_y;

int runTime;

Vector<sint4> mineParams, moveToMineParams, depositParams, moveToBaseParams;
Vector<sint4> tempMineParams;
};
*/
