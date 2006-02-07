#ifndef _FSM_H_
#define _FSM_H_
#include "FSM.h"
#endif


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
