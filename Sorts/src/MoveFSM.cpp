#include"MoveFSM.h"
#include<iostream>
using namespace std;

MoveFSM::MoveFSM()
{
 state = IDLE;
 name = SA_MOVE;
 gob = NULL;
}

MoveFSM::~MoveFSM()
{
 gob = NULL;
}


bool MoveFSM::update()
{
 switch(state){

	case IDLE:
	 //Start moving
	 gob->set_action("move",params);
   cout << "MOVING!" << endl;
	 break;

	case MOVING:
	 const ServerObjData &sod = gob->sod;

	 //Check to see if you arrived
	 if(*sod.x == params[1] && *sod.y == params[2])
	 {
	  //If you arrived, then pop the FSM
	  state = IDLE;
	  return false;
	 }
	 break;

	}
 return true;
}


//Might be worth it to push this up to FSM.h and template it for sint4 and objects
//void MoveFSM::setParams(std::vector<signed long> p)
//{
 //Push the movement params onto the FSM params
 //There should be 3 -> (x, y, speed)

// for(int i=0; i<static_cast<int>(p.size()); i++)
//  params.push_back(p[i]);
//}
