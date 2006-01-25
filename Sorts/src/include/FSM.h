#include<list>
#include<map>
#include<string>

class FSM{
 public:
	FSM();
	virtual ~FSM()=0;

	virtual bool update()=0;

 //private:
	std::string name;
};