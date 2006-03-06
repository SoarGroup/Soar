#ifndef MineFSM_H
#define MineFSM_H

#include "FSM.h"
#include "general.h"
#include "OrtsInterface.h"


class MineFSM : public FSM {
public:
	MineFSM(OrtsInterface*, GroupManager*, GameObj*);

  int update();
  void init(vector<signed long> p);
private:
  enum MineState { IDLE, MINING, MOVING_TO_MINE_ZONE, MOVING_TO_BASE_ZONE,
                   MOVING_TO_MINERAL, MOVING_TO_BASE,
                   SEND_MOVE_TO_MINE_COMMAND};
  MineState state;

  int mineZoneX, mineZoneY, baseZoneX, baseZoneY;

  int mineralX, mineralY, mineralId;
  int baseX, baseY, baseId;

  int worldId, myId;

  Vector<sint4> moveToMineZoneParams, moveToBaseZoneParams;
  Vector<sint4> tempParams;
  
  SoarGameObject* mineralObj;
  SoarGameObject* baseObj;
};

#endif
