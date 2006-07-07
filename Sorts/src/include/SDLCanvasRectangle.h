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
#ifndef SDLCanvasRectangle_h
#define SDLCanvasRectangle_h

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include "SDLCanvasSpace.h"
#include "SDLCanvasShape.h"

class SDLCanvasRectangle : public SDLCanvasShape {
public:
  SDLCanvasRectangle(
      SDL_Surface* surface,
      SDLCanvasSpace* space,
      double cx,
      double cy,
      double w,
      double h,
      double angle,
      char* label)
    : SDLCanvasShape(surface, space, cx, cy, label)
  {
    halfw = w / 2;
    halfh = h / 2;
    
    setAngle(angle);

    color = SDLColor(255, 255, 255);
  }

  void setAngle(double angle) {
    a = angle;
    double sinTheta = sin(angle);
    double cosTheta = cos(angle);

    double halfwCosTheta = cosTheta * halfw;
    double halfhCosTheta = cosTheta * halfh;
    double halfwSinTheta = sinTheta * halfw;
    double halfhSinTheta = sinTheta * halfh;

    double x[4], y[4];

    // (0,0) (1,1)
    //
    // (3,3) (2,2)

    x[0] = halfhSinTheta - halfwCosTheta;
    y[0] = -1 * (halfwSinTheta + halfhCosTheta);
    x[1] = halfwCosTheta + halfhSinTheta;
    y[1] = halfwSinTheta - halfhCosTheta;
    x[2] = halfwCosTheta - halfhSinTheta;
    y[2] = halfwSinTheta + halfhCosTheta;
    x[3] = -1 * (halfwCosTheta + halfhSinTheta);
    y[3] = halfhCosTheta - halfwSinTheta;

    SDLWorldPoint c = getCenter();
    for(int i = 0; i < 4; ++i) {
      vx[i] = getSpace()->toCS(c.x + x[i]);
      vy[i] = getSpace()->toCS(c.y + y[i]);
    }
  }

  virtual void draw() const {
    polygonRGBA(getSurface(), vx, vy, 4, color.r, color.g, color.b, color.a);
    Uint16 px = (vx[0] + vx[1]) / 2;
    Uint16 py = (vy[0] + vy[1]) / 2;
    lineRGBA(getSurface(), getCenterX(), getCenterY(), px, py, color.r, color.g, color.b, color.a);
    SDLCanvasShape::draw();
  }

  void setShapeColor(Uint8 r, Uint8 g, Uint8 b) {
    color = SDLColor(r, g, b);
  }
  
  void setShapeColor(const SDLColor& c) {
    color = SDLColor(c.r, c.g, c.b);
  }

  SDLColor getShapeColor() {
    return color;
  }

  virtual void moveTo(double x, double y) {
    SDLWorldPoint wp = getCenter();
    translate(x - wp.x, y - wp.y);
    SDLCanvasShape::moveTo(x, y);
  }

  virtual void translate(double dx, double dy) {
    Sint16 cdx = getSpace()->toCS(dx);
    Sint16 cdy = getSpace()->toCS(dy);
    for(int i = 0; i < 4; ++i) {
      vx[i] += cdx;
      vy[i] += cdy;
    }
    SDLCanvasShape::translate(dx, dy);
  }

  void resize(double w, double h) {
    halfw = w / 2;
    halfh = h / 2;
    setAngle(a);
  }

private:
  double halfw;
  double halfh;
  double a;
  Sint16 vx[4];
  Sint16 vy[4];

  SDLColor color;
};

#endif
