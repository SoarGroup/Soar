#ifndef CGALSupport
#define CGALSupport

//#include "bso_rational_nt.h"
#include <CGAL/Cartesian.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/enum.h>
#include <CGAL/basic.h>
#include <CGAL/MP_Float.h>
#include <CGAL/Quotient.h>
#include <CGAL/Polytope_distance_d.h>
#include <CGAL/Optimisation_d_traits_2.h>
#include <CGAL/box_intersection_d.h>
#include <CGAL/Bbox_2.h>

#include <iostream>
using namespace std;

//typedef CGAL::Cartesian<Number_type> CGALKernel;
typedef CGAL::Cartesian<double> CGALKernel;
typedef CGALKernel::Point_2 CGALPoint;
typedef CGAL::Polygon_2<CGALKernel>  CGALPolygon;
typedef CGALKernel::Circle_2 CGALCircle;
typedef CGALKernel::Direction_2 CGALDirection;
typedef CGAL::Segment_2<CGALKernel> CGALSegment;
typedef CGAL::Aff_transformation_2<CGALKernel> CGALTransformation;
typedef CGAL::Ray_2<CGALKernel> CGALRay;
typedef CGAL::Vector_2<CGALKernel> CGALVector;
typedef CGAL::Bbox_2  CGALBBox;
typedef CGALKernel::Compute_squared_distance_2 CGALComputeSquaredDistance;
typedef CGAL::Polytope_distance_d<CGAL::Optimisation_d_traits_2<CGALKernel> > CGALPolytopeDistance;
typedef CGAL::Box_intersection_d::Box_d<double,2> CGALBox;

/*ostream& operator << (ostream& os, const CGALPoint& c) {
   return os<< c.x() << "," << c.y();
}*/


// SIN_PI_4 == sin(pi/4)
#define SIN_PI_4 .7071
#define COS_PI_4 SIN_PI_4
#define SIN_PI_8 .3827
#define COS_PI_8 .9239

#define SIN_3PI_8 COS_PI_8
#define COS_3PI_8 SIN_PI_8
#define SIN_5PI_8 COS_PI_8
#define COS_5PI_8 -1*SIN_PI_8
#define SIN_7PI_8 SIN_PI_8
#define COS_7PI_8 -1*COS_PI_8
#define SIN_9PI_8 -1*SIN_PI_8
#define COS_9PI_8 -1*COS_PI_8
#define SIN_11PI_8 -1*COS_PI_8
#define COS_11PI_8 -1*SIN_PI_8
#define SIN_13PI_8 -1*COS_PI_8
#define COS_13PI_8 SIN_PI_8
#define SIN_15PI_8 -1*SIN_PI_8
#define COS_15PI_8 COS_PI_8

bool doIntersect(CGALCircle* circle, CGALSegment* segment);
bool doIntersect(CGALCircle* circle, CGALPoint* point);
bool doIntersect(CGALCircle* circle, CGALPolygon* polygon);
bool doIntersect(CGALCircle* circle, CGALCircle* circle2);
bool doIntersect(CGALSegment* segment, CGALPolygon* poly);

CGALPoint constrainEndpoint(CGALSegment* segment, CGALPolygon* poly);
CGALSegment shrinkSegment(CGALSegment* segment, double amount, bool fromSource);
CGALPolygon* newPolygonFromSegment(CGALSegment seg);

#endif 
