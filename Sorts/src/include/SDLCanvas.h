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
#ifndef SDLCanvas_h
#define SDLCanvas_h

#ifndef NO_CANVAS_COMPILED

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include <set>

#include "SDLCanvasSpace.h"
#include "SDLCanvasShape.h"
#include "SDLCanvasRectangle.h"
#include "SDLCanvasCircle.h"
#include "SDLCanvasDirCircle.h"
#include "SDLCanvasTrackedShape.h"
#include "SDLCanvasCompound.h"
#include "SDLCanvasStableLine.h"

struct TempShape {
  SDLCanvasShape* shape;
  int count;
};

class SDLCanvas {
public:
  SDLCanvas();
  ~SDLCanvas(); 

  int init(double worldWidth, double worldHeight, double scale);
  int redraw();

  SDLCanvasCircle*    makeCircle(double cx, double cy, double r);
  SDLCanvasDirCircle* makeDirCircle(double cx, double cy, double r, double a);
  SDLCanvasRectangle* makeRectangle(double cx, double cy, double w, double h, double a);
  SDLCanvasCompound*  makeCompound(double cx, double cy);
  SDLCanvasStableLine* makeLine(double x1, double y1, double x2, double y2);
  SDLCanvasTrackedShape* makeTracker(double sx, double sy, double dx, double dy, bool isStart);


  SDLCanvasCircle*    makeTempCircle(double cx, double cy, double r, int t);
  SDLCanvasDirCircle* makeTempDirCircle(double cx, double cy, double r, double a, int t);
  SDLCanvasRectangle* makeTempRectangle(double cx, double cy, double w, double h, double a, int t);

  void remove(SDLCanvasShape* shape);


  void clear();
  bool initted();

private:
  list<SDLCanvasShape*> shapes;
  list<TempShape> temps;
  SDLCanvasSpace space;
  SDL_Surface* screen;

  double worldw, worldh;
  Uint16 canvasw, canvash;
};

#endif
#endif
