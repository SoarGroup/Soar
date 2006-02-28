#ifndef MineFSM_H
#define MineFSM_H

#include "FSM.h"
#include "general.h"
#include "OrtsInterface.h"


class MineFSM : public FSM {
public:
	MineFSM(OrtsInterface* o, GameObj* g);

  int update(bool& updateRequiredNextCycle);
  void init(vector<signed long> p);
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
