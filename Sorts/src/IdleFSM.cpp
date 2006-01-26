#include"include/IdleFSM.h"
#include<iostream>

IdleFSM::IdleFSM()
{
 name = "Idle";
}

IdleFSM::~IdleFSM()
{
 
}

bool IdleFSM::update()
{
 std::cout<<"Idle"<<std::endl;
 return true;
}
