#include "Sorts.h"
#include "PersistentMoveFSM.h"
#include "general.h"

#define msg cout << "PMFSM(" << (int)this << "): "

#define MAX_REPATHS 30
#define MAX_UNREACHABLE 2

PersistentMoveFSM::PersistentMoveFSM(GameObj* go) 
         : FSM(go) {
  name = OA_MOVE;
  moveFSM = NULL;
}

int PersistentMoveFSM::update() {
  int moveStatus;
  switch (state) {
    case IDLE:
      tempParams.clear();
      tempParams.push_back(targetLoc.x);
      tempParams.push_back(targetLoc.y);

      moveFSM->init(tempParams);
      moveStatus = moveFSM->update();
      state = MOVE;
      break;
    case MOVE:
      moveStatus = moveFSM->update();
      if (moveStatus == FSM_RUNNING) {
        // do nothing
      }
      else if (moveStatus == FSM_FAILURE) {
        if (squaredDistance(*gob->sod.x, *gob->sod.y, targetLoc.x, targetLoc.y)
            <= tolerance) {
          return FSM_SUCCESS;
        }
        repathCount++;
        if (repathCount >= MAX_REPATHS) {
          msg << "repath count exceeded " << repathCount << endl;
          return FSM_FAILURE;
        }
        msg << "repathing, count " << repathCount << endl;
        tempParams.push_back(targetLoc.x);
        tempParams.push_back(targetLoc.y);
        moveFSM->init(tempParams);
      }
      else if (moveStatus == FSM_UNREACHABLE) {
        unreachableCount++;
        msg << "unreachable count" << unreachableCount << endl;
        if (unreachableCount >= MAX_UNREACHABLE) {
          return FSM_FAILURE;
        }
        tempParams.push_back(targetLoc.x);
        tempParams.push_back(targetLoc.y);
        moveFSM->init(tempParams);
      }
      else if (moveStatus == FSM_SUCCESS) {
        return FSM_SUCCESS;
      }
      else {
        assert(false);
      }
      break;
      
  }
  
  return FSM_RUNNING;
}
         
          
void PersistentMoveFSM::init(vector<sint4> p) {
  FSM::init(p);
  assert(p.size() >= 2);
  targetLoc.x = p[0];
  targetLoc.y = p[1];
  if (p.size() == 3) {
    tolerance = p[2];
  }
  else {
    tolerance = 10; // default
  }
  msg << "initted, tolerance is " << tolerance << endl;
  tolerance *= tolerance; // square it
  moveFSM = new MoveFSM(gob);
  state = IDLE;
 
  repathCount = 0;
  unreachableCount = 0;
}


PersistentMoveFSM::~PersistentMoveFSM() {
  if (moveFSM != NULL) {
    delete moveFSM;
  }
}

