
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
#ifndef SortsCanvas_h
#define SortsCanvas_h

#include <map>

#include "GameObj.H"
#include "PerceptualGroup.h"

#ifndef NO_CANVAS_COMPILED
#include "SDLCanvas.h"
#endif

using namespace std;

#ifndef NO_CANVAS_COMPILED
struct CanvasObjInfo {
  SDLCanvasCompound*     compound;
  SDLCanvasShape*       mainShape;
  SDLCanvasTrackedShape* tracker;
  SDLCanvasStableLine*   trackingLine;
  SDLColor               origColor;
  int                    flashColorCycles;

  CanvasObjInfo() : origColor(255, 255, 255) {
    compound = NULL;
    mainShape = NULL;
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
  void setCommandStatus(string);

private:
  SDLCanvas canvas;
  CanvasObjInfo statusObj;
  CanvasObjInfo soarStatObj;
  CanvasObjInfo commandStatObj;
public:
  map<SoarGameObject*, CanvasObjInfo> canvasObjs;
  map<PerceptualGroup*, CanvasGroupInfo> canvasGroups;
  int updateCounter;
};
#else
class CanvasObjInfo {};
class SortsCanvas {
public:
  SortsCanvas() {};

  void init(double ww, double wh, double scale) {}
  bool initted() { return false; }
  void clear() {}
  void registerSGO(SoarGameObject* sgo) {}
  void unregisterSGO(SoarGameObject* sgo) {}
  void resetSGO(SoarGameObject* sgo) {}

  void registerGroup(PerceptualGroup* group) {}
  void unregisterGroup(PerceptualGroup* group) {}

  void setColor(SoarGameObject* sgo, Uint8 r, Uint8 g, Uint8 b) {}
  void flashColor(SoarGameObject* sgo, Uint8 r, Uint8 g, Uint8 b, int cycles) {}
  void update() {}

  void trackDestination(SoarGameObject* sgo, double destx, double desty) {}
  void stopTracking(CanvasObjInfo& obj) {}

  bool sgoRegistered(SoarGameObject* sgo) { return false; }

  void drawLine(double x1, double y1, double x2, double y2) {}
  void setStatus(string) {}
  void clearStatus() {}
  
  void setSoarStatus(string) {}
  void setCommandStatus(string) {}

};
#endif

#endif
