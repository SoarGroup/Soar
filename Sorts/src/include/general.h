#ifndef general_h
#define general_h

#include <list>
#include <utility>
#include <iostream>

#include "Circle.h"
#include "Vec2d.h"

#include "sml_Client.h"

#include "GameObj.H"

#define PI 3.141592654

using namespace std;

// I had to move this here because of forward declaration crazyness
typedef list<pair<string, int> > groupPropertyList;

string catStrInt(const char* str, int x);

string int2str(int x);

const char* getCommandParameter(sml::Identifier* c, const char *name);

double squaredDistance(int x1, int y1, int x2, int y2);
double distance(int x1, int y1, int x2, int y2);

// translates the heading data from the heading variable
// in the game object into a vector (i, j). 
Vec2d getHeadingVector(int gameHeading);

struct VisionParameterStruct {
  int centerX;
  int centerY;
  int viewWidth;
  int focusX;
  int focusY;
  bool ownerGrouping;
  int numObjects;
  int groupingRadius;
};

struct coordinate {
  int x;
  int y;
};

enum BuildingType { 
  CONTROL_CENTER=0, 
  BARRACKS=1, 
  FACTORY=2 
}; // just for game 3

enum TrainingType {
  WORKER=0,
  MARINE=1,
  TANK=2
};

struct line {
  coordinate a;
  coordinate b;
};

double coordDistance(coordinate c1, coordinate c2);
double coordDistanceSq(coordinate c1, coordinate c2);
bool operator ==(const coordinate& c1, const coordinate& c2);
ostream& operator << (ostream& os, const coordinate& c);


/******************************************************
 * WEAPON RELATED FUNCTIONS                           *
 ******************************************************/
double weaponDamageRate(GameObj* gob);
bool canHit(GameObj *atk, GameObj *tgt);
bool canHit(GameObj *gob, const Circle& c, bool isGround);
bool canHit(GameObj* atk, const Circle& loc, GameObj *tgt);
bool canHit(const Circle& c1, const Circle& c2, double range); 

void positionsOnCircle (const Vec2d& center, const Vec2d& firstPos, 
  double chordLen, list<Vec2d>& positions);

#endif
