#ifndef MineFSM_H
#define MineFSM_H

#include "FSM.h"
#include "general.h"
#include "OrtsInterface.h"
#include "MineManager.h"
#include "MoveFSM.h"
struct MiningRoute;

class MineFSM : public FSM {
public:
	MineFSM(GameObj*);
  ~MineFSM();

 	int update();
	void init(vector<sint4> p);
  
  // called by MineManager if a mineral or cc disappears
  void abortMining();
  void setSoarGameObject(SoarGameObject* _sgo) { sgo = _sgo; }
  SoarGameObject* getSoarGameObject() { return sgo; }
private:
  enum MineState { IDLE, MINING, MOVING_TO_MINERAL, MOVING_TO_DROPOFF,
                   SEND_MOVE_TO_MINE_COMMAND };
  MineState state;
  MiningRoute* route;
  SoarGameObject* sgo;
  MoveFSM* moveFSM;
  int precision, timer;
};

#endif
