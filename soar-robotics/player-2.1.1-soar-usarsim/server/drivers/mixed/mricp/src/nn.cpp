/* K-nearest-neighbours for 2D point-sets.
 * Tim Bailey 2004.
 */
#include "nn.h"
#include "geometry2D.h"
#include <algorithm>
#include <cmath>
#include <cassert>

using namespace Geom2D;
using namespace std;
	
SweepSearch::SweepSearch(const vector<Point> &p, double lim) : limit(lim)
// Input: set of points, p, and maximum search distance, lim.
// Function: create data structure for nearest-neighbour search.
{
					// store points and their order
	dataset.reserve(p.size());
	for (int i=0; i < (int)p.size(); ++i)
		dataset.push_back(PointIdx(p[i], i));

	// sort points
	sort(dataset.begin(), dataset.end(), yorder);
}

int SweepSearch::query(const Point& q) const
// Input: query point.
// Output: index of nearest-neighbour.
{
	double d2min = sqr(limit);
	int idxmin = NOT_FOUND;

					// get upper bound
	PointIdx qi(q, 0);
	vector<PointIdx>::const_iterator upper = 
		upper_bound(dataset.begin(), dataset.end(), qi, yorder);
	vector<PointIdx>::const_iterator i; 

					// search backwards
	double min_y = q.y - limit;
	for (i = upper-1; i >= dataset.begin() && i->p.y > min_y; --i) 
		if (is_nearer(d2min, idxmin, q, *i))
			min_y = q.y - sqrt(d2min);

					// search forward
	double max_y = q.y+sqrt(d2min);
	for (i = upper; i < dataset.end() && i->p.y < max_y; ++i) 
		if (is_nearer(d2min, idxmin, q, *i))
			max_y = q.y+sqrt(d2min);

	return idxmin;
}

inline 
bool SweepSearch::is_nearer(double &d2min, int &idxmin, const Point &q, const PointIdx &pi) const
// Check whether new point is closer than previous points. If so, update d2min and idxmin.
{
	double d2 = dist_sqr(q, pi.p);
	if (d2 < d2min) {
		d2min = d2;
		idxmin = pi.i;
		return true;
	}
	return false;
}

std::vector<double>& SweepSearch::query(const Point &q, std::vector<int> &idx) 
// Input: query point and vector<int> of size k (number of neighbours)
// Output: vector<int> indices of nearest-neighbours.
// Returns: set of distances to k nn's.
{
					// can't have more nn's than reference points
	assert(idx.size() > 0);
	int size = idx.size();
	if ((int)dataset.size() < size) {
		size = dataset.size();
		idx.resize(size);
	}
	if ((int)nndists.size() != size)
		nndists.resize(size);

					// initialise set
	for (int j=0; j<size; ++j) {
		idx[j] = NOT_FOUND;
		nndists[j] = sqr(limit);
	}
					// get upper bound
	PointIdx qi(q, 0);
	vector<PointIdx>::const_iterator di = 
		upper_bound(dataset.begin(), dataset.end(), qi, yorder);
	vector<PointIdx>::const_iterator i;

					// sweep forwards
	double max_y = q.y + limit;
	for (i = di; i < dataset.end() && i->p.y < max_y; ++i) 	
		if (insert_neighbour(q, *i, nndists, idx))
			max_y = q.y + sqrt(nndists[size-1]);

					// sweep backwards
	double min_y = q.y - sqrt(nndists[size-1]);
	for (i = di-1; i >= dataset.begin() && i->p.y > min_y; --i) 
		if (insert_neighbour(q, *i, nndists, idx))
			min_y = q.y - sqrt(nndists[size-1]);

					// convert square distances to distances
	{ for (int i=0; i<size; ++i)
		nndists[i] = sqrt(nndists[i]);
	}
	return nndists;
}

bool SweepSearch::insert_neighbour(const Point &q, const PointIdx &pi, 
		std::vector<double> &nndists, std::vector<int> &idx)
// Check if new point is closer than any of the existing near-neighbours. If so,
// place the new point into near-neighbour set (in ascending order).
{
	const int size = nndists.size();
	double d2 = dist_sqr(q, pi.p);

	if (d2 >= nndists[size-1]) // no improvement possible
		return false;

	int i;
	for (i=0; d2 >= nndists[i]; ++i) // find location for new neighbour
		;

	for (int j = size-1; j > i; --j) { // shuffle in order
		nndists[j] = nndists[j-1];
		idx[j] = idx[j-1];
	}
	nndists[i] = d2;
	idx[i] = pi.i;

	return true;
}
