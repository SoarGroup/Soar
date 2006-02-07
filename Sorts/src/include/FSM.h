#ifndef FSM_H_
#define FSM_H_

#include<map>
#include<string>
#include<list>
#include<vector>

#include "Global.H"
#include "Game.H"
#include "ServerObjData.H"
#include "Options.H"


class FSM{
 public:
	FSM(){}
	virtual ~FSM(){}

	virtual bool update()=0;

 //private:
	std::string name;
	//const GameObj *gob;
};

#endif
