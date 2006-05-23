#ifndef Point_H
#define Point_H

#include <math.h>

class Point {
public:
  Point() : x(0), y(0) {}
  Point(double _x, double _y) : x(_x), y(_y) {}
  Point(const Point& p) : x(p.x), y(p.y) {}

  Point roundToward(const Point& p) {
    Point newp;
    if (p.x <= x) {
      newp.x = floor(x);
    }
    else {
      newp.x = ceil(x);
    }
    if (p.y <= y) {
      newp.y = floor(y);
    }
    else {
      newp.y = ceil(y);
    }
    return newp;
  }

  double distSqTo(const Point& p) {
    double dx = x - p.x;
    double dy = y - p.y;
    return dx * dx + dy * dy;
  }

  double distTo(const Point& p) {
    return sqrt(distSqTo(p));
  }

  int intx() { return (int) x; }
  int inty() { return (int) y; }

  double x;
  double y;
};

#endif
