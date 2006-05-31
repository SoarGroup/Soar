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
	void issueCommand(ObjectActionType name, Vector<sint4> p);
	void update();

	void setPerceptualGroup(PerceptualGroup *g);
	PerceptualGroup *getPerceptualGroup();
	//void setInternalGroup(InternalGroup *g);
	//InternalGroup *getInternalGroup();
	int getStatus();

  int  getOwner()     { return *gob->sod.owner; }
 	bool isFriendly()   { return friendly; }
  bool isWorld()      { return world; }
  int getID()         { return id; }
  coordinate getLocation();

  Rectangle getBoundingBox();

  GameObj *gob;

private:

  sint4 sat_loc;

	map<ObjectActionType, FSM *> behaviors;
	stack<FSM *> memory;

  ObjectActionType currentCommand;
	PerceptualGroup* pGroup;
	//InternalGroup* iGroup;
	bool friendly;
  bool world;
  int id;

  int frameOfLastUpdate;

  int status;

  bool friendlyWorker;
  // these are only updated for friendly workers!
  coordinate lastLocation;
  int motionlessFrames;

};

#define OBJ_IDLE 0
#define OBJ_RUNNING 1
#define OBJ_SUCCESS 2
#define OBJ_FAILURE 3
#define OBJ_STUCK 4

#endif
