#ifndef MineFSM_H
#define MineFSM_H

#include "FSM.h"

class MineFSM : public FSM {
public:
  MineFSM() {}

  bool update() {
    switch (state) {
      case IDLE:
        gob->set_action("mine", params);
        state = MINING;
        return true;

      case MINING:
        if (gob->get_int("is_mobile") == 0) {
          // the guy is still mining, don't do anything
          return true;
        }
        else {
          // finished mining
          return false;
        }
    }
  }


private:
  enum MineState { IDLE, MINING };
  MineState state;
};

#endif
