#include "general.h"
#include <math.h>
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

double squaredDistance(double x1, double y1, double x2, double y2) {
  return ((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

void getHeadingVector(int gameHeading, double *i, double *j) {
  double angle = 2 * PI / 32 * gameHeading;
  *i = -1 * cos(angle);
  *j = sin(angle);
}
