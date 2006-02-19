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
    firstMine = true;
  }

  int update() {
    cout << "WORKER HAS: " << gob->get_int("minerals") << " MINERALS " << endl;

    // TO DO: check if the mineral patch is out. If it is, return false

    switch (state) {
      case IDLE:
        // figure out what to do first
        cout << "IDLE" << endl;
        if (gob->get_int("minerals") > 1) {
          state = MOVING_TO_BASE;
        }
        else {
          state = MOVING_TO_MINE;
        }
        break;

      case MINING:
        cout << "MINING" << endl;

        if (firstMine) {
          cout << "SENT MINE COMMAND" << endl;
          gob->component("pickaxe")->set_action("mine", mineParams);
          firstMine = false;
        }
        else {
          if (gob->component("pickaxe")->get_int("active") == 0 &&
              gob->get_int("is_mobile") == 1) 
          {
            // finished mining
            cout << "FINISHED MINING" << endl;
            state = MOVING_TO_BASE;
            firstMine = true; // for the next time

            // have to help it along here, or else it's not going to change
            // next cycle
            gob->set_action("move", moveToBaseParams);
          }
        }
        
        break;

      case MOVING_TO_MINE:
        cout << "MOVING TO MINE: " << squaredDistance(*gob->sod.x, *gob->sod.y, mine_x, mine_y) << endl; 
        if (squaredDistance(*gob->sod.x, *gob->sod.y, mine_x, mine_y) < 100) {
          // close to mine. The 100 is a guess
          gob->set_action("mine", mineParams);
          state = MINING;

        }
        else {
          gob->set_action("move", moveToMineParams);
        }
        break;

      case MOVING_TO_BASE:
        cout << "MOVING TO BASE" << squaredDistance(*gob->sod.x, *gob->sod.y, base_x, base_y) << endl; 
        if (squaredDistance(*gob->sod.x, *gob->sod.y, base_x, base_y) < 2300) {
          // close to base. The 2300 is a guess
          gob->set_action("return_resources", depositParams);
          state = MOVING_TO_MINE;
        }
        else {
          gob->set_action("move", moveToBaseParams);
        }
        break;
    } // switch (state)

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
  enum MineState { IDLE, MINING, MOVING_TO_MINE, MOVING_TO_BASE };
  MineState state;

  int mine_id, mine_x, mine_y;
  int base_id, base_x, base_y;

  bool firstMine;

  Vector<sint4> mineParams, moveToMineParams, depositParams, moveToBaseParams;
};

#endif
