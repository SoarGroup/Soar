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

private:
  double x2;
  double y2;
  Uint16 cx2;
  Uint16 cy2;
  SDLColor lineColor;
};

#endif
