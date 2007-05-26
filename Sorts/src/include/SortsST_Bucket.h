#ifndef Sortsbucket_h
#define Sortsbucket_h

// $Id: Demo_ST_Bucket.H,v 1.3 2006/05/10 12:34:51 ddeutscher Exp $
      
// This is an ORTS file (c) Michael Buro, David Deutscher, licensed under the GPL

/**
 * A bucket provides noth,south,east, and west faces for the object
 * it contains. This is used to optimize search though the grid.
 * So if a move to the north is being considered, having a clear path
 * for the north face implise that one exists for the entire cirlce to 
 * move north. That way only the appropriate face needs to be checked.
 * */

#include "SortsSimpleTerrain.h"
#include "SortsST_SimpleMap.h"

#include "boost/array.hpp"
#include "boost/smart_ptr.hpp"
#include <bitset>

namespace SortsSimpleTerrain {

  typedef SortsSimpleTerrain::Loc Loc;

  class Bucket {

  public:

    Bucket() : p_size(0), N_size(0), E_size(0), S_size(0), W_size(0) {};
    Bucket( const Vector<Loc> &points_, 
            const Vector<Loc> &pointsN_,
            const Vector<Loc> &pointsE_,
            const Vector<Loc> &pointsS_,
            const Vector<Loc> &pointsW_);
    Bucket(const Bucket &o);
    Bucket& operator=(const Bucket&);
    ~Bucket() {};

    const Loc &get_center() const { return center; }
    const Vector<Loc> &get_points() const { return pointsVector; }

    void set_center(const Loc &c) { center = c; }

    /**
     * check the appropriet face(s) to move in the give direction,
     * place the cost of movement in that direction in cost
     * */
    bool check_direction(const SortsSimpleMap<SizeCell> &map, sint4 d, sint4 &cost);

    /**
     * check all 8 faces together, which should be faster relative to 8 seperate 
     * calls to check_direction() (since the latter perform replicated tests).
     * Free/blocked movement is returned in 'free', costs in 'cost'.
     * The 0-index direction is east, 1 is south-east etc.
     * */
    void check_all_directions(const SortsSimpleMap<SizeCell> &map, std::bitset<8> &free, boost::array<sint4,8> &cost);

    /**
     * check that there is open space for the movement of the squares in the
     * given vector in the given direction (using x and y displacements).
     * place the cost of the movement in cost.
     * */
    bool check_vector(const SortsSimpleMap<SizeCell> &map, const Vector<Loc> &points,
		      sint4 dx, sint4 dy, sint4 &cost);
    
  private:

    // This is a faster version, exploiting the internal less-safe-but-faster arrays
    bool check_vector(const SortsSimpleMap<SizeCell> &map, Loc *points, size_t count,
		      sint4 dx, sint4 dy, sint4 &cost);


    Loc center;
  
    // Keep the points internally in a plain block of memory to speed access
    // in the time-critical check_vector method

    boost::shared_array<Loc> points;  // points of the border of the bucket
    boost::shared_array<Loc> pointsN; // points of the north face of the bucket
    boost::shared_array<Loc> pointsE; // points of the east face of the bucket
    boost::shared_array<Loc> pointsS; // points of the south face of the bucket
    boost::shared_array<Loc> pointsW; // points of the west face of the bucket

    size_t p_size, N_size, E_size, S_size, W_size;

    // For external users, keep also in the safer Vector
    Vector<Loc> pointsVector;
  };

  class BucketFactory
  {
  public:
    static Bucket get_bucket(int r);

    // public to allow REGISTER_TYPEOF definitions
    typedef std::map<int, Bucket> BucketSamples; 

  private:  
    static BucketSamples samples;
  };
}

REGISTER_TYPEOF(4501, SortsSimpleTerrain::BucketFactory::BucketSamples::iterator);
REGISTER_TYPEOF(4502, SortsSimpleTerrain::BucketFactory::BucketSamples::const_iterator);

#endif
