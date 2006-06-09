#ifndef Demo_SimpleTerrain_H
#define Demo_SimpleTerrain_H

// $Id: Demo_SimpleTerrain.H,v 1.9 2006/06/06 11:25:38 ddeutscher Exp $

// This is an ORTS file 
// (c) Michael Buro, Sami Wagiaalla, David Deutscher
// licensed under the GPL

// simple grid-based terrain representation

#include "TerrainBasicImp.H"
#include "Random.H"
#include <boost/shared_ptr.hpp>

namespace Demo_SimpleTerrain {

  typedef TerrainBase::Loc      Loc;
  typedef TerrainBase::Segment  Segment;  
  typedef TerrainBase::Task     Task;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
  // fixme: Transfer this functionality into ST_Task, this is a heritage from before
  // the TerrainBase interface.
  struct UnitPath
  {
    const Object *obj;
    Vector<Loc> path;
    Loc goal;
    int current_target; // index of current way point in path vector
    bool no_orders_yet; // true 
    int collision_step; // this is incremented while solving unexpected stops
    uint4 wakeup;       // if SDL_GetTicks() >= wakeup then this UnitPath is not
                        // sleeping ie not to be skipped over.
    Loc spot;           // spot of the last unexpected collision

    UnitPath(const Object *obj_, const Vector<Loc> &path_);
    ~UnitPath() {};

    void mark_spot(const Loc &p) { spot = p; };
    void sleep(int time) { wakeup = SDL_GetTicks() + time; };
    bool sleeping() const { return wakeup > SDL_GetTicks(); };

    bool is_intersect(const Segment &) const;
  };


  /// The internal data complementing Task data:
  struct ST_Task {
    Task task;
    bool isPending;
    bool isFailed;
    bool noPathFound;
    boost::shared_ptr<UnitPath> path;    // todo: replace with a cleaner implementation

    ST_Task(const Task& task_) 
      : task(task_), isPending(true), isFailed(false), noPathFound(false) {};
    ST_Task(const ST_Task& other) 
      : task(other.task), isPending(other.isPending),
      isFailed(other.isFailed), noPathFound(other.noPathFound), path(other.path) {};
    void init_path(const Object *obj_, const Vector<Loc> &path_) { path=boost::shared_ptr<UnitPath>(new UnitPath(obj_, path_)); };
    bool isValid() const { return !isPending && !isFailed && !noPathFound && path; };
  private:
    ST_Task operator=(const ST_Task& other);
  };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

  /// A Simple implementation of the TerrainBase interface.
  /** This TerrainBase implementation is using a relatively basic Astar algorithm,
      originally written by Sami Wagiaalla and adapted by David Deutscher. 
      Limitations:
      - Only considers a single Obj per Task (ignoring the second and on obj_ids).
      - Assumes all objects are either ON_LAND or IN_AIR (and that boundaries do not 
        apply to the latter, though this is currently always true so it's not really a
        limitation).
      - Using a target *object*:
        a) No pursuit is implemented. Instead, the target location at the moment 
           of pathfinding is used as the goal.
        b) If the object is removed, it's last known location is used as goal.
        Nonetheless, it is useful (mainly for static targets) since it allows a 
        'touch' condition - where the unit is not really at the exact goal location -
        to be reported as a successful arrival and not retried.
  */
  class ST_Terrain : public TerrainBasicImp<ST_Task>
  {
  public:
    ST_Terrain() : rand(time(0)) {};
    virtual ~ST_Terrain() {};

    /// add command-line options specific to this implementation, using static methods of class Options
    static void add_options(void);

    //---------------------------------------------------------------------
    /// \name Implementation of relevant parts of the TerrainBase interface
    //---------------------------------------------------------------------
    //@{
    virtual void init(sint4 tiles_x_, sint4 tiles_y_, sint4 tile_points_);

    virtual void add_obj(const Object *obj);
    virtual void update_obj(const Object *obj);
    virtual void remove_obj(const Object *obj);
    virtual void add_segments(const Vector<TerrainBase::Segment> &s);

    virtual void cancel_task(const Object *obj);
    virtual void remove_task(TaskId tid);

    virtual void execute_tasks(Vector<MoveCmd> &cmds, Vector<StatusMsg> &msgs);
    virtual bool plan_tasks(sint4 max_time=0);

    virtual real8 find_path(const Loc &l1, const Loc &l2, sint4 radius, Path *path, ConsiderObjects consider = CONSIDER_ALL);
    virtual real8 find_path(const Object *obj, const Goal &goal, Path *path, ConsiderObjects consider = CONSIDER_ALL);
    
    // Sorts additions
    virtual void findPath(const Object* gob, const Loc &l2, Path &path);
    virtual void findPath(const Object* gob, const Object* l2, Path &path);
    virtual void findPath(const Loc &l1, const Loc &l2, Path &path);
    virtual void insertImaginaryWorker(Loc l);
    virtual void removeImaginaryWorker(Loc l);
    virtual void insertDynamicObjs();
    virtual void removeDynamicObjs();
    virtual void insertControlCenters();
    virtual void removeControlCenters();

    //@}
    //---------------------------------------------------------------------

  private:

    // prevent copying as we currently don't need it.
    ST_Terrain(const ST_Terrain& other);
    ST_Terrain operator=(const ST_Terrain& other);

    /// The mechanics of world representation and basic A* path-finding
    class PFEngine; friend class ST_Terrain::PFEngine;

    //---------------------------------------------------------------------
    /// \name Internal accessories
    //---------------------------------------------------------------------
    //@{
    /** Perform PF for a single pending task, and insert results to data structures. 
        Return true iff did some planning. */
    bool plan_pending_task(void);
    /** Perform PF for a single failed task, and insert results to data structures. 
        Return true iff did some planning. */
    bool plan_failed_task(void);
    /// Invalidate existing paths that intersect a given new boundary.
    void invalidate_paths(const Vector<Segment> &segments);
    /** Test if the object is at the given location, 
        accounting for a 1-tick look-ahead for moving objects. */
    static bool is_at_location(const Object* obj, const Loc& target);
    /** Test if the object is at the goal of the given task, implementing 
        (some) of the various definitions of 'goal'. */
    static bool is_at_goal(const Object* obj, const Task& task);
    /// Create a MoveCmd for the current waypoint of the given UnitPath, and add to the vector.
    static void add_move_command(const UnitPath& path, Vector<TerrainBase::MoveCmd> &cmds);
    //@}
    //---------------------------------------------------------------------

    boost::shared_ptr<PFEngine> pfEngine; ///< The internal Path Finding engine
    std::set<const Object*> objsToStop;   ///< Objects for which we should set a stop-moving action
    Random rand;

    friend class SimpleTerrainWidget;
    friend class SimpleTerrainOverlay;
  };

} // end of namespace Demo_SimpleTerrain

REGISTER_TYPEOFS_TerrainBasicImp_OneTime(2501, Demo_SimpleTerrain::ST_Task);
REGISTER_TYPEOFS_TerrainBasicImp_ForEach(2505, Demo_SimpleTerrain::ST_Task);

#endif
