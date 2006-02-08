#include"include/IdleFSM.h"
#include<iostream>

IdleFSM::IdleFSM()
{
 name = SA_IDLE;
}

IdleFSM::~IdleFSM()
{
 
}

bool IdleFSM::update()
{
 std::cout<<"Idle"<<std::endl;
 return true;
}
