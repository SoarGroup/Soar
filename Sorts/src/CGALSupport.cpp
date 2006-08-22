#include "CGALSupport.h"


bool doIntersect(CGALCircle* circle, CGALSegment* segment) {
  CGALComputeSquaredDistance sqdist;
  return (sqdist(*segment, circle->center()) <= circle->squared_radius());
}

bool doIntersect(CGALCircle* circle, CGALPoint* point) {
  CGALComputeSquaredDistance sqdist;
  return (sqdist(*point, circle->center()) <= circle->squared_radius());
}

bool doIntersect(CGALCircle* circle, CGALPolygon* polygon) {
  if (not polygon->has_on_unbounded_side(circle->center())) {
    // circle is centered in (or on edge of) poly-> intersect
    return true;
  }
  // circle is not centered in polygon: they intersect iff some edge of polygon
  // intersects the circle

  for (CGALPolygon::Edge_const_iterator it = polygon->edges_begin();
        it != polygon->edges_end();
        it++) {
    if (doIntersect(circle, &(*it))) {
      return true;
    }
  }
  return false;
}

bool doIntersect(CGALCircle* circle, CGALCircle* circle2) {
  CGALComputeSquaredDistance sqdist;
  return (sqrt(sqdist(circle2->center(), circle->center())) 
      <= (sqrt(circle->squared_radius()) + sqrt(circle2->squared_radius()))); 
}
