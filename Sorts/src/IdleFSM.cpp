#include"include/IdleFSM.h"
#include<iostream>

void IdleFSM::setName()
{
 name = SA_IDLE;
}

IdleFSM::~IdleFSM()
{
 
}
/*IdleFSM::IdleFSM(OrtsInterface* oio, GameObj* go) {
 // ORTSIO = oio;
 // gob = go;
 // setName();
}*/

/*IdleFSM::IdleFSM() {
}*/

int IdleFSM::update(bool& updateRequiredNextCycle)
{
 std::cout<<"Idle"<<std::endl;
 return FSM_RUNNING;
}
