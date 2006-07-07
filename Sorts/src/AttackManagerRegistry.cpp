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
        i =  members.begin();
        i != members.end();
        i++)
    {
      allTargets.insert(*i);
    }
  }

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
      managers.erase(i);
      dbg << managers.size() << endl;
      return;
    }
  }
}
