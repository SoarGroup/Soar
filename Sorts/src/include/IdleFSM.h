#ifndef _FSM_H_
#define _FSM_H_
#include "FSM.h"
#endif

class IdleFSM: public FSM{
 public:
	IdleFSM(GameObj*); 

	int update();

};
