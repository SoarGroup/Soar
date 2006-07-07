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
#ifndef SDLCanvasTrackedShape_h
#define SDLCanvasTrackedShape_h

#include "SDLCanvasShape.h"
#include "SDLCanvasStableLine.h"

class SDLCanvasTrackedShape : public SDLCanvasShape {
public:
  SDLCanvasTrackedShape (SDL_Surface* _surface, SDLCanvasSpace* _space)
    : SDLCanvasShape(_surface, _space)
  { 
    isStart = true;
    tline = NULL;
  }

  SDLCanvasTrackedShape
    ( SDL_Surface* surface, 
      SDLCanvasSpace* space, 
      bool _isStart,
      double cx,
      double cy,
      SDLCanvasStableLine* trackingLine,
      string label ) 
    : SDLCanvasShape(surface, space, cx, cy, label)
  {
    isStart = _isStart;
    tline = trackingLine;
    if (isStart) {
      trackingLine->moveTo(cx, cy);
    }
    else {
      trackingLine->moveEndPoint(cx, cy);
    }
  }

  virtual void moveTo(double x, double y) {
    SDLCanvasShape::moveTo(x, y);
    if (isStart) {
      tline->moveTo(x, y);
    }
    else {
      tline->moveEndPoint(x, y);
    }
  }

  virtual void translate(double dx, double dy) {
    SDLCanvasShape::translate(dx, dy);
    if (isStart) {
      tline->moveTo(getX(), getY());
    }
    else {
      tline->moveEndPoint(getX(), getY());
    }
  }

  SDLCanvasStableLine* getTrackingLine() {
    return tline;
  }

private:
  bool isStart;
  SDLCanvasStableLine* tline;
};

#endif
