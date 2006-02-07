#ifndef SoarGameObject_H
#define SoarGameObject_H

#include<stack>
#include<vector>

#include "GameObj.H"

#include "FSM.h"

//How do I transition from one FSM to another?

class SoarGameGroup;

class SoarGameObject{
  public:
    SoarGameObject(GameObj* _gameObj);
    ~SoarGameObject();

    void registerBehavior(FSM *);
    void removeBehavior(std::string cmd);

    void issueCommand(std::string name);
    void update();

    void setGroup(SoarGameGroup *g);
    SoarGameGroup *getGroup();

    // I assume this can be public?
    
    GameObj* gameObj;

  private:
    std::list<FSM *> behaviors;
    std::stack<FSM *> memory;

      SoarGameGroup* group;

};


#endif
