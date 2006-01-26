#include"include/SoarGameObject.h"

SoarGameObject::SoarGameObject()
{

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
 behaviors.push_back(b);
}

void SoarGameObject::removeBehavior(std::string name)
{
 std::list<FSM*>::iterator it;
 FSM *tmp;

 for(it = behaviors.begin(); it != behaviors.end(); it++)
  if((*it)->name.compare(name))
  {
   tmp = (*it);
   behaviors.erase(it);
   delete tmp;
   break;
  }
}


void SoarGameObject::issueCommand(std::string cmd)
{
 std::list<FSM*>::iterator it;

 //Whether we really want this is up for analysis
 while(!memory.empty())
  memory.pop();
 
 for(it = behaviors.begin(); it != behaviors.end(); it++)
  if((*it)->name.compare(cmd)) 
  {
   memory.push((*it));
   return;
  }
  
}


void SoarGameObject::update()
{
 if(!memory.empty())
  memory.top()->update();
}
