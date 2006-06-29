#ifndef SortsCanvas_h
#define SortsCanvas_h

#include <map>

#include "GameObj.H"
#include "PerceptualGroup.h"

#include "SDLCanvas.h"

using namespace std;

struct CanvasObjInfo {
  SDLCanvasCompound*     compound;
  SDLCanvasCircle*       mainCircle;
  SDLCanvasTrackedShape* tracker;
  SDLCanvasStableLine*   trackingLine;
  SDLColor               origColor;
  int                    flashColorCycles;

  CanvasObjInfo() : origColor(255, 255, 255) {
    compound = NULL;
    mainCircle = NULL;
    tracker = NULL;
    trackingLine = NULL;
    flashColorCycles = 0;
  }
};

struct CanvasGroupInfo {
  SDLCanvasCompound*     compound;
  SDLCanvasRectangle*       mainRectangle;

  CanvasGroupInfo() {
    compound = NULL;
    mainRectangle = NULL;
  }
};

class SortsCanvas {
public:
  SortsCanvas();

  void init(double ww, double wh, double scale);
  bool initted();
  void clear();
  void registerSGO(SoarGameObject* sgo);
  void unregisterSGO(SoarGameObject* sgo);
  void resetSGO(SoarGameObject* sgo);

  void registerGroup(PerceptualGroup* group);
  void unregisterGroup(PerceptualGroup* group);

  void setColor(SoarGameObject* sgo, Uint8 r, Uint8 g, Uint8 b);
  void flashColor(SoarGameObject* sgo, Uint8 r, Uint8 g, Uint8 b, int cycles);
  void update();

  void trackDestination(SoarGameObject* sgo, double destx, double desty);
  void stopTracking(CanvasObjInfo& obj);

  bool sgoRegistered(SoarGameObject* sgo);

  SDLCanvasCircle* makeTempCircle(double cx, double cy, double r, int t) {
    return canvas.makeTempCircle(cx, cy, r, t);
  }

  void drawLine(double x1, double y1, double x2, double y2);
  void setStatus(string);
  void clearStatus();
  
  void setSoarStatus(string);

private:
  SDLCanvas canvas;
  CanvasObjInfo statusObj;
  CanvasObjInfo soarStatObj;
public:
  map<SoarGameObject*, CanvasObjInfo> canvasObjs;
  map<PerceptualGroup*, CanvasGroupInfo> canvasGroups;
  int updateCounter;
};

#endif
