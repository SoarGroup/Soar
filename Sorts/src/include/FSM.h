#ifndef FSM_H_
#define FSM_H_

#include<map>
#include<string>
#include<list>
#include<vector>

class FSM{
 public:
	FSM(){}
	virtual ~FSM(){}

	virtual bool update()=0;

 //private:
	std::string name;
};

#endif
