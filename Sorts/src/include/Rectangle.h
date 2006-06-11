#ifndef Rectangle_H
#define Rectangle_H

#include <iostream>

#include "Circle.h"
#include "general.h"

using namespace std;

class Rectangle {
public:
  Rectangle();
  Rectangle(int _xmin, int _xmax, int _ymin, int _ymax);
  Rectangle(int x, int y, int width, int height, bool ugly);
  Rectangle(const Rectangle& other);

  void set(int _xmin, int _xmax, int _ymin, int _ymax);
  void collapse(int x, int y);
  void accomodate(int x, int y);
  void accomodate(const Rectangle& other);

  bool intersects(const Rectangle& other);
  bool intersects(const Circle& c);
  bool intersects(Line& l);
  bool contains(int x, int y);
  bool contains(const Rectangle& r);
  int  area();

  coordinate getClosestEdgePoint(coordinate& from);

  Circle getCircumscribingCircle();

  Rectangle& operator=(const Rectangle& rhs);
  friend ostream& operator<<(ostream& os, const Rectangle& r);

  int xmin, xmax, ymin, ymax;
};

ostream& operator<<(ostream& os, const Rectangle& r);

/*
ostream& operator<<(ostream& os, const Rectangle& r) {
  os << "((" << r.xmin << "," << r.ymin 
     << "),(" << r.xmax << "," << r.ymax << "))";
  return os;
}
*/

#endif
