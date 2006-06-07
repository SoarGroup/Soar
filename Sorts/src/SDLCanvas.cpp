#include "SDLCanvas.h"
#include <signal.h>
#ifdef USE_CANVAS


SDLCanvas::SDLCanvas()
{
  screen = NULL;
}

SDLCanvas::~SDLCanvas() {
  for(list<SDLCanvasShape*>::iterator
      i  = shapes.begin();
      i != shapes.end();
      ++i)
  {
    delete *i;
  }
}

int SDLCanvas::init(double worldWidth, double worldHeight, double scale) {
  worldw = worldWidth;
  worldh = worldHeight;
  space = SDLCanvasSpace(scale);
  canvasw = space.toCS(worldw);
  canvash = space.toCS(worldh);

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return 1;
  }
  screen = SDL_SetVideoMode(canvasw, canvash, 32, SDL_SWSURFACE);
  if (screen == NULL) {
    return 1;
  }
  
  signal(SIGSEGV, SIG_DFL);
  return 0;
}

int SDLCanvas::redraw() {
  if (SDL_MUSTLOCK(screen)) {
    if (SDL_LockSurface(screen) < 0) {
      return 1;
    }
  }

  // background
  boxRGBA(screen, 0, 0, canvasw, canvash, 0, 0, 0, 255);
  
  for(list<SDLCanvasShape*>::iterator
      i  = shapes.begin();
      i != shapes.end();
      ++i)
  {
    (*i)->draw();
  }
  
  for(list<TempShape>::iterator
      i  = temps.begin();
      i != temps.end();
      ++i)
  {
    i->shape->draw();
    i->count--;
    if (i->count == 0) {
      delete i->shape;
      temps.erase(i);
    }
  }

  if (SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }

  SDL_UpdateRect(screen, 0, 0, canvasw, canvash);
  return 0;
}

SDLCanvasCircle* SDLCanvas::makeCircle(double cx, double cy, double r) {
  SDLCanvasCircle* c = new SDLCanvasCircle(screen, &space, cx, cy, r, "");
  shapes.push_back(c);
  return c;
}

SDLCanvasDirCircle* SDLCanvas::makeDirCircle(double cx, double cy, double r, double a) {
  SDLCanvasDirCircle* c = new SDLCanvasDirCircle(screen, &space, cx, cy, r, 0, "");
  shapes.push_back(c);
  return c;
}

SDLCanvasRectangle* SDLCanvas::makeRectangle(double cx, double cy, double w, double h, double a) {
  SDLCanvasRectangle* r = new SDLCanvasRectangle(screen, &space, cx, cy, w, h, a, "");
  shapes.push_back(r);
  return r;
}

SDLCanvasCompound* SDLCanvas::makeCompound(double cx, double cy) {
  SDLCanvasCompound* c = new SDLCanvasCompound(screen, &space, cx, cy);
  shapes.push_back(c);
  return c;
}

SDLCanvasTrackedShape* SDLCanvas::makeTracker(double sx, double sy, double dx, double dy, bool isStart) {
  SDLCanvasStableLine* l = new SDLCanvasStableLine(screen, &space, sx, sy, dx, dy);
  SDLCanvasTrackedShape* t;
  if (isStart) {
    t = new SDLCanvasTrackedShape(screen, &space, true, sx, sy, l, "");
  }
  else {
    t = new SDLCanvasTrackedShape(screen, &space, false, dx, dy, l, "");
  }
  shapes.push_back(t);
  shapes.push_back(l);
  return t;
}

SDLCanvasCircle* SDLCanvas::makeTempCircle(double cx, double cy, double r, int t) {
  TempShape ts;
  SDLCanvasCircle* c = new SDLCanvasCircle(screen, &space, cx, cy, r, "");
  ts.shape = c;
  ts.count = t;
  temps.push_back(ts);
  return c;
}

SDLCanvasDirCircle* SDLCanvas::makeTempDirCircle(double cx, double cy, double r, double a, int t) {
  TempShape ts;
  SDLCanvasDirCircle* c = new SDLCanvasDirCircle(screen, &space, cx, cy, r, 0, "");
  ts.shape = c;
  ts.count = t;
  temps.push_back(ts);
  return c;
}

SDLCanvasRectangle* SDLCanvas::makeTempRectangle(double cx, double cy, double w, double h, double a, int t) {
  TempShape ts;
  SDLCanvasRectangle* r = new SDLCanvasRectangle(screen, &space, cx, cy, w, h, a, "");
  ts.shape = r;
  ts.count = t;
  temps.push_back(ts);
  return r;
}


void SDLCanvas::remove(SDLCanvasShape* shape) {
  list<SDLCanvasShape*>::iterator p = find(shapes.begin(), shapes.end(), shape);
  if (p != shapes.end()) {
    shapes.erase(p);
    delete shape;
  }
}

bool SDLCanvas::initted() {
  return screen != NULL;
}

void SDLCanvas::clear() {
  for(list<SDLCanvasShape*>::iterator
      i  = shapes.begin();
      i != shapes.end();
      ++i)
  {
    delete *i;
  }

  for(list<TempShape>::iterator
      i  = temps.begin();
      i != temps.end();
      ++i)
  {
    delete i->shape;
  }

  shapes.clear();
  temps.clear();
}

#endif
