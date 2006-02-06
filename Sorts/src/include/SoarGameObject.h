#ifndef SoarGameObject_H
#define SoarGameObject_H

#include<stack>
#include<vector>

#include "FSM.h"

//How do I transition from one FSM to another?

class SoarGameGroup;

class SoarGameObject{
  public:
    SoarGameObject();
    ~SoarGameObject();

    void registerBehavior(FSM *);
    void removeBehavior(std::string cmd);

    void issueCommand(std::string name);
    void update();

    void setGroup(SoarGameGroup *g);
    SoarGameGroup *getGroup();

    // I assume this can be public?

  private:
    std::list<FSM *> behaviors;
    std::stack<FSM *> memory;

      SoarGameGroup* group;

};


#endif
