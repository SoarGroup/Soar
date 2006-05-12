#ifndef MoveFSM_H
#define MoveFSM_H

#include "FSM.h"
#include "Sorts.h"
#include "Satellite.h"

class MoveFSM: public FSM {
 public:
  MoveFSM(GameObj*);
  ~MoveFSM();

  int update();
	void init(std::vector<sint4>);

  bool getMoveVector(); //returns truee if there needs to be a change in direction, otherwise false
  double getHeading(sint4, sint4);
  TerrainBase::Loc getHeadingVector(sint4 , sint4);
  

 private:
	enum{IDLE,WARMUP,MOVING,ALREADY_THERE, TURNING};

	int state;
  int runTime;
  sint4 sat_loc;
  double heading;

  TerrainBase::Loc loc;
  TerrainBase::Loc target;

  TerrainBase::Path path;
  int stagesLeft;
  Vector<sint4> moveParams;
  int counter;

  int vec_count;
  int precision;
};

#endif
