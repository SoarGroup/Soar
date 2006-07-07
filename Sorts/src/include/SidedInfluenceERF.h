/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
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
  int side;
  string rangeType;
};

#endif
