#include "include/FSM.h"
//#include "include/OrtsInterface.h"

FSM::FSM(OrtsInterface* _ORTSIO, GroupManager* _groupMan, GameObj* _gob)
{
  ORTSIO = _ORTSIO;
  gob = _gob;
  groupMan = _groupMan;
}

FSM::~FSM()
{ }

void FSM::init(std::vector<signed long> p) {
  params.clear();
  for(int i=0; i<static_cast<int>(p.size()); i++) {
    params.push_back(p[i]);
  }
}

ObjectActionType FSM::getName() {
  return name;
}
