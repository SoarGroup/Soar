#include"MoveFSM.h"
#include<iostream>

#include "Sorts.h"
using namespace std;

//Comment out to turn magnetism off
#define MAGNETISM

MoveFSM::MoveFSM(GameObj* go) 
            : FSM(go) 
{
  name = OA_MOVE;

  sat_loc = Sorts::satellite->addObject(gob);
  vec_count = 0;
  
  loc.x = (*gob->sod.x);
  loc.y = (*gob->sod.y);
  precision = 400;
}

MoveFSM::~MoveFSM()
{
}

int MoveFSM::update() {
  
  loc.x = (*gob->sod.x);
  loc.y = (*gob->sod.y);

  sat_loc = Sorts::satellite->updateObject(gob,sat_loc);

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
     if ((stagesLeft > 0 and distToTarget < 400)
         or (stagesLeft == 0 and distToTarget <= precision)) {
       counter = 0;
       //If you arrived, then check is there is another path segment to traverse
       if (stagesLeft >= 0) {
         target.x = moveParams[0] = path.locs[stagesLeft].x;
         target.y = moveParams[1] = path.locs[stagesLeft].y;
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
          moveParams[0] = target.x;
          moveParams[1] = target.y;
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
     target.x = moveParams[0] = path.locs[stagesLeft].x;
     target.y = moveParams[1] = path.locs[stagesLeft].y;
     stagesLeft--;
     gob->set_action("move", moveParams);
    }

#ifdef MAGENTISM    
    else
    {
     cout<<"Magnetized\n";
     if(getMoveVector())
     {
      cout<<"MoveParams: "<<moveParams[0]<<","<<moveParams[1]<<"\n";
      gob->set_action("move",moveParams);
     }
    }
#endif

   break;

	}
 
 return FSM_RUNNING;
}

void MoveFSM::init(vector<sint4> p) 
{
 FSM::init(p);
 
 TerrainBase::Loc l;
 l.x = p[0];
 l.y = p[1];
 
 if (p.size() == 3) {
   // third parameter specifies how close to the target we must get
   precision = p[2];
   precision *= precision; // since we use distance squared
 }

 Sorts::terrainModule->findPath(gob, l, path);
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
 cout<<"Vec_count: "<<vec_count<<"\n";
 if(vec_count < 5)
 {
  vec_count++;
  return answer;
 }
 answer = true;
 vec_count = 0;

 std::list<GameObj*> *neighbors = Sorts::satellite->getObjectsInRegion(*(gob->sod.x), *(gob->sod.y));
 std::list<GameObj*>::iterator it;
 
 float x = 0;
 float y = 0;

 // Add up the repulsion vectors
 for(it = neighbors->begin(); it!=neighbors->end(); it++)
 {
  x += (*(*it)->sod.x)-loc.x;
  y += (*(*it)->sod.y)-loc.y;
 }
 // Normalize the vector
 float d = sqrt((loc.x-x)*(loc.x-x)+(loc.y-y)*(loc.y-y));
 x /= d;
 y /= d;

 // Add in the attraction vector
 float x1 = target.x - loc.x;
 float y1 = target.y - loc.y;
 d = sqrt((loc.x-x1)*(loc.x-x1)+(loc.y-y1)*(loc.y-y1));

 cout<<"Size: "<< neighbors->size()<<"\n";
 cout<<"Repulsion Vector: ("<<x<<","<<y<<")\n";
 cout<<"Attraction Vector: ("<<x1/d<<","<<y1/d<<")\n";

 x += x1/d;
 y += y1/d;
 cout<<"Combined Vector: ("<<x<<","<<y<<")\n";
 
 TerrainBase::Loc loc = getHeadingVector(static_cast<sint4>(x),static_cast<sint4>(y));
 
 moveParams[0] = loc.x;
 moveParams[1] = loc.y;
 
 return answer;
}

void MoveFSM::stop() {
  Vector<sint4> params;
  params.push_back(*gob->sod.x);
  params.push_back(*gob->sod.y);
  params.push_back(0);
  gob->set_action("move", params);
  state = ALREADY_THERE;
}

double MoveFSM::getHeading(sint4 x, sint4 y)
{
 x=0;
 y=0;
 return 0.0;
}

TerrainBase::Loc MoveFSM::getHeadingVector(sint4 target_x, sint4 target_y)
{
 sint4 x = *(gob->sod.x);
 sint4 y = *(gob->sod.y);
 
 double m = (target_y-y)/(target_x-x);
 double b = (y-m*x);
 int quadrant;
 

 if(x<target_x)
  if(y<target_y)
   quadrant = 4;
  else
   quadrant = 1;
 else
  if(y<target_y)
   quadrant = 2; 
  else
   quadrant = 3;

 
 TerrainBase::Loc loc;
 switch(quadrant){
    case 1:
     if((loc.x = static_cast<sint4>(-1*b/m)) > Sorts::OrtsIO->getMapXDim())
     {
      loc.x = Sorts::OrtsIO->getMapXDim();
      loc.y = static_cast<sint4>(m*loc.x+b);
     }
     else
      loc.y = 0;
     break;
    case 2:
     if((loc.x = static_cast<sint4>(-1*b/m)) < 0)
     {
      loc.x = 0;
      loc.y = static_cast<sint4>(b);
     }
     else
      loc.y = 0;
     break;
    case 3:
     if((loc.x = static_cast<sint4>((Sorts::OrtsIO->getMapYDim()-b)/m)) < 0)
     {
      loc.x = 0;
      loc.y = static_cast<sint4>(b);
     }
     else
      loc.y = Sorts::OrtsIO->getMapYDim();
     break;
    case 4:
     if((loc.x = static_cast<sint4>((Sorts::OrtsIO->getMapYDim()-b)/m)) > Sorts::OrtsIO->getMapXDim())
     {
      loc.x = Sorts::OrtsIO->getMapXDim();
      loc.y = static_cast<sint4>(m*loc.x+b);
     }
     else
      loc.y = Sorts::OrtsIO->getMapYDim();
     break;
    }
    
 cout<<"Heading Vector: ("<<loc.x<<","<<loc.y<<","<<quadrant<<")\n";
 return loc;
}
