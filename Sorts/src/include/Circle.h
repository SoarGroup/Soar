#ifndef Circle_H
#define Circle_H

using namespace std;

// this class was originally for calculating weapon attack ranges
class Circle {
public:
  Circle() : x(0), y(0), r(0), rsq(0) {}
  Circle(double _x, double _y, double _r)
  : x(_x), y(_y), r(_r)
  { rsq = r * r; }

  bool contains(double px, double py) {
    double dx = px - x;
    double dy = py - y;
    return (dx * dx + dy * dy <= rsq);
  }


  bool intersects(const Circle& other) {
    double dx = other.x - x;
    double dy = other.y - y;
    double rs = other.r + r;
    return (dx * dx + dy * dy <= rs * rs);
  }

public:
  double x, y, r, rsq;
};

#endif

