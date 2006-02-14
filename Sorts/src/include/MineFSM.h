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

  bool update() {
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
        if (gob->get_int("minerals") > 1) {
          // finished mining
          state = MOVING_TO_BASE;
        }
        else {
          gob->set_action("mine", mineParams);
        }
        // otherwise don't change anything
        break;

      case MOVING_TO_MINE:
        cout << "MOVING TO MINE: " << squaredDistance(*gob->sod.x, *gob->sod.y, mine_x, mine_y) << endl; 
        if (squaredDistance(*gob->sod.x, *gob->sod.y, mine_x, mine_y) < 95) {
          // close to mine. The 95 is a guess
          gob->set_action("mine", mineParams);
          state = MINING;

        }
        else {
          gob->set_action("move", moveToMineParams);
        }
        break;

      case MOVING_TO_BASE:
        cout << "MOVING TO BASE" << squaredDistance(*gob->sod.x, *gob->sod.y, base_x, base_y) << endl; 
        if (squaredDistance(*gob->sod.x, *gob->sod.y, base_x, base_y) < 95) {
          // close to base. The 95 is a guess
          gob->set_action("return_resources", depositParams);
          state = MOVING_TO_MINE;

        }
        else {
          gob->set_action("move", moveToBaseParams);
        }
        break;
    } // switch (state)

    return true;
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

  Vector<sint4> mineParams, moveToMineParams, depositParams, moveToBaseParams;
};

#endif
