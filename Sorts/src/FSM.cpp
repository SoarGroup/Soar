#include "include/FSM.h"
//#include "include/OrtsInterface.h"

FSM::FSM(GameObj* _gob)
{
  gob = _gob;
}

FSM::~FSM()
{ }

void FSM::init(std::vector<sint4> p) {
  params.clear();
  for(int i=0; i<static_cast<int>(p.size()); i++) {
    params.push_back(p[i]);
  }
}

ObjectActionType FSM::getName() {
  return name;
}

void FSM::panic(){
}
void FSM::stop(){
}
