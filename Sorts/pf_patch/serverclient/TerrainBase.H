#ifndef TerrainBase_H
#define TerrainBase_H

// $Id: TerrainBase.H,v 1.11 2006/05/14 13:20:52 ddeutscher Exp $
// This is an ORTS file (c) Michael Buro, David Deutscher, licensed under the GPL

// Terrain Base Class
// detaches terrain code from ORTS

#include "Global.H"
#include "Object.H"

class CWidget;

/** A base interface to terrain (path-finding) implementations. 
This interface detaches path-finding (PF) code from ORTS.
A derived implementation is:
1. notified of world changes, including object location/speed changes.
2. requested to handle PF tasks in 3 stages:
   a. calls to add/remove tasks.
   b. calls to plan tasks - here the implementation can spend thinking time
   c. calls to execute_tasks - the implementation should return low-level 
      move commands and arrived/failed messages per path, where relevant.
*/
class TerrainBase
{
public :
//------------------------------------------------------------------------
  //old:  typedef const void* ObjId;   // address of object on heap; id factory better?
  typedef const void* PathId;  // address of object on heap; id factory better?
  typedef const void* TaskId;  // address of object on heap; id factory better?
  //static const ObjId NOT_AN_ID;


//------------------------------------------------------------------------
  /// A terrain location
  struct Loc
  {
    sint4 x, y;

    Loc(sint4 x_=0, sint4 y_=0) : x(x_), y(y_) {}

    bool operator==(const Loc &other) const {
      return x == other.x && y == other.y;
    }

    bool operator!=(const Loc &other) const {
      return x != other.x || y != other.y;
    }

    bool operator<(const Loc &other) const { 
      return x < other.x || (x == other.x && y < other.y);
    }
    
    real8 distance(const Loc &other) const {
      const real8 dx = x - other.x;
      const real8 dy = y - other.y;
      return sqrt(dx*dx + dy*dy);
    }
  };


//------------------------------------------------------------------------
  /// A pair of locations, representing boundaries etc.
  struct Segment
  {
    Loc l1, l2;

    Segment(sint4 x1=0, sint4 y1=0, sint4 x2=0, sint4 y2=0) : l1(x1,y1), l2(x2,y2) { };
    Segment(const Loc &l1_, const Loc &l2_) : l1(l1_), l2(l2_) { };

    /** positive if l3 is on the left of the line from l1 to l2;
        negative if l3 is on the right;
        0 if l3 is on the line containing the segment. */
    sint4 is_left_turn(const Loc &l3) const { return (l2.x-l1.x)*(l3.y-l1.y) - (l2.y-l1.y)*(l3.x-l1.x); };

    /// true if the segments intersect (or touch at an endpoint/s)
    bool touches(const Segment &o) const { 
      return is_left_turn(o.l1)*is_left_turn(o.l2) <= 0 &&
        o.is_left_turn(l1)*o.is_left_turn(l2) <= 0;
    };

    /// true if the segments' interiours intersect
    bool intersects(const Segment &o) const { 
      return is_left_turn(o.l1)*is_left_turn(o.l2) < 0 &&
        o.is_left_turn(l1)*o.is_left_turn(l2) < 0;
    };
  };


//------------------------------------------------------------------------
  /// A path is a sequence of locations (waypoints)
  struct Path
  {       
    PathId id;        //< for identifying path information

    Vector<Loc> locs;

    Path() : id() { }
    Path(const Path &other) : id(other.id), locs(other.locs) {};
  private:
    Path operator=(const Path &other);
  };


//------------------------------------------------------------------------
  /// A pathfinding task to be executed
  struct Task
  {
    /// target location or an object to reach?
    enum Target { LOCATION, OBJ }           target;

    /// reach vicinity, touch object, attack object?
    enum Mode   { VICINITY, TOUCH, ATTACK } mode;

    /// units traveling at individual speeds or in formation?
    enum Group  { INDIVIDUAL, FORMATION }   group;

    /// the objects to be moved
    std::set<const Object*> objs; // links
    //Vector<const Object*> objs; 

    /// The goal location (if target==LOCATION)
    Loc goal_loc;

    /** The goal object (if target==OBJ). It's the implementation's responsibility to
        track remove_object() calls and handle cases of a goal object dying or 
        vanishing in the FOW (and this pointer becoming invalid). */
    const Object *goal_obj;  // link

//    Task() { target = LOCATION; mode = VICINITY; group = INDIVIDUAL; }
    Task(Target t = LOCATION, Mode m = VICINITY, Group g = INDIVIDUAL) 
      : target(t), mode(m), group(g) {};
    Task(const Task &other) : 
      target(other.target), mode(other.mode), group(other.group), 
      objs(other.objs), goal_loc(other.goal_loc), goal_obj(other.goal_obj) {};
  private:
    Task operator=(const Task &other);
  };


//------------------------------------------------------------------------
  /// Shortcut construction of a "move a single obj to a location target" Task
  struct Obj2LocTask : public TerrainBase::Task {
    Obj2LocTask(Object *obj, const Loc& goal_loc_) : Task(LOCATION, TOUCH, INDIVIDUAL) {
        goal_loc = goal_loc_;
        objs.insert(obj);
      };
  };


//------------------------------------------------------------------------
  /// Shortcut construction of a "move a single obj to an object target" Task
  struct Obj2ObjTask : public TerrainBase::Task {
    Obj2ObjTask(Object *obj, Object *goal_obj_) : Task(OBJ, TOUCH, INDIVIDUAL) {
        goal_obj = goal_obj_;
        objs.insert(obj);
      };
  };


//------------------------------------------------------------------------
  /// A low-level move command to be executed
  struct MoveCmd
  {
    const Object *obj;  // link
    sint4   speed;      // 0 -> stop
    Loc     next_loc;

    MoveCmd() : obj(0), speed(0) { }
  };


//------------------------------------------------------------------------
  /** A task status notification to be sent. A seperate msg is
      created for each object in multi-object tasks, since each 
      can arrive/fail individually. */
  struct StatusMsg
  {
    enum Type { ARRIVED, FAILURE } type;

    TaskId task_id;
    const Object *obj; // link

    StatusMsg() : type(ARRIVED), task_id(), obj(0) { } 
  };

//------------------------------------------------------------------------
  // TerrainBase interface itself

  TerrainBase()
  {
    tiles_x = tiles_y = tile_points = 0;
  };

  virtual ~TerrainBase();

  /// Set map dimensions
  /** set playfield dimensions
      tiles_x/y : dimensions in tiles
      tile_points : points on tile edge on fine grid 
  */
  virtual void init(sint4 tiles_x_, sint4 tiles_y_, sint4 tile_points_)
  {
    tiles_x = tiles_x_;
    tiles_y = tiles_y_;
    tile_points = tile_points_;
  };

  //---------------------------------------------------------------------
  /// \name Notifications on world changes
  //---------------------------------------------------------------------
  //@{
  virtual void add_obj(const Object *obj) = 0;
  virtual void remove_obj(const Object *obj) = 0;
  virtual void update_obj(const Object *obj) = 0;
  virtual void add_segments(const Vector<Segment> &segs) = 0;
  //}@
  //---------------------------------------------------------------------


  /** Compute path between l1 and l2 for circular object
      and returns length (in tile_points); < 0 iff path doesn't exist. */
  virtual real8 find_path(const Loc &l1, const Loc &l2, sint4 radius, Path &path) = 0;
  virtual void findPath(const Object* gob, const Loc &l2, Path &path) = 0;
    virtual void findPath(const Loc &l1, const Loc &l2, Path &path) = 0;
      virtual void findPath(const Object* gob, const Object* l2, Path &path) = 0;
  

  //---------------------------------------------------------------------
  /** \name Initiate/end pathfinding and execution
      These methods are expected to return quickly. */
  //---------------------------------------------------------------------
  //@{
  virtual TaskId add_task(const Task &task) = 0;
  virtual void remove_task(TaskId tid) = 0;  
  virtual void cancel_task(const Object *obj) = 0;
  //@}
  //---------------------------------------------------------------------


  //---------------------------------------------------------------------
  /// \name "Do some work" methods
  //---------------------------------------------------------------------
  //@{
  /** \brief Monitor path execution. Should return move commands and status 
      messages, and is expected to return quickly. */
  virtual void execute_tasks(Vector<MoveCmd> &cmds, Vector<StatusMsg> &msgs) = 0;
  /** Plan the paths you're handling. This is where long thinking should be done.
      Return true iff did some lengthy processing */
  virtual bool plan_tasks(void) = 0;
  //@}
  //---------------------------------------------------------------------

#if 0
  // creates a gfx widget that displays the terrain status
  // implemented in libs/gfxclient to avoid linking of gfx stuff in
  // non-gfx clients
  virtual CWidget *create_widget() const { return 0; }
#endif

protected:
  // data
  sint4 tiles_x,          ///< playfield width
        tiles_y;          ///< playfield height
  sint4 tile_points;      ///< points on tile edge on fine grid
};

REGISTER_TYPEOF(2401, Vector<TerrainBase::Segment>::iterator);
REGISTER_TYPEOF(2402, Vector<TerrainBase::Segment>::const_iterator);

REGISTER_TYPEOF(2403, Vector<TerrainBase::Loc>::iterator);
REGISTER_TYPEOF(2404, Vector<TerrainBase::Loc>::const_iterator);

REGISTER_TYPEOF(2405, Vector<TerrainBase::StatusMsg>::iterator);
REGISTER_TYPEOF(2406, Vector<TerrainBase::StatusMsg>::const_iterator);

REGISTER_TYPEOF(2407, Vector<TerrainBase::MoveCmd>::iterator);
REGISTER_TYPEOF(2408, Vector<TerrainBase::MoveCmd>::const_iterator);

#if 0
  // later

  struct Tile
  {
    enum Split {
      NO,
      TOPBOTTOM, //  |\|
      BOTTOMTOP, //  |/|
    } type;
    sint4 tx, ty;         // tile position
    sint4 type_w, type_e; // triangle types: west, east
  };
  
  virtual void remove_segment(const Segment &s) = 0;
  virtual void new_tile(const Tile &tile) = 0;

  // path finding
  virtual real8 path_len(const Loc &start, const Loc &goal, sint4 radius) = 0;
  virtual void  find_path(const Loc &start, const Loc &goal, sint4 radius,
			  Path &path) = 0;
#endif

#endif
