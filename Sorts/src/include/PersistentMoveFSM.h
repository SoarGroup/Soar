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
