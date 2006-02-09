#ifndef SoarGameObject_H
#define SoarGameObject_H

#include"FSM.h"
#include "MoveFSM.h"

#include<stack>
#include<vector>

//How do I transition from one FSM to another?

class SoarGameGroup;

class SoarGameObject{
 public:
	SoarGameObject(GameObj *);
	~SoarGameObject();

	void registerBehavior(FSM *);
	void removeBehavior(SoarActionType);

	void issueCommand(SoarActionType name, Vector<sint4> p);
	void update();

	void setGroup(SoarGameGroup *g);
	SoarGameGroup *getGroup();
	SoarActionType getState();


	GameObj *gob;
 private:
	std::list<FSM *> behaviors;
	std::stack<FSM *> memory;

	SoarActionType state;
  	SoarGameGroup* group;

};

#endif
