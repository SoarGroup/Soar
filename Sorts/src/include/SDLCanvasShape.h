#ifndef SDLCanvasShape_h
#define SDLCanvasShape_h

#include <list>

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include "SDLCanvasDefs.h"
#include "SDLCanvasSpace.h"

using namespace std;

class SDLCanvasShape {
public:
  SDLCanvasShape (SDL_Surface* _surface, SDLCanvasSpace* _space)
    : surface(_surface), space(_space), center(0, 0), wCenter(0, 0), label("")
  {
    trace = false;
    labelColor = SDLColor(255, 255, 255);
    traceColor = SDLColor(150, 150, 150);
    path.push_back(center);
  }

  SDLCanvasShape
    ( SDL_Surface* _surface, 
      SDLCanvasSpace* _space, 
      double cx, 
      double cy, 
      string _label ) 
    : surface(_surface), 
      space(_space), 
      center(space->toCS(cx, cy)), 
      wCenter(cx, cy),
      label(_label) 
  {
    trace = false;
    labelColor = SDLColor(255, 255, 255);
    traceColor = SDLColor(150, 150, 150);
    path.push_back(center);
  }

  virtual ~SDLCanvasShape() { }

  virtual void draw() const {
    // draw label
    stringRGBA(surface, center.x, center.y, label.c_str(), 
               labelColor.r, labelColor.g, labelColor.b, labelColor.a);
    if (trace) {
      list<SDLCanvasPoint>::const_iterator trail = path.begin();
      list<SDLCanvasPoint>::const_iterator i = trail;
      ++i;
      for(; i != path.end(); ++i) {
        lineRGBA(surface, trail->x, trail->y, i->x, i->y, traceColor.r, traceColor.g, traceColor.b, traceColor.a);
      }
    }
  }

  virtual void moveTo(double x, double y) {
    wCenter.x = x;
    wCenter.y = y;
    center = space->toCS(wCenter);
    path.push_back(center);
  }

  virtual void translate(double dx, double dy) {
    wCenter.x += dx;
    wCenter.y += dy;
    center = space->toCS(wCenter);
    if (dx != 0 || dy != 0) {
      path.push_back(center);
    }
  }

  void setTrace(bool on) {
    trace = on;
  }

  void setLabelColor(Uint8 r, Uint8 g, Uint8 b) {
    labelColor = SDLColor(r, g, b);
  }

  void setLabel(string newLabel) {
    label = newLabel;
  }

  void setTraceColor(Uint8 r, Uint8 g, Uint8 b) {
    traceColor = SDLColor(r, g, b);
  }

  SDLCanvasSpace* getSpace()   const { return space; }
  SDL_Surface*    getSurface() const { return surface; }

  SDLWorldPoint   getCenter() const { return wCenter; }
  double          getX()      const { return wCenter.x; }
  double          getY()      const { return wCenter.y; }

protected:

  SDLCanvasPoint getCanvasCenter() const { 
    return center;
  }

  Uint16 getCenterX() const { return center.x; }
  Uint16 getCenterY() const { return center.y; }

private:
  SDL_Surface* surface;
  SDLCanvasSpace* space;
  SDLWorldPoint wCenter;
  SDLCanvasPoint center;
  string label;
  bool trace;
  list<SDLCanvasPoint> path;
  SDLColor labelColor;
  SDLColor traceColor;
};

#endif
