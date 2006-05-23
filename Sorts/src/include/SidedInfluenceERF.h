#ifndef SidedInfluenceERF_H
#define SidedInfluenceERF_H

#include "ERF.h"

#include "ScriptObj.H"

#define msg cout << "SidedInfluenceERF.h: "

class SidedInfluenceERF : public ERF {
public:
  SidedInfluenceERF(int time_ticks, int _side, bool isGround) 
  : ticks(time_ticks), side(_side)
  {
    if (isGround) {
      rangeType = "max_ground_range";
    }
    else {
      rangeType = "max_air_range";
    }
  }

  ~SidedInfluenceERF() { }

  double operator()(GameObj* gob) {
    if (gob->get_int("owner") != side) {
      return -1.0;
    }
    int move_dist = gob->get_int("max_speed") * ticks;
    ScriptObj* weapon = gob->component("weapon");
    if (weapon == NULL) {
      msg << "ER(MD): " << move_dist << endl;
      return move_dist;
    }
    else {
      msg << "ER(MD+A): " << move_dist + weapon->get_int(rangeType) << endl;
      return move_dist + weapon->get_int(rangeType);
    }
  }

  double maxRadius() {
    // this is for marines
    return ticks * 3 + 16 * 6;
  }

private:
  int ticks;
  int side;
  string rangeType;
};

#endif
