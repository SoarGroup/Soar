#include"MoveFSM.h"
#include<iostream>

#include "Sorts.h"
using namespace std;

SPathFinder *MoveFSM::pather=NULL;


MoveFSM::MoveFSM(const Sorts *so, GameObj* go) 
            : FSM(so,go) {
  name = OA_MOVE;

  if(!pather)
   pather = new SPathFinder(so);
}

int MoveFSM::update()
{
/*
 switch(state){

	case IDLE:
	 //Start moving
	 gob->set_action("move",params);
	 state = MOVING;
   break;

	case MOVING:
	 const ServerObjData &sod = gob->sod;

	 // if speed drops to 0
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
*/  
 return FSM_RUNNING;
}

void MoveFSM::init(vector<signed long> p) 
{
 FSM::init(p);
 real4 x,y;

 Vector<GameObj*> objs;
 objs.push_back(gob);
                
 x = params[0];
 y = params[1];
 pather->handle_event(PathEvent(EventFactory::new_who(), SPathFinder::FIND_PATH_MSG, Coor3(x,y,0),objs)); 
}
//Might be worth it to push this up to FSM.h and template it for sint4 and objects
//void MoveFSM::setParams(std::vector<signed long> p)
//{
 //Push the movement params onto the FSM params
 //There should be 3 -> (x, y, speed)

// for(int i=0; i<static_cast<int>(p.size()); i++)
//  params.push_back(p[i]);
//}
