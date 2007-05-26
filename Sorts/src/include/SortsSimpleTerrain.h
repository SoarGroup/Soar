#ifndef Sorts_SimpleTerrain_h
#define Sorts_SimpleTerrain_h

// $Id: SortsSimpleTerrain.H,v 1.11 2006/06/09 18:46:14 ddeutscher Exp $

// This is an ORTS file 
// (c) Michael Buro, Sami Wagiaalla, David Deutscher
// licensed under the GPL

// simple grid-based terrain representation

#include "Object.H"
#include "Global.H"
#include "Random.H"
#include <boost/shared_ptr.hpp>

namespace SortsSimpleTerrain {
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
    typedef const void* PathId;  // address of object on heap; id factory better?
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

  class SortsST_Terrain
  {
  public:
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
    sint4 tiles_x,          ///< playfield width
          tiles_y;          ///< playfield height
    sint4 tile_points;      ///< points on tile edge on fine grid
    sint4 client_pID,       ///< ID of the client player
          neutral_pID;      ///< ID of neutral player (eg. sheep)

    SortsST_Terrain() : rand(time(0)) {
      tiles_x = tiles_y = tile_points = 0;
    }
    ~SortsST_Terrain() {};

    /// add command-line options specific to this implementation, using static methods of class Options
    static void add_options(void);

    //---------------------------------------------------------------------
    /// \name Implementation of relevant parts of the SortsSimpleTerrain interface
    //---------------------------------------------------------------------
    //@{
     void init(sint4 tiles_x_, sint4 tiles_y_, sint4 tile_points_, sint4 me, sint4 neutral);

     void add_obj(const Object *obj);
     void update_obj(const Object *obj);
     void remove_obj(const Object *obj);
     void add_segments(const Vector<SortsSimpleTerrain::Segment> &s);

    // Sorts additions
     void findPath(const Object* gob, const Loc &l2, Path &path);
     void findPath(const Object* gob, const Object* l2, Path &path);
     void findPath(const Loc &l1, const Loc &l2, Path &path);
     void insertImaginaryWorker(Loc l);
     void removeImaginaryWorker(Loc l);
     void insertDynamicObjs();
     void removeDynamicObjs();
     void insertControlCenters();
     void removeControlCenters();

    //@}
    //---------------------------------------------------------------------

  private:

    // prevent copying as we currently don't need it.
    SortsST_Terrain(const SortsST_Terrain& other);
    SortsST_Terrain operator=(const SortsST_Terrain& other);

    /// The mechanics of world representation and basic A* path-finding
    class SortsPFEngine; friend class SortsST_Terrain::SortsPFEngine;

    /** Test if the object is at the given location, 
        accounting for a 1-tick look-ahead for moving objects. */
    static bool is_at_location(const Object* obj, const Loc& target);
    /** Test if the object is at the goal of the given task, implementing 
        (some) of the various definitions of 'goal'. */

    boost::shared_ptr<SortsPFEngine> pfEngine; ///< The internal Path Finding engine
    Random rand;

  };

  typedef SortsSimpleTerrain::Loc      Loc;
  typedef SortsSimpleTerrain::Segment  Segment;  


} // end of namespace SortsSimpleTerrain

#endif
