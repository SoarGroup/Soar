#include "TrainFSM.h"

#define CLASS_TOKEN "TRAINFSM"
#define DEBUG_OUTPUT false
#include "OutputDefinitions.h"

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
  assert(params.size() == 3);
  type = (TrainingType) params[0];
  num = params[1];
  bufferAvailable = params[2];

  state = IDLE;
  numTrained = 0;
  switch (type) {
    case WORKER:
      cost = 50;
      assert(gob->bp_name() == "controlCenter");
      command = "train_worker";
      break;
    case MARINE:
      cost = 50;
      assert(gob->bp_name() == "barracks");
      command = "train_marine";
      break;
    case TANK:
      cost = 200;
      assert(gob->bp_name() == "factory");
      command = "build_tank";
      break;
  }
  msg << "initted\n";
}

int TrainFSM::update() {  
  int active;
  if (gob->is_pending_action()) {
    msg << "action has not taken effect!\n";
    return FSM_RUNNING;
  }
  switch (state) {
    case IDLE: {
      msg << "training beginning.\n";
      int effectiveBuffer = Sorts::gameActionManager->getMineralBuffer()
                            - bufferAvailable;
      if (effectiveBuffer < 0) {
        msg << "did we buffer enough minerals?\n";
        effectiveBuffer = 0;
      }
      
      if ((Sorts::OrtsIO->getCurrentMinerals() 
                    - effectiveBuffer) < cost) {
        msg << "can't afford this unit now.\n";
        nextState = IDLE;
      }
      else {
        msg << "training beginning.\n";
        gob->set_action(command, dummy);
        int bufferDeduction;
        if (bufferAvailable >= cost) {
          bufferDeduction = cost;
        }
        else {
          bufferDeduction = bufferAvailable;
        }

        if (bufferDeduction) {
          int oldSize = Sorts::gameActionManager->getMineralBuffer();
          if (oldSize - bufferDeduction >= 0) {
            Sorts::gameActionManager->setMineralBuffer(oldSize 
                                                      - bufferDeduction);
          }
          else {
            Sorts::gameActionManager->setMineralBuffer(0);
          }
        }
        bufferAvailable -= bufferDeduction;
        trainCycles = 0;
        nextState = TRAINING;
      }
      break;
    }
    case TRAINING: {
      trainCycles++;
      active = gob->get_int("active");
      if (active == 0) {
        if (trainCycles < MAX_WARMUP_TIME) {
          msg << "giving the action a chance to stick\n";
        }
        else if (trainCycles < MIN_BUILD_TIME) {
          msg << "done too quickly, restarting.\n";
          // TODO: fix the buffer!
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
  }
  
  state = nextState;
  
  return FSM_RUNNING;
} 

