#include"include/IdleFSM.h"
#include<iostream>

IdleFSM::IdleFSM(GameObj* go) 
          : FSM(go) {
  name = OA_IDLE;
}

int IdleFSM::update()
{
 //std::cout<<"Idle"<<std::endl;
 return FSM_RUNNING;
}
