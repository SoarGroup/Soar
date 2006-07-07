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
#ifndef AttackFSM_H
#define AttackFSM_H

#include "Vec2d.h"
#include "FSM.h"
#include "MoveFSM.h"
#include "SoarGameObject.h"

class AttackManager;

class AttackFSM : public FSM {

public:
	AttackFSM(SoarGameObject* sgob);
	~AttackFSM();

  void init(vector<sint4> p);
	int update();

// these are all commands from the attack manager
  void attack(SoarGameObject* t);
  bool isFiring();

  int move(int x, int y, bool b);
  bool isMoving() { return moving; }
  Vec2d getDestination() { return dest; }
  void stopMoving();

  // AttackManager is about to be deallocated, 
  void disown(int lastStatus);

  void stop();

  int getAvgDamage();

  SoarGameObject* getTarget() { return target; }
  void setTarget(SoarGameObject* tar) { target = tar; }
  SoarGameObject* getSGO() { return sgob; }

  void setReassign(bool b) { reassign = b; }
  int failCount;
  bool waitingForCatchup;

private:
  bool reassign;
  SoarGameObject* target;
  bool panic;
  SoarGameObject* sgob;
  AttackManager* manager;
  int pendingManagerID;
  ScriptObj* weapon;
  MoveFSM* moveFSM;
  Vec2d dest;
  Vector<sint4> attackParams;
  bool moving;
  int disownedStatus;
  
  bool firstMove;
  int moveFails;
};


#endif
