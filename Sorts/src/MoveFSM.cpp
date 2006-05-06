#include"MoveFSM.h"
#include<iostream>

#include "Sorts.h"
using namespace std;

Satellite *MoveFSM::satellite = NULL;


MoveFSM::MoveFSM(GameObj* go) 
            : FSM(go) 
{
  name = OA_MOVE;

  if(!satellite)
  {
   satellite = new Satellite();
   satellite->init();
  }
  else
   satellite->refCount++;

  satellite->addObject(gob);
  vec_count = 0;
}

MoveFSM::~MoveFSM()
{
 if(satellite->refCount==1)
 {
  delete satellite;
  satellite = NULL;
 }
}

int MoveFSM::update() {
  
  if (gob->is_pending_action()) {
    cout << "MOVEFSM: action has not taken affect!\n";
    return FSM_RUNNING;
  }

 switch(state){

	case IDLE:
   gob->set_action("move", moveParams);
	 state = MOVING;
   break;
  
  case ALREADY_THERE:
   return FSM_SUCCESS; 
   break;

	case MOVING:
	 const ServerObjData &sod = gob->sod;
   double distToTarget = squaredDistance(*sod.x, *sod.y, moveParams[0], moveParams[1]); 
	 // if speed drops to 0
   // and we are not there, failure
   if (*sod.speed == 0) 
   {
     // this should be +/- some amount 
     // to account for multiple objects at the same location 
     if (distToTarget < 400) {
       counter = 0;
       //If you arrived, then check is there is another path segment to traverse
       if (stagesLeft >= 0) {
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
       if (counter++ < 5) {
       // just keep trying until its done.
	        gob->set_action("move", moveParams);
          cout << "MOVEFSM: temp stuck\n";
       }
       else {
        // cout << "SETA: " << moveParams[0] << " " << moveParams[1] << endl;
         return FSM_FAILURE;
       }
     }
   }
   else 
    if (distToTarget <= 1 and stagesLeft >= 0) 
    {
     cout << "MOVEFSM: in-motion dir change\n";
     moveParams[0] = path.locs[stagesLeft].x;
     moveParams[1] = path.locs[stagesLeft].y;
     stagesLeft--;
     gob->set_action("move", moveParams);
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
 
 Sorts::terrainModule->findPath(gob, l2, path);
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


// Returns the actual move vector of the object
// New move coordinates are placed into the moveParams structure
bool MoveFSM::getMoveVector()
{
 bool answer = false;
 // - Things don't change rapidly
 // - Return false is not enough time has passed
 //   - Need to figure out how much time is enough
 if(vec_count < 20)
 {
  vec_count++;
  return answer;
 }

 std::list<GameObj*> *neighbors = satellite->getObjectsInRegion(*(gob->sod.x), *(gob->sod.y));
 std::list<GameObj*>::iterator it;

 // Add up the repulsion vectors
 for(it = neighbors->begin(); it!=neighbors->end(); it++)
 {
  
 }
 
 //Add in the attraction vector
 sint4 x = path.locs[stagesLeft].x;
 sint4 y = path.locs[stagesLeft].y;

 //This won't work... Just trying something out
 if(abs(x-moveParams[0])>10)
 {
  moveParams[0] = x;
  answer = true;
 }
 if(abs(y-moveParams[1])>10)
 {
  moveParams[1] = y;
  answer = true;
 }
 
 return answer;
}

