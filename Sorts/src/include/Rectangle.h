#ifndef Rectangle_H
#define Rectangle_H

#include<iostream>

using namespace std;

class Rectangle {
public:
  Rectangle();
  Rectangle(int _xmin, int _xmax, int _ymin, int _ymax);

  void set(int _xmin, int _xmax, int _ymin, int _ymax);
  void collapse(int x, int y);
  void accomodate(int x, int y);

  bool intersects(const Rectangle& other);
  bool contains(int x, int y);
  bool contains(const Rectangle& r);
  int  area();

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
