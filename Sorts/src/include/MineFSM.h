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
#ifndef MineFSM_H
#define MineFSM_H

#include "FSM.h"
#include "general.h"
#include "OrtsInterface.h"
#include "MineManager.h"
#include "MoveFSM.h"
struct MiningRoute;

class MineFSM : public FSM {
public:
	MineFSM(GameObj*);
  ~MineFSM();

 	int update();
  void stop();
	void init(vector<sint4> p);
  
  // called by MineManager if a mineral or cc disappears
  void abortMining();
  void panic();
  void setSoarGameObject(SoarGameObject* _sgo) { sgo = _sgo; }
  SoarGameObject* getSoarGameObject() { return sgo; }
private:
  enum MineState { IDLE, MINING, MOVING_TO_MINERAL, MOVING_TO_DROPOFF,
                   SEND_MOVE_TO_MINE_COMMAND, PANIC_START, PANIC, FAIL };
  MineState state;
  MiningRoute* route;
  SoarGameObject* sgo;
  MoveFSM* moveFSM;
  int precision, timer;
  bool timed;
  int giveUpThreshold;
  int panicCount;
  coordinate dropoffLoc;
  void calcDropoffLoc();
};

#endif
