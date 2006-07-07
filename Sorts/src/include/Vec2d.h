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
#ifndef Vec2d_H
#define Vec2d_H

#include <assert.h>
#include <math.h>

using namespace std;

class Vec2d {
public:
  Vec2d() : i(0), j(0) { }

  Vec2d(double _i, double _j) : i(_i), j(_j) { }

  Vec2d(const Vec2d& v) : i(v.i), j(v.j) { }

  Vec2d(const Vec2d& dir, double mag) {
    double scale = mag / dir.mag();
    i = scale * dir.i;
    j = scale * dir.j;
  }

  double mag() const {
    return sqrt(i * i + j * j);
  }

  double magSq() const {
    return i * i + j * j;
  }

  Vec2d operator+(const Vec2d& v) const {
    return Vec2d(i + v.i, j + v.j);
  }

  void operator+=(const Vec2d& rhs) {
    i += rhs.i;
    j += rhs.j;
  }

  Vec2d operator-(const Vec2d& v) const {
    return Vec2d(i - v.i, j - v.j);
  }

  void operator-=(const Vec2d& rhs) {
    i -= rhs.i;
    j -= rhs.j;
  }

  double operator*(const Vec2d& v) const {
    return i * v.i + j * v.j;
  }
  
  Vec2d norm() const {
    double m = mag();
    return Vec2d(i / m, j / m);
  }

  Vec2d inv() const {
    return Vec2d(-1 * i, -1 * j);
  }

  double angleBetween(const Vec2d& v) const {
    return acos(operator*(v) / sqrt(magSq() * v.magSq()));
  }

  Vec2d roundToward(const Vec2d& v) const {
    Vec2d intVec;
    if (v.i < i) {
      intVec.i = floor(i);
    }
    else {
      intVec.i = ceil(i);
    }
    if (v.j < j) {
      intVec.j = floor(j);
    }
    else {
      intVec.j = ceil(j);
    }
    return intVec;
  }

  double operator()(int d) const {
    switch (d) {
      case 0:
        return i;
      case 1:
        return j;
      default:
        assert(false);
        return i;
    }
  }

  void set(int d, double v) {
    switch (d) {
      case 0:
        i = v;
        break;
      case 1:
        j = v;
        break;
      default:
        assert(false);
    }
  }

private:
  double i;
  double j;
};

#endif
