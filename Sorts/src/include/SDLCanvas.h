#ifndef SDLCanvas_h
#define SDLCanvas_h

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
