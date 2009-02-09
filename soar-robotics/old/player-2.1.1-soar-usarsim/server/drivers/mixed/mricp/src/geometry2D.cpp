/* Simple 2D geometric operations with points, poses, and lines.
 * Tim Bailey 2004.
 */

#include "geometry2D.h"
#include <cmath>
#include <cassert>

using namespace std;

namespace Geom2D {

#if 0
double pi_to_pi(double angle) 
// An alternative implementation that uses fmod() rather than while-loops.
{
	angle = fmod(angle, 2.*PI);
	if (angle < -PI)
		angle += 2.*PI;
	else if (angle > PI)
		angle -= 2.*PI;
	return angle;
}
#endif

double dist(const Point& p, const Point& q) 
// Distance between two points.
{ 
	return sqrt(dist_sqr(p,q)); 
}

Pose compute_relative_pose(const vector<Point>& a, const vector<Point>& b)
// Determine the relative Pose that best aligns two point-sets.
// INPUTS: a, b are two equal-length sets of points, such that a[i] aligns with b[i].
// OUTPUT: Best-fit alignment.
//
// Closed-form algorithm from Appendix C of:
// F. Lu and E. Milios. Robot pose estimation in unknown environments by matching
// 2D range scans. Journal of Intelligent and Robotic Systems, 18:249�275, 1997.
{
	assert(a.size() == b.size() && a.size() != 0);

	int n= a.size();
	double x1,x2,y1,y2,xx,yy,xy,yx;

	x1=x2=y1=y2=xx=yy=xy=yx=0.0;
	for (int i=0; i<n; ++i) { // calculate sums
		const Point& p1 = a[i];
		const Point& p2 = b[i];

		x1 += p1.x;
		x2 += p2.x;
		y1 += p1.y;
		y2 += p2.y;
		xx += p1.x*p2.x;
		yy += p1.y*p2.y;
		xy += p1.x*p2.y;
		yx += p1.y*p2.x;
	}

	double N = static_cast<double>(n);

	double Sxx = xx - x1*x2/N; // calculate S
	double Syy = yy - y1*y2/N;
	double Sxy = xy - x1*y2/N;
	double Syx = yx - y1*x2/N;

	double xm1 = x1/N; // calculate means
	double xm2 = x2/N; 
	double ym1 = y1/N; 
	double ym2 = y2/N; 

	double phi = atan2(Sxy-Syx, Sxx+Syy); 
	
	Pose pse; // calculate pose
	pse.p.x = xm2 - (xm1*cos(phi) - ym1*sin(phi));
	pse.p.y = ym2 - (xm1*sin(phi) + ym1*cos(phi));
	pse.phi = phi;

	return pse;
}

Transform2D::Transform2D(const Pose& ref) : base(ref) 
{
	c = cos(ref.phi);
	s = sin(ref.phi);
}

bool intersection_line_line (Point& p, const Line& l, const Line& m) 
// Compute the intersection point of two lines.
// Returns false for parallel lines.
{
	double gl, gm, bl, bm;
	bool lVert = true, mVert = true;

	// calculate gradients 
	if ((gl = l.second.x - l.first.x) != 0.0) {
		gl = (l.second.y - l.first.y)/gl;
		lVert = false;
	}
	if ((gm = m.second.x - m.first.x) != 0.0) {
		gm = (m.second.y - m.first.y)/gm;
		mVert = false;
	}

	if (lVert == mVert) { // check for parallelism 
		if (gl == gm)
			return false;
	}

	bl = l.first.y - gl*l.first.x; // calculate y intercepts 
	bm = m.first.y - gm*m.first.x;

	if (lVert) { // calculate intersection 
		p.x = l.first.x;
		p.y = gm*p.x + bm;
	}
	else if (mVert) {
		p.x = m.first.x;
		p.y = gl*p.x + bl;
	}
	else {
		p.x = (bm - bl)/(gl - gm);
		p.y = gm*p.x + bm;
	}

	return true;
}

double distance_line_point (const Line& lne, const Point& p)
// Note: distance is -ve if point is on left of line and +ve if it is on 
// the right (when looking from first to second). 
{
	Point v;
	v.x= lne.second.x - lne.first.x;
	v.y= lne.second.y - lne.first.y;

	return ((lne.second.y - p.y)*v.x 
		  - (lne.second.x - p.x)*v.y) 
		  / sqrt(v.x*v.x + v.y*v.y);
}

void intersection_line_point(Point& p, const Line& l, const Point& q) 
// Compute the perpendicular intersection from a point to a line
{
	Line m; 	// create line through q perpendicular to l
	m.first = q;
	m.second.x = q.x + (l.second.y - l.first.y);
	m.second.y = q.y - (l.second.x - l.first.x);

	bool not_parallel = intersection_line_line(p, l, m);
	//assert(not_parallel);
	if (not_parallel == 0)
		printf("\n Lines are parallel !!!");
}

} // namespace Geom2D
