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

  enum BUILDING_TYPE { CONTROL_CENTER, BARRACKS, FACTORY }; // just for game 3

  BuildFSM(GameObj* gob);
  ~BuildFSM();

  int update();
  void init(vector<sint4>);

private:
  int loc_x, loc_y;
  BUILDING_TYPE type;

  // for calculating intersections
  Rectangle buildingBounds;

  bool building, isMoving, finished;

  MoveFSM* moveFSM;
};

#endif
