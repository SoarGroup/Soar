#include"MoveFSM.h"
#include<iostream>

using namespace std;

MoveFSM::MoveFSM()
{
 name = SA_MOVE;
 gob = NULL;
}

MoveFSM::~MoveFSM()
{ }

int MoveFSM::update(bool& requireUpdateNextCycle)
{
 switch(state){

	case IDLE:
	 //Start moving
	 gob->set_action("move",params);
   requireUpdateNextCycle = true;
	 state = WARMUP;
   runTime = 0;
   break;

  case WARMUP:
    // this constant needs to be set more intelligently
    // it may actually depend on our refresh frequency
    
    // require an update, since we aren't moving and won't
    // get refreshed by the world
    requireUpdateNextCycle = true;
    if (runTime < 10) {
      runTime++;
    }
    else {
      state = MOVING;
    }
    break;
	case MOVING:
	 const ServerObjData &sod = gob->sod;

	 // if speed drops to 0 after some warmup period,
   // and we are not there, failure
   if (*sod.speed == 0) {
     // this should be +/- some amount 
     // to account for multiple objects at the same location 
     if((abs(*sod.x - params[1]) < 10) 
         and abs(*sod.y - params[2]) < 10) {
       //If you arrived, then pop the FSM
       return FSM_SUCCESS;
     }
     else {
       return FSM_FAILURE;
     }
   }
	 break;

	}
 return FSM_RUNNING;
}

void MoveFSM::init(vector<signed long> p) {
  FSM::init(p);
  state = IDLE;
}
//Might be worth it to push this up to FSM.h and template it for sint4 and objects
//void MoveFSM::setParams(std::vector<signed long> p)
//{
 //Push the movement params onto the FSM params
 //There should be 3 -> (x, y, speed)

// for(int i=0; i<static_cast<int>(p.size()); i++)
//  params.push_back(p[i]);
//}
