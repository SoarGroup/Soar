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
     if((abs(*sod.x - moveParams[0]) < 10) 
         and abs(*sod.y - moveParams[1]) < 10) {
       //If you arrived, then pop the FSM
       
       if (stagesLeft >= 0) {
         cout << "MOVEFSM: next stage\n";
         moveParams[0] = path.locs[stagesLeft].x;
         moveParams[1] = path.locs[stagesLeft].y;
         stagesLeft--;
         gob->set_action("move", moveParams);
       }
       else return FSM_SUCCESS;
     }
     else {
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
 real4 x,y;

 Vector<GameObj*> objs;
 objs.push_back(gob);
                
 x = params[0];
 y = params[1];
 counter = 0;

 TerrainBase::Loc l2;
 l2.x = x;
 l2.y = y;
 
 sorts->terrainModule->findPath(gob, l2, path);
/* pather->handle_event(PathEvent(EventFactory::new_who(), 
                                SPathFinder::FIND_PATH_MSG, 
                                Coor3(x,y,0),objs));
*/
  cout << "PF DONE:\n";
  
  for (int i=0; i<path.locs.size(); i++) {
    cout << "loc " << i << " " << path.locs[i].x << ", "
                               << path.locs[i].y << endl;
  }

  stagesLeft = path.locs.size()-1;
  moveParams.clear();
  if (path.locs.size() > 0) {
    moveParams.push_back(path.locs[stagesLeft].x);
    moveParams.push_back(path.locs[stagesLeft].y);
  }
}
//Might be worth it to push this up to FSM.h and template it for sint4 and objects
//void MoveFSM::setParams(std::vector<sint4> p)
//{
 //Push the movement params onto the FSM params
 //There should be 3 -> (x, y, speed)

// for(int i=0; i<static_cast<int>(p.size()); i++)
//  params.push_back(p[i]);
//}
