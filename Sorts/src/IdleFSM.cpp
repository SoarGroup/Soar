#include"include/IdleFSM.h"
#include<iostream>

IdleFSM::IdleFSM(OrtsInterface* oi, GroupManager* gm, GameObj* go) 
          : FSM(oi,gm,go) {
  name = OA_IDLE;
}

int IdleFSM::update()
{
 //std::cout<<"Idle"<<std::endl;
 return FSM_RUNNING;
}
