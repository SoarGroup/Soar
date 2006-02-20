#include"include/IdleFSM.h"
#include<iostream>

IdleFSM::IdleFSM()
{
 name = SA_IDLE;
}

IdleFSM::~IdleFSM()
{
 
}

int IdleFSM::update(bool& updateRequiredNextCycle)
{
 std::cout<<"Idle"<<std::endl;
 return FSM_RUNNING;
}
