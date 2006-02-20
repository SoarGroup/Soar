#ifndef FSM_H
#define FSM_H

#include<map>
#include<string>
#include<list>
#include<vector>

#include "general.h"
#include "GameObj.H"
#include "SoarAction.h"

class FSM{
 public:
	FSM();
	virtual ~FSM();

	virtual int update(bool& updateRequiredNextCycle)=0;

	virtual void setGameObject(GameObj *g){gob = g;}
	virtual GameObj *getGameObject(){return gob;}
	virtual void init(std::vector<sint4>);

 //private:
	SoarActionType name;
	GameObj *gob;
	Vector<sint4> params;
};


#define FSM_RUNNING 0
#define FSM_SUCCESS 1
#define FSM_FAILURE 2

#endif
