#ifndef MineFSM_H
#define MineFSM_H

#include "FSM.h"
#include "general.h"

#include "PlayerInfo.H"
#include "Game.H"
#include "GameStateModule.H"

class MineFSM : public FSM {
public:
  MineFSM() {
    name = SA_MINE;
  }

  int update(bool& updateRequiredNextCycle) {
    cout << "WORKER HAS: " << gob->get_int("minerals") << " MINERALS " << endl;

    // TO DO: check if the mineral patch is out. If it is, return false

    switch (state) {
      case IDLE:
        // figure out what to do first
        cout << "IDLE" << endl;
        runTime = 0;
        if (gob->get_int("minerals") > 1) {
          state = MOVING_TO_BASE_WARMUP;
          gob->set_action("move", moveToBaseParams);
        }
        else {
          state = MOVING_TO_MINE_WARMUP;
          gob->set_action("move", moveToMineParams);
        }
        break;

      case MINING:
        cout << "MINING" << endl;

        if (gob->component("pickaxe")->get_int("active") == 0 &&
            gob->get_int("is_mobile") == 1) 
        {
          // finished mining
          cout << "FINISHED MINING" << endl;
          state = MOVING_TO_BASE_WARMUP;
          runTime = 0;

          // have to help it along here, or else it's not going to change
          // next cycle
          gob->set_action("move", moveToBaseParams);
        }
        
        break;

      case MOVING_TO_MINE_WARMUP:
        if (runTime < 30) {
          runTime++;
        }
        else {
          state = MOVING_TO_MINE;
        }
        break;
      case MOVING_TO_MINE:
        cout << "MOVING TO MINE: " << squaredDistance(*gob->sod.x, *gob->sod.y, mine_x, mine_y) << endl; 
        //if (squaredDistance(*gob->sod.x, *gob->sod.y, mine_x, mine_y) < 100) {
        if (*gob->sod.speed == 0) {
          cout << "SENT MINE COMMAND" << endl;
          tempMineParams = mineParams;
          cout << tempMineParams.size() << endl;
          gob->component("pickaxe")->set_action("mine", tempMineParams);
          cout << tempMineParams.at(0) << endl;
          state = MINING;
        }
        break;
      
      case MOVING_TO_BASE_WARMUP:
        if (runTime < 30) {
          runTime++;
        }
        else {
          state = MOVING_TO_BASE;
        }
        break;

      case MOVING_TO_BASE:
        cout << "MOVING TO BASE" << squaredDistance(*gob->sod.x, *gob->sod.y, base_x, base_y) << endl; 
        //if (squaredDistance(*gob->sod.x, *gob->sod.y, base_x, base_y) < 2300) {
        if (*gob->sod.speed == 0) {
          gob->set_action("return_resources", depositParams);
          state = SEND_MOVE_TO_MINE_COMMAND;
          runTime = 0;
        }
        break;
        case SEND_MOVE_TO_MINE_COMMAND:
          // need an extra state here, since the action on the 
          // return is used up by return_resources
          gob->set_action("move", moveToMineParams);
          state = MOVING_TO_MINE_WARMUP;
        break;
    } // switch (state)
    
    updateRequiredNextCycle = true;

    return FSM_RUNNING;
  }

  /* the order of the parameters will be:
   *
   * mine id, mine x, mine y, base id, base x, base y
   */

  void init(vector<signed long> p) {
    FSM::init(p);

    state = IDLE;

    mine_x  = p[0];
    mine_y  = p[1];
    mine_id = p[2];
    base_x  = p[3];
    base_y  = p[4];
    base_id = p[5];

    // set up the parameter vectors for later use
    moveToMineParams.push_back(mine_x);
    moveToMineParams.push_back(mine_y);
    mineParams.push_back(mine_id);
    moveToBaseParams.push_back(base_x);
    moveToBaseParams.push_back(base_y);
    depositParams.push_back(base_id);
  }

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

#endif
