#ifndef MoveFSM_H
#define MoveFSM_H

#include "FSM.h"

class MoveFSM: public FSM {
 public:
	MoveFSM(const Sorts* sorts,  GameObj*);

	int update();
	void init(std::vector<signed long>);

 private:
	enum{IDLE,WARMUP,MOVING};

	int state;
  	int runTime;
};

#endif
