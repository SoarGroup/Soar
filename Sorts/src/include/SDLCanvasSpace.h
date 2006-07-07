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
#ifndef SDLCanvasSpace_h
#define SDLCanvasSpace_h

#include <SDL/SDL.h>
#include <iostream>

#include "SDLCanvasDefs.h"

using namespace std;

class SDLCanvasSpace {
public:
  SDLCanvasSpace() {
    scaleToWorld = 1;
    scaleToCanvas = 1;
  }

  SDLCanvasSpace(const SDLCanvasSpace& other) {
    scaleToWorld = other.scaleToWorld;
    scaleToCanvas = other.scaleToCanvas;
  }

  SDLCanvasSpace(double scale) {
    scaleToWorld = scale;
    scaleToCanvas = 1 / scale;
  }

  SDLCanvasSpace(double worldUnit, Uint16 canvasUnit)
  {
    scaleToWorld = worldUnit / canvasUnit;
    scaleToCanvas = canvasUnit / worldUnit;
  }

  Sint16 toCS(double l) {
    return (Uint16) floor(scaleToCanvas * l);
  }

  double toWS(Sint16 l) {
    return scaleToWorld * l;
  }

  SDLCanvasPoint toCS(double x, double y) const {
    return SDLCanvasPoint(
        (Uint16) floor(scaleToCanvas * x),
        (Uint16) floor(scaleToCanvas * y));
  }

  SDLCanvasPoint toCS(const SDLWorldPoint& wp) {
    return toCS(wp.x, wp.y);
  }

  SDLWorldPoint toWS(Sint16 x, Sint16 y) {
    return SDLWorldPoint(scaleToWorld * x, scaleToWorld * y);
  }

  SDLWorldPoint toWS(const SDLCanvasPoint& cp) {
    return toWS(cp.x, cp.y);
  }

private:
  double scaleToWorld;
  double scaleToCanvas;
};

#endif
