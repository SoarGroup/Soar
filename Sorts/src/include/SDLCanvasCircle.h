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
#ifndef SDLCanvasCircle_h
#define SDLCanvasCircle_h

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include "SDLCanvasShape.h"

class SDLCanvasCircle : public SDLCanvasShape {
public:
  SDLCanvasCircle(SDL_Surface* surface, SDLCanvasSpace* space)
    : SDLCanvasShape(surface, space)
  {
    color = SDLColor(255, 255, 255);
  }

  SDLCanvasCircle
    ( SDL_Surface* surface, 
      SDLCanvasSpace* space, 
      double cx, 
      double cy, 
      double radius,
      char* label) 
    : SDLCanvasShape(surface, space, cx, cy, label)
  {
    r = space->toCS(radius);
    color = SDLColor(255, 255, 255);
  }

  virtual ~SDLCanvasCircle() { }

  virtual void draw() const {
    circleRGBA(getSurface(), getCenterX(), getCenterY(), r, color.r, color.g, color.b, color.a);
    SDLCanvasShape::draw();
  }

  SDLColor getShapeColor() {
    return color;
  }

  void setShapeColor(Uint8 r, Uint8 g, Uint8 b) {
    color = SDLColor(r, g, b);
  }

  void setShapeColor(const SDLColor& c) {
    color = SDLColor(c.r, c.g, c.b);
  }

  void setRadius(double radius) {
    r = getSpace()->toCS(radius);
  }

private:
  Uint16 r;
  SDLColor color;
};

#endif
