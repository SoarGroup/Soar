#include"MoveFSM.h"
#include<iostream>
#include<cmath>

#include "Sorts.h"
using namespace std;

#define msg cout << "MOVEFSM: "
#define TOLERANCE 4 // for waypoints, squared
#define VEERWIDTH 10 // radius of worker == 3


//Comment out to turn magnetism off
//#define MAGNETISM

MoveFSM::MoveFSM(GameObj* go) 
            : FSM(go) {
  name = OA_MOVE;

  vec_count = 0;
  
  loc.x = (*gob->sod.x);
  loc.y = (*gob->sod.y);
  precision = 400;
}

MoveFSM::~MoveFSM() {
}

int MoveFSM::update() {
  
  loc.x = (*gob->sod.x);
  loc.y = (*gob->sod.y);
  msg << "current location: " << loc.x << "," << loc.y << endl;

  if (gob->is_pending_action()) {
    msg << "action has not taken affect!\n";
    return FSM_RUNNING;
  }

 switch(state) {

	case IDLE:
    gob->set_action("move", moveParams);
	  state = MOVING;
    counter = 0;
    //counter_max = 1 + (rand() % 4);
    counter_max = 1;// + (rand() %2 );//1 + (rand() % 4);
   break;
  
  case ALREADY_THERE:
    if (squaredDistance(*gob->sod.x, *gob->sod.y, target.x, target.y) 
        <= precision) {
      return FSM_SUCCESS; 
    }
    else {
      // no path was found, and we aren't close enough
      return FSM_FAILURE;
    }
    break;

	case MOVING:
	  const ServerObjData &sod = gob->sod;
    double distToTarget = squaredDistance(*sod.x, *sod.y, target.x, target.y);

	  // if speed drops to 0
    // and we are not there, failure
    if (*sod.speed == 0) {
      if ((stagesLeft > 0 and distToTarget < TOLERANCE)
         or (stagesLeft == -1 and distToTarget <= precision)) {
        counter = 0;
        counter_max = 1;// + (rand() %2 );//1 + (rand() % 4);
        msg << "using counter of " << counter_max << endl;
        // If you arrived, then check if 
        // there is another path segment to traverse
        if (stagesLeft >= 0) {
          target.x = moveParams[0] = path.locs[stagesLeft].x;
          target.y = moveParams[1] = path.locs[stagesLeft].y;
          stagesLeft--;
	        gob->set_action("move", moveParams);
        }
        else { 
          msg << "dist: " << distToTarget << endl;
          msg << "target: " << target.x << target.y << endl;
          // Otherwise, you are at your goal
          return FSM_SUCCESS;     
        }
      }
      else {//Try again
        if (counter++ < counter_max) {
          veerRight();
          // veerRight may or may not adjust target
          moveParams[0] = target.x;
          moveParams[1] = target.y;
	        gob->set_action("move", moveParams);
          msg << "veer right\n";
        }
        else {
        // cout << "SETA: " << moveParams[0] << " " << moveParams[1] << endl;
          return FSM_FAILURE;
        }
      }
    }
    else if (distToTarget <= 1 and stagesLeft >= 0) {
      cout << "MOVEFSM: in-motion dir change\n";
      target.x = moveParams[0] = path.locs[stagesLeft].x;
      target.y = moveParams[1] = path.locs[stagesLeft].y;
      stagesLeft--;
      gob->set_action("move", moveParams);
    }

#ifdef MAGNETISM    
    else {
      cout << "MOVEFSM: Magnetized\n";
      if(getMoveVector()) {
        cout<<"MOVEFSM: MoveParams: "<<moveParams[0]<<","<<moveParams[1]<<"\n";
        gob->set_action("move",moveParams);
      }
    }
#endif

    break;

	} // switch (state)
 
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
  msg << "initialized. Path " << *gob->sod.x << "," << *gob->sod.y << "->"
      << l.x << "," << l.y << endl;
  for (unsigned int i=0; i<path.locs.size(); i++) {
    msg << "loc " << i << " " 
        << path.locs[i].x << ", "<< path.locs[i].y << endl;
  }
  stagesLeft = path.locs.size()-1;
  moveParams.clear();
  if (path.locs.size() > 0) {
    moveParams.push_back(path.locs[stagesLeft].x);
    moveParams.push_back(path.locs[stagesLeft].y);
    state = IDLE;
    target.x = moveParams[0];
    target.y = moveParams[1];
  }
  else {
    target.x = l.x;
    target.y = l.y;
    state = ALREADY_THERE; // or no path!
  }
}

void MoveFSM::stop() {
  Vector<sint4> params;
  params.push_back(*gob->sod.x);
  params.push_back(*gob->sod.y);
  params.push_back(0);
  gob->set_action("move", params);
  state = ALREADY_THERE;
}

void MoveFSM::veerRight() {
  // if there is open space to the right,
  // make that the new target.
  // if that happens, increment stagesLeft so the current waypoint
  // is moved to after we get to the right
  // do not allow multiple rightward moves! better to just return failure
  // so the FSM is reinitted with a new path

  assert(stagesLeft+1 < path.locs.size());

  if (target != path.locs[stagesLeft+1]) {
    // we've already veered, do nothing
    msg << "already veered!\n";
    return;
  }
 
  double deltaX = target.x - *gob->sod.x;
  double deltaY = target.y - *gob->sod.y;
 
  double headingAngle = atan2(deltaY, deltaX); // -pi to pi (deltaX == 0 ok)
  headingAngle += (PI/2.0);

  int newX = *gob->sod.x + (int)(VEERWIDTH*cos(headingAngle));
  int newY = *gob->sod.y + (int)(VEERWIDTH*sin(headingAngle));

  msg << "veer " << *gob->sod.x << "," << *gob->sod.y << "->"
      << newX << "," << newY << " on the way to " 
      << target.x << "," << target.y << endl;

  if (!collision(newX, newY)) {
    target.x = newX;
    target.y = newY;
    stagesLeft++;
  }

  return;
}

bool MoveFSM::collision(int x, int y) {
  list<GameObj*> collisions;
  
  Sorts::satellite->getCollisions(x, y, 6, collisions);
  msg << x << "," << y << " collides with " << collisions.size() 
       << " things.\n";
 /* 
  for (list<GameObj*>::iterator it = collisions.begin();
       it != collisions.end();
       it++) {
    if ((*it)->bp_name() != "worker"
        and (*it)->bp_name() != "sheep") {
      cout << "MM: station collides with " << (*it)->bp_name() << endl;
      cout << "MM: loc: " << *(*it)->sod.x << "," << *(*it)->sod.y << endl;
      msg << "radius " << *(*it)->sod.radius << endl; 
      station->optimality = UNUSABLE_OPTIMALITY;
      return true;
    }
    else { 
      cout << "MM: ignoring worker collision.\n";
    }
  }
*/
  return (collisions.size() > 0);
}
// MAGNETISM CODE

// Returns the actual move vector of the object
// New move coordinates are placed into the moveParams structure
bool MoveFSM::getMoveVector()
{
 bool answer = false;
 // - Things don't change rapidly
 // - Return false is not enough time has passed
 //   - Need to figure out how much time is enough
 cout<<"MOVEFSM: Vec_count: "<<vec_count<<"\n";
// if(vec_count < 5)
 {
  vec_count++;
//  return answer;
 }
 answer = true;
 vec_count = 0;

 std::list<GameObj*> *neighbors = new list<GameObj*>;// = Sorts::satellite->getObjectsInRegion(*(gob->sod.x), *(gob->sod.y));
 std::list<GameObj*>::iterator it;
 
 float x = 0;
 float y = 0;

 // Add up the repulsion vectors
 for(it = neighbors->begin(); it!=neighbors->end(); it++)
 {
   // normalize? /dist^2
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

 cout<<"MOVEFSM: Size: "<< neighbors->size()<<"\n";
 cout<<"MOVEFSM: Repulsion Vector: ("<<x<<","<<y<<")\n";
 cout<<"MOVEFSM: Attraction Vector: ("<<x1/d<<","<<y1/d<<")\n";

 x += x1/d;
 y += y1/d;
 cout<<"Combined Vector: ("<<x<<","<<y<<")\n";
 
 TerrainBase::Loc loc = getHeadingVector(static_cast<sint4>(x),static_cast<sint4>(y));
 
 moveParams[0] = loc.x;
 moveParams[1] = loc.y;
 
 return answer;
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
    
 cout<<"MOVEFSM: Heading Vector: ("<<loc.x<<","<<loc.y<<","<<quadrant<<")\n";
 return loc;
}

