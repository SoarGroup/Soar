#include"Sorts.h"
#include"SPathFinder.h"
#include<iostream>

#include"Game.H"
#include"Options.H"

#define Time 1

using namespace std;

const sint4 SPathFinder::FIND_PATH_MSG = EventFactory::new_what();
const sint4 SPathFinder::FIND_PATH_STOP = EventFactory::new_what();


SPathFinder::SPathFinder(const Sorts *oi) 
{
  gsm = oi->OrtsIO->getGSM();
  env = NULL;
  width = height = 0;
}

SPathFinder::~SPathFinder()
{
 if(env)
  delete env;
 if(gsm)
  gsm = NULL;
}

bool SPathFinder::handle_event(const Event &e)
{
 static Game &game = gsm->get_game();
 sint4 vf = game.get_view_frame();
 sint4 ai = game.get_action_frame();
 bool behind = abs(vf-ai) >10;

 //Submit message to find a path
 if(e.get_what() == SPathFinder::FIND_PATH_MSG)
 {
  const PathEvent &pe = static_cast<const PathEvent&>(e);
  taskQ.push(pe);
  return true;
 }

 //Submit message to stop all pathfinding
 if(e.get_what() == SPathFinder::FIND_PATH_STOP)
 {
  const PathStopEvent &pe = static_cast<const PathStopEvent&>(e);
  FORALL(pe.get_objs(),i)
  {
   pp.remove_path(*i);
  }
 }

 if(e.get_who() == GameStateModule::FROM)
 {
  //I have no idea what this is doing... Likely need to check where it is coming from
  if(e.get_what() == GameStateModule::VIEW_MSG)
  {
   if(!env)
    initEnv();

   Vector<ScriptObj*> boundaries = gsm->get_changes().new_boundaries;
   FORALL(boundaries, it)
   {
    GameObj *gob = dynamic_cast<GameObj*> (*it);
    sint4 x1 = env->world2x(*gob->sod.x1);
    sint4 y1 = env->world2y(*gob->sod.y1);
    sint4 x2 = env->world2x(*gob->sod.x2);
    sint4 y2 = env->world2y(*gob->sod.y2);
   
    env->insert_line(x1,y1,x2,y2, Object::ON_LAND, CLOSED, 1, 0);
    //Compile warning: does not like my "&" usage... Leaving in for now
    updatePaths(&Point2i(x1,y1),&Point2i(x2,y2));
   }
  

   Vector<ScriptObj*> objs;
  
   objs = gsm->get_changes().vanished_objs;
   FORALL(objs,i)
   {
    GameObj *obj = dynamic_cast<GameObj*>(*i);
    if(obj==0)
    continue;
    if(!obj->has_attr("x") || !obj->has_attr("y") || !obj->has_attr("shape"))
    continue;
    env->remove_object(obj);
   }

   objs = gsm->get_changes().dead_objs;
   FORALL(objs,i)
   {
    GameObj *obj = dynamic_cast<GameObj*>(*i);
    if(obj==0)
    continue;
    if(!obj->has_attr("x") || !obj->has_attr("y") || !obj->has_attr("shape"))
    continue;
    env->remove_object(obj);
   }
  
   objs = gsm->get_changes().new_objs;
   FORALL(objs,i)
   {
    GameObj *obj = dynamic_cast<GameObj*>(*i);
    if(obj==0)
    continue;
    if(!obj->has_attr("x") || !obj->has_attr("y") || !obj->has_attr("shape"))
    continue;
    env->insert_object(obj);
   }
  
  
   objs = gsm->get_changes().changed_objs;
   FORALL(objs,i)
   {
    GameObj *obj = dynamic_cast<GameObj*>(*i);
    if(obj==0)
     continue;
    if(obj->attr_changed("x") || obj->attr_changed("y") || 
      obj->attr_changed("zcat") || obj->attr_changed("speed"))
    {
     env->remove_object(obj);
     env->insert_object(obj);
    }
   }
  
   Vector<UnitPath> &p = pp.get_failed();
   bool norepair = (p.size()==0);
  
   if(p.size()>0 && !behind)
   {
    Vector<UnitPath>::iterator i = p.begin();
    UnitPath path = env->find_path(i->unit,i->goal);
    path.tries = i->tries;
    path.spot = i->spot;

    if(path.get_path().size()>0)
    {
     pp.add_new_path(path);
    }
    else
    {
     //Report failure
    }
    p.erase(i);
   }
  
   if(norepair && taskQ.size() >0 && !behind)
   {
    findPath(&taskQ.front());
    taskQ.pop();
   }

   if(taskQ.size() == 0 && norepair && !behind)
     pp.process_paths();

   return true;
  }
 }
 return true; 
}

void SPathFinder::initEnv()
{
 width = gsm->get_game().get_map().get_width();
 height = gsm->get_game().get_map().get_height();
 tile_points = gsm->get_game().get_tile_points();
 
 sint4 gran;
 Options::get("-gran", gran);
 
 env = new SEnvironment(gsm, width, height, tile_points, gran);

 map = env->get_map();
}



void SPathFinder::findPath(const PathEvent *e)
{
 GameObj *obj;

 Coor3 p = e->get_dest();

 real4 locX, locY;
 locX = p.x;
 locY = p.y;
 obj = (e->get_objs())[0];

 UnitPath path = env->find_path(obj, Point2i((sint4)locX,(sint4)locY));
 if(path.get_path().size()>0)
  pp.add_new_path(path);
}


void SPathFinder::updatePaths(Point2i *a, Point2i *b)
{
 Vector<UnitPath>& p = pp.get_paths();
 Vector<Line>l;

 FORALL(p,i)
 {
  l = i->intersect(*a,*b);
  if(l.size() > 0)
  {
   UnitPath up = *i;
   p.erase(i);
   UnitPath path = env->find_path(up.unit, up.goal);
   if(path.get_path().size() >0)
   {
    pp.add_new_path(path);
   }
  }
 }
}
