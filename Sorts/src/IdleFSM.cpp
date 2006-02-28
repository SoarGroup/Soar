#include"include/IdleFSM.h"
#include<iostream>

IdleFSM::IdleFSM(OrtsInterface* o, GameObj* g) : FSM(o,g) {
  name = SA_IDLE;
}

int IdleFSM::update(bool& updateRequiredNextCycle)
{
 std::cout<<"Idle"<<std::endl;
 return FSM_RUNNING;
}
