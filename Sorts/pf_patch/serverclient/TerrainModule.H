#ifndef TerrainModule_H
#define TerrainModule_H

// $Id: TerrainModule.H,v 1.9 2006/06/09 12:48:47 ddeutscher Exp $
// This is an ORTS file (c) Michael Buro, David Deutscher, licensed under the GPL

// glue between ORTS and terrain code
// event-driven interface

#include "GameStateModule.H"
#include "TerrainBase.H"

class GameObj;

//===================================================================

class TerrainModule : public ClientModule, public EventHandler
{
public:
void findPath(GameObj* gob, TerrainBase::Loc goal, TerrainBase::Path& path);
  void findPath(TerrainBase::Loc start, TerrainBase::Loc goal, TerrainBase::Path& path);
    void findPath(GameObj* gob, GameObj* goal, TerrainBase::Path& path);
    void insertImaginaryWorker(TerrainBase::Loc l);
    void removeImaginaryWorker(TerrainBase::Loc l);
    void insertDynamicObjs();
    void removeDynamicObjs();
    void insertControlCenters();
    void removeControlCenters();


  //---------------------------------------------------------

  class PathEvent : public Event
  {
  public:

    PathEvent(sint4 who, sint4 what, const TerrainBase::Task &task_) :
      Event(who, what), task(task_) {}

    virtual ~PathEvent() {}

    const TerrainBase::Task& get_task() const { return task; }

  private:

    TerrainBase::Task task;
  };

  //---------------------------------------------------------

  class PathStopEvent : public Event
  {
  public:

    PathStopEvent(sint4 who, sint4 what, const Vector<GameObj*> &objs_) :
      Event(who, what), objs(objs_) {}

    virtual ~PathStopEvent() {};

    const Vector<GameObj*> &get_objs() const { return objs; }

  private:

    Vector<GameObj*> objs;
  };

  //---------------------------------------------------------

  class PathStatusEvent : public Event
  {
  public:
    
    PathStatusEvent(sint4 who, sint4 what, sint4 details_, GameObj *obj_) :
      Event(who, what), obj(obj_), details(details_) {}
    
    GameObj* get_obj() const { return obj; }
    sint4 get_details() const { return details; };
    
  private:
    GameObj *obj;
    sint4 details;
  };

  //---------------------------------------------------------

  static const sint4 FROM;
  static const sint4 FIND_PATH_MSG;
  static const sint4 FIND_PATH_STOP;
  static const sint4 PATH_SUCCESS;
  static const sint4 PATH_FAILURE;
  static const sint4 PATH_PLAN_FAILURE;
  static const sint4 PATH_MOVE_FAILURE;
  static const sint4 MOVE_PRIORITY;///< Priority of Move commands
  
  /** Initialize the TerrainModule with some implementation of the TerrainBase 
      interface. Optionally give a bound on the maximum time (in milliseconds)
      spent by the implementation on planning per each game tick 
      (0 = automatic limit). */
  TerrainModule(GameStateModule &gsm, TerrainBase &timp, sint4 max_ms_per_tick = 0);
  ~TerrainModule();

  bool handle_event(const Event &e);

private:

  /// apply set_action and send arrived/failed events
  void act(void);
  /// call the implementations' methods for view changes
  void notify_timp_on_world_changes(const GameChanges &changes);
  
  GameStateModule &gsm;
  TerrainBase &timp;        ///< terrain implementation
  sint4 max_ms_per_tick;    ///< max time (milliseconds) to be spent by timp on planning in each game tick
  
  CWidget *widget;
};

//===================================================================

#endif
