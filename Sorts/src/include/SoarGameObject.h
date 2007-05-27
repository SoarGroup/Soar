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
#ifndef SoarGameObject_H
#define SoarGameObject_H

#include"FSM.h"
#include "Rectangle.h"

#include<stack>
#include<map>

using namespace std;

class PerceptualGroup;
class InternalGroup;

class SoarGameObject{
 public:
	SoarGameObject 
  ( GameObj*       _gob,
    bool           _friendly, 
    bool           _world, 
    int            _id );

	~SoarGameObject();

	void identifyBehaviors();

	void registerBehavior(FSM *);
	void removeBehavior(ObjectActionType);

	//template<class T>
	bool assignAction(ObjectActionType name, Vector<sint4> p);
  void endCommand();
	void update();

	void setPerceptualGroup(PerceptualGroup *g);
	PerceptualGroup *getPerceptualGroup();
	//void setInternalGroup(InternalGroup *g);
	//InternalGroup *getInternalGroup();
	int getStatus();
  void removeFromGame();

  int  getOwner()     { return *gob->sod.owner; }
 	bool isFriendly()   { return friendly; }
  bool isWorld()      { return world; }
  int getID()         { return id; }
  coordinate getLocation();

  Rectangle getBoundingBox();

  GameObj* getGob()   { return gob; }

  int getX();
  int getY();
  int getRadius();

  void setLastAttacked(int id) { lastAttackedId = id; }
  int  getLastAttacked()       { return lastAttackedId; }
  string getName() { return name; }

  void printFeatures();

  bool isMobile() { return mobile; }
  bool isRectangle() { return rectangle; }
  int getWidth() { return width; }
  int getHeight() { return height; }

  void setLastAttackOpportunistic(bool b) { lastAttackOpportunistic = b; }
private:
  GameObj* gob;

  sint4 sat_loc;

  bool mobile;
  bool rectangle;
  int width, height;
  
	map<ObjectActionType, FSM *> behaviors;
  FSM* assignedBehavior;
  list<FSM*> defaultBehaviors;

  ObjectActionType currentCommand;
	PerceptualGroup* pGroup;
	//InternalGroup* iGroup;
	bool friendly;
  bool world;
  int id;

  int frameOfLastUpdate;

  int status;
  string name;
  int owner;

  bool friendlyWorker;
  // these are only updated for friendly workers!
  coordinate lastLocation;
  int motionlessFrames;

  int lastAttackedId;
  bool lastAttackOpportunistic;

  bool stopped;
};

#define OBJ_IDLE 0
#define OBJ_RUNNING 1
#define OBJ_SUCCESS 2
#define OBJ_FAILURE 3
#define OBJ_STUCK 4

#endif
