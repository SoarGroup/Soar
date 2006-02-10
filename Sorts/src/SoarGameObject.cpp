#include"SoarGameObject.h"
#include"SoarGameGroup.h"
#include<iostream>

SoarGameObject::SoarGameObject(GameObj *g)
{
 gob = g;
 FSM* temp = (FSM*)(new MoveFSM());
 registerBehavior(temp);
}

SoarGameObject::~SoarGameObject()
{
 FSM* tmp;

 while(!memory.empty())
  memory.pop();

 while(!behaviors.empty())
 {
  tmp = behaviors.front();
  behaviors.pop_front();
  delete tmp;
 }
}

void SoarGameObject::registerBehavior(FSM *b)
{
 b->setGameObject(gob);
 behaviors.push_back(b);
}

void SoarGameObject::removeBehavior(SoarActionType name)
{
 std::list<FSM*>::iterator it;
 FSM *tmp;

 for(it = behaviors.begin(); it != behaviors.end(); it++)
  if((*it)->name == name)
  {
   tmp = (*it);
   behaviors.erase(it);
   delete tmp;
   break;
  }
}


void SoarGameObject::issueCommand(SoarActionType cmd, Vector<sint4> prms)
{
 std::list<FSM*>::iterator it;

 //Whether we really want this is up for analysis
 while(!memory.empty())
  memory.pop();
 
 for(it = behaviors.begin(); it != behaviors.end(); it++)
  if((*it)->name == cmd) 
  {
   (*it)->setParams(prms);
   memory.push((*it));
   state = cmd;
   cout << "ACTION" << endl;
   update();
   return;
  }
  std::cout<<"No match for command"<<std::endl;
}


void SoarGameObject::update()
{
  //cout << "updated object" << endl;
  group->setStale();
 if(!memory.empty())
  if(!memory.top()->update())
  {
   memory.pop();
   if(!memory.empty())
    state = memory.top()->name;
   else
    state = SA_IDLE;
  }
}


void SoarGameObject::setGroup(SoarGameGroup *g)
{
 group = g;
}

SoarGameGroup *SoarGameObject::getGroup(void)
{
 return group;
}

SoarActionType SoarGameObject::getState()
{
 return state;
}
