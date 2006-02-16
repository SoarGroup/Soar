#include"include/IdleFSM.h"
#include<iostream>

IdleFSM::IdleFSM()
{
 name = SA_IDLE;
}

IdleFSM::~IdleFSM()
{
 
}

int IdleFSM::update()
{
 std::cout<<"Idle"<<std::endl;
 return FSM_RUNNING;
}
