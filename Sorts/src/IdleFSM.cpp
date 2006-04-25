#include"include/IdleFSM.h"
#include<iostream>

IdleFSM::IdleFSM(Sorts *so, GameObj* go) 
          : FSM(so,go) {
  name = OA_IDLE;
}

int IdleFSM::update()
{
 //std::cout<<"Idle"<<std::endl;
 return FSM_RUNNING;
}
