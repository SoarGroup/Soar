#include "TrainFSM.h"

#define msg cout << "TRAINFSM: "

#define MAX_WARMUP_TIME 5
#define MIN_BUILD_TIME 10

TrainFSM::TrainFSM(GameObj* _gob) 
: FSM(_gob)
{ 
  name = OA_TRAIN;  
  state = IDLE;
  num = 0;
  numTrained = 0;
}

TrainFSM::~TrainFSM() {
}

void TrainFSM::init(vector<sint4> params) {
  FSM::init(params);
  assert(params.size() == 2);
  type = (TrainingType) params[0];
  num = params[1];
  state = IDLE;
  numTrained = 0;
  msg << "initted\n";
}

int TrainFSM::update() {  
  int active;
  if (gob->is_pending_action()) {
    msg << "action has not taken effect!\n";
    return FSM_RUNNING;
  }
  switch (state) {
    case IDLE:
      msg << "training beginning.\n";
      switch (type) {
        case WORKER:
          assert(gob->bp_name() == "controlCenter");
          gob->set_action("train_worker", dummy);
          break;
        case MARINE:
          assert(gob->bp_name() == "barracks");
          gob->set_action("train_marine", dummy);
          break;
        case TANK:
          assert(gob->bp_name() == "factory");
          gob->set_action("build_tank", dummy);
          break;
      }
      trainCycles = 0;
      nextState = TRAINING;
      break;
    case TRAINING:
      trainCycles++;
      active = gob->get_int("active");
      if (active == 0) {
        if (trainCycles < MAX_WARMUP_TIME) {
          msg << "giving the action a chance to stick\n";
        }
        else if (trainCycles < MIN_BUILD_TIME) {
          msg << "done too quickly, restarting.\n";
          nextState = IDLE;
        }
        else {
          msg << "built one unit.\n";
          numTrained++;
          if (numTrained == num) {
            return FSM_SUCCESS;
          }
          else {
            nextState = IDLE;
          }
        }
      }
      break;
  }
  
  state = nextState;
  
  return FSM_RUNNING;
} 

