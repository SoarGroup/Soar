#include "include/FSM.h"
//#include "include/OrtsInterface.h"

FSM::FSM(OrtsInterface* _ORTSIO, GameObj* _gob)
{
  ORTSIO = _ORTSIO;
  gob = _gob;
//  setName();
}

FSM::~FSM()
{ }

void FSM::init(std::vector<signed long> p) {
  params.clear();
  for(int i=0; i<static_cast<int>(p.size()); i++) {
    params.push_back(p[i]);
  }
}

SoarActionType FSM::getName() {
  return name;
}

void FSM::setName() {
  // this should always be overridden.
  assert(false);
}
