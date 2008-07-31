/* Iterated closest point (ICP) algorithm.
 * Tim Bailey 2004.
 */
#include "icp.h"
#include <cassert>
#include <iostream>
using namespace std;

namespace Geom2D 
	{
	ICP::ICP()
		{
			index.push_back(0);
			index.push_back(0);			
		};
	ICP::~ICP()
		{
			a.clear();
			b.clear();
			index.clear();
			ref.clear();
		}

Pose ICP::align(vector<Point> reference, vector<Point> obs,Pose init, double gate, int nits, bool interp)
{
	ref = reference;
	nn = new SweepSearch(reference, gate);
	Pose pse = init; 
	double gate_sqr = sqr(gate); 
	int size_obs = obs.size();

	 // used if interp == true
	while (nits-- > 0) 
	{
		Transform2D tr(pse);
		a.clear();
		b.clear();
		// For each point in obs, find its NN in ref
		for (int i = 0; i < size_obs; ++i) 
		{
			Point p = obs[i];
			tr.transform_to_global(p); // transform obs[i] to estimated ref coord-frame

			Point q;
			// simple ICP
			if (interp == false) 
			{
				int idx = nn->query(p);
				if (idx == SweepSearch::NOT_FOUND)
					continue;

				q = ref[idx];
			}
			// ICP with interpolation between 2 nearest points in ref
			else
			{
				(void) nn->query(p, index);
				assert(index.size() == 2);
				if (index[1] == SweepSearch::NOT_FOUND)
					continue;

				Line lne;
				lne.first  = ref[index[0]];
				lne.second = ref[index[1]];
				intersection_line_point(q, lne, p);
			}
			if (dist_sqr(p,q) < gate_sqr) // check if NN is close enough 
			{
				a.push_back(obs[i]);
				b.push_back(q);
			}
		}
		//cout<<"\nObs size:"<<obs.size()<<" Ref:"<<ref.size()<<" Paired set size:"<<a.size();
		//If Less than half of the Observation is paired => not good alignment
		if( a.size() < obs.size()/2.0)
		{
			cout<<"\n Detected Possible misalignment --- Skipping this Laser Set! Gate is:"<<gate;;
			pse.p.x = -1 ; pse.p.y=-1 ; pse.phi = -1;
			break;
		}
		if (a.size() > 2)
			pse = compute_relative_pose(a, b); // new iteration result
	}
	delete nn;
	return pse;
}

} // namespace Geom2D
