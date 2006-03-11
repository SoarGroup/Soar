#ifndef SoarGameObject_H
#define SoarGameObject_H

#include"FSM.h"
#include "MoveFSM.h"
#include "Rectangle.h"

#include<stack>
#include<map>

using namespace std;


class SoarGameGroup;
class OrtsInterface;
class GroupManager;

class SoarGameObject{
 public:
	SoarGameObject(OrtsInterface* _ORTSIO, GroupManager* _groupMan,
                 GameObj* _gob, 
                 bool _friendly, bool _world, int _id);
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
  int getID()         { return id; }

  Rectangle getBoundingBox();

  GameObj *gob;

private:
  OrtsInterface* ORTSIO;
	map<SoarActionType, FSM *> behaviors;
	stack<FSM *> memory;

  SoarActionType currentCommand;
	SoarGameGroup* group;
  GroupManager* groupMan;
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

#endif
