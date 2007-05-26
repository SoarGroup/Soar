// $Id: SortsTerrainModule.C,v 1.24 2006/06/10 02:48:35 orts_mburo Exp $

// This is an ORTS file (c) Michael Buro, David Deutscher, licensed under the GPL

#include "SortsTerrainModule.h"
#include "Game.H"
#include "GameObj.H"
#include "Options.H"
#include "GameChanges.H"

using namespace std;

const sint4 SortsTerrainModule::FROM               = EventFactory::new_who();
const sint4 SortsTerrainModule::FIND_PATH_MSG      = EventFactory::new_what();
const sint4 SortsTerrainModule::FIND_PATH_STOP     = EventFactory::new_what();
const sint4 SortsTerrainModule::PATH_SUCCESS       = EventFactory::new_what();
const sint4 SortsTerrainModule::PATH_FAILURE       = EventFactory::new_what();
const sint4 SortsTerrainModule::PATH_PLAN_FAILURE  = EventFactory::new_what();
const sint4 SortsTerrainModule::PATH_MOVE_FAILURE  = EventFactory::new_what();
const sint4 SortsTerrainModule::MOVE_PRIORITY = 1; 

SortsTerrainModule::SortsTerrainModule(GameStateModule &gsm_, SortsSimpleTerrain::SortsST_Terrain &timp_, sint4 max_ms_per_tick_)
  : gsm(gsm_), timp(timp_), max_ms_per_tick(max_ms_per_tick_)
{
  gsm.add_handler(this);

  // Auto compute max_ms_per_tick
  //if( max_ms_per_tick == 0 ){    
  //max_ms_per_tick = (uint4)(1000/gsm.get_game().get_freq() - 20);  // milliseconds
    //}
}


SortsTerrainModule::~SortsTerrainModule()
{
  gsm.remove_handler(this);
  // delete widget;  // fixme: move code to gfxclient
}


#if 0

// translates GameObj into SortsSimpleTerrain::Obj

static SortsSimpleTerrain::Obj gameobj_to_obj(const GameObj *p)
{
  SortsSimpleTerrain::Obj obj;

  assert(p && p->has_attr("x") && p->has_attr("y") &&
	 p->has_attr("shape") && p->has_attr("zcat") && "missing attribute");

  obj.center = SortsSimpleTerrain::Loc(*p->sod.x, *p->sod.y);

  switch(*p->sod.shape) {
  case Object::RECTANGLE: obj.shape = SortsSimpleTerrain::Obj::RECTANGLE; break;
  case Object::CIRCLE:    obj.shape = SortsSimpleTerrain::Obj::CIRCLE;    break;
  default: ERR("unknown shape"); break;
  }

  switch(*p->sod.zcat) {
  case Object::ON_LAND: obj.layer = SortsSimpleTerrain::Obj::LAND; break;
  case Object::IN_AIR:  obj.layer = SortsSimpleTerrain::Obj::AIR;  break;
  default: ERR("unknown zcat"); break;
  }

  if (obj.shape == SortsSimpleTerrain::Obj::RECTANGLE) {

    assert(p->has_attr("width") && p->has_attr("height") && "missing attribute");    
    obj.width  = p->get_int("width");
    obj.height = p->get_int("height");

    obj.max_speed = 0; // rectangles immobile for now
    obj.speed = 0;
    obj.is_moving = 0;    
    
  } else { // CIRCLE

    assert(p->has_attr("radius") && "missing attribute");    
    obj.radius = p->get_int("radius");

    obj.max_speed = *p->sod.max_speed;
    obj.speed     = *p->sod.speed;
    obj.is_moving = *p->sod.is_moving;
  }

  obj.id = p; // object id is address of gameobj

  return obj;
}
#endif

bool SortsTerrainModule::handle_event(const Event &e)
{
  // fixme: add parameters to terrainmodule functions dealing with lag
  Game &game = gsm.get_game();
  sint4 vf = game.get_view_frame();
  sint4 ai = game.get_action_frame();
  bool behind = abs(vf-ai) > 10 || ai < 0;

  //------------------------------
  // handle pathfinding events
  
/*  if (e.get_what() == FIND_PATH_MSG) {
    
    REM("FIND_PATH_MSG EVENT");
    
    const PathEvent *pe = dynamic_cast<const PathEvent*>(&e);
    assert(pe && "no PathEvent");

    const SortsSimpleTerrain::Task &task = pe->get_task();
    timp.add_task(task);
    return true;
  }
  
  if (e.get_what() == FIND_PATH_STOP) {
    
    REM("STOP_PATH EVENT");
    const PathStopEvent *pe = dynamic_cast<const PathStopEvent*>(&e);
    assert(pe && "no PathStopEvent");
    FORALL (pe->get_objs(), it) timp.cancel_task(*it);
    return true;
  }
*/
  if (e.get_who() == GameStateModule::FROM) {

    if (e.get_what() == GameStateModule::VIEW_MSG) {

      // If this is the first view, initialize the SortsSimpleTerrain implementation
      if( game.get_view_frame() == 0 ) {
	if( max_ms_per_tick == 0 ){    
	  max_ms_per_tick = (uint4)(1000/gsm.get_game().get_freq() - 20);  // milliseconds
	  cout << "MAXTIME " << max_ms_per_tick << endl;
	}
        timp.init(game.get_map().get_width(),
                  game.get_map().get_height(),
                  game.get_tile_points(),
		  game.get_client_player(),
		  game.get_player_num()); //The total number of players is also the id of the neutral player.
      }

      notify_timp_on_world_changes(gsm.get_changes());

      /* - if we're behind, don't let the timp do anything.
         - as long as there is some paths to plan, do it.
         - if not behind and nothing to plan, executed the planned paths.

         This is not a very good logic, but since planning might take a long
         time, if we plan and execute in the same cycle, the actions we'll send
         might get skipped over for being too old when reaching the server.
         So we're using this temporary solution, and waiting for a better one,
         probably involving the main client loop controling when to set actions
         (e.g. when no server messages are available in the read buffer).
      */
      /*
      // Give the implementation a chance to plan
      if( !behind ) {
        behind = timp.plan_tasks();
	}
      // if still not behind, execute the active tasks
      if( !behind ) {
      act();
      }*/
      
      // plan and execute moves, but not if we're behind
      if( !behind ) {
        //timp.plan_tasks(max_ms_per_tick);
        //act();
      }
    }
  }

  return true;
}

void SortsTerrainModule::notify_timp_on_world_changes(const GameChanges &changes)
{
  // boundaries
  Vector<SortsSimpleTerrain::Segment> segments;      
  FORALL (changes.new_boundaries, it) {
    GameObj *gob = (*it)->get_GameObj();
    if (gob == 0) continue;
    segments.push_back(SortsSimpleTerrain::Segment(
      SortsSimpleTerrain::Loc(*gob->sod.x1, *gob->sod.y1),
      SortsSimpleTerrain::Loc(*gob->sod.x2, *gob->sod.y2) ));
  } 
  if (!segments.empty()) 
    timp.add_segments(segments);

  // vanished objects
  FORALL (changes.vanished_objs, i) {
    GameObj* p = (*i)->get_GameObj();
    if (!p ||
      !p->has_attr("x") ||
      !p->has_attr("y") ||
      !p->has_attr("shape"))
      continue;

    // fixme: what about immobile objects getting out of sight?
    // remove or not?
    timp.remove_obj(p);
  }

  // dead objects
  FORALL (changes.dead_objs, i) {
    GameObj* p = (*i)->get_GameObj();
    if (!p ||
      !p->has_attr("x") ||
      !p->has_attr("y") ||
      !p->has_attr("shape"))
      continue;
    timp.remove_obj(p);
  }

  // new objects
  FORALL (changes.new_objs, i) {
    GameObj *p = (*i)->get_GameObj();
    if (!p ||
      !p->has_attr("x") ||
      !p->has_attr("y") ||
      !p->has_attr("shape"))
      continue;

    timp.add_obj(p);
  }

  // changed objects
  FORALL (changes.changed_objs, i) {
    GameObj* p = (*i)->get_GameObj();
    if (!p) continue;
    if (p->attr_changed("x") || p->attr_changed("y") ||
      p->attr_changed("is_moving") || p->attr_changed("zcat") ||
      p->attr_changed("speed")) {

        timp.update_obj(p);
      }
  }
}
/*
void SortsTerrainModule::act(void)
{
  // get the actions from the terrain implementation
  Vector<SortsSimpleTerrain::MoveCmd> moves;
  Vector<SortsSimpleTerrain::StatusMsg> msgs;
  timp.execute_tasks(moves,msgs);

  // execute the moves
  FORALL(moves, m) {
    GameObj* gob = dynamic_cast<GameObj*>(const_cast<Object*>(m->obj));
    if( !gob ) continue;
    Vector<sint4> params;          
    params.push_back(m->next_loc.x);
    params.push_back(m->next_loc.y);
    params.push_back(max((sint4)0,m->speed));
    gob->set_action("move", params, MOVE_PRIORITY);
  }

  // send the msgs
  FORALL(msgs, m) {
    GameObj* gob = dynamic_cast<GameObj*>(const_cast<Object*>(m->obj));
    if( !gob )
      continue;

    sint4 type = PATH_SUCCESS;
    sint4 details = 0;
    if( m->type == SortsSimpleTerrain::StatusMsg::NO_PATH_FAILURE ) {
      type = PATH_FAILURE;
      details = PATH_PLAN_FAILURE;
    } else if( m->type == SortsSimpleTerrain::StatusMsg::MOVEMENT_FAILURE ) {
      type = PATH_FAILURE;
      details = PATH_MOVE_FAILURE;
    }
    call_handlers(PathStatusEvent(FROM, type, details, gob));
  }
}*/

void SortsTerrainModule::findPath(GameObj* gob, SortsSimpleTerrain::Loc goal, SortsSimpleTerrain::Path& path) {
    timp.findPath(gob, goal, path);
}

void SortsTerrainModule::findPath(SortsSimpleTerrain::Loc start, SortsSimpleTerrain::Loc goal, SortsSimpleTerrain::Path& path) {
    timp.findPath(start, goal, path);
}


void SortsTerrainModule::findPath(GameObj* gob, GameObj* goal, SortsSimpleTerrain::Path& path) {
    timp.findPath(gob, goal, path);
}

void SortsTerrainModule::insertImaginaryWorker(SortsSimpleTerrain::Loc l) {
  timp.insertImaginaryWorker(l);
}
void SortsTerrainModule::removeImaginaryWorker(SortsSimpleTerrain::Loc l) {
  timp.removeImaginaryWorker(l);
}
void SortsTerrainModule::insertDynamicObjs() {
  timp.insertDynamicObjs();
}
void SortsTerrainModule::removeDynamicObjs() {
  timp.removeDynamicObjs();
}
void SortsTerrainModule::insertControlCenters() {
  timp.insertControlCenters();
}
void SortsTerrainModule::removeControlCenters() {
  timp.removeControlCenters();
}

