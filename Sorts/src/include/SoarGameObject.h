#ifndef SoarGameObject_H
#define SoarGameObject_H

#include"FSM.h"
#include "MoveFSM.h"

#include<stack>
#include<map>

using namespace std;

//How do I transition from one FSM to another?

class SoarGameGroup;

class SoarGameObject{
 public:
	SoarGameObject(GameObj* _gob, bool _friendly, bool _world);
	~SoarGameObject();

	void identifyBehaviors();

	void registerBehavior(FSM *);
	void removeBehavior(SoarActionType);

	//template<class T>
	void issueCommand(SoarActionType name, Vector<sint4> p);
	void update();

	void setGroup(SoarGameGroup *g);
	SoarGameGroup *getGroup();
	SoarActionType getState();

  int  getOwner()     { return *gob->sod.owner; }
 	bool isFriendly()   { return friendly; }
  bool isWorld()      { return world; }

  GameObj *gob;

  private:
	map<SoarActionType, FSM *> behaviors;
	stack<FSM *> memory;

	SoarActionType state;
	SoarGameGroup* group;
	bool friendly;
  bool world;
};

#endif
