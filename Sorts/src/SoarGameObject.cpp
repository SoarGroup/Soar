#include"SoarGameObject.h"
#include<iostream>

SoarGameObject::SoarGameObject(GameObj *g)
{
 gob = g;
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

void SoarGameObject::removeBehavior(SoarAction name)
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


void SoarGameObject::issueCommand(SoarAction cmd, Vector<sint4> prms)
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
   return;
  }
  std::cout<<"No match for command"<<std::endl;
}


void SoarGameObject::update()
{
 if(!memory.empty())
  if(!memory.top()->update())
   memory.pop();
}


void SoarGameObject::setGroup(SoarGameGroup *g)
{
 group = g;
}

SoarGameGroup *SoarGameObject::getGroup(void)
{
 return group;
}

SoarAction SoarGameObject::getState()
{
 return state;
}
