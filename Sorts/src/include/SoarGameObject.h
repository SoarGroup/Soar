#include"FSM.h"
#include<stack>

//How do I transition from one FSM to another?

class SoarGameObject{
 public:
	SoarGameObject();
	~SoarGameObject();

	void registerBehavior(FSM *);
	void removeBehavior(std::string cmd);

	void issueCommand(std::string name);
	void update();

 private:
	std::list<FSM *> behaviors;
	std::stack<FSM *> memory;
};
