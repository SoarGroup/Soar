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
