#ifndef SEnvironment_H
#define SEnvironment_H

// $Id: SEnvironment.H,v 1.14 2005/06/28 08:42:18 orts_furtak Exp $

// This is an ORTS file 
// (c) Michael Buro,
// (c) Sami Wagiaalla 
// licensed under the GPL

#include "PathEvent.H"
#include "SimpleMap.H"
#include "Path.H"
#include "GameStateModule.H"
#include "BucketFactory.H"
#include "Bucket.H"

class ClientState;

/**
   history item
*/
class HistItem {

public:
  HistItem() {}
  HistItem(const Point2i &a_, sint4 speed_): a(a_), speed(speed_) {}

  Point2i a;
  sint4 speed;
};

/**
 * this is the bridge between the ORTS world and the SimpleMap;
 * provides functions for map manipulation
 */
class SEnvironment {

public:

  static const sint4 FACTOR = 128; // distance multiplier (real->int)
  static const sint4 DIAG   = sint4(FACTOR*1.4142);

  /** cost of passing though a square occupied by another unit  */
  static const sint4 UNIT_COST = 1000000;
  
  /** cost of passing though a square within the direction of
   * another units x-second distance
   * x-second distance: if a unit continues to move in a straight
   * line it will be there in x-seconds
   */
  static const sint4 CROSS_COST = 500;

  /**
   * cost of passing though a square occupied by and orts boundery */
  static const sint4 BOUND_COST = 5000000;

  SEnvironment(GameStateModule *c, int width, int height, int tile_points, int gran);
  ~SEnvironment();

  void insert_line(sint4 x1, sint4 y1, sint4 x2, sint4 y2, sint4, CellFlag f,
                   sint4 size, sint4 cost);    
  
  void init_map(SimpleMap<SizeCell> *map_) { map = map_; }

  void init_sizes(SimpleMap<SizeCell> &smap);
  void clear_flags(SimpleMap<SizeCell> &smap);
  void clear(CellFlag f); // resets the size of cells with flag f
  void clear_air(CellFlag f); // resets the size of cells with flag f
  
  void clear_id(sint4 id); // resets the size of cells occupied by unit id
  void clear_id_air(sint4 id); // resets the size of cells occupied by unit id
  
  void set_cell(SimpleMap<SizeCell> *m, sint4 x, sint4 y, CellFlag f,
                sint4 size, sint4 id, sint4 cost);

  void clear_location(Bucket&, const SimpleMap<SizeCell>&, Vector<Point2i>&);
  UnitPath find_path(GameObj *obj, const Point2i &goal);
  
  //void insert_object(GameObj *obj, sint4 size);
  void insert_shape(const Point2i &c,
                    sint4 w,
                    sint4 h,
                    sint4 r,
                    sint4 shape,
                    sint4 zcat,
                    CellFlag f,
                    sint4 x,
                    sint4 cost);

  void insert_bucket(const Bucket &b,
                     sint4 zcat,
                     CellFlag f,
                     sint4 x,
                     sint4 id,
                     sint4 cost);
  
  void ir_object(GameObj *obj, const Point2i &c, sint4 heading, bool insert);
  void remove_object(GameObj *obj);
  void insert_object(GameObj *obj);
  
  void get_head_and_tail(Point2i &head, Point2i &tail, sint4 heading);
  bool on_line(const Point2i &a, const Point2i &b, const Point2i &c);
  void smoothen_path(Vector<Point2i> &path);
  
  uint4 estimate(sint4 x1, sint4 y1, sint4 x2, sint4 y2);
  
  bool simple_astar(SimpleMap<SizeCell> &smap, SimpleMap<AStarCell> &amap,
            const sint4 size, sint4 x1, sint4 y1, sint4 x2, sint4 y2);
  
  bool simple_astar(SimpleMap<SizeCell> &smap, SimpleMap<AStarCell> &amap,
                    Bucket b, sint4 x2, sint4 y2);
  
  SimpleMap<SizeCell> *get_map() { return map; }
  SimpleMap<SizeCell> *get_air_map() { return air_map; }

  inline sint4 world2x(sint4 worldx) const { return ((worldx+disp) / subtile_points); }
  inline sint4 world2y(sint4 worldy) const { return ((worldy+disp) / subtile_points); }
  
  inline sint4 x2world(sint4 x) const { return (x)*subtile_points-disp + disp; }
  inline sint4 y2world(sint4 y) const { return (y)*subtile_points-disp + disp; }

private:

  sint4 subtile_points;
  SimpleMap<SizeCell> *map;
  SimpleMap<SizeCell> *air_map;
  std::map<GameObj*, HistItem> history;
  GameStateModule *state;
  
  sint4 disp;
};

#endif
