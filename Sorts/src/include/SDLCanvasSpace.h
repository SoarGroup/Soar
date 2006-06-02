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
