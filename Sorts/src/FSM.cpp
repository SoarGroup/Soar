#include"FSM.h"

FSM::FSM()
{

}

FSM::~FSM()
{

}


void FSM::setParams(std::vector<signed long> p)
{
 for(int i=0; i<static_cast<int>(p.size()); i++)
  params.push_back(p[i]);
}
