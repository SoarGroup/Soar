/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
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
