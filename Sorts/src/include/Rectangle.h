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

#include "Circle.h"
#include "general.h"

using namespace std;

class Rectangle {
public:
  Rectangle();
  Rectangle(int x, int y);
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

  int getWidth();
  int getHeight();
  void getCenterPoint(int& x, int& y);

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
