#include"MoveFSM.h"
#include<iostream>
#include<cmath>

#include "Sorts.h"
using namespace std;

#define msg cout << "MOVEFSM(" << (int)this << "): "
#define TOLERANCE 9 // for waypoints, squared
#define VEERWIDTH 10  // radius of worker == 3


// these quantities are determined randomly within a range
// veercount: number of veers allowed in a row
// if exceeded, veering does nothing (keep pushing ahead)
#define MIN_VEERCOUNT 1 
#define MAX_VEERCOUNT_DIFF 4 //  min < value < min+maxdiff

// define to veer right, left, right .. etc (vs. always right)
#define ALTERNATE_VEERING

// counter: number of consecutive in-place veers before FSM_FAILURE returned
// i.e. the max number of cycles to allow zero speed before giving up
#define MIN_COUNTER 1
#define MAX_COUNTER_DIFF 3 // min < value <  min+maxdiff

//Comment out to turn magnetism off
//#define MAGNETISM
#define RADAR

#define WAYPOINT_IMAGINARY_WORKERS
  
/*
  radar parameters used by the veerAhead function
  look in front of the gob, if the spot RADAR_FORWARD_DIST ahead
  is blocked, veer around it (if that spot is not blocked)

    radar forward distance
        v 
   obj --- obstacle
      \radar veer angle
       \
        \radar angle dist (length of line)
         \
          go here instead

   if the distance between the obstacle and the new spot is x,
   rva = tan-1(x/rfd) (in radians!)
   rad = sqrt(x^2 + rfd^2)
*/

                             // w/ itself!
#define RADAR_FORWARD_DIST_SQ RADAR_FORWARD_DIST * RADAR_FORWARD_DIST

// x=6, rfd 7
#define RADAR_FORWARD_DIST 7 // must be > 6, or the worker will collide 
#define RADAR_VEER_ANGLE .70862
#define RADAR_ANGLE_DIST 9.22

//x=10, rfd 7
//#define RADAR_FORWARD_DIST 7 // must be > 6, or the worker will collide 
//#define RADAR_VEER_ANGLE .96
//#define RADAR_ANGLE_DIST 12.2


MoveFSM::MoveFSM(GameObj* go) 
            : FSM(go) {
  name = OA_MOVE_INTERNAL;

  vec_count = 0;
  veerCount = 0;
  
  currentLocation.x = (*gob->sod.x);
  currentLocation.y = (*gob->sod.y);
  precision = 400;
  lastRight = false;
  lastLocation.x = -1;
  lastLocation.y = -1;
  nextWPIndex = -1;
  usingIWWP = false;
}

MoveFSM::~MoveFSM() {
}

int MoveFSM::update() {
 // bool noEffectLastFrame = noEffect;
  
  currentLocation.x = (*gob->sod.x);
  currentLocation.y = (*gob->sod.y);
  msg << "current location: " << currentLocation.x << "," 
                              << currentLocation.y << endl;

  if (gob->is_pending_action()) {
    msg << "action has not taken affect!\n";
    return FSM_RUNNING;
  }

  // FIXME
  if (rand()% 2) {
    lastRight = true;
  }
  else {
    lastRight = false;
  }
  
 switch(state) {

	case IDLE:
    msg << "idle\n";
    gob->set_action("move", moveParams);
	  state = MOVING;
    counter = 0;
    counter_max = MIN_COUNTER + (rand()%MAX_COUNTER_DIFF );
   break;
  
  case ALREADY_THERE:
   msg << "already there\n";
    if (squaredDistance(*gob->sod.x, *gob->sod.y, target.x, target.y) 
        <= precision) {
      return FSM_SUCCESS; 
    }
    else {
      // no path was found, and we aren't close enough
      return FSM_UNREACHABLE;
    }
    break;

	case MOVING:
	  msg << "moving @ speed " << *gob->sod.speed << "\n";
    const ServerObjData &sod = gob->sod;
    double distToTarget = squaredDistance(*sod.x, *sod.y, target.x, target.y);

	  // if speed drops to 0
    // and we are not there, failure
    if (*sod.speed == 0 or (currentLocation == lastLocation)) {
      if (*sod.speed > 0) {
        msg << "moving, but stuck, somehow.\n";
      }
      if ((nextWPIndex >= 0 and distToTarget < TOLERANCE)
         or (nextWPIndex == -1 and distToTarget <= precision)) {
        counter = 0;
        counter_max = MIN_COUNTER + (rand()%MAX_COUNTER_DIFF );
        msg << "using counter of " << counter_max << endl;
        // If you arrived, then check if 
        // there is another path segment to traverse
        if (nextWPIndex >= 0) {
          if (nextWPIndex+1 < path.locs.size() and
              target == path.locs[nextWPIndex+1]) {
            // this means we reached a "real" waypoint (not one set by veering)
            veerCount = 0;
          }
          if (usingIWWP and target == imaginaryWorkerWaypoint) {
            Sorts::terrainModule->
              removeImaginaryWorker(imaginaryWorkerWaypoint);
            usingIWWP = false;
          }
          target.x = moveParams[0] = path.locs[nextWPIndex].x;
          target.y = moveParams[1] = path.locs[nextWPIndex].y;
          nextWPIndex--;
	        gob->set_action("move", moveParams);
        }
        else { 
          // nextWPIndex == -1: we must be there
          msg << "dist: " << distToTarget << endl;
          msg << "target: " << target.x << target.y << endl;
          // Otherwise, you are at your goal
          if (usingIWWP and target == imaginaryWorkerWaypoint) {
            Sorts::terrainModule->
              removeImaginaryWorker(imaginaryWorkerWaypoint);
            usingIWWP = false;
          }
          return FSM_SUCCESS;     
        }
      }
      else {//Try again
        if (counter++ < counter_max) {
          if (Sorts::OrtsIO->getSkippedActions() == 0) {
            veerRight();
            // veerRight may or may not adjust target
          }
          moveParams[0] = target.x;
          moveParams[1] = target.y;
	        gob->set_action("move", moveParams);
        }
        else {
        // cout << "SETA: " << moveParams[0] << " " << moveParams[1] << endl;
          msg << "failed, must repath\n";
          clearWPWorkers();
          return FSM_FAILURE;
        }
      }
    }
    else if (distToTarget <= TOLERANCE and nextWPIndex >= 0) {
      counter = 0;
      cout << "MOVEFSM: in-motion dir change\n";
      if (nextWPIndex+1 < path.locs.size() and
          target == path.locs[nextWPIndex+1]) {
        // this means we reached a "real" waypoint (not one set by veering)
        veerCount = 0;
        if (usingIWWP and target == imaginaryWorkerWaypoint) {
          Sorts::terrainModule->
            removeImaginaryWorker(imaginaryWorkerWaypoint);
          usingIWWP = false;
        }
      }
        
      target.x = moveParams[0] = path.locs[nextWPIndex].x;
      target.y = moveParams[1] = path.locs[nextWPIndex].y;
      nextWPIndex--;
      gob->set_action("move", moveParams);
    }
    else {
      // moving, not at waypoint
      counter = 0;
#ifdef RADAR
      if (veerAhead((int)distToTarget)) {
        // may or may not change target
        moveParams[0] = target.x;
        moveParams[1] = target.y;
        gob->set_action("move", moveParams);
        msg << "pre-veering\n";
      }
#endif

#ifdef MAGNETISM    
      cout << "MOVEFSM: Magnetized\n";
      if(getMoveVector()) {
        cout<<"MOVEFSM: MoveParams: "<<moveParams[0]<<","<<moveParams[1]<<"\n";
        gob->set_action("move",moveParams);
      }
#endif
    }

    break;

    
	} // switch (state)
  lastLocation = currentLocation;
 
  return FSM_RUNNING;
}

void MoveFSM::init(vector<sint4> p) 
{
  FSM::init(p);
 
  TerrainBase::Loc l;
  l.x = p[0];
  l.y = p[1];
  int pathLength;
 
  if (p.size() == 3) {
    // third parameter specifies how close to the target we must get
    precision = p[2];
    precision *= precision; // since we use distance squared
  }

  clearWPWorkers();
  Sorts::terrainModule->findPath(gob, l, path);
  pathLength = path.locs.size();
  msg << "initialized. Path " << *gob->sod.x << "," << *gob->sod.y << "->"
      << l.x << "," << l.y << endl;
  
  for (unsigned int i=0; i<pathLength; i++) {
    msg << "loc " << i << " " 
        << path.locs[i].x << ", "<< path.locs[i].y << endl;
  }

  if (pathLength > 0) {
    usingIWWP = true;
    imaginaryWorkerWaypoint = path.locs[(path.locs.size()/2)];
    Sorts::terrainModule->insertImaginaryWorker(imaginaryWorkerWaypoint);
  }
  
  nextWPIndex = path.locs.size()-1;
  moveParams.clear();
  if (path.locs.size() > 0) {
    if (path.locs.size() > 1
        and collision(path.locs[nextWPIndex].x, path.locs[nextWPIndex].y)) {
      nextWPIndex--;
      msg << "first waypoint is inside an object, skipping.\n";
    }
      
    moveParams.push_back(path.locs[nextWPIndex].x);
    moveParams.push_back(path.locs[nextWPIndex].y);
    nextWPIndex--;
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

void MoveFSM::initNoPath(vector<sint4> p) 
{
  FSM::init(p);
 
  TerrainBase::Loc l;
  l.x = p[0];
  l.y = p[1];

  clearWPWorkers();
  if (p.size() == 3) {
    // third parameter specifies how close to the target we must get
    precision = p[2];
    precision *= precision; // since we use distance squared
  }

  path.locs.clear();
  path.locs.push_back(l);
 
  usingIWWP = false;
  nextWPIndex = path.locs.size()-1;
  moveParams.clear();
  if (path.locs.size() > 0) {
    moveParams.push_back(path.locs[nextWPIndex].x);
    moveParams.push_back(path.locs[nextWPIndex].y);
    nextWPIndex--;
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
  clearWPWorkers();
}

void MoveFSM::clearWPWorkers() {
/*  if (nextWPIndex+1 < path.locs.size() and
      nextWPIndex >= 0 and
      target == path.locs[nextWPIndex+1]) {
    Sorts::terrainModule->removeImaginaryWorker(target);
  }
  for (int i = nextWPIndex; i >=0; i--) {
    Sorts::terrainModule->removeImaginaryWorker(path.locs[i]);
  }
  nextWPIndex = -1;
  */
  if (usingIWWP) {
    Sorts::terrainModule->removeImaginaryWorker(imaginaryWorkerWaypoint);
    usingIWWP = false;
  }
}

void MoveFSM::veerRight() {
  // if there is open space to the right,
  // make that the new target.
  // if that happens, increment nextWPIndex so the current waypoint
  // is moved to after we get to the right
  // do not allow multiple rightward moves! better to just return failure
  // so the FSM is reinitted with a new path

  //if (nextWPIndex+1 < path.locs.size() 
   //   || target != path.locs[nextWPIndex+1]) {
    // we've already veered, do nothing
   // msg << "already veered!\n";
    if (veerCount > (rand()%MAX_VEERCOUNT_DIFF + MIN_VEERCOUNT)) {
      msg << "already veered too much!\n";
      return;
    }
  //}
 
  double deltaX = target.x - *gob->sod.x;
  double deltaY = target.y - *gob->sod.y;
 
  double headingAngle = atan2(deltaY, deltaX); // -pi to pi (deltaX == 0 ok)
  if (lastRight) {
    headingAngle -= (PI/2.0);
  }
  else {
    headingAngle += (PI/2.0);
  }
 
  int width = VEERWIDTH + rand()%8;
  int newX = *gob->sod.x + (int)(width*cos(headingAngle));
  int newY = *gob->sod.y + (int)(width*sin(headingAngle));

  if (!collision(newX, newY)) {
    msg << "trying right.\n";
    if ((nextWPIndex+1 < path.locs.size()) and
        target == path.locs[nextWPIndex+1]) {
      nextWPIndex++;
    }
    msg << "veer " << *gob->sod.x << "," << *gob->sod.y << "->"
        << newX << "," << newY << " on the way to " 
        << target.x << "," << target.y << endl;
    target.x = newX;
    target.y = newY;
    veerCount++;
  }
  else {
    if (lastRight) {
      headingAngle += PI;
    }
    else {
      headingAngle -= PI;  
    }
    newX = *gob->sod.x + (int)(width*cos(headingAngle));
    newY = *gob->sod.y + (int)(width*sin(headingAngle));

    msg << "veering left (obstacles be damned).\n";
    if ((nextWPIndex+1 < path.locs.size()) and
        target == path.locs[nextWPIndex+1]) {
      nextWPIndex++;
    }
    msg << "veer " << *gob->sod.x << "," << *gob->sod.y << "->"
        << newX << "," << newY << " on the way to " 
        << target.x << "," << target.y << endl;
    target.x = newX;
    target.y = newY;
    veerCount++;
  }

#ifdef ALTERNATE_VEERING
    lastRight = not lastRight;
#endif
  return;
}

bool MoveFSM::veerAhead(int distToTargetSq) {
  // estimate where we will be next cycle
  // if there is something there, try the space to the right
  // make that the new target if it is clear
  // if that happens, increment nextWPIndex so the current waypoint
  // is moved to after we get to the right
  // do not allow multiple rightward moves! better to just return failure
  // so the FSM is reinitted with a new path

  //if (target != path.locs[nextWPIndex+1]) {
    // we've already veered, do nothing
    if (veerCount > (rand()%MAX_VEERCOUNT_DIFF + MIN_VEERCOUNT)) {
      msg << "already veered too much!\n";
      return false;
    }
  //}
  if (distToTargetSq <= RADAR_FORWARD_DIST_SQ) {
    msg << "not veering, target is too close\n";
    return false;
  }
 
  double deltaX = target.x - *gob->sod.x;
  double deltaY = target.y - *gob->sod.y;
 
  double headingAngle = atan2(deltaY, deltaX); // -pi to pi (deltaX == 0 ok)
  //headingAngle += (PI/2.0);

  int newX = *gob->sod.x + (int)(RADAR_FORWARD_DIST*cos(headingAngle));
  int newY = *gob->sod.y + (int)(RADAR_FORWARD_DIST*sin(headingAngle));

  if (dynamicCollision(newX, newY)) {
    msg << "veerAhead sees an obstacle!\n";
    if (lastRight) {
      headingAngle -= RADAR_VEER_ANGLE;
    }
    else {
      headingAngle += RADAR_VEER_ANGLE;
    }
    newX = *gob->sod.x + (int)(RADAR_ANGLE_DIST*cos(headingAngle));
    newY = *gob->sod.y + (int)(RADAR_ANGLE_DIST*sin(headingAngle));
    if (!collision(newX, newY)) {
      if ((nextWPIndex+1 < path.locs.size()) and 
           target == path.locs[nextWPIndex+1]) {
        nextWPIndex++;
      }
      target.x = newX;
      target.y = newY;
      msg << "veering ahead!\n";
      veerCount++;
#ifdef ALTERNATE_VEERING
      lastRight = not lastRight;
#endif
      return true;
    }
    else {
      msg << "new path is blocked, not veering.\n";
      return false;
    }
  }

  msg << "clear ahead, no veer\n";
  return false;
}

bool MoveFSM::collision(int x, int y) {
  list<GameObj*> collisions;
  
  Sorts::spatialDB->getCollisions(x, y, 6, NULL, collisions);
  msg << x << "," << y << " collides with " << collisions.size() 
       << " things.\n";
  
  for (list<GameObj*>::iterator it = collisions.begin();
       it != collisions.end();
       it++) {
    if ((*it) == gob) {
      msg << "gob collides w/self, ignoring\n";
    }
    else {
      msg << "collides with " << (*it)->bp_name() << endl;
      msg << "loc: " << *(*it)->sod.x << "," << *(*it)->sod.y << endl;
      msg << "radius " << *(*it)->sod.radius << endl; 
      return true;
    }
  }

  return false;
}

bool MoveFSM::dynamicCollision(int x, int y) {
  // return true if loc collides with a sheep or worker
  list<GameObj*> collisions;
  
  Sorts::spatialDB->getCollisions(x, y, 6, NULL, collisions);
  msg << x << "," << y << " collides with " << collisions.size() 
       << " things.\n";
  
  for (list<GameObj*>::iterator it = collisions.begin();
       it != collisions.end();
       it++) {
    if ((*it) == gob) {
      msg << "gob collides w/self, ignoring\n";
    }
    else if ((*it)->bp_name() == "worker" or
             (*it)->bp_name() == "sheep") {
      msg << "collides with " << (*it)->bp_name() << endl;
      msg << "loc: " << *(*it)->sod.x << "," << *(*it)->sod.y << endl;
      msg << "radius " << *(*it)->sod.radius << endl; 
      return true;
    }
  }

  return false;
}
// MAGNETISM CODE

// Returns the actual move vector of the object
// New move coordinates are placed into the moveParams structure
bool MoveFSM::getMoveVector()
{
 bool answer = false;
 /*// - Things don't change rapidly
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

 std::list<GameObj*> *neighbors = new list<GameObj*>;// = Sorts::spatialDB->getObjectsInRegion(*(gob->sod.x), *(gob->sod.y));
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
 float d = sqrt((float)((loc.x-x)*(loc.x-x)+(loc.y-y)*(loc.y-y)));
 x /= d;
 y /= d;

 // Add in the attraction vector
 float x1 = target.x - loc.x;
 float y1 = target.y - loc.y;
 d = sqrt((float)((loc.x-x1)*(loc.x-x1)+(loc.y-y1)*(loc.y-y1)));

 cout<<"MOVEFSM: Size: "<< neighbors->size()<<"\n";
 cout<<"MOVEFSM: Repulsion Vector: ("<<x<<","<<y<<")\n";
 cout<<"MOVEFSM: Attraction Vector: ("<<x1/d<<","<<y1/d<<")\n";

 x += x1/d;
 y += y1/d;
 cout<<"Combined Vector: ("<<x<<","<<y<<")\n";
 
 TerrainBase::Loc loc = getHeadingVector(static_cast<sint4>(x),static_cast<sint4>(y));
 
 moveParams[0] = loc.x;
 moveParams[1] = loc.y;
 */
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

