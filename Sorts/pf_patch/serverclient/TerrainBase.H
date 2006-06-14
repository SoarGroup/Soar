#ifndef TerrainBase_H
#define TerrainBase_H

// $Id: TerrainBase.H,v 1.22 2006/06/09 15:14:01 ddeutscher Exp $
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
        0 if l3 is on the line containing the segment. 
        Adapted for the reversed y-axis used by ORTS. */
    sint4 is_left_turn(const Loc &l3) const { return (l2.y-l1.y)*(l3.x-l1.x) - (l2.x-l1.x)*(l3.y-l1.y); };

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
    Path& operator=(const Path &other) { 
      if( this != &other ) { 
        id = other.id;
        locs = other.locs;
      }
      return *this;
    };
  };


//------------------------------------------------------------------------
  /// A movement goal
  struct Goal
  {
    /// target location or an object to reach?
    enum Target { LOCATION, OBJ }           target;

    /// reach vicinity, touch object, attack object?
    enum Mode   { VICINITY, TOUCH, ATTACK } mode;

    /// The goal location (if target==LOCATION)
    Loc loc;
    
    /** The goal object (if target==OBJ). 
        It's the implementation's responsibility to track remove_object() 
        calls and handle cases of a goal object dying or vanishing in the 
        FOW (and this pointer becoming invalid). */
    const Object *obj;

    /// The distance from the target that is 'close enough' (if mode==VICINITY)
    sint4 distance;

    Goal(Target t, Mode m) : target(t), mode(m), obj(NULL), distance(0) {};
    Goal(const Goal &other) 
      : target(other.target), mode(other.mode), loc(other.loc), obj(other.obj),
        distance(other.distance) {};
  private:
    Goal& operator=(const Goal &other);

  };

  /// A pathfinding task to be executed
  struct Task
  {
    /// The goal of the movement.
    Goal goal;

    /// Units traveling at individual speeds or in formation?
    enum Group { INDIVIDUAL, FORMATION } group;

    /** The objects to be moved. 
        It's the implementation's responsibility to track remove_object() 
        calls and handle cases of a objects dying or vanishing in the 
        FOW (and pointers becoming invalid). */
    std::set<const Object*> objs;

    Task(Goal::Target t = Goal::LOCATION, Goal::Mode m = Goal::VICINITY, Group g = INDIVIDUAL) 
      : goal(t,m), group(g)  {};
    Task(const Task &other) : goal(other.goal), group(other.group), objs(other.objs) {};
  private:
    Task& operator=(const Task &other);
  };


//------------------------------------------------------------------------
  /// Shortcut construction of a "move a single obj to a location target" Task
  struct Obj2LocTask : public TerrainBase::Task {
    Obj2LocTask(Object *obj, const Loc& goal_loc_) : Task(Goal::LOCATION, Goal::TOUCH, INDIVIDUAL) {
      goal.loc = goal_loc_;
      objs.insert(obj);	
    };
  };


//------------------------------------------------------------------------
  /// Shortcut construction of a "move a single obj to an object target" Task
  struct Obj2ObjTask : public TerrainBase::Task {
    Obj2ObjTask(Object *obj, const Object *goal_obj_) : Task(Goal::OBJ, Goal::TOUCH, INDIVIDUAL) {
      goal.obj = goal_obj_;
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
    enum Type { ARRIVED,          ///< object arrived at goal
                NO_PATH_FAILURE,  ///< no path to goal could be computed
                MOVEMENT_FAILURE  ///< failed to execute the planned task
    } type;

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
  /** set playfield dimensions and gives the pathfinder the IDs of the client
      and the neutral players
      tiles_x/y : dimensions in tiles
      tile_points : points on tile edge on fine grid 
  */
  virtual void init(sint4 tiles_x_, sint4 tiles_y_, sint4 tile_points_, sint4 client_pID_, sint4 neutral_pID_)
  {
    tiles_x = tiles_x_;
    tiles_y = tiles_y_;
    tile_points = tile_points_;
    client_pID = client_pID_;
    neutral_pID = neutral_pID_;
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


  //---------------------------------------------------------------------
  /// \name Requests for an immediate planning of a path
  //---------------------------------------------------------------------
  //@{
  /// Flags that specify what objects to consider as obstacles.
  enum ConsiderObjects {
    CONSIDER_ALL                = 0,
    /// Ignore all friendly (owned) mobile objects.
    IGNORE_MOBILE_FRIENDS   = 0x001, 
    /// Ignore all enemy mobile objects.
    IGNORE_MOBILE_FOES      = 0x010,
    /// Ignore all neutral ("sheep") mobile objects.
    IGNORE_MOBILE_NEUTRALS  = 0x100,
    /// Ignore all mobile objects, considering only boundaries and non-mobile objects.
    IGNORE_ALL_MOBILE_OBJS      = IGNORE_MOBILE_FRIENDS | IGNORE_MOBILE_FOES | IGNORE_MOBILE_NEUTRALS,
  };

  /** Compute path between l1 and l2 for a circular object.
      @param l1         Start of requested path.
      @param l2         End of requested path.
      @param radius     Radius (half width) of the requested path.
      @param path       If != NULL, and a path was found, this will contain the it
                        (including l2 but not l1), with the first waypoint in locs[0].
      @param consider   What objects to consider as obstacles when searching for a path.
      @return           The length of the path (in tile_points); < 0 iff path doesn't exist. 
  */
  virtual real8 find_path(const Loc &l1, const Loc &l2, sint4 radius, Path *path, ConsiderObjects consider = CONSIDER_ALL) = 0;

virtual void findPath(const Object* gob, const Loc &l2, Path &path) = 0;
    virtual void findPath(const Loc &l1, const Loc &l2, Path &path) = 0;
      virtual void findPath(const Object* gob, const Object* l2, Path &path) = 0;
      virtual void insertImaginaryWorker(Loc l) = 0;
      virtual void removeImaginaryWorker(Loc l) = 0;
      virtual void insertDynamicObjs() = 0;
      virtual void removeDynamicObjs() = 0;
      virtual void insertControlCenters() = 0;
      virtual void removeControlCenters() = 0;


  /** Compute path for a given (circular only) Object with a given Goal,
      @param obj        The object to move, including the start position and radius.
      @param goal       Target of the requested path (location or object etc.).
      @param path       If != NULL, and a path was found, this will contain it 
                        (including the final but not the start position), with the 
                        first waypoint in locs[0].
      @param consider   What objects to consider as obstacles when searching for a path.
      @return           The length of the path (in tile_points); < 0 iff path doesn't exist. 
  */
  virtual real8 find_path(const Object *obj, const Goal &goal, Path *path, ConsiderObjects consider = CONSIDER_ALL) = 0;
  //---------------------------------------------------------------------
  

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
      Return true iff did some lengthy processing.
      Try to use only the specified amount of time for planning before returning
      control to the caller. If max_time == 0 then there is no requested time limit. */
  virtual bool plan_tasks(uint4 max_time = 0) = 0;
  //@}
  //---------------------------------------------------------------------

protected:
  // data
  sint4 tiles_x,          ///< playfield width
        tiles_y;          ///< playfield height
  sint4 tile_points;      ///< points on tile edge on fine grid
  sint4 client_pID,       ///< ID of the client player
        neutral_pID;      ///< ID of neutral player (eg. sheep)
};

extern std::ostream& operator<<(std::ostream& os, const TerrainBase::Loc& l);
extern std::ostream& operator<<(std::ostream& os, const TerrainBase::Segment& s);

REGISTER_TYPEOF(2401, Vector<TerrainBase::Segment>::iterator);
REGISTER_TYPEOF(2402, Vector<TerrainBase::Segment>::const_iterator);

REGISTER_TYPEOF(2403, Vector<TerrainBase::Loc>::iterator);
REGISTER_TYPEOF(2404, Vector<TerrainBase::Loc>::const_iterator);

REGISTER_TYPEOF(2405, Vector<TerrainBase::StatusMsg>::iterator);
REGISTER_TYPEOF(2406, Vector<TerrainBase::StatusMsg>::const_iterator);

REGISTER_TYPEOF(2407, Vector<TerrainBase::MoveCmd>::iterator);
REGISTER_TYPEOF(2408, Vector<TerrainBase::MoveCmd>::const_iterator);

REGISTER_TYPEOF(2409, Vector<TerrainBase::Loc>::reverse_iterator);
REGISTER_TYPEOF(2410, Vector<TerrainBase::Loc>::const_reverse_iterator);

REGISTER_TYPEOF(2411, Vector<TerrainBase::Path>::iterator);
REGISTER_TYPEOF(2412, Vector<TerrainBase::Path>::const_iterator);

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
