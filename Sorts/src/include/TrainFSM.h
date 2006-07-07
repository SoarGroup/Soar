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
