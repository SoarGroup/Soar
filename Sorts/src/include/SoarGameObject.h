#ifndef _FSM_H_
#define _FSM_H_
#include"FSM.h"
#endif

#include<stack>
#include<vector>

//How do I transition from one FSM to another?

class SoarGameGroup;

class SoarGameObject{
 public:
	SoarGameObject(GameObj *);
	~SoarGameObject();

	void registerBehavior(FSM *);
	void removeBehavior(std::string cmd);

	void issueCommand(std::string name, Vector<sint4> p);
	void update();

	void setGroup(SoarGameGroup *g);
	SoarGameGroup *getGroup();

 private:
	std::list<FSM *> behaviors;
	std::stack<FSM *> memory;

	GameObj *gob;
  	SoarGameGroup* group;

};
