#ifndef BuildFSM_H
#define BuildFSM_H

#include <vector>

#include "FSM.h"
#include "MoveFSM.h"

#include "GameObj.H"
#include "Vector.H"

using namespace std;

class BuildFSM : public FSM {
public:

  BuildFSM(GameObj* gob);
  ~BuildFSM();

  int update();
  void init(vector<sint4>);

private:
  int loc_x, loc_y, buildFrame;
  int buildCycles;
  BuildingType type;

  // for calculating intersections
  Rectangle buildingBounds;

  enum BuildState { IDLE, MOVING, START_BUILD, BUILDING };
  bool justStarted;
  BuildState state, nextState;
  
  MoveFSM* moveFSM;
};

#endif
