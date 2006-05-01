#include"MoveFSM.h"
#include<iostream>

#include "Sorts.h"
using namespace std;

//SPathFinder *MoveFSM::pather=NULL;


MoveFSM::MoveFSM(const Sorts *so, GameObj* go) 
            : FSM(so,go) 
{
  name = OA_MOVE;
}

int MoveFSM::update()
{

 switch(state){

	case IDLE:
	 //Start moving
	 gob->set_action("move", moveParams);
	 state = MOVING;
   break;

	case MOVING:
	 const ServerObjData &sod = gob->sod;

	 // if speed drops to 0
   // and we are not there, failure
   if (*sod.speed == 0) {
     // this should be +/- some amount 
     // to account for multiple objects at the same location 
     if((abs(*sod.x - moveParams[0]) < 10) && abs(*sod.y - moveParams[1]) < 10)
     {
       counter = 0;
       //If you arrived, then check is there is another path segment to traverse
       if (stagesLeft >= 0) {
         cout << "MOVEFSM: next stage\n";
         moveParams[0] = path.locs[stagesLeft].x;
         moveParams[1] = path.locs[stagesLeft].y;
         stagesLeft--;
         gob->set_action("move", moveParams);
       }
       else 
        //Otherwise, you are at your goal
        return FSM_SUCCESS;     
     }
     else {//Try again
       if (counter++ < 10)
          gob->set_action("move", moveParams);
       // just keep trying until its done.
       else
         return FSM_FAILURE;
     }
   }
	 break;

	}
 
 return FSM_RUNNING;
}

void MoveFSM::init(vector<sint4> p) 
{
 FSM::init(p);

 TerrainBase::Loc l2;
 l2.x = p[0];
 l2.y = p[1];
 
 sorts->terrainModule->findPath(gob, l2, path);
  
 for (unsigned int i=0; i<path.locs.size(); i++) 
  cout << "loc " << i << " " << path.locs[i].x << ", "<< path.locs[i].y << endl;

 stagesLeft = path.locs.size()-1;
 moveParams.clear();
 if (path.locs.size() > 0) 
 {
   moveParams.push_back(path.locs[stagesLeft].x);
   moveParams.push_back(path.locs[stagesLeft].y);
 }
}
