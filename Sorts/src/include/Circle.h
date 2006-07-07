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

  bool contains(double px, double py) const {
    double dx = px - x;
    double dy = py - y;
    return (dx * dx + dy * dy <= rsq);
  }


  bool intersects(const Circle& other) const {
    double dx = other.x - x;
    double dy = other.y - y;
    double rs = other.r + r;
    return (dx * dx + dy * dy <= rs * rs);
  }

public:
  double x, y, r, rsq;
};

#endif

