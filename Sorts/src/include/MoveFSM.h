#ifndef MoveFSM_H
#define MoveFSM_H

#include "FSM.h"

class MoveFSM: public FSM{
 public:
	MoveFSM();
	~MoveFSM();

	bool update();
//	void setParams(std::vector<signed long>);

 private:
	enum{IDLE,MOVING};

	int state;

};

#endif
