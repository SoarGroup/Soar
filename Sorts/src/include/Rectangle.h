#ifndef Rectangle_H
#define Rectangle_H

class Rectangle {
public:
  Rectangle() : xmin(0), xmax(0), ymin(0), ymax(0) {}

  Rectangle(int _xmin, int _xmax, int _ymin, int _ymax) 
  : xmin(_xmin), xmax(_xmax), ymin(_ymin), ymax(_ymax)
  {}

  void set(int _xmin, int _xmax, int _ymin, int _ymax) {
    xmin = _xmin; xmax = _xmax;
    ymin = _ymin; ymax = _ymax;
  }

  void collapse(int x, int y) {
    xmin = x; xmax = x;
    ymin = y; ymax = y;
  }

  bool intersects(const Rectangle& other) {
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

  bool contains(int x, int y) {
    return xmin <= x && xmax >= x && ymin <= y && ymax >= y;
  }

  bool contains(const Rectangle& r) {
    return xmin <= r.xmin && xmax >= r.xmax && ymin <= r.ymin && ymax >= r.ymax;
  }

  void accomodate(int x, int y) {
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

  int xmin, xmax, ymin, ymax;

};

#endif
