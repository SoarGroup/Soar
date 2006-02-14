#include"FSM.h"

FSM::FSM()
{ }

FSM::~FSM()
{ }

void FSM::init(std::vector<signed long> p) {
  params.clear();
  for(int i=0; i<static_cast<int>(p.size()); i++) {
    params.push_back(p[i]);
  }
}
