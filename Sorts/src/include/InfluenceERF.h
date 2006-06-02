#ifndef InfluenceERF_H
#define InfluenceERF_H

#include "ERF.h"

#include "ScriptObj.H"

#define msg cout << "InfluenceERF.h: "

class InfluenceERF : public ERF {
public:
  InfluenceERF(int time_ticks, bool isGround) 
  : ticks(time_ticks)
  {
    if (isGround) {
      rangeType = "max_ground_range";
    }
    else {
      rangeType = "max_air_range";
    }
  }

  ~InfluenceERF() { }

  double operator()(GameObj* gob) {
    int move_dist;
    if (gob->has_attr("max_speed")) {
      move_dist = gob->get_int("max_speed") * ticks;
    }
    else {
      move_dist = 0;
    }
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
  string rangeType;
};

#endif
