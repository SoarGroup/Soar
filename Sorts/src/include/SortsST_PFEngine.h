#ifndef SortsSTPFEh
#define SortsSTPFEh

// $Id: Demo_ST_PFEngine.H,v 1.4 2006/05/10 12:34:51 ddeutscher Exp $

// This is an ORTS file 
// (c) Michael Buro, Sami Wagiaalla, David Deutscher
// licensed under the GPL

#include "SortsST_SimpleMap.h"
#include "SortsST_Bucket.h"
#include "SortsSimpleTerrain.h"

namespace SortsSimpleTerrain {

  class SortsST_Terrain::SortsPFEngine {
  public:
    SortsPFEngine(sint4 tiles_x_, sint4 tiles_y_, sint4 tile_points_, int gran);

    //---------------------------------------------------------------------
    /// \name Changes to the world.
    //---------------------------------------------------------------------
    //@{
    void insert_object(const Object *obj);
    void remove_object(const Object *obj);
    void update_object(const Object *obj);
    void insert_boundary(const SortsSimpleTerrain::Segment& line);
    //@}
    //---------------------------------------------------------------------


    /// Perform pathfinding, return true iff path found, and the path itself in 'output'
    bool find_path(const Object *obj, const SortsSimpleTerrain::Loc &goal, SortsSimpleTerrain::Path& output);
    bool find_path(const SortsSimpleTerrain::Loc &start, const SortsSimpleTerrain::Loc &goal, SortsSimpleTerrain::Path& output);
    void clearGobLocation(const Object*);
    void insertImaginaryWorker(SortsSimpleTerrain::Loc l);
    void removeImaginaryWorker(SortsSimpleTerrain::Loc l);
    void insertDynamicObjs();
    void removeDynamicObjs();
    void insertControlCenters();
    void removeControlCenters();
    std::set<const Object*> dynamicObjs;
    std::set<const Object*> controlCenters;


//    typedef boost::shared_ptr< SortsSimpleMap<SizeCell> > MapPtr;
    typedef SortsSimpleMap<SizeCell>* MapPtr;

    sint4 subtile_points;         ///< points per tile in our representation
    sint4 disp;                   ///< objects are displaced half a tile so boundaries fall in the middle of tiles
    SortsSimpleMap<SizeCell> map;
    SortsSimpleMap<SizeCell> air_map;

    //---------------------------------------------------------------------
    /// \name Updates to the map
    //---------------------------------------------------------------------
    //@{
    void init_sizes(SortsSimpleMap<SizeCell> &smap);

    void ir_object(const Object *obj, bool insert, 
      bool use_prev_attributes=false);                              ///< Expects world coordinates
    void insert_rectangle(const Loc &topleft,const Loc &bottomright, 
      Object::ZCat zcat, CellFlag f, sint4 x, sint4 cost);          ///< Expects world coordinates

    void insert_bucket(const Bucket &b, Object::ZCat zcat, 
      CellFlag f, sint4 x, sint4 cost);                             ///< Expects inner coordinates
    void insert_line(sint4 x1, sint4 y1, sint4 x2, sint4 y2,
      Object::ZCat zcat, CellFlag f, sint4 size, sint4 cost);       ///< Expects inner coordinates
    void set_cell(MapPtr m, sint4 x, sint4 y,
      CellFlag f, sint4 size, sint4 cost);                          ///< Expects inner coordinates
    //@}
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    /// \name Coordinate translation: world <-> inner representation
    // fixme: the problem I found in the translation (ddeutscher)
    //---------------------------------------------------------------------
    //@{
    sint4 world2x(sint4 worldx) const { return ((worldx+disp) / subtile_points); };
    sint4 world2y(sint4 worldy) const { return ((worldy+disp) / subtile_points); };
    sint4 x2world(sint4 x) const { return (x)*subtile_points-disp + disp; };
    sint4 y2world(sint4 y) const { return (y)*subtile_points-disp + disp; };
    //@}
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    /// \name Path finding
    //---------------------------------------------------------------------
    //@{
    /// The actual A* implementation
    bool simple_astar(SortsSimpleMap<SizeCell> &smap, SortsSimpleMap<AStarCell> &amap,
      Bucket b, sint4 x2, sint4 y2);
    /// If the currentl location it is occupied, move away from it.
    void clear_location(Bucket &b, const SortsSimpleMap<SizeCell> &smap, Vector<SortsSimpleTerrain::Loc> &path);
    /// Remove redundant waypoints (that fall on a straight line)
    void smoothen_path(Vector<SortsSimpleTerrain::Loc> &path);
    /// Compute the octile distance between two points
    uint4 estimate(sint4 x1, sint4 y1, sint4 x2, sint4 y2);
    /// Check whether the given three points are on a straight line
    bool on_line(const SortsSimpleTerrain::Loc &a, const SortsSimpleTerrain::Loc &b, const SortsSimpleTerrain::Loc &c);
    //@}
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    /// \name Cost constants
    //---------------------------------------------------------------------
    //@{
    static const sint4 FACTOR = 128; ///< distance multiplier (real->int)
    static const sint4 DIAG   = sint4(FACTOR*1.4142);
    /// cost of passing though a square occupied by another unit
    static const sint4 UNIT_COST = 5000000;
    //static const sint4 UNIT_COST = 1000000;
    /** cost of passing though a square within the direction of
    * another units x-second distance
    * x-second distance: if a unit continues to move in a straight
    * line it will be there in x-seconds */
    static const sint4 CROSS_COST = 500;
    /// cost of passing though a square occupied by and orts boundery
    static const sint4 BOUND_COST = 5000000;
    //@}
    //---------------------------------------------------------------------

    friend class SimpleTerrainWidget;
  };

} // end of namespace SortsSimpleTerrain

#endif
