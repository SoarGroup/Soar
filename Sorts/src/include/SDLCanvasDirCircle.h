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
#ifndef SDLCanvasDirCircle_h
#define SDLCanvasDirCircle_h

#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include "SDLCanvasCircle.h"

class SDLCanvasDirCircle : public SDLCanvasCircle {

public:
  SDLCanvasDirCircle(SDL_Surface* surface, SDLCanvasSpace* space)
    : SDLCanvasCircle(surface, space)
  {
    pointerLen = 0;
    point_dx = 0;
    point_dy = 0;
    color = SDLColor(255, 255, 255);
  }

  SDLCanvasDirCircle
    ( SDL_Surface* surface, 
      SDLCanvasSpace* space, 
      double cx, 
      double cy, 
      double radius,
      double angle,
      char* label) 
    : SDLCanvasCircle(surface, space, cx, cy, radius, label)
  {
    pointerLen = space->toCS(radius * 1.1);
    setAngle(angle);
    color = SDLColor(255, 255, 255);
  }

  void setAngle(double angle) {
    point_dx = (Uint16) (cos(angle) * pointerLen);
    point_dy = (Uint16) (sin(angle) * pointerLen);
  }

  virtual void draw() const {
    lineRGBA(
        getSurface(), 
        getCenterX(), 
        getCenterY(), 
        getCenterX() + point_dx, 
        getCenterY() + point_dy, 
        color.r,
        color.g,
        color.b,
        color.a);
    SDLCanvasCircle::draw();
  }

  void setPointerColor(Uint8 r, Uint8 g, Uint8 b) {
    color = SDLColor(r, g, b);
  }

private:
  Uint16 point_dx;
  Uint16 point_dy;
  Uint16 pointerLen;
  SDLColor color;
};

#endif
