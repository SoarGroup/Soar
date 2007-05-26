// $Id: Demo_ST_Bucket.C,v 1.3 2006/05/10 12:34:51 ddeutscher Exp $

// This is an ORTS file (c) Michael Buro, licensed under the GPL

// Speed analysis:
// check_direction() is the main bottleneck of the simple_astar() algorithm, 
// consuming about 80% of the time. The calls to check_vector() are responsible 
// to 80% of this, or about 2/3 of the PF time.
// Optimizations applied:
//   check_all_directions() is an improvement.
//   rewriting check_vector() using plain arrays is an improvement.

#include "SortsST_Bucket.h"

using namespace std;
using boost::array;
using boost::shared_array;

namespace SortsSimpleTerrain {
  
    Bucket::Bucket( const Vector<Loc> &points_, 
                    const Vector<Loc> &pointsN_,
                    const Vector<Loc> &pointsE_,
                    const Vector<Loc> &pointsS_,
                    const Vector<Loc> &pointsW_)
      : points(new Loc[points_.size()]),
        pointsN(new Loc[pointsN_.size()]), pointsE(new Loc[pointsE_.size()]),
        pointsS(new Loc[pointsS_.size()]), pointsW(new Loc[pointsW_.size()]),
        p_size(points_.size()),
        N_size(pointsN_.size()), E_size(pointsE_.size()), 
        S_size(pointsS_.size()), W_size(pointsW_.size()),
        pointsVector(points_)
    {

      copy(points_.begin(), points_.end(), points.get());
      copy(pointsN_.begin(), pointsN_.end(), pointsN.get());
      copy(pointsE_.begin(), pointsE_.end(), pointsE.get());
      copy(pointsS_.begin(), pointsS_.end(), pointsS.get());
      copy(pointsW_.begin(), pointsW_.end(), pointsW.get());
    }

    // We can use copies of the pointers since the arrays never change after
    // construction, and shared_ptr automatically handles reference counting.
    Bucket::Bucket(const Bucket &o)
      : center(o.center),
        points(o.points),
        pointsN(o.pointsN), pointsE(o.pointsE),
        pointsS(o.pointsS), pointsW(o.pointsW),
        p_size(o.p_size),
        N_size(o.N_size), E_size(o.E_size),
        S_size(o.S_size), W_size(o.W_size),
        pointsVector(o.pointsVector)
    {
    }

    Bucket& Bucket::operator=(const Bucket &o)
    {
      if( &o != this ) { // Check for self assignment
        p_size = o.p_size;
        points = o.points;
        N_size = o.N_size; E_size = o.E_size;
        S_size = o.S_size; W_size = o.W_size;
        pointsN = o.pointsN; pointsE = o.pointsE;
        pointsS = o.pointsS; pointsW = o.pointsW;
        center = o.center;
        pointsVector = o.pointsVector;
      }
      return *this;
    }


  /**
   * checks movement of this bucket in the given direction on the given map
   * is allawed, and stores the cost of that movement in cost
   * */
  bool Bucket::check_direction(const SortsSimpleMap<SizeCell> &map, sint4 d, sint4& cost)
  {
    switch (d){
    case 0: //E
      return check_vector(map, pointsE.get(), E_size, 1, 0, cost);
      break;

    case 1: //SE
      return check_vector(map, pointsS.get(), S_size, 1, 1, cost) && 
        check_vector(map, pointsE.get(), E_size, 1, 1, cost) &&
        check_vector(map, pointsS.get(), S_size, 0, 1, cost) && 
        check_vector(map, pointsE.get(), E_size, 1, 0, cost);
      break;

    case 2: //S
      return check_vector(map, pointsS.get(), S_size, 0, 1, cost);
      break;

    case 3: //SW
      return check_vector(map, pointsS.get(), S_size, -1, 1, cost) && 
        check_vector(map, pointsW.get(), W_size, -1, 1, cost) &&
        check_vector(map, pointsS.get(), S_size,  0, 1, cost) && 
        check_vector(map, pointsW.get(), W_size, -1, 0, cost);
      break;

    case 4: //W
      return check_vector(map, pointsW.get(), W_size, -1, 0, cost);
      break;

    case 5: //NW
      return check_vector(map, pointsN.get(), N_size, -1, -1, cost) && 
        check_vector(map, pointsW.get(), W_size, -1, -1, cost) &&
        check_vector(map, pointsN.get(), N_size,  0, -1, cost) && 
        check_vector(map, pointsW.get(), W_size, -1,  0, cost);
      break;

    case 6: //N
      return check_vector(map, pointsN.get(), N_size, 0, -1, cost);
      break;

    case 7: //NE
      return check_vector(map, pointsN.get(), N_size, 1, -1, cost) && 
        check_vector(map, pointsE.get(), E_size, 1, -1, cost) &&
        check_vector(map, pointsN.get(), N_size, 0, -1, cost) && 
        check_vector(map, pointsE.get(), E_size, 1,  0, cost);
      break;
    }

    return false;
  }

  void Bucket::check_all_directions(const SortsSimpleMap<SizeCell> &map, bitset<8> &free, array<sint4,8> &cost)
  {
    bool raw_freeE, raw_freeESE, raw_freeSSE, 
         raw_freeS, raw_freeSSW, raw_freeWSW, 
         raw_freeW, raw_freeWNW, raw_freeNNW, 
         raw_freeN, raw_freeNNE, raw_freeENE;
    sint4 raw_costE = 0, raw_costESE = 0, raw_costSSE = 0,
          raw_costS = 0, raw_costSSW = 0, raw_costWSW = 0, 
          raw_costW = 0, raw_costWNW = 0, raw_costNNW = 0, 
          raw_costN = 0, raw_costNNE = 0, raw_costENE = 0;

    // check raw directions:
    raw_freeE   = check_vector(map, pointsE.get(), E_size,  1,  0, raw_costE);   // pointsE moving E
    raw_freeESE = check_vector(map, pointsE.get(), E_size,  1,  1, raw_costESE); // pointsE moving SE
    raw_freeSSE = check_vector(map, pointsS.get(), S_size,  1,  1, raw_costSSE); // pointsS moving SE
    raw_freeS   = check_vector(map, pointsS.get(), S_size,  0,  1, raw_costS);   // pointsS moving S
    raw_freeSSW = check_vector(map, pointsS.get(), S_size, -1,  1, raw_costSSW); // pointsS moving SW
    raw_freeWSW = check_vector(map, pointsW.get(), W_size, -1,  1, raw_costWSW); // pointsW moving SW
    raw_freeW   = check_vector(map, pointsW.get(), W_size, -1,  0, raw_costW);   // pointsW moving W
    raw_freeWNW = check_vector(map, pointsW.get(), W_size, -1, -1, raw_costWNW); // pointsW moving NW
    raw_freeNNW = check_vector(map, pointsN.get(), N_size, -1, -1, raw_costNNW); // pointsN moving NW
    raw_freeN   = check_vector(map, pointsN.get(), N_size,  0, -1, raw_costN);   // pointsN moving N
    raw_freeNNE = check_vector(map, pointsN.get(), N_size,  1, -1, raw_costNNE); // pointsN moving NE
    raw_freeENE = check_vector(map, pointsE.get(), E_size,  1, -1, raw_costENE); // pointsE moving NE

    // combine conditions & costs
    // East:
    free[0] = raw_freeE;
    cost[0] = raw_costE;
    // South-East:
    free[1] = raw_freeSSE && raw_freeESE && raw_freeE && raw_freeS;
    cost[1] = raw_costSSE  + raw_costESE  + raw_costE  + raw_costS;
    // South:
    free[2] = raw_freeS;
    cost[2] = raw_costS;
    // South-West:
    free[3] = raw_freeSSW && raw_freeWSW && raw_freeS && raw_freeW;
    cost[3] = raw_costSSW  + raw_costWSW  + raw_costS  + raw_costW;
    // West:
    free[4] = raw_freeW;
    cost[4] = raw_costW;
    // North-West:
    free[5] = raw_freeNNW && raw_freeWNW && raw_freeN && raw_freeW;
    cost[5] = raw_costNNW  + raw_costWNW  + raw_costN  + raw_costW;
    // North:
    free[6] = raw_freeN;
    cost[6] = raw_costN;
    // North-East:
    free[7] = raw_freeNNE && raw_freeENE && raw_freeN && raw_freeE;
    cost[7] = raw_costNNE  + raw_costENE  + raw_costN  + raw_costE;

/*
//VERSION1, a little slower:
//--------------------------
    enum { E, ESE, SSE, S, SSW, WSW, W, WNW, NNW, N, NNE, ENE, TOTAL };
    bitset<TOTAL> raw_free;
    array<sint4, TOTAL> raw_cost = {0}; // Initialize all elements to 0

    // check raw directions:
    raw_free[E]   = check_vector(map, pointsE,  1,  0, raw_cost[E]);   // pointsE moving E
    raw_free[ESE] = check_vector(map, pointsE,  1,  1, raw_cost[ESE]); // pointsE moving SE
    raw_free[SSE] = check_vector(map, pointsS,  1,  1, raw_cost[SSE]); // pointsS moving SE
    raw_free[S]   = check_vector(map, pointsS,  0,  1, raw_cost[S]);   // pointsS moving S
    raw_free[SSW] = check_vector(map, pointsS, -1,  1, raw_cost[SSW]); // pointsS moving SW
    raw_free[WSW] = check_vector(map, pointsW, -1,  1, raw_cost[WSW]); // pointsW moving SW
    raw_free[W]   = check_vector(map, pointsW, -1,  0, raw_cost[W]);   // pointsW moving W
    raw_free[WNW] = check_vector(map, pointsW, -1, -1, raw_cost[WNW]); // pointsW moving NW
    raw_free[NNW] = check_vector(map, pointsN, -1, -1, raw_cost[NNW]); // pointsN moving NW
    raw_free[N]   = check_vector(map, pointsN,  0, -1, raw_cost[N]);   // pointsN moving N
    raw_free[NNE] = check_vector(map, pointsN,  1, -1, raw_cost[NNE]); // pointsN moving NE
    raw_free[ENE] = check_vector(map, pointsE,  1, -1, raw_cost[ENE]); // pointsE moving NE

    // combine conditions & costs
    // East:
    free[0] = raw_free[E];
    cost[0] = raw_cost[E];
    // South-East:
    free[1] = raw_free[SSE] && raw_free[ESE] && raw_free[E] && raw_free[S];
    cost[1] = raw_cost[SSE]  + raw_cost[ESE]  + raw_cost[E]  + raw_cost[S];
    // South:
    free[2] = raw_free[S];
    cost[2] = raw_cost[S];
    // South-West:
    free[3] = raw_free[SSW] && raw_free[WSW] && raw_free[S] && raw_free[W];
    cost[3] = raw_cost[SSW]  + raw_cost[WSW]  + raw_cost[S]  + raw_cost[W];
    // West:
    free[4] = raw_free[W];
    cost[4] = raw_cost[W];
    // North-West:
    free[5] = raw_free[NNW] && raw_free[WNW] && raw_free[N] && raw_free[W];
    cost[5] = raw_cost[NNW]  + raw_cost[WNW]  + raw_cost[N]  + raw_cost[W];
    // North:
    free[6] = raw_free[N];
    cost[6] = raw_cost[N];
    // North-East:
    free[7] = raw_free[NNE] && raw_free[ENE] && raw_free[N] && raw_free[E];
    cost[7] = raw_cost[NNE]  + raw_cost[ENE]  + raw_cost[N]  + raw_cost[E];*/
  }


  bool Bucket::check_vector(const SortsSimpleMap<SizeCell> &map,
			    const Vector<Loc>& points,
			    sint4 dx, sint4 dy, sint4 &cost)
  {
    FORALL (points, i){
      if (center.x+i->x+dx < 0
	  || center.x+i->x+dx >= map.get_w()
	  || center.y+i->y+dy < 0
	  || center.y+i->y+dy >= map.get_h() ) return false;
      if (map(center.x+i->x+dx,center.y+i->y+dy).max_size < 1) return false;
      cost += map(center.x+i->x+dx, center.y+i->y+dy).cost;
    }
    return true;
  }

  bool Bucket::check_vector(const SortsSimpleMap<SizeCell> &map,
			    Loc *points, size_t count,
			    sint4 dx, sint4 dy, sint4 &cost)
  {
    for( Loc *end=points+count; points != end ; points++ ) {
      sint4 x = center.x + points->x + dx;
      sint4 y = center.y + points->y + dy;
      if (x < 0 || x >= map.get_w() || y < 0 || y >= map.get_h() ) 
        return false;
      const SizeCell& sc = map(x,y);
      if (sc.max_size < 1)
        return false;
      cost += sc.cost;
    }
    return true;
  }


//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

  BucketFactory::BucketSamples BucketFactory::samples;

  // FIXME: UGLY FUNCTIONS

  template <class T>
  static void make_unique(Vector<T> &v)
  {
    typename  Vector<T>::iterator j;
    FORALL_DYN (v, i) {
      for (j = i+1; j != v.end();) {
	if (*i == *j) {
	  j = v.erase(j);
	  continue;
	}
	++j;
      }
    }
  }

  static void unique_x(Vector<Loc> &v)
  {
    Vector<Loc>::iterator j;
    FORALL_DYN (v, i){
      for (j = i+1; j != v.end();) {
	if (i->x == j->x) {
	  j = v.erase(j);
	  continue;
	}
	++j;
      }
    }
  }

  static void unique_y(Vector<Loc> &v)
  {
    Vector<Loc>::iterator j;
    FORALL_DYN (v, i) {
      for (j = i+1; j != v.end();) {
	if (i->y == j->y) {
	  j = v.erase(j);
	  continue;
	}
	++j;
      }
    }
  }

  Bucket BucketFactory::get_bucket(int r)
  {
    if (r < 0) r = 0;
    
    FIND(samples, i, r);
    if( i != samples.end() )
      return i->second;

    // Bresenham circle algorithm
    // retrieved from http://www.gamedev.net/reference/articles/article767.asp and 
    // modified
    int X = 0;
    int Y = 0;

    Vector<Loc> points; // points of the border of the bucket
    Vector<Loc> N; // points of the north face of the bucket
    Vector<Loc> E; // points of the north face of the bucket
    Vector<Loc> S; // points of the north face of the bucket
    Vector<Loc> W; // points of the north face of the bucket
  
    double d = 3 - 2*r;
    double x = 0;
    double y = r;

    while (x <= y) {
      X = (int)x;
      Y = (int)y;
      //Draw the 8 circle pixels

      points.push_back(Loc(+X, +Y)); // SE
      S.push_back(Loc(+X, +Y)); 
      E.push_back(Loc(+X, +Y)); 
    
      points.push_back(Loc(-X, +Y)); // SW
      S.push_back(Loc(-X, +Y)); 
      W.push_back(Loc(-X, +Y));
  
      points.push_back(Loc(+X, -Y)); // NE
      N.push_back(Loc(+X, -Y));
      E.push_back(Loc(+X, -Y));
  
      points.push_back(Loc(-X, -Y)); // NW
      N.push_back(Loc(-X, -Y));
      W.push_back(Loc(-X, -Y));
      
      points.push_back(Loc(+Y, +X)); // SE
      S.push_back(Loc(+Y, +X));
      E.push_back(Loc(+Y, +X));

      points.push_back(Loc(-Y, +X)); // SW
      S.push_back(Loc(-Y, +X));
      W.push_back(Loc(-Y, +X));

      points.push_back(Loc(+Y, -X)); // NE
      N.push_back(Loc(+Y, -X));
      E.push_back(Loc(+Y, -X));

      points.push_back(Loc(-Y, -X)); // NW
      N.push_back(Loc(-Y, -X));
      W.push_back(Loc(-Y, -X));

      if (d < 0) {
	d = d + (4 * x) + 6;
      } else {
	d = d + 4 * (x - y) + 10;
	y = y - 1;
      }

      x = x + 1;

    }

    make_unique(points);
    unique_x(N);
    unique_x(S);
    unique_y(E);
    unique_y(W);
  
    Bucket b(points, N, E, S, W);
    samples.insert(BucketSamples::value_type(r,b));
    return b;
  }
}
