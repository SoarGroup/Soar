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
  if (gob->is_pending_action()) {
    cout << "MOVEFSM: action has not taken affect!\n";
    return FSM_RUNNING;
  }

 switch(state){

	case IDLE:
	 //Start moving
   cout << "SETA: " << moveParams[0] << " " << moveParams[1] << endl;
   cout << "on GOB: " << (int)gob << endl;
	 cout << "resp: " << gob->set_action("move", moveParams) << endl;
	 state = MOVING;
   cout << "IDLE!\n";
   break;
  
  case ALREADY_THERE:
   return FSM_SUCCESS; 
   break;

	case MOVING:
	 const ServerObjData &sod = gob->sod;
   cout << "MOVE @ " << *sod.speed << endl;

	 // if speed drops to 0
   // and we are not there, failure
   if (*sod.speed == 0) {
     cout << "ZERO SPEED\n";
     // this should be +/- some amount 
     // to account for multiple objects at the same location 
     if((abs(*sod.x - moveParams[0]) < 30) && abs(*sod.y - moveParams[1]) < 30)
     {
       counter = 0;
       //If you arrived, then check is there is another path segment to traverse
       if (stagesLeft >= 0) {
         cout << "MOVEFSM: next stage\n";
         moveParams[0] = path.locs[stagesLeft].x;
         moveParams[1] = path.locs[stagesLeft].y;
         stagesLeft--;
         cout << "SETA: " << moveParams[0] << " " << moveParams[1] << endl;
	       cout << "resp: " << gob->set_action("move", moveParams) << endl;
       }
       else 
        //Otherwise, you are at your goal
        return FSM_SUCCESS;     
     }
     else {//Try again
       if (counter++ < 2) {
       // just keep trying until its done.
          cout << "MOVEFSM: temp stuck\n";
       }
       else {
         cout << "SETA: " << moveParams[0] << " " << moveParams[1] << endl;
	       cout << "resp: " << gob->set_action("move", moveParams) << endl;
         counter = 0;
        }
          /// return FSM_FAILURE;
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
 //path.locs.clear();
 //path.locs.push_back(l2); 
 for (unsigned int i=0; i<path.locs.size(); i++) 
  cout << "loc " << i << " " << path.locs[i].x << ", "<< path.locs[i].y << endl;

 stagesLeft = path.locs.size()-1;
 moveParams.clear();
 if (path.locs.size() > 0) {
   moveParams.push_back(path.locs[stagesLeft].x);
   moveParams.push_back(path.locs[stagesLeft].y);
   state = IDLE;
 }
 else {
   state = ALREADY_THERE;
 }
}
