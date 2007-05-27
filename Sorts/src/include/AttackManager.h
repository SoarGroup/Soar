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
#ifndef AttackManager_H
#define AttackManager_H

#include <vector>
#include <set>
#include <map>

#include "Point.h"
#include "Circle.h"
#include "SoarGameObject.h"
#include "AttackFSM.h"
#include "AttackTargetInfo.h"

#include "Vector.H"
#include "GameObj.H"

#include "SortsCanvas.h"

using namespace std;

class AttackManager {
public:
  AttackManager(const set<SoarGameObject*>& _targets);
  ~AttackManager();

  void addNewAttackers(int num);
  void decNewAttackers();
  void registerFSM(AttackFSM* fsm);
  void unregisterFSM(AttackFSM* fsm);
  int direct(AttackFSM* fsm);

  set<SoarGameObject*>* getTargets() { return &targetSet; }

private: // functions
  void  selectTarget();
  int   updateTargetList();
  void  reprioritize();
  void  attackArcPos(GameObj* atk, GameObj* tgt, int layer, list<Vec2d>& positions);

  void  assignTarget(AttackFSM* fsm, SoarGameObject* target);
  void  unassignTarget(AttackFSM* fsm);
  void  unassignAll(SoarGameObject* target);
  bool  findTarget(AttackFSM* fsm);

private: // variables
  list<AttackFSM*> team;
  set<SoarGameObject*> targetSet;
  map<SoarGameObject*, AttackTargetInfo> targets;
  vector<SoarGameObject*> sortedTargets;
  
  set<int> idSet;

  int numNewAttackers;

  int reprioritizeCounter;
};

#endif
