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
	SoarGameObject(GameObj *, int, bool);
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

  	int getOwner();
  	bool getFriendly();

   	GameObj *gob;

  private:
	map<SoarActionType, FSM *> behaviors;
	stack<FSM *> memory;

	SoarActionType state;
	SoarGameGroup* group;
	bool friendly;
	int owner;
};

#endif
