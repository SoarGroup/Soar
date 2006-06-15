#ifndef TrainFSM_H
#define TrainFSM_H

#include <vector>

#include "FSM.h"
#include "MoveFSM.h"

#include "GameObj.H"
#include "Vector.H"

using namespace std;

class TrainFSM : public FSM {
public:

  TrainFSM(GameObj* gob);
  ~TrainFSM();

  int update();
  void init(vector<sint4>);

private:
  int num, numTrained;
  TrainingType type;
  int trainCycles;
  int cost;
  string command;
  int bufferAvailable;

  enum TrainState { IDLE, TRAINING };
  TrainState state, nextState;
  Vector<sint4> dummy;
};

#endif
