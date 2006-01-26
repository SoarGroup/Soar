#include<list>
#include<map>
#include<string>

class FSM{
 public:
	FSM(){}
	virtual ~FSM(){}

	virtual bool update()=0;

 //private:
	std::string name;
};
