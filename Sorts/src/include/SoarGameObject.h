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
	int getStatus();

  int  getOwner()     { return *gob->sod.owner; }
 	bool isFriendly()   { return friendly; }
  bool isWorld()      { return world; }
  bool getUpdateRequired() { return updateRequired; }
  //void setUpdateRequired() { updateRequired = true; }

  GameObj *gob;

  private:
	map<SoarActionType, FSM *> behaviors;
	stack<FSM *> memory;

  SoarActionType currentCommand;
	SoarGameGroup* group;
	bool friendly;
  bool world;

  // this is true if the object must be updated next cycle,
  // whether it changed in the world or not
  // cleared every cycle (set it in the FSM),
  // also kept in a queue in the OrtsInterface
  bool updateRequired;

  int status;
};

#define OBJ_IDLE 0
#define OBJ_RUNNING 1
#define OBJ_SUCCESS 2
#define OBJ_FAILURE 3

#endif
