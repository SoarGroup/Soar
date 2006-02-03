#include<map>
#include<string>
#include<list>

class FSM{
 public:
	FSM(){}
	virtual ~FSM(){}

	virtual bool update()=0;

 //private:
	std::string name;
};
