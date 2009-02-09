/* Nearest-neighbours algorithm for 2D point-sets.
 * Compute the single nearest-neighbour of a point, or the set of k-nearest neighbours.
 *
 * This is a very simple algorithm that is reasonably efficient for planar
 * points. However much faster algorithms are possible.
 *
 * Tim Bailey 2004.
 */

#ifndef NN_HEADER
#define NN_HEADER

#include "geometry2D.h"
#include <vector>

namespace Geom2D {

// Simple 2D k-nearest-neighbours search
//
class SweepSearch {
public:
	enum { NOT_FOUND = -1 };

	SweepSearch(const std::vector<Point> &p, double dmax);

	int query(const Point &q) const;
	std::vector<double>& query(const Point &q, std::vector<int> &idx);

private:	
	struct PointIdx { 
		PointIdx() {}
		PointIdx(const Point &p_, const int& i_) : p(p_), i(i_) {}
		Point p;
		int i; 
	};

	const double limit;
	std::vector<PointIdx> dataset;
	std::vector<double> nndists;

	bool is_nearer(double &d2min, int &idxmin, const Point &q, const PointIdx &pi) const;

	bool insert_neighbour(const Point &q, const PointIdx &pi, 
		std::vector<double> &nndists, std::vector<int> &idx);

	static bool yorder (const PointIdx& p, const PointIdx& q) {
		return p.p.y < q.p.y;
	}
};

}

#endif
