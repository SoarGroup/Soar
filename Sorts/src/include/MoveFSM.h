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

 private:
	enum{IDLE,WARMUP,MOVING,ALREADY_THERE};

	int state;
  int runTime;

  TerrainBase::Loc loc;

  TerrainBase::Path path;
  int stagesLeft;
  Vector<sint4> moveParams;
  int counter;

  int vec_count;
};

#endif
