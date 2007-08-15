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
#include "AttackManagerRegistry.h"

#include "SoarGameObject.h"
#include "PerceptualGroup.h"

#define CLASS_TOKEN "AMREG"
#define DEBUG_OUTPUT false 
#include "OutputDefinitionsUnique.h"

int AttackManagerRegistry::assignManager
( const list<PerceptualGroup*>& targets, int numNewAttackers )
{
  set<SoarGameObject*> allTargets;
  for(list<PerceptualGroup*>::const_iterator
      i =  targets.begin();
      i != targets.end();
      i++)
  {
    list<SoarGameObject*> members;
    (*i)->getMembers(members);
    for(list<SoarGameObject*>::iterator
        j =  members.begin();
        j != members.end();
        j++)
    {
      allTargets.insert(*j);
    }
  }
  cout << "total number of targets: " << allTargets.size() << endl;
  for(list<ManagerRecord>::iterator
      i =  managers.begin();
      i != managers.end();
      i++)
  {
    // this check can take a while
    if (*(*i).manager->getTargets() == allTargets) { 
      (*i).manager->addNewAttackers(numNewAttackers);
      return (*i).id;
    }
  }

  // create a new one
  dbg << managers.size() << endl;
  ManagerRecord newRecord;
  AttackManager* m = new AttackManager(allTargets);
  newRecord.id = uid++;
  newRecord.manager = m;
  m->addNewAttackers(numNewAttackers);
  managers.push_back(newRecord);
  return newRecord.id;
}

AttackManager* AttackManagerRegistry::getManager(int id) {
  for(list<ManagerRecord>::iterator
      i =  managers.begin();
      i != managers.end();
      i++)
  {
    if ((*i).id == id) {
      return (*i).manager;
    }
  }
  return NULL;
}

void AttackManagerRegistry::removeManager(AttackManager* m) {
  for(list<ManagerRecord>::iterator
      i =  managers.begin();
      i != managers.end();
      i++)
  {
    if ((*i).manager == m) {
      cout << "THIS IS " << managers.size() << endl;
      managers.erase(i);
      dbg << managers.size() << endl;
      return;
    }
  }
}

int AttackManagerRegistry::numAttackUnits() {
  int total = 0; //keeps track of the total number of units
  for(list<ManagerRecord>::iterator i = managers.begin(); i!= managers.end();i++)
  {
    total += (*i).manager->size();
  }
  return total;
}

int AttackManagerRegistry::numGroups(){
  return managers.size();
}

Vec2d AttackManagerRegistry::mainCentroid(){
  int xSum, ySum, units, total;
  xSum = ySum = units = total = 0;
  for(list<ManagerRecord>::iterator i = managers.begin(); i!= managers.end();i++)
  {
    cout << "inside" << endl;
    units = (*i).manager->size();
    cout << "Check size: " << units << endl;
    total += units;
    cout << "and" << endl;
    Vec2d centroid = (*i).manager->findCentroid();
    cout << "we" << endl;
    xSum += (int) centroid.getX() *  units;
    ySum += (int) centroid.getY() *  units;
  }
  cout << "out" << endl;
  return Vec2d(xSum/total, ySum/total);
}

