#ifndef MoveFSM_H
#define MoveFSM_H

#include "FSM.h"
#include "Sorts.h"

class MoveFSM: public FSM {
 public:
  MoveFSM(const Sorts* sorts,  GameObj*);

  int update();
	void init(std::vector<sint4>);

 private:
	enum{IDLE,WARMUP,MOVING};

	int state;
  int runTime;

//  static SPathFinder *pather;
 
  TerrainBase::Path path;
  int stagesLeft;
  Vector<sint4> moveParams;
  int counter;
};

#endif
