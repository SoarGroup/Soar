#include "general.h"
#include <cmath>
#include <iostream>
#include <sstream>

using namespace std;

string catStrInt(const char* str, int x) {
    ostringstream sstr;
    sstr << str << x;
    return sstr.str();
}

string int2str(int x) {
    ostringstream sstr;
    sstr << x;
    return sstr.str();
}

const char* getCommandParameter(sml::Identifier* c, const char *name) {
    const char *val = c->GetParameterValue(name);
    if (strlen(val) == 0) {
        cout << "Error: Parameter " << name << " does not exist." << endl;
        fflush(stdout);
        exit(1);
    }
    return val;
}

double squaredDistance(int x1, int y1, int x2, int y2) {
  return ((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

void getHeadingVector(int gameHeading, double *i, double *j) {
  double angle = 2 * PI / 32 * gameHeading;
  *i = -1 * cos(angle);
  *j = sin(angle);
}

double coordDistanceSq(coordinate c1, coordinate c2) {
  return ((c2.x-c1.x)*(c2.x-c1.x)+(c2.y-c1.y)*(c2.y-c1.y));
}

double coordDistance(coordinate c1, coordinate c2) {
  return sqrt(((c2.x-c1.x)*(c2.x-c1.x)+(c2.y-c1.y)*(c2.y-c1.y)));
}

bool operator ==(const coordinate& c1, const coordinate& c2) {
  return ((c1.x == c2.x) && (c1.y == c2.y));
}

ostream& operator << (ostream& os, const coordinate& c) {
   return os<< c.x << "," << c.y <<endl;
}


