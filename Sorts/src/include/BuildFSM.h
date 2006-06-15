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
  void setSoarGameObject(SoarGameObject* _sgo) { sgo = _sgo; }

private:
  void deductFromBuffer();
  int loc_x, loc_y, buildFrame;
  int buildCycles;
  int cost;
  BuildingType type;
  SoarGameObject* sgo;
  int bufferAvailable;
  void setBuildingInfo(BuildingType type, int centerX, int centerY);

  // for calculating intersections
  Rectangle buildingBounds;

  enum BuildState { IDLE, MOVING, START_BUILD, BUILDING };
  bool justStarted;
  BuildState state, nextState;
  
  MoveFSM* moveFSM;
};

#endif
