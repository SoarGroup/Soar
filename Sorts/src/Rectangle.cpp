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
#include "Rectangle.h"
#include "math.h"
#include <iostream>

using namespace std;

#define CLASS_TOKEN "RECT"
#define DEBUG_OUTPUT false 
#include "OutputDefinitions.h"

Rectangle::Rectangle() : xmin(0), xmax(0), ymin(0), ymax(0) {}

Rectangle::Rectangle(int x, int y) : xmin(x), xmax(x), ymin(y), ymax(y) {}

Rectangle::Rectangle(int _xmin, int _xmax, int _ymin, int _ymax)
: xmin(_xmin), xmax(_xmax), ymin(_ymin), ymax(_ymax)
{ }

Rectangle::Rectangle(int x, int y, int width, int height, bool ugly) {
  xmin = x - width / 2;
  xmax = x + width / 2;
  ymin = y - height / 2;
  ymax = y + height / 2;
}

Rectangle::Rectangle(const Rectangle& other) 
: xmin(other.xmin), xmax(other.xmax), ymin(other.ymin), ymax(other.ymax)
{ }

void Rectangle::set(int _xmin, int _xmax, int _ymin, int _ymax) {
  xmin = _xmin; xmax = _xmax;
  ymin = _ymin; ymax = _ymax;
}

void Rectangle::collapse(int x, int y) {
  xmin = x; xmax = x;
  ymin = y; ymax = y;
}

void Rectangle::accomodate(int x, int y) {
  if (x < xmin) {
    xmin = x;
  }
  else if (x > xmax) {
    xmax = x;
  }
  if (y < ymin) {
    ymin = y;
  }
  else if (y > ymax) {
    ymax = y;
  }
}

void Rectangle::accomodate(const Rectangle& other) {
  dbg << "accomodating " << other << endl; 
      
  if (other.xmin < xmin) { xmin = other.xmin; }
  if (other.xmax > xmax) { xmax = other.xmax; }
  if (other.ymin < ymin) { ymin = other.ymin; }
  if (other.ymax > ymax) { ymax = other.ymax; }
}

bool Rectangle::intersects(const Rectangle& other) {
  bool x_intersect = false;
  // check the x dimension
  if (xmin <= other.xmax) {
    if (xmax >= other.xmin) x_intersect = true;
  }

  if (x_intersect) {
    // check the y dimension
    if (ymin <= other.ymax) {
      if (ymax >= other.ymin) return true;
    }
  }
  return false;
}

bool Rectangle::intersects(const Circle& c) {
  if (xmin <= c.x && c.x <= xmax) {
    return (ymin - c.r <= c.y && c.y <= ymax + c.r);
  }
  if (ymin <= c.y && c.y <= ymax) {
    return (xmin - c.r <= c.x && c.x <= xmax + c.r);
  }
  if (c.x < xmin) {
    if (c.y < ymin) {
      return c.contains(xmin, ymin);
    }
    return c.contains(xmin, ymax);
  }
  else { // c.x > xmax
    if (c.y < ymin) {
      return c.contains(xmax, ymin);
    }
    return c.contains(xmax, ymax);
  }
}

bool Rectangle::intersects(Line& l) {
  
  // cases where line completely away from box (most common)
  if ((l.a.x < xmin && l.b.x < xmin) ||
      (l.a.x > xmax && l.b.x > xmax) ||
      (l.a.y < ymin && l.b.y < ymin) ||
      (l.a.y > ymax && l.b.y > ymax)) {
    return false;
  }
  else if (l.a.x == l.b.x ||
           l.a.y == l.b.y) {
    // straight lines: if they pass the previous condition, they must
    // intersect (this is probably the common intersect case)
    return true;
  }
  // cases where endpoint is in the box
  else if (contains(l.a.x, l.a.y)) {
    return true;
  }
  else if (contains(l.b.x, l.b.y)) {
    return true;
  }
  else {
    // pretend the line is infinitely long, and determine whether or not
    // the box corners all lie on the same side of it.
    if (l.a.x > l.b.x) {
      coordinate temp = l.a;
      l.a = l.b;
      l.b = l.a;
    }

    int slope = (l.b.y - l.a.y)/(l.b.x - l.a.x);
    // can't div by 0: l.b.x == l.a.x handled above

    bool above[4];

    if (ymax > (l.a.y + (xmin-l.a.x)*slope)) {
      above[0] = true;
    }
    else {
      above[0] = false;
    }
                                          
    if (ymin > (l.a.y + (xmin-l.a.x)*slope)) {
      above[1] = true;
    }
    else {
      above[1] = false;
    }
    
    if (ymax > (l.a.y + (xmax-l.a.x)*slope)) {
      above[2] = true;
    }
    else {
      above[2] = false;
    }
    if (ymin > (l.a.y + (xmax-l.a.x)*slope)) {
      above[3] = true;
    }
    else {
      above[3] = false;
    }
    
    if (above[0] && above[1] && above[2] && above[3]) {
      return false;
    }
    else if (!above[0] && !above[1] && !above[2] && !above[3]) {
      return false;
    }
    return true;
  }
}

bool Rectangle::contains(int x, int y) {
  return xmin <= x && xmax >= x && ymin <= y && ymax >= y;
}

bool Rectangle::contains(const Rectangle& r) {
  return xmin <= r.xmin && xmax >= r.xmax && ymin <= r.ymin && ymax >= r.ymax;
}

int Rectangle::area() {
  return (xmax - xmin) * (ymax - ymin);
}

Circle Rectangle::getCircumscribingCircle() {
  int dx = (xmax - xmin) / 2;
  int dy = (ymax - ymin) / 2;
  double d  = sqrt((double)(dx * dx + dy * dy));
  return Circle(xmin + dx, ymin + dy, d);
}

Rectangle& Rectangle::operator=(const Rectangle& rhs) {
  xmin = rhs.xmin;
  xmax = rhs.xmax;
  ymin = rhs.ymin;
  ymax = rhs.ymax;
  return *this;
}

ostream& operator<<(ostream& os, const Rectangle& r) {
  os << "((" << r.xmin << "," << r.ymin 
     << "),(" << r.xmax << "," << r.ymax << "))";
  return os;
}

coordinate Rectangle::getClosestEdgePoint(coordinate& from) {
  // assume coordinate is not in box!
  coordinate result;
  if (from.x >= xmin and from.x <= xmax) {
    result.x = from.x;
    if (from.y < ymin) {
      result.y = ymin;
    }
    else {
      result.y = ymax;
    }
    return result;
  }
  
  if (from.y >= ymin and from.y <= ymax) {
    result.y = from.y;
    if (from.x < xmin) {
      result.x = xmin;
    }
    else {
      result.x = xmax;
    }
    return result;
  }
  
  if (from.y <= ymin and from.x <= xmin) {
    result.x = xmin;
    result.y = ymin;
    return result;
  }
  if (from.y >= ymax and from.x >= xmax) {
    result.x = xmax;
    result.y = ymax;
    return result;
  }
  if (from.y <= ymin and from.x >= xmax) {
    result.x = xmax;
    result.y = ymin;
    return result;
  }
  //if (from.y >= ymax and from.x <= xmin) {
    // implied
    result.x = xmin;
    result.y = ymax;
    return result;
  //}
}
  
int Rectangle::getWidth() {
  return xmax - xmin;
}

int Rectangle::getHeight() {
  return ymax - ymin;
}

void Rectangle::getCenterPoint(int& x, int& y) {
  x = (xmax + xmin) / 2; 
  y = (ymax + ymin) / 2; 
}
