#include"include/MoveFSM.h"


MoveFSM::MoveFSM()
{
 state = IDLE;
}

MoveFSM::~MoveFSM()
{

}


bool MoveFSM::update()
{
 switch(state){
	case IDLE:
	 //Start moving
	 break;
	case MOVING:
	 //Check to see if you arrived
		//If you arrived, then pop the FSM
	 break;
	}
 return true;
}

void MoveFSM::setParams(std::vector<signed long> p)
{
 //Push the movement params onto the FSM params
 //There should be 3 -> (x, y, speed)

 for(int i=0; i<p.size(); i++)
  params.push_back(p[i]);
}
