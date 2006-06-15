#ifndef PersistentMoveFSM_H
#define PersistentMoveFSM_H

#include "FSM.h"
#include "general.h"
#include "OrtsInterface.h"
#include "MoveFSM.h"

class PersistentMoveFSM : public FSM {
public:
	PersistentMoveFSM(GameObj*);
  ~PersistentMoveFSM();

 	int update();
	void init(vector<sint4> p);
  
private:
  enum PMState {IDLE, MOVE, PANIC};
  int unreachableCount, repathCount, tolerance;
  coordinate targetLoc;
  PMState state;
  MoveFSM* moveFSM;
  vector<sint4> tempParams;
  int panicUpdateCount;
};

#endif
