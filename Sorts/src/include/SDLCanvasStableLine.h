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
#ifndef SDLCanvasStableLine_h
#define SDLCanvasStableLine_h

#include "SDLCanvasShape.h"

class SDLCanvasStableLine : public SDLCanvasShape {
public:
  SDLCanvasStableLine
    ( SDL_Surface* surface,
      SDLCanvasSpace* space,
      double _x1,
      double _y1,
      double _x2,
      double _y2 )
    : SDLCanvasShape(surface, space, _x1, _y1, "")
  {
    x2 = _x2;
    y2 = _y2;
    cx2 = space->toCS(x2);
    cy2 = space->toCS(y2);
    lineColor = SDLColor(255, 255, 255);
  }

  virtual void draw() const {
    lineRGBA(getSurface(), getCenterX(), getCenterY(), cx2, cy2, lineColor.r, lineColor.g, lineColor.b, lineColor.a);
    SDLCanvasShape::draw();
  }

  void moveEndPoint(double _x2, double _y2) {
    x2 = _x2;
    y2 = _y2;
    cx2 = getSpace()->toCS(x2);
    cy2 = getSpace()->toCS(y2);
  }

  void setLineColor(Uint8 r, Uint8 g, Uint8 b) {
    lineColor = SDLColor(r, g, b);
  }
  void setShapeColor(Uint8 r, Uint8 g, Uint8 b) {
    lineColor = SDLColor(r,g,b);
  }

private:
  double x2;
  double y2;
  Uint16 cx2;
  Uint16 cy2;
  SDLColor lineColor;
};

#endif
