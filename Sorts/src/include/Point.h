#ifndef Point_H
#define Point_H

#include <math.h>

class Point {
public:
  Point() : x(0), y(0) {}
  Point(double _x, double _y) : x(_x), y(_y) {}
  Point(const Point& p) : x(p.x), y(p.y) {}

  Point roundToward(const Point& p) const {
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

  double distSqTo(const Point& p) const {
    double dx = x - p.x;
    double dy = y - p.y;
    return dx * dx + dy * dy;
  }

  double distTo(const Point& p) const {
    return sqrt(distSqTo(p));
  }

  bool operator<(const Point& rhs) const {
    if (x != rhs.x) {
      return x < rhs.x;
    }
    return y < rhs.y;
  }

  int intx() const { return (int) x; }
  int inty() const { return (int) y; }

  double x;
  double y;
};

#endif
