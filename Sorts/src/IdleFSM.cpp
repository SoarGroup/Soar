#include"include/IdleFSM.h"
#include<iostream>

IdleFSM::IdleFSM(OrtsInterface* oi, GroupManager* gm, GameObj* go) 
          : FSM(oi,gm,go) {
  name = SA_IDLE;
}

int IdleFSM::update(bool& updateRequiredNextCycle)
{
 std::cout<<"Idle"<<std::endl;
 return FSM_RUNNING;
}
