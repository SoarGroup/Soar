#include "CGALSupport.h"
#define CLASS_TOKEN "CGALSUPP"
#define DEBUG_OUTPUT true 
#include "OutputDefinitionsUnique.h"


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

CGALPoint constrainEndpoint(CGALSegment* segment, CGALPolygon* poly) {
  // segment is a segment beginning in the polygon, ending either inside or
  // outside of the polygon. If segment ends inside poly, return segment
  // endpoint, otherwise return intersection of segment with polygon

  // if multiple intersections exist, return one of them arbitrarily

  assert(poly->has_on_bounded_side(segment->source()));

  if (poly->has_on_bounded_side(segment->target())) {
    return segment->target();
  }
  else {
    for (CGALPolygon::Edge_const_iterator it = poly->edges_begin();
          it != poly->edges_end();
          it++) {
      if (do_intersect(*it, *segment)) {
        CGALPoint intersectionPoint;
        if (CGAL::assign(intersectionPoint, intersection(*it, *segment))) {
          return intersectionPoint;
        }
        else {
          assert(false);
        }
      }
    }
  }
  assert(false);
  msg << "ERROR: logical problem in constrainEndpoint().\n";
  return CGALPoint(0,0);
}
