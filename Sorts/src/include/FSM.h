#ifndef FSM_H
#define FSM_H

#include<map>
#include<string>
#include<list>
#include<vector>

//#include "OrtsInterface.h"
#include "general.h"
#include "GameObj.H"
#include "SoarAction.h"

class OrtsInterface;
class GroupManager;

class FSM{
 public:
	FSM(OrtsInterface*, GroupManager*, GameObj*);
	virtual ~FSM();

	virtual int update()=0;

	virtual GameObj *getGameObject(){return gob;}
	virtual void init(std::vector<sint4>);
  virtual SoarActionType getName();

protected:
	SoarActionType name;
	GameObj *gob;
	Vector<sint4> params;
  OrtsInterface* ORTSIO;
  GroupManager* groupMan;
};


#define FSM_RUNNING 0
#define FSM_SUCCESS 1
#define FSM_FAILURE 2

#endif
