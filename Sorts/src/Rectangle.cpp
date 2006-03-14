#include "Rectangle.h"
#include <iostream>

using namespace std;

Rectangle::Rectangle() : xmin(0), xmax(0), ymin(0), ymax(0) {}

Rectangle::Rectangle(int _xmin, int _xmax, int _ymin, int _ymax)
: xmin(_xmin), xmax(_xmax), ymin(_ymin), ymax(_ymax)
{ }

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

bool Rectangle::contains(int x, int y) {
  return xmin <= x && xmax >= x && ymin <= y && ymax >= y;
}

bool Rectangle::contains(const Rectangle& r) {
  return xmin <= r.xmin && xmax >= r.xmax && ymin <= r.ymin && ymax >= r.ymax;
}

int Rectangle::area() {
  return (xmax - xmin) * (ymax - ymin);
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
