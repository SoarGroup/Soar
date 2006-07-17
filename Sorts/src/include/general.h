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
#ifndef general_h
#define general_h

#include <list>
#include <utility>
#include <iostream>

#include "Vec2d.h"
#include "Circle.h"

#include "sml_Client.h"

#include "GameObj.H"

#define SHAPE_CIRCLE 1
#define SHAPE_RECTANGLE 3

#define PI 3.141592654

#define SHAPE_CIRCLE 1
#define SHAPE_RECTANGLE 3

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
Vec2d getDamageVector(int damageDir);

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

  coordinate(int _x, int _y) {
    x = _x; y = _y;
  }

  coordinate() {}

  bool operator==(const coordinate& c) {
    return x == c.x and y == c.y;
  }
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

struct Line {
  coordinate a;
  coordinate b;

  Line(int x1, int y1, int x2, int y2) : a(x1, y1), b(x2, y2) { }

  bool operator==(const Line& l) {
    return (a == l.a and b == l.b) or
           (a == l.b and b == l.a);
  }

  bool adjacent(const Line& l) {
    return a == l.a or a == l.b or b == l.a or b == l.b;
  }
};

double coordDistance(coordinate c1, coordinate c2);
double coordDistanceSq(coordinate c1, coordinate c2);
bool operator ==(const coordinate& c1, const coordinate& c2);
ostream& operator << (ostream& os, const coordinate& c);


int getRelativeSector(int fromX, int fromY, int fromHeading, int toX, int toY);

  /******************************************************
 * WEAPON RELATED FUNCTIONS                           *
 ******************************************************/
double weaponDamageRate(GameObj* gob);
bool canHit(GameObj *atk, GameObj *tgt);
bool canHit(GameObj *gob, const Circle& c, bool isGround);
bool canHit(GameObj* atk, const Vec2d& loc, GameObj *tgt);
bool canHit(const Circle& c1, const Circle& c2, double range); 

void positionsOnCircle 
( const Vec2d& center, 
  const Vec2d& firstPos, 
  double chordLen, 
  list<Vec2d>& positions);


unsigned long gettime();

int gobX(GameObj* gob);
int gobY(GameObj* gob);

bool damageTaken(int dir, uint4 dmg_vector);

#endif
