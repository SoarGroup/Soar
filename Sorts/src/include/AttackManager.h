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
  int size();

  set<SoarGameObject*>* getTargets() { return &targetSet; }
  /***
   * Method: findCentroid
   * --------------------
   * Iterates over the list of team members
   * and calculates the centroid of the group.
   */
  Vec2d findCentroid();

private: // functions
  void  selectTarget();
  int   updateTargetList();
  void  reprioritize();
  void  attackArcPos(GameObj* atk, GameObj* tgt, int layer, list<Vec2d>& positions);

  /***
   * Method: assignTarget
   * --------------------
   *  Given an attacking unit and a target unit, this method
   *  checks that the target being assigned does exist, adds the
   *  attacking unit to the set of units attacking the target object,
   *  and sets the attacking unit's target to be the target unit.
   */
  void  assignTarget(AttackFSM* fsm, SoarGameObject* target);

  /***
   * Method unassignTarget
   * ---------------------
   * Removes an attacking unit's target.
   */
  void  unassignTarget(AttackFSM* fsm);

  /***
   * Method: unassignAll
   * -------------------
   * Given a target, this frees all units that had been attacking it
   * to do something else.
   */
  void  unassignAll(SoarGameObject* target);

  /***
   * Method: findTarget
   * ------------------
   * Given an attack unit, this method attacks nearby enemy units,
   * making its target selection based on enemy health and distance.
   */
  bool  findTarget(AttackFSM* fsm);

  /***
   * Method: gather
   * --------------
   * Collects all attack units at a point.
   */
  void  gather(AttackFSM* fsm);

  /***
   * Method: attackLinePos
   * ---------------------
   * Determines if the attacking group is in a line, and if so we
   * find corresponding positions across that line to attack them from.
   */
  bool attackLinePos(int range, list<Vec2d> &positions);

  /***
   * Method: getSlope()
   * ---------------------
   * Self-explanatory. Checks to see if input list has a linear
   * correlation higher than 0.8.
   */
  double getSlope();

  /***
   * Method: findLineMidpoint
   * -------------------
   * This method finds the side of the
   * attackers we should be on, and returns the
   * location on a line parallel to the attackers
   * that is across from the primary enemy target.
   */
  Vec2d getLineMidpoint(double slope, int range);

private: // variables
  list<AttackFSM*> team;
  set<SoarGameObject*> targetSet;
  map<SoarGameObject*, AttackTargetInfo> targets;
  vector<SoarGameObject*> sortedTargets;
  vector<SoarGameObject*> bases;
  vector<SoarGameObject*> sortedBases;
  
  set<int> idSet;

  int numNewAttackers;

  int reprioritizeCounter;
};

#endif
