#ifndef MoveFSM_H
#define MoveFSM_H

#include "FSM.h"
#include "Sorts.h"
#include "Demo_SimpleTerrain.H"
#include "general.h"

class MoveFSM: public FSM {
 public:
  MoveFSM(GameObj*);
  ~MoveFSM();

  int update();
	void init(std::vector<sint4>);
  void initNoPath(std::vector<sint4>);

  bool getMoveVector(); //returns truee if there needs to be a change in direction, otherwise false
  TerrainBase::Loc getHeadingVector(sint4 , sint4);
  

  void stop();
  void panic();
  coordinate currentLocation;

 private:
	enum{IDLE,WARMUP,MOVING,ALREADY_THERE,UNREACHABLE,STUCK};

  void veerRight();
  bool veerAhead(int dtt);
  bool collision(int x, int y);
  bool dynamicCollision(int x, int y);
  bool isReachableFromBuilding(TerrainBase::Loc l);
	int state;
  int runTime;
  double heading;

  void clearWPWorkers(); 

  TerrainBase::Loc target;

  TerrainBase::Path path;
  int nextWPIndex;
  Vector<sint4> moveParams;
  int counter;
  int counter_max;

  int vec_count;
  int precision;
  bool lastRight;
  int veerCount;
  coordinate lastLocation;

  TerrainBase::Loc imaginaryWorkerWaypoint;
  bool usingIWWP;
};

#endif
