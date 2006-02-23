#include "Rectangle.h"
#include <iostream>

using namespace std;

Rectangle::Rectangle() : xmin(0), xmax(0), ymin(0), ymax(0) {}

Rectangle::Rectangle(int _xmin, int _xmax, int _ymin, int _ymax)
: xmin(_xmin), xmax(_xmax), ymin(_ymin), ymax(_ymax)
{}

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

bool Rectangle::intersects(const Rectangle& other) {
  bool x_intersect = false;
  // check the x dimension
  if (xmin < other.xmin) {
    if (xmax > other.xmin) x_intersect = true;
  }
  else {
    if (other.xmax > xmin) x_intersect = true;
  }

  if (x_intersect) {
    // check the y dimension
    if (ymin < other.ymin) {
      if (ymax > other.ymin) return true;
    }
    else {
      if (other.ymax > ymin) return true;
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

ostream& operator<<(ostream& os, const Rectangle& r) {
  os << "((" << r.xmin << "," << r.ymin 
     << "),(" << r.xmax << "," << r.ymax << "))";
  return os;
}
