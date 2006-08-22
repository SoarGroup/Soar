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
#ifndef Rectangle_H
#define Rectangle_H

#include <iostream>
#include <list>
#include "Vec2d.h"

using namespace std;

class Rectangle {
public:
  Rectangle();
  Rectangle(double x, double y);
  Rectangle(double _xmin, double _xmax, double _ymin, double _ymax);
  Rectangle(double x, double y, double width, double height, bool ugly);
  Rectangle(const Rectangle& other);

  void set(double _xmin, double _xmax, double _ymin, double _ymax);
  void collapse(double x, double y);
  void accomodate(double x, double y);
  void accomodate(const Rectangle& other);

  bool intersects(const Rectangle& other);
  //bool intersects(Line& l);
  bool contains(double x, double y);
  bool contains(const Rectangle& r);
  double  area();

  double getWidth();
  double getHeight();
  Vec2d getCenterPoint();

  Vec2d getClosestEdgePoint(const Vec2d& from);

  list<pair<double, double> > getPointList();
  //Circle getCircumscribingCircle();

  Rectangle& operator=(const Rectangle& rhs);
  friend ostream& operator<<(ostream& os, const Rectangle& r);

  double xmin, xmax, ymin, ymax;
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
