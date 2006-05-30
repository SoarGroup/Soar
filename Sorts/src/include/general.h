#ifndef general_h
#define general_h

#include<list>
#include<utility>
#include<iostream>

#include "sml_Client.h"

#define PI 3.141592654

using namespace std;

// I had to move this here because of forward declaration crazyness
typedef list<pair<string, int> > groupPropertyList;
struct groupPropertyStruct {
  list<pair<string, int> >    stringIntPairs;
  list<pair<string, float> >  stringFloatPairs;
  list<pair<string, string> > stringStringPairs;
  list<int> regionsOccupied;
};

string catStrInt(const char* str, int x);

string int2str(int x);

const char* getCommandParameter(sml::Identifier* c, const char *name);

double squaredDistance(int x1, int y1, int x2, int y2);
double distance(int x1, int y1, int x2, int y2);

// translates the heading data from the heading variable
// in the game object into a vector (i, j). 
void getHeadingVector(int gameHeading, double *i, double *j);

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

double coordDistance(coordinate c1, coordinate c2);
double coordDistanceSq(coordinate c1, coordinate c2);
bool operator ==(const coordinate& c1, const coordinate& c2);
ostream& operator << (ostream& os, const coordinate& c);

#endif
