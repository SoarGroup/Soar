#ifndef MoveFSM_H
#define MoveFSM_H

#include "FSM.h"
#include "Sorts.h"
#include "Satellite.h"
#include "Demo_SimpleTerrain.H"

class MoveFSM: public FSM {
 public:
  MoveFSM(GameObj*);
  ~MoveFSM();

  int update();
	void init(std::vector<sint4>);

  bool getMoveVector(); //returns truee if there needs to be a change in direction, otherwise false
  TerrainBase::Loc getHeadingVector(sint4 , sint4);
  

  void stop();

 private:
	enum{IDLE,WARMUP,MOVING,ALREADY_THERE, TURNING};

  void veerRight();
  bool collision(int x, int y);
	int state;
  int runTime;
  double heading;

  TerrainBase::Loc loc;
  TerrainBase::Loc target;

  TerrainBase::Path path;
  int stagesLeft;
  Vector<sint4> moveParams;
  int counter;
  int counter_max;

  int vec_count;
  int precision;
};

#endif
