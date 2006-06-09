#ifndef TerrainModule_H
#define TerrainModule_H

// $Id: TerrainModule.H,v 1.6 2006/05/10 12:34:52 ddeutscher Exp $
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
    
    PathStatusEvent(sint4 who, sint4 what, GameObj *obj_) :
      Event(who, what), obj(obj_) {}
    
    GameObj* get_obj() const { return obj; }
    
  private:
    
    GameObj *obj;
  };

  //---------------------------------------------------------

  static const sint4 FROM;
  static const sint4 FIND_PATH_MSG;
  static const sint4 FIND_PATH_STOP;
  static const sint4 PATH_SUCCESS;
  static const sint4 PATH_FAILURE;

  // can pass different terrain implementations here
  TerrainModule(GameStateModule &gsm_, TerrainBase &timp);
  ~TerrainModule();

  bool handle_event(const Event &e);

private:

  /// apply set_action and send arrived/failed events
  void act(void);
  /// call the implementations' methods for view changes
  void notify_timp_on_world_changes(const GameChanges &changes);
  
  GameStateModule &gsm;
  TerrainBase &timp;    // terrain implementation
  
  CWidget *widget;
};

//===================================================================

#endif
