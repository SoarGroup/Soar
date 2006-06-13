#ifndef TerrainContour_H
#define TerrainContour_H

#include "bso_rational_nt.h"
#include <CGAL/Cartesian.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/enum.h>

#include <list>

#include "Rectangle.h"
#include "Point.h"

typedef CGAL::Cartesian<Number_type> CGALKernel;
typedef CGALKernel::Point_2          Point_2;
typedef CGAL::Polygon_2<CGALKernel>  Polygon;

using namespace std;

class TerrainContour {
public:
  TerrainContour(int x1, int y1, int x2, int y2, int x3, int y3, bool _open);
  TerrainContour(list<Point> cycle);

  // returns whether a certain segment should be part of this
  // contour
  bool segmentBelongs(int x1, int y1, int x2, int y2) const;
  // returns whether two contours meet
  bool canJoin(const TerrainContour& other);

  void addSegment(int x1, int y1, int x2, int y2);
  void join(const TerrainContour& other);

  bool intersectsCircle(int cx, int cy, int r) const;
  bool intersectsRectangle(int x1, int y1, int x2, int y2) const;
  bool intersectsContour(const TerrainContour& other) const;

  bool isOpen() const { return open; }

  Rectangle getBoundingBox() { return bbox; }

private: // functions
  void reconstruct();
  bool testReconstruct();

private:
  bool open;
  bool valid;
  list<Point_2> vertices;
  Polygon contour;

  Rectangle bbox;
};
#endif
