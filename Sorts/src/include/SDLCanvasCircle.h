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
