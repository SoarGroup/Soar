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
#ifndef AttackTargetInfo_H_
#define AttackTargetInfo_H_
#include <assert.h>

#include "GameObj.H"
#include "ScriptObj.H"

#include "SoarGameObject.h"
#include "AttackFSM.h"
#include "Circle.h"

#include <map>

class AttackTargetInfo {
public:
  AttackTargetInfo() { assert(false); }
  AttackTargetInfo(SoarGameObject* _target);

  void assignAttacker(AttackFSM* fsm);
  void unassignAttacker(AttackFSM* fsm);

  set<AttackFSM*>::const_iterator attackers_begin() { 
    return attackers.begin();
  }

  set<AttackFSM*>::const_iterator attackers_end() {
    return attackers.end();
  }

  void unassignAll() { attackers.clear(); }

  double avgAttackerDistance();

  bool isSaturated() {
    return volleyDamage > gob->get_int("hp");
  }

private:
  int volleyDamage;
  SoarGameObject* target;
  GameObj*        gob;
  set<AttackFSM*> attackers;
  list<Circle>    attackPos;

  map<AttackFSM*, int> avgDmg;
};

#endif
