#ifndef MoveFSM_H
#define MoveFSM_H

#include "FSM.h"
#include "Sorts.h"

class MoveFSM: public FSM {
 public:
  MoveFSM(GameObj*);

  int update();
	void init(std::vector<sint4>);

 private:
	enum{IDLE,WARMUP,MOVING,ALREADY_THERE};

	int state;
  int runTime;

  TerrainBase::Path path;
  int stagesLeft;
  Vector<sint4> moveParams;
  int counter;
};

#endif
