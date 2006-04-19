#ifndef SoarGameObject_H
#define SoarGameObject_H

#include"FSM.h"
#include "MoveFSM.h"
#include "Rectangle.h"

#include<stack>
#include<map>

using namespace std;

class SoarGameGroup;
class Sorts;

class SoarGameObject{
 public:
	SoarGameObject 
  ( GameObj*       _gob,
    const Sorts*   _sorts,
    bool           _friendly, 
    bool           _world, 
    int            _id );

	~SoarGameObject();

	void identifyBehaviors();

	void registerBehavior(FSM *);
	void removeBehavior(ObjectActionType);

	//template<class T>
	void issueCommand(ObjectActionType name, Vector<sint4> p);
	void update();

	void setGroup(SoarGameGroup *g);
	SoarGameGroup *getGroup();
	int getStatus();

  int  getOwner()     { return *gob->sod.owner; }
 	bool isFriendly()   { return friendly; }
  bool isWorld()      { return world; }
  int getID()         { return id; }

  Rectangle getBoundingBox();

  GameObj *gob;

private:
  const Sorts* sorts;

	map<ObjectActionType, FSM *> behaviors;
	stack<FSM *> memory;

  ObjectActionType currentCommand;
	SoarGameGroup* group;
	bool friendly;
  bool world;
  int id;

  int frameOfLastUpdate;

  int status;

};

#define OBJ_IDLE 0
#define OBJ_RUNNING 1
#define OBJ_SUCCESS 2
#define OBJ_FAILURE 3
#define OBJ_STUCK 4

#endif
